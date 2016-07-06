// ==========================================================================
// Header file for movers_group class.  
// ==========================================================================
// Last updated on 1/22/09; 1/24/09; 2/18/09; 4/5/14
// ==========================================================================

#ifndef MOVERS_GROUP_H
#define MOVERS_GROUP_H

#include <map>
#include <vector>
#include "math/lttwovector.h"
#include "messenger/Messenger.h"
#include "track/mover.h"
#include "network/Network.h"

class polyline;
class tracks_group;

class movers_group
{

  public:

   typedef std::map<twovector,int,lttwovector > MOVERS_MAP;

   movers_group();
   ~movers_group();
   friend std::ostream& operator<< (
      std::ostream& outstream,const movers_group& MG);

// Set & get member functions:

   unsigned int size() const;
   unsigned int get_n_UAVs() const;
   int get_next_unused_ID() const;
   int get_next_unused_network_index() const;
   int get_n_particular_movers(mover::MoverType m_type) const;
   mover* get_mover_ptr(int r);
   const mover* get_mover_ptr(int r) const;

   Network<mover*>* get_movers_network_ptr() const;
   void set_Messenger_ptr(Messenger* M_ptr);
   Messenger* get_Messenger_ptr();
   const Messenger* get_Messenger_ptr() const;
   int get_latest_ROI_ID() const;
   int get_latest_KOZ_ID() const;

   std::vector<mover*>& get_movers_queue();
   const std::vector<mover*>& get_movers_queue() const;
   std::vector<int>& get_encountered_ROI_IDs();
   const std::vector<int>& get_encountered_ROI_IDs() const;
   std::vector<int>& get_encountered_vehicle_IDs();
   const std::vector<int>& get_encountered_vehicle_IDs() const;

// Mover generation and propagation member functions:

   mover* generate_new_mover(mover::MoverType type,int ID=-1);
   mover* generate_new_vehicle(track* vehicle_track_ptr,int ID=-1);
   mover* generate_new_ROI(
      tracks_group* spatially_fixed_tracks_group_ptr,int ID=-1);
   mover* generate_new_KOZ(tracks_group* KOZ_tracks_group_ptr,int ID=-1);
   void delete_mover(mover::MoverType type,int ID);
   std::vector<int> get_particular_mover_IDs(mover::MoverType m_type) const;
   void purge_all_particular_movers(mover::MoverType m_type);
   int get_next_unused_ROI_ID();
   int get_next_unused_KOZ_ID();

// Mover network member functions:

   int get_mover_network_index(
      mover::MoverType mover_type,int mover_ID) const;
   mover* get_mover_ptr(mover::MoverType mover_type,int mover_ID);
   void add_mover_network_link(
      mover::MoverType first_mover_type,int first_mover_ID,
      mover::MoverType second_mover_type,int second_mover_ID);

// ROI-vehicle association member functions:

   void add_ROI_vehicle_network_link(int ROI_ID,int vehicle_ID);
   void add_vehicle_vehicle_network_link(
      int first_vehicle_ID,int second_vehicle_ID);
   void add_ROI_ROI_network_link(int first_ROI_ID,int second_ROI_ID);
   void associate_vehicles_with_ROI(
      int ROI_ID,tracks_group* tracks_group_ptr);
   void associate_vehicles_with_ROI(
      int ROI_ID,const std::vector<int>& vehicle_label_IDs);

// Ground mover-UAV association member functions:

   void check_for_UAV_ground_target_encounters(
      double curr_t,movers_group* UAV_movers_group_ptr,
      mover::MoverType mover_type);
   void compute_passed_ground_targets(
      double curr_t,int UAV_ID,const threevector& curr_UAV_posn,
      movers_group* UAV_movers_group_ptr,
      mover::MoverType mover_type);

// Ground target path generation member functions:

   std::vector<threevector> generate_ground_target_posns(
      double curr_t,const std::vector<int>& ground_target_IDs,
      mover::MoverType mover_type);

// Message handling member functions:

   void add_mover_to_outgoing_queue(mover* mover_ptr);
   void issue_add_vertex_message(mover* mover_ptr);

   void issue_delete_vertex_message(mover* mover_ptr);
   void issue_delete_edge_message(mover* mover1_ptr,mover* mover2_ptr);
   void issue_delete_all_message();

   void issue_add_track_message(
      bool nothern_hemisphere_flag,int UTM_zone,track* track_ptr,
      bool compute_posns_with_distinct_dirs_flag=true);
   void issue_delete_track_message(int track_ID);

  private:

   unsigned int n_vehicles,n_ROIs,n_UAVs,n_KOZs;
   int latest_ROI_ID,latest_KOZ_ID;
   Network<mover* >* movers_network_ptr;
   MOVERS_MAP* movers_map_ptr;
   Messenger* Messenger_ptr;
   std::vector<mover*> movers_queue;
   std::vector<int> encountered_ROI_IDs,encountered_vehicle_IDs;

   void allocate_member_objects();
   void initialize_member_objects();

   void insert_mover_into_network(mover* curr_mover_ptr);
   void print_n_ROIs_vehicles_UAVs_and_KOZs();
   std::string get_MoverType_string(mover* mover_ptr);

   void issue_add_edge_message(mover* mover1_ptr,mover* mover2_ptr);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline unsigned int movers_group::size() const
{
   return movers_network_ptr->size();
}

inline unsigned int movers_group::get_n_UAVs() const
{
   return n_UAVs;
}

inline mover* movers_group::get_mover_ptr(int r)
{
   if (r >= 0)
   {
      return movers_network_ptr->get_site_data_ptr(r);
   }
   else
   {
      return NULL;
   }
}

inline const mover* movers_group::get_mover_ptr(int r) const
{
   if (r >= 0)
   {
      return movers_network_ptr->get_site_data_ptr(r);
   }
   else
   {
      return NULL;
   }
}

inline Network<mover*>* movers_group::get_movers_network_ptr() const
{
   return movers_network_ptr;
}

inline void movers_group::set_Messenger_ptr(Messenger* M_ptr)
{
   Messenger_ptr=M_ptr;
}

inline Messenger* movers_group::get_Messenger_ptr()
{
   return Messenger_ptr;
}

inline const Messenger* movers_group::get_Messenger_ptr() const
{
   return Messenger_ptr;
}

inline int movers_group::get_latest_ROI_ID() const
{
   return latest_ROI_ID;
}

inline int movers_group::get_latest_KOZ_ID() const
{
   return latest_KOZ_ID;
}

inline std::vector<mover*>& movers_group::get_movers_queue() 
{
   return movers_queue;
}

inline const std::vector<mover*>& movers_group::get_movers_queue() const
{
   return movers_queue;
}

inline std::vector<int>& movers_group::get_encountered_ROI_IDs() 
{
   return encountered_ROI_IDs;
}

inline const std::vector<int>& movers_group::get_encountered_ROI_IDs() const
{
   return encountered_ROI_IDs;
}

inline std::vector<int>& movers_group::get_encountered_vehicle_IDs() 
{
   return encountered_vehicle_IDs;
}

inline const std::vector<int>& movers_group::get_encountered_vehicle_IDs() const
{
   return encountered_vehicle_IDs;
}

#endif // movers_group.h

