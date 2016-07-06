// ========================================================================
// Dave Ceddia's MESSAGE_RECEIVER program

// ActiveMQ running on ISD3D laptop:

// 			message_receiver 155.34.162.148:61616

// ========================================================================
// Last updated on 5/26/09; 6/17/09; 10/13/09
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "messenger/Messenger.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   cout << "=====================================================" << endl;
   cout << "Starting the message receiver:" << endl;
   cout << "=====================================================" << endl;

// Set the URI to point to the IPAddress of your broker.  add any
// optional params to the url to enable things like tightMarshalling
// or tcp logging etc.

   string broker_URL = "tcp://155.34.162.244:61616";
//   string broker_URL = "tcp://127.0.0.1:61617";
   if ( argc > 1 ) broker_URL = "tcp://" + string(argv[1]);

   string message_queue_channel_name="155.34.162.119";
   
   bool include_sender_and_timestamp_info_flag=false;
   Messenger m(broker_URL,message_queue_channel_name,
               include_sender_and_timestamp_info_flag);

   while (true) 
   {
      int n_mailbox_messages=m.get_n_messages_in_mailbox();
      if (n_mailbox_messages==0) continue;

//         cout << "n_mailbox_messages = " << n_mailbox_messages << endl;
      if (n_mailbox_messages > 1)
      {
         cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
         cout << "get_n_messages_in_mailbox() = " 
              << n_mailbox_messages << endl;
         cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
//            outputfunc::enter_continue_char();
      }

      if (m.copy_messages_and_purge_mailbox())
      {
         cout << "n_curr_messages() from messenger = " 
              << m.get_n_curr_messages() << endl;
         for (int i=0; i<m.get_n_curr_messages(); i++)
         {
            const message* curr_message_ptr=m.get_message_ptr(i);
            if (curr_message_ptr != NULL)
            {
               cout << "curr_message from messenger = " 
                    << *curr_message_ptr << endl;
            }
         } // loop over index i labeling received messages

         cout << "n_received_messages from messenger = " 
              << m.get_n_received_messages() << endl;
      } // m.copy_messages_and_purge_mailbox conditional
      
   } // infinite while loop
}
