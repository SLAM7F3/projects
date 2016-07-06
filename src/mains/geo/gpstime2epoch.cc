// ==========================================================================
// Program GPSTIME2EPOCH takes in the number of seconds elapsed since
// the reference epoch.  It returns the corresponding GPS week and GPS
// seconds.
// ==========================================================================
// Last updated on 9/9/11 
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
      int GPS_week_number;
      double secs_into_GPS_week;
      cout << "Enter GPS week number:" << endl;
      cin >> GPS_week_number;
      cout << "Enter seconds into GPS week:" << endl;
      cin >> secs_into_GPS_week;

      double elapsed_secs=clock.GPS_time_to_elapsed_secs(
         GPS_week_number,secs_into_GPS_week);
      clock.convert_elapsed_secs_to_date(elapsed_secs);

      int GPS_week_number2;
      double secs_into_GPS_week2;
      clock.elapsed_secs_to_GPS_time(
         elapsed_secs,GPS_week_number2,secs_into_GPS_week2);

      cout << "Secs since reference epoch = " << elapsed_secs << endl;
      cout << "Recomputed GPS_week = " << GPS_week_number2
           << " Recomputed GPS secs = " << secs_into_GPS_week2 << endl;
      cout << "YYYY-MM-DD-H-M-S = "
           << clock.YYYY_MM_DD_H_M_S() << endl;
      cout << endl;


   }   

}
