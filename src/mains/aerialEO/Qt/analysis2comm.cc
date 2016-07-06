// ========================================================================
// Simulation program for sending byte ROI, NFOV etc messages to Alex'
// ground-air communication station from Analyst workstation for RTPS
// project.

// 			      analysis2comm

// ========================================================================
// Last updated on 11/25/09; 11/30/09; 12/21/09
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

typedef pair<string,string> property;

// Method generate_command() stuffs input message_wrapper into a
// stringstream.  It then dumps the stringstream to output string
// *msg_ptr:

void generate_command(sMessageWrapper& message_wrapper,string* msg_ptr)
{
   stringstream buffer;
   buffer.clear();
   buffer.str("");
   buffer << message_wrapper;

   *msg_ptr = buffer.str();
//   cout << "*msg_ptr = " << *msg_ptr << endl;
}

void createROICommand(string* msg_ptr) 
{
//   cout << "inside Alex' createROICommand()" << endl;

   sROICommand cmd;

   cmd.regionID = 2;
   cmd.enable = true;
   cmd.utmCoord.Easting  = 218062;
   cmd.utmCoord.Northing = 3713021;
   strcpy(cmd.utmCoord.Zone, "14 South");
   cmd.trackID = 13;
   cmd.trackUpdatePriority = 3;
   cmd.imageUpdatePriority = 1;
   cmd.imagePeriod = 0.5;
   cmd.jpegQuality = 90;
   cmd.numChips = 3;

// Stuff cmd into sMessageWrapper struct

   sMessageWrapper sMessageWrapper;
   sMessageWrapper << cmd;
   generate_command(sMessageWrapper,msg_ptr);
}

void createROIImageUpdate(string* msg_ptr) 
{
   sROIImageUpdate cmd;

   cmd.regionID=1;
   cmd.imageUpdatePriority=2;

   cmd.utmCorners.Northing[0]=123;
   cmd.utmCorners.Northing[1]=234;
   cmd.utmCorners.Northing[2]=345;
   cmd.utmCorners.Northing[3]=456;
   cmd.utmCorners.Easting[0]=321;
   cmd.utmCorners.Easting[1]=432;
   cmd.utmCorners.Easting[2]=543;
   cmd.utmCorners.Easting[3]=654;
   strcpy(cmd.utmCorners.Zone, "14 South");
   cmd.imageSize = 1000;

   sMessageWrapper sMessageWrapper;
   sMessageWrapper << cmd;
   generate_command(sMessageWrapper,msg_ptr);
}

void createROITrackUpdate(string* msg_ptr) 
{
   sROITrackUpdate cmd;
   cmd.regionID=1;
   cmd.trackUpdatePriority=3;
   cmd.numTracks=2;
   cmd.numChips=3;

   sMessageWrapper sMessageWrapper;
   sMessageWrapper << cmd;
   generate_command(sMessageWrapper,msg_ptr);
}

void createNFOVCommand(string* msg_ptr) 
{
   sNFOVCommand cmd;

   cmd.currentNFOVCommandID = 0;
   cmd.enable = true;
   cmd.utmCoord.Easting  = 218067;
   cmd.utmCoord.Northing = 3713029;
   strcpy(cmd.utmCoord.Zone, "14 South");
   cmd.imageUpdatePriority = 2;
   cmd.imagePeriod = 1;
   cmd.jpegQuality = 70;

   sMessageWrapper sMessageWrapper;
   sMessageWrapper << cmd;
   generate_command(sMessageWrapper,msg_ptr);
}

void createNFOVImageUpdate(string* msg_ptr) 
{
   sNFOVImageUpdate cmd;

   cmd.utmCorners.Northing[0]=123;
   cmd.utmCorners.Northing[1]=234;
   cmd.utmCorners.Northing[2]=345;
   cmd.utmCorners.Northing[3]=456;
   cmd.utmCorners.Easting[0]=321;
   cmd.utmCorners.Easting[1]=432;
   cmd.utmCorners.Easting[2]=543;
   cmd.utmCorners.Easting[3]=654;
   strcpy(cmd.utmCorners.Zone, "14 South");
   
   cmd.imageSize = 1000;

   sMessageWrapper sMessageWrapper;
   sMessageWrapper << cmd;
   generate_command(sMessageWrapper,msg_ptr);
}

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   cout << "=====================================================" << endl;
   cout << "Starting PETER's ANALYST machine message sender:" << endl;
   cout << "=====================================================" << endl;

// Set the URL to point to the IPAddress of your broker:

   string broker_URL = "tcp://127.0.0.1:61616";
//   string broker_URL = "tcp://155.34.162.179:61616";
   if ( argc > 1 ) broker_URL = "tcp://" + string(argv[1]);
   cout << "broker_URL = " << broker_URL << endl;
   
   bool include_sender_and_timestamp_info_flag=true;

// As of 11/30/09, Alex's code on the communications machine is
// expecting the message queue channel name to be
// ANALYSIS_TO_COMMUNICATION for messages originating from Peter's
// analysis machine:

   string message_sender_ID="PETER_ANALYSIS_MACHINE";
   string message_queue_channel_name="ANALYSIS_TO_COMMUNICATION";

   RTPSMessenger ROI_messenger(
      broker_URL, message_queue_channel_name, message_sender_ID, 
      include_sender_and_timestamp_info_flag);

   RTPSMessenger NFOV_messenger(
      broker_URL, message_queue_channel_name, message_sender_ID, 
      include_sender_and_timestamp_info_flag);

// -----------------------------------------------------------------
// Publish ActiveMQ messages for ROIs, NFOVs, etc:

   string message;

   const int n_max=10000;
   for (int n=0; n<n_max; n++) 
   {   
      sleep(1);
      cout << "n = " << n << endl;
      
// Publish analyst-generated ROI message:

      createROICommand(&message);
      cout << "Sending ROI message:" << endl;
      ROI_messenger.sendBytesMessage(message.c_str(), message.length());
      
// Publish analyst-generated ROI Image Update message:

      createROIImageUpdate(&message);
      cout << "Sending ROI image update message:" << endl;
      ROI_messenger.sendBytesMessage(message.c_str(), message.length());
      
// Publish analyst-generated ROI Track Update message:

      createROITrackUpdate(&message);
      cout << "Sending ROI track update message:" << endl;
      ROI_messenger.sendBytesMessage(message.c_str(), message.length());

// Publish analyst-generated NFOV message:

      createNFOVCommand(&message);
      cout << "Sending NFOV message" << endl;
      NFOV_messenger.sendBytesMessage(message.c_str(), message.length());

// Publish analyst-generated NFOV Image Update message:

      createNFOVImageUpdate(&message);
      cout << "Sending NFOV image update message" << endl;
      NFOV_messenger.sendBytesMessage(message.c_str(), message.length());

   } // loop over index n labeling messages 

}
