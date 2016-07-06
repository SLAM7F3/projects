// ========================================================================
// Simulation program for Alex' ground-air communication workstation
// receiving byte messages from Peter's analyst workstation.

// 			      commfromanalysis

// ========================================================================
// Last updated on 10/15/09; 11/25/09
// ========================================================================

#include <iostream>
#include <set>
#include <string>
#include <vector>
#include "Qt/rtps/MessageWrapper.h"
#include "Qt/rtps/RTPSMessenger.h"

#include "general/outputfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::pair;
using std::string;
using std::vector;

/*

typedef pair<string,string> property;


void createROICommand(string* msg_ptr) 
{
   cout << "inside Alex' createROICommand()" << endl;
   cout << "msg_ptr = " << msg_ptr << endl;

   sROICommand cmd;
   sMessageWrapper sMessageWrapper;

   cmd.regionID = 2;
   cmd.enable = true;
   cmd.utmCoord.Easting  = 218062;
   cmd.utmCoord.Northing = 3713021;
   strcpy(cmd.utmCoord.Zone, "14s");
   cmd.trackID = 13;
   cmd.trackUpdatePriority = 3;
   cmd.imageUpdatePriority = 1;
   cmd.imagePeriod = 0.5;
   cmd.jpegQuality = 90;

   // Stuff cmd into sMessageWrapper struct

   sMessageWrapper << cmd;


   // Stuff sMessageWrapper into a stringstream

   stringstream buffer;
   buffer.clear();
   buffer.str("");
   buffer << sMessageWrapper;

   // Dump stringstream to string
   *msg_ptr = buffer.str();

   cout << "*msg_ptr = " << *msg_ptr << endl;
   cout << "at end of Alex' createROICommand()" << endl;
}

void createNFOVCommand(string* msg) 
{
   sNFOVCommand cmd;
   sMessageWrapper sMessageWrapper;

   cmd.currentNFOVCommandID = 0;
   cmd.enable = true;
   cmd.utmCoord.Easting  = 218062;
   cmd.utmCoord.Northing = 3713021;
   strcpy(cmd.utmCoord.Zone, "14s");
   cmd.imageUpdatePriority = 2;
   cmd.imagePeriod = 1;
   cmd.jpegQuality = 70;

   // Stuff cmd into sMessageWrapper struct
   sMessageWrapper << cmd;

   // Stuff sMessageWrapper into a stringstream
   stringstream buffer;
   buffer.clear();
   buffer.str("");
   buffer << sMessageWrapper;

   // Dump stringstream to string
   (*msg) = buffer.str();
}
*/

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   cout << "=====================================================" << endl;
   cout << "Starting ALEX' COMM machine message receiver:" << endl;
   cout << "=====================================================" << endl;

// Set the URI to point to the IPAddress of your broker:

   string broker_URL = "tcp://127.0.0.1:61616";
   if ( argc > 1 ) broker_URL = "tcp://" + string(argv[1]);

   string message_queue_channel_name = "ANALYSIS_TO_COMMUNICATION";  
   string message_sender_ID="ALEX_COMM_MACHINE";
   bool include_sender_and_timestamp_info_flag=false;
   RTPSMessenger m(broker_URL,message_queue_channel_name,
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
               string roiMessage;
               m.reconstruct_ROI_message(roiMessage);
            }
         } // loop over index i labeling received messages

         cout << "n_received_messages from messenger = " 
              << m.get_n_received_messages() << endl;
      } // m.copy_messages_and_purge_mailbox conditional
      
   } // infinite while loop
}

