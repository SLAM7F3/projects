// ==========================================================================
// Program CALIB_FLIRTIME performs a polynomial fit in order to relate FLIR
// video frame numbers to Boston-area time stamps recorded by Ross
// Anderson's program which ran on the Twin Otter's MacMini.  
// ==========================================================================
// Last updated on 7/14/11
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "astro_geo/Clock.h"
#include "math/mypolynomial.h"
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

   Clock clock;

   vector<double> flir_framenumber;
   vector<double> utc_time;

   int year=2011;
   int month=5;
   int day=14;
//  int UTC_hour=15+4;	// Clip #5
   int UTC_hour=16+4;	// Clip #6
   int minute;
   double secs,elapsed_secs;

/*
// Clip #5 matches

   flir_framenumber.push_back(181);
   minute=15;
   secs=0.5;
   clock.set_UTC_time(year,month,day,UTC_hour,minute,secs);
   elapsed_secs=clock.secs_elapsed_since_reference_date();
   utc_time.push_back(elapsed_secs);

   flir_framenumber.push_back(200);
   minute=15;
   secs=22.5;
   clock.set_UTC_time(year,month,day,UTC_hour,minute,secs);
   elapsed_secs=clock.secs_elapsed_since_reference_date();
   utc_time.push_back(elapsed_secs);
   
   flir_framenumber.push_back(260);
   minute=16;
   secs=29.5;
   clock.set_UTC_time(year,month,day,UTC_hour,minute,secs);
   elapsed_secs=clock.secs_elapsed_since_reference_date();
   utc_time.push_back(elapsed_secs);
   
   flir_framenumber.push_back(320);
   minute=17;
   secs=36.5;
   clock.set_UTC_time(year,month,day,UTC_hour,minute,secs);
   elapsed_secs=clock.secs_elapsed_since_reference_date();
   utc_time.push_back(elapsed_secs);

   flir_framenumber.push_back(506);
   minute=21;
   secs=5.565;
   clock.set_UTC_time(year,month,day,UTC_hour,minute,secs);
   elapsed_secs=clock.secs_elapsed_since_reference_date();
   utc_time.push_back(elapsed_secs);

   flir_framenumber.push_back(1004);
   minute=30;
   secs=24.592;
   clock.set_UTC_time(year,month,day,UTC_hour,minute,secs);
   elapsed_secs=clock.secs_elapsed_since_reference_date();
   utc_time.push_back(elapsed_secs);

   flir_framenumber.push_back(1500);
   minute=39;
   secs=51.621;
   clock.set_UTC_time(year,month,day,UTC_hour,minute,secs);
   elapsed_secs=clock.secs_elapsed_since_reference_date();
   utc_time.push_back(elapsed_secs);

   flir_framenumber.push_back(1998);
   minute=49;
   secs=33.65;
   clock.set_UTC_time(year,month,day,UTC_hour,minute,secs);
   elapsed_secs=clock.secs_elapsed_since_reference_date();
   utc_time.push_back(elapsed_secs);
*/

// Clip #6 matches

   flir_framenumber.push_back(923);
   minute=14;
   secs=40.7;
   clock.set_UTC_time(year,month,day,UTC_hour,minute,secs);
   elapsed_secs=clock.secs_elapsed_since_reference_date();
   utc_time.push_back(elapsed_secs);

   flir_framenumber.push_back(912);
   minute=14;
   secs=35.725;
   clock.set_UTC_time(year,month,day,UTC_hour,minute,secs);
   elapsed_secs=clock.secs_elapsed_since_reference_date();
   utc_time.push_back(elapsed_secs);

   flir_framenumber.push_back(649);
   minute=12;
   secs=53.720;
   clock.set_UTC_time(year,month,day,UTC_hour,minute,secs);
   elapsed_secs=clock.secs_elapsed_since_reference_date();
   utc_time.push_back(elapsed_secs);

   flir_framenumber.push_back(543);
   minute=12;
   secs=12.718;
   clock.set_UTC_time(year,month,day,UTC_hour,minute,secs);
   elapsed_secs=clock.secs_elapsed_since_reference_date();
   utc_time.push_back(elapsed_secs);

   flir_framenumber.push_back(140);
   minute=9;
   secs=36.710;
   clock.set_UTC_time(year,month,day,UTC_hour,minute,secs);
   elapsed_secs=clock.secs_elapsed_since_reference_date();
   utc_time.push_back(elapsed_secs);

   flir_framenumber.push_back(1);
   minute=8;
   secs=44.707;
   clock.set_UTC_time(year,month,day,UTC_hour,minute,secs);
   elapsed_secs=clock.secs_elapsed_since_reference_date();
   utc_time.push_back(elapsed_secs);


   cout.precision(13);
   for (unsigned int i=0; i<flir_framenumber.size(); i++)
   {
      cout << "i = " << i
           << " framenumber = " << flir_framenumber[i]
           << " elapsed secs = " << utc_time[i] << endl;
   }

//   mypolynomial poly(1);
//   mypolynomial poly(2);
   mypolynomial poly(3);
   double chisq;
//   bool fit_flag=
      poly.fit_coeffs(flir_framenumber,utc_time,chisq);
   
   cout << "poly = " << poly << endl;
   cout << "chisq = " << chisq << endl;

//   int frame_start=181;	// GC clip #5
//   int frame_stop=1999; // GC clip #5

   int frame_start=1;	// GC clip #6
   int frame_stop=1478; // GC clip #6

   vector<int> frame_numbers;
   vector<double> elapsed_seconds;
   for (int frame=frame_start; frame<= frame_stop; frame++)
   {
      double curr_elapsed_secs = poly.value(frame);
      clock.convert_elapsed_secs_to_date(curr_elapsed_secs);
      frame_numbers.push_back(frame);
      elapsed_seconds.push_back(curr_elapsed_secs);

      string date_str=clock.YYYY_MM_DD_H_M_S();
//      cout << frame << "   "
//           << curr_elapsed_secs << "   "
//           << date_str << endl;
   }

   cout << "elapsed_seconds.size() = " << elapsed_seconds.size() << endl;

/*
// Given input UTC time, find corresponding frame number:

   while(true)
   {
      int local_hour;
      cout << "Enter local Boston hour:" << endl;
      cin >> local_hour;
      UTC_hour=local_hour+4;
      cout << "Enter minute:" << endl;
      cin >> minute;
      cout << "Enter secs:" << endl;
      cin >> secs;
      clock.set_UTC_time(year,month,day,UTC_hour,minute,secs);
      double elapsed_secs=clock.secs_elapsed_since_reference_date();
      
      double secs_frac=(elapsed_secs-elapsed_seconds.front())/
         (elapsed_seconds.back()-elapsed_seconds.front());
      cout << "secs_frac = " << secs_frac << endl;
      
      int index=secs_frac*(elapsed_seconds.size()-1);
      int matching_framenumber=frame_numbers[index];
      cout << "Corresponding framenumber = " << matching_framenumber
           << endl;
   }
*/
 
// Given input frame number, find corresponding UTC time:

   int curr_framenumber;
   while(true)
   {
      cout << "Enter frame number:" << endl;
      cin >> curr_framenumber;
      double curr_time = poly.value(curr_framenumber);
      cout << "curr elapsed secs = " << curr_time << endl;
      clock.convert_elapsed_secs_to_date(curr_time);
      cout << "corresponding date = "
           << clock.YYYY_MM_DD_H_M_S() << endl;
   }
   

} 

