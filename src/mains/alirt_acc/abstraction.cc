// ==========================================================================
// Program ABSTRACTION attempts to pull out basic information about
// buildings and road networks from ALIRT imagery.  It takes as input
// refined feature maps in which trees, rooftops, grass and asphalt
// have been classified on a per-pixel basis.  ABSTRACTION fits a
// parallelepiped to each building which contains useful location,
// orientation and size information.  It also computes Voronoi regions
// around each building as well as a Delaunay triangulation for the
// building network.  

// The Voronoi polygons act as seeds for the road network.
// ABSTRACTION identifies those Voronoi regions where asphalt pixels
// clearly indicate the front sides of the building parallelepipeds.
// It uses this information along with the reasonable requirement that
// a road not pass too closely between any two buildings to prune the
// Voronoi network and convert it into an initial estimate for the
// road network.  Candidate intersection roadpoints are identified and
// consolidated by this program.
// ==========================================================================
// Last updated on 7/5/05; 4/24/06; 6/14/06
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "urban/cityblockfuncs.h"
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
#include "urban/roadfuncs.h"
#include "urban/roadpoint.h"
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
   string logfilename=sysfunc::get_projectsrootdir()
      +"src/mains/alirt_acc/abstraction.logfile";

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

   string asphalt_filename=cityimage.get_imagedir()+"binary_asphalt.xyzp";
   twoDarray* binary_asphalt_twoDarray_ptr=featurefunc::
      generate_binary_asphalt_image(features_twoDarray_ptr);
   featurefunc::transfer_feature_pixels(
      featurefunc::building_sentinel_value,-1,features_twoDarray_ptr,
      binary_asphalt_twoDarray_ptr);
   featurefunc::transfer_feature_pixels(
      featurefunc::grass_sentinel_value,-0.25,features_twoDarray_ptr,
      binary_asphalt_twoDarray_ptr);
   featurefunc::transfer_feature_pixels(
      featurefunc::tree_sentinel_value,-0.1,features_twoDarray_ptr,
      binary_asphalt_twoDarray_ptr);

//   xyzpfunc::write_xyzp_data(
//      ztwoDarray_ptr,binary_asphalt_twoDarray_ptr,asphalt_filename,false);
//   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
//      binary_asphalt_twoDarray_ptr,asphalt_filename);

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

// Draw tree cluster COMs and cylinders:

   twoDarray* COMs_twoDarray_ptr=new twoDarray(
      features_and_heights_twoDarray_ptr);
   features_and_heights_twoDarray_ptr->copy(COMs_twoDarray_ptr);
//   cityimage.draw_network_posns(
//      treefunc::trees_network_ptr,COMs_twoDarray_ptr);

   string COMs_filename=cityimage.get_imagedir()+"tree_cylinders.xyzp";   
   xyzpfunc::write_xyzp_data(
      ztwoDarray_ptr,COMs_twoDarray_ptr,COMs_filename);
   cityimage.draw_network_contour_cylinders(
      COMs_filename,treefunc::trees_network_ptr);
   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
      COMs_twoDarray_ptr,COMs_filename);

// ==========================================================================
// Building network generation
// ==========================================================================

// Generate network containing linkedlists of connected rooftop
// pixels:

   double min_bldg_footprint_area=10;	// meters**2
   cityimage.generate_buildings_network(
      min_bldg_footprint_area,ztwoDarray_ptr,features_twoDarray_ptr);
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

// ==========================================================================
// Tree cluster Voronoi region & neighbor determination via Delaunay
// triangulation
// ==========================================================================

   cityimage.generate_site_voronoi_regions(
      treefunc::trees_network_ptr);

// Create another abstract map containing only tree cylinder contours
// + voronoi polygons:

