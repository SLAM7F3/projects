// ========================================================================
// Dave Ceddia's MESSAGE_SENDER program

// ActiveMQ running on ISD3D laptop:

// 			message_sender		// uses 127.0.0.1:61616
//			message_receiver

// 			message_sender 155.34.162.148:61616
// 			message_receiver 155.34.162.148:61616

// ActiveMQ running on touchy:

// 			message_sender 155.34.162.230:61616
// 			message_receiver 155.34.162.230:61616

// 			message_sender 155.34.125.216:61616	  family day
// 			message_receiver 155.34.125.216:61616	  family day

// 			message_sender 155.34.135.239:61616	  G104 conf rm

// ========================================================================
// Last updated on 6/27/09; 12/4/10; 4/19/11; 1/24/14
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
   cout << "Starting message sender:" << endl;
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
//   string message_queue_channel_name="photo_network";
//   string message_queue_channel_name="cars";
//   string message_queue_channel_name="viewer_update";
//   string message_queue_channel_name="thinclient_viewer";
   string message_queue_channel_name="127.0.0.1";
   // M. Yee's social network tool communicates on urban_network channel
//   string message_queue_channel_name="robots";
   // M. Yee's & L. Bertucelli's multi-robot path planning algorithms
   // communicate on robots channel

//   string message_queue_channel_name="GoogleEarth";

// Michael Yee's Process launcher which can interpret a general URL
// command:

//   string message_queue_channel_name="wiki";

   string message_sender_ID="MESSAGE_SENDER";
//   string message_sender_ID="MESSAGE_SENDER_"+timefunc::getcurrdate();
   bool include_sender_and_timestamp_info_flag=false;
   Messenger m(broker_URL,message_queue_channel_name,message_sender_ID,
               include_sender_and_timestamp_info_flag);

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

/*
   longitudes.push_back(44.29);
   longitudes.push_back(44.34);
   longitudes.push_back(44.51);
   longitudes.push_back(44.39);
   longitudes.push_back(44.31);
   longitudes.push_back(44.47);
   longitudes.push_back(44.42);
   longitudes.push_back(44.44);
   longitudes.push_back(44.48);
   longitudes.push_back(44.35);

   longitudes.push_back(44.36);
   longitudes.push_back(44.49);
   longitudes.push_back(44.29);
   longitudes.push_back(44.39);
   longitudes.push_back(44.34);
   longitudes.push_back(44.41);
   longitudes.push_back(44.43);
   longitudes.push_back(44.47);
   longitudes.push_back(44.49);
   longitudes.push_back(44.42);

      longitudes.push_back(44.43);
      longitudes.push_back(44.42);
      longitudes.push_back(44.32);
      longitudes.push_back(44.42);
      longitudes.push_back(44.47);
      longitudes.push_back(44.42);
      longitudes.push_back(44.37);
      longitudes.push_back(44.44);
      longitudes.push_back(44.31);
      longitudes.push_back(44.31);

      longitudes.push_back(44.34);
      longitudes.push_back(44.28);
      longitudes.push_back(44.39);
      longitudes.push_back(44.32);
      longitudes.push_back(44.38);
      longitudes.push_back(44.40);
      longitudes.push_back(44.35);
      longitudes.push_back(44.30);
      longitudes.push_back(44.42);
      longitudes.push_back(44.36);

*/


/*
      longitudes.push_back(44.44);
      longitudes.push_back(44.46);
      longitudes.push_back(44.39);
      longitudes.push_back(44.31);
      longitudes.push_back(44.41);
      longitudes.push_back(44.37);
      longitudes.push_back(44.43);
      longitudes.push_back(44.34);
      longitudes.push_back(44.45);
      longitudes.push_back(44.33);
*/

// -----------------------------------------------------------------


