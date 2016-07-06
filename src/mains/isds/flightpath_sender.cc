// ========================================================================
// FLIGHTPATH_SENDER program meant to emulate ISAT system inputs and
// outputs to UAV path planner algorithms.

// ActiveMQ running on ISD3D laptop:

// 			flightpath_sender		// uses 127.0.0.1:61616
// 			flightpath_sender 155.34.162.148:61616
// 			message_receiver 155.34.162.148:61616

// ActiveMQ running on touchy:

// 			flightpath_sender 155.34.162.230:61616
// 			message_receiver 155.34.162.230:61616

// 			flightpath_sender 155.34.125.216:61616	  family day
// 			message_receiver 155.34.125.216:61616	  family day

// 			flightpath_sender 155.34.135.239:61616	  G104 conf rm

// ========================================================================
// Last updated on 2/19/09; 6/27/09; 12/4/10; 1/24/13
// ========================================================================

#include <iostream>
#include <set>
#include <string>
#include <unistd.h>
#include <vector>
#include "math/basic_math.h"
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

//   string message_queue_channel_name="people";
//   string message_queue_channel_name="powerpoint";
//   string message_queue_channel_name="SAM";
//   string message_queue_channel_name="UAV";

//   string message_queue_channel_name="urban_network";  
   // M. Yee's social network tool communicates on urban_network channel
//   string message_queue_channel_name="robots";
   // M. Yee's & L. Bertucelli's multi-robot path planning algorithms
   // communicate on robots channel

   string message_queue_channel_name="GoogleEarth";

//   string message_queue_channel_name="wiki";
//   Messenger m( broker_URL, message_queue_channel_name);

   string flightpath_sender_ID="FLIGHTPATH_SENDER";
//   string flightpath_sender_ID="FLIGHTPATH_SENDER_"+timefunc::getcurrdate();
   Messenger m( broker_URL, message_queue_channel_name , flightpath_sender_ID);

//   sleep(2);

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

// Hardwire quasi-random geocoords for static ground targets:

   vector<double> longitudes,latitudes;
   
   longitudes.push_back(-70.75068611111111);
   latitudes.push_back(42.12296666666667);

   longitudes.push_back(-71.04068055555555);
   latitudes.push_back(42.460033333333335);

   longitudes.push_back(-71.28159722222222);
   latitudes.push_back(42.55441388888889);

   longitudes.push_back(-71.55374722222221);
   latitudes.push_back(42.342061111111114);

/*
   longitudes.push_back(-70.3894);
   latitudes.push_back(41.6495);

   longitudes.push_back(-70.74029);
   latitudes.push_back(41.8163);

   longitudes.push_back(-70.8236);
   latitudes.push_back(42.0767);

   longitudes.push_back(-70.9488);
   latitudes.push_back(42.29889);

   longitudes.push_back(-71.14801);
   latitudes.push_back(42.304959);

   longitudes.push_back(-71.267779);
   latitudes.push_back(41.975739);

   longitudes.push_back(-71.272091);
   latitudes.push_back(41.838520);

   longitudes.push_back(-71.368358);
   latitudes.push_back(41.700739);
*/
   
//      int n_waypoints=6;
//      int n_waypoints=20;

   int n_waypoints=latitudes.size();
   if (latitudes.size() != longitudes.size())
   {
      cout << "Error in FLIGHTPATH_SENDER!" << endl;
      cout << "latitudes.size() = " << latitudes.size()
           << " longitudes.size() = " << longitudes.size() << endl;
      n_waypoints=basic_math::min(latitudes.size(),longitudes.size());
   }

   cout << "n_waypoints = " << n_waypoints << endl;

// -----------------------------------------------------------------      
// Hardwire quasi-random geocoords for starting UAV positions:

   vector<double> UAV_longitudes,UAV_latitudes;
   
   UAV_longitudes.push_back(-70.176);
   UAV_latitudes.push_back(41.684);

   int n_UAVs=UAV_latitudes.size();
   
// -----------------------------------------------------------------
// Set up ActiveMQ messages for both ground targets and UAVs.
// Messages consist of a single command string along with an STL
// vector of key-value string pair properties:

   string command,key,value;
   typedef pair<string,string> property;
   vector<property> properties;
   vector<TextMessage*> TextMessage_ptrs;

   int index=0;
   cout << "Enter label indicating type of message to be broadcast:" << endl;
   cout << "index = 0 --> INSTANTIATE_UAV message" << endl;
   cout << "index = 1 --> SET_INIT_WAYPOINTS message" << endl;
   cout << "index = 2 --> UPDATE_UAV_STATEVECTOR message" << endl;
   cout << "index = 3 --> UPDATE_WAYPOINTS message" << endl;
   cout << "index = 4 --> UPDATE_UAV_PATHS message" << endl;
   cout << "index = 5 --> PURGE_WAYPOINTS message" << endl;
   cout << "index = 6 --> PURGE_UAVS message" << endl;
   
   cin >> index;
   string input_command;
   if (index==0)
   {
      input_command="INSTANTIATE_UAV";
   }
   else if (index==1)
   {
      input_command="SET_INIT_WAYPOINTS";
   }
   else if (index==2)
   {
      input_command="UPDATE_UAV_STATEVECTOR";
   }
   else if (index==3)
   {
      input_command="UPDATE_WAYPOINTS";
   }
   else if (index==4)
   {
      input_command="UPDATE_UAV_PATHS";
   }
   else if (index==5)
   {
      input_command="PURGE_WAYPOINTS";
   }
   else if (index==6)
   {
      input_command="PURGE_UAVS";
   }
   cout << "input_command = " << input_command << endl;

   properties.clear();

