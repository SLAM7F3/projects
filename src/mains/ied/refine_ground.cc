// ==========================================================================
// Program REFINE_GROUND takes in an XYZP file which is assumed to
// represent an IED scene that has already been filtered and coarsely
// flattened.  It performs an "oozing" technique to completely flatten
// the ground, and it generates a mask in which all non-ground pixels
// are nulled.
// ==========================================================================
// Last updated on 5/22/05; 4/24/06; 10/30/07
// ==========================================================================

#include <iostream>
#include <iomanip>
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
#include "geometry/parallelogram.h"
#include "image/recursivefuncs.h"
#include "general/sysfuncs.h"
#include "urban/tree_cluster.h"
#include "image/TwoDarray.h"
#include "geometry/voronoifuncs.h"
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
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);
  
   bool input_param_file;
   int ninputlines,currlinenumber;
   string inputline[200];

   filefunc::parameter_input(
      argc,argv,input_param_file,inputline,ninputlines);
   currlinenumber=0;

// Read in contents of partially processed binary xyzp file:

   cout << "Enter flattened and filtered IED image:" << endl;
   ladarimage iedimage;
//   iedimage.set_public_software(true);

// Chimney footprint dimensions:
   const double delta_x=0.15;	// meters
   const double delta_y=0.15;	// meters
   iedimage.initialize_image(input_param_file,inputline,currlinenumber);
   iedimage.parse_and_store_input_data(
      delta_x,delta_y,false,false,false,false);
   iedimage.compute_trivial_xy_data_bbox(iedimage.get_z2Darray_ptr());

   twoDarray* ztwoDarray_ptr=iedimage.get_z2Darray_ptr();
   twoDarray const *ptwoDarray_ptr=iedimage.get_p2Darray_ptr();

   string flattened_filename=iedimage.get_imagedir()+"flattened.xyzp";   
//   xyzpfunc::write_xyzp_data(
//      ztwoDarray_ptr,ptwoDarray_ptr,flattened_filename);
//   draw3Dfunc::append_fake_z_points_in_twoDarray_middle(
//      ztwoDarray_ptr,flattened_filename);

// Plot height distribution for flattened ground image:

   ladarfunc::compute_z_distribution(iedimage.get_imagedir(),ztwoDarray_ptr);

// Find relatively low and high regions within z-image.  Then use
// "oozing" technique to completely flatten ground:

   twoDarray* zhilo_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
   groundfunc::find_low_local_pixels(ztwoDarray_ptr,zhilo_twoDarray_ptr);
//   string zhilo_filename=iedimage.get_imagedir()+"zhilo.xyzp";
//   xyzpfunc::write_xyzp_data(
//      zhilo_twoDarray_ptr,iedimage.get_p2Darray_ptr(),zhilo_filename);

   twoDarray* zcompletely_flattened_twoDarray_ptr=
      groundfunc::completely_flatten_ground(
         ztwoDarray_ptr,zhilo_twoDarray_ptr);
   delete zhilo_twoDarray_ptr;

   string flat_filenamestr=iedimage.get_imagedir()+"complete_flat_"
      +iedimage.get_xyz_filenamestr();
   xyzpfunc::write_xyzp_data(
      zcompletely_flattened_twoDarray_ptr,iedimage.get_p2Darray_ptr(),
      flat_filenamestr);
   draw3Dfunc::append_fake_z_points_in_twoDarray_middle(
      iedimage.get_z2Darray_ptr(),flat_filenamestr);

// Set all entries within completely flattened ground image that do
// NOT equal 0 height equal to xyzpfunc::null_value for masking
// purposes...

   imagefunc::particular_cutoff_threshold(
      0,zcompletely_flattened_twoDarray_ptr,xyzpfunc::null_value);
   string groundmask_filename=iedimage.get_imagedir()+"groundmask_"
      +iedimage.get_xyz_filenamestr();
   xyzpfunc::write_xyzp_data(
      zcompletely_flattened_twoDarray_ptr,iedimage.get_p2Darray_ptr(),
      groundmask_filename);
   draw3Dfunc::append_fake_z_points_in_twoDarray_middle(
      iedimage.get_z2Darray_ptr(),groundmask_filename);

