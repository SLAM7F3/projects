// ==========================================================================
// Program ORTHOCONTOUR is a playground for refining building
// structure abstraction.  It attempts to represent each building's
// footprint with shrink-wrapped orthogonal contours.

//		 		ortho t.45-51

// ==========================================================================
// Last updated on 5/22/05; 4/24/06; 6/14/06; 4/5/14
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
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
   std::set_new_handler(sysfunc::out_of_memory);
  
   bool input_param_file;
   unsigned int ninputlines;
   string inputline[200];
   filefunc::parameter_input(
      argc,argv,input_param_file,inputline,ninputlines);

// Read in contents of partially processed binary xyzp file:

   cout << "Enter refined feature image:" << endl;
   urbanimage cityimage;
//   cityimage.set_public_software(true);

   unsigned int currlinenumber=0;
   cityimage.initialize_image(input_param_file,inputline,currlinenumber);

// Chimney footprint dimensions:
   const double delta_x=0.3;	// meters
   const double delta_y=0.3;	// meters

   cityimage.parse_and_store_input_data(delta_x,delta_y,true,false,false);
   cityimage.compute_data_bbox(cityimage.get_z2Darray_ptr(),false);

// Eliminate junk nearby edges of data bounding box:

   ladarfunc::crop_data_inside_bbox(0.01,cityimage.get_data_bbox_ptr(),
                                    cityimage.get_z2Darray_ptr());
   ladarfunc::crop_data_inside_bbox(0.01,cityimage.get_data_bbox_ptr(),
                                    cityimage.get_p2Darray_ptr());

   twoDarray const *ztwoDarray_ptr=cityimage.get_z2Darray_ptr();
   twoDarray const *features_twoDarray_ptr=cityimage.get_p2Darray_ptr();

   string features_filename=cityimage.get_imagedir()+"features.xyzp";   
//   xyzpfunc::write_xyzp_data(
//      ztwoDarray_ptr,features_twoDarray_ptr,features_filename);
//   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
//      features_twoDarray_ptr,features_filename);
//   cityimage.writeimage(
//      "features",features_twoDarray_ptr,false,ladarimage::p_data);

   twoDarray const *features_and_heights_twoDarray_ptr=
      urbanfunc::color_feature_heights(
      ztwoDarray_ptr,features_twoDarray_ptr);
   string features_and_heights_filename=
      cityimage.get_imagedir()+"features_and_heights.xyzp";   

/*
   xyzpfunc::write_xyzp_data(
      ztwoDarray_ptr,features_and_heights_twoDarray_ptr,
      features_and_heights_filename);
   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
      ztwoDarray_ptr,features_and_heights_filename);
*/

/*
   twoDarray const *features_and_heights_RGB_twoDarray_ptr=
      ladarfunc::recolor_feature_heights_for_RGB_colormap(
         features_and_heights_twoDarray_ptr);
   string features_and_heights_RGB_filename=
      cityimage.get_imagedir()+"features_and_heights_RGB.xyzp";   
   xyzpfunc::write_xyzp_data(
      ztwoDarray_ptr,features_and_heights_RGB_twoDarray_ptr,
      features_and_heights_RGB_filename);
   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
      ztwoDarray_ptr,features_and_heights_RGB_filename);
*/

// ==========================================================================
// Building network generation
// ==========================================================================

// Generate network containing linkedlists of connected rooftop
// pixels:

   double min_footprint_area=10;	// meters**2
   cityimage.generate_buildings_network(
      min_footprint_area,ztwoDarray_ptr,features_twoDarray_ptr);
   cityimage.score_buildings_contour_edge_fits(features_twoDarray_ptr);
   cityimage.construct_rooftop_orthogonal_polygons(features_twoDarray_ptr);
   cityimage.improve_rooftop_sym_dirs();

