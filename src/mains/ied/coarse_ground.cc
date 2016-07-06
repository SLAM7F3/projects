// ==========================================================================
// Program COARSE_GROUND reads in a rotated IED xyzp file.  It
// coarsely identifies and extracts the ground surface from the input data.
// ==========================================================================
// Last updated on 1/11/05
// ==========================================================================

#include <iomanip>
#include <iostream>
#include <set>
#include <string>
#include <vector>
#include "image/binaryimagefuncs.h"
#include "image/connectfuncs.h"
#include "threeDgraphics/draw3Dfuncs.h"
#include "ladar/featurefuncs.h"
#include "general/filefuncs.h"
#include "image/graphicsfuncs.h"
#include "ladar/groundfuncs.h"
#include "datastructures/Hashtable.h"
#include "image/imagefuncs.h"
#include "ladar/ladarfuncs.h"
#include "ladar/ladarimage.h"
#include "general/outputfuncs.h"
#include "image/recursivefuncs.h"
#include "general/sysfuncs.h"
#include "image/TwoDarray.h"
#include "threeDgraphics/xyzpfuncs.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ifstream;
   using std::ios;
   using std::istream;
   using std::ofstream;
   using std::ostream;
   using std::pair;
   using std::string;
   using std::vector;

   std::set_new_handler(sysfunc::out_of_memory);
  
   bool input_param_file;
   int ninputlines,currlinenumber;
   string inputline[200];
   string logfilename=sysfunc::get_cplusplusrootdir()+"alirt/ground.logfile";

   filefunc::parameter_input(
      argc,argv,input_param_file,inputline,ninputlines);
   currlinenumber=0;

// ==========================================================================
// Raw image initialization
// ==========================================================================

// Read contents of raw binary xyzp file:
   
   ladarimage iedimage;
//    iedimage.set_public_software(true);

// Chimney footprint dimensions:
//   const double delta_x=0.3;	// meters
//   const double delta_y=0.3;	// meters
   const double delta_x=0.15;	// meters
   const double delta_y=0.15;	// meters
   iedimage.initialize_image(input_param_file,inputline,currlinenumber);
   iedimage.parse_and_store_input_data(
      delta_x,delta_y,true,false,false,false,0);
   iedimage.compute_trivial_xy_data_bbox(iedimage.z2Darray_ptr);

// ==========================================================================
// Median filling
// ==========================================================================

// Use median filling and isolated outlier elimination to clean up raw
// height imagery.  On 8/12/04, we discovered that rotated IED xyzp
// data appears to have systematic blank spots.  We definitely need to
// perform a little bit of median filling in order to remove these
// spots.  The ALIRT scan pattern also leaves holes in the data which
// are probably best filled in as well.

   int niters=5;
   int nsize=3;
   ladarfunc::median_fill_image(niters,nsize,iedimage.get_data_bbox_ptr(),
                                iedimage.z2Darray_ptr);
   ladarfunc::remove_isolated_outliers(
      iedimage.get_data_bbox_ptr(),iedimage.z2Darray_ptr);

// Plot height distribution for image with original ground surface:

   ladarfunc::compute_z_distribution(
      iedimage.imagedir,iedimage.z2Darray_ptr);

   string filled_zimage_filename=iedimage.get_imagedir()+
      "filled_zimage.xyzp";
   xyzpfunc::write_xyzp_data(
      iedimage.z2Darray_ptr,iedimage.get_p2Darray_ptr(),
      filled_zimage_filename);
   draw3Dfunc::append_fake_z_points_in_twoDarray_middle(
      iedimage.z2Darray_ptr,filled_zimage_filename);

   filefunc::gunzip_file_if_gzipped(filled_zimage_filename);
//   draw3Dfunc::draw_thick_polygon(
//      (*iedimage.get_data_bbox_ptr()),filled_zimage_filename,
//      draw3Dfunc::annotation_value1);
//   ladarfunc::draw_xy_coordinate_system(
//      filled_zimage_filename,100,iedimage.z2Darray_ptr,50);

// Compute distributions for filled z and p images:
   
//   iedimage.plot_zp_distributions(
//      iedimage.z2Darray_ptr,iedimage.get_p2Darray_ptr());

// Create binary mask of partially cleaned height image:

   twoDarray* mask_twoDarray_ptr=new twoDarray(iedimage.z2Darray_ptr);
   binaryimagefunc::binary_threshold(
      0.5*xyzpfunc::null_value,iedimage.z2Darray_ptr,
      mask_twoDarray_ptr,xyzpfunc::null_value);

   int n_iters=5;
   int n_size=5;
   graphicsfunc::turtle_erode_binary_region(
      n_iters,n_size,mask_twoDarray_ptr,xyzpfunc::null_value);

   string mask_filename=iedimage.get_imagedir()+"mask.xyzp";
   xyzpfunc::write_xyzp_data(
      iedimage.z2Darray_ptr,mask_twoDarray_ptr,mask_filename);
//   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
//      mask_twoDarray_ptr,mask_filename);

// ==========================================================================
// Mario bashing and local ground oozing
// ==========================================================================

