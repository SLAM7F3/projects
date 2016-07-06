// ==========================================================================
// Clock class member function definitions
// ==========================================================================
// Last modified on 5/14/13; 10/29/13; 12/3/13
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "astro_geo/astrofuncs.h"
#include "astro_geo/Clock.h"
#include "astro_geo/geofuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void Clock::allocate_member_objects()
{
}		       

void Clock::initialize_member_objects()
{
   local_daylight_savings_flag=false;
   UTM_zone_time_offset=0;

// On Feb 24, 2008, we confirmed that the following call [ and NOT
// set_reference_date(1970,1,0) ! ] yields seconds elapsed since epoch
// consistent with multiple online date calculators...

   set_reference_date(1970,1,1);	// Midnight on 1 Jan 1970
}		       

Clock::Clock()
{
   allocate_member_objects();
   initialize_member_objects();
}

// Copy constructor:

Clock::Clock(const Clock& C)
{
//   docopy(C);
}

// ---------------------------------------------------------------------
Clock::~Clock()
{
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const Clock& c)
{
   outstream << endl;
   outstream << "year = " << c.year << " month = " << c.month 
             << " day = " << c.day << " decimal day = "
             << c.decimal_day << endl;
   outstream << "local_hour = " << c.local_hour << " UTC_hour = "
             << c.UTC_hour << " min = " << c.min 
             << " sec = " << c.sec << endl;
   outstream << "decimal year = " << c.decimal_year << endl;
   outstream << "phi_greenwich = " << c.phi_greenwich*180/PI << endl;


   return outstream;
}

// ==========================================================================
// Set and get member functions
// ==========================================================================

void Clock::set_time_params(int year,int month,int day,int minute,double sec)
{
   set_year_month_day(year,month,day);
   min=minute;
   this->sec=sec;

   compute_astronomical_params();
}

// ---------------------------------------------------------------------
void Clock::set_year_month_day(int year,int month,int day)
{
//   cout << "inside Clock::set_year_month_day()" << endl;
   this->year=year;
   this->month=month;
   this->day=day;
}

// ---------------------------------------------------------------------
void Clock::set_local_time(
   int year,int month,int day,int hour,int minute,double sec)
{
//   cout << "inside Clock::set_local_time()"  << endl;
   local_hour=hour;
   UTC_hour=local_to_UTC_hour(local_hour);
//   cout << "UTC_hour = " << UTC_hour << endl;
//   cout << "local_hour = " << local_hour << endl;
   set_time_params(year,month,day,minute,sec);
}

// ---------------------------------------------------------------------
void Clock::set_UTC_time(
   int year,int month,int day,int hour,int minute,double sec)
{
//   cout << "inside Clock::set_UTC_time()" << endl;

   UTC_hour=hour;
   local_hour=UTC_to_local_hour(UTC_hour);
//   cout << "UTC_hour = " << UTC_hour << endl;
//   cout << "local_hour = " << local_hour << endl;
   set_time_params(year,month,day,minute,sec);
}

// ---------------------------------------------------------------------
string Clock::get_month_name() const
{
//   cout << "inside Clock::get_month_name()" << endl;

   if (month==1)
   {
      return "Jan";
   }
   else if (month==2)
   {
      return "Feb";
   }
   else if (month==3)
   {
      return "Mar";
   }
   else if (month==4)
   {
      return "Apr";
   }
   else if (month==5)
   {
      return "May";
   }
   else if (month==6)
   {
      return "Jun";
   }
   else if (month==7)
   {
      return "Jul";
   }
   else if (month==8)
   {
      return "Aug";
   }
   else if (month==9)
   {
      return "Sep";
   }
   else if (month==10)
   {
      return "Oct";
   }
   else if (month==11)
   {
      return "Nov";
   }
   else if (month==12)
   {
      return "Dec";
   }
   return "undefined";
}

// ==========================================================================
// Local computer time member functions
// ==========================================================================

// Member function set_time_based_on_local_computer_clock() calls Unix
// command localtime_r() to reset current Clock object's local and
// UTC time.  

