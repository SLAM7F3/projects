// ==========================================================================
// Program STREETS constructs a high-level road network starting from
// a cleaned ALIRT pixel-level feature map.  It first generates a new
// feature map with score values favoring asphalt and disfavoring
// buildings, trees and grass regions.  STREETS next generates the
// network for building centers-of-mass as well as their Voronoi
// diagram.  It uses the distribution of asphalt in the vicinity of
// individual buildings to identify some building front directions.
// STREETS next generates a roadpoints network starting from the
// buildings Voronoi diagram.  It uses feature information to prune
// away most of the links within the Voronoi diagram to come up with a
// reasonable first estimate for the road network.

// Program STREETS next generates a preliminary city block map and
// computes city block contours.  It uses this cityblock information
// to identify building "islands" which are completely surrounded by
// roadways.  It subsequently instantiates a road intersections
// network generated from roadpoints which have 3 or more netlinks to
// other roadpoints.  The location of each intersection roadpoint is
// scored based upon the product of think line integrals of asphalt
// feature information running from the intersection to all its
// neighbors.  The intersection locations are perturbed until this
// score function is maximized.  Following this optimization,
// intersection points lying too close together are merged, and links
// with low fractional asphalt content are deleted from the
// intersection roadpoints network.

// Finally, city block information is refined based upon road
// intersections network, and a network of city blocks is dynamically
// generated.  Blocks containing no buildings are eliminated, and
// buildings are assigned cityblock IDs.  Intersection network
// information is written to ascii text file output in a form which
// can be read in by other main programs.

// 				streets t.45-51

// ==========================================================================
// Last updated on 5/22/05; 4/24/06; 6/14/06
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "urban/cityblock.h"
#include "urban/cityblockfuncs.h"
#include "image/connectfuncs.h"
#include "threeDgraphics/draw3Dfuncs.h"
#include "ladar/featurefuncs.h"
#include "general/filefuncs.h"
#include "geometry/geometry_funcs.h"
#include "image/graphicsfuncs.h"
#include "ladar/groundfuncs.h"
#include "datastructures/Hashtable.h"
#include "image/imagefuncs.h"
#include "ladar/ladarfuncs.h"
#include "templates/mytemplates.h"
#include "general/outputfuncs.h"
#include "geometry/parallelogram.h"
#include "image/recursivefuncs.h"
#include "urban/roadfuncs.h"
#include "urban/roadpoint.h"
#include "general/stringfuncs.h"
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
      +"src/mains/alirt_acc/streets.logfile";

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
   cityimage.parse_and_store_input_data(delta_x,delta_y);
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
//   double dist_from_bbox=10;	// meters
//   ladarfunc::color_points_near_data_bbox(
//      dist_from_bbox,cityimage.get_data_bbox_ptr(),
//      features_and_heights_twoDarray_ptr);
//   string features_and_heights_filename=
//      cityimage.get_imagedir()+"features_and_heights.xyzp";   
//   xyzpfunc::write_xyzp_data(
//      ztwoDarray_ptr,features_and_heights_twoDarray_ptr,
//      features_and_heights_filename);
//   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
//      features_and_heights_twoDarray_ptr,features_and_heights_filename);

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
// Building network generation
// ==========================================================================

// Generate network containing linkedlists of connected rooftop
// pixels:

   double min_footprint_area=10;	// meters**2
   cityimage.generate_buildings_network(
      min_footprint_area,ztwoDarray_ptr,features_twoDarray_ptr);
//   urbanfunc::annotate_particular_building(
//      cityimage.get_buildings_network_ptr(),130,
//      features_and_heights_filename);
//   cityimage.draw_network_contour_cylinders(
//      features_and_heights_filename,cityimage.get_buildings_network_ptr());

// Write out and draw building COM info:

//   cityimage.output_network_posns(cityimage.get_buildings_network_ptr());
//   twoDarray* COMs_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
//   features_and_heights_twoDarray_ptr->copy(COMs_twoDarray_ptr);
//   cityimage.draw_network_posns(
//      cityimage.get_buildings_network_ptr(),COMs_twoDarray_ptr);
//   string COMs_filename=cityimage.get_imagedir()+"bldg_coms.xyzp";   
//   ladarfunc::write_annotated_xyzp_data(
//      ztwoDarray_ptr,COMs_twoDarray_ptr,COMs_filename);
//   xyzpfunc::write_xyzp_data(
//      ztwoDarray_ptr,COMs_twoDarray_ptr,COMs_filename);
//   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
//      COMs_twoDarray_ptr,COMs_filename);
//   urbanfunc::annotate_particular_building(
//      54,cityimage.get_buildings_network_ptr(),COMs_filename);
//   delete COMs_twoDarray_ptr;

