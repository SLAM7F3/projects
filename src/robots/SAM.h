// ==========================================================================
// Header file for SAM class
// ==========================================================================
// Last modified on 3/7/08; 3/12/08; 3/25/08
// ==========================================================================

#ifndef SAM_H
#define SAM_H

#include <string>
#include "astro_geo/geopoint.h"
#include "geometry/polygon.h"

class SAM
{

  public:

   enum SAMType 
   {
      SA2,SA3,SA4,SA5,SA6,SA7,SA8,SA9,SA10,SA11,SA12,SA13,SA14,SA15,
      SA16,SA18,FT2000,FM90,HQ2,Pegasus,other
   };

// Initialization, constructor and destructor functions:

   SAM(int ID);
   SAM(const SAM& s);

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~SAM();
   SAM& operator= (const SAM& s);
   friend std::ostream& operator<< (std::ostream& outstream,
                                    const SAM& s);

// Set and get member functions:

   int get_ID() const;
   void set_entity_ID(std::string eID);
   std::string get_entity_ID() const;

   std::string get_name() const;
   void set_SAM_type(SAMType type);
   const SAMType& get_SAM_type() const;
   int get_IOC() const;
   double get_max_range() const;
   double get_max_target_altitude() const;
   double get_warhead_weight() const;
   double get_max_speed() const;
   void set_site_location(const geopoint& posn);
   geopoint& get_site_location();
   const geopoint& get_site_location() const;
   void set_country_owner(std::string country);
   std::string get_country_owner() const;

   std::vector<threevector> get_threat_region_long_lat_alt_vertices();
   const std::vector<threevector> get_threat_region_long_lat_alt_vertices()
      const;

// Threat polygon member functions:

   polygon& generate_threat_region();
   std::string generate_dynamic_URL();

  private: 

   int ID;
   std::string entity_ID;

   int IOC;	// initial operational capability
   double max_range,max_target_altitude,warhead_weight,max_speed;
   std::string country_owner;
   SAMType sam_type;
   geopoint site_location;
   polygon threat_region;
   std::vector<threevector> threat_region_long_lat_alt_vertices;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const SAM& s);

   void initialize_params_based_on_type();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline int SAM::get_ID() const
{
   return ID;
}

inline void SAM::set_entity_ID(std::string eID)
{
   entity_ID=eID;
}

inline std::string SAM::get_entity_ID() const
{
   return entity_ID;
}

inline void SAM::set_SAM_type(SAMType type)
{
   sam_type=type;
   initialize_params_based_on_type();
}

inline const SAM::SAMType& SAM::get_SAM_type() const
{
   return sam_type;
}

inline int SAM::get_IOC() const
{
   return IOC;
}

inline double SAM::get_max_range() const
{
   return max_range;
}

inline double SAM::get_max_target_altitude() const
{
   return max_target_altitude;
}

inline double SAM::get_warhead_weight() const
{
   return warhead_weight;
}

inline double SAM::get_max_speed() const
{
   return max_speed;
}

inline void SAM::set_site_location(const geopoint& posn)
{
   site_location=posn;
}

inline geopoint& SAM::get_site_location()
{
   return site_location;
}

inline const geopoint& SAM::get_site_location() const
{
   return site_location;
}

inline void SAM::set_country_owner(std::string country)
{
   country_owner=country;
}

inline std::string SAM::get_country_owner() const
{
   return country_owner;
}

inline std::vector<threevector> SAM::get_threat_region_long_lat_alt_vertices()
{
   return threat_region_long_lat_alt_vertices;
}

inline const std::vector<threevector> 
SAM::get_threat_region_long_lat_alt_vertices() const
{
   return threat_region_long_lat_alt_vertices;
}

#endif  // SAM.h



