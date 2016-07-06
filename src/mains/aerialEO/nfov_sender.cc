// ========================================================================
// NFOV_SENDER simulates the narrow field-of-view camera system
// broadcasting its instantaneous ground location via ActiveMQ
// messages once per second.

//				nfov_sender

// ========================================================================
// Last updated on 5/26/09; 6/27/09; 8/26/09; 9/18/09; 1/23/14
// ========================================================================

#include <iostream>
#include <set>
#include <string>
#include <unistd.h>
#include <vector>
#include "math/basic_math.h"
#include "astro_geo/geopoint.h"
#include "messenger/Messenger.h"
#include "numrec/nrfuncs.h"
#include "general/stringfuncs.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::pair;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   cout << "=====================================================" << endl;
   cout << "Starting NFOV path sender:" << endl;
   cout << "=====================================================" << endl;


// Set the URI to point to the IPAddress of your broker.  add any
// optional params to the url to enable things like tightMarshalling
// or tcp logging etc.

   string broker_URL = "tcp://127.0.0.1:61616";
   if ( argc > 1 ) broker_URL = "tcp://" + string(argv[1]);

   string message_queue_channel_name="current_NFOV_lookpoint";
   string flightpath_sender_ID="NFOV_SENDER";
   bool include_sender_and_timestamp_info_flag=false;
   Messenger m(broker_URL,message_queue_channel_name,flightpath_sender_ID,
               include_sender_and_timestamp_info_flag);

   nrfunc::init_time_based_seed();

//      double time_interval_between_messages=0.01;	// secs
//      double time_interval_between_messages=0.02;	// secs
//      double time_interval_between_messages=0.05;	// secs
//      double time_interval_between_messages=0.2;	// secs
//     double time_interval_between_messages=0.5;	// secs
   double time_interval_between_messages=1.0;	// secs
//      double time_interval_between_messages=2.0;	// secs
   usleep(time_interval_between_messages*1000);

//      double time_interval_between_messages=1;	// secs
//      double time_interval_between_messages=2;	// secs
//      double time_interval_between_messages=5;	// secs
   sleep(time_interval_between_messages);

// -----------------------------------------------------------------      
// In Aug 2009, we received an Excel file from Bob Hatch containing
// "typical" MASSIVs parameters.  The following values are
// conservative bounds based upon the values in this excel file:

//   double racetrack_altitude=5800;   	// meters 
   double racetrack_radius=3500;      	// meters
   double aircraft_speed=56.6;		// meters/sec
   double period=2*PI*racetrack_radius/aircraft_speed; 	// 388.5 secs=6.5 mins
   double omega=2*PI/period;

// Hardwire geocoords for racetrack orbit's center and radius:

// Center of Milwaukee racetrack orbit:
// Longitude = W88
// Latitude = N43
// UTM zone = 16

//   double racetrack_center_longitude=-87.9;  // degs
//   double racetrack_center_latitude=43;    // degs
//   int UTM_zone=16;	

// Center of Lowell racktrack orbit:

// Longitude = W71.33
// Latitude = N42.64
// UTM zone = 19

   double racetrack_center_longitude=-71.30;  // degs
   double racetrack_center_latitude=42.63;    // degs
   int UTM_zone=19;		

   geopoint racetrack_center(
      racetrack_center_longitude,racetrack_center_latitude,UTM_zone);
   
//   double NFOV_radius=1000; 	   // meters
   double NFOV_radius=1; 	   // meters

// -----------------------------------------------------------------
   string command,key,value;
   typedef pair<string,string> property;
   vector<property> properties;
   vector<TextMessage*> TextMessage_ptrs;
   
// -----------------------------------------------------------------
// Next transmit sequence of UPDATE_NFOV_LOOKPOINT messages 

   command="UPDATE_NFOV_LOOKPOINT";
 
   double t0=0;
   double delta_t=1;	// sec
   for (int i=0; i<10000; i++)
   {
      double t=t0+i*delta_t;
      double theta=omega*t;
      double NFOV_lookpoint_x=
         racetrack_center.get_UTM_easting()+NFOV_radius*cos(theta);
      double NFOV_lookpoint_y=
         racetrack_center.get_UTM_northing()+NFOV_radius*sin(theta);

      properties.clear();
      int UAV_ID=0;
      key="UAV_ID";
      value=stringfunc::number_to_string(UAV_ID);
      properties.push_back(property(key,value));

      key="X Y";
//      value=stringfunc::number_to_string(curr_UAV_posn.get_longitude())+" "
//         +stringfunc::number_to_string(curr_UAV_posn.get_latitude())+" "
//         +stringfunc::number_to_string(curr_UAV_posn.get_altitude());
      value=stringfunc::number_to_string(NFOV_lookpoint_x)+" "
         +stringfunc::number_to_string(NFOV_lookpoint_y);
      properties.push_back(property(key,value));

      bool print_msg_flag=true;
      m.sendTextMessage(command,properties,print_msg_flag);

      sleep(1);
   } // loop over index i labeling time step

}
