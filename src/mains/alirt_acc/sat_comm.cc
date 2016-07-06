// ==========================================================================
// Program SATELLITE_COMM_SHADOWING is a specialized variant of
// program ABSTRACTON intended for notional viewgraph chart
// preparation illustrating W. Mark Smith's idea for using ALIRT
// imagery exploitation results to determine satellite communcation
// obstruction.  This program queries the user to enter elevation and
// azimuth angles for a hypothetical satellite which we assume lies
// infinitely far away from the 3D urban site.  It computes the tree
// and building networks within the site.  The program subsequently
// computes the shadows cast by the side walls of the buildings'
// parallelepipeds as well as by the sides of the tree cluster
// contours onto flattened ground.  Shadowed ground regions are
// displayed (in purple) on a 3D features & heights map.
// ==========================================================================
// Last updated on 5/22/05; 4/24/06
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

   double elevation=25;
   double azimuth=-120;
   cout << "Enter elevation angle in degs:" << endl;
   cin >> elevation;
   elevation *= PI/180;
   cout << "Enter azimuth angle in degs:" << endl;
   cin >> azimuth;
   azimuth *= PI/180;

   bool input_param_file;
   unsigned int ninputlines,currlinenumber;
   string inputline[200];
   string logfilename=sysfunc::get_projectsrootdir()
      +"src/mains/alirt_acc/orthocontour.logfile";

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
//   cityimage.writeimage(
//      "features",features_twoDarray_ptr,false,ladarimage::p_data);

   twoDarray const *features_and_heights_twoDarray_ptr=
      urbanfunc::color_feature_heights(
      ztwoDarray_ptr,features_twoDarray_ptr);
   string features_and_heights_filename=
      cityimage.get_imagedir()+"features_and_heights.xyzp";   
   xyzpfunc::write_xyzp_data(
      ztwoDarray_ptr,features_and_heights_twoDarray_ptr,
      features_and_heights_filename);
   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
      features_and_heights_twoDarray_ptr,features_and_heights_filename);

// ==========================================================================
// Tree cluster network generation
// ==========================================================================

// Generate network containing linkedlists of connected treetop
// pixels:

   double min_tree_footprint_area=5;	// meters**2
   treefunc::trees_network_ptr=treefunc::generate_trees_network(
      cityimage.get_imagedir(),min_tree_footprint_area,
      ztwoDarray_ptr,features_twoDarray_ptr);
   cityimage.output_network_posns(treefunc::trees_network_ptr);

// ==========================================================================
// Building network generation
// ==========================================================================

// Generate network containing linkedlists of connected rooftop
// pixels:

   double min_footprint_area=10;	// meters**2
//   double min_footprint_area=100;	// meters**2
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

//   cityimage.draw_network_contour_cylinders(
//      features_and_heights_filename,cityimage.get_buildings_network_ptr());
//   cityimage.draw_network_subcontour_cylinders(
//      features_and_heights_filename,cityimage.get_buildings_network_ptr());
   delete features_and_heights_twoDarray_ptr;

// ==========================================================================
// Compute tree cluster and building shadows
// ==========================================================================

   twoDarray* shadows_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
   features_twoDarray_ptr->copy(shadows_twoDarray_ptr);

   const threevector ray_hat(
      cos(elevation)*cos(azimuth),cos(elevation)*sin(azimuth),sin(elevation));
   cityimage.compute_building_shadows(
      ray_hat,ztwoDarray_ptr,features_twoDarray_ptr,shadows_twoDarray_ptr);
   cityimage.compute_tree_shadows(
      treefunc::trees_network_ptr,ray_hat,ztwoDarray_ptr,
      features_twoDarray_ptr,shadows_twoDarray_ptr);
   cityimage.reset_nonground_feature_values(
      features_twoDarray_ptr,shadows_twoDarray_ptr);

   twoDarray* shadows_and_heights_twoDarray_ptr=
      urbanfunc::color_feature_heights(ztwoDarray_ptr,shadows_twoDarray_ptr);
   string shadows_and_heights_filename=cityimage.get_imagedir()
      +"shadows.xyzp";   
   xyzpfunc::write_xyzp_data(
      ztwoDarray_ptr,shadows_and_heights_twoDarray_ptr,
      shadows_and_heights_filename);
   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
      shadows_and_heights_twoDarray_ptr,shadows_and_heights_filename);

   urbanfunc::draw_3D_buildings(
      cityimage.get_buildings_network_ptr(),
      shadows_and_heights_filename,draw3Dfunc::annotation_value1);

// Construct xy grid coordinate system surrounding xyzp data:

   ladarfunc::draw_xy_coordinate_system(
      shadows_and_heights_filename,draw3Dfunc::annotation_value2,
      ztwoDarray_ptr);

   cityimage.summarize_results();
   cityimage.update_logfile("satellite");

   delete shadows_twoDarray_ptr;
   delete shadows_and_heights_twoDarray_ptr;
}


