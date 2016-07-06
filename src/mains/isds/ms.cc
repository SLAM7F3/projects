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
// Last updated on 6/27/09; 12/4/10; 4/19/11
// ========================================================================

#include <iostream>
#include <set>
#include <string>
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
//   string broker_URL = "tcp://155.34.162.244:61616";
   if ( argc > 1 ) broker_URL = "tcp://" + string(argv[1]);

   string message_queue_channel_name="thinclient_viewer";
   string message_sender_ID="MESSAGE_SENDER";
//   string message_sender_ID="MESSAGE_SENDER_"+timefunc::getcurrdate();
   Messenger m(broker_URL,message_queue_channel_name,message_sender_ID);

// -----------------------------------------------------------------

   string command,key,value;
   typedef pair<string,string> property;
   vector<property> properties;

   while(true)
   {
      command="UPDATE_CURRENT_NODE";

      properties.clear();
   
      key="hierarchy_ID";
      value="1";		
      properties.push_back(property(key,value));

      key="graph_level";
      value="0";		
      properties.push_back(property(key,value));

      key="node_ID";

      int node_ID;
      cout << "Enter node ID:" << endl;
      cin >> node_ID;
      value=stringfunc::number_to_string(node_ID);
      properties.push_back(property(key,value));

      bool print_msg_flag=true;
      m.sendTextMessage(command,properties,print_msg_flag);

/*
// Experiment with switching topic names dynamically:

      if (node_ID==-1)
      {
         string topic_name;
         cout << "Enter new topic name:" << endl;
         cin >> topic_name;
         m.changeTopic(topic_name);
      }
*/
    
   }
   
}
