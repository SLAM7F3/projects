// ==========================================================================
// Header file for Clock class 
// ==========================================================================
// Last modified on 2/11/12; 4/5/12; 5/23/12; 5/14/13
// ==========================================================================

#ifndef CLOCK_H
#define CLOCK_H

#include <sys/time.h>
#include "math/threevector.h"
#include "time/timefuncs.h"

class Clock
{
   
  public:

// Initialization, constructor and destructor functions:

   Clock();
   Clock(const Clock& C);
   ~Clock();
//   Clock operator= (const Clock& C);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const Clock& C);

// Set and get member functions

   void set_daylight_savings_flag(bool daylight_savings);
   void set_local_time(
      int year,int month,int day,int hour,int minute,double sec);
   void set_UTC_time(
      int year,int month,int day,int hour,int minute,double sec);
   void set_year_month_day(int year,int month,int day);
   int get_year() const;
   int get_month() const;
   std::string get_month_name() const;
   int get_day() const;
   int get_UTC_hour() const;
   int get_local_hour() const;
   int get_minute() const;
   double get_seconds() const;

   double get_decimal_day() const;
   double get_juliandate() const;
   double get_decimal_year() const;
   double get_phi_greenwich() const;

// Local computer time member functions:

   void set_initial_truetime();
   const time_t& get_initial_truetime() const;
   void set_time_based_on_local_computer_clock(
      int UTM_zonenumber,bool daylight_savings_flag);
   void set_time_based_on_local_computer_clock();

// Epoch time member functions

   double set_reference_date(int yr,int mnth,double dec_day);
   double get_reference_date() const;
   double secs_elapsed_since_reference_date() const;
   double secs_elapsed_since_reference_date(
      int yr,int mnth,double dec_day) const;
   double secs_elapsed_since_initial_truetime();
   void convert_elapsed_secs_to_date(double secs_elapsed);

// MATLAB datenum conversion member functions

   double datenum_to_elapsed_secs(double datenum);
   double elapsed_secs_to_datenum(double elapsed_secs);

// GPS time conversion member functions:

   double GPS_time_to_elapsed_secs(
      int GPS_week_number,double secs_into_GPS_week);
   void elapsed_secs_to_GPS_time(
      int& GPS_week_number,double& secs_into_GPS_week);
   void elapsed_secs_to_GPS_time(
      double secs_since_epoch,int& GPS_week_number,double& secs_into_GPS_week);

// UTC to local time conversion member functions

   int UTC_to_local_hour(int hour_utc);
   int local_to_UTC_hour(int hour_local);

   void set_UTM_zone_time_offset(int offset);
   int get_UTM_zone_time_offset() const;
   int compute_UTM_zone_time_offset(int UTM_zone);
   void current_local_time_and_UTC();
   void compute_local_time_and_UTC(const time_t& truetime);

   std::string YYYY_MM_DD_H_M_S(
      std::string day_hour_separator_char=" ",
      std::string time_separator_char=":",bool display_UTC_flag=true,
      int n_secs_digits=2,bool display_year_month_day_flag=true);
   std::string ISO_format();
   double timestamp_string_to_elapsed_secs(
      std::string timestamp,bool UTC_flag);
   void parse_YYYYMMDD_string(
      std::string YYYYMMDD,int& year,int& month,int& day);
   void parse_HHMMSS_string(
      std::string HHMMSS,int& hour,int& minute,double& secs);

   bool reset_local_computer_time();

// Astronomical parameter member functions:

   void compute_astronomical_params();
   const threevector& get_sun_direction_ECI() const;

  private: 

   bool local_daylight_savings_flag;
   int year,month,day;
   int UTM_zone_time_offset;	// -12 hours to +12 hours
   int local_hour,UTC_hour,min;
   double sec,decimal_day,decimal_year;
   double reference_julian_date;
   time_t initial_truetime,curr_truetime;

// Time-dependent astronomical parameters:

   double juliandate,phi_greenwich;
   threevector sun_direction_ECI;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const Clock& C);

   void set_time_params(int year,int month,int day,int minute,double sec);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

inline void Clock::set_daylight_savings_flag(bool daylight_savings)
{
   local_daylight_savings_flag=daylight_savings;
}

inline int Clock::get_UTM_zone_time_offset() const
{
   return UTM_zone_time_offset;
}

// ---------------------------------------------------------------------
inline int Clock::get_year() const
{
   return year;
}

inline int Clock::get_month() const
{
   return month;
}

inline int Clock::get_day() const
{
   return day;
}

inline double Clock::get_decimal_day() const
{
   return decimal_day;
}

inline int Clock::get_UTC_hour() const
{
   return UTC_hour;
}

inline int Clock::get_local_hour() const
{
   return local_hour;
}

inline int Clock::get_minute() const
{
   return min;
}

inline double Clock::get_seconds() const
{
   return sec;
}

inline double Clock::get_juliandate() const
{
   return juliandate;
}

inline double Clock::get_decimal_year() const
{
   return decimal_year;
}

inline double Clock::get_phi_greenwich() const
{
   return phi_greenwich;
}

inline const threevector& Clock::get_sun_direction_ECI() const
{
   return sun_direction_ECI;
}

inline void Clock::set_initial_truetime()
{
   initial_truetime=time(NULL);
}

inline const time_t& Clock::get_initial_truetime() const
{
   return initial_truetime;
}


#endif  // Clock.h


