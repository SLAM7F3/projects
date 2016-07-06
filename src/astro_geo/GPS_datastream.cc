// ==========================================================================
// GPS_datastream class member function definitions
// ==========================================================================
// Last modified on 8/22/10; 8/27/11; 8/28/11
// ==========================================================================

#include <iostream>
#include <vector>
#include "filter/filterfuncs.h"
#include "astro_geo/GPS_datastream.h"
#include "astro_geo/latlong2utmfuncs.h"
#include "general/stringfuncs.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void GPS_datastream::allocate_member_objects()
{
}		       

void GPS_datastream::initialize_member_objects()
{
   year=month=day=0;
   clock.current_local_time_and_UTC();
   lat_lon_alt_counter=0;
}

GPS_datastream::GPS_datastream(string serialPort)
{
   int baud_rate=4800;
   GPS_device_ptr=new serial_device(serialPort,baud_rate);
   allocate_member_objects();
   initialize_member_objects();
}

GPS_datastream::~GPS_datastream()
{
   delete GPS_device_ptr;
}

// ==========================================================================
// Update member functions
// ==========================================================================

// Member function read_curr_data() parses GPS "sentences" read in
// from a GPS device connected to the linux machine's serial port.  It
// specifically extracts the current UTC time, geoposition and number
// of visible satellites.  If no data is successfully read in from the
// GPS device, this boolean method returns false.

bool GPS_datastream::read_curr_data()
{
   bool date_read_flag,time_read_flag;
   return read_curr_data(date_read_flag,time_read_flag);
}

bool GPS_datastream::read_curr_data(bool& date_read_flag,bool& time_read_flag)
{
//   cout << "inside GPS_datastream::read_curr_data()" << endl;
   string curr_data=GPS_device_ptr->readData();
//   cout << "curr_data = " << curr_data << endl;
//   cout << "curr_data.size() = " << curr_data.size() << endl;

   date_read_flag=time_read_flag=false;

   if (curr_data.size() < 1) return false;

   string separator_chars=",";
   vector<string> substrings=stringfunc::decompose_string_into_substrings(
      curr_data,separator_chars);
   if (substrings.size() < 11) return false;

   cout.precision(12);
   if (substrings[0]=="$GPRMC")
   {
      int date_stamp=stringfunc::string_to_number(substrings[9]);
      cout.precision(15);
//      cout << "date_stamp = " << date_stamp << endl;

      day=basic_math::mytruncate(0.0001*date_stamp);
      date_stamp -= 10000*day;
      month=basic_math::mytruncate(0.01*date_stamp);
      date_stamp -= 100*month;
      year=2000+date_stamp;
//      cout << "year = " << year << " month = " << month << " day = " << day
//           << endl;
      clock.set_year_month_day(year,month,day);
      date_read_flag=true;
      return true;
   }
   else if (substrings[0]=="$GPGGA")
   {
      double UTC_value=stringfunc::string_to_number(substrings[1]);
      cout.precision(15);
//      cout << "initial UTC_value = " << UTC_value << endl;

      UTC_hours=basic_math::mytruncate(0.0001*UTC_value);
      UTC_value -= 10000*UTC_hours;
      UTC_minutes=basic_math::mytruncate(0.01*UTC_value);
      UTC_value -= 100*UTC_minutes;
      UTC_seconds=UTC_value;
      clock.set_UTC_time(year,month,day,UTC_hours,UTC_minutes,UTC_seconds);

//      cout << "UTC_value = " << UTC_value << endl;
//      cout << "UTC_hours = " << UTC_hours
//           << " UTC_mins = " << UTC_minutes
//           << " UTC_secs = " << UTC_seconds << endl;

      bool display_UTC_flag=false;
      if (year==0 && month==0 && day==0)
      {
      }
      else
      {
         cout << "Current time = " 
              << clock.YYYY_MM_DD_H_M_S(" ",":",display_UTC_flag) << endl;
      }

      fix_quality=stringfunc::string_to_number(substrings[6]);
      n_satellites=stringfunc::string_to_number(substrings[7]);
      horizontal_dilution=stringfunc::string_to_number(substrings[8]);

      cout << "Fix_quality = " << fix_quality 
           << " n_satellites = " << n_satellites 
           << " horiz_dilution = "<< horizontal_dilution << endl;

// Fix quality: 0 = invalid, 1 = GPS fix, 2 = dgps fix

// Horizontal dilution of precision:

// 1:  ideal
// 1-2: excellent
// 2-5: good
// 5-10: moderate
// 10-20: fair
// >20: poor

      double lat_value=stringfunc::string_to_number(substrings[2]);
      string NS_flag=substrings[3];
      double lon_value=stringfunc::string_to_number(substrings[4]);
      string EW_flag=substrings[5];

      int lat_degs=basic_math::mytruncate(0.01*lat_value);
      lat_value -= 100*lat_degs;
      double lat_minutes=lat_value;
      double latitude=latlongfunc::dm_to_decimal_degs(
         lat_degs,lat_minutes);
      if (NS_flag != "N") latitude *= -1;

      int lon_degs=basic_math::mytruncate(0.01*lon_value);
      lon_value -= 100*lon_degs;
      double lon_minutes=lon_value;
      double longitude=latlongfunc::dm_to_decimal_degs(
         lon_degs,lon_minutes);
      if (EW_flag=="W") longitude *= -1;

      double altitude=stringfunc::string_to_number(substrings[9]);
      string alt_unit=substrings[10];
      geo_posn=geopoint(longitude,latitude,altitude);

      curr_lat_lon_alt=threevector(latitude,longitude,altitude);

//      cout << "lon value = " << lon_value << " " << EW_flag << endl;
//      cout << "lat value = " << lat_value << " " << NS_flag << endl;

//      cout << "lon_degs = " << lon_degs
//           << " lon_mins = " << lon_minutes << endl;
//      cout << "lat_degs = " << lat_degs
//           << " lat_mins = " << lat_minutes << endl;
//      cout << "longitude = " << longitude
//           << " latitude = " << latitude << endl;
//      cout << "altitude = " << altitude << " " << alt_unit << endl;

//      cout << "Current geo position = " << geo_posn << endl;
//      cout << endl;

      int UTM_zonenumber=get_geoposn().get_UTM_zonenumber();
//      cout << "UTM_zonenumber = " << UTM_zonenumber << endl;
//      int utm_zone_time_offset=
         get_clock().compute_UTM_zone_time_offset(UTM_zonenumber);
//      cout << "utm_zone_time_offset = " << utm_zone_time_offset << endl;

      time_read_flag=true;
      return true;
   }

   return false;
}