//   string COMs_filename=cityimage.get_imagedir()+"bldg_coms.xyzp";   
//   xyzpfunc::write_xyzp_data(
//      ztwoDarray_ptr,features_and_heights_twoDarray_ptr,COMs_filename);
//   cityimage.draw_3D_building_COMs(COMs_filename);

// ==========================================================================
// Building Voronoi region & neighbor determination via Delaunay
// triangulation
// ==========================================================================

   cityimage.generate_site_voronoi_regions(
      cityimage.get_buildings_network_ptr());

   twoDarray* voronoi_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
   features_and_heights_twoDarray_ptr->copy(voronoi_twoDarray_ptr);
   string voronoi_filename=cityimage.get_imagedir()+"bldg_voronoi.xyzp";   
   cityimage.draw_site_voronoi_regions(
      voronoi_twoDarray_ptr,cityimage.get_buildings_network_ptr());
//   xyzpfunc::write_xyzp_data(
//      ztwoDarray_ptr,voronoi_twoDarray_ptr,voronoi_filename);
//  draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
//      voronoi_twoDarray_ptr,voronoi_filename);
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
//   xyzpfunc::write_xyzp_data(
//      ztwoDarray_ptr,bldgs_network_twoDarray_ptr,bldgs_network_filename);
//   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
//      bldgs_network_twoDarray_ptr,bldgs_network_filename);
//   urbanfunc::annotate_building_labels(
//      cityimage.get_buildings_network_ptr(),bldgs_network_filename);
   delete bldgs_network_twoDarray_ptr;

// Establish some building fronts based upon asphalt feature information:

   cityimage.compute_asphalt_distribution_relative_to_buildings(
      features_twoDarray_ptr);
//   twoDarray* fasphalt_twoDarray_ptr=featurefunc::cull_feature_pixels(
//      featurefunc::road_sentinel_value,features_twoDarray_ptr);
//   string abstract_bldg_orientation_filename=cityimage.get_imagedir()+
//      "abstract_bldg_orientation.xyzp";   
//   xyzpfunc::write_xyzp_data(
//      ztwoDarray_ptr,fasphalt_twoDarray_ptr,
//      abstract_bldg_orientation_filename);
//   delete fasphalt_twoDarray_ptr;
//   cityimage.draw_3D_building_boxes(abstract_bldg_orientation_filename);
//   cityimage.draw_3D_voronoi_regions(abstract_bldg_orientation_filename);
//   cityimage.draw_3D_bldg_orientations(abstract_bldg_orientation_filename);

// ==========================================================================
// Road network generation
// ==========================================================================

// Instantiate twoDarray holding only asphalt features with all others
// set to zero:

   twoDarray* just_asphalt_twoDarray_ptr=featurefunc::cull_feature_pixels(
      featurefunc::road_sentinel_value,0.0,features_twoDarray_ptr);

   cityimage.generate_roadpoints_network(ztwoDarray_ptr);
   cityimage.get_roadpoints_network_ptr()->sort_all_site_neighbors();

   cityimage.delete_roadlinks_too_close_to_buildings(
      just_asphalt_twoDarray_ptr);
   cityimage.delete_roadlinks_passing_thru_buildings(features_twoDarray_ptr);
   cityimage.delete_lonely_roadpoints();

   cityimage.prune_roadpoints_at_infinity();
   cityimage.delete_lonely_roadpoints();
   cityimage.compute_asphalt_angular_distribution_relative_to_buildings(
      features_twoDarray_ptr);
//   cityimage.display_street_islands_and_peninsulas(
//      features_and_heights_filename);
   cityimage.delete_roadlinks_behind_buildings(
      7,30,5,just_asphalt_twoDarray_ptr);
   delete just_asphalt_twoDarray_ptr;
   cityimage.delete_lonely_roadpoints();

//   cityimage.identify_front_roadpoint_seeds();
//   cityimage.propagate_roadpoint_frontness();

// Add intersection points to road network:

   roadfunc::find_intersection_roadpoints(
      cityimage.get_roadpoints_network_ptr(),
      cityimage.get_data_bbox_ptr());
   roadfunc::define_intersections_on_data_bbox(
      cityimage.get_roadpoints_network_ptr(),
      cityimage.get_data_bbox_ptr());
   roadfunc::delete_roadpoints_outside_data_bbox(
      cityimage.get_roadpoints_network_ptr(),
      cityimage.get_data_bbox_ptr());
   roadfunc::adjust_bbox_intersection_roadpoints(
      cityimage.get_roadpoints_network_ptr(),
      cityimage.get_data_bbox_ptr());
   roadfunc::consolidate_roadpoints_close_to_intersections(
      cityimage.get_roadpoints_network_ptr());

