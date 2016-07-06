// ==========================================================================
// Program ENHANCE_SCALE
// ==========================================================================
// Last updated on 5/22/05; 4/24/06; 10/30/07
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "image/connectfuncs.h"
#include "threeDgraphics/draw3Dfuncs.h"
#include "ladar/featurefuncs.h"
#include "general/filefuncs.h"
#include "image/graphicsfuncs.h"
#include "ladar/groundfuncs.h"
#include "datastructures/Hashtable.h"
#include "image/imagefuncs.h"
#include "ladar/ladarfuncs.h"
#include "general/outputfuncs.h"
#include "geometry/parallelogram.h"
#include "image/recursivefuncs.h"
#include "general/sysfuncs.h"
#include "urban/tree_cluster.h"
#include "image/TwoDarray.h"
#include "urban/urbanfuncs.h"
#include "urban/urbanimage.h"
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
   unsigned int ninputlines,currlinenumber;
   string inputline[200];

   filefunc::parameter_input(
      argc,argv,input_param_file,inputline,ninputlines);
   currlinenumber=0;

// Read in contents of partially processed binary xyzp file:

   cout << "Enter flattened and filtered IED image:" << endl;
   urbanimage cityimage;
//   cityimage.set_public_software(true);

// Chimney footprint dimensions:
   const double delta_x=0.15;	// meters
   const double delta_y=0.15;	// meters
   cityimage.initialize_image(input_param_file,inputline,currlinenumber);
   cityimage.parse_and_store_input_data(
      delta_x,delta_y,false,false,false,false);
   cityimage.compute_trivial_xy_data_bbox(cityimage.get_z2Darray_ptr());

   twoDarray* ztwoDarray_ptr=cityimage.get_z2Darray_ptr();
   twoDarray const *ptwoDarray_ptr=cityimage.get_p2Darray_ptr();

   string flattened_filename=cityimage.get_imagedir()+"flattened.xyzp";   
//   xyzpfunc::write_xyzp_data(
//      ztwoDarray_ptr,ptwoDarray_ptr,flattened_filename);
//   draw3Dfunc::append_fake_z_points_in_twoDarray_middle(
//      ztwoDarray_ptr,flattened_filename);

// Plot height distribution for flattened ground image:

   ladarfunc::compute_z_distribution(cityimage.get_imagedir(),ztwoDarray_ptr);

// Find relatively low and high regions within z-image.  Then use
// "oozing" technique to completely flatten ground:

   twoDarray* zhilo_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
   groundfunc::find_low_local_pixels(ztwoDarray_ptr,zhilo_twoDarray_ptr);
//   string zhilo_filename=cityimage.get_imagedir()+"zhilo.xyzp";
//   xyzpfunc::write_xyzp_data(
//      zhilo_twoDarray_ptr,cityimage.get_p2Darray_ptr(),zhilo_filename);

   twoDarray* zcompletely_flattened_twoDarray_ptr=
      groundfunc::completely_flatten_ground(
         ztwoDarray_ptr,zhilo_twoDarray_ptr);
   delete zhilo_twoDarray_ptr;

   string flat_filenamestr=cityimage.get_imagedir()+"complete_flat_"
      +cityimage.get_xyz_filenamestr();
   xyzpfunc::write_xyzp_data(
      zcompletely_flattened_twoDarray_ptr,cityimage.get_p2Darray_ptr(),
      flat_filenamestr);
   draw3Dfunc::append_fake_z_points_in_twoDarray_middle(
      cityimage.get_z2Darray_ptr(),flat_filenamestr);

// Set all entries within completely flattened ground image that do
// NOT equal 0 height equal to xyzpfunc::null_value for masking
// purposes...

   imagefunc::particular_cutoff_threshold(
      0,zcompletely_flattened_twoDarray_ptr,xyzpfunc::null_value);
   string groundmask_filename=cityimage.get_imagedir()+"groundmask_"
      +cityimage.get_xyz_filenamestr();
   xyzpfunc::write_xyzp_data(
      zcompletely_flattened_twoDarray_ptr,cityimage.get_p2Darray_ptr(),
      groundmask_filename);
   draw3Dfunc::append_fake_z_points_in_twoDarray_middle(
      cityimage.get_z2Darray_ptr(),groundmask_filename);

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

   string just_flat_ground_filename=cityimage.get_imagedir()+
      "just_flat_ground_"+cityimage.get_xyz_filenamestr();
   xyzpfunc::write_xyzp_data(
      ztwoDarray_ptr,cityimage.get_p2Darray_ptr(),
      just_flat_ground_filename);
//   draw3Dfunc::append_fake_z_points_in_twoDarray_middle(
//      cityimage.get_z2Darray_ptr(),just_flat_ground_filename);

   twoDarray* zsmoothed_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
   int n_iters=10;
   for (int iter=0; iter<n_iters; iter++)
   {
      groundfunc::average_down_small_height_fluctuations(
         ztwoDarray_ptr,zsmoothed_twoDarray_ptr);
      zsmoothed_twoDarray_ptr->copy(ztwoDarray_ptr);
   }
   
   string smoothed_ground_filename=cityimage.get_imagedir()+
      "smoothed_ground_"+cityimage.get_xyz_filenamestr();
   xyzpfunc::write_xyzp_data(
      zsmoothed_twoDarray_ptr,cityimage.get_p2Darray_ptr(),
      smoothed_ground_filename);

   ladarfunc::compute_z_distribution(
      cityimage.get_imagedir(),zsmoothed_twoDarray_ptr);

   exit(-1);

   double z1=0.1; // meter
   double z2=0.5; // meter
   twoDarray* exaggerated_surface_twoDarray_ptr=
      groundfunc::exaggerate_surface_image(z1,z2,ztwoDarray_ptr);
   string surface_filename=cityimage.get_imagedir()+"surface.xyzp";   
   xyzpfunc::write_xyzp_data(
      ztwoDarray_ptr,exaggerated_surface_twoDarray_ptr,surface_filename,
      false,true);

   exit(-1);
   cityimage.summarize_results();
   cityimage.update_logfile("abstraction");
}


