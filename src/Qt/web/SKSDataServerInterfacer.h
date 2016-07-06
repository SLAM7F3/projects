// ========================================================================
// SKSDATASERVERINTERFACER header file
// ========================================================================
// Last updated on 3/23/08; 4/28/08; 4/29/08; 6/26/08
// ========================================================================

#ifndef SKSDATASERVERINTERFACER_H
#define SKSDATASERVERINTERFACER_H

#include <string>
#include <vector>
#include <QDomElement>
#include "Qt/web/DOMParser.h"
#include "math/threevector.h"

class SKSDataServerInterfacer
{

  public:

   SKSDataServerInterfacer();
   SKSDataServerInterfacer(std::string URL);

// Bluegrass query member functions

//   std::string form_vehicle_tracks_query();
   std::string form_vehicle_tracks_query(
      double min_longitude,double min_latitude,
      double max_longitude,double max_latitude,
      long long t_start,long long t_stop);

// SAM query member functions

   std::string form_all_SAM_attributes_query();
   std::string form_particular_SAM_attributes_query(std::string SAM_name);
   std::string form_filtered_SAM_range_query(double range);
   std::string form_filtered_SAM_altitude_query(double altitude);
   std::string form_filtered_SAM_range_altitude_IOC_query(
      double range,double altitude,int IOC);
   std::string form_filtered_SAM_IOC_query(int IOC);
   std::string form_filtered_SAM_range_and_IOC_query(double range,int IOC);
   std::string form_countries_owning_SAM_query(std::string SAM_name);
   std::string form_SAM_sites_query(std::string SAM_name);

// Response parsing member functions

//   bool load_returned_XML_into_DOM(std::string XML_filename);
   bool load_returned_XML_into_DOM(std::string XML_string);

   bool extract_named_values_from_attributes(
      std::string name_field,std::vector<std::string>& named_values);
   bool extract_named_geometries_from_coverages(
      std::string name_field,std::vector<std::string>& geometries);
   std::vector<threevector> extract_points(
      const std::vector<std::string>& geometries);

  private:

   std::string DataServer_URL;
   DOMParser parser;

   void allocate_member_objects();
   void initialize_member_objects();

   std::string initial_Bluegrass_GET_request();
   std::string initial_SAM_GET_request();

};

#endif // SKSDATASERVERINTERFACER_H