/*
   latitudes.push_back(33.27);
   latitudes.push_back(33.335);
   latitudes.push_back(33.28);
   latitudes.push_back(33.21);
   latitudes.push_back(33.343);
   latitudes.push_back(33.26);
   latitudes.push_back(33.30);
   latitudes.push_back(33.38);
   latitudes.push_back(33.32);
   latitudes.push_back(33.40);

   latitudes.push_back(33.355);
   latitudes.push_back(33.37);
   latitudes.push_back(33.26);
   latitudes.push_back(33.34);
   latitudes.push_back(33.28);
   latitudes.push_back(33.37);
   latitudes.push_back(33.33);
   latitudes.push_back(33.23);
   latitudes.push_back(33.40);
   latitudes.push_back(33.32);

      latitudes.push_back(33.26);
      latitudes.push_back(33.33);
      latitudes.push_back(33.40);
      latitudes.push_back(33.23);
      latitudes.push_back(33.25);
      latitudes.push_back(33.33);
      latitudes.push_back(33.40);
      latitudes.push_back(33.25);
      latitudes.push_back(33.39);
      latitudes.push_back(33.35);

      latitudes.push_back(33.22);
      latitudes.push_back(33.30);
      latitudes.push_back(33.34);
      latitudes.push_back(33.25);
      latitudes.push_back(33.35);
      latitudes.push_back(33.27);
      latitudes.push_back(33.36);
      latitudes.push_back(33.28);
      latitudes.push_back(33.37);
      latitudes.push_back(33.30);
*/

/*
      latitudes.push_back(33.31);
      latitudes.push_back(33.39);
      latitudes.push_back(33.32);
      latitudes.push_back(33.29);
      latitudes.push_back(33.36);
      latitudes.push_back(33.38);
      latitudes.push_back(33.26);
      latitudes.push_back(33.33);
      latitudes.push_back(33.40);
      latitudes.push_back(33.34);
*/

      double min_longitude=44.25;
      double max_longitude=44.54;
      double min_latitude=33.20;
      double max_latitude=33.43;


      int n_targets=100;
//      int n_targets=200;
      cout << "Enter number of extra targets:" << endl;
      cin >> n_targets;
      for (int n=0; n<n_targets; n++)
      {
         double curr_longitude=min_longitude+nrfunc::ran1()*(
            max_longitude-min_longitude);
         double curr_latitude=min_latitude+nrfunc::ran1()*(
            max_latitude-min_latitude);
         longitudes.push_back(curr_longitude);
         latitudes.push_back(curr_latitude);
      }

//      int n_ground_targets=6;
//      int n_ground_targets=20;

   int n_ground_targets=latitudes.size();
   if (latitudes.size() != longitudes.size())
   {
      cout << "Error in MESSAGE_SENDER!" << endl;
      cout << "latitudes.size() = " << latitudes.size()
           << " longitudes.size() = " << longitudes.size() << endl;
      n_ground_targets=basic_math::min(latitudes.size(),longitudes.size());
   }

   cout << "n_ground_targets = " << n_ground_targets << endl;

// -----------------------------------------------------------------      
// Hardwire quasi-random geocoords for starting UAV positions:

   vector<double> UAV_longitudes,UAV_latitudes;
   
   UAV_longitudes.push_back(44.23);
   UAV_longitudes.push_back(44.39);
   UAV_longitudes.push_back(44.54);

   UAV_latitudes.push_back(33.34);
   UAV_latitudes.push_back(33.31);
   UAV_latitudes.push_back(33.27);

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
   cout << "index = 0 --> ADD_ROI_CENTER messages" << endl;
   cout << "index = 1 --> INSTANTIATE_UAV messages" << endl;
   cout << "index = 2 --> UPDATE_UAV_PATHS messages" << endl;
   cout << "index = 3 --> Process launcher messages" << endl;
   cout << "index = 4 --> SELECT VERTEX messages" << endl;
   
   cin >> index;
   string input_command;
   if (index==0)
   {
      input_command="ADD_ROI_CENTER";
   }
   else if (index==1)
   {
      input_command="INSTANTIATE_UAV";
   }
   else if (index==2)
   {
      input_command="UPDATE_UAV_PATHS";
   }
   else if (index==3)
   {
      input_command="LAUNCH_PROCESS";
   }
   else if (index==4)
   {
      input_command="SELECT_VERTEX";
   }