void Clock::set_time_based_on_local_computer_clock(
   int UTM_zonenumber,bool daylight_savings_flag)
{
   set_initial_truetime();
   struct tm* result_ptr=new tm;
   localtime_r(&initial_truetime,result_ptr);

   int year=1900+result_ptr->tm_year;
   int month=1+result_ptr->tm_mon;
   int day=result_ptr->tm_mday;
   int hour=result_ptr->tm_hour;
   int mins=result_ptr->tm_min;
   int secs=result_ptr->tm_sec;

//   cout << "year = " << year << endl;
//   cout << "month = " << month << endl;
//   cout << "day = " << day << endl;
//   cout << "hour = " << hour << endl;
//   cout << "mins = " << mins << endl;
//   cout << "secs = " << secs << endl;

   compute_UTM_zone_time_offset(UTM_zonenumber);
   set_daylight_savings_flag(daylight_savings_flag);
   set_local_time(year,month,day,hour,mins,secs);
}

// This overloaded version of set_time_based_on_local_computer_clock()
// assumes that the UTM_zone_time_offset and daylight_savings_flag
// have previously been set.

void Clock::set_time_based_on_local_computer_clock()
{
   set_initial_truetime();
   struct tm* result_ptr=new tm;
   localtime_r(&initial_truetime,result_ptr);

   int year=1900+result_ptr->tm_year;
   int month=1+result_ptr->tm_mon;
   int day=result_ptr->tm_mday;
   int hour=result_ptr->tm_hour;
   int mins=result_ptr->tm_min;
   int secs=result_ptr->tm_sec;

//   cout << "year = " << year << endl;
//   cout << "month = " << month << endl;
//   cout << "day = " << day << endl;
//   cout << "hour = " << hour << endl;
//   cout << "mins = " << mins << endl;
//   cout << "secs = " << secs << endl;

   set_local_time(year,month,day,hour,mins,secs);
}

// ==========================================================================
// Epoch time member functions
// ==========================================================================

// Member function set_reference_date()

double Clock::set_reference_date(int yr,int mnth,double dec_day)
{
   reference_julian_date=astrofunc::julian_day(yr,mnth,dec_day);
//   cout << "reference_julian_date = " << reference_julian_date
//        << endl;
   return reference_julian_date;
}

double Clock::get_reference_date() const
{
   return reference_julian_date;
}

// ---------------------------------------------------------------------
// Member function secs_elapsed_since_reference_date() returns the
// difference between the current julian date and the reference julian
// date measured in seconds.

double Clock::secs_elapsed_since_reference_date() const
{
   return secs_elapsed_since_reference_date(year,month,decimal_day);
}

double Clock::secs_elapsed_since_reference_date(
   int yr,int mnth,double dec_day) const
{
   double julian_date=astrofunc::julian_day(yr,mnth,dec_day);
   double days_elapsed=julian_date-reference_julian_date;
   double secs_elapsed=24*3600*days_elapsed;
   return secs_elapsed;
}

// ---------------------------------------------------------------------
// Member function secs_elapsed_since_initial_truetime() assumes that
// some main program has previously called set_initial_truetime().
// This method returns the difference between that time and the
// current time measured in seconds.

double Clock::secs_elapsed_since_initial_truetime() 
{
   compute_local_time_and_UTC(initial_truetime);
   double initial_truetime_in_secs=secs_elapsed_since_reference_date();
   current_local_time_and_UTC();
   double current_truetime_in_secs=secs_elapsed_since_reference_date();
   return current_truetime_in_secs-initial_truetime_in_secs;
}

// ---------------------------------------------------------------------
// Member function convert_elapsed_secs_to_date() performs the inverse
// operation to secs_elapsed_since_reference_date().

