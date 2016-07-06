// ==========================================================================
// Program CARS is a testing ground for vehicle and other small bump
// detection algorithms.
// ==========================================================================
// Last updated on 7/5/05; 4/24/06
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include "image/binaryimagefuncs.h"
#include "image/connectfuncs.h"
#include "threeDgraphics/draw3Dfuncs.h"
#include "ladar/featurefuncs.h"
#include "general/filefuncs.h"
#include "ladar/groundfuncs.h"
#include "datastructures/Hashtable.h"
#include "image/imagefuncs.h"
#include "ladar/ladarfuncs.h"
#include "general/outputfuncs.h"
#include "geometry/parallelogram.h"
#include "image/recursivefuncs.h"
#include "general/sysfuncs.h"
#include "image/TwoDarray.h"
#include "urban/urbanfuncs.h"
#include "urban/urbanimage.h"
#include "geometry/voronoifuncs.h"
#include "threeDgraphics/xyzpfuncs.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::string;
   using std::ostream;
   using std::istream;
   using std::ifstream;
   using std::ofstream;
   using std::cout;
   using std::cin;
   using std::ios;
   using std::endl;
   std::set_new_handler(sysfunc::out_of_memory);
  
   bool input_param_file;
   unsigned int ninputlines,currlinenumber;
   string inputline[200];
   string logfilename=sysfunc::get_projectsrootdir()+
      "src/mains/alirt_acc/abstraction.logfile";

   filefunc::parameter_input(
      argc,argv,input_param_file,inputline,ninputlines);
   currlinenumber=0;

// Read contents of binary xyzp file into 1D x, y, z and p arrays:

   cout << "Enter refined feature image:" << endl;
   urbanimage cityimage;
//   cityimage.set_public_software(true);
   cityimage.initialize_image(input_param_file,inputline,currlinenumber);
   cityimage.parse_and_store_input_data(true,false,false);

// Eliminate junk nearby edges of data bounding box:

   cityimage.compute_data_bbox(cityimage.get_z2Darray_ptr(),false);
   ladarfunc::crop_data_inside_bbox(0.01,cityimage.get_data_bbox_ptr(),
                                    cityimage.get_z2Darray_ptr());
   ladarfunc::crop_data_inside_bbox(0.01,cityimage.get_data_bbox_ptr(),
                                    cityimage.get_p2Darray_ptr());

   twoDarray const *ztwoDarray_ptr=cityimage.get_z2Darray_ptr();
   twoDarray const *features_twoDarray_ptr=cityimage.get_p2Darray_ptr();

   twoDarray* features_and_heights_twoDarray_ptr=
      urbanfunc::color_feature_heights(
      ztwoDarray_ptr,features_twoDarray_ptr);
   string features_and_heights_filename=cityimage.get_imagedir()+
      "features_and_heights.xyzp";
   xyzpfunc::write_xyzp_data(
      ztwoDarray_ptr,features_and_heights_twoDarray_ptr,
      features_and_heights_filename);
   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
      ztwoDarray_ptr,features_and_heights_filename);

// First write out height image corresponding to just asphalt pixels:

   twoDarray* zasphalt_twoDarray_ptr=featurefunc::cull_feature_pixels(
      featurefunc::road_sentinel_value,ztwoDarray_ptr,features_twoDarray_ptr);
   const double height_cutoff=3.0;	// meters
   imagefunc::threshold_intensities_above_cutoff(
      zasphalt_twoDarray_ptr,height_cutoff,xyzpfunc::null_value);
   twoDarray* asphalt_and_heights_twoDarray_ptr=
      urbanfunc::color_feature_heights(
         zasphalt_twoDarray_ptr,features_twoDarray_ptr);

   string asphalt_filename=cityimage.get_imagedir()+"asphalt_and_heights.xyzp";
   xyzpfunc::write_xyzp_data(
      zasphalt_twoDarray_ptr,asphalt_and_heights_twoDarray_ptr,
      asphalt_filename);
   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
      ztwoDarray_ptr,asphalt_filename);
   delete zasphalt_twoDarray_ptr;
//   delete asphalt_and_heights_twoDarray_ptr;

