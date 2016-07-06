// ==========================================================================
// Header file for SAMs_group class.  
// ==========================================================================
// Last updated on 3/12/08; 3/21/08; 3/24/08; 3/25/08
// ==========================================================================

#ifndef SAMSGROUP_H
#define SAMSGROUP_H

#include <vector>
#include "color/colorfuncs.h"
#include "astro_geo/geopoint.h"
#include "robots/SAM.h"

class Messenger;
class polyline;

class SAMs_group
{

  public:

   SAMs_group();
   SAMs_group(Messenger* m_ptr);
   ~SAMs_group();

// Set & get member functions:

   void set_messenger_ptr(Messenger* m_ptr);
   int get_n_SAMs() const;
   SAM* get_SAM_ptr(int ID);
   const SAM* get_SAM_ptr(int ID) const;
   std::vector<SAM*> get_SAM_ptrs();
   const std::vector<SAM*> get_SAM_ptrs() const;
   SAM* get_closest_SAM_ptr();
   const SAM* get_closest_SAM_ptr() const;

// SAM generation and propagation member functions:

   SAM* generate_new_SAM(int ID=-1);
   SAM* generate_new_SAM(
      SAM::SAMType type, std::string country,
      double site_longitude,double site_latitude,
      double site_altitude,int ID=-1);
   void generate_all_SAMs(std::string SAM_list_filename);
   void collate_individual_region_lla_vertices( 
      int s,std::vector<SAM*>& input_SAM_ptrs,
      std::vector<std::vector<threevector> >& V);
   void collate_threat_regions_lla_vertices( 
      std::vector<SAM*>& input_SAM_ptrs,
      std::vector<std::vector<threevector> >& V);

// SAM querying member functions:

   SAM::SAMType get_type(std::string SAM_name) const;
   std::vector<SAM*> get_matching_SAMs(std::string SAM_name);
   std::vector<SAM*> get_matching_SAMs(
      const std::vector<std::string>& SAM_names);
   SAM* get_representative_SAM(std::string SAM_name);

// Country & site query member functions:

   std::vector<std::string> countries_owning_particular_SAMs(
      std::string SAM_name);
   std::vector<SAM*> SAMs_in_particular_country(std::string country_name);
   std::vector<SAM*> representative_SAMs_in_particular_country(
      std::string country_name);

   void queried_SAM_sites(const std::vector<SAM*>& particular_SAMs_ptrs);
   std::vector<int> retrieve_SAM_IDs(
      const std::vector<SAM*>& particular_SAMs_ptrs);
   
// KML file output member functions:

   void generate_empty_KML_files();
   void generate_owner_country_KML_file(std::string country_name);
   void generate_owner_countries_KML_file(
      std::vector<std::string> country_owner_names);
   void generate_SAM_sites_KML_file(
      std::vector<SAM*>& matching_SAMs_ptrs,colorfunc::Color curr_color,
      std::string output_KML_filename="");
   void generate_SAM_sites_KML_file(
      std::string query_SAM_name,colorfunc::Color curr_color,
      std::string output_KML_filename="");
   void generate_single_SAM_site_KML_file(
      int SAM_ID,colorfunc::Color curr_color);

// Flight path evaluation member functions:

   std::vector<geopoint>& construct_flight_waypoints(
      const polyline& flight_path);
   double compute_flight_path_distance(
      const polyline& flight_path);
   double find_closest_SAM_site(
      polyline& flight_path,double flight_path_distance_in_meters);
   void generate_flightpath_KML_file(const polyline& flight_path);
   void analyze_flight_path(
      polyline& flight_path,double& flight_path_distance_in_kms,
      double& min_distance_to_SAM_in_kms,int& closest_SAM_ID);

  private:

   int curr_SAM_ID;
   std::vector<SAM*> SAM_ptrs;
   std::vector<geopoint> flight_waypoints;
   std::vector<std::string> SAM_output_KML_filename;
   SAM* closest_SAM_ptr;
   Messenger* messenger_ptr;

   void allocate_member_objects();
   void initialize_member_objects();

   void issue_update_message();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void SAMs_group::set_messenger_ptr(Messenger* m_ptr)
{
   messenger_ptr=m_ptr;
}

inline int SAMs_group::get_n_SAMs() const
{
   return SAM_ptrs.size();
}

inline SAM* SAMs_group::get_SAM_ptr(int ID)
{
   if (ID >= 0 && ID < get_n_SAMs())
   {
      return SAM_ptrs[ID];
   }
   else
   {
      return NULL;
   }
}

inline const SAM* SAMs_group::get_SAM_ptr(int ID) const
{
   if (ID >= 0 && ID < get_n_SAMs())
   {
      return SAM_ptrs[ID];
   }
   else
   {
      return NULL;
   }
}

inline std::vector<SAM*> SAMs_group::get_SAM_ptrs()
{
   return SAM_ptrs;
}

inline const std::vector<SAM*> SAMs_group::get_SAM_ptrs() const
{
   return SAM_ptrs;
}

inline SAM* SAMs_group::get_closest_SAM_ptr() 
{
   return closest_SAM_ptr;
}

inline const SAM* SAMs_group::get_closest_SAM_ptr() const
{
   return closest_SAM_ptr;
}

#endif // SAMs_group.h

