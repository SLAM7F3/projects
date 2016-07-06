// ==========================================================================
// Header file for CITYBLOCKFUNCS namespace
// ==========================================================================
// Last modified on 6/21/05; 7/29/06
// ==========================================================================

#ifndef CITYBLOCKFUNCS_H
#define CITYBLOCKFUNCS_H

#include "threeDgraphics/draw3Dfuncs.h"
#include "network/Network.h"
#include "urban/roadsegment.h"

class building;
class building_info;
class cityblock;
class contour;
class parallelogram;
class roadpoint;
template <class T> class Strand;
template <class T> class TwoDarray;
typedef TwoDarray<double> twoDarray;

namespace cityblockfunc
{
   extern Network<cityblock*>* cityblocks_network_ptr;

// ---------------------------------------------------------------------

   int get_n_cityblocks(); 
   cityblock* get_cityblock_ptr(int c);

// City block road methods:

   void generate_cityblocks_network(
      std::string imagedir,parallelogram* data_bbox_ptr,
      Network<roadpoint*>* roadpoints_network_ptr,
      twoDarray const *ztwoDarray_ptr,
      twoDarray* cityblock_regions_twoDarray_ptr);
   twoDarray* generate_road_network_mask(
      parallelogram const *data_bbox_ptr,
      Network<roadpoint*>* roadpoints_network_ptr,
      twoDarray const *ztwoDarray_ptr);
   std::vector<contour*>* generate_cityblock_contours(
      std::string imagedir,parallelogram* data_bbox_ptr,
      twoDarray const *ztwoDarray_ptr,twoDarray const *mask_twoDarray_ptr,
      twoDarray* cityblock_regions_twoDarray_ptr);
   void find_cityblock_road_segments(
      Network<roadpoint*> const *roadpoints_network_ptr);
   void identify_cityblocks_near_roadpoints(
      twoDarray const *cityblock_regions_twoDarray_ptr,
      Network<roadpoint*>* roadpoints_network_ptr);

// City block building methods:

   void count_buildings_in_cityblocks(
      Network<building*>* buildings_network_ptr,
      twoDarray const *cityblock_regions_twoDarray_ptr);
   void set_building_cityblock_IDs(
      Network<building*>* buildings_network_ptr,
      twoDarray const *cityblock_regions_twoDarray_ptr);
   void identify_building_islands(
      Network<building*>* buildings_network_ptr,
      twoDarray const *cityblock_regions_twoDarray_ptr);
   void eliminate_blocks_with_no_buildings(
      Network<building*>* buildings_network_ptr,
      Network<roadpoint*>* roadpoints_network_ptr,
      twoDarray const *cityblock_regions_twoDarray_ptr);
   Network<building*>* generate_cityblock_building_network(
      int c,Network<building*> const *buildings_network_ptr);

   void prune_multiblock_links(Network<building*>* buildings_network_ptr);
   std::vector<int>* identify_nextdoor_neighbors(
      const Network<building*>* buildings_network_ptr);
   Network<building*>* generate_nextdoor_neighbor_network(
      int c,Network<building*> const *buildings_subnet_ptr);
   void generate_cityblock_neighbors_networks(
      const Network<building*>* buildings_network_ptr);
   void identify_building_tadpoles();
   void eliminate_building_tripoles();
   void eliminate_inconsistent_nextdoor_links();
   void copy_only_nextdoor_neighbor_links(
      Network<building*>* buildings_network_ptr);
   void examine_buildings_network(Network<building*>* buildings_network_ptr);

// Building-road relationship methods:

   void identify_buildings_on_street();
   void identify_intersections_near_building_islands(
      const Network<building*>* buildings_network_ptr);
   void identify_roadpoints_near_building_islands(
      const Network<building*>* buildings_network_ptr,
      Network<roadpoint*>* roadpoints_network_ptr);
   void identify_street_corner_buildings(
      const Network<roadpoint*>* roadpoints_network_ptr,
      Network<building*>* buildings_network_ptr);
   void compute_building_front_dirs();
   void propagate_building_front_dirs();
   void establish_building_neighbor_handedness();
   void infer_more_building_front_dirs();
   void improve_corner_building_identification();
   void break_remaining_building_tripoles();
   void associate_buildings_with_roadsegments();
   int roadsegment_for_bldg(int n);

// Strand construction & searching methods:

   void construct_building_neighbor_strands(
      int strand_length,
      Linkedlist<int> const *bldgs_with_tall_trees_in_back_list_ptr,
      Linkedlist<int> const *bldgs_with_tall_trees_on_rear_left_list_ptr);
   void score_strands_agreement_with_video_data(
      Strand<building_info*> const *video_strand_ptr,
      const int score_threshold);

// Drawing and annotation methods:

   void annotate_block_labels(
      std::string xyzp_filename,double annotation_value=
      draw3Dfunc::annotation_value1);
   twoDarray* recolor_cityblocks(
      twoDarray const *cityblock_regions_twoDarray_ptr);
   void draw_3D_strands(
      Network<building*> const *buildings_network_ptr,
      std::string xyzp_filename,double annotation_value=
      draw3Dfunc::annotation_value1);

} // cityblockfunc namespace

// ==========================================================================
// Inlined methods:
// ==========================================================================

#endif // cityblockfuncs.h



