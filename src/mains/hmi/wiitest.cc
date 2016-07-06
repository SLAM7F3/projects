// ========================================================================
// Program WIITEST
// ========================================================================
// Last updated on 3/31/11; 4/1/11
// ========================================================================

#include <iostream>
#include "time/timefuncs.h"
#include "ins/wiimote.h"

using std::cin;
using std::cout;
using std::endl;

int main(void)
{
   wiimote WM;
   WM.initialize_wiimote();

//   timefunc::initialize_timeofday_clock();

//   double t_previous_button_click=NEGATIVEINFINITY;
   while (true)
   {
      WM.update_state();
      int button_value=WM.get_curr_button_value();
      if (button_value > 0) cout << "button value = " << button_value << endl;
      
      WM.print_state();

/*

      double t=timefunc::elapsed_timeofday_time();

// Ignore any Wii input made less than delta_t since previous input:

      double dt=t-t_previous_button_click;
      const double min_delta_t=3;	// secs
      if (dt < min_delta_t) continue;



      int curr_button_value=WM.get_button_value();
      if (curr_button_value > 0)
      {
         cout << "button value = " << curr_button_value << endl;
         t_previous_button_click=t;
      }
*/

/*      
      continue;

      WM.print_state();

      int n_IR_sources=WM.get_n_IR_sources();
      if (n_IR_sources < 1) continue;
      
      for (int i=0; i<n_IR_sources; i++)
      {
         bool source_detected_flag=WM.get_IR_source_detected_flag(i);
         if (!source_detected_flag) continue;
         twovector curr_posn=WM.get_IR_source_posn(i);
         int curr_size=WM.get_IR_source_size(i);
         cout << "i = " << i
              << " X = " << curr_posn.get(0)
              << " Y = " << curr_posn.get(1)
              << " size = " << curr_size << endl;
      }

*/


/*
      if (timefunc::elapsed_timeofday_time() < 0.5) continue;
      timefunc::initialize_timeofday_clock();
    
//      if (fabs(t-basic_math::mytruncate(t)) > 0.01) continue;

      cout << "t = " << t
           << " az = " << INS.get_az()*180/PI
           << " el = " << INS.get_el()*180/PI
           << " roll = " << INS.get_roll()*180/PI  << endl;
*/


   }
}
