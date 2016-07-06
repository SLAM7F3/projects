// Potentially very important note: On 8/31/04, we realized that it
// may turn out to be much faster to hang onto voxels within the
// hashtable which need to be removed from the final image rather than
// to dynamically delete them.  We would need to replace their counts
// and or probability values with negative sentinel values.  

// ==========================================================================
// VolumetricCoincidenceProcessor class member functions
// ==========================================================================
// Last updated on 9/1/12; 1/17/13; 1/22/13; 4/5/14
// ==========================================================================

#include <algorithm>
#include "math/basic_math.h"
#include "geometry/bounding_box.h"
#include "general/filefuncs.h"
#include "plot/metafile.h"
#include "math/mypolynomial.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "coincidence_processing/VolumetricCoincidenceProcessor.h"
#include "coincidence_processing/voxel_coords.h"

using std::cin;
using std::cout;
using std::endl;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void VolumetricCoincidenceProcessor::initialize_member_objects()
{
   imagenumber=0;
   ordered_voxels_ptr=NULL;
}		       

void VolumetricCoincidenceProcessor::allocate_member_objects()
{
   ordered_voxels_ptr=new vector<pair<float,long> >(2*n_expected_points);
   voxels_map_ptr=new VOXEL_MAP;
}		       

VolumetricCoincidenceProcessor::VolumetricCoincidenceProcessor(
   int n_points)
{	
   n_expected_points=basic_math::max(250000,n_points);

   initialize_member_objects();
   allocate_member_objects();
}

// Copy constructor:

VolumetricCoincidenceProcessor::VolumetricCoincidenceProcessor(
   const VolumetricCoincidenceProcessor& v)
{
   initialize_member_objects();
   allocate_member_objects();
   docopy(v);
}

VolumetricCoincidenceProcessor::~VolumetricCoincidenceProcessor()
{
//   cout << "inside VCP destructor" << endl;

   delete ordered_voxels_ptr;
   delete voxels_map_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,
const VolumetricCoincidenceProcessor& vcp)
{
   outstream << " mdim = " << vcp.get_mdim() 
             << " ndim = " << vcp.get_ndim()
             << " pdim = " << vcp.get_pdim() << endl;

   outstream << "xlo = " << vcp.get_xlo() 
             << " xhi = " << vcp.get_xhi() << endl;
   outstream << "ylo = " << vcp.get_ylo() 
             << " yhi = " << vcp.get_yhi() << endl;
   outstream << "zlo = " << vcp.get_zlo() 
             << " zhi = " << vcp.get_zhi() << endl;
   outstream << "dx = " << vcp.get_dx() 
             << " dy = " << vcp.get_dy() 
             << " dz = " << vcp.get_dz() << endl;

   outstream << "voxels_map.size() = " << vcp.size() << endl;

   return outstream;
}

// ---------------------------------------------------------------------
void VolumetricCoincidenceProcessor::docopy(
   const VolumetricCoincidenceProcessor& v)
{
//   cout << "inside VCP::docopy()" << endl;
   
   imagenumber=v.imagenumber;
   mdim=v.mdim;
   ndim=v.ndim;
   pdim=v.pdim;
   xlo=v.xlo;
   xhi=v.xhi;
   ylo=v.ylo;
   yhi=v.yhi;
   zlo=v.zlo;
   zhi=v.zhi;
   dx=v.dx;
   dy=v.dy;
   dz=v.dz;

   if (v.ordered_voxels_ptr != NULL)
   {
      if (ordered_voxels_ptr==NULL)
      {
         ordered_voxels_ptr=new vector<pair<float,long> >(
            *v.ordered_voxels_ptr);
      }
      else
      {
         *ordered_voxels_ptr=*v.ordered_voxels_ptr;
      }
   }

   if (v.voxels_map_ptr != NULL)
   {
      if (voxels_map_ptr==NULL)
      {
         voxels_map_ptr=new VOXEL_MAP(*v.voxels_map_ptr);
      }
      else
      {
         *voxels_map_ptr=*v.voxels_map_ptr;
      }
   }
   
}

// ==========================================================================

// It is important to recall that transverse bin size is set by the
// sensor's angular resolution times the range to the volume of
// interest.  On the other hand, the longitudinal bin size is set by
// the sensor's temporal resolution which is independent of range.  In
// the special case where the sensor is basically static and points
// along the +yhat direction, we can meaningfully differentiate
// between the transverse and longitudinal directions.

void VolumetricCoincidenceProcessor::initialize_coord_system(
   const threevector& XYZ_minimum,const threevector& XYZ_maximum,
   float binlength)
{
   initialize_coord_system(XYZ_minimum,XYZ_maximum,binlength,binlength);
}

void VolumetricCoincidenceProcessor::initialize_coord_system(
   const threevector& XYZ_minimum,const threevector& XYZ_maximum,
   float XY_binlength,float Z_binlength)
{
   xlo=XYZ_minimum.get(0);
   ylo=XYZ_minimum.get(1);
   zlo=XYZ_minimum.get(2);
   xhi=XYZ_maximum.get(0);
   yhi=XYZ_maximum.get(1);
   zhi=XYZ_maximum.get(2);

   dx=dy=XY_binlength;
   dz=Z_binlength;

   mdim=basic_math::round((xhi-xlo)/dx)+1;
   ndim=basic_math::round((yhi-ylo)/dy)+1;
   pdim=basic_math::round((zhi-zlo)/dz)+1;
   
   xhi=xlo+(mdim-1)*dx;
   yhi=ylo+(ndim-1)*dy;
   zhi=zlo+(pdim-1)*dz;

/*
   cout << "mdim = " << mdim << " ndim = " << ndim << " pdim = " << pdim
        << endl;
   cout << "xlo = " << xlo << " xhi = " << xhi << endl;
   cout << "ylo = " << ylo << " yhi = " << yhi << endl;
   cout << "zlo = " << zlo << " zhi = " << zhi << endl;
   cout << "dx = " << dx << " dy = " << dy << " dz = " << dz << endl;
*/

}		       

// ==========================================================================
// Counts accumulation member functions
// ==========================================================================

// Member function increment_voxel_counts[_prob] increases the counts
// [probability] part of the pair corresponding to the voxel specified
// by input key within *voxels_map_ptr by delta_counts
// [delta_prob].

int VolumetricCoincidenceProcessor::increment_voxel_counts(
   long key,int delta_counts)
{
   VOXEL_MAP::iterator voxel_iter=voxels_map_ptr->find(key);
   if (voxel_iter==voxels_map_ptr->end()) 
   {
      (*voxels_map_ptr)[key]=Voxel_type(delta_counts,0);
   }
   else
   {
      voxel_iter->second.first += delta_counts;
   }
   return voxel_iter->second.first;
}

void VolumetricCoincidenceProcessor::set_voxel_prob(
   unsigned int m,unsigned int n,unsigned int p,float prob)
{
   long key=mnp_to_key(m,n,p);
   set_voxel_prob(key,prob);
}

void VolumetricCoincidenceProcessor::set_voxel_prob(long key,float prob)
{
   VOXEL_MAP::iterator voxel_iter=voxels_map_ptr->find(key);
   if (voxel_iter==voxels_map_ptr->end()) 
   {
      (*voxels_map_ptr)[key]=Voxel_type(0,prob);
   }
   else
   {
      voxel_iter->second.second=prob;
   }
}

void VolumetricCoincidenceProcessor::increment_voxel_prob(long key,float prob)
{
   VOXEL_MAP::iterator voxel_iter=voxels_map_ptr->find(key);
   if (voxel_iter==voxels_map_ptr->end()) 
   {
      (*voxels_map_ptr)[key]=Voxel_type(0,prob);
   }
   else
   {
      voxel_iter->second.second += prob;
   }
}

// ---------------------------------------------------------------------
// Member function accumulate_points takes in array
// pixels_with_counts[] which encodes those APDs that actually fired
// for a particular pulse as well as array
// xyz_pnts_associated_with_APD_counts[].  This method increments the
// count number for the entriess within *voxels_map_ptr
// corresponding to the input (x,y,z) points.

void VolumetricCoincidenceProcessor::accumulate_points(
   double x,double y,double z)
{
   if (xyz_inside_volume(x,y,z))
   {
      long key=xyz_to_key(x,y,z);
      increment_voxel_counts(key);
   }
}

void VolumetricCoincidenceProcessor::accumulate_points_and_counts(
   double x,double y,double z,int n_counts)
{
//   cout << "inside VCP::accumulate_points() #2" << endl;
   if (xyz_inside_volume(x,y,z))
   {
      long key=xyz_to_key(x,y,z);
      increment_voxel_counts(key,n_counts);
   }
}