// Copy current working image into a new, dynamically generated
// twoDarray *zground_silhouetted_twoDarray_ptr which will be
// destructively processsed:

   twoDarray* zground_silhouetted_twoDarray_ptr=
      new twoDarray(iedimage.z2Darray_ptr);
   iedimage.z2Darray_ptr->copy(zground_silhouetted_twoDarray_ptr);

// Use gradient field information to locate relatively high objects
// within median filled height image:

   const double spatial_resolution=0.5*(delta_x+delta_y);	// meter
   twoDarray* xderiv_twoDarray_ptr=new twoDarray(iedimage.z2Darray_ptr);
   twoDarray* yderiv_twoDarray_ptr=new twoDarray(iedimage.z2Darray_ptr);

   bool mask_points_near_border=true;
   iedimage.estimate_ground_surface_using_gradient_info(
      spatial_resolution,xderiv_twoDarray_ptr,yderiv_twoDarray_ptr,
      zground_silhouetted_twoDarray_ptr,mask_points_near_border,
      mask_twoDarray_ptr);
   delete xderiv_twoDarray_ptr;
   delete yderiv_twoDarray_ptr;
   delete mask_twoDarray_ptr;

   twoDarray* zlevel_twoDarray_ptr=
      iedimage.interpolate_and_flatten_ground_surface(
         iedimage.z2Darray_ptr,zground_silhouetted_twoDarray_ptr,
         iedimage.get_p2Darray_ptr(),15);
   delete zground_silhouetted_twoDarray_ptr;
   recursivefunc::binary_null(
      0.5*xyzpfunc::null_value,zlevel_twoDarray_ptr,
      iedimage.z2Darray_ptr,xyzpfunc::null_value);

   string zlevel_filename=iedimage.get_imagedir()+"zlevel.xyzp";
   xyzpfunc::write_xyzp_data(
      zlevel_twoDarray_ptr,iedimage.get_p2Darray_ptr(),
      zlevel_filename);
   draw3Dfunc::append_fake_z_points_in_twoDarray_middle(
      iedimage.z2Darray_ptr,zlevel_filename);

// Find relatively low and high regions within z-image.  Then use
// "oozing" technique to completely flatten ground:

   twoDarray* zhilo_twoDarray_ptr=new twoDarray(zlevel_twoDarray_ptr);
   groundfunc::find_low_local_pixels(
      zlevel_twoDarray_ptr,zhilo_twoDarray_ptr);
//   string zhilo_filename=iedimage.get_imagedir()+"zhilo.xyzp";
//   xyzpfunc::write_xyzp_data(
//      zhilo_twoDarray_ptr,iedimage.get_p2Darray_ptr(),zhilo_filename);

   twoDarray* zcompletely_flattened_twoDarray_ptr=
      groundfunc::completely_flatten_ground(
         zlevel_twoDarray_ptr,zhilo_twoDarray_ptr);
   delete zlevel_twoDarray_ptr;

   string flat_filenamestr=iedimage.get_imagedir()+"complete_flat_"
      +iedimage.get_xyz_filenamestr();
   xyzpfunc::write_xyzp_data(
      zcompletely_flattened_twoDarray_ptr,iedimage.get_p2Darray_ptr(),
      flat_filenamestr);
   draw3Dfunc::append_fake_z_points_in_twoDarray_middle(
      iedimage.z2Darray_ptr,flat_filenamestr);

// Set all entries within refined ground mask image corresponding to
// relatively high objects equal to xyzpfunc::null_value:

   twoDarray* groundmask_twoDarray_ptr=new twoDarray(iedimage.z2Darray_ptr);
   iedimage.z2Darray_ptr->copy(groundmask_twoDarray_ptr);
   recursivefunc::binary_null(
      1.0,groundmask_twoDarray_ptr,zhilo_twoDarray_ptr,xyzpfunc::null_value,
      true);
   delete zhilo_twoDarray_ptr;
   string groundmask_filename=iedimage.get_imagedir()+"groundmask.xyzp";
   xyzpfunc::write_xyzp_data(
      groundmask_twoDarray_ptr,iedimage.get_p2Darray_ptr(),
      groundmask_filename);
   draw3Dfunc::append_fake_z_points_in_twoDarray_middle(
      iedimage.z2Darray_ptr,groundmask_filename);

// Perform final round of ground extraction using refined version of
// silhouetted ground mask:

   twoDarray* zlevel2_twoDarray_ptr=
      iedimage.interpolate_and_flatten_ground_surface(
         iedimage.z2Darray_ptr,groundmask_twoDarray_ptr,
         iedimage.get_p2Darray_ptr(),3);
   delete groundmask_twoDarray_ptr;
   recursivefunc::binary_null(
      0.5*xyzpfunc::null_value,zlevel2_twoDarray_ptr,
      iedimage.z2Darray_ptr,xyzpfunc::null_value);

   string zlevel2_filename=iedimage.get_imagedir()+"zlevel2.xyzp";
   xyzpfunc::write_xyzp_data(
      zlevel2_twoDarray_ptr,iedimage.get_p2Darray_ptr(),zlevel2_filename);
   draw3Dfunc::append_fake_z_points_in_twoDarray_middle(
      iedimage.z2Darray_ptr,zlevel2_filename);

   zlevel2_twoDarray_ptr->copy(iedimage.z2Darray_ptr);
   delete zlevel2_twoDarray_ptr;

   iedimage.update_logfile("coarse_ground");
}


