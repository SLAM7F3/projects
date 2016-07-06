// ==========================================================================
// Header file for geopoint class
// ==========================================================================
// Last modified on 8/29/08; 2/17/09; 6/20/10
// ==========================================================================

#ifndef GEOPOINT_H
#define GEOPOINT_H

#include <string>
#include <vector>
#include "math/threevector.h"

class geopoint
{
   
  public:

// Initialization, constructor and destructor functions:

   geopoint(void);
   geopoint(double currlong,double currlat);
   geopoint(bool northern_hemisphere_flag,int UTM_zone,
            double easting,double northing,double height=0);
   geopoint(bool northern_hemisphere_flag,int UTM_zone,
            const threevector& east_north_height);
   geopoint(double currlong,double currlat,double height);
   geopoint(double currlong,double currlat,double height,
            int specified_UTM_zonenumber);
   geopoint(const std::vector<std::string>& value_substrings,
            int specified_UTM_zonenumber);

   geopoint(const geopoint& g);
   ~geopoint();
   void docopy(const geopoint& g);
   geopoint operator= (const geopoint& g);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const geopoint& g);

// Set and get methods:

   void set_northern_hemisphere_flag(bool flag);
   void set_UTM_zonenumber(int number);
   void set_UTM_easting(double easting);
   void set_UTM_northing(double northing);
   void set_longitude(double lng);
   void set_latitude(double lat);
   void set_altitude(double alt);
   void set_time(double t);
   void set_dt(double dtime);

   bool get_northern_hemisphere_flag() const;
   int get_UTM_zonenumber() const;
   double get_UTM_easting() const;
   double get_UTM_northing() const;
   threevector get_UTM_posn() const;
   double get_longitude() const;
   int get_long_degs() const;
   int get_long_mins() const;
   double get_long_secs() const;
   double get_latitude() const;
   int get_lat_degs() const;
   int get_lat_mins() const;
   double get_lat_secs() const;
   double get_altitude() const;
   double get_time() const;
   double get_dt() const;

// Update member functions:

   void recompute_UTM_coords(int specified_UTM_zonenumber=-1);
   void recompute_LL_coords();

  private: 

   bool northern_hemisphere_flag;
   int UTM_zonenumber;
   double UTM_easting,UTM_northing;
   
   double longitude;	// decimal degs
   double latitude;	// decimal degs
   double altitude;	// height above sea level (meters)

   int long_degs,long_mins;
   double long_secs;
   int lat_degs,lat_mins;
   double lat_secs;

   double time;		// optional time value associated with geopoint
   double dt;		// optional time uncertainty associated with geopoint
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

inline void geopoint::set_northern_hemisphere_flag(bool flag)
{
   northern_hemisphere_flag=flag;
}

inline bool geopoint::get_northern_hemisphere_flag() const
{
   return northern_hemisphere_flag;
}

inline void geopoint::set_UTM_zonenumber(int number)
{
   UTM_zonenumber=number;
}

inline int geopoint::get_UTM_zonenumber() const
{
   return UTM_zonenumber;
}

inline void geopoint::set_UTM_easting(double easting)
{
   UTM_easting=easting;
}

inline void geopoint::set_UTM_northing(double northing)
{
   UTM_northing=northing;
}

inline void geopoint::set_altitude(double alt)
{
   altitude=alt;
}

inline void geopoint::set_time(double t)
{
   time=t;
}

inline void geopoint::set_dt(double dtime)
{
   dt=dtime;
}

inline double geopoint::get_UTM_easting() const
{
   return UTM_easting;
}

inline double geopoint::get_UTM_northing() const
{
   return UTM_northing;
}

inline threevector geopoint::get_UTM_posn() const
{
   return threevector(UTM_easting,UTM_northing,altitude);
}

inline double geopoint::get_longitude() const
{
   return longitude;
}

inline int geopoint::get_long_degs() const
{
   return long_degs;
}

inline int geopoint::get_long_mins() const
{
   return long_mins;
}

inline double geopoint::get_long_secs() const
{
   return long_secs;
}

inline double geopoint::get_latitude() const
{
   return latitude;
}

inline int geopoint::get_lat_degs() const
{
   return lat_degs;
}

inline int geopoint::get_lat_mins() const
{
   return lat_mins;
}

inline double geopoint::get_lat_secs() const
{
   return lat_secs;
}

inline double geopoint::get_altitude() const
{
   return altitude;
}

inline double geopoint::get_time() const
{
   return time;
}

inline double geopoint::get_dt() const
{
   return dt;
}

#endif  // geopoint.h


