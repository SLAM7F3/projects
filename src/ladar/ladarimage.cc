// On 1/6/03, we doublechecked that double loops over pixels should
// definitely have the px loop on the outside and the py loop on the
// inside!

// ==========================================================================
// LADARIMAGE base class member function definitions
// ==========================================================================
// Last modified on 8/3/06; 3/17/09; 5/20/09; 12/4/10; 4/5/14
// ==========================================================================

#include "math/basic_math.h"
#include "image/binaryimagefuncs.h"
#include "image/compositefuncs.h"
#include "image/connectfuncs.h"
#include "math/constants.h"
#include "datastructures/datapoint.h"
#include "image/drawfuncs.h"
#include "threeDgraphics/draw3Dfuncs.h"
#include "general/filefuncs.h"
#include "ladar/groundfuncs.h"
#include "datastructures/Hashtable.h"
#include "image/imagefuncs.h"
#include "ladar/ladarfuncs.h"
#include "ladar/ladarimage.h"
#include "datastructures/Linkedlist.h"
#include "math/mathfuncs.h"
#include "datastructures/Mynode.h"
#include "templates/mytemplates.h"
#include "general/outputfuncs.h"
#include "geometry/parallelogram.h"
#include "geometry/polygon.h"
#include "math/prob_distribution.h"
#include "image/recursivefuncs.h"
#include "geometry/regular_polygon.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/threevector.h"
#include "time/timefuncs.h"
#include "image/TwoDarray.h"
#include "threeDgraphics/xyzpfuncs.h"

typedef Linkedlist<datapoint> linkedlist;

using std::cin;
using std::cout;
using std::endl;
using std::ios;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void ladarimage::allocate_member_objects()
{
}		       

void ladarimage::initialize_member_objects()
{
   public_software=false;
   imagedir="";
   n_xyz_points=0;
   delta_z=0.3;		// 30 centimeter resolution for ALIRT data assumed
   image_origin=Zero_vector;
   pmin=0;
   pmax=1;
   data_bbox_ptr=NULL;
   p2Darray_orig_ptr=NULL;
   p2Darray_ptr=NULL;
   gradient_mag_twoDarray_ptr=NULL;
   gradient_phase_twoDarray_ptr=NULL;
   zlaplacian_twoDarray_ptr=NULL;
   normal_twoDarray_ptr=NULL;
   connected_heights_hashtable_ptr=NULL;
   connected_gradient_phases_hashtable_ptr=NULL;
   set_black_corresponds_to_minimumz(false);
   start_processing_time=time(NULL);
}		       

ladarimage::ladarimage()
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

// Copy constructor:

ladarimage::ladarimage(const ladarimage& m):
myimage(m)
{
   initialize_member_objects();
   allocate_member_objects();
   docopy(m);
}

ladarimage::ladarimage(int Nxbins,int Nybins):
myimage(Nxbins,Nybins)
{
   initialize_member_objects();
   allocate_member_objects();
}

// This overloaded constructor function creates a ladarimage object and
// fills its entries with values from twoDarray T:

ladarimage::ladarimage(const twoDarray& T):
myimage(T)
{		  
   initialize_member_objects();
   allocate_member_objects();
}		       

ladarimage::~ladarimage()
{
   delete data_bbox_ptr;
   delete p2Darray_ptr;
   delete p2Darray_orig_ptr;
   delete gradient_mag_twoDarray_ptr;
   delete gradient_phase_twoDarray_ptr;
   delete zlaplacian_twoDarray_ptr;
   delete normal_twoDarray_ptr;
   connectfunc::delete_connected_hashtable(
      connected_heights_hashtable_ptr);
   connectfunc::delete_connected_hashtable(
      connected_gradient_phases_hashtable_ptr);
}

// ---------------------------------------------------------------------
void ladarimage::docopy(const ladarimage& m)
{
   public_software=m.public_software;
   logfilename=m.logfilename;
   xyz_datadir=m.xyz_datadir;
   xyz_filenamestr=m.xyz_filenamestr;
   n_xyz_points=m.n_xyz_points;
   start_processing_time=m.start_processing_time;
   image_origin=m.image_origin;
   xextent=m.xextent;
   yextent=m.yextent;
   zextent=m.zextent;
   pmin=m.pmin;
   pmax=m.pmax;
   pextent=m.pextent;
   flightpath_poly=m.flightpath_poly;
}

// Overload = operator:

ladarimage& ladarimage::operator= (const ladarimage& m)
{
   if (this==&m) return *this;
   myimage::operator=(m);
   docopy(m);
   return *this;
}

// ==========================================================================
// Metafile display member functions:
// ==========================================================================

// This overridden version of generate_colortable is tailored for
// ALIRT applications.  The strength of each color in the table (which
// we define as sqrt(R**2+G**2+B**2) is fixed by input parameter
// intensity.  The number of entries within the table is also passed
// via input parameter ncolorsteps.  The minimum [maximum] value of z
// is mapped to purple [red].  Black is reserved for values of z less
// than zmin (which typically equals 0).  White is reserved for
// "POSITIVEINFINITY" values of z.

   void ladarimage::generate_colortable(
      int ncolorsteps,double zmin,double zmax,double intensity,
      double height,double width,double legloc_x,double legloc_y)
   {
      insert_colortable_header(height,width,legloc_x,legloc_y);

      const int max_intensity=255;
      int nsteps1=basic_math::round(floor(0.2*ncolorsteps));
      int nsteps2=basic_math::round(floor(0.4*ncolorsteps));
      int nsteps3=basic_math::round(floor(0.4*ncolorsteps));
      int ninterpolations=basic_math::round(floor(200.0/double(ncolorsteps)));
      int red,green,blue;
      red=green=blue=0;
      double currz,deltaz=zmax-zmin;

      imagestream << "colortable" << endl;
      imagestream << "# index    red     green   blue    interpolations  label"
                  << endl;

// Map z values less than zmin to black:

      imagestream << zmin-10 << "\t" << 0 << " \t" << 0 << "\t"
                  << 0 << "\t" << 2 << endl;
   
// Lowest 20% of z values map from purple to blue:

      double theta_lo=PI/4;
      double theta_hi=0;
      double dtheta=(theta_hi-theta_lo)/nsteps1;
      for (int n=0; n<nsteps1; n++)
      {
         double theta=theta_lo+n*dtheta;
         red=basic_math::round(intensity*sin(theta));
         green=0;
         blue=basic_math::round(intensity*cos(theta));
         currz=zmin+basic_math::round(double(n)/double(ncolorsteps)*deltaz);
         imagestream << currz << "\t" << red << " \t" << green << "\t"
                     << blue << "\t" << ninterpolations << "\t" 
                     << "'" << stringfunc::number_to_string(currz) << "'" 
                     << endl;
      }

// Middle 40% of z values map from blue to green:

      theta_lo=0;
      theta_hi=PI/2;
      dtheta=(theta_hi-theta_lo)/nsteps2;
      for (int n=nsteps1; n<nsteps1+nsteps2; n++)
      {
         double theta=theta_lo+(n-nsteps1)*dtheta;
         red=0;
         green=basic_math::round(intensity*sin(theta));
         blue=basic_math::round(intensity*cos(theta));
         currz=zmin+basic_math::round(double(n)/double(ncolorsteps)*deltaz);
         imagestream << currz << "\t" << red << " \t" << green << "\t"
                     << blue << "\t" << ninterpolations << "\t" 
                     << "'" << stringfunc::number_to_string(currz) << "'" 
                     << endl;
      }

// Top 40% of z values map from green to red:

      theta_lo=0;
      theta_hi=PI/2;
      dtheta=(theta_hi-theta_lo)/(nsteps3-1);
      for (int n=nsteps1+nsteps2; n<ncolorsteps; n++)
      {
         double theta=theta_lo+(n-nsteps1-nsteps2)*dtheta;
         red=basic_math::round(intensity*sin(theta));
         green=basic_math::round(intensity*cos(theta));
         blue=0;
         currz=zmin+basic_math::round(double(n)/double(ncolorsteps)*deltaz);
         imagestream << currz << "\t" << red << " \t" << green << "\t"
                     << blue << "\t" << ninterpolations << "\t" 
                     << "'" << stringfunc::number_to_string(currz) << "'" 
                     << endl;
      }

// 1000 to "POSITIVEINFINITY" z values map to white:

      imagestream << 999 << "\t" << red << " \t" << green << "\t"
                  << blue << "\t" << 2 << endl;
      imagestream << 1000 << "\t" << max_intensity << " \t" 
                  << max_intensity << "\t"
                  << max_intensity << "\t" << 2 << endl;
   }

// ---------------------------------------------------------------------
// Member function writeimage generates metafile output for either a
// z-image, p-image or a fused height & detection probability image.

void ladarimage::writeimage(
   string filename,twoDarray const *ztwoDarray_ptr,
   bool display_flightpath,Data_kind datatype,string title)
{
   classified=false;
   if (title=="") title=xyz_filenamestr;


   string colortable_dirname=filefunc::get_pwd()+"colortables/";
   if (datatype==z_data)
   {
      subtitle="Height Image";
      colortablefilename=colortable_dirname+"colortable.ladar";
   }
   else if (datatype==p_data)
   {
      subtitle="Intensity Image";
      colortablefilename=colortable_dirname+"colortable.prob";
   }
   else if (datatype==fused_data)
   {
      subtitle="Fused Image";
      colortablefilename=colortable_dirname+"colortable.fuse";
   }
   else if (datatype==phase_data)
   {
      subtitle="Phase Image";
      colortablefilename=colortable_dirname+"colortable.phase";
   }
   else if (datatype==direction_data)
   {
      subtitle="Direction Image";
      colortablefilename=colortable_dirname+"colortable.direction";
   }
   xaxislabel="Relative X (meters)";
   yaxislabel="Relative Y (meters)";

   twoDarray *zcopy_twoDarray_ptr=new twoDarray(*ztwoDarray_ptr);
   double final_xextent=zcopy_twoDarray_ptr->get_xhi()-
      zcopy_twoDarray_ptr->get_xlo();
   double final_yextent=zcopy_twoDarray_ptr->get_yhi()-
      zcopy_twoDarray_ptr->get_ylo();

   if (display_flightpath) draw_fitted_flightpath(zcopy_twoDarray_ptr);
   if (data_bbox_ptr != NULL)
   {
      ladarfunc::draw_data_bbox(data_bbox_ptr,zcopy_twoDarray_ptr);
   }
   
   xtic=xsubtic=trunclog(final_xextent);		// meters
   ytic=ysubtic=trunclog(final_yextent);		// meters
   adjust_x_scale=false;

   myimage::writeimage(filename,-1,title,final_xextent,final_yextent,
                       zcopy_twoDarray_ptr);
   delete zcopy_twoDarray_ptr;
}

// ---------------------------------------------------------------------
// Member function update_logfile records xyzp file name and timing
// information to a running log file.

void ladarimage::update_logfile(string main_program_filename)
{
   ofstream logstream;
   filefunc::appendfile(logfilename,logstream);

   outputfunc::write_initial_results_info(logstream);
   logstream << "Main program filename = " << main_program_filename << endl;
   logstream << "Input xyz filename = " << xyz_filenamestr << endl;
   logstream << "Output directory containing all system results = "
             << imagedir << endl;

   outputfunc::report_processing_time_info(start_processing_time,logstream);
   filefunc::closefile(logfilename,logstream);
}

// ==========================================================================
// Data entry member functions:
// ==========================================================================

void ladarimage::initialize_image(
   bool input_param_file,string inputline[],unsigned int& currlinenumber)
{
   string outputline[5];
   if (public_software)
   {
      logfilename=outputfunc::select_logfile_name(
         input_param_file,inputline,currlinenumber);
      outputline[0]="Enter full path for input binary xyzp data file:";
      string full_filenamestr=stringfunc::mygetstring(
         1,outputline,input_param_file,inputline,currlinenumber);

      xyz_filenamestr=filefunc::getbasename(full_filenamestr);
      xyz_datadir=filefunc::getdirname(full_filenamestr);
      if (imagedir=="")
      {
         imagedir=outputfunc::select_output_directory(
            public_software,false,
            input_param_file,inputline,currlinenumber,"images");
      }
   }
   else
   {
      outputline[0]="Enter name of input binary xyzp data file:";
      xyz_filenamestr=stringfunc::mygetstring(
         1,outputline,input_param_file,inputline,currlinenumber);
      xyz_datadir=filefunc::get_pwd()+"xyz_data/";

      imagedir=filefunc::get_pwd()+"images/"+xyz_filenamestr+"/";
      filefunc::dircreate(imagedir);
      logfilename=filefunc::get_pwd()+"images/ladar.logfile";
//      cout << "logfilename = " << logfilename << endl;
   }

   cout << "Output subdirectory = " << imagedir << endl;
   outputfunc::newline();
}