// Generate flattened ground image with all non-ground pixels nulled:

   recursivefunc::binary_null(
      0.5*xyzpfunc::null_value,ztwoDarray_ptr,
      zcompletely_flattened_twoDarray_ptr,xyzpfunc::null_value,false);
   delete zcompletely_flattened_twoDarray_ptr;
   
   double max_ground_height=1.0;	// meter
   double min_ground_height=-1.0;	// meter
   imagefunc::threshold_intensities_above_cutoff(
      ztwoDarray_ptr,max_ground_height,xyzpfunc::null_value);
   imagefunc::threshold_intensities_below_cutoff(
      ztwoDarray_ptr,min_ground_height,xyzpfunc::null_value);

   string just_flat_ground_filename=iedimage.get_imagedir()+
      "just_flat_ground_"+iedimage.get_xyz_filenamestr();
   xyzpfunc::write_xyzp_data(
      ztwoDarray_ptr,iedimage.get_p2Darray_ptr(),
      just_flat_ground_filename);
//   draw3Dfunc::append_fake_z_points_in_twoDarray_middle(
//      iedimage.get_z2Darray_ptr(),just_flat_ground_filename);

   twoDarray* zsmoothed_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
   int n_iters=10;
   for (int iter=0; iter<n_iters; iter++)
   {
      groundfunc::average_down_small_height_fluctuations(
         ztwoDarray_ptr,zsmoothed_twoDarray_ptr);
      zsmoothed_twoDarray_ptr->copy(ztwoDarray_ptr);
//      groundfunc::remove_individual_height_outliers(
//         ztwoDarray_ptr,zsmoothed_twoDarray_ptr);
//      zsmoothed_twoDarray_ptr->copy(ztwoDarray_ptr);
   }
   
   string smoothed_ground_filename=iedimage.get_imagedir()+
      "smoothed_ground_"+iedimage.get_xyz_filenamestr();
   xyzpfunc::write_xyzp_data(
      zsmoothed_twoDarray_ptr,iedimage.get_p2Darray_ptr(),
      smoothed_ground_filename);
   draw3Dfunc::append_fake_z_points_in_twoDarray_middle(
      ztwoDarray_ptr,smoothed_ground_filename,-1,1);

   ladarfunc::compute_z_distribution(
      iedimage.get_imagedir(),zsmoothed_twoDarray_ptr);

// Compute connected ground components. Delete small "islands"
// surrounded by large black "oceans" (which generally correspond to
// little areas underneath large tree canopies:

   double min_projected_area=20;    // m**2
   const double zmin=0.5*NEGATIVEINFINITY;
   Hashtable<linkedlist*>* connected_heights_hashtable_ptr=
      ladarfunc::connect_height_components(
         zmin,min_projected_area,zsmoothed_twoDarray_ptr);

   twoDarray* pconnected_components_twoDarray_ptr=
      new twoDarray(ztwoDarray_ptr);
   connectfunc::decode_connected_hashtable(
      connected_heights_hashtable_ptr,pconnected_components_twoDarray_ptr,
      true,xyzpfunc::null_value);
   connectfunc::delete_connected_hashtable(connected_heights_hashtable_ptr);
   binaryimagefunc::binary_threshold(
      0.5*xyzpfunc::null_value,pconnected_components_twoDarray_ptr,
      xyzpfunc::null_value);
   recursivefunc::binary_null(
      0.5*xyzpfunc::null_value,zsmoothed_twoDarray_ptr,
      pconnected_components_twoDarray_ptr,xyzpfunc::null_value,false);

   groundfunc::remove_isolated_pixels(zsmoothed_twoDarray_ptr);

   string connected_filename=iedimage.get_imagedir()+"connected.xyzp";   
   xyzpfunc::write_xyzp_data(
      zsmoothed_twoDarray_ptr,pconnected_components_twoDarray_ptr,
      connected_filename);
   draw3Dfunc::append_fake_z_points_in_twoDarray_middle(
      ztwoDarray_ptr,smoothed_ground_filename,-1,1);
