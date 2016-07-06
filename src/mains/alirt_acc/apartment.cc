// ==========================================================================
// Program APARTMENT is a hacked up version of ORTHO which is meant to
// be run on chunk 63-69 containing our favorite Lowell apartment
// building.  It generates an XYZP file encompassing a circular region
// around the apartment building.  The 3D pointcloud output from this
// program can be concatenated with the draped 2D photo imagery output
// from program DRAPE_BLDG to produce a fused aerial ladar + ground
// video image of the apartment building.  It should be viewed using
// our clugey RGB colormap in the group 106 dataviewer.
// ==========================================================================
// Last updated on 2/13/05; 4/24/06
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
   unsigned int ninputlines,currlinenumber;
   string inputline[200];

   filefunc::parameter_input(
      argc,argv,input_param_file,inputline,ninputlines);
   currlinenumber=0;

// Read in contents of partially processed binary xyzp file:

   cout << "Enter refined feature image:" << endl;
   urbanimage cityimage;
//   cityimage.set_public_software(true);

// Chimney footprint dimensions:
   const double delta_x=0.3;	// meters
   const double delta_y=0.3;	// meters
   cityimage.initialize_image(input_param_file,inputline,currlinenumber);
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

   twoDarray const *features_and_heights_twoDarray_ptr=
      urbanfunc::color_feature_heights(
      ztwoDarray_ptr,features_twoDarray_ptr);
   string features_and_heights_filename=
      cityimage.get_imagedir()+"features_and_heights.xyzp";   
//   xyzpfunc::write_xyzp_data(
//      ztwoDarray_ptr,features_and_heights_twoDarray_ptr,
//      features_and_heights_filename);
//   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
//      ztwoDarray_ptr,features_and_heights_filename);


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
//   delete bbox_twoDarray_ptr

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

//   int n_building=30;	// apartment building in chunk 63-69:
//   string walls_filename="apartment_wall_polys.txt";
//   urbanfunc::writeout_building_wall_polys(
//      n_building,cityimage.get_buildings_network_ptr(),
//      walls_filename);

// Superpose building wireframe models on buildings and features map:

   outputfunc::write_banner(
      "Writing out buildings on features & heights map:");
   string buildings_and_features_filename=
      cityimage.get_imagedir()+"buildings_and_features.xyzp";   
   xyzpfunc::write_xyzp_data(
      ztwoDarray_ptr,features_and_heights_twoDarray_ptr,
      buildings_and_features_filename);
   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
      features_and_heights_twoDarray_ptr,buildings_and_features_filename);

   urbanfunc::draw_3D_buildings(
      cityimage.get_buildings_network_ptr(),
      buildings_and_features_filename,draw3Dfunc::annotation_value1,false);
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

   delete features_and_heights_twoDarray_ptr;

// Write out xyzp points only within local vicinity of apartment
// building for photograph draping purposes:

   string apartment_filename=cityimage.get_imagedir()+
      "apartment_model_and_features.xyzp";
   filefunc::deletefile(apartment_filename);
   filefunc::deletefile(apartment_filename+".gz");

   std::vector<fourvector>* xyzp_pnt_ptr=
      xyzpfunc::read_xyzp_float_data(buildings_and_features_filename);

   int n=30; // label for apartment building in Lowell chunk #63-69:
   const double rho=30;	// meters
   building* building_ptr=cityimage.get_buildings_network_ptr()->
      get_site_data_ptr(n);

   xyzpfunc::write_local_xyzp_data(
      building_ptr->get_posn(),rho,apartment_filename,xyzp_pnt_ptr);
   delete xyzp_pnt_ptr;

// For video draping purposes, we convert p-values so that they can be viewed 
// using our "RGB" dataviewer colormap rather than our "Hue + value" colormap:

   string apartment_RGB_filename=cityimage.get_imagedir()+
      "apartment_model_and_features_RGB.xyzp";
   filefunc::deletefile(apartment_RGB_filename);
   filefunc::deletefile(apartment_RGB_filename+".gz");

   xyzpfunc::recolor_p_values_for_RGB_colormap(
      apartment_filename,apartment_RGB_filename);

   draw3Dfunc::append_fake_xyzp_points_for_dataviewer_coloring(
      apartment_filename,building_ptr->get_posn());
   draw3Dfunc::append_fake_xyzp_points_for_dataviewer_coloring(
      apartment_RGB_filename,building_ptr->get_posn());

// For video draping purposes, resample XYZ points in vicinity of
// apartment building at much higher density:

   double ds_orig=ztwoDarray_ptr->get_deltax();
//   double ds_new=0.05;	// meter
   double ds_new=0.10;	// meter

   std::vector<fourvector>* xyzp_point_ptr=
      xyzpfunc::read_xyzp_float_data(apartment_RGB_filename);

   string dense_features_and_heights_filename=
      cityimage.get_imagedir()+"dense_features_and_heights.xyzp";
   xyzpfunc::subdivide_xy_spacing(
      dense_features_and_heights_filename,
      ds_orig,ds_new,building_ptr->get_posn(),rho,xyzp_point_ptr);
   xyzpfunc::write_xyzp_data(
      dense_features_and_heights_filename,xyzp_point_ptr);

   cityimage.summarize_results();
}