//   twoDarray* bbox_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
//   features_and_heights_twoDarray_ptr->copy(bbox_twoDarray_ptr);
//   cityimage.draw_building_bboxes(bbox_twoDarray_ptr);
//   string bbox_filename=cityimage.get_imagedir()+"bbox.xyzp";   
//   xyzpfunc::write_xyzp_data(
//      ztwoDarray_ptr,bbox_twoDarray_ptr,bbox_filename);
//   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
//      ztwoDarray_ptr,bbox_filename);
//   delete bbox_twoDarray_ptr;

// Once building symmetry directions have been refined based upon both
// initial orthogonal contour fitting to rooftop footprint and
// azimuthal rooftop point rotation methods, recompute rooftop
// footprint orthogonal contours:

   cityimage.generate_network_contours(
      ztwoDarray_ptr,cityimage.get_buildings_network_ptr());
   cityimage.construct_rooftop_orthogonal_polygons(features_twoDarray_ptr);
   
   cityimage.simplify_rooftop_orthogonal_polygons();
   cityimage.decompose_rooftop_orthogonal_polygons();
   cityimage.construct_building_wireframe_models(
      ztwoDarray_ptr,features_twoDarray_ptr);
//   cityimage.print_building_data();
//   double max_bldg_height=cityimage.compute_tallest_building_height();
//   cout << "max_bldg_height = " << max_bldg_height << endl;

// Generate and draw nearest neighbor links:

   cityimage.generate_site_voronoi_regions(
      cityimage.get_buildings_network_ptr());
   voronoifunc::generate_nearest_neighbor_links(
      ztwoDarray_ptr,cityimage.get_buildings_network_ptr());
   cityimage.get_buildings_network_ptr()->sort_all_site_neighbors();

// Save buildings network information to ascii text file:

   string buildings_network_text_filename=
      cityimage.get_imagedir()+"buildings_network.txt";
   urbanfunc::output_buildings_network_to_textfile(
      cityimage.get_buildings_network_ptr(),buildings_network_text_filename);

// Superpose building wireframe models on buildings and features map:

   outputfunc::write_banner(
      "Writing out buildings on features & heights map:");
   string buildings_and_features_filename=
      cityimage.get_imagedir()+"buildings_and_features.xyzp";   
   xyzpfunc::write_xyzp_data(
      ztwoDarray_ptr,features_and_heights_twoDarray_ptr,
      buildings_and_features_filename);
   delete features_and_heights_twoDarray_ptr;

   urbanfunc::draw_3D_buildings(
      cityimage.get_buildings_network_ptr(),
      buildings_and_features_filename,draw3Dfunc::annotation_value1);
//   urbanfunc::annotate_building_labels(
//      cityimage.get_buildings_network_ptr(),
//      buildings_and_features_filename,draw3Dfunc::annotation_value2,false);

//   cityimage.draw_network_contour_cylinders(
//      buildings_and_features_filename,cityimage.get_buildings_network_ptr());
//   cityimage.draw_network_subcontour_cylinders(
//      buildings_and_features_filename,cityimage.get_buildings_network_ptr());

// Construct xy grid coordinate system surrounding xyzp data:

//   ladarfunc::draw_xy_coordinate_system(
//      buildings_and_features_filename,draw3Dfunc::annotation_value2,
//      ztwoDarray_ptr);

/*
// For video draping purposes, recolor p-values so that they can be
// viewed with the group 94/106 dataviewer using our RGB colormap:

   string buildings_and_features_RGB_filename=
      cityimage.get_imagedir()+"buildings_and_features_RGB.xyzp";   
   xyzpfunc::recolor_p_values_for_RGB_colormap(
      buildings_and_features_filename,
      buildings_and_features_RGB_filename);
   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
      ztwoDarray_ptr,buildings_and_features_RGB_filename);
*/

   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
      ztwoDarray_ptr,buildings_and_features_filename);

   cityimage.summarize_results();
   cityimage.update_logfile("ortho");
}