void Clock::convert_elapsed_secs_to_date(double secs_elapsed)
{
//   cout << "inside Clock::convert_elapsed_secs_to_date()" << endl;
   double days_elapsed=secs_elapsed/(24.0*3600.0);
   double julian_date=reference_julian_date+days_elapsed;
   astrofunc::julian_to_calendar_date(
      julian_date,year,month,decimal_day);
   day=basic_math::mytruncate(decimal_day);
   double frac_day=decimal_day-day;
//   cout << "frac_day = " << frac_day << endl;
   timefunc::frac_day_to_hms(frac_day,UTC_hour,min,sec);
//   cout << "min = " << min << " sec = " << sec << endl;

//   cout << "UTC_hour = " << UTC_hour << endl;
   local_hour=UTC_to_local_hour(UTC_hour);
//   cout << "local_hour = " << local_hour << endl;
   set_time_params(year,month,day,min,sec);
}

// ==========================================================================
// MATLAB datenum conversion member functions
// ==========================================================================

// Member function datenum_to_elapsed_secs() takes in double datenum
// which represents the number of *days* since 1 January 0000.  It
// returns the corresponding number of seconds elapsed since 1 Jan
// 1970 at midnight.

double Clock::datenum_to_elapsed_secs(double datenum)
{
//   cout << "inside Clock::datenum_to_elapsed_secs()" << endl;

   const double reference_datenum=719529;	// 1 Jan 1970 at midnight
   double delta_datenum=datenum-reference_datenum;
   double elapsed_secs=24*3600*delta_datenum;
   return elapsed_secs;
}

double Clock::elapsed_secs_to_datenum(double elapsed_secs)
{
//   cout << "inside Clock::datenum_to_elapsed_secs()" << endl;

   const double reference_datenum=719529;	// 1 Jan 1970 at midnight

   double delta_datenum=elapsed_secs/(24.0*3600.0);
   double datenum=reference_datenum+delta_datenum;
   return datenum;
}

// ==========================================================================
// GPS time conversion member functions
// ==========================================================================

// Member function GPS_time_to_elapsed_secs() takes in GPS_week_number
// and secs_into_GPS_week.  It computes the number of secs elapsed
// since the GPS reference date [6 Jan 1980 at 0 hour].  This method
// returns the corresponding UTC time measured in secs since Jan 1,
// 1970 at midnight.

double Clock::GPS_time_to_elapsed_secs(
   int GPS_week_number,double secs_into_GPS_week)
{
//   cout << "inside Clock::GPS_time_to_elapsed_secs()" << endl;

   int year=1980;
   int month=1;
   int day=6;
   int hour=0;
   int minute=0;
   double secs=0;

//   int year=1999;
//   int month=8;
//   int day=21;
//   int hour=23;
//   int minute=59;
//   double secs=47;
   
   Clock GPS_clock;
   GPS_clock.set_UTC_time(year,month,day,hour,minute,secs);
   double GPS_time_origin=GPS_clock.secs_elapsed_since_reference_date();
   
   const int secs_per_GPS_week=7*24*60*60;
   double secs_since_GPS_time_origin=
      GPS_week_number*secs_per_GPS_week+secs_into_GPS_week;
   double secs_since_epoch=GPS_time_origin+secs_since_GPS_time_origin;

   const int leap_secs_adjustment=15; // Good as of 2010
   secs_since_epoch -= leap_secs_adjustment;

   return secs_since_epoch;
}

// ---------------------------------------------------------------------
// Member function elapsed_secs_to_GPS_time() converts secs since
// epoch into integer GPS_week_number and double secs_into_GPS_week.

void Clock::elapsed_secs_to_GPS_time(
   int& GPS_week_number,double& secs_into_GPS_week)
{
   double secs_since_epoch=secs_elapsed_since_reference_date();
   elapsed_secs_to_GPS_time(
      secs_since_epoch,GPS_week_number,secs_into_GPS_week);
}

