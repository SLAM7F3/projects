// ==========================================================================
// Program EPOCH2GPSTIME takes in a GPS week and GPS seconds.  It
// returns the corresponding number of seconds elapsed since
// the reference epoch.  
// ==========================================================================
// Last updated on 9/8/11; 9/9/11 
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "astro_geo/Clock.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   Clock clock;

   cout.precision(12);
   while(true)
   {
      double elapsed_secs;
      cout << "Enter secs since epoch" << endl;
      cin >> elapsed_secs;

      int GPS_week_number;
      double secs_into_GPS_week;
      clock.elapsed_secs_to_GPS_time(
         elapsed_secs,GPS_week_number,secs_into_GPS_week);
      clock.convert_elapsed_secs_to_date(elapsed_secs);

      cout << "GPS_week = " << GPS_week_number
           << " GPS secs = " << secs_into_GPS_week << endl;
      cout << "recomputed epoch secs = "
           << clock.GPS_time_to_elapsed_secs(
              GPS_week_number,secs_into_GPS_week) << endl;
      cout << "YYYY-MM-DD-H-M-S = "
           << clock.YYYY_MM_DD_H_M_S() << endl;
      cout << endl;


   }   

}
