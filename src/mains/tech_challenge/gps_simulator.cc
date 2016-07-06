// ========================================================================
// Program GPS_SIMULATOR

//				gps_simulator

// ========================================================================
// Last updated on 5/27/10; 5/29/10; 6/17/10
// ========================================================================

#include <iostream>
#include "math/constant_vectors.h"
#include "astro_geo/GPS_datastream.h"
#include "messenger/Messenger.h"
#include "general/sysfuncs.h"
#include "track/tracks_group.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::ifstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Instantiate GPS messenger:

   string broker_URL="tcp://127.0.0.1:61616";
   cout << "ActiveMQ broker_URL = " << broker_URL << endl;

   string GPS_message_queue_channel_name="viewer_update";
   Messenger GPS_messenger( broker_URL, GPS_message_queue_channel_name );

   tracks_group gps_tracks_group;
   int GPS_ID=1;
   track* gps_trackone_ptr=gps_tracks_group.generate_new_track(GPS_ID);
   gps_trackone_ptr->set_description("Ground rover GPS #1");

   track* gps_tracktwo_ptr=gps_tracks_group.generate_new_track(GPS_ID+1);
   gps_tracktwo_ptr->set_description("Ground rover GPS #2");

   string serialPort="/dev/ttyUSB0";
   GPS_datastream GPS_datastream(serialPort);

   Clock clock;
   clock.current_local_time_and_UTC();

   double prev_time=0;
   double start_time=clock.secs_elapsed_since_reference_date();
   double delta_time=300;	// secs
   geopoint start_geoposn(-71.2872,42.4584);
   geopoint stop_geoposn(-71.2728,42.4764);
   threevector velocity=(stop_geoposn.get_UTM_posn()-
                         start_geoposn.get_UTM_posn())/delta_time;

   geopoint start_two_geoposn(-71.2944,42.4728);
   geopoint stop_two_geoposn(-71.2656,42.462);
   threevector velocity_two=(stop_two_geoposn.get_UTM_posn()-
                         start_two_geoposn.get_UTM_posn())/delta_time;

   colorfunc::Color trackone_color=colorfunc::red;
   colorfunc::Color tracktwo_color=colorfunc::cyan;

   cout.precision(12);
   while(true)
   {
//      GPS_datastream.read_curr_data();

//      double gps_time=GPS_datastream.get_clock().
//         secs_elapsed_since_reference_date();
//      cout << "gps_time = " << gps_time 
//           << " prev_time = " << prev_time << endl;

//      if (abs(gps_time-prev_time) < 0.5) continue;
//      prev_time=gps_time;

//      cout << "Current time = " 
//           << GPS_datastream.get_clock().YYYY_MM_DD_H_M_S() << endl;
//      cout << "Fix quality = " << GPS_datastream.get_fix_quality()
//           << " n_satellites = " << GPS_datastream.get_n_satellites()
//           << " horiz_dilution = " << GPS_datastream.get_horiz_dilution()
//           << endl;
//      cout << "Current geoposition = " << GPS_datastream.get_geoposn() 
//           << endl;
//      cout << endl;

      clock.current_local_time_and_UTC();
      double curr_time=clock.secs_elapsed_since_reference_date();

      if (abs(curr_time-prev_time) < 0.5) continue;

      cout.precision(12);
      cout << "curr_time = " << curr_time
           << " prev_time = " << prev_time
           << " dt = " << curr_time-prev_time << endl;
      
      prev_time=curr_time;

      double dt=curr_time-start_time;
      threevector posn=start_geoposn.get_UTM_posn()+velocity*dt;
      threevector posn_two=start_two_geoposn.get_UTM_posn()+velocity_two*dt;

      bool northern_hemisphere_flag=true;
      int UTM_zone=19;
      geopoint curr_geopoint(
         northern_hemisphere_flag,UTM_zone,posn.get(0),posn.get(1));
      threevector curr_posn(curr_geopoint.get_longitude(),
                            curr_geopoint.get_latitude(),
                            curr_geopoint.get_altitude());

      geopoint curr_geopoint_two(
         northern_hemisphere_flag,UTM_zone,posn_two.get(0),posn_two.get(1));
      threevector curr_posn_two(curr_geopoint_two.get_longitude(),
                                curr_geopoint_two.get_latitude(),
                                curr_geopoint_two.get_altitude());

      threevector velocity=Zero_vector;
      gps_trackone_ptr->set_posn_velocity(curr_time,curr_posn,velocity);
      gps_tracktwo_ptr->set_posn_velocity(curr_time,curr_posn_two,velocity);

      gps_trackone_ptr->broadcast_statevector(curr_time,&GPS_messenger,
         trackone_color);
      gps_tracktwo_ptr->broadcast_statevector(curr_time,&GPS_messenger,
         tracktwo_color);

   } // loop over iter labeling number of GPS device reads
}