void Clock::elapsed_secs_to_GPS_time(
   double secs_since_epoch,int& GPS_week_number,double& secs_into_GPS_week)
{
//   cout << "inside Clock::GPS_time_to_elapsed_secs()" << endl;

   const int leap_secs_adjustment=15; // Good as of 2010
   secs_since_epoch += leap_secs_adjustment;

   int year=1980;
   int month=1;
   int day=6;
   int hour=0;
   int minute=0;
   double secs=0;

   Clock GPS_clock;
   GPS_clock.set_UTC_time(year,month,day,hour,minute,secs);
   double GPS_time_origin=GPS_clock.secs_elapsed_since_reference_date();

   double secs_since_GPS_time_origin=secs_since_epoch-GPS_time_origin;
   int integer_secs_since_GPS_time_origin=
      basic_math::mytruncate(secs_since_GPS_time_origin);
//   double frac_secs_since_GPS_time_origin=
//      secs_since_GPS_time_origin-integer_secs_since_GPS_time_origin;

   const int secs_per_GPS_week=7*24*60*60;

   GPS_week_number=integer_secs_since_GPS_time_origin/secs_per_GPS_week;
   secs_into_GPS_week=secs_since_GPS_time_origin-
      GPS_week_number*secs_per_GPS_week;

//   cout << "GPS_week_number = " << GPS_week_number << endl;
//   cout << "secs_into_GPS_week = " << secs_into_GPS_week << endl;
}

// ==========================================================================
// UTC to local time conversion member functions
// ==========================================================================

// Member function UTC_to_local_hour() and local_to_UTC_hour() take
// daylight savings into account. 

// Recall local hour = UTC + UTM_zone_time_offset + daylight_savings_flag

int Clock::UTC_to_local_hour(int hour_utc)
{
//   cout << "inside Clock::UTC_to_local_hour()" << endl;
//   cout << "UTM_zone_time_offset = " << UTM_zone_time_offset << endl;

   int hour_local=hour_utc+UTM_zone_time_offset;
   if (local_daylight_savings_flag) hour_local++;
   return hour_local;
}

int Clock::local_to_UTC_hour(int hour_local)
{
//   cout << "inside Clock::local_to_UTC_hour()" << endl;
//   cout << "UTM_zone_time_offset = " << UTM_zone_time_offset << endl;

   int hour_utc=local_hour-UTM_zone_time_offset;
   if (local_daylight_savings_flag) hour_utc--;
   return hour_utc;
}

// ---------------------------------------------------------------------
void Clock::set_UTM_zone_time_offset(int offset)
{
//   cout << "inside Clock::set_UTM_zone_time_offset()" << endl;
   
   UTM_zone_time_offset=offset;
   if (UTM_zone_time_offset > 12)
   {
      UTM_zone_time_offset -= 24;
   }

//   cout << "UTM_zone_time_offset = " << UTM_zone_time_offset 
//        << " hours relative to Greenwich" << endl;
}

// ---------------------------------------------------------------------
// Recall that each UTM zone corresponds to 6 degrees of earth
// longitude.  Eastings are measured from the center of each UTM zone.
// Moreover, the UTM zonenumber for Greenwich equals 30.  This member
// function takes in a UTM_zonenumber and returns the corresponding
// UTM zone time offset measured in integer hours.

int Clock::compute_UTM_zone_time_offset(int UTM_zonenumber)
{
//   cout << "inside Clock::compute_UTM_zone_time_offset()" << endl;
//   cout << "UTM_zonenumber = " << UTM_zonenumber << endl;
   double dt=0.4*(UTM_zonenumber-30);
   UTM_zone_time_offset=basic_math::mytruncate(dt);
//   cout << "UTM_zone_time_offset = " << UTM_zone_time_offset << endl;
   return UTM_zone_time_offset;
}

// ---------------------------------------------------------------------
void Clock::current_local_time_and_UTC()
{
//   cout << "inside Clock::current_local_time_and_UTC()" << endl;
   curr_truetime=time(NULL);
   compute_local_time_and_UTC(curr_truetime);
}

// ---------------------------------------------------------------------
// Note added on 7/7/11: The daylight savings time flag is NOT always
// correctly reported via calls to gmtime() !  So this method should
// be deprecated compared to set_time_based_on_local_computer_clock()
// where both the UTM_zonenumber and daylight_savings_flag are passed
// as explicit input arguments.