// ---------------------------------------------------------------------
// Member function alpha_filter_lat_lon_alt() performs alpha filtering
// of raw latitude, longitude and altitude GPS measurements.  Filtered
// output is stored within member threevector
// curr_alpha_filtered_lat_lon_alt.

void GPS_datastream::alpha_filter_lat_lon_alt()
{
//   cout << "inside GPS_datastream::alpha_filter_lat_lon_alt()" << endl;

// Perform alpha filtering of raw GPS measurements.  Recall smaller
// values for alpha imply more filtering with previously measured
// values:

   const double alpha=0.01;
//   const double alpha=0.03;
//   const double alpha=0.1;
//   const double alpha=0.2;
//   const double alpha=0.9;
//   const double alpha=0.999;

   alpha_filter_lat_lon_alt(alpha);
}

void GPS_datastream::alpha_filter_lat_lon_alt(double alpha)
{
//   cout << "inside GPS_datastream::alpha_filter_lat_lon_alt()" << endl;
   
   if (lat_lon_alt_counter==0)
   {
      curr_alpha_filtered_lat_lon_alt=curr_lat_lon_alt;
   }
   else
   {
      curr_alpha_filtered_lat_lon_alt=filterfunc::alpha_filter(
         curr_lat_lon_alt,prev_alpha_filtered_lat_lon_alt,alpha);
   }   

   prev_alpha_filtered_lat_lon_alt=curr_alpha_filtered_lat_lon_alt;
   lat_lon_alt_counter++;

   filtered_geo_posn=geopoint(
      get_alpha_filtered_lon(),get_alpha_filtered_lat(),
      get_alpha_filtered_alt());
}
