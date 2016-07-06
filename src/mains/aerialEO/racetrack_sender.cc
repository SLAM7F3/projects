// ========================================================================
// RACETRACK_SENDER simulates Massivs system broadcasting its
// instantaneous position via ActiveMQ messages once per second.

//			       racetrack_sender

// ========================================================================
// Last updated on 5/25/09; 6/27/09; 8/26/09; 9/18/09; 1/23/14
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
   cout << "Starting flight path sender:" << endl;
   cout << "=====================================================" << endl;


// Set the URI to point to the IPAddress of your broker.  add any
// optional params to the url to enable things like tightMarshalling
// or tcp logging etc.

   string broker_URL = "tcp://127.0.0.1:61616";
   if ( argc > 1 ) broker_URL = "tcp://" + string(argv[1]);

   string message_queue_channel_name="aircraft";
   string flightpath_sender_ID="RACETRACK_SENDER";
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
// On 8/26/09, we spoke with Peter Boettcher in G99 to get some basic
// MASSIVs info.  He told us that MASSIVs has 12 imagers with 2500
// pixels across per square imager.  So the total number of pixels
// across either the U and V axes = 12 * 2500 = 30,000.  Each pixel is
// 2.2 microns in size on the focal plane array.  So the total size of
// the array in the U and V directions = 2.2 microns * 30,000 = 66
// mms.  The focal length of the imagers = 45 mm.  

// FOV_u,v = 2 * atan(33/45) = 1.265 radians = 72.5 degs.

// According to Boettcher, the desired coverage area on the ground is
// 10 km x 10 km.  As of summer 2009, MASSIVS generally does not tilt
// beyond nadir.  But there's no reason in principle it could not.


// In Aug 2009, we received an Excel file from Bob Hatch containing
// "typical" MASSIVs parameters.  The following values are
// conservative bounds based upon the values in this excel file:

   double racetrack_altitude=5800;   	// meters 
   double racetrack_radius=3000;      	// meters
   double aircraft_speed=56.6;		// meters/sec
   double period=2*PI*racetrack_radius/aircraft_speed; 	// 388.5 secs=6.5 mins
   double omega=2*PI/period;

// Hardwire geocoords for racetrack orbit's center and radius:

// Center of Milwaukee racetrack orbit:

// Longitude = W88
// Latitude = N43
// UTM zone = 16

//   double racetrack_center_longitude=-88;  // degs
//   double racetrack_center_latitude=43;    // degs
//   int UTM_zone=16;	

// Center of Lowell racktrack orbit:

// Longitude = W71.33
// Latitude = N42.64
// UTM zone = 19

   double racetrack_center_longitude=-71.33;  // degs
   double racetrack_center_latitude=42.64;    // degs
   int UTM_zone=19;		

   geopoint racetrack_center(
      racetrack_center_longitude,racetrack_center_latitude,UTM_zone);

// -----------------------------------------------------------------
   string command,key,value;
   typedef pair<string,string> property;
   vector<property> properties;
   vector<TextMessage*> TextMessage_ptrs;
   
// -----------------------------------------------------------------
// Transmit initial INSTANTIATE_UAV message:

   command="INSTANTIATE_UAV";

/*
   double initial_longitude=-87.87733;		// degs
   double initial_latitude=43.00101;		// degs
   double initial_altitude=5000;		// meters
   geopoint initial_posn(initial_longitude,initial_latitude,initial_altitude,
                         UTM_zone);
   threevector initial_UTM_posn=initial_posn.get_UTM_posn();
*/

   double t=0;
   double theta=omega*t;
   double UAV_x=
      racetrack_center.get_UTM_easting()+racetrack_radius*cos(theta);
   double UAV_y=
      racetrack_center.get_UTM_northing()+racetrack_radius*sin(theta);
   double UAV_z=racetrack_altitude;
   double UAV_Vx=-racetrack_radius*sin(theta)*omega;
   double UAV_Vy=racetrack_radius*cos(theta)*omega;
   double UAV_Vz=0;


   properties.clear();
//   key="LONGITUDE LATITUDE ALTITUDE ";
   key="X Y Z ";
   value=stringfunc::number_to_string(UAV_x)+" "+
      stringfunc::number_to_string(UAV_y)+" "+
      stringfunc::number_to_string(UAV_z);
//   value=stringfunc::number_to_string(initial_UTM_posn.get(0))+" "+
//      stringfunc::number_to_string(initial_UTM_posn.get(1))+" "+
//      stringfunc::number_to_string(initial_UTM_posn.get(2));
   properties.push_back(property(key,value));

//   double initial_Vx=0;
//   double initial_Vy=104.72;	// meters/sec
//   double initial_Vz=0;

   key="Vx Vy Vz ";
   value=stringfunc::number_to_string(UAV_Vx)+" "+
      stringfunc::number_to_string(UAV_Vy)+" "+
      stringfunc::number_to_string(UAV_Vz);
//   value=stringfunc::number_to_string(initial_Vx)+" "+
//      stringfunc::number_to_string(initial_Vy)+" "+
 //     stringfunc::number_to_string(initial_Vz);
   properties.push_back(property(key,value));
   
   bool print_msg_flag=true;
   m.sendTextMessage(command,properties,print_msg_flag);

// -----------------------------------------------------------------
// Next transmit sequence of UPDATE_UAV_STATEVECTOR messages with UAV
// positions corresponding to racetrack circle:

   command="UPDATE_UAV_STATEVECTOR";
 
   double t0=0;
   double delta_t=1;	// sec
   for (int i=0; i<10000; i++)
   {
      double t=t0+i*delta_t;
      double theta=omega*t;
      double UAV_x=
         racetrack_center.get_UTM_easting()+racetrack_radius*cos(theta);
      double UAV_y=
         racetrack_center.get_UTM_northing()+racetrack_radius*sin(theta);
      double UAV_z=racetrack_altitude;
      double UAV_Vx=-racetrack_radius*sin(theta)*omega;
      double UAV_Vy=racetrack_radius*cos(theta)*omega;
      double UAV_Vz=0;
      
//      geopoint curr_UAV_posn(racetrack_center.get_northern_hemisphere_flag(),
//                             racetrack_center.get_UTM_zonenumber(),
//                             UAV_x,UAV_y,racetrack_altitude);

      properties.clear();
      int UAV_ID=0;
      key="UAV_ID";
      value=stringfunc::number_to_string(UAV_ID);
      properties.push_back(property(key,value));

//      key="LONGITUDE LATITUDE ALTITUDE";
      key="X Y Z";
//      value=stringfunc::number_to_string(curr_UAV_posn.get_longitude())+" "
//         +stringfunc::number_to_string(curr_UAV_posn.get_latitude())+" "
//         +stringfunc::number_to_string(curr_UAV_posn.get_altitude());
      value=stringfunc::number_to_string(UAV_x)+" "
         +stringfunc::number_to_string(UAV_y)+" "
         +stringfunc::number_to_string(UAV_z);
      properties.push_back(property(key,value));

      key="Vx Vy Vz";
      value=stringfunc::number_to_string(UAV_Vx)+" "
         +stringfunc::number_to_string(UAV_Vy)+" "
         +stringfunc::number_to_string(UAV_Vz);
      properties.push_back(property(key,value));

      bool print_msg_flag=true;
      m.sendTextMessage(command,properties,print_msg_flag);

      sleep(1);
      
   } // loop over index i labeling time step

}
