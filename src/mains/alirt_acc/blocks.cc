// ==========================================================================
// Program BLOCKS
// ==========================================================================
// Last updated on 12/3/04; 6/14/06
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "ladar/cityblockfuncs.h"
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
#include "general/outputfuncs.h"
#include "geometry/parallelogram.h"
#include "image/recursivefuncs.h"
#include "ladar/roadfuncs.h"
#include "ladar/roadpoint.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "ladar/tree_cluster.h"
#include "image/TwoDarray.h"
#include "ladar/urbanfuncs.h"
#include "ladar/urbanimage.h"
#include "geometry/voronoifuncs.h"

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
   string logfilename=sysfunc::get_cplusplusrootdir()
      +"alirt/abstraction.logfile";

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
   cityimage.compute_data_bbox(cityimage.z2Darray_ptr,false);

// Eliminate junk nearby edges of data bounding box:

   ladarfunc::crop_data_inside_bbox(0.01,cityimage.get_data_bbox_ptr(),
                                    cityimage.z2Darray_ptr);
   ladarfunc::crop_data_inside_bbox(0.01,cityimage.get_data_bbox_ptr(),
                                    cityimage.get_p2Darray_ptr());

   twoDarray const *ztwoDarray_ptr=cityimage.z2Darray_ptr;
   twoDarray const *features_twoDarray_ptr=cityimage.get_p2Darray_ptr();

   string features_filename=cityimage.imagedir+"features.xyzp";   
//   ladarfunc::write_xyzp_data(
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
//      cityimage.imagedir+"features_and_heights.xyzp";   
//   ladarfunc::write_xyzp_data(
//      ztwoDarray_ptr,features_and_heights_twoDarray_ptr,
//      features_and_heights_filename);
//   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
//      features_and_heights_twoDarray_ptr,features_and_heights_filename);

   string asphalt_filename=cityimage.imagedir+"binary_asphalt.xyzp";
   twoDarray* binary_asphalt_twoDarray_ptr=featurefunc::
      generate_binary_asphalt_image(features_twoDarray_ptr);
   featurefunc::transfer_feature_pixels(
      urbanimage::building_sentinel_value,-1,features_twoDarray_ptr,
      binary_asphalt_twoDarray_ptr);
   featurefunc::transfer_feature_pixels(
      urbanimage::grass_sentinel_value,-0.25,features_twoDarray_ptr,
      binary_asphalt_twoDarray_ptr);
   featurefunc::transfer_feature_pixels(
      urbanimage::tree_sentinel_value,-0.1,features_twoDarray_ptr,
      binary_asphalt_twoDarray_ptr);

//   ladarfunc::write_xyzp_data(
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
//   urbanfunc::annotate_building_labels(
//      cityimage.get_buildings_network_ptr(),features_and_heights_filename,
//      draw3Dfunc::annotation_value1);
//   urbanfunc::annotate_particular_building(
//      cityimage.get_buildings_network_ptr(),130,
//      features_and_heights_filename,draw3Dfunc::annotation_value1);
//   cityimage.draw_network_contour_cylinders(
//      features_and_heights_filename,cityimage.get_buildings_network_ptr());

// Write out and draw building COM info:

/*
   cityimage.output_network_posns(cityimage.get_buildings_network_ptr());
   twoDarray* COMs_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
   features_and_heights_twoDarray_ptr->copy(COMs_twoDarray_ptr);
   cityimage.draw_network_posns(
      cityimage.get_buildings_network_ptr(),COMs_twoDarray_ptr);
   string COMs_filename=cityimage.imagedir+"bldg_coms.xyzp";   
//   ladarfunc::write_annotated_xyzp_data(
//      ztwoDarray_ptr,COMs_twoDarray_ptr,COMs_filename);
//   ladarfunc::write_xyzp_data(
//      ztwoDarray_ptr,COMs_twoDarray_ptr,COMs_filename);
//   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
//      COMs_twoDarray_ptr,COMs_filename);

   delete COMs_twoDarray_ptr;
*/

//   string COMs_filename=cityimage.imagedir+"bldg_coms.xyzp";   
//   ladarfunc::write_xyzp_data(
//      ztwoDarray_ptr,features_and_heights_twoDarray_ptr,COMs_filename);
//   cityimage.draw_3D_building_COMs(COMs_filename);

// ==========================================================================
// Building Voronoi region & neighbor determination via Delaunay
// triangulation
// ==========================================================================

   cityimage.generate_site_voronoi_regions(
      cityimage.get_buildings_network_ptr());