// Search for genuinely low asphalt pixels which almost certainly
// represent true street rather than cars:

   twoDarray* lo_asphalt_twoDarray_ptr=groundfunc::find_low_local_pixels(
      ztwoDarray_ptr,features_twoDarray_ptr);
   string lo_asphalt_filename=cityimage.get_imagedir()+"lo_asphalt.xyzp";
   xyzpfunc::write_xyzp_data(
//      lo_asphalt_twoDarray_ptr,features_twoDarray_ptr,
      lo_asphalt_twoDarray_ptr,asphalt_and_heights_twoDarray_ptr,
      lo_asphalt_filename);
   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
      lo_asphalt_twoDarray_ptr,lo_asphalt_filename);

// Perform oozing operation to connect together more asphalt pixels:

   twoDarray* ooze_asphalt_twoDarray_ptr=
      groundfunc::interpolate_lo_asphalt_heights(
         ztwoDarray_ptr,features_twoDarray_ptr,lo_asphalt_twoDarray_ptr);
   delete lo_asphalt_twoDarray_ptr;

   string ooze_filename=cityimage.get_imagedir()+"ooze_asphalt.xyzp";
   xyzpfunc::write_xyzp_data(
//      ooze_asphalt_twoDarray_ptr,features_twoDarray_ptr,ooze_filename);
      ooze_asphalt_twoDarray_ptr,asphalt_and_heights_twoDarray_ptr,
      ooze_filename);
   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
      ooze_asphalt_twoDarray_ptr,ooze_filename);

   twoDarray* zasphalt_bump_twoDarray_ptr=
      featurefunc::subtract_genuine_ground_asphalt_pixels(
         ztwoDarray_ptr,features_twoDarray_ptr,ooze_asphalt_twoDarray_ptr);
   delete ooze_asphalt_twoDarray_ptr;
   string cars_filename=cityimage.get_imagedir()+"asphalt_bumps.xyzp";
   xyzpfunc::write_xyzp_data(
//      zasphalt_bump_twoDarray_ptr,features_twoDarray_ptr,cars_filename);
      zasphalt_bump_twoDarray_ptr,asphalt_and_heights_twoDarray_ptr,
      cars_filename);
   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
      zasphalt_bump_twoDarray_ptr,cars_filename);

/*
// Simulate ladar cross-range resolution degradation by smearing
// asphalt-bumps map with 2D gaussian:

   double dilate_dist=2;	// meters
   twoDarray* smeared_zbump_twoDarray_ptr=binaryimagefunc::binary_dilate(
      dilate_dist,dilate_dist,0.99*xyzpfunc::null_value,
      zasphalt_bump_twoDarray_ptr,xyzpfunc::null_value);
*/

   featurefunc::color_local_height_bumps(
      zasphalt_bump_twoDarray_ptr,features_and_heights_twoDarray_ptr,
      draw3Dfunc::annotation_value1);
   string height_bumps_filename=
      cityimage.get_imagedir()+"height_bumps.xyzp";
   xyzpfunc::write_xyzp_data(
      ztwoDarray_ptr,features_and_heights_twoDarray_ptr,
      height_bumps_filename);
   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
      features_and_heights_twoDarray_ptr,height_bumps_filename);

   double z_threshold=0.99*xyzpfunc::null_value;
   double dA=zasphalt_bump_twoDarray_ptr->get_deltax()*
      zasphalt_bump_twoDarray_ptr->get_deltay();
   const int min_area=0.5*(4.5*2);	// meters**2
   int min_component_pixels=basic_math::round(min_area/dA);
   Hashtable<linkedlist*>* car_hashtable_ptr=
      connectfunc::generate_connected_hashtable(
         z_threshold,min_component_pixels,zasphalt_bump_twoDarray_ptr);
   delete zasphalt_bump_twoDarray_ptr;

   twoDarray* car_ztwoDarray_ptr=new twoDarray(ztwoDarray_ptr);
   connectfunc::decode_connected_hashtable(
      car_hashtable_ptr,car_ztwoDarray_ptr);
//   cityimage.writeimage("cars",car_ztwoDarray_ptr);

   delete car_hashtable_ptr;

   delete features_and_heights_twoDarray_ptr;
}