//   ladarfunc::draw_xy_coordinate_system(
//      smoothed_ground_filename,1.0,ztwoDarray_ptr,3);

// Perform another round of ground extraction:

   twoDarray* zlevel3_twoDarray_ptr=
      iedimage.interpolate_and_flatten_ground_surface(
         zsmoothed_twoDarray_ptr,zsmoothed_twoDarray_ptr,
         iedimage.get_p2Darray_ptr(),2.0,0.20);
   recursivefunc::binary_null(
      0.5*xyzpfunc::null_value,zlevel3_twoDarray_ptr,
      zsmoothed_twoDarray_ptr,xyzpfunc::null_value);

   string zlevel3_filename=iedimage.get_imagedir()+"zlevel3.xyzp";
   xyzpfunc::write_xyzp_data(
      zlevel3_twoDarray_ptr,iedimage.get_p2Darray_ptr(),zlevel3_filename);
   draw3Dfunc::append_fake_z_points_in_twoDarray_middle(
      ztwoDarray_ptr,zlevel3_filename,-1,1);

   zlevel3_twoDarray_ptr->copy(ztwoDarray_ptr);
   delete zlevel3_twoDarray_ptr;
   
   exit(-1);
   
/*
// Experiment on Sunday Aug 22....Set all z values less than some
// zthreshold equal to zthreshold within connected heights image...

   twoDarray* zthreshold_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
   zsmoothed_twoDarray_ptr->copy(zthreshold_twoDarray_ptr);

   const double zthreshold=0;	// meters
   imagefunc::threshold_intensities_below_cutoff(
      zthreshold_twoDarray_ptr,zthreshold,zthreshold);
   recursivefunc::binary_null(
      0.5*xyzpfunc::null_value,zthreshold_twoDarray_ptr,
      zsmoothed_twoDarray_ptr,xyzpfunc::null_value,false);

   string threshold_filename=iedimage.get_imagedir()+"threshold.xyzp";   
   xyzpfunc::write_xyzp_data(
      zthreshold_twoDarray_ptr,pconnected_components_twoDarray_ptr,
      threshold_filename);
*/

// Experiment on Sunday Aug 22...Try median filtering to smooth noisy
// fluctuations in extracted ground surface...

   twoDarray* zmedian_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
   imagefunc::median_filter(3,3,zsmoothed_twoDarray_ptr,zmedian_twoDarray_ptr,
                            xyzpfunc::null_value,false);
   recursivefunc::binary_null(
      0.5*xyzpfunc::null_value,zmedian_twoDarray_ptr,
      zsmoothed_twoDarray_ptr,xyzpfunc::null_value,false);

   string median_filename=iedimage.get_imagedir()+"median.xyzp";   
   xyzpfunc::write_xyzp_data(
      zmedian_twoDarray_ptr,pconnected_components_twoDarray_ptr,
      median_filename);

   delete zsmoothed_twoDarray_ptr;
   delete zmedian_twoDarray_ptr;
   
   exit(-1);
   
   double z1=0.1; // meter
   double z2=0.5; // meter
   twoDarray* exaggerated_surface_twoDarray_ptr=
      groundfunc::exaggerate_surface_image(z1,z2,ztwoDarray_ptr);
   string surface_filename=iedimage.get_imagedir()+"surface.xyzp";   
   xyzpfunc::write_xyzp_data(
      ztwoDarray_ptr,exaggerated_surface_twoDarray_ptr,surface_filename,
      false,true);

   exit(-1);
   iedimage.update_logfile("refine_ground");
}