//   twoDarray* voronoi_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
//   features_and_heights_twoDarray_ptr->copy(voronoi_twoDarray_ptr);
//   string voronoi_filename=cityimage.imagedir+"bldg_voronoi.xyzp";   
//   cityimage.draw_site_voronoi_regions(
//      voronoi_twoDarray_ptr,cityimage.get_buildings_network_ptr());
//   ladarfunc::write_xyzp_data(
//      ztwoDarray_ptr,voronoi_twoDarray_ptr,voronoi_filename);
//   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
//      voronoi_twoDarray_ptr,voronoi_filename);
//   delete voronoi_twoDarray_ptr;

// Generate and draw nearest neighbor links:

   voronoifunc::generate_nearest_neighbor_links(
      ztwoDarray_ptr,cityimage.get_buildings_network_ptr());
/*
   twoDarray* neighbors_twoDarray_ptr=
      new twoDarray(features_and_heights_twoDarray_ptr);
   features_and_heights_twoDarray_ptr->copy(neighbors_twoDarray_ptr);
   cityimage.draw_nearest_neighbor_links(
      neighbors_twoDarray_ptr,cityimage.get_buildings_network_ptr());
   string neighbors_filename=cityimage.imagedir+"bldg_neighbors.xyzp";   
   ladarfunc::write_xyzp_data(
      ztwoDarray_ptr,neighbors_twoDarray_ptr,neighbors_filename);
   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
      neighbors_twoDarray_ptr,neighbors_filename);
   delete neighbors_twoDarray_ptr;
*/

// Establish some building fronts based upon asphalt feature information:

   cityimage.compute_asphalt_distribution_relative_to_buildings(
      features_twoDarray_ptr);
//   twoDarray* fasphalt_twoDarray_ptr=featurefunc::cull_feature_pixels(
//      urbanimage::road_sentinel_value,features_twoDarray_ptr);
//   string abstract_bldg_orientation_filename=cityimage.imagedir+
//      "abstract_bldg_orientation.xyzp";   
//   ladarfunc::write_xyzp_data(
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
      urbanimage::road_sentinel_value,0.0,features_twoDarray_ptr);

   cityimage.generate_roadpoints_network(ztwoDarray_ptr);

   cityimage.delete_roadlinks_too_close_to_buildings(
      just_asphalt_twoDarray_ptr);
   cityimage.delete_roadlinks_passing_thru_buildings(features_twoDarray_ptr);
   cityimage.delete_lonely_roadpoints();

   cityimage.prune_roadpoints_at_infinity();
   cityimage.delete_lonely_roadpoints();
//   cityimage.delete_roadpoints_too_far_from_asphalt(features_twoDarray_ptr);
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

   twoDarray* roadnet_twoDarray_ptr=new twoDarray(
      features_and_heights_twoDarray_ptr);
   features_and_heights_twoDarray_ptr->copy(roadnet_twoDarray_ptr);

   roadfunc::draw_road_network(cityimage.get_roadpoints_network_ptr(),
                               roadnet_twoDarray_ptr);

   string network_filename=cityimage.imagedir+"road_network.xyzp";   
//   ladarfunc::write_xyzp_data(
//      ztwoDarray_ptr,roadnet_twoDarray_ptr,network_filename);
//   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
//      roadnet_twoDarray_ptr,network_filename);
//   roadfunc::annotate_roadpoint_labels(
//      cityimage.get_roadpoints_network_ptr(),network_filename,
//      draw3Dfunc::annotation_value1);
   delete roadnet_twoDarray_ptr;

/*
// Convert roadpoints network into a network of road intersections:

   roadfunc::generate_intersections_network(
      cityimage.get_roadpoints_network_ptr(),
      cityimage.get_data_bbox_ptr());
   roadfunc::improve_intersections_network(
      4,cityimage.get_data_bbox_ptr(),binary_asphalt_twoDarray_ptr);
   roadfunc::consolidate_roadpoints_close_to_intersections(
      roadfunc::intersections_network_ptr,3);
   roadfunc::delete_low_score_segments(binary_asphalt_twoDarray_ptr);
   roadfunc::improve_intersections_network(
      2,cityimage.get_data_bbox_ptr(),binary_asphalt_twoDarray_ptr);
   roadfunc::consolidate_roadpoints_close_to_intersections(
      roadfunc::intersections_network_ptr,2);
   roadfunc::delete_low_score_segments(binary_asphalt_twoDarray_ptr);

   twoDarray* intersections_twoDarray_ptr=new twoDarray(
      features_and_heights_twoDarray_ptr);
   features_and_heights_twoDarray_ptr->copy(intersections_twoDarray_ptr);
   roadfunc::draw_intersections_network(intersections_twoDarray_ptr);
   string intersections_filename=cityimage.imagedir+"intersections.xyzp";   
//   ladarfunc::write_xyzp_data(
//      ztwoDarray_ptr,intersections_twoDarray_ptr,intersections_filename);
//   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
//      intersections_twoDarray_ptr,intersections_filename);
//   roadfunc::annotate_roadpoint_labels(
//      roadfunc::intersections_network_ptr,intersections_filename,
//      draw3Dfunc::annotation_value1);
*/