//   string abstract_voronoi_filename=cityimage.get_imagedir()+"abstract_voronoi.xyzp";
//   filefunc::deletefile(abstract_voronoi_filename);
//   cityimage.draw_3D_buildings(abstract_voronoi_filename);
//   draw3Dfunc::draw_3D_voronoi_regions(
//      cityimage.get_data_bbox_ptr(),
//      abstract_voronoi_filename,cityimage.get_buildings_network_ptr(),
//      draw3Dfunc::annotation_value1);

   twoDarray* voronoi_twoDarray_ptr=new twoDarray(
      features_and_heights_twoDarray_ptr);
   features_and_heights_twoDarray_ptr->copy(voronoi_twoDarray_ptr);
   string voronoi_filename=cityimage.get_imagedir()+"tree_voronoi.xyzp";   
   cityimage.draw_site_voronoi_regions(
      voronoi_twoDarray_ptr,treefunc::trees_network_ptr);
   xyzpfunc::write_xyzp_data(
      ztwoDarray_ptr,voronoi_twoDarray_ptr,voronoi_filename);
   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
      voronoi_twoDarray_ptr,voronoi_filename);
//   cityimage.writeimage(
//      "tree_voronoi",voronoi_twoDarray_ptr,false,ladarimage::p_data);

// Generate and draw nearest neighbor links:

   voronoifunc::generate_nearest_neighbor_links(
      ztwoDarray_ptr,treefunc::trees_network_ptr);
   twoDarray* tree_neighbors_twoDarray_ptr=new twoDarray(
      features_and_heights_twoDarray_ptr);
   features_and_heights_twoDarray_ptr->copy(tree_neighbors_twoDarray_ptr);
   cityimage.draw_nearest_neighbor_links(
      tree_neighbors_twoDarray_ptr,treefunc::trees_network_ptr);
   string tree_neighbors_filename=cityimage.get_imagedir()+"tree_neighbors.xyzp";   
   xyzpfunc::write_xyzp_data(
      ztwoDarray_ptr,tree_neighbors_twoDarray_ptr,tree_neighbors_filename);
   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
      tree_neighbors_twoDarray_ptr,tree_neighbors_filename);
   delete tree_neighbors_twoDarray_ptr;

// Create another abstract map containing only tree pixels + nearest
// neighbor lines:

   string abstract_tree_neighbors_filename=cityimage.get_imagedir()+
      "abstract_tree_neighbors.xyzp";   
   filefunc::deletefile(abstract_tree_neighbors_filename);

   twoDarray* ztree_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
   twoDarray* ftree_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
   treefunc::draw_tree_cluster_pixels(
      treefunc::trees_network_ptr,ztwoDarray_ptr,ztree_twoDarray_ptr,
      ftree_twoDarray_ptr);
   twoDarray* fztree_twoDarray_ptr=urbanfunc::color_feature_heights(
      ztree_twoDarray_ptr,ftree_twoDarray_ptr);
//   delete ftree_twoDarray_ptr;
   cout << "Before writing xyzp data " << endl;
   xyzpfunc::write_xyzp_data(
      ztree_twoDarray_ptr,fztree_twoDarray_ptr,
      abstract_tree_neighbors_filename);
//   delete ztree_twoDarray_ptr;
//   delete fztree_twoDarray_ptr;

   cout << "Before drawing network contour cylinders" << endl;
   
   cityimage.draw_network_contour_cylinders(
      abstract_tree_neighbors_filename,treefunc::trees_network_ptr,0.4);
   cout << "Before drawing 3D nearest neighbor links" << endl;
   
   draw3Dfunc::draw_3D_nearest_neighbor_links(
      abstract_tree_neighbors_filename,treefunc::trees_network_ptr,
      draw3Dfunc::annotation_value2);
   cout << "Before appending fake xyzp points in twoDarray middle" << endl;
   
   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
      ztwoDarray_ptr,abstract_tree_neighbors_filename);

/*
   string abstract_tree_neighbors_filename=cityimage.get_imagedir()+
      "abstract_tree_neighbors.xyzp";   
   filefunc::deletefile(abstract_tree_neighbors_filename);

   cityimage.draw_network_contour_cylinders(
      abstract_tree_neighbors_filename,treefunc::trees_network_ptr,
      featurefunc::tree_sentinel_value);
   draw3Dfunc::draw_3D_nearest_neighbor_links(
      abstract_tree_neighbors_filename,treefunc::trees_network_ptr,
      draw3Dfunc::annotation_value2);
   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
      ztwoDarray_ptr,abstract_tree_neighbors_filename);
*/


// ==========================================================================
// Building 3D box fitting
// ==========================================================================

