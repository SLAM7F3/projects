// ==========================================================================
// Program TREES generates a trees network starting from a cleaned
// ALIRT pixel-level feature map.  It wraps deformable contours around
// the base of individual tree clusters, and then computes local
// height information along the contour.  Tree clusters are
// consequently represented by variable height cylinders.  This
// cylinder information is written to an output ascii file which can
// later be read in by program MULTINETS.  Voronoi regions and
// neighbor links are also calculated by program TREES.
// ==========================================================================
// Last updated on 5/22/05; 4/24/06; 6/14/06
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
#include "urban/treefuncs.h"
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
      +"src/mains/alirt_acc/trees.logfile";

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
//   xyzpfunc::write_xyzp_data(
//      ztwoDarray_ptr,features_and_heights_twoDarray_ptr,
//      features_and_heights_filename);
//   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
//      features_and_heights_twoDarray_ptr,features_and_heights_filename);

// ==========================================================================
// Tree cluster network generation
// ==========================================================================

// Generate network containing linkedlists of connected treetop
// pixels:

   double min_tree_footprint_area=5;	// meters**2
   treefunc::trees_network_ptr=treefunc::generate_trees_network(
      cityimage.get_imagedir(),min_tree_footprint_area,ztwoDarray_ptr,
      features_twoDarray_ptr);

// Write out trees network information to ascii text file:

   string trees_network_filename=cityimage.get_imagedir()+"trees_network.txt";
   treefunc::output_trees_network_to_textfile(
      treefunc::trees_network_ptr,trees_network_filename);
//   cityimage.output_network_posns(treefunc::trees_network_ptr);

// Draw tree cluster cylinders:

   twoDarray* treecyls_twoDarray_ptr=new twoDarray(
      features_and_heights_twoDarray_ptr);
   features_and_heights_twoDarray_ptr->copy(treecyls_twoDarray_ptr);
//   cityimage.draw_network_posns(
//      treefunc::trees_network_ptr,treecyls_twoDarray_ptr);

   string tree_cyls_filename=cityimage.get_imagedir()+"tree_cylinders.xyzp";   
   xyzpfunc::write_xyzp_data(
      ztwoDarray_ptr,treecyls_twoDarray_ptr,tree_cyls_filename);
   cityimage.draw_network_contour_cylinders(
      tree_cyls_filename,treefunc::trees_network_ptr,
      draw3Dfunc::annotation_value2);

// Next line for 2006 ISDS abstract generation purposes only:

//   cityimage.draw_network_contour_cylinders(
//      tree_cyls_filename,treefunc::trees_network_ptr,0.4);

//   urbanfunc::annotate_treecluster_labels(
//      treefunc::trees_network_ptr,tree_cyls_filename,
//      draw3Dfunc::annotation_value2);
   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
      treecyls_twoDarray_ptr,tree_cyls_filename);

   exit(-1);

// ==========================================================================
// Tree cluster Voronoi region & neighbor determination via Delaunay
// triangulation
// ==========================================================================

   cityimage.generate_site_voronoi_regions(treefunc::trees_network_ptr);

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
   delete ftree_twoDarray_ptr;
   xyzpfunc::write_xyzp_data(
      ztree_twoDarray_ptr,fztree_twoDarray_ptr,
      abstract_tree_neighbors_filename);
   delete ztree_twoDarray_ptr;
   delete fztree_twoDarray_ptr;

   
   cityimage.draw_network_contour_cylinders(
      abstract_tree_neighbors_filename,treefunc::trees_network_ptr,0.4);
   draw3Dfunc::draw_3D_nearest_neighbor_links(
      abstract_tree_neighbors_filename,treefunc::trees_network_ptr,
      draw3Dfunc::annotation_value2);
   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
      ztwoDarray_ptr,abstract_tree_neighbors_filename);

/*
   string abstract_tree_neighbors_filename=cityimage.get_imagedir()+
      "abstract_tree_neighbors.xyzp";   
   filefunc::deletefile(abstract_tree_neighbors_filename);

   cityimage.draw_network_contour_cylinders(
      abstract_tree_neighbors_filename,treefunc::trees_network_ptr,
      urbanimage::tree_sentinel_value);
   draw3Dfunc::draw_3D_nearest_neighbor_links(
      abstract_tree_neighbors_filename,treefunc::trees_network_ptr,
      draw3Dfunc::annotation_value2);
   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
      ztwoDarray_ptr,abstract_tree_neighbors_filename);
*/

   delete features_and_heights_twoDarray_ptr;
   treefunc::delete_trees_network(treefunc::trees_network_ptr);

   cityimage.summarize_results();
   cityimage.update_logfile("trees");
}