void VolumetricCoincidenceProcessor::accumulate_points_and_probs(
   double x,double y,double z,double p)
{
//   cout << "inside VCP::accumulate_points_and_probs()" << endl;
   if (xyz_inside_volume(x,y,z))
   {
      long key=xyz_to_key(x,y,z);
      increment_voxel_counts(key);
      set_voxel_prob(key,p);
   }
}

void VolumetricCoincidenceProcessor::accumulate_points_and_calculate_probs(
   double x,double y,double z,double numer,double denom)
{
//   cout << "inside VCP::accumulate_points_and_probs()" << endl;
   if (xyz_inside_volume(x,y,z))
   {
      long key=xyz_to_key(x,y,z);
      increment_voxel_counts(key,int(numer));
      set_voxel_prob(key,numer/denom);
   }
}

// Member function accumulate_points_and_increment_probs() 

void VolumetricCoincidenceProcessor::accumulate_points_and_increment_probs(
   double x,double y,double z,double p)
{
//   cout << "inside VCP::accumulate_points_and_increment_probs()" << endl;
   if (xyz_inside_volume(x,y,z))
   {
      long key=xyz_to_key(x,y,z);

/*
      int prev_counts;
      double prev_prob;
      key_to_voxel_counts_and_prob(key,prev_counts,prev_prob);
      int curr_counts=increment_voxel_counts(key);
      
      double w_prev=double(prev_counts)/double(curr_counts);
      double curr_p=w_prev*prev_prob+(1-w_prev)*p;
      set_voxel_prob(key,curr_p);
*/

// FAKE FAKE:  Sun Nov 20, 2011 at 12:04 pm

      set_voxel_prob(key,p);
      
   }
}

void VolumetricCoincidenceProcessor::accumulate_points(
   const threevector& curr_point)
{
   if (xyz_inside_volume(curr_point))
   {
      long key=xyz_to_key(curr_point);
      increment_voxel_counts(key);
   }
}

void VolumetricCoincidenceProcessor::accumulate_points(
   const threevector& curr_point,int n_counts)
{
   if (xyz_inside_volume(curr_point))
   {
      long key=xyz_to_key(curr_point);
      increment_voxel_counts(key,n_counts);
   }
}

// ---------------------------------------------------------------------
// Method generate_counts_histogram scans through all of the entries
// within *voxels_map_ptr and forms a counts frequency
// array.  It plots this array as a frequency histogram.

void VolumetricCoincidenceProcessor::generate_counts_histogram()
{

// Scan through current voxels map and fill counts STL vector

   int max_counts=-1;
   
   vector<double>* counts_ptr=new vector<double>;

   for (VOXEL_MAP::iterator iter=voxels_map_ptr->begin(); 
        iter != voxels_map_ptr->end(); iter++)
   {
      Voxel_type p(iter->second);
      int curr_counts=p.second;
      counts_ptr->push_back(curr_counts);
      max_counts=basic_math::max(max_counts,curr_counts);
   }

   prob_distribution prob(*counts_ptr,max_counts+1);
   prob.set_xmin(0);
   prob.set_xmax(max_counts);
   prob.set_xtic(5);
   prob.set_xsubtic(5);
   prob.set_freq_histogram(true);
   prob.set_densityfilenamestr(
      "counts_"+stringfunc::number_to_string(imagenumber+1)+".meta");
   prob.set_xlabel("Counts");
   prob.write_density_dist();

   delete counts_ptr;
}

// ---------------------------------------------------------------------
// Method generate_p_distribution() scans through all of the
// entries within *voxels_map_ptr and forms a probs frequency
// array.  It plots this array as a frequency histogram.

void VolumetricCoincidenceProcessor::generate_p_distribution()
{

// Scan through current voxels map and fill counts STL vector

   vector<double>* probs_ptr=new vector<double>;

   for (VOXEL_MAP::iterator iter=voxels_map_ptr->begin(); 
        iter != voxels_map_ptr->end(); iter++)
   {
      Voxel_type p(iter->second);
      double curr_p=p.second;
      probs_ptr->push_back(curr_p);
   }

   prob_distribution prob(*probs_ptr,1000);
   prob.set_xmin(0);
   prob.set_xmax(1);
   prob.set_xtic(0.2);
   prob.set_xsubtic(0.1);
//   prob.set_densityfilenamestr("
//      "counts_"+stringfunc::number_to_string(imagenumber+1)+".meta");
   prob.set_xlabel("P values");
   prob.write_density_dist();

   delete probs_ptr;
}

// ---------------------------------------------------------------------
// Method generate_p_vs_z_profiles() iterates over all voxels and
// records their Z and P values.  It then generates and outputs a
// "scatter plot" metafile which illustrates Ps against Zs.

void VolumetricCoincidenceProcessor::generate_p_vs_z_profiles()
{
   outputfunc::write_banner("Generating P vs Z profiles");

   const unsigned int n_xsteps=3;
   const unsigned int n_ysteps=3;
   double delta_x=(xhi-xlo)/(n_xsteps-1);
   double delta_y=(yhi-ylo)/(n_ysteps-1);

   metafile curr_metafile;

   for (unsigned int px=0; px<n_xsteps; px++)
   {
      double xmin=xlo+px*delta_x;
      double xmax=xmin+delta_x;
      for (unsigned int py=0; py<n_ysteps; py++)
      {
         cout << endl;
         cout << "px = " << px << " py = " << py << endl;
         double ymin=ylo+py*delta_y;
         double ymax=ymin+delta_y;
         bounding_box XY_bbox(xmin,xmax,ymin,ymax);

// First scan through *voxels_map_ptr and fill Z, P and logP STL
// vectors:

         vector<double>* Z_ptr=new vector<double>;
         vector<double>* P_ptr=new vector<double>;
         vector<double>* logP_ptr=new vector<double>;
   
         int counter=0;
         for (VOXEL_MAP::iterator iter=voxels_map_ptr->begin(); 
              iter != voxels_map_ptr->end(); iter++)
         {
            long key=iter->first;
            double curr_x=key_to_x(key);
            double curr_y=key_to_y(key);
            if (!XY_bbox.point_inside(curr_x,curr_y)) continue;
            
            counter++;
            if (counter%10 != 0) continue;
      
            double curr_P=iter->second.second;

            const double min_P=0.0;
//            const double min_P=0.25;
            if (curr_P <= min_P) continue;

            Z_ptr->push_back(key_to_z(key));
            P_ptr->push_back(curr_P);
            logP_ptr->push_back(log(curr_P));
         } // loop over voxels map iterator

         cout << "logP.size() = " << logP_ptr->size() << endl;

/*
// Fit line to lnP vs Z:

         double chisq;
         mypolynomial poly(1);
         poly.fit_coeffs_using_residuals(*Z_ptr,*logP_ptr,chisq);
         cout << "Line fit poly = " << poly << endl;
         cout << "chisq = " << chisq << endl;

         double P0=exp(poly.get_coeff(0));
         double lambda=poly.get_coeff(1);
         cout << "P0 = " << P0 << " lambda = " << lambda
              << " 1/lambda = " << 1.0/lambda << endl;

         double delta_z=1;
         int n_zbins=(zhi-zlo)/delta_z+1;
         vector<double> fitted_z,fitted_p;
         
         for (unsigned int n=0; n<n_zbins; n++)
         {
            fitted_z.push_back(zlo+n*delta_z);
            fitted_p.push_back(P0*exp(lambda*fitted_z.back()));
         }
*/
       
         string metafile_name="p_vs_z_x"+stringfunc::number_to_string(px)+
            "_y"+stringfunc::number_to_string(py);
         string title="P vs Z";
         string x_label="Z (meters)";
         string y_label="Detection probability";

         curr_metafile.set_parameters(
            metafile_name,title,x_label,y_label,zlo,zhi,0,1,0.1,0.05);
         curr_metafile.openmetafile();
         curr_metafile.write_header();
         curr_metafile.write_markers(*Z_ptr,*P_ptr);
//         curr_metafile.write_curve(fitted_z,fitted_p,colorfunc::green);
         curr_metafile.closemetafile();
         filefunc::meta_to_jpeg(metafile_name);

         delete Z_ptr;
         delete P_ptr;
         delete logP_ptr;
      } // loop over py index
   } // loop over px index
}

// ---------------------------------------------------------------------
// Method generate_xyz_profiles() collapses the counts within the
// allowed VCP volume onto the x, y and z axes.  It then generates
// metafile plots of the x, y and z counts profiles.

