// ==========================================================================
// Timefuncs namespace header
// ==========================================================================
// Last updated on 7/11/13; 7/24/13; 1/6/14; 11/28/15
// ==========================================================================

#ifndef TIMEFUNCS_H
#define TIMEFUNCS_H

#include <iostream>
#include <string>
#include <sys/time.h>

// ==========================================================================
// Function declarations
// ==========================================================================

namespace timefunc
{
   double hms_to_frac_day(int hours,int minutes,double seconds);
   void frac_day_to_hms(double frac_day,int& hours,int& minutes,double& secs);

   void initialize_timeofday_clock();
   void initialize_timeofday_clock(timeval* starttime_tv_ptr);
   double elapsed_timeofday_time();
   double elapsed_timeofday_time(timeval const *starttime_tv_ptr);

   void get_elapsed_time(
      double& elapsed_secs,double& elapsed_mins,double& elapsed_hours);
   std::string getcurrdate();
   std::string getcurrUTC();
//   int utc_localtime_diff_in_hours();
   time_t utc_mktime(tm& utctm,int hourshift);
   int secs_since_Y2K();

// ==========================================================================
// Inlined methods
// ==========================================================================

// Method getcurrdate() returns the current localtime in ascii form as
// a C++ string.  This routine returns the equivalent of the Unix
// "date" system call:

   inline std::string getcurrdate()
      {
         std::string return_str("\n");

         time_t curr_truetime=time(NULL);
         std::string currtime_str=asctime(localtime(&curr_truetime));

// Remove return character which is attached to end of currtime_str:

         int return_posn=currtime_str.find_first_of(return_str,0);
         std::string currtime_substr=currtime_str.substr(0,return_posn);
         return currtime_substr;
      }
}

#endif  // timefuncs.h