// ---------------------------------------------------------------------
// Member function parse_and_store_input_data parses a Group 94 binary
// xyzp file, translates the data so that it starts at (x,y)=(0,0) and
// then fills the current ladarimage object's z and p twoDarrays with
// the data.

// Note: We generally need to set input flag
// renormalize_just_xy_values to true for input data sets which were
// cropped out from the middle parts of larger data sets!

void ladarimage::parse_and_store_input_data(
   double deltax,double deltay,
   bool renormalize_just_xy_values,bool renormalize_xyz_data,
   bool remove_snowflakes,bool rotate_xy_values,double theta)
{
   set_npoints(xyzpfunc::npoints_inside_xyzp_file(
                  xyz_datadir+xyz_filenamestr));
   vector<threevector> XYZ;
   vector<double> P;
   XYZ.reserve(n_xyz_points);
   P.reserve(n_xyz_points);

   xyzpfunc::read_xyzp_float_data(xyz_datadir+xyz_filenamestr,XYZ,P);

   if (renormalize_xyz_data) 
      ladarfunc::shift_HAFB_to_Greenwich_origin(XYZ);
   compute_xyzp_origin_and_extents(XYZ,P);

   double renorm_xlo=image_origin.get(0);
   double renorm_ylo=image_origin.get(1);
   if (renormalize_just_xy_values)
   {
      convert_from_absolute_to_relative_xyz(
         threevector(image_origin.get(0),image_origin.get(1)),XYZ);
   }
   else if (renormalize_xyz_data)
   {
      convert_from_absolute_to_relative_xyz(image_origin,XYZ);
   }
   renorm_xlo=renorm_ylo=0;

// In order to cut down on wasted twoDarray storage space, we
// sometimes may need to rotate the x and y coordinates for all raw
// xyzp quadruples BEFORE they are binned into twoDarrays:

   string rotated_filename=imagedir+"rotated_"+xyz_filenamestr;
   if (rotate_xy_values)
   {
      threevector origin(0.5*xextent,0.5*yextent);
      ladarfunc::rotate_xy_coords(theta,origin,XYZ);
      xyzpfunc::write_xyzp_data(XYZ,P,rotated_filename,true);
   }
   
   if (remove_snowflakes)
   {
      ladarfunc::mark_snowflake_points(
         image_origin.get(2),image_origin.get(2)+zextent,delta_z,XYZ);
//      string nosnow_filename=imagedir+"nosnow_"+xyz_filenamestr;
//      xyzpfunc::write_xyzp_data(n_xyz_points,x,y,z,p,nosnow_filename,true);
//      threevector midpoint(0.5*xextent,0.5*yextent,10);
//      filefunc::gunzip_file_if_gzipped(nosnow_filename);
//      draw3Dfunc::draw_tiny_cube(
//         midpoint,nosnow_filename,1.0,0.05,10);
   }

   initialize_image_parameters(
      z2Darray_orig_ptr,p2Darray_orig_ptr,deltax,deltay,
      renorm_xlo,renorm_ylo);

   xyzpfunc::fill_image_with_z_and_p_values(
      XYZ,P,z2Darray_orig_ptr,p2Darray_orig_ptr);
   ladarfunc::compute_xyzp_distributions(this,XYZ,P);

   delete z2Darray_ptr;
   delete p2Darray_ptr;
   z2Darray_ptr=new twoDarray(*z2Darray_orig_ptr);
   p2Darray_ptr=new twoDarray(*p2Darray_orig_ptr);
}

// ---------------------------------------------------------------------
void ladarimage::store_input_data(
   double xmin,double ymin,double xmax,double ymax,
   double deltax,double deltay,
   osg::Vec3Array* vertices_ptr,const osg::FloatArray* probs_ptr)
{
   cout << "inside ladarimage::store_input_data()" << endl;
   
   xextent=xmax-xmin;
   yextent=ymax-ymin;

   if (probs_ptr==NULL)
   {
      initialize_image_parameters(
         z2Darray_orig_ptr,deltax,deltay,xmin,ymin);
      xyzpfunc::fill_image_with_z_values(vertices_ptr,z2Darray_orig_ptr);
   }
   else
   {
      initialize_image_parameters(
         z2Darray_orig_ptr,p2Darray_orig_ptr,deltax,deltay,xmin,ymin);
      xyzpfunc::fill_image_with_z_and_p_values(
         vertices_ptr,probs_ptr,z2Darray_orig_ptr,p2Darray_orig_ptr);
   }

   delete z2Darray_ptr;
   delete p2Darray_ptr;
   z2Darray_ptr=new twoDarray(*z2Darray_orig_ptr);

   if (p2Darray_orig_ptr != NULL)
      p2Darray_ptr=new twoDarray(*p2Darray_orig_ptr);

   cout << "at end of ladarimage::store_input_data()" << endl;
}

// ---------------------------------------------------------------------
// Member function compute_xyzp_origin_and_extents locates the
// extremal members within the XYZ and P vectors.

void ladarimage::compute_xyzp_origin_and_extents(
   const vector<threevector>& XYZ,const vector<double>& p)
{
//   cout << "inside ladarimage::compute_xyzp_origin_and_extents()" << endl;
   double xmin,xmax,ymin,ymax,zmin,zmax;
   xmin=ymin=zmin=pmin=POSITIVEINFINITY;
   xmax=ymax=zmax=pmax=NEGATIVEINFINITY;
   for (unsigned int n=0; n<XYZ.size(); n++)
   {
      xmin=basic_math::min(xmin,XYZ[n].get(0));
      xmax=basic_math::max(xmax,XYZ[n].get(0));
      ymin=basic_math::min(ymin,XYZ[n].get(1));
      ymax=basic_math::max(ymax,XYZ[n].get(1));
      zmin=basic_math::min(zmin,XYZ[n].get(2));
      zmax=basic_math::max(zmax,XYZ[n].get(2));
      pmin=basic_math::min(pmin,p[n]);
      pmax=basic_math::max(pmax,p[n]);
   }
   image_origin=threevector(xmin,ymin,zmin);

   cout.precision(10);
   cout << "xmin = " << xmin << " xmax = " << xmax << endl;
   cout << "ymin = " << ymin << " ymax = " << ymax << endl;
   cout << "zmin = " << zmin << " zmax = " << zmax << endl;
   cout << "pmin = " << pmin << " pmax = " << pmax << endl;

   xextent=xmax-xmin;
   yextent=ymax-ymin;
   zextent=zmax-zmin;
   pextent=pmax-pmin;
   
   cout << "x min = " << xmin << " x extent = " << xextent 
        << " meters" << endl;
   cout << "y min = " << ymin << " y extent = " << yextent 
        << " meters" << endl;
   cout << "z min = " << zmin << " z extent = " << zextent 
        << " meters" << endl;
   cout << "p extent = " << pextent << endl;
   outputfunc::newline();
}

// ---------------------------------------------------------------------
// Member function convert_from_absolute_to_relative_xyz converts
// ABSOLUTE xyz data (measured relative to some origin which should be
// Greenwich) into RELATIVE values (measured relative to the input
// absolute_origin) which range over the intervals 0 <= x <= x_extent,
// 0 <= y <= y_extent and 0 <= z <= z_extent:

void ladarimage::convert_from_absolute_to_relative_xyz(
   const threevector& absolute_origin,vector<threevector>& XYZ)
{
   for (unsigned int n=0; n<XYZ.size(); n++)
   {
      XYZ[n] -= absolute_origin;
   }
}
     
// ---------------------------------------------------------------------
// Member function initialize_image_parameters takes in "chimney" bin
// dimensions delta_x and delta_y measured in meters.  It uses them to
// establish a relative Cartesian coordinate whose origin is centered
// in the bottom left corner of the ladarimage.

void ladarimage::initialize_image_parameters(
   twoDarray*& ztwoDarray_ptr,
   double deltax,double deltay,double xlo,double ylo)
{
//   const int mdim_max=13000;
//   const int ndim_max=7000;
   int mdim=basic_math::round(xextent/deltax)+1;
   int ndim=basic_math::round(yextent/deltay)+1;
   cout << "mdim = " << mdim << " ndim = " << ndim << endl;

/*
  if (mdim > mdim_max)
  {
  deltax *= double(mdim)/double(mdim_max);
  mdim=mdim_max;
  }
  if (ndim > ndim_max)
  {
  deltay *= double(ndim)/double(ndim_max);
  ndim=ndim_max;
  }
*/

   delete ztwoDarray_ptr;
   ztwoDarray_ptr=new twoDarray(mdim,ndim);
   ztwoDarray_ptr->set_deltax(deltax);
   ztwoDarray_ptr->set_xlo(xlo);
   ztwoDarray_ptr->set_xhi(xlo+(mdim-1)*deltax);
   ztwoDarray_ptr->set_deltay(deltay);
   ztwoDarray_ptr->set_ylo(ylo);
   ztwoDarray_ptr->set_yhi(ylo+(ndim-1)*deltay);

   cout.precision(10);
   cout << "xlo = " << ztwoDarray_ptr->get_xlo() 
        << " xhi = " << ztwoDarray_ptr->get_xhi() << endl;
   cout << "ylo = " << ztwoDarray_ptr->get_ylo() 
        << " yhi = " << ztwoDarray_ptr->get_yhi() << endl;
   cout << "deltax = " << ztwoDarray_ptr->get_deltax()
        << " deltay = " << ztwoDarray_ptr->get_deltay() << endl;
   cout << "mdim = " << ztwoDarray_ptr->get_mdim() 
        << " ndim = " << ztwoDarray_ptr->get_ndim() << endl;
   cout << "mdim*ndim = " << ztwoDarray_ptr->get_mdim()*
      ztwoDarray_ptr->get_ndim() << endl;
   outputfunc::newline();
}

void ladarimage::initialize_image_parameters(
   twoDarray*& ztwoDarray_ptr,twoDarray*& ptwoDarray_ptr,
   double deltax,double deltay,double xlo,double ylo)
{
   initialize_image_parameters(ztwoDarray_ptr,deltax,deltay,xlo,ylo);
   delete ptwoDarray_ptr;
   ptwoDarray_ptr=new twoDarray(*ztwoDarray_ptr);
}

// ---------------------------------------------------------------------
// Member function fill_arrays_with_x_y_z_and_p_values takes in z and
// p twoDarrays.  It converts the non-null valued information within
// these twoDarrays into one-dimensional x[], y[], z[] and p[] output
// arrays.

int ladarimage::fill_arrays_with_x_y_z_and_p_values(
   double xoffset,double yoffset,
   twoDarray const *ztwoDarray_ptr,twoDarray const *ptwoDarray_ptr,
   double x[],double y[],double z[],double p[])
{
   int n=0;
   double currx,curry;
   for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
   {
      for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
      {
         if (ztwoDarray_ptr->get(px,py) > 0.99*NEGATIVEINFINITY)
         {
            ztwoDarray_ptr->pixel_to_point(px,py,currx,curry);
            x[n]=currx+xoffset;
            y[n]=curry+yoffset;
            z[n]=ztwoDarray_ptr->get(px,py);
            p[n]=ptwoDarray_ptr->get(px,py);
            n++;
         } // ztwoDarray(px,py) > NEGATIVEINFINITY conditional
      } // loop over index py
   } // loop over index px
   return n;
}

// ==========================================================================
// Flight path member functions:
// ==========================================================================

// Member function fit_flightpath_poly fits a linear polynomial to
// yflight as a function of xflight over the imaged region contained
// within *ztwoDarray_ptr.

void ladarimage::fit_flightpath_poly(
   int nflight_points,double xflight[],double yflight[],
   twoDarray* ztwoDarray_ptr)
{
   int nskip=nflight_points/2000;
   int n=0;
   int npnts_inside_image=0;
   
   double x[nskip+1],y[nskip+1];
   while (n<nflight_points)
   {
      if (ztwoDarray_ptr->point_inside_working_region(xflight[n],yflight[n]))
      {
         x[npnts_inside_image]=xflight[n];
         y[npnts_inside_image]=yflight[n];
         npnts_inside_image++;
      }
      n += nskip;
   }
 
   flightpath_poly.set_order(1);
   flightpath_poly.set_basis(mypolynomial::chebyshev);
//   double chisq=
      flightpath_poly.fit_coeffs_using_residuals(npnts_inside_image,x,y);
   cout << "flightpath_poly = " << flightpath_poly << endl;
//   cout << "chisq = " << chisq << endl;
}

