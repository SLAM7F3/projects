// ==========================================================================
// Header file for GPS_datastream class
// ==========================================================================
// Last modified on 7/17/10; 8/27/11; 8/28/11
// ==========================================================================

#ifndef GPS_DATASTREAM_H
#define GPS_DATASTREAM_H

#include <string>
#include "astro_geo/Clock.h"
#include "astro_geo/geopoint.h"
#include "messenger/serial_device.h"

class GPS_datastream
{
   
  public:

// Initialization, constructor and destructor functions:

   GPS_datastream(std::string serialPort);
   ~GPS_datastream();
   void docopy(const GPS_datastream& g);

// Set and get methods:

   int get_fix_quality() const;
   int get_n_satellites() const;
   double get_horiz_dilution() const;
   Clock& get_clock();
   const Clock& get_clock() const;
   geopoint& get_geoposn();
   const geopoint& get_geoposn() const;
   threevector get_raw_UTM_posn() const;

   double get_alpha_filtered_lat() const;
   double get_alpha_filtered_lon() const;
   double get_alpha_filtered_alt() const;
   threevector get_alpha_filtered_UTM_posn() const;

// Update member functions:

   bool read_curr_data();
   bool read_curr_data(bool& date_read_flag,bool& time_read_flag);
   void alpha_filter_lat_lon_alt();
   void alpha_filter_lat_lon_alt(double alpha);

  private: 

   int fix_quality,n_satellites;
   int year,month,day,UTC_hours,UTC_minutes;
   int lat_lon_alt_counter;
   double UTC_seconds;
   double horizontal_dilution;

   threevector curr_lat_lon_alt;
   threevector curr_alpha_filtered_lat_lon_alt,prev_alpha_filtered_lat_lon_alt;

   Clock clock;
   geopoint geo_posn,filtered_geo_posn;
   serial_device* GPS_device_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

inline int GPS_datastream::get_fix_quality() const
{
   return fix_quality;
}

inline int GPS_datastream::get_n_satellites() const
{
   return n_satellites;
}

inline double GPS_datastream::get_horiz_dilution() const
{
   return horizontal_dilution;
}

inline Clock& GPS_datastream::get_clock() 
{
   return clock;
}

inline const Clock& GPS_datastream::get_clock() const
{
   return clock;
}

inline geopoint& GPS_datastream::get_geoposn() 
{
   return geo_posn;
}

inline const geopoint& GPS_datastream::get_geoposn() const
{
   return geo_posn;
}

inline threevector GPS_datastream::get_raw_UTM_posn() const
{
   return geo_posn.get_UTM_posn();
}

inline double GPS_datastream::get_alpha_filtered_lat() const
{
   return curr_alpha_filtered_lat_lon_alt.get(0);
}

inline double GPS_datastream::get_alpha_filtered_lon() const
{
   return curr_alpha_filtered_lat_lon_alt.get(1);
}

inline double GPS_datastream::get_alpha_filtered_alt() const
{
   return curr_alpha_filtered_lat_lon_alt.get(2);
}

inline threevector GPS_datastream::get_alpha_filtered_UTM_posn() const
{
   return filtered_geo_posn.get_UTM_posn();
}


#endif  // GPS_datastream.h


