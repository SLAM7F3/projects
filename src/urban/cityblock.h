// ==========================================================================
// Header file for CITYBLOCK class
// ==========================================================================
// Last modified on 6/21/05; 4/23/06; 6/14/06; 7/29/06
// ==========================================================================

#ifndef CITYBLOCK_H
#define CITYBLOCK_H

#include <set>
#include <string>
#include <vector>
#include "urban/building_info.h"
#include "geometry/contour_element.h"
#include "threeDgraphics/draw3Dfuncs.h"
#include "network/Network.h"
#include "network/Strand.h"

class building;
class contour;
class threevector;
class roadpoint;
class roadsegment;

class cityblock: public contour_element
{

  public:

// Initialization, constructor and destructor functions:

   cityblock();
   cityblock(int identification);
   cityblock(int identification,const threevector& posn);
   cityblock(const cityblock& b);
   virtual ~cityblock();
   cityblock& operator= (const cityblock& b);

   friend std::ostream& operator<< 
      (std::ostream& outstream,const cityblock& b);

// Set & get member functions:

   void set_n_buildings(int n);
   void set_building_tadpoles_list_ptr(Linkedlist<int>* list_ptr);
   void set_block_buildings_network_ptr(Network<building*>* network_ptr);
   void set_block_roadsegment_ptrs(std::vector<roadsegment*>* V_ptr);

   int get_n_buildings() const;
   Linkedlist<int>* get_building_tadpoles_list_ptr();
   Network<building*>* get_block_buildings_network_ptr();
   building* get_building_ptr(int n);
   std::vector<roadsegment*>* get_block_roadsegment_ptrs();
   Linkedlist<Strand<building_info*>*>* get_strand_list_ptr();

// Road spatial relationships methods:

   void find_road_segments(Network<roadpoint*> const *roadpoints_network_ptr);
   std::vector<int> identify_nearby_roadpoints(
      double max_distance,Network<roadpoint*> const *roadpoints_network_ptr);

// Building spatial relationships methods:
   
   void identify_building_tadpoles();
   void identify_buildings_on_street();
   void estimate_building_front_dirs();
   void estimate_building_front_dir(int n,double max_bldg_to_road_dist);
   void estimate_corner_building_front_dirs(
      int n,double max_bldg_to_road_dist);
   void propagate_building_front_dirs();
   bool improve_guessed_fhat(
      double max_distance,const threevector& b_posn,threevector& f_hat);   
   
   void eliminate_building_tripoles();
   void eliminate_inconsistent_nextdoor_links();
   void initialize_building_neighbor_handedness();
   void propagate_building_neighbor_handedness();
   void infer_more_building_front_dirs();
   void improve_corner_building_identification();
   void break_remaining_building_tripoles();
   void associate_buildings_with_roadsegments();

// Drawing & annotation methods:

   void annotate_block_label(
      std::string xyzp_filename,double annotation_value=
      draw3Dfunc::annotation_value1);
   void annotate_roadsegment_labels(
      std::string xyzp_filename,double annotation_value=
      draw3Dfunc::annotation_value1);

// Strand construction & searching methods:

   int construct_building_neighbor_strands(int length);
   void assign_strand_member_relative_heights();
   void assign_strand_member_spine_dirs();
   void assign_strand_member_gross_vegetation_dirs(
      Linkedlist<int> const *bldgs_with_tall_trees_in_back_list_ptr,
      Linkedlist<int> const *bldgs_with_small_shrubs_on_rear_left_list_ptr);

  private:

   int n_buildings;
   Linkedlist<int>* building_tadpoles_list_ptr;
   Network<building*>* block_buildings_network_ptr;
   std::vector<roadsegment*>* block_roadsegment_ptrs;
   Linkedlist<Strand<building_info*>*>* strand_list_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const cityblock& b);

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void cityblock::set_n_buildings(int n)
{
   n_buildings=n;
}

inline void cityblock::set_building_tadpoles_list_ptr(
   Linkedlist<int>* list_ptr)
{
   building_tadpoles_list_ptr=list_ptr;
}

inline void cityblock::set_block_buildings_network_ptr(
   Network<building*>* network_ptr)
{
   block_buildings_network_ptr=network_ptr;
}

inline void cityblock::set_block_roadsegment_ptrs(
   std::vector<roadsegment*>* V_ptr)
{
   block_roadsegment_ptrs=V_ptr;
}

inline int cityblock::get_n_buildings() const
{
   return n_buildings;
}

inline Linkedlist<int>* cityblock::get_building_tadpoles_list_ptr()
{
   return building_tadpoles_list_ptr;
}

inline Network<building*>* cityblock::get_block_buildings_network_ptr()
{
   return block_buildings_network_ptr;
}

inline building* cityblock::get_building_ptr(int n)
{
   return block_buildings_network_ptr->get_site_data_ptr(n);
}

inline std::vector<roadsegment*>* cityblock::get_block_roadsegment_ptrs() 
{
   return block_roadsegment_ptrs;
}

inline Linkedlist<Strand<building_info*>*>* cityblock::get_strand_list_ptr() 
{
   return strand_list_ptr;
}

#endif // cityblock.h