// ---------------------------------------------------------------------
// Member function draw_fitted_flightpath draws a white path on top of
// the z values contained within *z2Darray_ptr to indicate the fitted
// polynomial path of the airplane.

void ladarimage::draw_fitted_flightpath(twoDarray* ztwoDarray_ptr)
{
   double x_extent=ztwoDarray_ptr->get_xhi()-ztwoDarray_ptr->get_xlo();
   double y_extent=ztwoDarray_ptr->get_yhi()-ztwoDarray_ptr->get_ylo();
   double radius=0.005*sqrt(sqr(x_extent)+sqr(y_extent));
   double xstep=x_extent/30.0;	// meters
   double currx=ztwoDarray_ptr->get_xlo();
   while (currx < ztwoDarray_ptr->get_xhi())
   {
      double curry=flightpath_poly.value(currx);
      drawfunc::draw_hugepoint(
         threevector(currx,curry,0),radius,colorfunc::white,ztwoDarray_ptr);
      currx += xstep;
   }
}

// ---------------------------------------------------------------------
// Member function compute_data_bbox searches for non-null pixels
// located along the edges of the image contained in input image
// *ztwoDarray_ptr.  It moves a small bounding box nearby the edges of
// the image and requires that this box be filled with some
// non-negligible fraction of non-null valued pixels.  This method
// takes the locations where the bounding box is filled as vertices of
// a bounding quadrilateral which wraps around the data.  It then
// deforms the quadrilateral into a bounding parallelogram which can
// be used to rapidly determine whether points lie within its interior
// or not.

void ladarimage::compute_data_bbox(
   twoDarray* ztwoDarray_ptr,bool scale_data_bbox,
   bool conservative_parallelogram_flag)
{
   outputfunc::write_banner("Computing data bounding box:");

   double bbox_width=1;		// meter
   double bbox_length=1;	// meter
   double bbox_area=bbox_width*bbox_length;	// meter**2
   double pixel_area=ztwoDarray_ptr->get_deltax()*
      ztwoDarray_ptr->get_deltay();
   int n_bbox_pixels=basic_math::round(bbox_area/pixel_area);
   const double min_nonnull_frac=0.5;
   const int min_nonnull_pixels=
      basic_math::round(min_nonnull_frac*n_bbox_pixels);

// Find x locations of non-null pixels located near top and bottom
// edges of image:

   double x2,x4,y2,y4;
   bool found_nonnull_pixel=false;
   unsigned int row_number=0;
   int n_nonnull_pixels=-1;
   do
   {
      y2=ztwoDarray_ptr->get_yhi()-row_number*ztwoDarray_ptr->get_deltay();
      x2=ztwoDarray_ptr->get_xlo();
      do
      {
         n_nonnull_pixels=
            imagefunc::count_pixels_above_zmin_inside_bbox(
               x2,y2,x2+bbox_width,y2+bbox_length,
               xyzpfunc::null_value,ztwoDarray_ptr);
         found_nonnull_pixel=(n_nonnull_pixels > min_nonnull_pixels);
         x2 += ztwoDarray_ptr->get_deltax();
      }
      while (!found_nonnull_pixel && x2 < ztwoDarray_ptr->get_xhi());
      row_number++;
//      cout << "row_number = " << row_number 
//           << " x2 = " << x2 << " y2 = " << y2 << endl;
   }
   while (!found_nonnull_pixel && row_number < ztwoDarray_ptr->get_ndim());

   found_nonnull_pixel=false;
   row_number=0;
   do
   {
      y4=ztwoDarray_ptr->get_ylo()+row_number*ztwoDarray_ptr->get_deltay();
      x4=ztwoDarray_ptr->get_xlo();
      do
      {
         n_nonnull_pixels=
            imagefunc::count_pixels_above_zmin_inside_bbox(
               x4,y4,x4+bbox_width,y4+bbox_length,
               xyzpfunc::null_value,ztwoDarray_ptr);
         found_nonnull_pixel=(n_nonnull_pixels > min_nonnull_pixels);
         x4 += ztwoDarray_ptr->get_deltax();
      }
      while (!found_nonnull_pixel && x4 < ztwoDarray_ptr->get_xhi());
      row_number++;
//      cout << "row_number = " << row_number 
//           << " x4 = " << x4 << " y4 = " << y4 << endl;
   }
   while (!found_nonnull_pixel && row_number < ztwoDarray_ptr->get_ndim());

// Find y locations of non-null pixels located near left & right edges
// of image:

   double x1,y1,x3,y3;
   found_nonnull_pixel=false;
   unsigned int column_number=0;
   do
   {
      x1=ztwoDarray_ptr->get_xlo()+column_number*ztwoDarray_ptr->
         get_deltax();
      y1=ztwoDarray_ptr->get_ylo();
      do
      {
         n_nonnull_pixels=
            imagefunc::count_pixels_above_zmin_inside_bbox(
               x1,y1,x1+bbox_width,y1+bbox_length,
               xyzpfunc::null_value,ztwoDarray_ptr);
         found_nonnull_pixel=(n_nonnull_pixels > min_nonnull_pixels);
         y1 += ztwoDarray_ptr->get_deltay();
      }
      while (!found_nonnull_pixel && y1 < ztwoDarray_ptr->get_yhi());
//      cout << "column_number = " << column_number 
//           << " x1 = " << x1 << " y1 = " << y1 << endl;
      column_number++;
   }
   while (!found_nonnull_pixel && column_number < ztwoDarray_ptr->get_mdim());

   found_nonnull_pixel=false;
   column_number=0;
   do
   {
      x3=ztwoDarray_ptr->get_xhi()-column_number*ztwoDarray_ptr->get_deltax();
      y3=ztwoDarray_ptr->get_ylo();
      do
      {
         n_nonnull_pixels=
            imagefunc::count_pixels_above_zmin_inside_bbox(
               x3,y3,x3+bbox_width,y3+bbox_length,
               xyzpfunc::null_value,ztwoDarray_ptr);
         found_nonnull_pixel=(n_nonnull_pixels > min_nonnull_pixels);
         y3 += ztwoDarray_ptr->get_deltay();
      }
      while (!found_nonnull_pixel && y3 < ztwoDarray_ptr->get_yhi());
//      cout << "column_number = " << column_number 
//           << " x3 = " << x3 << " y3 = " << y3 << endl;
      column_number++;
   }
   while (!found_nonnull_pixel && column_number < ztwoDarray_ptr->get_mdim());

   vector<threevector> vertex(4);
   vertex[0]=threevector(x1,y1,0);
   vertex[1]=threevector(x2,y2,0);
   vertex[2]=threevector(x3,y3,0);
   vertex[3]=threevector(x4,y4,0);
   polygon* quadrilateral_bbox_ptr=new polygon(vertex);
//   cout << "quadrilateral bbox = " << *quadrilateral_bbox_ptr << endl;

// If the bounding box is forced to be a parallelogram, we can use
// simple metric techniques to determine whether or not some (x,y)
// lies within its interior.  This simple change to our bounding box
// computation significantly speeds up median filling and perhaps
// ground extraction.

   quadrilateral_bbox_ptr->initialize_edge_segments();

   threevector l_hat=0.5*(quadrilateral_bbox_ptr->get_edge(0).get_ehat()-
                          quadrilateral_bbox_ptr->get_edge(2).get_ehat());
   threevector w_hat=0.5*(quadrilateral_bbox_ptr->get_edge(1).get_ehat()-
                          quadrilateral_bbox_ptr->get_edge(3).get_ehat());
   l_hat=l_hat.unitvector();
   w_hat=w_hat.unitvector();

   double L,W;
   if (conservative_parallelogram_flag)
   {
      L=basic_math::min(quadrilateral_bbox_ptr->get_edge(0).get_length(),
                        quadrilateral_bbox_ptr->get_edge(2).get_length());
      W=basic_math::min(quadrilateral_bbox_ptr->get_edge(1).get_length(),
                        quadrilateral_bbox_ptr->get_edge(3).get_length());
   }
   else
   {
      L=basic_math::max(quadrilateral_bbox_ptr->get_edge(0).get_length(),
                        quadrilateral_bbox_ptr->get_edge(2).get_length());
      W=basic_math::max(quadrilateral_bbox_ptr->get_edge(1).get_length(),
                        quadrilateral_bbox_ptr->get_edge(3).get_length());
   }
   delete quadrilateral_bbox_ptr;

   vertex[0]=threevector(x1,y1,0);
   vertex[1]=vertex[0]+L*l_hat;
   vertex[2]=vertex[1]+W*w_hat;
   vertex[3]=vertex[2]-L*l_hat;
   data_bbox_ptr=new parallelogram(vertex);
   if (scale_data_bbox) data_bbox_ptr->scale(0.95);
//   cout << "parallelogram bbox = " << *data_bbox_ptr << endl;
}

// ---------------------------------------------------------------------
// Member function compute_trivial_xy_data_bbox takes in twoDarray
// *ztwoDarray_ptr which is assumed to contain a height image whose
// symmetry directions have already been aligned with the x and y
// axes.  (This is the case, for instance, when we're working with IED
// strip data which has been rotated so that the x and y axes coincide
// with the flight path's along-track and cross-track directions.)
// This method simply constructs a rectangular bounding box based upon
// the twoDarray's x and y extents and sets it equal to the ladar
// image's data bounding box.

void ladarimage::compute_trivial_xy_data_bbox(twoDarray const *ztwoDarray_ptr)
{
   outputfunc::write_banner("Computing trivial data bounding box:");

   double xlo=ztwoDarray_ptr->get_xlo();
   double xhi=ztwoDarray_ptr->get_xhi();
   double ylo=ztwoDarray_ptr->get_ylo();
   double yhi=ztwoDarray_ptr->get_yhi();
   vector<threevector> vertex(4);
   vertex[0]=threevector(xlo,ylo);
   vertex[1]=threevector(xhi,ylo);
   vertex[2]=threevector(xhi,yhi);
   vertex[3]=threevector(xlo,yhi);
   data_bbox_ptr=new parallelogram(vertex);
//   cout << "parallelogram bbox = " << *data_bbox_ptr << endl;
}

// ---------------------------------------------------------------------
// Member function use_bbox_to_resize_ladarimage effectively crops
// away those parts of the z and p images which lie outside of the
// data bounding box.  It first recomputes the x and y extents of the
// ladar data based upon bounding box vertex information.  It then
// copies x, y, z and p information into temporary one-dimensional
// arrays.  The original *z2Darray_ptr and *p2Darray_ptr twoDarrays
// are deleted, smaller versions of these twoDarrays are instantiated
// and height/intensity data is copied from the temporary arrays into
// these smaller containers.  Finally, this method translates the
// bounding box so that it touches the edges of the smaller new
// images.

void ladarimage::use_bbox_to_resize_ladarimage(double deltax,double deltay)
{
   outputfunc::write_banner("Resizing ladar image based upon bbox info:");

   double xmin=data_bbox_ptr->get_vertex(0).get(0);
   double xmax=data_bbox_ptr->get_vertex(2).get(0);
   double ymin=data_bbox_ptr->get_vertex(3).get(1);
   double ymax=data_bbox_ptr->get_vertex(1).get(1);
   xextent=xmax-xmin;
   yextent=ymax-ymin;

   double *x=new double[n_xyz_points];
   double *y=new double[n_xyz_points];
   double *z=new double[n_xyz_points];
   double *p=new double[n_xyz_points];

   int npoints=fill_arrays_with_x_y_z_and_p_values(
      -xmin,-ymin,z2Darray_ptr,p2Darray_ptr,x,y,z,p);
   initialize_image_parameters(z2Darray_ptr,p2Darray_ptr,deltax,deltay);
   xyzpfunc::fill_image_with_z_and_p_values(
      npoints,x,y,z,p,z2Darray_ptr,p2Darray_ptr);
   delete [] x;
   delete [] y;
   delete [] z;
   delete [] p;

   data_bbox_ptr->translate(threevector(-xmin,-ymin,0));

// Destroy twoDarrays *z2Darray_orig_ptr and *p2Darray_orig_ptr.  Then
// reinstantiate new versions of these arrays:

   delete z2Darray_orig_ptr;
   delete p2Darray_orig_ptr;
   z2Darray_orig_ptr=new twoDarray(*z2Darray_ptr);
   p2Darray_orig_ptr=new twoDarray(*p2Darray_ptr);
}

// ==========================================================================
// Ground extraction member functions:
// ==========================================================================

