// ==========================================================================
// Program TIMINGS converts takeoff/landing metadata extracted from
// the Puma metadata file into epoch seconds.
// ==========================================================================
// Last updated on 5/20/13
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "astro_geo/Clock.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::string;
using std::vector;

int main (int argc, char * argv[])
{
   std::set_new_handler(sysfunc::out_of_memory);
   cout.precision(16);

   Clock clock;
   int UTM_zone=19;	// Boston
   int UTM_time_offset=clock.compute_UTM_zone_time_offset(UTM_zone);
   cout << "UTM_time_offset = " << UTM_time_offset << endl;

   int year=2013;
   int month=4;
//   int day=24;
   int day=25;

// Take off time:

   int local_hour=14;
   int minute=41;
   double sec=10;
   clock.set_local_time(year,month,day,local_hour,minute,sec);
   double takeoff_time=
      clock.secs_elapsed_since_reference_date();

   cout << "takeoff_time = " << takeoff_time << endl;
   cout << "takeoff time in UTC = " 
        << clock.YYYY_MM_DD_H_M_S() << endl;

// Landing time:

   local_hour=15;
   minute=54;
   sec=40;
   clock.set_local_time(year,month,day,local_hour,minute,sec);
   double landing_time=
      clock.secs_elapsed_since_reference_date();

   cout << "landing_time = " << landing_time << endl;
   cout << "landing time in UTC = " 
        << clock.YYYY_MM_DD_H_M_S() << endl;
} 