void VolumetricCoincidenceProcessor::generate_xyz_profiles(
   string output_subdir)
{
   outputfunc::write_banner("Generating xyz profiles");
   
// First scan through *voxels_map_ptr and fill X,Y and Z STL vectors:

   vector<double>* X_ptr=new vector<double>;
   vector<double>* Y_ptr=new vector<double>;
   vector<double>* Z_ptr=new vector<double>;
   
   for (VOXEL_MAP::iterator iter=voxels_map_ptr->begin(); 
        iter != voxels_map_ptr->end(); iter++)
   {
      long key=iter->first;
      X_ptr->push_back(key_to_x(key));
      Y_ptr->push_back(key_to_y(key));
      Z_ptr->push_back(key_to_z(key));
   } // loop over voxels map iterator

/*
   prob_distribution prob_x(n_counts,x,100);
   prob_x.xmin=xlo;
   prob_x.xmax=xhi;
   prob_x.xtic=5;
   prob_x.xsubtic=2.5;
   prob_x.densityfilenamestr=output_subdir+
      "/xdist_"+stringfunc::number_to_string(imagenumber)+".meta";
   prob_x.xlabel="X (meters)";
   prob_x.write_density_dist();

   prob_distribution prob_y(n_counts,y,100);
   prob_y.xmin=ylo;
   prob_y.xmax=yhi;
   prob_y.xtic=5;
   prob_y.xsubtic=2.5;
   prob_y.densityfilenamestr=output_subdir+
      "/ydist_"+stringfunc::number_to_string(imagenumber)+".meta";
   prob_y.xlabel="Y (meters)";
   prob_y.write_density_dist();
*/

   prob_distribution prob_z(*Z_ptr,100);
   prob_z.set_xmin(zlo);
   prob_z.set_xmax(zhi);
   prob_z.set_xtic(5);
   prob_z.set_xsubtic(2.5);
   prob_z.set_densityfilenamestr(output_subdir+
   "/zdist_"+stringfunc::number_to_string(imagenumber)+".meta");
   prob_z.set_xlabel("Z (meters)");
   prob_z.write_density_dist();

   delete X_ptr;
   delete Y_ptr;
   delete Z_ptr;
}

// ---------------------------------------------------------------------
// Method compute_COM() returns the average of the non-empty voxels
// within *voxels_map_ptr weighted by their numbers of counts.

threevector VolumetricCoincidenceProcessor::compute_COM()
{
   
// First scan through *voxels_map_ptr and fill X,Y and Z STL vectors:

   vector<double>* X_ptr=new vector<double>;
   vector<double>* Y_ptr=new vector<double>;
   vector<double>* Z_ptr=new vector<double>;
   vector<double>* P_ptr=new vector<double>;

   for (VOXEL_MAP::iterator iter=voxels_map_ptr->begin(); 
        iter != voxels_map_ptr->end(); iter++)
   {
      long key=iter->first;
      X_ptr->push_back(key_to_x(key));
      Y_ptr->push_back(key_to_y(key));
      Z_ptr->push_back(key_to_z(key));
      P_ptr->push_back(iter->second.first);
   } // loop over voxels map iterator

   double xavg,yavg,zavg,psum;
   xavg=yavg=zavg=psum=0;
   for (unsigned int n=0; n<X_ptr->size(); n++)
   {
      xavg += X_ptr->at(n) * P_ptr->at(n);
      yavg += Y_ptr->at(n) * P_ptr->at(n);
      zavg += Z_ptr->at(n) * P_ptr->at(n);
      psum += P_ptr->at(n);
   }
   xavg /= psum;
   yavg /= psum;
   zavg /= psum;

   delete X_ptr;
   delete Y_ptr;
   delete Z_ptr;
   delete P_ptr;

   return threevector(xavg,yavg,zavg);
}

// ==========================================================================
// Gridding member functions
// ==========================================================================

// Member function x_to_m(), y_to_n(), ... , xyz_to_mnp() returns the
// pixel coordinates of the voxel closest to the input (x,y,z) point.

unsigned int VolumetricCoincidenceProcessor::x_to_m(double x) const
{
   unsigned int m=basic_math::round((x-xlo)/dx);
   m=basic_math::max(m,Unsigned_Zero);
   m=basic_math::min(m,mdim-1);
   return m;
}

unsigned int VolumetricCoincidenceProcessor::y_to_n(double y) const
{
   unsigned int n=basic_math::round((y-ylo)/dy);
   n=basic_math::max(n,Unsigned_Zero);
   n=basic_math::min(n,ndim-1);
   return n;
}

unsigned int VolumetricCoincidenceProcessor::z_to_p(double z) const
{
   unsigned int p=basic_math::round((z-zlo)/dz);
   p=basic_math::max(p,Unsigned_Zero);
   p=basic_math::min(p,pdim-1);
   return p;
}

void VolumetricCoincidenceProcessor::xyz_to_mnp(
   double x,double y,double z,
   unsigned int& m,unsigned int& n,unsigned int& p) const
{
   m=basic_math::round((x-xlo)/dx);
   n=basic_math::round((y-ylo)/dy);
   p=basic_math::round((z-zlo)/dz);

   m=basic_math::max(m,Unsigned_Zero);
   m=basic_math::min(m,mdim-1);
   n=basic_math::max(n,Unsigned_Zero);
   n=basic_math::min(n,ndim-1);
   p=basic_math::max(p,Unsigned_Zero);
   p=basic_math::min(p,pdim-1);
}

void VolumetricCoincidenceProcessor::xyz_to_mnp(
   const threevector& V,unsigned int& m,unsigned int& n,unsigned int& p) const
{
   xyz_to_mnp(V.get(0),V.get(1),V.get(2),m,n,p);
}

// ==========================================================================
// Counts and probability retrieval member functions
// ==========================================================================

bool VolumetricCoincidenceProcessor::key_to_voxel_counts_and_prob(
   long key,int& counts,double& prob) const
{
//   cout << "inside VCP::key_to_voxel_counts_and_prob()" << endl;
   counts=0;
   prob=0;
   
//   cout << "key = " << key << endl;
   VOXEL_MAP::iterator voxel_iter=voxels_map_ptr->find(key);
   if (voxel_iter== voxels_map_ptr->end()) return false;

   counts=voxel_iter->second.first;
   prob=voxel_iter->second.second;
//      cout << "counts = " << counts << " prob = " << prob << endl;
   return true;
}

bool VolumetricCoincidenceProcessor::mnp_to_voxel_counts_and_prob(
   int m,int n,int p,int& counts,double& prob) const
{
//   cout << "inside VCP::mnp_to_voxel_counts_and_prob()" << endl;
//   cout << "m = " << m << " n = " << n << " p = " << p << endl;
   
   long key=mnp_to_key(m,n,p);
   return key_to_voxel_counts_and_prob(key,counts,prob);
}

// ---------------------------------------------------------------------
// Method column_prob_integral() works with the voxel column
// corresponding to input integer indices m and n.  It integrates
// integrates the probabilities for all voxels with z >= z_start in
// this column.

double VolumetricCoincidenceProcessor::column_prob_integral(
   int m,int n,double z_start)
{
//   cout << "inside VCP::column_prob_integral()" << endl;

   int p_start=z_to_p(z_start);

   double prob_integral=0;
   for (unsigned int p=p_start; p<pdim; p++)
   {
      int curr_counts;
      double curr_prob;
      mnp_to_voxel_counts_and_prob(m,n,p,curr_counts,curr_prob);
      prob_integral += curr_prob;
   }

//   cout << "m = " << m << " n = " <<  n << " z_start = " << z_start 
//        << "p_start = " << p_start << " pdim =  " << pdim 
//        << " prob_integral = " << prob_integral << endl;

   return prob_integral;
}

// ---------------------------------------------------------------------
// Method integrate_probs_within_column_integrals() works with the voxel
// column corresponding to input integer indices m and n.  It forms a
// "cumulative probability distribution" as a function of decreasing Z
// by simply summing all probs within a voxel and its neighboring
// voxels with larger z values.


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!11
// Note added on Mon Dec 5, 2011:

// This method needs to be generalized to handle falling upwards as
// well as falling downwards case

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!11


void VolumetricCoincidenceProcessor::integrate_probs_within_column_integrals()
{
//   cout << "inside VCP::integrate_probs_within_column_integrals()" << endl;

   string banner="Integrating VCP probabilities within column integrals:";
   outputfunc::write_banner(banner);

   for (unsigned int m=0; m<mdim; m++)
   {
      outputfunc::update_progress_fraction(m,100,mdim);
      for (unsigned int n=0; n<ndim; n++)
      {
         double prob_integral=0;
         for (unsigned int p=pdim-1; p >= 0; p--)
         {
            VOXEL_MAP::iterator voxel_iter=mnp_to_voxel_iterator(m,n,p);
            if (voxel_iter==voxels_map_ptr->end()) continue;
            
            prob_integral += voxel_iter->second.second;
            voxel_iter->second.second=prob_integral;
         } // loop over index p

//         if (prob_integral > 0)
//         {
//            cout << "m = " << m 
//                 << " n = " << n 
//                 << " cum prob = " << prob_integral 
//                 << endl;
//         }

      } // loop over index n 
   } // loop over index m 
}

