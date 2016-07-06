// ==========================================================================
// Fishnet class member function definitions
// ==========================================================================
// Last updated on 12/8/11; 12/10/11; 12/11/11; 4/5/14
// ==========================================================================

#include "image/compositefuncs.h"
#include "general/filefuncs.h"
#include "osg/osg3D/Fishnet.h"
#include "general/outputfuncs.h"
#include "passes/Pass.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "math/prob_distribution.h"
#include "general/sysfuncs.h"
#include "osg/osg3D/tdpfuncs.h"

using std::cout;
using std::endl;
using std::flush;
using std::ostream;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void Fishnet::allocate_member_objects()
{
}		       

void Fishnet::initialize_member_objects()
{
   Emin=POSITIVEINFINITY;
   linewidth=1;

   weights_term_coeff=1.0;
   springs_term_coeff=10.0;	    	// MPSCP T1
//   springs_term_coeff=30.0;	    	// MPSCP Pender
//   springs_term_coeff=0.3;		// Peter's coincidence processing
//   pressure_term_coeff=3.0;		// MPSCP Pender
   pressure_term_coeff=30.0;		// MPSCP T1
//    pressure_term_coeff=100.0;	// Peter's coincidence processing

   ztwoDarray_ptr=NULL;
   dz_twoDarray_ptr=NULL;
   ground_twoDarray_ptr=NULL;
   pressure_twoDarray_ptr=NULL;
   VCP_ptr=NULL;

   PointsGroup_ptr->set_crosshairs_size(1);
}		       

Fishnet::Fishnet(Pass* pass_ptr,int ID,bool fall_down_flag):
   Geometrical(3,ID)
{	
   fall_downwards_flag=fall_down_flag;
   PointsGroup_ptr=new osgGeometry::PointsGroup(3,pass_ptr);
   PolyLinesGroup_ptr=new PolyLinesGroup(3,pass_ptr);

   allocate_member_objects();
   initialize_member_objects();
}		       

Fishnet::~Fishnet()
{
   delete PointsGroup_ptr;
   delete PolyLinesGroup_ptr;

   delete ztwoDarray_ptr;
   delete dz_twoDarray_ptr;
   delete ground_twoDarray_ptr;
   delete pressure_twoDarray_ptr;
}

void Fishnet::init_coord_system(
   double dx,double dy,double min_x,double max_x,double min_y,double max_y,
   double init_z)
{
//   cout << "inside Fishnet:init_coord_system()" << endl;
//   cout << "init_z = " << init_z << endl;
   
   max_x += dx;
   max_y += dy;

   int mdim=(max_x-min_x)/dx+1;
   int ndim=(max_y-min_y)/dy+1;

   ztwoDarray_ptr=new twoDarray(mdim,ndim);
   ztwoDarray_ptr->init_coord_system(min_x,max_x,min_y,max_y);
   ztwoDarray_ptr->initialize_values(init_z);

   dz_twoDarray_ptr=new twoDarray(mdim,ndim);
   dz_twoDarray_ptr->init_coord_system(min_x,max_x,min_y,max_y);
   dz_twoDarray_ptr->initialize_values(1);	// meter

   ground_twoDarray_ptr=new twoDarray(mdim,ndim);
   ground_twoDarray_ptr->init_coord_system(min_x,max_x,min_y,max_y);
   ground_twoDarray_ptr->initialize_values(NEGATIVEINFINITY);	

   pressure_twoDarray_ptr=new twoDarray(
      VCP_ptr->get_mdim(),VCP_ptr->get_ndim());
   pressure_twoDarray_ptr->init_coord_system(
      VCP_ptr->get_xlo(),VCP_ptr->get_xhi(),
      VCP_ptr->get_ylo(),VCP_ptr->get_yhi());
   pressure_twoDarray_ptr->initialize_values(-1);  // zero pressure flag

//   cout << "*ztwoDarray_ptr = " << *ztwoDarray_ptr << endl;
}

// ---------------------------------------------------------------------
// Member function refine_coord_system() multiplies the current
// ztwoDarray's mdim and ndim by 2.  It then instantiates a new
// ztwoDarray with the same xlo,xhi,ylo,yhi extrema but new mdim and
// ndim.  Resampled values within *ztwoDarray offset but input
// z_offset and loaded into the new, higher density twoDarray.
// ztwoDarray is redefined as the new twoDarray, and dz_twoDarray_ptr
// and ground_twoDarray_ptr are regenerated.