// Annotate height map with building box information:

   string bldg_boxes_filename=cityimage.get_imagedir()+"bldg_boxes.xyzp";   
//   xyzpfunc::write_xyzp_data(
//      ztwoDarray_ptr,features_twoDarray_ptr,bldg_boxes_filename);
//   xyzpfunc::write_xyzp_data(
//      ztwoDarray_ptr,ztwoDarray_ptr,bldg_boxes_filename,false);
   xyzpfunc::write_xyzp_data(
      ztwoDarray_ptr,features_and_heights_twoDarray_ptr,bldg_boxes_filename);

//   cityimage.draw_network_contour_cylinders(
//      features_and_heights_filename,cityimage.get_buildings_network_ptr());
//   cityimage.draw_network_subcontour_cylinders(
//      features_and_heights_filename,cityimage.get_buildings_network_ptr());
   urbanfunc::draw_3D_buildings(
      cityimage.get_buildings_network_ptr(),
      bldg_boxes_filename,draw3Dfunc::annotation_value1);

// Construct xy grid coordinate system surrounding xyzp data:

//   ladarfunc::draw_xy_coordinate_system(
//      features_and_heights_filename,draw3Dfunc::annotation_value2,ztwoDarray_ptr);

// Create another abstract map containing only building boxes:

   string abstract_bldg_boxes_filename=cityimage.get_imagedir()+
      "abstract_bldg_boxes.xyzp";   
   filefunc::deletefile(abstract_bldg_boxes_filename);
   urbanfunc::draw_3D_buildings(
      cityimage.get_buildings_network_ptr(),
      abstract_bldg_boxes_filename,draw3Dfunc::annotation_value1);

// ==========================================================================
// Building Voronoi region & neighbor determination via Delaunay
// triangulation
// ==========================================================================

   cityimage.generate_site_voronoi_regions(
      cityimage.get_buildings_network_ptr());

// Create another abstract map containing only building boxes +
// voronoi polygons:

   string abstract_voronoi_filename=cityimage.get_imagedir()+
      "abstract_voronoi.xyzp";
   filefunc::deletefile(abstract_voronoi_filename);
   urbanfunc::draw_3D_buildings(
      cityimage.get_buildings_network_ptr(),
      abstract_voronoi_filename,draw3Dfunc::annotation_value1);
   draw3Dfunc::draw_3D_voronoi_regions(
      cityimage.get_data_bbox_ptr(),
      abstract_voronoi_filename,cityimage.get_buildings_network_ptr(),
      draw3Dfunc::annotation_value1);

   features_and_heights_twoDarray_ptr->copy(voronoi_twoDarray_ptr);
   voronoi_filename=cityimage.get_imagedir()+"bldg_voronoi.xyzp";   
//   ladarfunc::write_annotated_xyzp_data(
//      ztwoDarray_ptr,ftwoDarray_ptr,voronoi_filename);
   cityimage.draw_site_voronoi_regions(
      voronoi_twoDarray_ptr,cityimage.get_buildings_network_ptr());
   xyzpfunc::write_xyzp_data(
      ztwoDarray_ptr,voronoi_twoDarray_ptr,voronoi_filename);
   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
      voronoi_twoDarray_ptr,voronoi_filename);
//   cityimage.writeimage(
//      "house_voronoi",voronoi_twoDarray_ptr,false,ladarimage::p_data);
   delete voronoi_twoDarray_ptr;

// Generate and draw nearest neighbor links:

   voronoifunc::generate_nearest_neighbor_links(
      ztwoDarray_ptr,cityimage.get_buildings_network_ptr());
   cityimage.get_buildings_network_ptr()->sort_all_site_neighbors();
 
   twoDarray* bldgs_network_twoDarray_ptr=
      new twoDarray(features_and_heights_twoDarray_ptr);
   features_and_heights_twoDarray_ptr->copy(bldgs_network_twoDarray_ptr);
   cityimage.draw_nearest_neighbor_links(
      bldgs_network_twoDarray_ptr,cityimage.get_buildings_network_ptr());
   string bldgs_network_filename=cityimage.get_imagedir()+"bldg_network.xyzp";   
   xyzpfunc::write_xyzp_data(
      ztwoDarray_ptr,bldgs_network_twoDarray_ptr,bldgs_network_filename);
   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
      bldgs_network_twoDarray_ptr,bldgs_network_filename);