// -----------------------------------------------------------------
// INSTANTIATE_UAV messages

   command="INSTANTIATE_UAV";
   if (input_command==command)
   {
      for (int l=0; l<n_UAVs; l++)
      {
         cout << "l = " << l << " of n_UAVs = " << n_UAVs << endl;

         properties.clear();
         key="LONGITUDE LATITUDE ALTITUDE ";
         value=stringfunc::number_to_string(UAV_longitudes[l])+" "+
            stringfunc::number_to_string(UAV_latitudes[l])+" 2500";
         properties.push_back(property(key,value));

         bool print_msg_flag=true;
         m.sendTextMessage(command,properties,print_msg_flag);

      } // loop over index l labeling UAVs
   } // input_command==command conditional

// -----------------------------------------------------------------
// SET_INIT_WAYPOINTS messages

   command="SET_INIT_WAYPOINTS";
   if (input_command==command)
   {
      key="UAV_ID";
      value="0";
      properties.push_back(property(key,value));

      for (int l=0; l<n_waypoints; l++)
      {
         cout << "l = " << l << " of n_waypoints = " << n_waypoints << endl;

         key="LONGITUDE LATITUDE ALTITUDE "+
            stringfunc::integer_to_string(l,3);
         value=stringfunc::number_to_string(longitudes[l])+" "+
            stringfunc::number_to_string(latitudes[l])+" 10";
         properties.push_back(property(key,value));
      } // loop over index l labeling longitude,latitude waypoint pairs
   
      bool print_msg_flag=true;
      m.sendTextMessage(command,properties,print_msg_flag);

   } // input_command==command conditional
 
// -----------------------------------------------------------------
// UPDATE_UAV_STATEVECTOR message

   command="UPDATE_UAV_STATEVECTOR";
   if (input_command==command)
   {
      int UAV_ID=0;
      key="UAV_ID";
      value=stringfunc::number_to_string(UAV_ID);
      properties.push_back(property(key,value));

      key="LONGITUDE LATITUDE ALTITUDE";
      value="-70.8 42.03 2500";
//      value="-71.2 42.1 2500";


      properties.push_back(property(key,value));

      bool print_msg_flag=true;
      m.sendTextMessage(command,properties,print_msg_flag);
   } // input_command==command conditional

// -----------------------------------------------------------------
// UPDATE_WAYPOINTS messages

   command="UPDATE_WAYPOINTS";
   if (input_command==command)
   {
      longitudes.push_back(-71.0);
      longitudes.push_back(-71.4);
      longitudes.push_back(-70.2);
      longitudes.push_back(-72);

      latitudes.push_back(42.6);
      latitudes.push_back(41.8);
      latitudes.push_back(42.2);
      latitudes.push_back(42);
    
      for (unsigned int l=0; l<longitudes.size(); l++)
      {
         key="LONGITUDE LATITUDE ALTITUDE "+
            stringfunc::integer_to_string(l,3);
         value=stringfunc::number_to_string(longitudes[l])+" "+
            stringfunc::number_to_string(latitudes[l])+" 10";
         properties.push_back(property(key,value));
      } // loop over index l labeling ROI longitude,latitude pairs

      bool print_msg_flag=true;
      m.sendTextMessage(command,properties,print_msg_flag);
   }
   
// -----------------------------------------------------------------
// UPDATE_UAV_PATHS message

   command="UPDATE_UAV_PATHS";

   if (input_command==command)
   {
      bool print_msg_flag=true;
      m.sendTextMessage(command,properties,print_msg_flag);
   } // input_command==command conditional
   
// -----------------------------------------------------------------
// PURGE_WAYPOINTS message

   command="PURGE_WAYPOINTS";

   if (input_command==command)
   {
      bool print_msg_flag=true;
      m.sendTextMessage(command,properties,print_msg_flag);
   } // input_command==command conditional
   
// -----------------------------------------------------------------
// PURGE_UAVS message

   command="PURGE_UAVS";

   if (input_command==command)
   {
      bool print_msg_flag=true;
      m.sendTextMessage(command,properties,print_msg_flag);
   } // input_command==command conditional

}