void Clock::compute_local_time_and_UTC(const time_t& truetime)
{
//   cout << "inside Clock::compute_local_time_and_UTC()" << endl;

   tm* tm_ptr=new tm(*gmtime(&truetime));
   tm* local_tm_ptr=new tm(*localtime(&truetime));

   year=1900+tm_ptr->tm_year;

// Note:  gmtime's month integer runs from 0 to 11.  

// But month member variable needs to run from 1 - 12 for astronomy
// subroutines.  So we add 1 to tm_ptr->tm_mon below:

   month=tm_ptr->tm_mon+1;
   day=tm_ptr->tm_mday;
   UTC_hour=tm_ptr->tm_hour;
   local_hour=local_tm_ptr->tm_hour;
   min=tm_ptr->tm_min;
   sec=tm_ptr->tm_sec;
   local_daylight_savings_flag=(local_tm_ptr->tm_isdst > 0);
   set_UTM_zone_time_offset(local_hour-local_daylight_savings_flag-UTC_hour);

//   cout << "year = " << year << " month = " << month << endl;
//   cout << "day = " << day << endl;
//   cout << "UTC hour = " << UTC_hour << endl;
//   cout << "local hour = " << local_hour << endl;
//   cout << "min = " << min << endl;
//   cout << "sec = " << sec << endl;
//   cout << "daylight savings flag = " << local_daylight_savings_flag << endl;
//   cout << "UTM_zone_time_offset = " << UTM_zone_time_offset << endl;

   delete tm_ptr;
   delete local_tm_ptr;

   compute_astronomical_params();
}

// ---------------------------------------------------------------------
// This next member function generates a date/time string which can be
// read into a Postgres database.

// On 8/22/10, we empirically found that we could not create
// subdirectories on SD cards named like
// webcam_images_2010-08-22_8:48:31.00.  But if the colons in the
// time stamp part of the directory name are replaced by underscores,
// we can create the folder on an SD chip.  So this overloaded version
// of YYYY_MM_DD_H_M_S() allows for this naming flexibility.

string Clock::YYYY_MM_DD_H_M_S(
   string day_hour_separator_char,string time_separator_char,
   bool display_UTC_flag,int n_secs_digits,
   bool display_year_month_day_flag)
{
//   cout << "inside Clock::YYYY_MM_DD_H_M_S()" << endl;

   string date_string="";

   if (display_year_month_day_flag)
   {
      date_string=stringfunc::integer_to_string(year,4);
      date_string += "-"+stringfunc::integer_to_string(month,2);
   }

//   cout << "day = " << day << endl;
//   cout << "UTC_hour = " << UTC_hour << endl;
//   cout << "local_hour = " << local_hour << endl;

   int day_to_display=day;
   int UTC_hour_to_display=UTC_hour;
   int local_hour_to_display=local_hour;
   int min_to_display=min;
   double sec_to_display=sec;
   if (sec_to_display > 59.99)
   {
      min_to_display++;
      sec_to_display=0;
   }

   if (min_to_display >= 60)
   {
      UTC_hour_to_display++;
      local_hour_to_display++;
      min_to_display=0;
   }

   if (display_UTC_flag && UTC_hour_to_display >= 24)
   {
      day_to_display++;
      UTC_hour_to_display -= 24;
   }
   else if (display_UTC_flag && UTC_hour_to_display < 0)
   {
      day_to_display--;
      UTC_hour_to_display += 24;
   }
   else if (!display_UTC_flag && local_hour_to_display >= 24)
   {
      day_to_display++;
      local_hour_to_display -= 24;
   }
   else if (!display_UTC_flag && local_hour_to_display < 0)
   {
      day_to_display--;
      local_hour_to_display += 24;
   }

   if (display_year_month_day_flag)
      date_string += "-"+stringfunc::integer_to_string(day_to_display,2);

   if (display_UTC_flag)
   {
      date_string += day_hour_separator_char
         +stringfunc::integer_to_string(UTC_hour_to_display,2);
   }
   else
   {
      date_string += day_hour_separator_char
         +stringfunc::integer_to_string(local_hour_to_display,2);
   }

   date_string += time_separator_char+
	stringfunc::integer_to_string(min_to_display,2);

// Make sure integer part of sec_to_display always takes up 2 digits.
// Add leading zero if necessary to fulfill this consistency condition:

//   date_string += time_separator_char+
//	stringfunc::number_to_string(sec_to_display,n_secs_digits);

   int int_secs=basic_math::mytruncate(sec_to_display);
   double frac_secs=sec_to_display-int_secs;
   int fraction_secs=basic_math::round(frac_secs*pow(10,n_secs_digits));

// On 10/29/13, we empirically found that sec_to_display can be very
// close but less than an integer (e.g. 2.999999903).  In this case,
// fraction_secs can actually equal 100.  So add in following logic to
// handle this edge case:

   if (fraction_secs >= 100)
   {
      int_secs++;
      fraction_secs -= 100;
   }

//   cout << "sec_to_display = " << sec_to_display << endl;
//   cout << "int_secs = " << int_secs << " frac_secs = " << frac_secs
//        << " fraction_secs = " << fraction_secs << endl;

   date_string += time_separator_char+
      stringfunc::integer_to_string(int_secs,2);
   date_string += "."+stringfunc::integer_to_string(
      fraction_secs,n_secs_digits);

   if (display_UTC_flag && display_year_month_day_flag)
   {
      date_string += " UTC";
   }
   else if (!display_UTC_flag && display_year_month_day_flag)
   {
      date_string += " local";
   }
   
   return date_string;
}