// -----------------------------------------------------------------
// ADD_ROI_CENTER messages

   for (int l=0; l<n_ground_targets; l++)
   {
      cout << "l = " << l << " of n_ground_targets = "
           << n_ground_targets << endl;

      key="LONGITUDE LATITUDE ALTITUDE ";
      value=stringfunc::number_to_string(longitudes[l])+" "+
         stringfunc::number_to_string(latitudes[l])+" 10";
      properties.push_back(property(key,value));

/*
  key="0";
//      key="TYPE";
value="6,3,5,0,2,4,1";
//      value="ROI";
//      value="VEHICLE";
properties.push_back(property(key,value));

key="ID";
int label_ID=-1;
//      cout << "Enter "+value+" label ID:" << endl;
//      cin >> label_ID;
//      value=stringfunc::number_to_string(label_ID);

int random_index=100*nrfunc::ran1();
value=stringfunc::number_to_string(random_index);
//      properties.push_back(property(key,value));

*/

/*
// Adding/selecting edge example:

command="ADD_EDGE";
command="SELECT_EDGE";

key="TYPE1";
value="ROI";
properties.push_back(property(key,value));

key="ID1";
value=stringfunc::number_to_string(random_index);
properties.push_back(property(key,value));

key="TYPE2";
value="VEHICLE";
properties.push_back(property(key,value));
property curr_property(key,value);

key="ID2";
value=stringfunc::number_to_string(random_index);
properties.push_back(property(key,value));
*/

// Adding/selecting vertex example:

      command="ADD_ROI_CENTER";
//      command="ASSIGN_TASK";
//      command="DELETE_VERTEX";
//      command="ADD_VERTEX";
//      command="SELECT_VERTEX";

      if (input_command==command)
      {
         bool print_msg_flag=true;
         m.sendTextMessage(command,properties,print_msg_flag);
      }
      
   } // loop over index l labeling Baghdad longitude,latitude pairs

// -----------------------------------------------------------------
// INSTANTIATE_UAV messages
    
   for (int l=0; l<n_UAVs; l++)
   {
      cout << "l = " << l << " of n_UAVs = " << n_UAVs << endl;

      properties.clear();

      key="LONGITUDE LATITUDE ALTITUDE ";
      value=stringfunc::number_to_string(UAV_longitudes[l])+" "+
         stringfunc::number_to_string(UAV_latitudes[l])+" 2500";
      properties.push_back(property(key,value));

      command="INSTANTIATE_UAV";

      if (input_command==command)
      {
         bool print_msg_flag=true;
         m.sendTextMessage(command,properties,print_msg_flag);
      }
   } // loop over index l labeling UAVs
   
// -----------------------------------------------------------------
// UPDATE_UAV_PATHS message

   command="UPDATE_UAV_PATHS";

   if (input_command==command)
   {
      properties.clear();
      bool print_msg_flag=true;
      m.sendTextMessage(command,properties,print_msg_flag);
   }
   
// -----------------------------------------------------------------
// UPDATE_UAV_PATHS message

   command="LAUNCH_PROCESS";

   if (input_command==command)
   {
      properties.clear();

     string url=
        "http://www.google.com/search?hl=en&btnI=I%27m+Feeling+Lucky&q=";
//     url += "Majestic+Apartments+New+York+City+wiki";
     url += "Empire+State+Buiding+New+York+City+wiki";
     cout << "url = " << url << endl;

      bool print_msg_flag=true;
      m.sendTextMessage(command,properties,print_msg_flag);
   }
   
// -----------------------------------------------------------------
// SELECT_VERTEX message

   command="SELECT_VERTEX";

   if (input_command==command)
   {
      properties.clear();
      
      key="TYPE";
      value="PHOTO";		
      properties.push_back(property(key,value));

      key="ID";
      int ID;
      cout << "Enter OBSFRUSTUM ID" << endl;
      cin >> ID;
      value=stringfunc::number_to_string(ID);
      properties.push_back(property(key,value));

      bool print_msg_flag=true;
      m.sendTextMessage(command,properties,print_msg_flag);
   }
}