// ---------------------------------------------------------------------
// Member function get_cumulative_prob_integral() takes
// in m & n voxel coordinates along with some starting z value.  It
// sums and returns the probability values for all voxels labeled by m
// and n with z values greater [less] than z_start if fall_downwards_flag = 
// true [false].

double VolumetricCoincidenceProcessor::get_cumulative_prob_integral(
   int m,int n,double z_start,bool fall_downwards_flag)
{
//   cout << "inside VCP::get_cumulative_prob_integral()" << endl;

   int p_start=z_to_p(z_start);
//   cout << "m = " << m << " n = " << n << endl;
//   cout << "p_start = " << p_start << endl;

   int p_stop=pdim;
   int p_step=1;
   if (!fall_downwards_flag)
   {
      p_stop=-1;
      p_step=-1;
   }

   return get_cumulative_prob_integral(m,n,p_start,p_stop,p_step);
}

double VolumetricCoincidenceProcessor::get_cumulative_prob_integral(
   unsigned int m,unsigned int n,
   unsigned int p_start,unsigned int p_stop,unsigned int p_step)
{
//   cout << "inside VCP::get_cumulative_prob_integral()" << endl;

   double prob_integral=0;

//   const int min_n_counts=5;
   for (unsigned int p=p_start; p != p_stop; p += p_step)
   {
      VOXEL_MAP::iterator voxel_iter=voxels_map_ptr->find(mnp_to_key(m,n,p));
      if (voxel_iter == voxels_map_ptr->end()) continue;

// Ignore pressure integral contribution from any voxels which contain
// very small numbers of counts:

//      int n_counts=voxel_iter->second.first;
//      if (n_counts < min_n_counts) continue;

      prob_integral=voxel_iter->second.second;
      break;
   }

//   cout << "m = " << m << " n = " <<  n 
//        << " p_start = " << p_start << " p_stop = " << p_stop
//        << " p_step = " << p_step 
//        << " prob_integral = " << prob_integral << endl;

   return prob_integral;
}

// ---------------------------------------------------------------------
// Member function compute_counts_sum() iterates over all voxels and
// returns the integral of their counts.

int VolumetricCoincidenceProcessor::compute_counts_sum()
{
//   cout << "inside VCP::compute_counts_sum()" << endl;

   int counts_sum=0;
   for (VOXEL_MAP::iterator iter=voxels_map_ptr->begin(); 
        iter != voxels_map_ptr->end(); iter++)
   {
      counts_sum += iter->second.first;
   }
   return counts_sum;
}

// ---------------------------------------------------------------------
// Member function compute_prob_integral() iterates over all voxels
// and returns the integral of their probabilities.

double VolumetricCoincidenceProcessor::compute_prob_integral()
{
//   cout << "inside VCP::compute_prob_integral()" << endl;

   double prob_integral=0;
   for (VOXEL_MAP::iterator iter=voxels_map_ptr->begin(); 
        iter != voxels_map_ptr->end(); iter++)
   {
      prob_integral += iter->second.second;
   }
   return prob_integral;
}

// ---------------------------------------------------------------------
// Member function set_probs_to_renormalized_counts()

void VolumetricCoincidenceProcessor::set_probs_to_renormalized_counts()
{
//   cout << "inside VCP::set_probs_to_renormalized_counts()" << endl;

   int counts_sum=compute_counts_sum();
   for (VOXEL_MAP::iterator iter=voxels_map_ptr->begin(); 
        iter != voxels_map_ptr->end(); iter++)
   {
      iter->second.second=double(iter->second.first)/double(counts_sum);
   }
}

// ==========================================================================
// Noise reduction member functions
// ==========================================================================

// Member function delete_all_voxels() 

void VolumetricCoincidenceProcessor::delete_all_voxels()
{
   cout << "inside VCP::delete_all_voxels()" << endl;

   for (VOXEL_MAP::iterator iter=voxels_map_ptr->begin(); 
        iter != voxels_map_ptr->end(); iter++)
   {
      voxels_map_ptr->erase(iter);
   }
   cout << "voxels_map_ptr->size() = "
        << voxels_map_ptr->size() << endl;
}

// ---------------------------------------------------------------------
// Member function delete_isolated_voxels() scans over all entries
// within *voxels_map_ptr.  It deletes any "lonely" voxel which
// has no neighbor within delta_vox on the m,n,p lattice.  This method
// is intended to remove isolated noise counts in a 3D image.


void VolumetricCoincidenceProcessor::delete_isolated_voxels(
   int delta_vox,int min_neighbors,int min_avg_counts)
{
   cout << "inside VCP::delete_isolated_voxels()" << endl;
   cout << "Initially, voxels_map_ptr->size() = "
        << voxels_map_ptr->size() << endl;
   cout << "delta_vox = " << delta_vox << " min_neighbors = " 
        << min_neighbors << endl;

   for (VOXEL_MAP::iterator iter=voxels_map_ptr->begin(); 
        iter != voxels_map_ptr->end(); iter++)
   {
      long curr_key=iter->first;
      if (isolated_voxel(
         curr_key,delta_vox,min_neighbors,min_avg_counts))
      {
         voxels_map_ptr->erase(iter);
      }
   }

   cout << "At end of VCP::delete_isolated_voxels:" << endl;
   cout << "voxels_map_ptr->size() = "
        << voxels_map_ptr->size() << endl;
}

void VolumetricCoincidenceProcessor::delete_isolated_voxels(
   int delta_vox,int min_neighbors,double min_Z_threshold,int min_avg_counts)
{
   cout << "inside VCP::delete_isolated_voxels() #2" << endl;
   cout << "min_Z_threshold = " << min_Z_threshold << endl;
   cout << "voxels_map_ptr->size() = "
        << voxels_map_ptr->size() << endl;
   cout << "delta_vox = " << delta_vox << " min_neighbors = " 
        << min_neighbors << endl;

   int n_voxels_above_z_threshold=0;
   int n_erasures=0;
   for (VOXEL_MAP::iterator iter=voxels_map_ptr->begin(); 
        iter != voxels_map_ptr->end(); iter++)
   {
      long curr_key=iter->first;
      if (key_to_z(curr_key) < min_Z_threshold) continue;

      n_voxels_above_z_threshold++;
      if (isolated_voxel(curr_key,delta_vox,min_neighbors,min_avg_counts))
      {
         voxels_map_ptr->erase(iter);
         n_erasures++;
      }
   }

   cout << "At end of VCP::delete_isolated_voxels() #2" << endl;
   cout << "voxels_map_ptr->size() = "
        << voxels_map_ptr->size() << endl;
   cout << "n_voxels_above_z_threshold = "
        << n_voxels_above_z_threshold << endl;
   cout << "# voxels deleted = " << n_erasures << endl;
}

// ---------------------------------------------------------------------
// Boolean member function isolated_voxel() first converts an input
// key to mnp Voxel coords.  It next checks whether the voxel's
// immediate neighbors exist within *voxels_map_ptr.  If not, the
// point is declared to be "lonely" and this method returns true.

bool VolumetricCoincidenceProcessor::isolated_voxel(
   long curr_key,int delta_vox,int min_neighbors,int min_avg_counts) const
{
//   cout << "inside VCP::isolated_voxel()" << endl;
//   cout << "Delta_vox = " << delta_vox << " min_neighbors = " << min_neighbors
//        << endl;
   voxel_coords central_voxel=key_to_mnp(curr_key);

   int neighbor_counts_sum=0;
   for (unsigned int m=central_voxel.m-delta_vox; 
        m<=central_voxel.m+delta_vox; m++)
   {
      for (unsigned int n=central_voxel.n-delta_vox; 
           n<=central_voxel.n+delta_vox; n++)
      {
         for (unsigned int p=central_voxel.p-delta_vox; 
              p<=central_voxel.p+delta_vox; p++)
         {
//            cout << "m = " << m << " n = " << n << " p = " << p
//                 << " n_neighbors_with_counts = " 
//                 << n_neighbors_with_counts << endl;
            long key=mnp_to_key(m,n,p);
            VOXEL_MAP::iterator voxel_iter=voxels_map_ptr->find(key);
            if (voxel_iter==voxels_map_ptr->end()) continue;

            if (m != central_voxel.m && n != central_voxel.n &&
            p != central_voxel.p) 
            {
               neighbor_counts_sum += voxel_iter->second.first;
               if (neighbor_counts_sum > min_neighbors*min_avg_counts)
                  return false;

//               n_neighbors_with_counts++;
//               if (prob_integral >= min_neighbors*min_avg_prob)
//                  return false;
               
//               if (n_neighbors_with_counts==min_neighbors)
//                  return false;
            }
         } // loop over index p
      } // loop over index n
   } // loop over index m

//   cout << "Before returning true from isolated_voxel()" << endl;
   return true;
}

