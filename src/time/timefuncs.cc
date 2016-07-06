// ==========================================================================
// Some useful time functions 
// ==========================================================================
// Last updated on 7/11/13; 7/24/13; 1/6/14; 11/28/15
// ==========================================================================

#include <iostream>
#include "math/mathfuncs.h"
#include "time/timefuncs.h"

using std::cout;
using std::endl;
using std::ofstream;
using std::string;

namespace timefunc
{
   struct timeval starttime_tv,currtime_tv;
   struct timezone tz;

// ---------------------------------------------------------------------
   
   double hms_to_frac_day(int hours,int minutes,double seconds)
      {
         return (hours+minutes/60.0+seconds/3600.0)/24.0;
      }
   
   void frac_day_to_hms(
      double frac_day,int& hours,int& minutes,double& secs)
      {
//         cout << "inside timefunc::frac_day_to_hms()" << endl;
         double decimal_hours=frac_day*24.0;
         hours=basic_math::mytruncate(decimal_hours);
         double delta_minutes=60*(decimal_hours-hours);
         minutes=basic_math::mytruncate(delta_minutes);
         secs=60*(delta_minutes-minutes);
//         cout << "hours = " << hours << " mins = " << minutes
//              << " secs = " << secs << endl;
      }

// ---------------------------------------------------------------------
// Method initialize_timeofday_clocks initializes the values within
// the global namespace timeval structure starttime_tv.  In May 2004,
// we learned (after much hassle!) that the gettimeofday() system call
// returns the closest thing to true wrist-watch time which is
// accurate to the microsecond level.  [ The clock() function returns
// only elapsed CPU time for the calling process, while ftime() is
// deprecated. ]

// Subsequent calls to elapsed_timeofday_time() should return genuine
// wrist-watch elapsed time in seconds since the call to
// initialize_timeofday_clock().

// As Vadim reminded us on 5/14/04, the data structures used by
// various system timing methods such as gettimeofday() must be
// allocated BEFORE calls to these methods are made!
   
   void initialize_timeofday_clock()
      {
         initialize_timeofday_clock(&starttime_tv);
      }

   void initialize_timeofday_clock(timeval* starttime_tv_ptr)
      {
//         cout << "inside initialize_timeofday_clock()" << endl;
         gettimeofday(starttime_tv_ptr,&tz);
      }
   
   double elapsed_timeofday_time()
      {
         return elapsed_timeofday_time(&starttime_tv);
      }

   double elapsed_timeofday_time(timeval const *starttime_tv_ptr)
      {
         gettimeofday(&currtime_tv,&tz);
         long seconds=currtime_tv.tv_sec-starttime_tv_ptr->tv_sec;
         long microseconds=currtime_tv.tv_usec-starttime_tv_ptr->tv_usec;
         return seconds+1E-6*microseconds;
      }

   void get_elapsed_time(
      double& elapsed_secs,double& elapsed_mins,double& elapsed_hours)
   {
      elapsed_secs=elapsed_timeofday_time();
      elapsed_mins=elapsed_secs/60;
      elapsed_hours=elapsed_mins/60;
   }

// ---------------------------------------------------------------------
// Routine getcurrUTC() returns the current UTC in ascii form as a C++
// string.  This routine returns the equivalent of the Unix "date -u"
// system call:

   string getcurrUTC()
      {
         time_t curr_truetime=time(NULL);
         string currUTC_str=asctime(gmtime(&curr_truetime));
         return currUTC_str;
      }

// ---------------------------------------------------------------------
// Function utc_localtime_diff_in_hours returns the difference between
// Coordinated Universal Time (UTC) and local standard time in hours.
// This method takes into account variation due to daylight
// savings.  See the CTIME(3) manual page.

//   int utc_localtime_diff_in_hours()
//      {
//         time_t currtime=time(NULL);
//         ctime(&currtime);
//         extern long int timezone;
//         int hourdiff=mathfunc::round(timezone/3600);
//         return hourdiff;
//      }

// ---------------------------------------------------------------------
// Function utc_mktime converts a broken-down time structure,
// expressed as Coordinated Universal Time (UTC), to calendar time
// representation which corresponds to the number of elapsed seconds
// since 00:00:00 on Jan 1, 1970.  This method is our attempt to
// generalize the mktime() function which performs the analogous
// conversion for a broken-down time structure expressed as *local*
// time.  See the CTIME(3) manual page.

   time_t utc_mktime(tm& utctm,int hourshift)
      {
         tm localtm;
         localtm.tm_sec=utctm.tm_sec;
         localtm.tm_min=utctm.tm_min;
         localtm.tm_hour=utctm.tm_hour-hourshift;
         localtm.tm_mday=utctm.tm_mday;
         localtm.tm_mon=utctm.tm_mon;
         localtm.tm_year=utctm.tm_year;
         localtm.tm_wday=utctm.tm_wday;
         localtm.tm_yday=utctm.tm_yday;
         localtm.tm_isdst=utctm.tm_isdst;
         return mktime(&localtm);
      }

// ---------------------------------------------------------------------
// Function secs_since_Y2K() returns number of seconds elapsed in
// current timezone since 1 Jan 2000.  This code snippet comes from 
// http://www.cplusplus.com/reference/ctime/time/

   int secs_since_Y2K()
   {
      time_t timer;
      time(&timer);  /* get current time; same as: timer = time(NULL)  */

      struct tm y2k;
      y2k.tm_hour = 0;   y2k.tm_min = 0; y2k.tm_sec = 0;
      y2k.tm_year = 100; y2k.tm_mon = 0; y2k.tm_mday = 1;
      
      int seconds = difftime(timer,mktime(&y2k));
      return seconds;
   }

} // timefunc namespace