// Member function subsample_zimage takes in some height image.  It
// outputs a new twoDarray containing a subsampled version of this
// height data with a pixel size set by const parameter
// min_pixel_length below.

twoDarray* ladarimage::subsample_zimage(twoDarray const *ztwoDarray_ptr)
{
   outputfunc::write_banner("Subsampling z data:");
//   const double min_pixel_length=1;	// meter
   const double min_pixel_length=1.5;	// meter
   int percentile_sentinel=1;	// median sampling
   twoDarray* zsubsampled_twoDarray_ptr=
      compositefunc::downsample_to_specified_resolution(
         min_pixel_length,ztwoDarray_ptr,percentile_sentinel);
   writeimage("subsample",zsubsampled_twoDarray_ptr);
   string subsampled_xyz_filenamestr=imagedir+"subsampled_"+xyz_filenamestr;
   xyzpfunc::write_xyzp_data(
      zsubsampled_twoDarray_ptr,zsubsampled_twoDarray_ptr,
      subsampled_xyz_filenamestr);
   return zsubsampled_twoDarray_ptr;
}

// ---------------------------------------------------------------------
// Member function estimate_ground_surface_using_gradient_info takes
// in a height image within *ztwoDarray_ptr which we assume has
// already been median filled and has had snowflakes removed.  This
// method iteratively computes the image's gradient field and erodes
// away locally tall pixels within the image using gradient magnitude
// and direction angle information.  At the end of the iterative loop,
// *ztwoDarray_ptr contains a spatially noisy estimate for the ground
// surface.

void ladarimage::estimate_ground_surface_using_gradient_info(
   double spatial_resolution,twoDarray* const xderiv_twoDarray_ptr,
   twoDarray* const yderiv_twoDarray_ptr,twoDarray* ztwoDarray_ptr)

{
   bool mask_points_near_border=false;
   twoDarray* mask_twoDarray_ptr=xderiv_twoDarray_ptr;
   estimate_ground_surface_using_gradient_info(
      spatial_resolution,xderiv_twoDarray_ptr,yderiv_twoDarray_ptr,
      ztwoDarray_ptr,mask_points_near_border,mask_twoDarray_ptr);
}

void ladarimage::estimate_ground_surface_using_gradient_info(
   double spatial_resolution,twoDarray* const xderiv_twoDarray_ptr,
   twoDarray* const yderiv_twoDarray_ptr,twoDarray* ztwoDarray_ptr,
   bool mask_points_near_border,twoDarray const *mask_twoDarray_ptr)
{

   outputfunc::write_banner(
      "Estimating ground surface using gradient info:");

// Before performing any destructive ground extraction computation,
// first create a copy of *ztwoDarray_ptr contents:

   twoDarray* zmedian_filled_twoDarray_ptr=new twoDarray(*ztwoDarray_ptr);

   const double grad_mag_min_threshold=5;	 // Almost certainly needs
// to be revised after Jan 04 modification to compute x y partial derivs...

//   const double grad_mag_min_threshold=5*ztwoDarray_ptr->get_deltax();

   int max_iters=1+basic_math::round(0.3/ztwoDarray_ptr->get_deltax())*10;
//   int max_iters=11;	// Try this value for Ft Devens IED data (8/12)
//   int max_iters=21;	// Use this value for Lowell data
   int iter=0;
   double min_distance_to_border=3;	// meters
   bool strong_gradient_region_found=false;
   do
   {
      cout << "Iteration number = " << iter << endl;
      ladarfunc::compute_x_y_deriv_fields(
         spatial_resolution,data_bbox_ptr,ztwoDarray_ptr,
         xderiv_twoDarray_ptr,yderiv_twoDarray_ptr,min_distance_to_border,
         xyzpfunc::null_value);

//      string xderiv_filename=imagedir+"xderiv.xyzp";
//      string yderiv_filename=imagedir+"yderiv.xyzp";
//      xyzpfunc::write_xyzp_data(
//         ztwoDarray_ptr,xderiv_twoDarray_ptr,xderiv_filename);
//      xyzpfunc::write_xyzp_data(
//         ztwoDarray_ptr,yderiv_twoDarray_ptr,yderiv_filename);

      double min_gradmag_threshold=-0.1;
      double max_gradmag_threshold=6000;
      compute_gradient_magnitude_field(
         xderiv_twoDarray_ptr,yderiv_twoDarray_ptr);
      
//      string gradient_mag_filename=imagedir+"grad_mag.xyzp";
//      xyzpfunc::write_xyzp_data(
//         ztwoDarray_ptr,gradient_mag_twoDarray_ptr,gradient_mag_filename);

//      char junkchar;
//      cout << "Enter any char to continue:" << endl;
//      cin >> junkchar;
      
      compute_gradient_direction_field(
         min_gradmag_threshold,max_gradmag_threshold,
         xderiv_twoDarray_ptr,yderiv_twoDarray_ptr,
         gradient_mag_twoDarray_ptr);

      if (mask_points_near_border)
      {
         recursivefunc::binary_null(
            0.5*xyzpfunc::null_value,gradient_mag_twoDarray_ptr,
            mask_twoDarray_ptr,xyzpfunc::null_value);
         recursivefunc::binary_null(
            0.5*xyzpfunc::null_value,gradient_phase_twoDarray_ptr,
            mask_twoDarray_ptr,xyzpfunc::null_value);
      }

      strong_gradient_region_found=groundfunc::erode_strong_gradient_regions(
         grad_mag_min_threshold,ztwoDarray_ptr,
         gradient_mag_twoDarray_ptr,gradient_phase_twoDarray_ptr);
      if (iter%5==0)
      {
//         writeimage("erode"+stringfunc::number_to_string(iter),ztwoDarray_ptr);
         string erode_filename=imagedir+"erode_"
            +stringfunc::number_to_string(iter)+".xyzp";
//         xyzpfunc::write_xyzp_data(
//            ztwoDarray_ptr,gradient_phase_twoDarray_ptr,erode_filename);
//         draw3Dfunc::append_fake_z_points_in_twoDarray_middle(
//            ztwoDarray_ptr,erode_filename);
      }
      iter++;
   }
   while (iter < max_iters && strong_gradient_region_found);

// Subtract flattened height image from original one.  Set to null any
// points in flattened image which differ by more than some small
// height (e.g. 2.5 meters) from original image.  Result should be
// silhouettes in place of buildings and trees.

   const double height_threshold=2.5;
   for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
   {
      for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
      {
         double curr_diff=zmedian_filled_twoDarray_ptr->get(px,py)-
            ztwoDarray_ptr->get(px,py);
         if (curr_diff > height_threshold) 
         {
            ztwoDarray_ptr->put(px,py,xyzpfunc::null_value);
         }
      }
   }
   delete zmedian_filled_twoDarray_ptr;

   string ground_filenamestr=imagedir+"ground_silhouette.xyzp";
   xyzpfunc::write_xyzp_data(
      ztwoDarray_ptr,p2Darray_ptr,ground_filenamestr);
   draw3Dfunc::append_fake_z_points_in_twoDarray_middle(
      ztwoDarray_ptr,ground_filenamestr);
}

// ---------------------------------------------------------------------
// Member function interpolate_and_flatten_ground_surface performs
// multiple rounds of ground extraction and filtering.  It returns a
// dynamically generated height image in which slow terrain variations
// have been removed as best as possible.

twoDarray* ladarimage::interpolate_and_flatten_ground_surface(
   twoDarray const *ztwoDarray_ptr,
   twoDarray const *zground_silhouetted_twoDarray_ptr,
   twoDarray const *ptwoDarray_ptr,
   double correlation_distance,double local_threshold_frac)
{
   outputfunc::write_banner("Extracting & filtering ground surface:");

// Compute ground surface via differential thresholding:

//   double local_threshold_frac=0.5;
   
   twoDarray* zground_twoDarray_ptr=groundfunc::extract_ground(
      xextent,yextent,correlation_distance,data_bbox_ptr,
      local_threshold_frac,zground_silhouetted_twoDarray_ptr);

// Remove any filled in regions within zground which correspond to
// empty data regions within the median filled ztwoDarray_ptr image:

   recursivefunc::binary_null(
      0.5*xyzpfunc::null_value,zground_twoDarray_ptr,
      ztwoDarray_ptr,xyzpfunc::null_value);

//   writeimage("ground",zground_twoDarray_ptr);
   string ground_filename=imagedir+"ground_"+xyz_filenamestr;
   xyzpfunc::write_xyzp_data(
      zground_twoDarray_ptr,ptwoDarray_ptr,ground_filename);
   draw3Dfunc::append_fake_z_points_in_twoDarray_middle(
      ztwoDarray_ptr,ground_filename);

// Subtract final ground surface from median filled z-image:

   cout << "Before subtracting ground surface from median filled zimage"
        << endl;
   twoDarray* zlevel_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
   compositefunc::average_identically_sized_twoDarrays(
      1,ztwoDarray_ptr,-1,zground_twoDarray_ptr,
      zlevel_twoDarray_ptr,xyzpfunc::null_value);

// Write out leveled z-image and extracted ground height information
// to Group 94 binary xyzp files:

   string filtered_and_flattened_filename=
      imagedir+"filtered_flattened_"+xyz_filenamestr;
   xyzpfunc::write_xyzp_data(
      zlevel_twoDarray_ptr,ptwoDarray_ptr,
      filtered_and_flattened_filename);
   draw3Dfunc::append_fake_z_points_in_twoDarray_middle(
      ztwoDarray_ptr,ground_filename);
//   writeimage("flattened_zimage",zlevel_twoDarray_ptr);
   delete zground_twoDarray_ptr;
   
   return zlevel_twoDarray_ptr;
}

// ==========================================================================
// Gradient field computation member functions:
// ==========================================================================

// Member function compute_gradient_magnitude_field then returns the
// gradient field's magnitude and direction angle within member
// twoDarray *gradient_mag_twoDarray_ptr.

void ladarimage::compute_gradient_magnitude_field(
   twoDarray const *xderiv_twoDarray_ptr,
   twoDarray const *yderiv_twoDarray_ptr)
{
   delete gradient_mag_twoDarray_ptr;
   gradient_mag_twoDarray_ptr=new twoDarray(xderiv_twoDarray_ptr);
   imagefunc::compute_gradient_magnitude_field(
      xderiv_twoDarray_ptr,yderiv_twoDarray_ptr,gradient_mag_twoDarray_ptr);

//    writeimage("grad_mag_orig",gradient_mag_twoDarray_ptr);
   
//   string grad_orig_filename=imagedir+"grad_orig_"+xyz_filenamestr;
//   xyzpfunc::write_xyzp_data(
//      z2Darray_ptr,gradient_mag_twoDarray_ptr,grad_orig_filename,false);

// Threshold away spurious high intensity and low intensity values
// within gradient image:

//   double zmag_min_threshold=-0.1;
//   double zmag_max_threshold=6000;

/*
//   double zmag_min_threshold=0.1;
//   double zmag_min_threshold=3.5;
//   double zmag_min_threshold=4.25;
double zmag_min_threshold=5;
double zmag_max_threshold=65;
//   double zmag_min_threshold=5*xderiv_twoDarray_ptr->get_deltax();
//   double zmag_max_threshold=65*yderiv_twoDarray_ptr->get_deltay();
imagefunc::threshold_intensities_above_cutoff(
gradient_mag_twoDarray_ptr,zmag_max_threshold,xyzpfunc::null_value);
imagefunc::threshold_intensities_below_cutoff_frac(
gradient_mag_twoDarray_ptr,0.5);
//   writeimage("grad_mag_thresh",gradient_mag_twoDarray_ptr);  

// Recursively empty gradient magnitude image of small noise islands:

recursivefunc::recursive_empty(3,gradient_mag_twoDarray_ptr,false);
//   writeimage("gradient_mag",gradient_mag_twoDarray_ptr);
*/
}

// ---------------------------------------------------------------------
// Member function compute_gradient_direction_field takes in partial
// derivative information within *xderiv_twoDarray_ptr and
// *yderiv_twoDarray_ptr as well as gradient magnitude field
// *grad_mag_twoDarray_ptr.  At those points where the gradient
// magnitude lies within the interval
// [min_gradmag_threshold,max_gradmag_threshold], this method computes
// the gradient's direction angle.  The result is returned within the
// dynamically generated member *gradient_phase_twoDarray_ptr.