//   urbanfunc::annotate_building_labels(
//      cityimage.get_buildings_network_ptr(),bldgs_network_filename);
   delete bldgs_network_twoDarray_ptr;

// Create another abstract map containing only building boxes +
// neighbor lines:

   string abstract_bldg_neighbors_filename=cityimage.get_imagedir()+
      "abstract_bldg_neighbors.xyzp";   
   filefunc::deletefile(abstract_bldg_neighbors_filename);
   urbanfunc::draw_3D_buildings(
      cityimage.get_buildings_network_ptr(),
      abstract_bldg_neighbors_filename,draw3Dfunc::annotation_value1);
   draw3Dfunc::draw_3D_nearest_neighbor_links(
      abstract_bldg_neighbors_filename,
      cityimage.get_buildings_network_ptr(),draw3Dfunc::annotation_value1);

// Superpose building boxes, tree cylinder contours and their
// corresponding nearest neighbor lines all within a single abstract
// map:

   string abstract_networks_filename=cityimage.get_imagedir()+
      "abstract_networks.xyzp";   
   filefunc::deletefile(abstract_networks_filename);

   treefunc::draw_tree_cluster_pixels(
      treefunc::trees_network_ptr,ztwoDarray_ptr,ztree_twoDarray_ptr,
      ftree_twoDarray_ptr);
   fztree_twoDarray_ptr=urbanfunc::color_feature_heights(
      ztree_twoDarray_ptr,ftree_twoDarray_ptr);
   delete ftree_twoDarray_ptr;
   xyzpfunc::write_xyzp_data(
      ztree_twoDarray_ptr,fztree_twoDarray_ptr,abstract_networks_filename);
   delete ztree_twoDarray_ptr;
   delete fztree_twoDarray_ptr;

   cityimage.draw_network_contour_cylinders(
      abstract_networks_filename,treefunc::trees_network_ptr,0.4);
   draw3Dfunc::draw_3D_nearest_neighbor_links(
      abstract_networks_filename,treefunc::trees_network_ptr,
      draw3Dfunc::annotation_value2);

   urbanfunc::draw_3D_buildings(
      cityimage.get_buildings_network_ptr(),
      abstract_networks_filename,draw3Dfunc::annotation_value1);
   draw3Dfunc::draw_3D_nearest_neighbor_links(
      abstract_networks_filename,cityimage.get_buildings_network_ptr(),
      draw3Dfunc::annotation_value1);
   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
      ztwoDarray_ptr,abstract_networks_filename);

// Establish some building fronts based upon asphalt feature information:

   cityimage.compute_asphalt_distribution_relative_to_buildings(
      features_twoDarray_ptr);
   twoDarray* fasphalt_twoDarray_ptr=featurefunc::cull_feature_pixels(
      featurefunc::road_sentinel_value,features_twoDarray_ptr);
   string abstract_bldg_orientation_filename=cityimage.get_imagedir()+
      "abstract_bldg_orientation.xyzp";   
//   filefunc::deletefile(abstract_bldg_orientation_filename);
   xyzpfunc::write_xyzp_data(
      ztwoDarray_ptr,fasphalt_twoDarray_ptr,
      abstract_bldg_orientation_filename);
   delete fasphalt_twoDarray_ptr;
   urbanfunc::draw_3D_buildings(
      cityimage.get_buildings_network_ptr(),
      abstract_bldg_orientation_filename,draw3Dfunc::annotation_value1);
//   draw3Dfunc::draw_3D_voronoi_regions(
//      cityimage.get_data_bbox_ptr(),abstract_bldg_orientation_filename,
//      cityimage.get_buildings_network_ptr(),draw3Dfunc::annotation_value1);
   urbanfunc::draw_3D_building_front_dirs(
      cityimage.get_buildings_network_ptr(),
      abstract_bldg_orientation_filename);

   delete features_and_heights_twoDarray_ptr;
   delete binary_asphalt_twoDarray_ptr;
   
   cityimage.summarize_results();
   cityimage.update_logfile("abstraction");
}