// ---------------------------------------------------------------------
// Method delete_small_[large]count_voxels() inspects the numbers of
// counts within each entry in *voxels_map_ptr.  It removes any
// entry whose count number is less [greater] than
// min_voxel_counts [max_voxel_counts].

void VolumetricCoincidenceProcessor::delete_small_count_voxels(
   int min_voxel_counts)
{
//   cout << "Number of keys in table before thresholding = " 
//        << voxels_map_ptr->size() << endl;

   for (VOXEL_MAP::iterator iter=voxels_map_ptr->begin(); 
        iter != voxels_map_ptr->end(); iter++)
   {
      int curr_counts=iter->second.first;
      if (curr_counts < min_voxel_counts)
      {
         voxels_map_ptr->erase(iter);
      }
   } // loop over voxels map iterator

//   cout << "Number of keys in table after thresholding = " 
//        << voxels_map_ptr->size() << endl;
//   cout << "Number of hot voxels = " << n_hot_voxels << endl;
}

void VolumetricCoincidenceProcessor::delete_large_count_voxels(
   int max_voxel_counts)
{
   for (VOXEL_MAP::iterator iter=voxels_map_ptr->begin(); 
        iter != voxels_map_ptr->end(); iter++)
   {
      int curr_counts=iter->second.first;
      if (curr_counts < max_voxel_counts)
      {
         voxels_map_ptr->erase(iter);
      }
   } // loop over voxels map iterator
}

// ---------------------------------------------------------------------
// Member function bbox_voxel_coords() takes in an (x,y,z) point which
// is assumed to lie within the allowed VCP volume.  It also takes in
// the x,y,z extents of the bounding box which is to surround the
// point.  This method returns the voxel coordinates of the bounding
// box's extremal corners.

void VolumetricCoincidenceProcessor::bbox_voxel_coords(
   const threevector& curr_point,const threevector& extent,
   unsigned int& min_m,unsigned int& min_n,unsigned int& min_p,
   unsigned int& max_m,unsigned int& max_n,unsigned int& max_p)
{
   threevector min_point=curr_point-0.5*extent;
   threevector max_point=curr_point+0.5*extent;
   xyz_to_mnp(min_point,min_m,min_n,min_p);
   xyz_to_mnp(max_point,max_m,max_n,max_p);
}

// ---------------------------------------------------------------------
// Method delete_points_below[above]_zplane() scans through all
// entries within the *voxels_map_ptr and deletes any whose z values
// are less [greater] than z_min [z_max].

void VolumetricCoincidenceProcessor::delete_points_below_zplane(float z_min)
{
   for (VOXEL_MAP::iterator iter=voxels_map_ptr->begin(); 
        iter != voxels_map_ptr->end(); iter++)
   {
      long key=iter->first;
      if (key_to_z(key) < z_min)
      {
         voxels_map_ptr->erase(iter);
      }
   } // loop over voxels map iterator
}

void VolumetricCoincidenceProcessor::delete_points_above_zplane(float z_max)
{
   for (VOXEL_MAP::iterator iter=voxels_map_ptr->begin(); 
        iter != voxels_map_ptr->end(); iter++)
   {
      long key=iter->first;
      if (key_to_z(key) > z_max)
      {
         voxels_map_ptr->erase(iter);
      }
   } // loop over voxels map iterator
}

// ==========================================================================
// Range tail squishing member functions
// ==========================================================================

// Member function squish_range_tails() is a high-level method which
// takes in the range direction vector r_hat.  It transfers counts
// from voxels in *voxels_map_ptr which have too many neighbors with
// nonzero counts located upstream in the -r_hat direction.

void VolumetricCoincidenceProcessor::squish_range_tails(
   double squish_upstream_distance,const threevector& r_hat)
{
//   cout << "inside VCP::squish_range_tails()" << endl;
   
// In order to cut down on execution time, we need to minimize
// upstream_distance as much as possible...

   threevector start_point(-squish_upstream_distance*r_hat);
   threevector stop_point(0,0,0);

//   cout << "Start_point = " << start_point << endl;
//   cout << "stop_point = " << stop_point << endl;

   Linkedlist<voxel_coords>* rel_coords_list_ptr=
      relative_voxel_coords_for_line_segment(start_point,stop_point);
   order_voxels_by_range(r_hat);
   trim_visible_surface_depths(rel_coords_list_ptr);
   
   delete rel_coords_list_ptr;
}

// ---------------------------------------------------------------------
// Member function relative_voxel_coords_for_line_segment takes in the
// starting and stopping points for some line segment.  This method
// returns a dynamically generated linked list containing the voxel
// coordinates for all voxels which lie as close as possible along the
// line segment.

Linkedlist<voxel_coords>* VolumetricCoincidenceProcessor::
relative_voxel_coords_for_line_segment(
   const threevector& start_point,const threevector& stop_point)
{
//   cout << "inside VCP::relative_voxel_coords_for_line_segment()" << endl;
   
   Linkedlist<voxel_coords>* relative_voxel_coords_list_ptr=
      new Linkedlist<voxel_coords>;

   unsigned int m_stop,n_stop,p_stop;
   xyz_to_mnp(stop_point,m_stop,n_stop,p_stop);
   Linkedlist<long>* keylist_ptr=voxels_along_line_segment(
      start_point,stop_point);
//   cout << "keylist_ptr->size() = " << keylist_ptr->size() << endl;

   unsigned int curr_m,curr_n,curr_p;
   for (Mynode<long>* currnode_ptr=keylist_ptr->get_start_ptr();
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      long curr_key=currnode_ptr->get_data();
      key_to_mnp(curr_key,curr_m,curr_n,curr_p);
      relative_voxel_coords_list_ptr->append_node(
         voxel_coords(curr_m-m_stop,curr_n-n_stop,curr_p-p_stop));
//      cout << "curr_m-m_stop = " << curr_m-m_stop
//           << " curr_n-n_stop = " << curr_n-n_stop
//           << " curr_p-p_stop = " << curr_p-p_stop << endl;
   } // loop over nodes in *keylist_ptr
   delete keylist_ptr;

   return relative_voxel_coords_list_ptr;
}

// ---------------------------------------------------------------------
// Member function voxels_along_line_segment returns a dynamically
// generated linked list containing keys for all voxels located along
// the line segment defined by the starting and stop coordinates
// passed as inputs.  This method uses a 3D generalization of the
// midpoint line algorithm described in section 3.2.2. of "Computer
// graphics: principles and practice", 2nd edition by Fley, van Dam,
// Feiner and Hughes.