void ladarimage::compute_gradient_direction_field(
   double min_gradmag_threshold,double max_gradmag_threshold,
   twoDarray const *xderiv_twoDarray_ptr,
   twoDarray const *yderiv_twoDarray_ptr,twoDarray* grad_mag_twoDarray_ptr)
{
   outputfunc::write_banner("Computing gradient direction field:");
      
   delete gradient_phase_twoDarray_ptr;
   gradient_phase_twoDarray_ptr=new twoDarray(xderiv_twoDarray_ptr);
   imagefunc::compute_gradient_phase_field(
      min_gradmag_threshold,max_gradmag_threshold,
      xderiv_twoDarray_ptr,yderiv_twoDarray_ptr,
      gradient_mag_twoDarray_ptr,gradient_phase_twoDarray_ptr);

//   string grad_phase_filename=imagedir+"grad_phase_"+xyz_filenamestr;
//   xyzpfunc::write_xyzp_data(
//      z2Darray_ptr,gradient_phase_twoDarray_ptr,grad_phase_filename,false);

// Recursively empty gradient phase image of small noise islands:

//   double min_phase=-2.1*PI;
//   recursivefunc::recursive_empty(
//      5,min_phase,gradient_phase_twoDarray_ptr,false,xyzpfunc::null_value);
//   writeimage("gradient_phase",gradient_phase_twoDarray_ptr,false,
//              ladarimage::phase_data);

//   grad_phase_filename=imagedir+"grad_phase_clean"+xyz_filenamestr;
//   xyzpfunc::write_xyzp_data(
//      z2Darray_ptr,gradient_phase_twoDarray_ptr,grad_phase_filename,false);
}

// ---------------------------------------------------------------------
// Member function compute_surface_normal_field calculates and stores
// surface normal unit vectors within the *normal_twoDarray_ptr member
// object:

void ladarimage::compute_surface_normal_field(
   twoDarray const *xderiv_twoDarray_ptr,
   twoDarray const *yderiv_twoDarray_ptr)
{
   outputfunc::write_banner("Computing surface normal field:");

   delete normal_twoDarray_ptr;
   normal_twoDarray_ptr=new TwoDarray<threevector>(
      xderiv_twoDarray_ptr->get_mdim(),xderiv_twoDarray_ptr->get_ndim());
   xderiv_twoDarray_ptr->copy_metric_data(normal_twoDarray_ptr);
   
   twoDarray* polarangle_twoDarray_ptr=new twoDarray(xderiv_twoDarray_ptr);
   for (unsigned int px=0; px<xderiv_twoDarray_ptr->get_mdim(); px++)
   {
      for (unsigned int py=0; py<xderiv_twoDarray_ptr->get_ndim(); py++)
      {
         double xderiv=xderiv_twoDarray_ptr->get(px,py);
         double yderiv=yderiv_twoDarray_ptr->get(px,py);
         if (xderiv > xyzpfunc::null_value && yderiv > xyzpfunc::null_value)
         {
            threevector curr_normal(-xderiv_twoDarray_ptr->get(px,py),
                                    -yderiv_twoDarray_ptr->get(px,py),1);
            normal_twoDarray_ptr->put(px,py,curr_normal);
//            normal_twoDarray_ptr->put(px,py,curr_normal.unitvector());
         }
         else
         {
            normal_twoDarray_ptr->put(px,py,Zero_vector);
         }

         if (normal_twoDarray_ptr->get(px,py).magnitude() > 10000)
         {
            cout << "Trouble !!  px = " << px << " py = " << py
                 << " normal magnitude = "
                 << normal_twoDarray_ptr->get(px,py).magnitude() << endl;
            cout << "xderiv = " << xderiv << " yderiv = " << yderiv << endl;
         }
         

         double polar_dotproduct=z_hat.dot(normal_twoDarray_ptr->get(px,py));
         polarangle_twoDarray_ptr->put(px,py,180.0/PI*acos(polar_dotproduct));
      } // loop over py index
   } // loop over px index

//   dynamic_colortable=true;
//   dynamic_colortable_minz=0;
//   dynamic_colortable_maxz=90;
//   writeimage("normal_polarangle",polarangle_twoDarray_ptr);
//   dynamic_colortable=false;

   delete polarangle_twoDarray_ptr;
}

// ---------------------------------------------------------------------
// This overloaded version of member function
// compute_surface_normal_field attempts to compute surface normal
// information using small pixel value differences rather than x and y
// partial derivatives obtained from smooth kernels.  In late January
// 04, we concluded that this method's approach is hopeless
// impractical, for it is far too slow...

void ladarimage::compute_surface_normal_field(twoDarray const *ztwoDarray_ptr)
{
   outputfunc::write_banner("Computing surface normal field:");

   delete normal_twoDarray_ptr;
   normal_twoDarray_ptr=new TwoDarray<threevector>(
      ztwoDarray_ptr->get_mdim(),ztwoDarray_ptr->get_ndim());
   ztwoDarray_ptr->copy_metric_data(normal_twoDarray_ptr);
   
   double deltax=ztwoDarray_ptr->get_deltax();
   double deltay=ztwoDarray_ptr->get_deltay();

   int nsize=2;
   int ndim=sqr(2*nsize+1);
   double x[ndim],y[ndim],z[ndim];
   twoDarray* polarangle_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
   polarangle_twoDarray_ptr->initialize_values(xyzpfunc::null_value);
   twoDarray* azangle_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
   azangle_twoDarray_ptr->initialize_values(xyzpfunc::null_value);
   twoDarray* residual_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
   residual_twoDarray_ptr->initialize_values(xyzpfunc::null_value);
   twoDarray* nplanar_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
   nplanar_twoDarray_ptr->initialize_values(xyzpfunc::null_value);
   
   double min_z_value=2.5;	 // meters
   for (unsigned int px=nsize; px<ztwoDarray_ptr->get_mdim()-nsize; px++)
   {
      if (px%10==0)
      {
         cout << "px = " << px 
              << " px_max = " << ztwoDarray_ptr->get_mdim()-nsize << endl;
      }
      
      for (unsigned int py=nsize; py<ztwoDarray_ptr->get_ndim()-nsize; py++)
      {
         double curr_z=ztwoDarray_ptr->get(px,py);
         if (curr_z > xyzpfunc::null_value)
         {

            double curr_x,curr_y;
            ztwoDarray_ptr->pixel_to_point(px,py,curr_x,curr_y);

// Fit plane to current pixel and its ndim x ndim -1 nearest neighbors:

            int pixel_counter=0;
            for (int i=-nsize; i<=nsize; i++)
            {
               for (int j=-nsize; j<=nsize; j++)
               {
                  double neighbor_z=ztwoDarray_ptr->get(px+i,py+j);
//                  if (neighbor_z > xyzpfunc::null_value)
                  if (neighbor_z > min_z_value)
                  {
                     x[pixel_counter]=curr_x+i*deltax;
                     y[pixel_counter]=curr_y-j*deltay;
                     z[pixel_counter]=neighbor_z;
                     pixel_counter++;
                  }
               } // loop over j index
            } // loop over i index
            if (pixel_counter > 2)
            {
               double a,b,c;
               mathfunc::fit_plane(pixel_counter,x,y,z,a,b,c);
               double denom=sqrt(1+sqr(a)+sqr(b));
               normal_twoDarray_ptr->put(
                  px,py,threevector(-a/denom,-b/denom,1.0/denom));
//               polarangle_twoDarray_ptr->put(px,py,180/PI*acos(1.0/denom));
//               double azangle=mathfunc::phase_to_canonical_interval(
//                  true,atan2(-b,-a)*180/PI,-180,180);
//               azangle_twoDarray_ptr->put(px,py,azangle);

// Compute residual metric as a measure of local planarity:
               
               double sqr_sum=0;
               for (int p=0; p<pixel_counter; p++)
               {
                  sqr_sum += sqr(z[p]-(a*x[p]+b*y[p]+c));
               }
//               double residual=sqrt(sqr_sum/double(pixel_counter));
               double residual=sqr_sum/double(pixel_counter);
               residual_twoDarray_ptr->put(px,py,residual);

// Compute integrated residual metric for candidate rooftop detection
// measure:

               int n_planar_pixels=0;
               double zmin=2.5;	 // meters
               double dmin=0.1;	 // meter
               double sqr_dmin=sqr(dmin);
               if (curr_z > zmin)
               {
//                  threevector nhat(normal_twoDarray_ptr->get(px,py));
                  for (unsigned int qx=
                          (unsigned int) basic_math::max(int(px-50),0); 
                       qx<basic_math::min(px+50,ztwoDarray_ptr->get_mdim()); 
                       qx++)
                  {
                     for (unsigned int qy=
                             (unsigned int) basic_math::max(int(py-50),0); 
                          qy<basic_math::min(py+50,ztwoDarray_ptr->get_ndim());
                          qy++)
                     {
                        double testpnt_z=ztwoDarray_ptr->get(qx,qy);     
                        if (testpnt_z > zmin)
                        {
                           double testpnt_x,testpnt_y;
                           ztwoDarray_ptr->pixel_to_point(
                              qx,qy,testpnt_x,testpnt_y);
                           double sqr_zdiff=sqr(
                              testpnt_z-(a*testpnt_x+b*testpnt_y+c));
                           if (sqr_zdiff < sqr_dmin)
                           {
                              n_planar_pixels++;
                           }
                        } // testpnt_z > zmin conditional
                     } // qy loop
                  } // qx loop
                  nplanar_twoDarray_ptr->put(px,py,n_planar_pixels);
               } // curr_z > zmin conditional
            }
            else
            {
               normal_twoDarray_ptr->put(px,py,Zero_vector);
               polarangle_twoDarray_ptr->put(px,py,xyzpfunc::null_value);  
               azangle_twoDarray_ptr->put(px,py,xyzpfunc::null_value);      
               residual_twoDarray_ptr->put(px,py,xyzpfunc::null_value);      
               nplanar_twoDarray_ptr->put(px,py,xyzpfunc::null_value);      
            }
         }
         else
         {
            normal_twoDarray_ptr->put(px,py,Zero_vector);
            polarangle_twoDarray_ptr->put(px,py,xyzpfunc::null_value);      
            azangle_twoDarray_ptr->put(px,py,xyzpfunc::null_value);      
            residual_twoDarray_ptr->put(px,py,xyzpfunc::null_value);      
            nplanar_twoDarray_ptr->put(px,py,xyzpfunc::null_value);      
         }
      } // loop over py index
   } // loop over px index

   string nplanar_filename=imagedir+"nplanar_"+xyz_filenamestr;
   xyzpfunc::write_xyzp_data(
      ztwoDarray_ptr,nplanar_twoDarray_ptr,nplanar_filename,false);

/*
  dynamic_colortable=true;
  dynamic_colortable_minz=0;
  dynamic_colortable_maxz=100;
  writeimage("nplanar",nplanar_twoDarray_ptr);
  dynamic_colortable=false;
*/

/*
  string az_filename=imagedir+"normal_az_"+xyz_filenamestr;
  xyzpfunc::write_xyzp_data(
  ztwoDarray_ptr,azangle_twoDarray_ptr,az_filename,false);
*/

/*
  dynamic_colortable=true;
  dynamic_colortable_minz=-180;
  dynamic_colortable_maxz=180;
  writeimage("normal_az",azangle_twoDarray_ptr);
  dynamic_colortable=false;
*/

   string residual_filename=imagedir+"residual_"+xyz_filenamestr;
   xyzpfunc::write_xyzp_data(
      ztwoDarray_ptr,residual_twoDarray_ptr,residual_filename,false);

/*   
     dynamic_colortable=true;
     dynamic_colortable_minz=0;
     dynamic_colortable_maxz=1;
     writeimage("planar_residual",residual_twoDarray_ptr);
     dynamic_colortable=false;
*/

/*
  string polarangle_filename=imagedir+"polarangle_"+xyz_filenamestr;
  xyzpfunc::write_xyzp_data(
  ztwoDarray_ptr,polarangle_twoDarray_ptr,polarangle_filename,false);
*/

/*
  dynamic_colortable=true;
  dynamic_colortable_minz=0;
  dynamic_colortable_maxz=90;
  writeimage("normal_polarangle",polarangle_twoDarray_ptr);
  dynamic_colortable=false;
*/

   delete polarangle_twoDarray_ptr;
   delete azangle_twoDarray_ptr;
   delete residual_twoDarray_ptr;
   delete nplanar_twoDarray_ptr;
}