//   twoDarray* roadnet_twoDarray_ptr=new twoDarray(
//      features_and_heights_twoDarray_ptr);
//   features_and_heights_twoDarray_ptr->copy(roadnet_twoDarray_ptr);
//   roadfunc::draw_road_network(cityimage.get_roadpoints_network_ptr(),
//                               roadnet_twoDarray_ptr,true,false);
//   string network_filename=cityimage.get_imagedir()+"prelim_road_network.xyzp";   
//   xyzpfunc::write_xyzp_data(
//      ztwoDarray_ptr,roadnet_twoDarray_ptr,network_filename);
//   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
//      roadnet_twoDarray_ptr,network_filename);
//   roadfunc::annotate_roadpoint_labels(
//      cityimage.get_roadpoints_network_ptr(),network_filename,
//      draw3Dfunc::annotation_value1);
//   delete roadnet_twoDarray_ptr;

// Generate city block map and city block contours:

   twoDarray* cityblock_regions_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);

   cityblockfunc::generate_cityblocks_network(
      cityimage.get_imagedir(),cityimage.get_data_bbox_ptr(),
      cityimage.get_roadpoints_network_ptr(),ztwoDarray_ptr,
      cityblock_regions_twoDarray_ptr);

// Assign city block IDs to buildings & identify building islands:

   cityblockfunc::set_building_cityblock_IDs(
      cityimage.get_buildings_network_ptr(),
      cityblock_regions_twoDarray_ptr);
   cityblockfunc::identify_building_islands(
      cityimage.get_buildings_network_ptr(),
      cityblock_regions_twoDarray_ptr);

//   string cityblock_regions_filename=cityimage.get_imagedir()+
//      "raw_cityblock_regions.xyzp";
//   xyzpfunc::write_xyzp_data(
//      ztwoDarray_ptr,cityblock_regions_twoDarray_ptr,
//      cityblock_regions_filename,false);
//   delete cityblock_regions_twoDarray_ptr;
//   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
//      ztwoDarray_ptr,cityblock_regions_filename);
//   urbanfunc::annotate_building_labels(
//      cityimage.get_buildings_network_ptr(),cityblock_regions_filename,
//      draw3Dfunc::annotation_value1,true);

   string cityblock_contours_filename=cityimage.get_imagedir()+
      "cityblocks_contours.xyzp";   
   xyzpfunc::write_xyzp_data(
      ztwoDarray_ptr,features_and_heights_twoDarray_ptr,
      cityblock_contours_filename,true,false);
   for (int c=0; c<cityblockfunc::cityblocks_network_ptr->size(); c++)
   {
      cityblock* curr_cityblock_ptr=cityblockfunc::cityblocks_network_ptr->
         get_site_data_ptr(c);
      contour* curr_contour_ptr=curr_cityblock_ptr->get_contour_ptr();
      curr_contour_ptr->translate(threevector(0,0,5));
      draw3Dfunc::draw_thick_lines=true;
      draw3Dfunc::draw_contour(
         *curr_contour_ptr,cityblock_contours_filename,
         draw3Dfunc::annotation_value1);
      draw3Dfunc::draw_thick_lines=false;
   }
   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
      ztwoDarray_ptr,cityblock_contours_filename);
   urbanfunc::annotate_street_islands_and_peninsulas(
      cityimage.get_buildings_network_ptr(),cityblock_contours_filename);

// ==========================================================================
// Road intersections network generation
// ==========================================================================

   roadfunc::generate_intersections_network(
      cityimage.get_roadpoints_network_ptr(),
      cityimage.get_data_bbox_ptr());
   roadfunc::intersections_network_ptr->sort_all_site_neighbors();
   cityblockfunc::identify_roadpoints_near_building_islands(
      cityimage.get_buildings_network_ptr(),
      roadfunc::intersections_network_ptr);

   roadfunc::insert_fake_intersections_nearby_bldg_islands();

   roadfunc::improve_intersections_network(
      4,cityimage.get_data_bbox_ptr(),binary_asphalt_twoDarray_ptr);
   roadfunc::insert_sites_at_netlink_intersections(
      roadfunc::intersections_network_ptr);
   roadfunc::consolidate_roadpoints_close_to_intersections(
      roadfunc::intersections_network_ptr,3);
   roadfunc::intersections_network_ptr->merge_close_sites_and_links(2.0);
   roadfunc::delete_low_score_segments(0.20,binary_asphalt_twoDarray_ptr);

   const int max_iters=10;
   for (int iter=0; iter<max_iters; iter++)
   {
      roadfunc::improve_intersections_network(
         2,cityimage.get_data_bbox_ptr(),binary_asphalt_twoDarray_ptr);
      roadfunc::insert_sites_at_netlink_intersections(
         roadfunc::intersections_network_ptr);
      roadfunc::consolidate_roadpoints_close_to_intersections(
         roadfunc::intersections_network_ptr,2);
      roadfunc::intersections_network_ptr->merge_close_sites_and_links(3.0);
      roadfunc::delete_low_score_segments(0.25,binary_asphalt_twoDarray_ptr);
   }

