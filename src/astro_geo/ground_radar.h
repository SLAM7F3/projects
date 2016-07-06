// ==========================================================================
// Header file for ground_radar class
// ==========================================================================
// Last modified on 4/28/06; 6/8/06; 12/21/06; 10/8/11
// ==========================================================================

#ifndef GROUND_RADAR_H
#define GROUND_RADAR_H

#include <string>
#include "astro_geo/geopoint.h"
#include "math/statevector.h"
#include "math/threevector.h"

class ground_radar
{

  public:

// Initialization, constructor and destructor functions:

   void allocate_member_objects();
   void initialize_member_objects();
   ground_radar(void);
   ground_radar(std::string ground_radar_name,double ground_radar_bandwidth,
          const geopoint& ground_radar_posn);
   ground_radar(const ground_radar& s);
   ~ground_radar();
   void docopy(const ground_radar& s);
   ground_radar operator= (const ground_radar& s);
   friend std::ostream& operator<< (std::ostream& outstream,ground_radar& s);

// Set & get member functions:

   void set_name(const std::string& n);
   void set_bandwidth(double b);
   void set_max_slew_rate();
   void set_geolocation(const geopoint& geoposn);
   void set_statevector(const statevector& PV);

   std::string get_name() const; 
   double get_bandwidth() const;
   double get_max_slew_rate() const;
   const geopoint& get_geolocation() const;
   statevector& get_statevector();
   const statevector& get_statevector() const;

   threevector compute_position(
      double t,const threevector& init_ground_radar_posn);
   statevector& compute_statevector(
      double t,const threevector& init_ground_radar_posn);

  private: 

   std::string name;
   double bandwidth;	// Hz
   double max_slew_rate;	// degs/sec
   geopoint geolocation;
   statevector posn_vel;
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

inline void ground_radar::set_name(const std::string& n)
{
   name=n;
}

inline void ground_radar::set_bandwidth(double b)
{
   bandwidth=b;
}

inline void ground_radar::set_geolocation(const geopoint& geoposn)
{
   geolocation=geoposn;
}

inline void ground_radar::set_statevector(const statevector& PV)
{
   posn_vel=PV;
}


inline std::string ground_radar::get_name() const
{
   return name;
}

inline double ground_radar::get_bandwidth() const
{
   return bandwidth;
}

inline double ground_radar::get_max_slew_rate() const
{
   return max_slew_rate;
}

inline const geopoint& ground_radar::get_geolocation() const
{
   return geolocation;
}

inline statevector& ground_radar::get_statevector() 
{
   return posn_vel;
}

inline const statevector& ground_radar::get_statevector() const
{
   return posn_vel;
}


#endif  // ground_radar.h