// ---------------------------------------------------------------------
// Member functions compute_gradient_mags_at_mask_locations and
// compute_gradient_phases_at_mask_locationis take in twoDarray
// *ztwoDarray_ptr along with a mask twoDarray *zmask_twoDarray_ptr.
// These methods return dynamically generated twoDarrays which contain
// gradient information only at those pixel locations where the mask
// function exceeds xyzpfunc::null_value.  These methods allow us to
// compute gradient information at high spatial resolution only at
// those points in a cleaned, lower resolution phase gradient field in
// order to search for randomness associated with trees.

twoDarray* ladarimage::compute_gradient_mags_at_mask_locations(
   double spatial_resolution,twoDarray const *ztwoDarray_ptr,
   twoDarray const *zmask_twoDarray_ptr)
{
   outputfunc::write_banner("Computing gradient mags at masked locations:");
   double min_distance_to_border=10;	// meters
   twoDarray* xderiv_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
   twoDarray* yderiv_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
   ladarfunc::compute_x_y_deriv_fields(
      spatial_resolution,data_bbox_ptr,ztwoDarray_ptr,
      xderiv_twoDarray_ptr,yderiv_twoDarray_ptr,
      min_distance_to_border,xyzpfunc::null_value);

   twoDarray* zgradient_mag_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
   compositefunc::combine_identically_sized_twoDarrays_in_quadrature(
      xderiv_twoDarray_ptr,yderiv_twoDarray_ptr,
      zgradient_mag_twoDarray_ptr);
   delete xderiv_twoDarray_ptr;
   delete yderiv_twoDarray_ptr;

   recursivefunc::binary_null(
      0.5*xyzpfunc::null_value,zgradient_mag_twoDarray_ptr,
      zmask_twoDarray_ptr,xyzpfunc::null_value);
   return zgradient_mag_twoDarray_ptr;
}

// ---------------------------------------------------------------------
twoDarray* ladarimage::compute_gradient_phases_at_mask_locations(
   double spatial_resolution,twoDarray const *ztwoDarray_ptr,
   twoDarray const *zmask_twoDarray_ptr)
{
   outputfunc::write_banner("Computing gradient phases at masked locations:");
   cout << "Spatial resolution = " << spatial_resolution << endl;
   double min_distance_to_border=10;	// meters
   twoDarray* xderiv_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
   twoDarray* yderiv_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
   ladarfunc::compute_x_y_deriv_fields(
      spatial_resolution,data_bbox_ptr,ztwoDarray_ptr,
      xderiv_twoDarray_ptr,yderiv_twoDarray_ptr,
      min_distance_to_border,xyzpfunc::null_value);

   double zmask_threshold=0.5*xyzpfunc::null_value;
   twoDarray* zgradient_phase_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
   imagefunc::compute_gradient_phase_field(
      zmask_threshold,POSITIVEINFINITY,
      xderiv_twoDarray_ptr,yderiv_twoDarray_ptr,
      zmask_twoDarray_ptr,zgradient_phase_twoDarray_ptr);
   delete xderiv_twoDarray_ptr;
   delete yderiv_twoDarray_ptr;
   return zgradient_phase_twoDarray_ptr;
}

// ---------------------------------------------------------------------
// Member function connect_phase_components generates a linked list of
// horizontal pixel runs for the binary thresholded version of
// *gradient_phase_twoDarray_ptr.  It subsequently converts this pixel
// run information into a hashtable whose entries correspond to linked
// pixel lists for each connected component within the phase field.
// This method generates a metafile plot of the connected phase
// components which are colored differently for visualization
// purposes.

twoDarray* ladarimage::connect_phase_components(
   double min_projected_area,bool plot_connected_phase_components)
{
   outputfunc::write_banner("Connecting phase components:");
   twoDarray* zbinary_twoDarray_ptr=new twoDarray(
      gradient_phase_twoDarray_ptr);
   binaryimagefunc::binary_threshold(
      -7,gradient_phase_twoDarray_ptr,zbinary_twoDarray_ptr);

   linkedlist* run_list_ptr=
      connectfunc::run_length_encode(zbinary_twoDarray_ptr);
   int n_connected_runs=connectfunc::connect_runs(
      run_list_ptr,zbinary_twoDarray_ptr);
   delete zbinary_twoDarray_ptr;
   outputfunc::newline();
   cout << "n_connected_runs = " << n_connected_runs << endl;
   outputfunc::newline();

   const double dA=gradient_phase_twoDarray_ptr->get_deltax()*
      gradient_phase_twoDarray_ptr->get_deltay();	// bin area on ground
   const unsigned int min_component_pixels=
      basic_math::round(min_projected_area/dA);
   connected_gradient_phases_hashtable_ptr=
      connectfunc::generate_connected_hashtable(
         n_connected_runs,min_component_pixels,run_list_ptr,
         gradient_phase_twoDarray_ptr);
   delete run_list_ptr;

// Plot connected components of gradient phase field:

   twoDarray* zconnected_phase_twoDarray_ptr=new twoDarray(
      gradient_phase_twoDarray_ptr);
   connectfunc::decode_connected_hashtable(
      connected_gradient_phases_hashtable_ptr,
      zconnected_phase_twoDarray_ptr);
   if (plot_connected_phase_components)
      writeimage("connected_phase",zconnected_phase_twoDarray_ptr);
   return zconnected_phase_twoDarray_ptr;
}

// ---------------------------------------------------------------------
// Member function connected_component_phase_gradient_distribution
// computes a probability density for each connected component's
// gradient direction angles.  We expect buildings to have phase
// distributions which are peaked at 4 or more angles that are roughly
// 90 degrees apart.  In theory, trees should exhibit more random
// looking phase angle distributions.  But as of early Sept 03, we
// have been surprised to find that tree clusters often yield phase
// distributions that have quite a bit of order in them...

void ladarimage::connected_component_phase_gradient_distribution()
{
   outputfunc::write_banner(
      "Computing connected component phase gradient distribution");

   for (unsigned int n=0; n<connected_gradient_phases_hashtable_ptr->
           size(); n++)
   {
      linkedlist* currlist_ptr=
         connected_gradient_phases_hashtable_ptr->retrieve_key(n)->
         get_data();

      if (currlist_ptr != NULL)
      {
         int n_currpixel=0;
         double phase[currlist_ptr->size()];
         mynode* curr_pixel_ptr=currlist_ptr->get_start_ptr();
         while (curr_pixel_ptr != NULL)
         {
            unsigned int px=basic_math::round(
               curr_pixel_ptr->get_data().get_var(0));
            unsigned int py=basic_math::round(
               curr_pixel_ptr->get_data().get_var(1));
            phase[n_currpixel++]=gradient_phase_twoDarray_ptr->get(px,py);
            curr_pixel_ptr=curr_pixel_ptr->get_nextptr();
         }

         if (n_currpixel > 1000)
         {
            prob_distribution prob(
               -PI,PI,currlist_ptr->size(),phase,720);
            prob.set_densityfilenamestr(
               imagedir+"phase_density"+stringfunc::number_to_string(n)
               +".meta");
            prob.set_xlabel("Phase");
            prob.write_density_dist();
         }
      } // currlist_ptr != NULL conditional
   } // loop over n labeling connected components
}

// ---------------------------------------------------------------------
// Member function compute_laplacian_field takes in a cleaned height
// image within twoDarray *ztwoDarray_ptr.  This method computes and
// plots the laplacian of the height function.

void ladarimage::compute_laplacian_field(twoDarray const *ztwoDarray_ptr)
{
   outputfunc::write_banner("Computing laplacian field:");
   double spatial_resolution=0.7;	// meter
//   double spatial_resolution=0.3;	// meter
   zlaplacian_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
   imagefunc::laplacian_field(
      spatial_resolution,ztwoDarray_ptr,zlaplacian_twoDarray_ptr);

   dynamic_colortable=true;
   dynamic_colortable_minz=0;
   dynamic_colortable_maxz=2;
//   writeimage("zlaplacian",zlaplacian_twoDarray_ptr);
   dynamic_colortable=false;
}
   
// ---------------------------------------------------------------------
// Member function intersect_gradient_and_laplacian_fields starts with
// the connected components of the gradient magnitude field.  This
// method retains those connected gradient components which have a
// non-negligible overlap (as defined by the min_overlap_frac
// parameter below) with the laplacian field of a cleaned height
// image.

void ladarimage::intersect_gradient_and_laplacian_fields(
   double laplacian_threshold)
{
   outputfunc::write_banner("Intersecting gradient & laplacian fields:");

   cout << "Before intersection, nconnected_components = "
        << connected_gradient_phases_hashtable_ptr->size() 
        << endl;

   int n_connected_overlap=0;
   for (unsigned int n=0; n<connected_gradient_phases_hashtable_ptr->
           size(); n++)
   {
      cout << "n = " << n << endl;
      linkedlist* currlist_ptr=
         connected_gradient_phases_hashtable_ptr->retrieve_key(n)->
         get_data();

      if (currlist_ptr != NULL)
      {
         int npixels_in_curr_component=0;
         int nhot_laplacian_pixels=0;
         mynode* curr_pixel_ptr=currlist_ptr->get_start_ptr();
         while (curr_pixel_ptr != NULL)
         {
            unsigned int px=basic_math::round(
               curr_pixel_ptr->get_data().get_var(0));
            unsigned int py=basic_math::round(
               curr_pixel_ptr->get_data().get_var(1));
            if (zlaplacian_twoDarray_ptr->get(px,py) > laplacian_threshold)
               nhot_laplacian_pixels++;
            npixels_in_curr_component++;
            curr_pixel_ptr=curr_pixel_ptr->get_nextptr();
         }

         double hot_laplacian_frac=0;
         if (npixels_in_curr_component > 0)
         {
            hot_laplacian_frac=double(nhot_laplacian_pixels)/
               double(npixels_in_curr_component);
         }
         cout << "hot_laplacian_frac = " << hot_laplacian_frac << endl;

         const double min_overlap_frac=0.5;
         if (hot_laplacian_frac < min_overlap_frac)
         {
            delete currlist_ptr;
            connected_gradient_phases_hashtable_ptr->retrieve_key(n)->
               set_data(NULL);
         }
         else
         {
            n_connected_overlap++;
            cout << "component=" << n 
                 << " nhot pixels=" << nhot_laplacian_pixels
                 << " npixels=" << npixels_in_curr_component
                 << " f=" << hot_laplacian_frac << endl;
         }
      } // currlist_ptr != NULL conditional
   } // loop over n labeling connected components
   cout << "After intersection, nconnected_components = "
        << n_connected_overlap << endl;

// Plot surviving connected components of gradient magnitude field:

   twoDarray* zconnected_twoDarray_ptr=new twoDarray(
      gradient_mag_twoDarray_ptr);
   connectfunc::decode_connected_hashtable(
      connected_gradient_phases_hashtable_ptr,zconnected_twoDarray_ptr);
//   writeimage("intersected_laplace_grads",zconnected_twoDarray_ptr);
   delete zconnected_twoDarray_ptr;
}

// ---------------------------------------------------------------------
// Member function intersect_height_grad_components 

void ladarimage::intersect_height_grad_components(
   double height_threshold,twoDarray const *ztwoDarray_ptr)
{
   outputfunc::write_banner("Intersecting height & gradient components:");

   int n_intersected_components=0;
   for (unsigned int n=0; n<connected_gradient_phases_hashtable_ptr->
           size(); n++)
   {
      cout << "n = " << n << endl;
      linkedlist* currlist_ptr=
         connected_gradient_phases_hashtable_ptr->retrieve_key(n)->
         get_data();

      if (currlist_ptr != NULL)
      {
         int npixels_in_curr_component=0;
         int nintersected_pixels=0;
         mynode* curr_pixel_ptr=currlist_ptr->get_start_ptr();
         while (curr_pixel_ptr != NULL)
         {
            unsigned int px=basic_math::round(
               curr_pixel_ptr->get_data().get_var(0));
            unsigned int py=basic_math::round(
               curr_pixel_ptr->get_data().get_var(1));
            if (ztwoDarray_ptr->get(px,py) > height_threshold)
               nintersected_pixels++;
            npixels_in_curr_component++;
            curr_pixel_ptr=curr_pixel_ptr->get_nextptr();
         }

         double intersection_frac=0;
         if (npixels_in_curr_component > 0)
         {
            intersection_frac=double(nintersected_pixels)/
               double(npixels_in_curr_component);
         }
         cout << "intersection_frac = " << intersection_frac << endl;

         const double min_overlap_frac=0.1;
         if (intersection_frac < min_overlap_frac)
         {
            delete currlist_ptr;
            connected_gradient_phases_hashtable_ptr->retrieve_key(n)->
               set_data(NULL);
//         connected_gradient_phases_hashtable_ptr->delete_key(n);
         }
         else
         {
            n_intersected_components++;
            cout << "component=" << n 
                 << " nintersected pixels=" << nintersected_pixels
                 << " npixels=" << npixels_in_curr_component
                 << " f=" << intersection_frac << endl;
         }
      } // currlist_ptr != NULL conditional
   } // loop over n labeling connected components
   cout << "After intersection, n_intersected_components = "
        << n_intersected_components << endl;

// Plot surviving connected components of gradient magnitude field:

   twoDarray* zconnected_twoDarray_ptr=new twoDarray(
      gradient_mag_twoDarray_ptr);
   connectfunc::decode_connected_hashtable(
      connected_gradient_phases_hashtable_ptr,zconnected_twoDarray_ptr);
//   writeimage("intersected_heights_grads",zconnected_twoDarray_ptr);
   delete zconnected_twoDarray_ptr;
}

