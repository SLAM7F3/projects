 // ==========================================================================
// Header file for tracks_group class.  
// ==========================================================================
// Last updated on 1/22/09; 1/23/09; 3/20/13
// ==========================================================================

#ifndef TRACKSGROUP_H
#define TRACKSGROUP_H

#include <iostream>
#include <map>
#include <vector>
#include "track/track.h"

class tracks_group
{

  public:

   typedef std::map<int,track*> TRACK_PTRS_MAP;

   typedef std::map<int,int> TRACK_LABELS_MAP;
// Independent int = track_label_ID
// Dependent int = track_ID   

   tracks_group();
   ~tracks_group();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const tracks_group& tg);

// Set & get member functions:

   int get_n_tracks() const;
   std::vector<track*> get_all_track_ptrs() const;
   TRACK_LABELS_MAP* get_track_labels_map_ptr();
   const TRACK_LABELS_MAP* get_track_labels_map_ptr() const;
   track* get_track_ptr(int ID);
   const track* get_track_ptr(int ID) const;

// Track generation and propagation member functions:

   track* generate_new_track(int ID=-1);
   int get_track_ID(int track_label_ID);
   track* get_track_given_label(int track_label_ID);
   void associate_track_labelID_and_ID(track* curr_track_ptr);

   bool destroy_track(int ID);
   bool destroy_track(track* track_to_destroy_ptr);
   void destroy_all_tracks();

// Track manipulation member functions:

   void rescale_time_values_for_all_tracks(double scale_factor);

// Spacetime proximity member functions:

   std::vector<double> closest_approach_times(const threevector& POI);
   bool ground_target_already_exists(
      double curr_t,double input_easting,double input_northing,
      double min_separation_distance);

// Velocity member functions

   void compute_speed_dependent_vehicle_track_colors();
   void read_speeds_from_file(std::string filename);

  private:

   int curr_track_ID;
   int prev_framenumber;
   double start_time,curr_time;
   TRACK_PTRS_MAP* track_ptrs_map_ptr;
   TRACK_LABELS_MAP* track_labels_map_ptr;

   void allocate_member_objects();
   void initialize_member_objects();

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline int tracks_group::get_n_tracks() const
{
   return track_ptrs_map_ptr->size();
}

inline track* tracks_group::get_track_ptr(int ID)
{
   TRACK_PTRS_MAP::iterator track_ptr_iter=track_ptrs_map_ptr->find(ID);
   if (track_ptr_iter != track_ptrs_map_ptr->end())
   {
      return track_ptr_iter->second;
   }
   else
   {
      return NULL;
   }
}

inline const track* tracks_group::get_track_ptr(int ID) const
{
   TRACK_PTRS_MAP::iterator track_ptr_iter=track_ptrs_map_ptr->find(ID);
   if (track_ptr_iter != track_ptrs_map_ptr->end())
   {
      return track_ptr_iter->second;
   }
   else
   {
      return NULL;
   }
}

inline tracks_group::TRACK_LABELS_MAP* 
tracks_group::get_track_labels_map_ptr()
{
   return track_labels_map_ptr;
}

inline const tracks_group::TRACK_LABELS_MAP* 
tracks_group::get_track_labels_map_ptr() const
{
   return track_labels_map_ptr;
}

#endif // tracks_group.h