Linkedlist<long>* VolumetricCoincidenceProcessor::voxels_along_line_segment(
   const threevector& start_point,const threevector& stop_point)
{
//   cout << "inside VCP::voxels_along_line_segment()" << endl;
   
// First instantiate linked list to hold keys for voxels encountered
// along line segment:

   Linkedlist<long>* keylist_ptr=new Linkedlist<long>;

// Next initialize starting voxel:

   unsigned int m_start,m_stop,n_start,n_stop,p_start,p_stop;
   xyz_to_mnp(start_point,m_start,n_start,p_start);
   xyz_to_mnp(stop_point,m_stop,n_stop,p_stop);

//   cout << "m_start = " << m_start << " n_start = " << n_start
//        << " p_start = " << p_start << endl;
//   cout << "m_stop = " << m_stop << " n_stop = " << n_stop
//        << " p_stop = " << p_stop << endl;
   
   unsigned int m=m_start;
   unsigned int n=n_start;
   unsigned int p=p_start;
   long key=mnp_to_key(m,n,p);
   keylist_ptr->append_node(key);

// Sequentially progress from first to last voxel:

   int dx=m_stop-m_start;
   int dy=n_stop-n_start;
   int dz=p_stop-p_start;
   int abs_dx=abs(dx);
   int abs_dy=abs(dy);
   int abs_dz=abs(dz);
   int sgn_dx=sgn(dx);
   int sgn_dy=sgn(dy);
   int sgn_dz=sgn(dz);

   if (abs_dx > abs_dy && abs_dx > abs_dz) // x step dominates
   {
      int dxy=2*abs_dy-abs_dx;
      int increE=2*abs_dy;
      int increNE=2*(abs_dy-abs_dx);

      int dxz=abs_dx-2*abs_dz;
      int increN=-2*abs_dz;
      int increEN=2*(abs_dx-abs_dz);
      
      while (sgn_dx*m < sgn_dx*m_stop)
      {
         m += sgn_dx;
         if (dxy <= 0)
         {
            dxy += increE;	// midpoint below line
         }
         else
         {
            dxy += increNE;	// midpoint above line
            n += sgn_dy;
         }

         if (dxz >= 0)
         {
            dxz += increN;
         }
         else
         {
            dxz += increEN;
            p += sgn_dz;
         }

         key=mnp_to_key(m,n,p);
         keylist_ptr->append_node(key);
      } // while loop over m index 
   }
   else if (abs_dy > abs_dx && abs_dy > abs_dz) // y step dominates
   {
      int dxy=abs_dy-2*abs_dx;
      int increN=-2*abs_dx;
      int increNE=2*(abs_dy-abs_dx);

      int dyz=2*abs_dz-abs_dy;
      int increE=2*abs_dz;
      int increEN=2*(abs_dz-abs_dy);

      while (sgn_dy*n < sgn_dy*n_stop)
      {
         n += sgn_dy;
         if (dxy >= 0)
         {
            dxy += increN;
         }
         else
         {
            dxy += increNE;
            m += sgn_dx;
         }

         if (dyz <= 0)
         {
            dyz += increE;	// midpoint below line
         }
         else
         {
            dyz += increEN;	// midpoint above line
            p += sgn_dz;
         }

         key=mnp_to_key(m,n,p);
         keylist_ptr->append_node(key);
      } // while loop over n index
   }
   else	 // z step dominates
   {
      int dzx=2*abs_dx-abs_dz;
      int increE=2*abs_dx;
      int increNE=2*(abs_dx-abs_dz);

      int dzy=abs_dz-2*abs_dy;
      int increN=-2*abs_dy;
      int increEN=2*(abs_dz-abs_dy);
      
      while (sgn_dz*p < sgn_dz*p_stop)
      {
         p += sgn_dz;
         if (dzx <= 0)
         {
            dzx += increE;	// midpoint below line
         }
         else
         {
            dzx += increNE;	// midpoint above line
            m += sgn_dx;
         }

         if (dzy >= 0)
         {
            dzy += increN;
         }
         else
         {
            dzy += increEN;
            n += sgn_dy;
         }

         key=mnp_to_key(m,n,p);
         keylist_ptr->append_node(key);
      } // while loop over n index
   } // largest step in x, y or z direction conditional
   return keylist_ptr;
}

// ---------------------------------------------------------------------
// Member function order_voxels_by_range takes in a range direction
// vector r_hat.  Looping over all entries within *voxels_map_ptr, this
// method fills member STL vector *ordered_voxels_ptr with <range,key>
// pairs.  It subquently reverse sorts the STL vector in range.
// (Voxels with maximum range appears first position of the STL
// vector.)

void VolumetricCoincidenceProcessor::order_voxels_by_range(
   const threevector& r_hat)
{
   pair<float,long> p;
   ordered_voxels_ptr->clear();

   for (VOXEL_MAP::iterator iter=voxels_map_ptr->begin(); 
        iter != voxels_map_ptr->end(); iter++)
   {
      long key=iter->first;
      p.first=key_to_xyz(key).dot(r_hat);
      p.second=key;
      ordered_voxels_ptr->push_back(p);
   } // loop over voxels map iterator
   std::sort(ordered_voxels_ptr->rbegin(),ordered_voxels_ptr->rend());

/*
   for (unsigned int i=0; i<10; i++)
   {
      cout << "i = " << i << " range = " << (*ordered_voxels_ptr)[i].first
           << " key = " << (*ordered_voxels_ptr)[i].second << endl;
   }
   int size=ordered_voxels_ptr->size();
   for (unsigned int i=size-10; i<size; i++)
   {
      cout << "i = " << i << " range = " << (*ordered_voxels_ptr)[i].first
           << " key = " << (*ordered_voxels_ptr)[i].second << endl;
   }
   char junkchar;
   cout << "Enter any char to continue:" << endl;
   cin >> junkchar;
*/
}

// ---------------------------------------------------------------------
// Member function trim_visible_surface_depths scans over every entry
// within *voxels_map_ptr.  For a given voxel in the
// map, this method searches over voxels located upstream by the
// voxel coordinate displacements within the nodes of the relative
// coordinates linked list.  The related voxels are oriented in the
// negative range direction with respect to the starting voxel.  This
// member function counts the number of upstream illuminated voxels
// containing positive numbers of counts.  If the number exceeds
// jigparam::visible_surface_depth (measured in numbers of voxels),
// the starting voxel's counts are added (with some amplification
// factor) to the threshold upstream voxel's.  The starting voxel is
// subsequently deleted from *voxels_map_ptr.

void VolumetricCoincidenceProcessor::trim_visible_surface_depths(
   Linkedlist<voxel_coords>* rel_coords_list_ptr)
{
   cout << "inside VCP::trim_visible_surface_depths()" << endl;
   cout << "ordered_voxels_ptr->size() = " << ordered_voxels_ptr->size()
        << endl;

   for (unsigned int n=0; n<ordered_voxels_ptr->size(); n++)
   {
      long curr_key=(*ordered_voxels_ptr)[n].second;

      if (curr_key < 0) 
      {
         cout << "n = " << n << " curr_key = " << curr_key << endl;
         outputfunc::enter_continue_char();
      }
      
      voxel_coords curr_voxel=key_to_mnp(curr_key);

      VOXEL_MAP::iterator curr_voxel_iter=voxels_map_ptr->find(curr_key);
//       int curr_counts=curr_voxel_iter->second.first;

      int n_illuminated_upstream_voxels=0;
      for (Mynode<voxel_coords>* coord_node_ptr=
              rel_coords_list_ptr->get_stop_ptr();
           coord_node_ptr != rel_coords_list_ptr->get_start_ptr();
           coord_node_ptr=coord_node_ptr->get_prevptr())                 
      {
         long rel_key=mnp_to_key(curr_voxel+coord_node_ptr->get_data());

         VOXEL_MAP::iterator rel_voxel_iter=voxels_map_ptr->find(rel_key);
         if (rel_voxel_iter==voxels_map_ptr->end()) continue;
//         int rel_counts=rel_voxel_iter->second.first;

         n_illuminated_upstream_voxels++;

// If number of upstream illuminated voxels is too large, increase
// voxel counts in the upstream direction within *voxels_map_ptr.

         const int visible_surface_depth=2;

         if (n_illuminated_upstream_voxels > visible_surface_depth)
         {
            const double amplify_factor=1;
//            const double amplify_factor=2;
//               const double amplify_factor=10;

//            cout << "curr counts = " << curr_voxel_iter->second.first
//                 << " rel counts = " << rel_voxels_iter->second.first << endl;
            
            rel_voxel_iter->second.first += 
               amplify_factor*curr_voxel_iter->second.first;

// As of Mon, Nov 28, 2011 at 4:11 pm, we are storing pure noise probs
// within the voxel_iter->second.second.  Multiply rel voxel's pure
// noise prob by curr voxel's pure noise prob to obtain prob that
// squished voxel's counts correspond to pure noise:

            rel_voxel_iter->second.second *= 
               curr_voxel_iter->second.second;

// After squishing counts information forward in the upstream
// direction, delete current downstream voxel from *voxels_map_ptr:

            voxels_map_ptr->erase(curr_voxel_iter);
            break;
         }

      } // loop over nodes in *rel_coords_list_ptr 
   } // loop over index n labeling ordered voxels
}

// ==========================================================================
// Illumination counts and probabilities member functions
// ==========================================================================

// Member function renormalize_counts_into_probs() first iterates over
// all non-empty voxels and finds the maximum number of voxel counts.
// It converts each voxel's counts into a "probability" ranging
// from 0 to 1 by dividing by the maximum voxel count number.

void VolumetricCoincidenceProcessor::renormalize_counts_into_probs()
{
   cout << "inside VCP::renormalize_counts_into_probs()" << endl;

   int n_nonempty_voxels=0;
   int max_voxel_counts=0;
   
   for (VOXEL_MAP::iterator iter=voxels_map_ptr->begin(); 
        iter != voxels_map_ptr->end(); iter++)
   {
      int curr_counts=iter->second.first;
      if (curr_counts > 0) 
      {
         n_nonempty_voxels++;
         max_voxel_counts=basic_math::max(max_voxel_counts,curr_counts);
      }
   }

   cout << "# nonempty voxels = " << n_nonempty_voxels << endl;
   cout << "max_voxels_counts = " << max_voxel_counts << endl;

   for (VOXEL_MAP::iterator iter=voxels_map_ptr->begin(); 
        iter != voxels_map_ptr->end(); iter++)
   {
      int curr_counts=iter->second.first;
      double curr_prob=curr_counts/double(max_voxel_counts);
//      curr_prob=basic_math::min(1.0,curr_prob);
      iter->second.second=curr_prob;
//      cout << "iter->second.second = " << iter->second.second << endl;
   } // loop over voxels map iterator
}

