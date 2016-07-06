// ==========================================================================
// Header file for ROADFUNCS namespace
// ==========================================================================
// Last modified on 4/17/05; 7/30/06
// ==========================================================================

#ifndef ROADFUNCS_H
#define ROADFUNCS_H

#include <iostream>
#include <string>
#include "threeDgraphics/draw3Dfuncs.h"
#include "math/threevector.h"
#include "network/Network.h"

class contour;
class netlink;
class parallelogram;
class roadpoint;
template <class T> class Site;
template <class T> class TwoDarray;
typedef TwoDarray<double> twoDarray;

namespace roadfunc
{

// Network *intersections_network_ptr holds road intersection
// points information:

   extern Network<roadpoint*>* intersections_network_ptr;

// ---------------------------------------------------------------------

   Site<roadpoint*>* get_roadpoint_site_ptr(
      Network<roadpoint*>* roadpoints_network_ptr,int r);
   roadpoint* get_roadpoint_ptr(
      Network<roadpoint*>* roadpoints_network_ptr,int r);
   netlink* get_roadlink_ptr(
      Network<roadpoint*> roadpoints_network_ptr,int r,int q);

// Road network computation methods:

   int search_for_closest_point_in_road_network(
      Network<roadpoint*>* roadpoints_network_ptr,
      const threevector& posn,bool exclude_intersections=true);
   double search_for_closest_link_in_road_network(
      Network<roadpoint*>* roadpoints_network_ptr,
      const threevector& posn,bool exclude_intersections,int& r,int& q);;
   
// Road intersection methods:

   void find_intersection_roadpoints(
      Network<roadpoint*>* roadpoints_network_ptr,
      parallelogram* data_bbox_ptr);
   void reset_intersection_roadpoints(
      Network<roadpoint*>* roadpoints_network_ptr);
   int find_next_intersection(
      Network<roadpoint*>* roadpoints_network_ptr,int r,int r_prev);
   void define_intersections_on_data_bbox(
      Network<roadpoint*>* roadpoints_network_ptr,
      parallelogram* data_bbox_ptr);
   void delete_roadpoints_outside_data_bbox(
      Network<roadpoint*>* roadpoints_network_ptr,
      parallelogram* data_bbox_ptr);

   threevector radially_scale_roadpoint_on_bbox(
      const threevector& roadpoint_posn,parallelogram* data_bbox_ptr);
   void adjust_bbox_intersection_roadpoints(
      Network<roadpoint*>* roadpoints_network_ptr,
      parallelogram* data_bbox_ptr);
   void consolidate_roadpoints_close_to_intersections(
      Network<roadpoint*>* roadpoints_network_ptr,int max_iters=3);
   bool merge_nearby_roadpoints(
      int r,int q,const double min_dist_between_pnts,
      Network<roadpoint*>* roadpoints_network_ptr);
   std::vector<int> find_intersection_neighbors_to_intersection(
      int r,Network<roadpoint*>* roadpoints_network_ptr,
      parallelogram* data_bbox_ptr);
   void generate_intersections_network(
      Network<roadpoint*>* roadpoints_network_ptr,
      parallelogram* data_bbox_ptr);
   void delete_roadpoints_network(
      Network<roadpoint*>* roadpoints_network_ptr);

// Intersections network refinement methods:

   std::vector<int> fill_intersections_vector();
   std::vector<int> fill_intersections_vector(
      Network<roadpoint*> const *roadpoints_network_ptr);
   std::vector<int> fill_neighbors_vector(int r);
   void improve_intersections_network(
      int max_iters,parallelogram* data_bbox_ptr,
      twoDarray const *binary_asphalt_twoDarray_ptr);
   std::vector<threevector> generate_intersection_spread(
      double displacement_dist,const threevector& start_pnt);
   std::vector<threevector> generate_intersection_testpnts(
      bool startpnt_on_data_bbox,parallelogram* data_bbox_ptr,
      double displacement_dist,const threevector& start_pnt,
      const threevector& e_hat);
   void perturb_intersection_position(
      int r,std::vector<int> neighbor_intersection,
      Network<roadpoint*>* roadpoints_network_ptr,
      parallelogram* data_bbox_ptr,
      twoDarray const *binary_asphalt_twoDarray_ptr);
   double score_link(
      int r,int q,Network<roadpoint*> const *roadpoints_network_ptr,
      twoDarray const *binary_asphalt_twoDarray_ptr);
   double score_link(
      const threevector& r_posn,const threevector& q_posn,
      twoDarray const *binary_asphalt_twoDarray_ptr);
   void delete_low_score_segments(
      double min_frac_score,twoDarray const *binary_asphalt_twoDarray_ptr);
   void delete_low_score_link(
      int r,double min_frac_score,std::vector<int> neighbor_intersection,
      Network<roadpoint*>* roadpoints_network_ptr,
      twoDarray const *binary_asphalt_twoDarray_ptr);
   void insert_fake_intersections_nearby_bldg_islands();
   void test_intersections_near_bldg_islands(
      int r,std::vector<int>& neighbor_intersection);
   void insert_sites_at_netlink_intersections(
      Network<roadpoint*>* roadpoints_network_ptr);
   void test_linear_links_between_intersections(
      Network<roadpoint*>* roadpoints_network_ptr,
      parallelogram* data_bbox_ptr,
      twoDarray const *binary_asphalt_twoDarray_ptr);
   void move_roadpoints_near_data_bbox_onto_bbox(
      Network<roadpoint*>* roadpoints_network_ptr,
      parallelogram* data_bbox_ptr);

// Road network display methods:

   void draw_road_network(
      Network<roadpoint*>* roadpoints_network_ptr,
      std::string xyzp_filename,double annotation_value=
      draw3Dfunc::annotation_value1,double delta_z=0);
   void draw_road_network(
      Network<roadpoint*>* roadpoints_network_ptr,
      twoDarray* ftwoDarray_ptr,bool display_roadpoints_flag=true,
      bool display_bbox_intersections=false);
   void draw_roadpoints(
      Network<roadpoint*>* roadpoints_network_ptr,twoDarray* ftwoDarray_ptr,
      bool display_bbox_intersections=true);
   void draw_roadpoint_links(
      Network<roadpoint*>* roadpoints_network_ptr,int r,
      twoDarray* ftwoDarray_ptr);
   void draw_road_link(
      Network<roadpoint*>* roadpoints_network_ptr,int r,int q,
      twoDarray* ftwoDarray_ptr);
   void annotate_roadpoint_labels(
      Network<roadpoint*>* roadpoints_network_ptr,
      std::string xyzp_filename,double annotation_value=
      draw3Dfunc::annotation_value1);
   void draw_intersections_network(twoDarray* ftwoDarray_ptr);

// Road network text input/output methods:

   void output_road_network_to_textfile(
      Network<roadpoint*>* roadpoints_network_ptr,
      parallelogram* data_bbox_ptr,std::string output_filename);
   void output_roadpoint_and_its_links(
      Network<roadpoint*>* roadpoints_network_ptr,
      parallelogram* data_bbox_ptr,int r,std::ofstream& outstream);
   Network<roadpoint*>* readin_road_network_from_textfile(
      std::string input_filename);

} // roadfunc namespace

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline Site<roadpoint*>* get_roadpoint_site_ptr(
   Network<roadpoint*>* roadpoints_network_ptr,int r)
{
   return roadpoints_network_ptr->get_site_ptr(r);
}

inline roadpoint* get_roadpoint_ptr(
   Network<roadpoint*>* roadpoints_network_ptr,int r)
{
   return roadpoints_network_ptr->get_site_data_ptr(r);
}

inline netlink* get_roadlink_ptr(
   Network<roadpoint*>* roadpoints_network_ptr,int r,int q)
{
   return roadpoints_network_ptr->get_netlink_ptr(r,q);
}

#endif // roadfuncs.h