double Fishnet::refine_coord_system(double z_offset)
{
   cout << "inside Fishnet:refine_coord_system()" << endl;
   
   unsigned int mdim=ztwoDarray_ptr->get_xdim();
   unsigned int ndim=ztwoDarray_ptr->get_ydim();
   mdim *= 2;
   ndim *= 2;
   
   twoDarray* refined_ztwoDarray_ptr=new twoDarray(mdim,ndim);
   double xlo=ztwoDarray_ptr->get_xlo();
   double xhi=ztwoDarray_ptr->get_xhi();
   double ylo=ztwoDarray_ptr->get_ylo();
   double yhi=ztwoDarray_ptr->get_yhi();

   int percentile_sentinel=1;	// median values
//   int percentile_sentinel=2;	// maximal values
   compositefunc::regrid_twoDarray(
      xhi,xlo,yhi,ylo,ztwoDarray_ptr,refined_ztwoDarray_ptr,
      percentile_sentinel);

// Increase all entries within *refined_ztwoDarray_ptr by some z_offset:

   for (unsigned int px=0; px<mdim; px++)
   {
      for (unsigned int py=0; py<ndim; py++)
      {
         double curr_z=refined_ztwoDarray_ptr->get(px,py);
         refined_ztwoDarray_ptr->put(px,py,curr_z+z_offset);
      } // loop over py index
   } // loop over px index

   delete ztwoDarray_ptr;
   ztwoDarray_ptr=refined_ztwoDarray_ptr;

   delete dz_twoDarray_ptr;
   dz_twoDarray_ptr=new twoDarray(mdim,ndim);
   dz_twoDarray_ptr->init_coord_system(xlo,xhi,ylo,yhi);
   dz_twoDarray_ptr->initialize_values(1);	// meter

   delete ground_twoDarray_ptr;
   ground_twoDarray_ptr=new twoDarray(mdim,ndim);
   ground_twoDarray_ptr->init_coord_system(xlo,xhi,ylo,yhi);
   ground_twoDarray_ptr->initialize_values(NEGATIVEINFINITY);	

//   cout << "*ztwoDarray_ptr = " << *ztwoDarray_ptr << endl;

   return ztwoDarray_ptr->get_deltax();
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const Fishnet& f)
{
//   outstream << "inside Fishnet::operator<<" << endl;
   outstream << static_cast<const Geometrical&>(f) << endl;
   if (f.ztwoDarray_ptr != NULL)
   {
      cout << "*ztwoDarray_ptr = " << *f.ztwoDarray_ptr << endl;
   }

   return outstream;
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

// ==========================================================================
// Scenegraph member functions
// ==========================================================================

// Member function generate_drawable_geode instantiates an osg::Geode
// containing a single Fishnet drawable.

osg::Geode* Fishnet::generate_drawable_geode()
{
//   cout << "inside Fishnet::generate_drawable_geode()" << endl;
   
   geode_refptr = new osg::Geode();
   return geode_refptr.get();
}

void Fishnet::initialize()
{
   get_PAT_ptr()->addChild(PointsGroup_ptr->get_OSGgroup_ptr());
   get_PAT_ptr()->addChild(PolyLinesGroup_ptr->get_OSGgroup_ptr());
}

// ==========================================================================
// Drawing member functions
// ==========================================================================

// Member function regenerate_PolyLines destroys the fishnet's current
// PolyLines and redraws an entire new set.

unsigned int Fishnet::regenerate_PolyLines()
{
//   cout << "inside Fishnet::regenerate_PolyLines()" << endl;
   
   if (ztwoDarray_ptr==NULL) 
   {
      cout << "Error in Fishnet::draw()" << endl;
      cout << "ztwoDarray_ptr = NULL!" << endl;
      return 0;
   }

   PolyLinesGroup_ptr->destroy_all_PolyLines();
   colorfunc::Color PolyLine_color=colorfunc::purple;

   double origin_X,origin_Y,origin_Z;
   ztwoDarray_ptr->fast_pixel_to_XYZ(0,0,origin_X,origin_Y,origin_Z);
   threevector origin(origin_X,origin_Y,origin_Z);

   for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
   {
      vector<threevector> curr_column;
      for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
      {
         double X,Y,Z;
         ztwoDarray_ptr->fast_pixel_to_XYZ(px,py,X,Y,Z);
         curr_column.push_back(threevector(X,Y,Z));
      }
      PolyLine* curr_PolyLine_ptr=PolyLinesGroup_ptr->generate_new_PolyLine(
         origin,curr_column,colorfunc::get_OSG_color(PolyLine_color));
      curr_PolyLine_ptr->set_linewidth(linewidth);
   }

   for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
   {
      vector<threevector> curr_row;
      for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
      {
         double X,Y,Z;
         ztwoDarray_ptr->fast_pixel_to_XYZ(px,py,X,Y,Z);
         curr_row.push_back(threevector(X,Y,Z));
      }
      PolyLine* curr_PolyLine_ptr=PolyLinesGroup_ptr->generate_new_PolyLine(
         curr_row,colorfunc::get_OSG_color(PolyLine_color));
      curr_PolyLine_ptr->set_linewidth(linewidth);
   }

   unsigned int n_PolyLines=PolyLinesGroup_ptr->get_n_Graphicals();
//   cout << "Total number of polylines generated = " 
//        << n_PolyLines << endl;
   return n_PolyLines;
}

// ---------------------------------------------------------------------
void Fishnet::toggle_PolyLines()
{
   PolyLinesGroup_ptr->toggle_OSGgroup_nodemask();
}

// ==========================================================================
// Ground surface modeling member functions
// ==========================================================================

// Member function generate_pressure_mask() loops over the m and n
// dimensions within member *VCP_ptr.  For each m,n cell, if any voxel
// in the p direction is found to be nonempty,
// *pressure_twoDarray_ptr(m,n) is set equal to unity. 

void Fishnet::generate_pressure_mask()
{
   cout << "inside Fishnet::generate_pressure_mask()" << endl;

   if (VCP_ptr==NULL)
   {
      cout << "Error in Fishnet::generate_pressure_mask()" << endl;
      cout << "VCP_ptr = NULL !!!" << endl;
      return;
   }

   int n_cells_w_pressure=0;
   for (unsigned int m=0; m<VCP_ptr->get_mdim(); m++)
   {
      for (unsigned int n=0; n<VCP_ptr->get_ndim(); n++)
      {
         double pressure=VCP_ptr->get_cumulative_prob_integral(
            m,n,0,VCP_ptr->get_pdim()-1,1);
         if (pressure > 0)
         {
            pressure_twoDarray_ptr->put(m,n,1);  // nonzero pressure flag
            n_cells_w_pressure++;
         }
      } // loop over VCP index n 
   } // loop over VCP index m

   cout << "Number of cells with pressure = " << n_cells_w_pressure
        << endl;
}

// ---------------------------------------------------------------------
// Member function compute_total_current_energy() calculates gravity,
// spring and pressure term contributions to the fishnet's current
// energy function.  These terms are normalized by the number of
// fishnet masses, springs and pressure points.  So the fishnet's
// energy should be insensitive to the mdim,ndim dimensions of
// *ztwoDarray_ptr.  A weighted sum of the 3 energy terms is returned
// by this method.

double Fishnet::compute_total_current_energy()
{
//   cout << "inside Fishnet::compute_total_current_energy()" << endl;

   if (VCP_ptr==NULL)
   {
      cout << "Error in Fishnet::compute_total_current_energy()" << endl;
      cout << "VCP_ptr = NULL !!!" << endl;
      return -1;
   }

   double z_minimum=VCP_ptr->p_to_z(0);
   double z_maximum=VCP_ptr->p_to_z(VCP_ptr->get_pdim()-1);

   double E_weights=0;
   double E_springs=0;

   int n_masses=0;
   int n_springs=0;
   for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
   {
      for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
      {
         double X,Y,Z;
         ztwoDarray_ptr->fast_pixel_to_XYZ(px,py,X,Y,Z);
         threevector curr_XYZ(X,Y,Z);

// Calculate gravity (weights) term contribution:

         if (fall_downwards_flag)
         {
            E_weights += weights_term_coeff*(Z-z_minimum);
         }
         else
         {
            E_weights += weights_term_coeff*(z_maximum-Z);
         }
         

         n_masses++;

// Calculate springs term contribution:

         vector<threevector> neighbor_XYZ,nominal_XYZ;
         if (px > 0)
         {
            ztwoDarray_ptr->fast_pixel_to_XYZ(px-1,py,X,Y,Z);
            neighbor_XYZ.push_back(threevector(X,Y,Z));
            nominal_XYZ.push_back(threevector(X,Y,curr_XYZ.get(2)));
         }
         if (px < ztwoDarray_ptr->get_mdim()-1)
         {
            ztwoDarray_ptr->fast_pixel_to_XYZ(px+1,py,X,Y,Z);
            neighbor_XYZ.push_back(threevector(X,Y,Z));
            nominal_XYZ.push_back(threevector(X,Y,curr_XYZ.get(2)));
         }
         if (py > 0)
         {
            ztwoDarray_ptr->fast_pixel_to_XYZ(px,py-1,X,Y,Z);
            neighbor_XYZ.push_back(threevector(X,Y,Z));
            nominal_XYZ.push_back(threevector(X,Y,curr_XYZ.get(2)));
         }
         if (py < ztwoDarray_ptr->get_ndim()-1)
         {
            ztwoDarray_ptr->fast_pixel_to_XYZ(px,py+1,X,Y,Z);
            neighbor_XYZ.push_back(threevector(X,Y,Z));
            nominal_XYZ.push_back(threevector(X,Y,curr_XYZ.get(2)));
         }
         
         for (unsigned int n=0; n<neighbor_XYZ.size(); n++)
         {
            double nominal_separation=(nominal_XYZ[n]-curr_XYZ).magnitude();
            double stretched_separation=(neighbor_XYZ[n]-curr_XYZ).magnitude();
            E_springs += 
               springs_term_coeff*sqr(stretched_separation-nominal_separation);
            n_springs++;
         }
         
      } // loop over px index
   } // loop over py index

// Calculate pressure term contribution:

   double E_pressure=0;   

   int n_pressure_points=0;
//   cout << "mdim = " << VCP_ptr->get_mdim()
//        << " ndim = " << VCP_ptr->get_ndim() << endl;
   
   for (unsigned int m=0; m<VCP_ptr->get_mdim(); m++)
   {
//      outputfunc::update_progress_fraction(m,100,VCP_ptr->get_mdim());

      double curr_x=VCP_ptr->m_to_x(m);
      for (unsigned int n=0; n<VCP_ptr->get_ndim(); n++)
      {

// Don't perform expensive pressure computation if mask value for
// (m,n) pixel = -1:

         if (pressure_twoDarray_ptr->get(m,n) < 0) continue;

         double curr_y=VCP_ptr->n_to_y(n);

         double curr_z;
         bool inside_flag=ztwoDarray_ptr->point_to_interpolated_value(
            curr_x,curr_y,curr_z);
         if (!inside_flag) continue;

//         cout << "m = " << m << " n = " << n 
//              << " curr_x = " << curr_x << " curr_y = " << curr_y
//              << endl;

         E_pressure += pressure_term_coeff*
            VCP_ptr->get_cumulative_prob_integral(
               m,n,curr_z,fall_downwards_flag);
         n_pressure_points++; 

      } // loop over n index labeling voxels
   } // loop over m index labeling voxels
//   cout << endl;

//   cout << "E_pressure = " << E_pressure
//        << " n_pressure_points = " << n_pressure_points << endl;

// Renormalize weight, spring and pressure energy terms so that
// they're independent of fishnet scale size:

//   cout << "n_masses = " << n_masses 
//        << " n_springs = " << n_springs
//        << " n_pressure_points = " << n_pressure_points << endl;

   E_weights /= n_masses;
   E_springs /= n_springs;
   E_pressure /= n_pressure_points;

   double E_total=E_weights+E_springs+E_pressure;

//   cout << "Eweights = " << stringfunc::number_to_string(E_weights,2)
//        << " Esprings = " << stringfunc::number_to_string(E_springs,2)
//        << " Epressure = " << stringfunc::number_to_string(E_pressure,2)
//        << " Etotal = " << stringfunc::number_to_string(E_total,3)
//        << endl;

//   outputfunc::enter_continue_char();
   
   return E_total;
}

// ---------------------------------------------------------------------
// Member function perturb_ground_surface() takes in a fishnet mass
// location labeled by input parameters px,py.  It first checks
// whether the candidate fishnet location should be altered.  If so,
// it computes the current local energy associated with px,py.  The
// method then adds or subtracts a local dz value to the px,py mass
// depending upon whether the fishnet is falling down or up.  The
// local energy for the perturbed fishnet mass location is calculated.
// This method returns the difference between the trial new and
// original local energies.

double Fishnet::perturb_ground_surface_trial(
   unsigned int px,unsigned int py,double Zstop)
{
//   cout << "inside Fishnet::perturb_ground_surface()" << endl;

// Don't perturb ground location if
// ground_twoDarray_ptr(px,py) doesn't equal sentinel NEGATIVEINFINITY
// value:

   if (ground_twoDarray_ptr->get(px,py) > 0.5*NEGATIVEINFINITY)
   {
      return NEGATIVEINFINITY;
   }

// Don't bother to perform expensive ground cell perturbation for very
// small values of delta_z:

   double delta_z=dz_twoDarray_ptr->get(px,py);
//   cout << "px = " << px << " py = " << py << " dz = " << delta_z << endl;

   const double min_delta_z=0.05;	// 5 cm
   if (fabs(delta_z) < min_delta_z)
   {
      return NEGATIVEINFINITY;
   }
   
// Don't bother to perform expensive ground cell perturbation if
// current Z is already less than (greater than) Zstop:

   double orig_z=ztwoDarray_ptr->get(px,py);

   if (fall_downwards_flag)
   {
      if (orig_z < Zstop) return NEGATIVEINFINITY;
   }
   else
   {
      if (orig_z > Zstop) return NEGATIVEINFINITY;
   }
   
   double E_original=compute_local_current_energy(px,py);

   double trial_z;
   if (fall_downwards_flag)
   {
      trial_z=orig_z-delta_z;
   }
   else
   {
      trial_z=orig_z+delta_z;
   }
   ztwoDarray_ptr->put(px,py,trial_z);
   
   double E_trial=compute_local_current_energy(px,py);
   
//   cout << "Eorig = " << E_original
//        << " Etrial = " << E_trial << endl;

   double delta_E=E_trial-E_original;
   return delta_E;
}

// ---------------------------------------------------------------------
// Member function compute_local_current_energy() takes in integer
// coordinates px,py for some fishnet mass.  It sums the mass'
// gravity and spring neighbor contributions to the fishnet energy
// function.  It also integrates columnar pressure within an XY region
// bounded by the mass' neighbor XYZs.  The sum of all 3 energy terms
// is returned.

double Fishnet::compute_local_current_energy(unsigned int px,unsigned int py)
{
//   cout << "inside Fishnet::compute_local_current_energy()" << endl;

   double z_minimum=VCP_ptr->p_to_z(0);
   double z_maximum=VCP_ptr->p_to_z(VCP_ptr->get_pdim()-1);

   double X,Y,Z;
   ztwoDarray_ptr->fast_pixel_to_XYZ(px,py,X,Y,Z);
   threevector curr_XYZ(X,Y,Z);

// Calculate weights term contribution:

   double E_weights=0;
   if (fall_downwards_flag)
   {
      E_weights += weights_term_coeff*(Z-z_minimum);
   }
   else
   {
      E_weights += weights_term_coeff*(z_maximum-Z);
   }

// Calculate springs term contribution:

   double E_springs=0;
   vector<threevector> neighbor_XYZ,nominal_XYZ;
   if (px > 0)
   {
      ztwoDarray_ptr->fast_pixel_to_XYZ(px-1,py,X,Y,Z);
      neighbor_XYZ.push_back(threevector(X,Y,Z));
      nominal_XYZ.push_back(threevector(X,Y,curr_XYZ.get(2)));
   }
   if (px < ztwoDarray_ptr->get_mdim()-1)
   {
      ztwoDarray_ptr->fast_pixel_to_XYZ(px+1,py,X,Y,Z);
      neighbor_XYZ.push_back(threevector(X,Y,Z));
      nominal_XYZ.push_back(threevector(X,Y,curr_XYZ.get(2)));
   }
   if (py > 0)
   {
      ztwoDarray_ptr->fast_pixel_to_XYZ(px,py-1,X,Y,Z);
      neighbor_XYZ.push_back(threevector(X,Y,Z));
      nominal_XYZ.push_back(threevector(X,Y,curr_XYZ.get(2)));
   }
   if (py < ztwoDarray_ptr->get_ndim()-1)
   {
      ztwoDarray_ptr->fast_pixel_to_XYZ(px,py+1,X,Y,Z);
      neighbor_XYZ.push_back(threevector(X,Y,Z));
      nominal_XYZ.push_back(threevector(X,Y,curr_XYZ.get(2)));
   }
         
   for (unsigned int i=0; i<neighbor_XYZ.size(); i++)
   {
      double nominal_separation=(nominal_XYZ[i]-curr_XYZ).magnitude();
      double stretched_separation=(neighbor_XYZ[i]-curr_XYZ).magnitude();
      E_springs += 
         springs_term_coeff*sqr(stretched_separation-nominal_separation);
   }

// Compute pressure term integral within XY region bounded by
// neighbor_XYZ's:

   double xlo=1E15;
   double ylo=1E15;
   double xhi=-1E15;
   double yhi=-1E15;
   for (unsigned int i=0; i<neighbor_XYZ.size(); i++)
   {
      xlo=basic_math::min(xlo,neighbor_XYZ[i].get(0));
      ylo=basic_math::min(ylo,neighbor_XYZ[i].get(1));
      xhi=basic_math::max(xhi,neighbor_XYZ[i].get(0));
      yhi=basic_math::max(yhi,neighbor_XYZ[i].get(1));
   }
   unsigned int mlo=VCP_ptr->x_to_m(xlo);
   unsigned int mhi=VCP_ptr->x_to_m(xhi);
   unsigned int nlo=VCP_ptr->y_to_n(ylo);
   unsigned int nhi=VCP_ptr->y_to_n(yhi);

//   cout << "xlo = " << xlo << " xhi = " << xhi << endl;
//   cout << "ylo = " << ylo << " yhi = " << yhi << endl;
//   cout << "mlo = " << mlo << " mhi = " << mhi << endl;
//   cout << "nlo = " << nlo << " nhi = " << nhi << endl;
   
   double E_pressure=0;   

   for (unsigned int m=mlo; m<=mhi; m++)
   {
      double curr_x=VCP_ptr->m_to_x(m);
      for (unsigned int n=nlo; n<=nhi; n++)
      {

// Don't perform expensive pressure computation if mask value for
// (m,n) pixel = -1:

         if (pressure_twoDarray_ptr->get(m,n) < 0) continue;

         double curr_y=VCP_ptr->n_to_y(n);
         double curr_z;
         bool inside_flag=ztwoDarray_ptr->point_to_interpolated_value(
            curr_x,curr_y,curr_z);

         if (!inside_flag) continue;
         
         E_pressure += pressure_term_coeff*
            VCP_ptr->get_cumulative_prob_integral(
               m,n,curr_z,fall_downwards_flag);
      } // loop over n index
   } // loop over m index
   
   double E_local=E_weights+E_springs+E_pressure;

//   cout << "Eweights = " << stringfunc::number_to_string(E_weights,2)
//        << " Esprings = " << stringfunc::number_to_string(E_springs,2)
//        << " Epressure = " << stringfunc::number_to_string(E_pressure,2)
//        << " Elocal = " << stringfunc::number_to_string(E_local,3)
//        << endl;

//   outputfunc::enter_continue_char();
   
   return E_local;
}

// ---------------------------------------------------------------------
// Member function compute_local_ground_pressure_energy() takes in
// integer px,py coordinates for some mass location.  It first
// calculates the lateral distance to the closest fishnet mass
// neighbors.  This method next integrates the column
// pressures within a lateral cell surrounding px,py from zero to the
// *ztwoDarray_ptr(px,py) and *ztwoDarray_ptr(px,py) +/-
// sub_surface_z, depending upon whether the fishnet is falling down
// or up.  The difference between the two pressures is
// returned as a measure of the local pressure energy near the top or
// bottom of the column.

double Fishnet::compute_local_ground_pressure_energy(
   unsigned int px,unsigned int py)
{
//   cout << "inside Fishnet::compute_local_ground_pressure_energy()" << endl;

//    const double pressure_term_coeff=100.0;

   double X,Y,Z;
   ztwoDarray_ptr->fast_pixel_to_XYZ(px,py,X,Y,Z);
   threevector curr_XYZ(X,Y,Z);

// Calculate spring neighbor positions:

   vector<threevector> neighbor_XYZ,nominal_XYZ;
   if (px > 0)
   {
      ztwoDarray_ptr->fast_pixel_to_XYZ(px-1,py,X,Y,Z);
      neighbor_XYZ.push_back(threevector(X,Y,Z));
      nominal_XYZ.push_back(threevector(X,Y,curr_XYZ.get(2)));
   }
   if (px < ztwoDarray_ptr->get_mdim()-1)
   {
      ztwoDarray_ptr->fast_pixel_to_XYZ(px+1,py,X,Y,Z);
      neighbor_XYZ.push_back(threevector(X,Y,Z));
      nominal_XYZ.push_back(threevector(X,Y,curr_XYZ.get(2)));
   }
   if (py > 0)
   {
      ztwoDarray_ptr->fast_pixel_to_XYZ(px,py-1,X,Y,Z);
      neighbor_XYZ.push_back(threevector(X,Y,Z));
      nominal_XYZ.push_back(threevector(X,Y,curr_XYZ.get(2)));
   }
   if (py < ztwoDarray_ptr->get_ndim()-1)
   {
      ztwoDarray_ptr->fast_pixel_to_XYZ(px,py+1,X,Y,Z);
      neighbor_XYZ.push_back(threevector(X,Y,Z));
      nominal_XYZ.push_back(threevector(X,Y,curr_XYZ.get(2)));
   }

// Compute pressure term integrals within XY region bounded by
// neighbor_XYZ's:

   double xlo=1E15;
   double ylo=1E15;
   double xhi=-1E15;
   double yhi=-1E15;
   for (unsigned int i=0; i<neighbor_XYZ.size(); i++)
   {
      xlo=basic_math::min(xlo,neighbor_XYZ[i].get(0));
      ylo=basic_math::min(ylo,neighbor_XYZ[i].get(1));
      xhi=basic_math::max(xhi,neighbor_XYZ[i].get(0));
      yhi=basic_math::max(yhi,neighbor_XYZ[i].get(1));
   }
   unsigned int mlo=VCP_ptr->x_to_m(xlo);
   unsigned int mhi=VCP_ptr->x_to_m(xhi);
   unsigned int nlo=VCP_ptr->y_to_n(ylo);
   unsigned int nhi=VCP_ptr->y_to_n(yhi);

//   cout << "xlo = " << xlo << " xhi = " << xhi << endl;
//   cout << "ylo = " << ylo << " yhi = " << yhi << endl;
//   cout << "mlo = " << mlo << " mhi = " << mhi << endl;
//   cout << "nlo = " << nlo << " nhi = " << nhi << endl;
   
   double E_pressure_at_surface=0;
   double E_pressure_sub_surface=0;

   for (unsigned int m=mlo; m<=mhi; m++)
   {
      double curr_x=VCP_ptr->m_to_x(m);
      for (unsigned int n=nlo; n<=nhi; n++)
      {

// Don't perform expensive pressure computation if mask value for
// (m,n) pixel = -1:

         if (pressure_twoDarray_ptr->get(m,n) < 0) continue;

         double curr_y=VCP_ptr->n_to_y(n);
         double curr_z;
         bool inside_flag=ztwoDarray_ptr->point_to_interpolated_value(
            curr_x,curr_y,curr_z);

         if (!inside_flag) continue;
         
         E_pressure_at_surface += pressure_term_coeff*
            VCP_ptr->get_cumulative_prob_integral(
               m,n,curr_z,fall_downwards_flag);

         const double sub_surface_dz=1;	// meters
//         const double sub_surface_dz=2;	// meters
         double sub_surface_z;
         if (fall_downwards_flag)
         {
            sub_surface_z=curr_z-sub_surface_dz;
         }
         else
         {
            sub_surface_z=curr_z+sub_surface_dz;
         }
         
         E_pressure_sub_surface += pressure_term_coeff*
            VCP_ptr->get_cumulative_prob_integral(
               m,n,sub_surface_z,fall_downwards_flag);

      } // loop over n index
   } // loop over m index
   
   double E_ground_pressure=E_pressure_sub_surface-E_pressure_at_surface;

   return E_ground_pressure;
}

// --------------------------------------------------------------------------
// Member function identify_masses_close_to_pointcloud() loops over
// all fishnet masses.  It computes and writes out a probability
// distribution for local ground pressure energies.  The user is then
// queried to enter a threshold ground pressure energy.  Any fishnet
// mass which has a local ground pressure exceeding the threshold is
// marked with a 3D Point, and its Z location is entered into
// *ground_twoDarray_ptr.

void Fishnet::identify_masses_close_to_pointcloud()
{
   cout << "inside Fishnet::identify_masses_close_to_pointcloud()" << endl;

// Mark masses with significant ground pressure with Points at their
// locations:

   threevector mass_XYZ;
   vector<double> E_ground_pressures;

   bool draw_text_flag=false;
   PointsGroup_ptr->destroy_all_Points();
   ground_twoDarray_ptr->initialize_values(NEGATIVEINFINITY);	

   for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
   {
      for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
      {
         double curr_E_ground_pressure=
            compute_local_ground_pressure_energy(px,py);
         E_ground_pressures.push_back(curr_E_ground_pressure);
      } // loop over py index
   } // loop over px index
   
   prob_distribution prob_ground_pressure(E_ground_pressures,100);
   prob_ground_pressure.writeprobdists(false);

   string banner="Wrote out ground pressure density distribution";
   outputfunc::write_big_banner(banner);

   double threshold_E_ground_pressure=300;
   cout << "Enter E ground pressure threshold value:" << endl;
   cin >> threshold_E_ground_pressure;

   for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
   {
      for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
      {
         double curr_E_ground_pressure=
            compute_local_ground_pressure_energy(px,py);
         if (curr_E_ground_pressure < threshold_E_ground_pressure) continue;

         ztwoDarray_ptr->pixel_to_threevector(px,py,mass_XYZ);
//          osgGeometry::Point* Point_ptr=
            PointsGroup_ptr->generate_new_Point(mass_XYZ,draw_text_flag);
         ground_twoDarray_ptr->put(px,py,mass_XYZ.get(2));
      } // loop over py index
   } // loop over px index
}

// --------------------------------------------------------------------------
// Member function export_inverted_ground_points() generates a text
// file contaiing X, Y and Z_ground entries.

void Fishnet::export_inverted_ground_points()
{
   cout << "inside Fishnet::export_inverted_ground_points()" << endl;

   string ground_filename="ground_surface.dat";
   ofstream groundstream;
   filefunc::openfile(ground_filename,groundstream);

   groundstream.precision(10);
   
   double x,y,z;
   for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
   {
      for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
      {
         ztwoDarray_ptr->fast_pixel_to_XYZ(px,py,x,y,z);
         
         double inverted_z=min_z_points+max_z_points-z;
         double z_ground=inverted_z+ground_surface_thickness;
         groundstream << x << "  " << y << "  "
                      << z_ground << endl;
      }	 // loop over py index
   } // loop over px index

   filefunc::closefile(ground_filename,groundstream);

   string banner="Wrote inverted ground points to "+ground_filename;
   outputfunc::write_big_banner(banner);
}

// --------------------------------------------------------------------------
// Member function export_inverted_ground_surface() generates a TDP
// file containing Zground values as a function of X & Y.

void Fishnet::export_inverted_ground_surface()
{
   cout << "inside Fishnet::export_inverted_ground_surface()" << endl;

   double xlo=ztwoDarray_ptr->get_xlo();
   double xhi=ztwoDarray_ptr->get_xhi();
   double ylo=ztwoDarray_ptr->get_ylo();
   double yhi=ztwoDarray_ptr->get_yhi();
   
   double dx=0.3;	// meter
   double dy=0.3;	// meter
   unsigned int mdim=(xhi-xlo)/dx+1;
   unsigned int ndim=(yhi-ylo)/dy+1;
   unsigned int n_exported_points=mdim*ndim;

   vector<double>* X_ptr=new vector<double>;
   vector<double>* Y_ptr=new vector<double>;
   vector<double>* Z_ptr=new vector<double>;
   vector<double>* P_ptr=new vector<double>;

   X_ptr->reserve(n_exported_points);
   Y_ptr->reserve(n_exported_points);
   Z_ptr->reserve(n_exported_points);
   P_ptr->reserve(n_exported_points);

   double p=0.5;
   for (unsigned int px=0; px<mdim; px++)
   {
      for (unsigned int py=0; py<ndim; py++)
      {

// Since x is altered later on, we need to set x INSIDE and not
// outside the py for loop !

         double x=xlo+px*dx;
         double y=ylo+py*dy;

         if (!ztwoDarray_ptr->point_inside_working_region(x,y)) continue;
         
         double z_interpolated;
//         bool flag=
            ztwoDarray_ptr->point_to_interpolated_value(x,y,z_interpolated);
         double inverted_z=min_z_points+max_z_points-z_interpolated;
         double z_ground=inverted_z+ground_surface_thickness;

         X_ptr->push_back(x);
         Y_ptr->push_back(y);
         Z_ptr->push_back(z_ground);
         P_ptr->push_back(p);
      } // loop over py index
   } // loop over px index

   string ground_tdp_filename="ground_surface.tdp";
   tdpfunc::write_xyzp_data(ground_tdp_filename,X_ptr,Y_ptr,Z_ptr,P_ptr);
   string unix_cmd="lodtree "+ground_tdp_filename;
   sysfunc::unix_command(unix_cmd);
 
   delete X_ptr;
   delete Y_ptr;
   delete Z_ptr;
   delete P_ptr;

   string banner="Wrote inverted ground surface to "+ground_tdp_filename;
   outputfunc::write_big_banner(banner);
}

    