// ---------------------------------------------------------------------
// Member function copy_counts_onto_probs() simply copies the
// integer counts stored within voxel onto its double probs entry.

void VolumetricCoincidenceProcessor::copy_counts_onto_probs()
{
   cout << "inside VCP::copy_counts_onto_probs()" << endl;

   int n_nonempty_voxels=0;
   int voxel_count_sum=0;
   int max_voxel_counts=0;
   int n_1_counts=0;
   int n_2_counts=0;
   int n_3_counts=0;
   int n_4_counts=0;
   int n_5_counts=0;
   int n_6_counts=0;
   int n_7_counts=0;
   int n_8_counts=0;
   int n_9_counts=0;
   
   for (VOXEL_MAP::iterator iter=voxels_map_ptr->begin(); 
        iter != voxels_map_ptr->end(); iter++)
   {
      int curr_counts=iter->second.first;

      if (curr_counts > 0) 
      {
         voxel_count_sum += curr_counts;
         n_nonempty_voxels++;
      }
      max_voxel_counts=basic_math::max(max_voxel_counts,curr_counts);

      if (curr_counts==1)
      {
         n_1_counts++;
      }
      else if (curr_counts==2)
      {
         n_2_counts++;
      }
      else if (curr_counts==3)
      {
         n_3_counts++;
      }
      else if (curr_counts==4)
      {
         n_4_counts++;
      }
      else if (curr_counts==5)
      {
         n_5_counts++;
      }
      else if (curr_counts==6)
      {
         n_6_counts++;
      }
      else if (curr_counts==7)
      {
         n_7_counts++;
      }
      else if (curr_counts==8)
      {
         n_8_counts++;
      }
      else if (curr_counts==9)
      {
         n_9_counts++;
      }

      iter->second.second=curr_counts;
//      cout << "iter->second.second = " << iter->second.second << endl;
      
   } // loop over voxels map iterator

   cout << "Voxel count sum = " << voxel_count_sum << endl;
   cout << "# nonempty voxels = " << n_nonempty_voxels << endl;

//   double avg_counts_per_voxel = double(voxel_count_sum)/
//      double(n_nonempty_voxels);

//   cout << "avg_voxel_counts = " << avg_counts_per_voxel << endl;
//   cout << "max_voxel_counts = " << max_voxel_counts << endl;
//   cout << "n_1_counts = " << n_1_counts << endl;
//   cout << "n_2_counts = " << n_2_counts << endl;
//   cout << "n_3_counts = " << n_3_counts << endl;
//   cout << "n_4_counts = " << n_4_counts << endl;
//   cout << "n_5_counts = " << n_5_counts << endl;
//   cout << "n_6_counts = " << n_6_counts << endl;
//   cout << "n_7_counts = " << n_7_counts << endl;
//   cout << "n_8_counts = " << n_8_counts << endl;
//   cout << "n_9_counts = " << n_9_counts << endl;
}

// ---------------------------------------------------------------------
// Member function find_signal_reflectivity_threshold() takes in an
// illumination pattern calculated within a constant Z plane.
// Iterating over all voxels and assuming exponential illumination
// attenuation as a function of Z, it converts counts into
// reflectivity values.  If input bool flag
// enforce_prob_limits_flag==true, all reflectivity
// values are constrained to lie within the integral [0,1].
// minimum_signal_reflectivity next fits an exponential curve to the
// reflectivity probability distribution's tail.  Coming downwards in
// the reflectivity distribution, it finds the threshold
// where reflectivity exceeds exponential fit by max_ratio=2.  This
// threshold represents a reasonable estimate for the reflectivity
// value below which noise dominates genuine signal.

double VolumetricCoincidenceProcessor::find_signal_reflectivity_threshold(
   twoDarray* illumpattern_twoDarray_ptr,int min_illum_counts,
   double e_folding_distance,bool enforce_prob_limits_flag)
{
   cout << "inside VCP::compute_detection_probabilities()" << endl;

// Iterate over all voxels and extract their counts.  If counts are
// less than input min_illum_counts, set voxel's corresponding
// reflectivity=0. Otherwise, compute number of expected illuminations
// of the voxel taking exponential illumination attenuation into
// account.  Set reflectivity equal to ratio of number of counts over
// number of times voxel was illuminated.

   vector<double>* P_ptr=new vector<double>;
   for (VOXEL_MAP::iterator iter=voxels_map_ptr->begin(); 
        iter != voxels_map_ptr->end(); iter++)
   {
      double numer=iter->second.first;

      long key=iter->first;
      double curr_x=key_to_x(key);
      double curr_y=key_to_y(key);

      double denom=illumpattern_twoDarray_ptr->fast_XY_to_Z(curr_x,curr_y);
      if (denom < min_illum_counts)
      {
         iter->second.second=0;
      }
      else
      {
         double curr_z=key_to_z(key);
         double illum_attenuation_factor=exp(-(zhi-curr_z)/e_folding_distance);
         denom *= illum_attenuation_factor;
         double curr_p=numer/denom;
         
         if (enforce_prob_limits_flag)
         {
            curr_p=basic_math::max(0.0,curr_p);
            curr_p=basic_math::min(1.0,curr_p);
         }
         iter->second.second=curr_p;
         P_ptr->push_back(curr_p);
      }

//      cout << "numer = " << numer << " denom = " << denom << " p_det = "
//           << iter->second.second << endl;
   } // loop over voxels map iterator

// Generate reflectivity probability distribution:

   double xplot_min=0.02;
   double xplot_max=0.40;
   prob_distribution P_prob(*P_ptr,10000);

   int n_xplot_min=P_prob.get_bin_number(xplot_min);
   double p_xplot_min=P_prob.get_p(n_xplot_min);
   double ymax=1.5*p_xplot_min;

   P_prob.set_xmin(xplot_min);
   P_prob.set_xmax(xplot_max);
   P_prob.set_xtic(0.04);
   P_prob.set_ymax(1.5*p_xplot_min);
   P_prob.set_ytic(0.2*ymax);
   P_prob.writeprobdists(false);
   delete P_ptr;

// We assume genuine signal's reflectivity probability distribution
// obeys exponential form

// 			y = A exp(-B p)

// for 0.2 <= p <= 0.3

// 		--->   ln y = ln A - B p

// Perform linear fit for lnA and B coefficients:

   double plo=0.2;
   double phi=0.3;
   unsigned int nlo=P_prob.get_bin_number(plo);
   unsigned int nhi=P_prob.get_bin_number(phi);
   cout << "nlo = " << nlo << " nhi = " << nhi << endl;

   vector<double> X,logY,Y;
   for (unsigned int n=nlo; n<=nhi; n++)
   {
      double curr_y=P_prob.get_p(n);

// If y=0, don't attempt to compute log(y) !

      if (fabs(curr_y) < 1.0E-15) continue;

      logY.push_back(log(curr_y));

      double curr_p=P_prob.get_x(n);
      X.push_back(curr_p);

//      cout << "n = " << n << " p = " << curr_p << " y = " << curr_y << endl;
   } // loop over index n

   mypolynomial poly(1);
   double chisq;

   poly.fit_coeffs_using_residuals(X,logY,chisq);
//   cout << "poly w residuals = " << poly << endl;
   double A=exp(poly.get_coeff(0));
   double B=poly.get_coeff(1);
   cout << "A = " << A << " B = " << B << endl;

// Coming downwards in the reflectivity distribution, find threshold
// where reflectivity exceeds exponential fit by max_ratio:

   X.clear();
   int nbins=100;
   double dx=(xplot_max-xplot_min)/nbins;
   double x_threshold=-1;
   for (unsigned int n=nbins; n>0; n--)
   {
      double curr_x=xplot_min+n*dx;
      int curr_nbin=P_prob.get_bin_number(curr_x);
      double curr_y=P_prob.get_p(curr_nbin);
      double fitted_y=A*exp(B*curr_x);
      double ratio=curr_y/fitted_y;

      const double max_ratio=2.0;
      if (ratio < max_ratio) x_threshold=curr_x;

//      cout << "x = " << curr_x 
//           << " y = " << curr_y
//           << " fitted_y = " << fitted_y
//           << " ratio = " << ratio << endl;

      X.push_back(curr_x);
      Y.push_back(fitted_y);
   }
   
   cout << "x_threshold = " << x_threshold << endl;
   vector<double> Xthreshold,Ythreshold;
   Xthreshold.push_back(x_threshold);
   Xthreshold.push_back(x_threshold);
   Ythreshold.push_back(0);
   Ythreshold.push_back(1000);

// Append fitted exponential curve as well as reflectivity threshold
// to metafile plot:
   
   metafile M;
   string meta_filename="prob_density";
   M.set_filename(meta_filename);
   M.appendmetafile();
   M.write_curve(X,Y);
   M.write_curve(Xthreshold,Ythreshold,colorfunc::green);
   M.closemetafile();
   string unix_cmd="meta_to_jpeg "+meta_filename;
   sysfunc::unix_command(unix_cmd);

   string prob_dist_jpg_filename="prob_density.jpg";
   string prob_Pdist_jpg_filename="prob_P_density.jpg";
   unix_cmd="mv "+prob_dist_jpg_filename+" "+prob_Pdist_jpg_filename;
   sysfunc::unix_command(unix_cmd);

   string banner="Wrote reflectivity distribution to prob_P_density.jpg";
   outputfunc::write_big_banner(banner);

   cout << "Finished computing detection probabilities" << endl;

   return x_threshold;
}