// ---------------------------------------------------------------------
// Member ISO_format() returns the UTC in ISO 8601 format which is the
// International Standard for the representation of dates and times.

string Clock::ISO_format()
{
//   cout << "inside Clock::ISO_form()" << endl;
   string iso8601_format=YYYY_MM_DD_H_M_S("T",":",true,2);
   iso8601_format += "Z";
   return iso8601_format;
}

// ---------------------------------------------------------------------
// Member function reset_local_computer_time() runs the local
// computer's date command to reset the computer's clock to the
// current Clock object's local time.  We wrote this utility method in
// July 2010 in order to synchronize a local machine's clock with GPS
// time read in via a GPS unit attached as a USB device.  If the local
// machine's clock is successfully reset, this boolean method returns
// true. This method can only be run by the root superuser.

bool Clock::reset_local_computer_time()
{
   cout << "inside Clock::reset_local_computer_time()" << endl;
   bool display_UTC_flag=false;
   string unix_cmd="date -s '"+
      YYYY_MM_DD_H_M_S(" ",":",display_UTC_flag)+"'";
   cout << "unix_cmd = " << unix_cmd << endl;
   int flag=sysfunc::unix_command(unix_cmd);
   cout << "flag = " << flag << endl;
   if (flag==-1 || flag==127 || flag==256)
   {
      return false;
   }
   else
   {
      string banner="Computer clock reset to GPS time";
      outputfunc::write_big_banner(banner);
      return true;
   }
}

// ==========================================================================
// Astronomical parameter member functions
// ==========================================================================

void Clock::compute_astronomical_params()
{
//   cout << "inside Clock::compute_astronomical_params()" << endl;

   decimal_day=day+timefunc::hms_to_frac_day(UTC_hour,min,sec);
   juliandate=astrofunc::julian_day(year,month,decimal_day);

   double newyear_juliandate=astrofunc::julian_day(year,1,0);
   double endyear_juliandate=astrofunc::julian_day(year+1,1,0);
   double ndays_in_year=endyear_juliandate-newyear_juliandate;
   double year_frac=(juliandate-newyear_juliandate)/ndays_in_year;
   decimal_year=year+year_frac;

   phi_greenwich=geofunc::compute_phi_greenwich(juliandate);

   double alpha,delta;
   astrofunc::sun_right_ascension_declination(
      year,month,decimal_day,alpha,delta);
   double sun_RA=180/PI*alpha;	// degs
   double sun_DEC=180/PI*delta;	// degs
   sun_direction_ECI=astrofunc::ECI_vector(sun_RA,sun_DEC);

//   cout << "UTC hour = " << UTC_hour << endl;
//   cout << "UTC decimal_day = " << decimal_day << endl;
//   cout << "julian date = " << juliandate << endl;
//   cout << "decimal year = " << decimal_year << endl;
//   cout << "phi_greenwich = " << phi_greenwich*180/PI << " degs" << endl;
//   cout << "SUN_direction_ECI = " << sun_direction_ECI << endl;
}