// Before the intersections network is written to an output XYZP file,
// reset all intersection roadpoint flags to false.  Then recompute
// which roadpoints within the sites within *intersections_network_ptr
// are genuine intersections:

   roadfunc::reset_intersection_roadpoints(
      roadfunc::intersections_network_ptr);
   roadfunc::find_intersection_roadpoints(
      roadfunc::intersections_network_ptr,
      cityimage.get_data_bbox_ptr());
   roadfunc::move_roadpoints_near_data_bbox_onto_bbox(
      roadfunc::intersections_network_ptr,
      cityimage.get_data_bbox_ptr());
   roadfunc::test_linear_links_between_intersections(
      roadfunc::intersections_network_ptr,cityimage.get_data_bbox_ptr(),
      binary_asphalt_twoDarray_ptr);
   roadfunc::intersections_network_ptr->sort_all_site_neighbors();
 
// ==========================================================================
// Refine city blocks based upon road intersections network
// ==========================================================================

// Generate city block map and city block contours:

   cityblock_regions_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
   delete cityblockfunc::cityblocks_network_ptr;
   cityblockfunc::generate_cityblocks_network(
      cityimage.get_imagedir(),cityimage.get_data_bbox_ptr(),
         roadfunc::intersections_network_ptr,ztwoDarray_ptr,
         cityblock_regions_twoDarray_ptr);

// Assign city block IDs to buildings & identify building islands:

   cityblockfunc::set_building_cityblock_IDs(
      cityimage.get_buildings_network_ptr(),
      cityblock_regions_twoDarray_ptr);
   cityblockfunc::eliminate_blocks_with_no_buildings(
      cityimage.get_buildings_network_ptr(),
      roadfunc::intersections_network_ptr,cityblock_regions_twoDarray_ptr);

// Write out road intersection network information to ascii text file:

   string intersections_network_filename=cityimage.get_imagedir()+
      "intersections_network.txt";
   filefunc::deletefile(intersections_network_filename);
   roadfunc::output_road_network_to_textfile(
      roadfunc::intersections_network_ptr,cityimage.get_data_bbox_ptr(),
      intersections_network_filename);

// Write final road intersections network to XYZP output file:

   twoDarray* intersections_twoDarray_ptr=new twoDarray(
      features_and_heights_twoDarray_ptr);
   features_and_heights_twoDarray_ptr->copy(intersections_twoDarray_ptr);

   string intersections_filename=cityimage.get_imagedir()+"intersections.xyzp";   
   filefunc::deletefile(intersections_filename);
   bool display_roadpoints_flag=false;
   roadfunc::draw_road_network(
      roadfunc::intersections_network_ptr,intersections_twoDarray_ptr,
      display_roadpoints_flag,false);
   xyzpfunc::write_xyzp_data(
      ztwoDarray_ptr,intersections_twoDarray_ptr,intersections_filename);
   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
      intersections_twoDarray_ptr,intersections_filename);
//   roadfunc::draw_road_network(
//      copied_intersections_network_ptr,intersections_filename,
//      draw3Dfunc::annotation_value2);
//   roadfunc::annotate_roadpoint_labels(
//      roadfunc::intersections_network_ptr,intersections_filename,
//      draw3Dfunc::annotation_value1);

   string cityblock_regions_filename=cityimage.get_imagedir()+
      "cityblock_regions.xyzp";
   twoDarray* recolored_blocks_twoDarray_ptr=
      cityblockfunc::recolor_cityblocks(cityblock_regions_twoDarray_ptr);
   xyzpfunc::write_xyzp_data(
      ztwoDarray_ptr,recolored_blocks_twoDarray_ptr,
      cityblock_regions_filename,false);
   delete recolored_blocks_twoDarray_ptr;
   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
      ztwoDarray_ptr,cityblock_regions_filename);
//   cityblockfunc::annotate_block_labels(cityblock_regions_filename);

   delete cityblock_regions_twoDarray_ptr;
   delete features_and_heights_twoDarray_ptr;
   delete binary_asphalt_twoDarray_ptr;
   delete cityblockfunc::cityblocks_network_ptr;
   roadfunc::delete_roadpoints_network(roadfunc::intersections_network_ptr);

   cityimage.summarize_results();
   cityimage.update_logfile("streets");
}