// ==========================================================================
// Voxelized data export member functions
// ==========================================================================

// Method retrieve_XYZ_points() fills input STL vector with points
// transfered from *voxels_map_ptr.

void VolumetricCoincidenceProcessor::retrieve_XYZ_points(
   vector<threevector>& XYZ_points)
{
   for (VOXEL_MAP::iterator iter=voxels_map_ptr->begin(); 
        iter != voxels_map_ptr->end(); iter++)
   {
      int curr_counts=iter->second.first;
      if (curr_counts > 0)
      {
         long key=iter->first;
         XYZ_points.push_back(key_to_xyz(key));
      }
   } // loop over index voxels map iterator
}

// ---------------------------------------------------------------------
// Method retrieve_XYZP_points() fills input STL vectors with point
// coordinate values transfered from *voxels_map_ptr.

void VolumetricCoincidenceProcessor::retrieve_XYZP_points(
   vector<double>* X_ptr,vector<double>* Y_ptr,vector<double>* Z_ptr,
   vector<double>* P_ptr,double min_prob_threshold,bool perturb_voxels_flag)
{
//   cout << "inside VCP::retrieve_XYZP_points()" << endl;
//   cout << "min_prob_threshold = " << min_prob_threshold << endl;
//   cout << "perturb_voxels_flag = " << perturb_voxels_flag << endl;
//   cout << "dx = " << dx << " dy = " << dy << " dz = " << dz << endl;

   for (VOXEL_MAP::iterator iter=voxels_map_ptr->begin(); 
        iter != voxels_map_ptr->end(); iter++)
   {
      double curr_prob=iter->second.second;

      if (curr_prob < min_prob_threshold) continue;

/*
      if (curr_prob > 1)
      {
         cout << "Warning in VCP::retrieve_XYZP_points(), p = " << curr_prob
              << endl;
      }
*/
  
      long key=iter->first;
      double curr_X=key_to_x(key);
      double curr_Y=key_to_y(key);
      double curr_Z=key_to_z(key);

      if (perturb_voxels_flag)
      {
         double delta_x=0.5*(2*nrfunc::ran1()-1)*dx;
         double delta_y=0.5*(2*nrfunc::ran1()-1)*dy;
         double delta_z=0.5*(2*nrfunc::ran1()-1)*dz;
         curr_X += delta_x;
         curr_Y += delta_y;
         curr_Z += delta_z;
      }

      X_ptr->push_back(curr_X);
      Y_ptr->push_back(curr_Y);
      Z_ptr->push_back(curr_Z);
      P_ptr->push_back(curr_prob);

//      cout.precision(10);
//      cout << "curr_prob = " << curr_prob << endl;
      
   } // loop over voxels map iterator
}

// ==========================================================================
// Pure noise characterization member functions
// ==========================================================================

// Member function extract_pure_noise_counts()

vector<int> VolumetricCoincidenceProcessor::extract_pure_noise_counts(
   const threevector& A_hat,const threevector& B_hat,
   const threevector& origin,
   double Amin,double Amax,double Bmin,double Bmax,double Zmin,double Zmax)
{
   cout << "inside VCP::extract_pure_noise_counts()" << endl;

   cout << "xlo = " << get_xlo() << " xhi = " << get_xhi() << endl;
   cout << "ylo = " << get_ylo() << " yhi = " << get_yhi() << endl;
   cout << "zlo = " << get_zlo() << " zhi = " << get_zhi() << endl;

   cout << "origin = " << origin << endl;
   cout << "Amin = " << Amin << " Amax = " << Amax << endl;
   cout << "Bmin = " << Bmin << " Bmax = " << Bmax << endl;
   cout << "Zmin = " << Zmin << " Zmax = " << Zmax << endl;

   vector<int> noise_counts;
   noise_counts.reserve(mdim*ndim*pdim);
   
   double delta_A=Amax-Amin;
   double delta_B=Bmax-Bmin;
   cout << "dA = " << delta_A << " dB = " << delta_B << endl;
   cout << "initially, noise_vcp_ptr->size() = " << size() << endl;
   
   double ADotOrigin=A_hat.dot(origin);
   double BDotOrigin=B_hat.dot(origin);
   
   for (unsigned int m=0; m<mdim; m++)
   {
      outputfunc::update_progress_fraction(m,100,mdim);
      for (unsigned int n=0; n<ndim; n++)
      {
         for (unsigned int p=0; p<pdim; p++)
         {
            VOXEL_MAP::iterator voxel_iter=mnp_to_voxel_iterator(m,n,p);
            threevector XYZ=mnp_to_xyz(m,n,p);

//            cout << "m = " << m << " n = " << n << " p = " << p
//                 << " x = " << XYZ.get(0)
//                 << " y = " << XYZ.get(1)
//                 << " z = " << XYZ.get(2) << endl;

// Ignore any voxels lying within the "Signal" slice Zmin < Z < Zmax:

            double curr_Z=XYZ.get(2);
            if (curr_Z > Zmin && curr_Z < Zmax) 
            {
               if (voxel_iter != voxels_map_ptr->end()) 
               {
//                  cout << "Voxel lies in signal ground Z interval" << endl;
                  voxels_map_ptr->erase(voxel_iter);               
//                  cout << "noise VCP size = " << size() << endl;
               }
               continue;
            }

// Convert from XYZ to ABN coordinates.  Ignore any voxel which does
// not lie relatively near center of ABN bounding box:

            double curr_A=XYZ.dot(A_hat)-ADotOrigin;
//            cout << "curr_A = " << curr_A << endl;
            if (curr_A < Amin+0.25*delta_A || curr_A > Amax-0.25*delta_A) 
            {
               if (voxel_iter != voxels_map_ptr->end()) 
               {
//                  cout << "Voxel A value not close enough to center" << endl;
                  voxels_map_ptr->erase(voxel_iter);               
//                  cout << "noise VCP size = " << size() << endl;
               }
               continue;
            }

            double curr_B=XYZ.dot(B_hat)-BDotOrigin;
//            cout << "curr_B = " << curr_B << endl;
            if (curr_B < Bmin+0.25*delta_B || curr_B > Bmax-0.25*delta_B) 
            {
               if (voxel_iter != voxels_map_ptr->end()) 
               {
//                  cout << "Voxel B value not close enough to center" << endl;
                  voxels_map_ptr->erase(voxel_iter);               
//                  cout << "noise VCP size = " << size() << endl;
               }
               continue;
            }

            if (voxel_iter==voxels_map_ptr->end()) 
            {
               noise_counts.push_back(0);
               double x=m_to_x(m);
               double y=n_to_y(n);
               double z=p_to_z(p);
               accumulate_points_and_probs(x,y,z,0.3);
            }
            else 
            {
               noise_counts.push_back(voxel_iter->second.first);
               voxel_iter->second.second=0.7;
            }
         } // loop over p index
      } // loop over n index
   } // loop over m index
   
   cout << endl;
   cout << "noise_counts.size() = " << noise_counts.size() << endl;
   cout << "Finally, noise_vcp_ptr->size() = " << size() << endl;
   
   return noise_counts;
}

// ---------------------------------------------------------------------
// Member function assign_pure_noise_probabilities() iterates over
// every voxel within the VCP which has non-zero number of counts N.
// It sets such voxels' probabilities equal to pure_noise_probs[N].

void VolumetricCoincidenceProcessor::assign_pure_noise_probabilities(
   const vector<double>& pure_noise_probs)
{
   cout << "inside VCP::assign_pure_noise_probabilities()" << endl;

   int max_noise_counts=pure_noise_probs.size();
   for (VOXEL_MAP::iterator iter=voxels_map_ptr->begin(); 
        iter != voxels_map_ptr->end(); iter++)
   {
      int n_counts=iter->second.first;

      double p_pure_noise=0;
      if (n_counts < max_noise_counts)
      {
         p_pure_noise=pure_noise_probs[n_counts];
      }
      iter->second.second=p_pure_noise;
   } // loop over voxels map iterator
}