// ---------------------------------------------------------------------
// Member function timestamp_string_to_elapsed_secs() takes in a
// timestamp string of the form YYYY_MM_DD_H_M_S as well as a bool
// flag indicating UTC or local time.  This method returns the
// corresponding number of seconds elapsed since the reference epoch.

double Clock::timestamp_string_to_elapsed_secs(
   string curr_timestamp,bool UTC_flag)
{
//   cout << "inside Clock::timestamp_string_to_elapsed_secs()" << endl;
//   cout << "curr_timestamp = " << curr_timestamp << endl;
   
   vector<string> time_substrings=
      stringfunc::decompose_string_into_substrings(curr_timestamp);

   string ymd,hms;
   if (time_substrings.size()==2)
   {
      ymd=time_substrings[0];
      hms=time_substrings[1];

// UTC timestamps stored in Postgres databases can take the form
// 2010-08-29 21:25:05.9-04.  Here the last -04 indicates that the UTM
// zone runs 4 hours behind GMT.  We need to eliminate any such local
// time zone indicator from the hms substring:

      hms=stringfunc::prefix(hms,"-");
   }
   else if (time_substrings.size()==1)
   {
      ymd=curr_timestamp.substr(0,10);
      hms=curr_timestamp.substr(11,8);
   }
//   cout << "ymd = " << ymd << " hms = " << hms << endl;

   int year=stringfunc::string_to_number(ymd.substr(0,4));
   int month=stringfunc::string_to_number(ymd.substr(5,2));
   int day=stringfunc::string_to_number(ymd.substr(8,2));

   int hour=stringfunc::string_to_number(hms.substr(0,2));
   int minute=stringfunc::string_to_number(hms.substr(3,2));
   double second=stringfunc::string_to_number(hms.substr(6,hms.size()-6));

//   double fractional_sec=0.001*stringfunc::string_to_number(frac_sec);
//   second += fractional_sec;

//   cout << " y = " << year
//        << " m = " << month
//        << " d = " << day 
//        << " h = " << hour
//        << " min = " << minute
//        << " sec = " << second << endl << endl;
   
   if (UTC_flag)
   {
      set_UTC_time(year,month,day,hour,minute,second);
   }
   else
   {
      set_local_time(year,month,day,hour,minute,second);
   }
   return secs_elapsed_since_reference_date(); 
}

// ---------------------------------------------------------------------
// Member function parse_YYYYMMDD_string() takes in a string of the form
// YYYYMMDD.  It extracts and returns year, month and day information.

void Clock::parse_YYYYMMDD_string(
   string YYYYMMDD,int& year,int& month,int& day)
{
   double yyyymmdd=stringfunc::string_to_number(YYYYMMDD);
   year=yyyymmdd/10000;
//   cout << "year = " << year << endl;
   yyyymmdd -= year*10000;
   month=yyyymmdd/100;
//   cout << "month = " << month << endl;
   yyyymmdd -= month*100;
   day=yyyymmdd;
//   cout << "day = " << day << endl;
}

// ---------------------------------------------------------------------
// Member function parse_HHMMSS_string() takes in a string of the form
// HHMMSS.  It extracts and returns hour, minute and second component
// information.

void Clock::parse_HHMMSS_string(string HHMMSS,int& hour,int& minute,double& secs)
{
   double hhmmss=stringfunc::string_to_number(HHMMSS);
   hour=hhmmss/10000;
//   cout << "hour = " << hour << endl;
   hhmmss -= hour*10000;
   minute=hhmmss/100;
//   cout << "minute = " << minute << endl;
   hhmmss -= minute*100;
   secs=hhmmss;
//   cout << "secs = " << secs << endl;
}