// Search for city blocks and assign buildings city block IDs:
   
   twoDarray* mask_twoDarray_ptr=
      cityblockfunc::generate_road_network_mask(
         cityimage.get_roadpoints_network_ptr(),ztwoDarray_ptr);
//   string mask_filename=cityimage.imagedir+"mask.xyzp";   
//   ladarfunc::write_xyzp_data(
//      ztwoDarray_ptr,mask_twoDarray_ptr,mask_filename);
   twoDarray* cityblock_regions_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
   std::vector<contour*>* cityblock_contour_ptr=
      cityblockfunc::generate_cityblock_contours(
         cityimage.imagedir,cityimage.get_data_bbox_ptr(),
         ztwoDarray_ptr,mask_twoDarray_ptr,cityblock_regions_twoDarray_ptr);
   delete mask_twoDarray_ptr;

   cityblockfunc::set_building_cityblock_IDs(
      cityimage.get_buildings_network_ptr(),
      cityblock_regions_twoDarray_ptr);

   string cityblock_regions_filename=cityimage.imagedir+
      "raw_cityblock_regions.xyzp";
   ladarfunc::write_xyzp_data(
      ztwoDarray_ptr,cityblock_regions_twoDarray_ptr,
      cityblock_regions_filename,false);
   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
      ztwoDarray_ptr,cityblock_regions_filename);
//   urbanfunc::annotate_building_labels(
//      cityimage.get_buildings_network_ptr(),cityblock_regions_filename,
//      draw3Dfunc::annotation_value1,true);
   urbanfunc::annotate_street_islands_and_peninsulas(
      cityimage.get_buildings_network_ptr(),cityblock_regions_filename,
      cityblockfunc::n_cityblocks+1);

// Generate city block building sub-networks:

   cityblockfunc::prune_multiblock_links(
      cityimage.get_buildings_network_ptr());

   vector<Network<building*>* > cityblock_neighbors_networks=
      cityblockfunc::generate_cityblock_neighbors_networks(
         cityimage.get_buildings_network_ptr());
   
   twoDarray* neighbors_twoDarray_ptr=
      new twoDarray(features_and_heights_twoDarray_ptr);
   features_and_heights_twoDarray_ptr->copy(neighbors_twoDarray_ptr);
   for (int cityblock_ID=0; cityblock_ID<cityblockfunc::n_cityblocks; 
        cityblock_ID++)
   {
      cityimage.draw_nearest_neighbor_links(
         neighbors_twoDarray_ptr,cityblock_neighbors_networks[cityblock_ID]);
      delete cityblock_neighbors_networks[cityblock_ID];
   }

   string neighbors_filename=cityimage.imagedir+"block_neighbors.xyzp";
   ladarfunc::write_xyzp_data(
      ztwoDarray_ptr,neighbors_twoDarray_ptr,neighbors_filename);
   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
      neighbors_twoDarray_ptr,neighbors_filename);
   delete neighbors_twoDarray_ptr;

   exit(-1);
   
   string cityblock_contours_filename=cityimage.imagedir+
      "cityblocks_contours.xyzp";   
   ladarfunc::write_xyzp_data(
      ztwoDarray_ptr,features_and_heights_twoDarray_ptr,
      cityblock_contours_filename,true,false);

   for (int j=0; j<cityblock_contour_ptr->size(); j++)
   {
      string contour_filename="contour_"+stringfunc::number_to_string(j)+
         ".binary";
//      geometry_func::write_contour_info_to_file(
//         (*cityblock_contour_ptr)[j],contour_filename);
      contour* curr_contour_ptr=(*cityblock_contour_ptr)[j];
      curr_contour_ptr->translate(myvector(0,0,5));
//      draw3Dfunc::draw_thick_contour(
//         curr_contour_ptr,neighbors_filename,
//         draw3Dfunc::annotation_value2);
//         curr_contour_ptr,cityblock_contours_filename,
//         draw3Dfunc::annotation_value1);
   }
   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
      ztwoDarray_ptr,cityblock_contours_filename);

   for (int j=0; j<cityblock_contour_ptr->size(); j++)
   {
      delete (*cityblock_contour_ptr)[j];
   }
   delete features_and_heights_twoDarray_ptr;
   delete binary_asphalt_twoDarray_ptr;
   delete cityblock_regions_twoDarray_ptr;
   
   cityimage.summarize_results();
   cityimage.update_logfile("abstraction");
}