// ---------------------------------------------------------------------
// Member function intersect_connected_heights_with_phase_grads counts
// the number of non-null pixels within the current gradient phase
// image which overlap each connected component within the heights
// hashtable.  If input boolean flag
// discard_components_if_difference_too_small==true, this method
// throws away any connected component whose fractional overlap with
// the gradient phase image excceds input parameter
// intersection_frac_threshold.  This capability is useful for
// eliminating clusters of pixels which correspond to tree clumps.  On
// the other hand, if discard_component_if_difference_too_small=false,
// the method discards any component whose fractional overal is less
// than intersection_frac_threshold.  This capability is useful for
// removing noise from the connected heights component hashtable.

twoDarray* ladarimage::intersect_connected_heights_with_phase_grads(
   twoDarray const *zcluster_twoDarray_ptr,double intersection_frac_threshold,
   bool discard_component_if_difference_too_small,
   bool plot_surviving_components)
{
   outputfunc::write_banner("Intersecting connected heights & phase grads:");

   for (unsigned int n=0; n<connected_heights_hashtable_ptr->size(); n++)
   {
      linkedlist* currlist_ptr=
         connected_heights_hashtable_ptr->retrieve_key(n)->get_data();

      if (currlist_ptr != NULL)
      {
         int npixels_in_curr_component=0;
         int nintersected_pixels=0;
         double x_sum=0;
         double y_sum=0;
         mynode* curr_pixel_ptr=currlist_ptr->get_start_ptr();
         while (curr_pixel_ptr != NULL)
         {
            unsigned int px=basic_math::round(
               curr_pixel_ptr->get_data().get_var(0));
            unsigned int py=basic_math::round(
               curr_pixel_ptr->get_data().get_var(1));
            double x,y;
            zcluster_twoDarray_ptr->pixel_to_point(px,py,x,y);
            x_sum += x;
            y_sum += y;
            if (gradient_phase_twoDarray_ptr->get(px,py) 
                > 0.5*xyzpfunc::null_value) nintersected_pixels++;
            npixels_in_curr_component++;
            curr_pixel_ptr=curr_pixel_ptr->get_nextptr();
         }
//         double xCOM=x_sum/npixels_in_curr_component;
//         double yCOM=y_sum/npixels_in_curr_component;
         double intersection_frac=double(nintersected_pixels)/
            double(npixels_in_curr_component);
//         cout << "Component number = " << n << endl;
//         cout << "xCOM = " << xCOM << " yCOM = " << yCOM << endl;
//         cout << "npixels = " << npixels_in_curr_component 
//              << " nintersections = " << nintersected_pixels 
//              << " frac = " << intersection_frac << endl;

         bool discard_component=false;
         if ((discard_component_if_difference_too_small &&
              intersection_frac > intersection_frac_threshold) ||
             (!discard_component_if_difference_too_small &&
              intersection_frac < intersection_frac_threshold))
         {
            discard_component=true;
         }
            
         if (discard_component)
         {
            delete currlist_ptr;
            connected_heights_hashtable_ptr->retrieve_key(n)->
               set_data(NULL);
         }
      }
   } // loop over index n labeling connected component number

// Plot surviving connected components:

   twoDarray* zsurviving_connected_twoDarray_ptr=
      new twoDarray(zcluster_twoDarray_ptr);
   connectfunc::decode_connected_hashtable(
      connected_heights_hashtable_ptr,zsurviving_connected_twoDarray_ptr);
   if (plot_surviving_components)
      writeimage("surviving_components",zsurviving_connected_twoDarray_ptr);
   return zsurviving_connected_twoDarray_ptr;
}

// ---------------------------------------------------------------------
// Member function intersect_binary_image_with_phase_grads takes in a
// binary heights or phase image.  This method retains within the
// gradient phase image only those pixels which have nonzero overlap
// with the binary image.  The result of this intersection is placed
// back into member twoDarray *gradient_phase_twoDarray_ptr.

void ladarimage::intersect_binary_image_with_phase_grads(
   twoDarray const *zbinary_twoDarray_ptr,
   bool retain_unity_valued_binary_pixels)
{
   outputfunc::write_banner("Intersecting binary image with grad images:");

   twoDarray* mag_intersection_twoDarray_ptr=new twoDarray(
      gradient_mag_twoDarray_ptr);
   twoDarray* phase_intersection_twoDarray_ptr=new twoDarray(
      gradient_phase_twoDarray_ptr);
   mag_intersection_twoDarray_ptr->initialize_values(xyzpfunc::null_value);
   phase_intersection_twoDarray_ptr->initialize_values(xyzpfunc::null_value);

   for (unsigned int px=0; px<zbinary_twoDarray_ptr->get_mdim(); px++)
   {
      for (unsigned int py=0; py<zbinary_twoDarray_ptr->get_ndim(); py++)
      {
         if (retain_unity_valued_binary_pixels)
         {
            if (zbinary_twoDarray_ptr->get(px,py) > 0.5)
            {
               mag_intersection_twoDarray_ptr->put(
                  px,py,gradient_mag_twoDarray_ptr->get(px,py));
               phase_intersection_twoDarray_ptr->put(
                  px,py,gradient_phase_twoDarray_ptr->get(px,py));
            }
         }
         else
         {
            if (zbinary_twoDarray_ptr->get(px,py) < 0.5)
            {
               mag_intersection_twoDarray_ptr->put(
                  px,py,gradient_mag_twoDarray_ptr->get(px,py));
               phase_intersection_twoDarray_ptr->put(
                  px,py,gradient_phase_twoDarray_ptr->get(px,py));
            }
            else
            {
               mag_intersection_twoDarray_ptr->put(px,py,POSITIVEINFINITY);
               phase_intersection_twoDarray_ptr->put(px,py,POSITIVEINFINITY);
            }
         }
      } // loop over py index
   } // loop over px index
   mag_intersection_twoDarray_ptr->copy(gradient_mag_twoDarray_ptr);
   phase_intersection_twoDarray_ptr->copy(gradient_phase_twoDarray_ptr);
   delete mag_intersection_twoDarray_ptr;
   delete phase_intersection_twoDarray_ptr;
}

// ==========================================================================
// Height and/or intensity fluctuations member functions:
// ==========================================================================

// Member function compute_z_fluctuations runs a window around the
// height image contained within input twoDarray *ztwoDarray_ptr.  The
// size of the window is an increasing function of height.  This
// method calculates the fluctuations in z values between the center
// pixel and its nearest neighbors.  It returns a dynamically
// generated twoDarray which contains this fluctuation information on
// a normalized scale between 0 and 1.

twoDarray* ladarimage::compute_z_fluctuations(twoDarray const *ztwoDarray_ptr)
{
   outputfunc::write_banner("Computing z fluctuations:");

   unsigned int nsize_max=10;
   twoDarray* zfluctuation_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
   twoDarray* normalized_fluctuation_twoDarray_ptr=
      new twoDarray(ztwoDarray_ptr);
   zfluctuation_twoDarray_ptr->initialize_values(xyzpfunc::null_value);
   normalized_fluctuation_twoDarray_ptr->initialize_values(
      xyzpfunc::null_value);
   
   int mdim=ztwoDarray_ptr->get_mdim();
   int ndim=ztwoDarray_ptr->get_ndim();
   const double zmin=2.5;	 // meters
   double max_zfluc=NEGATIVEINFINITY;
   double abs_zdiff[sqr(2*nsize_max+1)];
   for (unsigned int px=nsize_max; px<mdim-nsize_max; px++)
   {
      for (unsigned int py=nsize_max; py<ndim-nsize_max; py++)
      {
         int n_eff=ndim*px+py;
         double curr_z=ztwoDarray_ptr->get(n_eff);
         if (curr_z > zmin)
         {
            int nsize=0;
            if (curr_z < 5)
            {
               nsize=2;
            }
            else if (curr_z < 10)
            {
               nsize=4;
            }
            else if (curr_z < 15)
            {
               nsize=6;
            }
            else
            {
               nsize=8;
            }
            
// Compute variance in fractional delta-z values between current pixel
// and its (2*nsize+1)**2 -1 nearest neighbors:

            int pixel_counter=0;
            for (int i=-nsize; i<=nsize; i++)
            {
               for (int j=-nsize; j<=nsize; j++)
               {
                  double neighbor_z=ztwoDarray_ptr->get((px+i)*ndim+(py+j));
                  if (neighbor_z > zmin && !(i==0 && j==0))
                  {
                     abs_zdiff[pixel_counter]=sqr(neighbor_z-curr_z);
                     pixel_counter++;
                  }
               } // loop over j index
            } // loop over i index
            Quicksort(abs_zdiff,pixel_counter);
            double zfluc=
               sqrt(abs_zdiff[basic_math::round(0.97*pixel_counter)]);
            zfluctuation_twoDarray_ptr->put(n_eff,zfluc);

            double znorm=4;	// meters
            double normalized_fluctuation=basic_math::min(1.0,zfluc/znorm);
            normalized_fluctuation_twoDarray_ptr->put(
               n_eff,normalized_fluctuation);
            max_zfluc=basic_math::max(max_zfluc,zfluc);
         }
      } // py loop
   } // px loop

   cout << "max_zfluctuation = " << max_zfluc << endl;

//   string zfluc_filenamestr=imagedir+"zfluc_"+xyz_filenamestr;
//   xyzpfunc::write_xyzp_data(
//      ztwoDarray_ptr,zfluctuation_twoDarray_ptr,zfluc_filenamestr,false);

//   dynamic_colortable=true;
//   dynamic_colortable_minz=0;
//   dynamic_colortable_maxz=3.0;
//   writeimage("zfluctuations",zfluctuation_twoDarray_ptr);
//   dynamic_colortable=false;

//   string norm_fluc_filenamestr=imagedir+"norm_fluc_"+xyz_filenamestr;
//   xyzpfunc::write_xyzp_data(
//      ztwoDarray_ptr,normalized_fluctuation_twoDarray_ptr,
//      norm_fluc_filenamestr);
//   writeimage("norm_zfluc",normalized_fluctuation_twoDarray_ptr,false,
//              ladarimage::p_data);

   delete zfluctuation_twoDarray_ptr;
   return normalized_fluctuation_twoDarray_ptr;
}

// ---------------------------------------------------------------------
// Member function compute_p_fluctuations generates an outut xyzp file
// displaying probability value standard deviation as a function of
// position within the ladar image.

void ladarimage::compute_p_fluctuations(
   twoDarray const *ztwoDarray_ptr,twoDarray const *ptwoDarray_ptr)
{
   outputfunc::write_banner("Computing p fluctuations:");

   int nsize_max=10;
   int ndim=sqr(2*nsize_max+1);
   twoDarray* pfluctuation_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
   pfluctuation_twoDarray_ptr->initialize_values(xyzpfunc::null_value);
   
   const double zmin=2.5;	 // meters
   double p_diff[ndim];
   for (unsigned int px=nsize_max; px<ztwoDarray_ptr->get_mdim()-nsize_max; 
        px++)
   {
      for (unsigned int py=nsize_max; py<ztwoDarray_ptr->get_ndim()-nsize_max;
           py++)
      {
         double curr_z=ztwoDarray_ptr->get(px,py);
         if (curr_z > zmin)
         {
            int nsize=0;
            if (curr_z < 5)
            {
               nsize=3;
            }
            else if (curr_z < 10)
            {
               nsize=5;
            }
            else if (curr_z < 15)
            {
               nsize=7;
            }
            else
            {
               nsize=9;
            }
            
// Compute variance in fractional delta-p values between current pixel
// and its ndim x ndim -1 nearest neighbors:

            int pixel_counter=0;
            double curr_p=ptwoDarray_ptr->get(px,py);
            for (int i=-nsize; i<=nsize; i++)
            {
               for (int j=-nsize; j<=nsize; j++)
               {
                  double neighbor_z=ztwoDarray_ptr->get(px+i,py+j);
                  if (neighbor_z > zmin && !(i==0 && j==0))
                  {
                     double neighbor_p=ptwoDarray_ptr->get(px+i,py+j);
                     p_diff[pixel_counter]=sqr(neighbor_p-curr_p)
                        *sqr(neighbor_z-curr_z);
                     pixel_counter++;
                  }
               } // loop over j index
            } // loop over i index
            Quicksort(p_diff,pixel_counter);
            double p_fluc=sqrt(p_diff[basic_math::round(0.90*pixel_counter)]);
            pfluctuation_twoDarray_ptr->put(px,py,p_fluc);
         }
      } // py loop
   } // px loop

   string pfluc_filenamestr=imagedir+"pfluc_"+xyz_filenamestr;
   xyzpfunc::write_xyzp_data(
      ztwoDarray_ptr,pfluctuation_twoDarray_ptr,pfluc_filenamestr,false);
//   writeimage("p_fluctuations",pfluctuation_twoDarray_ptr,
//              false,ladarimage::p_data);
}

// ---------------------------------------------------------------------
// Member function compute_surface_normal_field_variation

void ladarimage::compute_surface_normal_field_variation(
   twoDarray const *ztwoDarray_ptr)
{
   outputfunc::write_banner("Computing surface normal field variation:");

   threevector neighbor_normal;
   twoDarray* normal_dotproduct_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
   normal_dotproduct_twoDarray_ptr->initialize_values(xyzpfunc::null_value);

   int counter=0;
   double max_dotproduct=NEGATIVEINFINITY;
   double min_dotproduct=POSITIVEINFINITY;
   double scalar_product[ztwoDarray_ptr->get_mdim()*ztwoDarray_ptr->
                         get_ndim()];
   for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
   {
      for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
      {
         if (ztwoDarray_ptr->get(px,py) > xyzpfunc::null_value)
         {
            threevector curr_normal=normal_twoDarray_ptr->get(px,py);
            int n_neighbors=0;
            double dotproduct_sum=0;
            if (px < ztwoDarray_ptr->get_mdim()-1)
            {
               neighbor_normal=normal_twoDarray_ptr->get(px+1,py);
               if (neighbor_normal.magnitude() > 0)
               {
                  dotproduct_sum += curr_normal.dot(neighbor_normal);
                  n_neighbors++;
               }
            }
            if (px > 0)
            {
               neighbor_normal=normal_twoDarray_ptr->get(px-1,py);
               if (neighbor_normal.magnitude() > 0)
               {
                  dotproduct_sum += curr_normal.dot(neighbor_normal);
                  n_neighbors++;
               }
            }
            if (py < ztwoDarray_ptr->get_ndim()-1)
            {
               neighbor_normal=normal_twoDarray_ptr->get(px,py+1);
               if (neighbor_normal.magnitude() > 0)
               {
                  dotproduct_sum += curr_normal.dot(neighbor_normal);
                  n_neighbors++;
               }
            }
            if (py > 0)
            {
               neighbor_normal=normal_twoDarray_ptr->get(px,py-1);
               if (neighbor_normal.magnitude() > 0)
               {
                  dotproduct_sum += curr_normal.dot(neighbor_normal);
                  n_neighbors++;
               }
            }

            if (n_neighbors > 2)
            {
               double avg_dotproduct=dotproduct_sum/double(n_neighbors);
               scalar_product[counter++]=avg_dotproduct;
               avg_dotproduct=basic_math::max(0.0,avg_dotproduct);
               avg_dotproduct=basic_math::min(10.0,avg_dotproduct);
               max_dotproduct=basic_math::max(max_dotproduct,avg_dotproduct);
               min_dotproduct=basic_math::min(min_dotproduct,avg_dotproduct);
               normal_dotproduct_twoDarray_ptr->put(px,py,avg_dotproduct);
            }
         }
      } // loop over py index
   } // loop over px index

   prob_distribution prob(counter,scalar_product,3000);
   prob.set_densityfilenamestr(imagedir+"dotproduct_density.meta");
   prob.set_cumulativefilenamestr(imagedir+"dotproduct_cum.meta");
   prob.set_xlabel("Dotproduct");
   prob.write_density_dist();

   cout << "max_dotproduct = " << max_dotproduct 
        << " min_dotproduct = " << min_dotproduct << endl;

   string normal_dotproduct_filename=imagedir+"normal_dotprod"
      +xyz_filenamestr;
   xyzpfunc::write_xyzp_data(
      ztwoDarray_ptr,normal_dotproduct_twoDarray_ptr,
      normal_dotproduct_filename,false);

   delete normal_dotproduct_twoDarray_ptr;
}

// ---------------------------------------------------------------------
void ladarimage::compute_local_planarity(
   twoDarray const *ztwoDarray_ptr,twoDarray const *zhilo_twoDarray_ptr,
   twoDarray* zplanar_twoDarray_ptr)
{
   outputfunc::write_banner("Calculating local planarity:");

   groundfunc::compute_planarity_in_bbox(
//      6,3,normal_twoDarray_ptr,ztwoDarray_ptr,zplanar_twoDarray_ptr);
//      10,5,normal_twoDarray_ptr,ztwoDarray_ptr,zplanar_twoDarray_ptr);
//      8,24,normal_twoDarray_ptr,ztwoDarray_ptr,zplanar_twoDarray_ptr);
//      8,32,normal_twoDarray_ptr,ztwoDarray_ptr,zplanar_twoDarray_ptr);
      10,40,normal_twoDarray_ptr,ztwoDarray_ptr,
      zhilo_twoDarray_ptr,zplanar_twoDarray_ptr);
//      10,80,normal_twoDarray_ptr,ztwoDarray_ptr,zplanar_twoDarray_ptr);

   }

// ---------------------------------------------------------------------
void ladarimage::compute_local_planarity_in_bbox(
   double width,double length,double theta,
   twoDarray const *ztwoDarray_ptr,twoDarray* zplanar_twoDarray_ptr)
{
   outputfunc::write_banner("Calculating local planarity:");

//   const double radius=2;	// meters
//   const double radius=4;	// meters
//   regular_polygon hexagon(6,radius);

   parallelogram bbox(width,length);
   bbox.rotate(Zero_vector,0,0,theta);
   groundfunc::compute_local_planarity(
      bbox,normal_twoDarray_ptr,ztwoDarray_ptr,zplanar_twoDarray_ptr);

   }

// ---------------------------------------------------------------------
// Member function accumulate_planarity_info takes in a cleaned height
// image *ztwoDarray_ptr, an expanded version of the image within
// *zexpand_twoDarray_ptr and planarity information within
// *zplanar_rot_twoDarray_ptr.  It also takes in the rotation angle of
// the bounding box which was used to compute the results within
// *zplanar_rot_twoDarray_ptr.  This method adds the planarity values
// from *zplanar_rot_twoDarray_ptr to *zplanar_summary_twoDarray_ptr
// if they are greater than those within the latter twoDarray.

void ladarimage::accumulate_planarity_info(
   double bbox_rotation_angle,
   twoDarray const *ztwoDarray_ptr,twoDarray const *zexpand_twoDarray_ptr,
   twoDarray const *zplanar_rot_twoDarray_ptr,
   twoDarray* zplanar_summary_twoDarray_ptr,
   twoDarray* planarity_direction_twoDarray_ptr)
{
   outputfunc::write_banner("Accumulate planarity info:");
   
   for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
   {
      for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
      {
         double currz=ztwoDarray_ptr->get(px,py);
         if (currz > xyzpfunc::null_value)
         {
            unsigned int px_rot,py_rot;
            ztwoDarray_ptr->rotated_pixel_coords(
               px,py,bbox_rotation_angle,px_rot,py_rot,zexpand_twoDarray_ptr);
            double curr_planarity_score=zplanar_rot_twoDarray_ptr->
               get(px_rot,py_rot);
            if (curr_planarity_score > 
                zplanar_summary_twoDarray_ptr->get(px,py))
            {
               zplanar_summary_twoDarray_ptr->put(px,py,curr_planarity_score);
               planarity_direction_twoDarray_ptr->put(
                  px,py,bbox_rotation_angle);
            }
         }
      } // loop over py index
   } // looop over px index
}

// ==========================================================================
// Feature extraction member functions:
// ==========================================================================

// Member function find_large_hot_lumps applies a recursive clustering
// technique in order to find lumps of "hot" = high pixels within the
// input twoDarray *ztwoDarray_ptr.

void ladarimage::find_large_hot_lumps(
   twoDarray* ztwoDarray_ptr,bool annotate_ztwoDarray)
{
   outputfunc::write_banner("Finding large hot lumps:");
   const double min_lump_xseparation=2;	// meters
   const double min_lump_yseparation=2;	// meters

   bool merge_close_lumps_together=true;
   unsigned int max_nlumps=10000;
   unsigned int nlumps;
   vector<threevector> lumpCOM(max_nlumps);
   vector<linkedlist*> lumplist_ptr(max_nlumps);
   recursivefunc::locate_hot_pixel_clusters(
      max_nlumps,nlumps,lumpCOM,lumplist_ptr,
      ztwoDarray_ptr,min_lump_xseparation,min_lump_yseparation,
      merge_close_lumps_together);

// Number of pixels within each lump is proportional to projected area
// of tall structure onto ground.  Delete any lump whose projected
// area is too small:

   vector<threevector> lumpCOM_pruned(nlumps);
   vector<linkedlist*> lumplist_pruned_ptr(nlumps);
   double min_area=100;	// meter**2
   unsigned int nlumps_pruned=0;
   for (unsigned int n=0; n<nlumps; n++)
   {
      const double dA=ztwoDarray_ptr->get_deltax()*
         ztwoDarray_ptr->get_deltay();
      double projected_area=lumplist_ptr[n]->size()*dA;
      if (projected_area < min_area)
      {
         mynode* currnode_ptr=lumplist_ptr[n]->get_start_ptr();
         while (currnode_ptr != NULL)
         {
            unsigned int px=basic_math::round(
               currnode_ptr->get_data().get_var(0));
            unsigned int py=basic_math::round(
               currnode_ptr->get_data().get_var(1));
            ztwoDarray_ptr->put(px,py,xyzpfunc::null_value);
            currnode_ptr=currnode_ptr->get_nextptr();
         }
         delete lumplist_ptr[n];
      }
      else
      {
         lumplist_pruned_ptr[nlumps_pruned++]=lumplist_ptr[n];
         lumpCOM_pruned[nlumps_pruned]=lumpCOM[n];
      }
   } // loop over index n labeling lump number
   
   cout << "nlumps_pruned = " << nlumps_pruned << endl;
   if (annotate_ztwoDarray)
   {
      for (unsigned int n=0; n<nlumps_pruned; n++)
      {
         drawfunc::draw_hugepoint(lumpCOM_pruned[n],2,colorfunc::white,
                                  ztwoDarray_ptr);
      }
   }
//   writeimage("cluster_zimage",ztwoDarray_ptr,false);      
}

// ---------------------------------------------------------------------
// Member function plot_zp_distributions generates probability
// distributions for z and p data contained within input twoDarrays
// *ztwoDarray_ptr and *ptwoDarray_ptr.

void ladarimage::plot_zp_distributions(
   twoDarray const *ztwoDarray_ptr,twoDarray const *ptwoDarray_ptr)
{
   outputfunc::write_banner("Plotting z & p distributions:");
   int n_output_bins=3000;
   
   double zmin=0.5*xyzpfunc::null_value;
   prob_distribution prob_z;
   cout << "before call to imagefunc::image_intensity_distribution()" 
        << endl;
   imagefunc::image_intensity_distribution(
      zmin,ztwoDarray_ptr,prob_z,n_output_bins);
   cout << "after call to image_intensity_distribution()" << endl;
   
   prob_z.set_densityfilenamestr(imagedir+"z_density.meta");
   prob_z.set_cumulativefilenamestr(imagedir+"z_cum.meta");
   prob_z.set_xlabel("Height z (meters)");
   prob_z.write_density_dist();

   double pmin=-1;
   prob_distribution prob_p;
   imagefunc::image_intensity_distribution(
      pmin,ptwoDarray_ptr,prob_p,n_output_bins);
   prob_p.set_densityfilenamestr(imagedir+"p_density.meta");
   prob_p.set_cumulativefilenamestr(imagedir+"p_cum.meta");
   prob_p.set_xlabel("Detection probability p");
   prob_p.write_density_dist();
}
