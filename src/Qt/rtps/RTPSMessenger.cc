// ==========================================================================
// RTPSMessenger class member function definitions
// ==========================================================================
// Last modified on 10/13/09; 10/15/09; 11/30/09
// ==========================================================================

#include "RTPSMessenger.h"

using namespace activemq::core;
using namespace activemq::util;

using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void RTPSMessenger::allocate_member_objects()
{
}

void RTPSMessenger::initialize_member_objects()
{
}

// ---------------------------------------------------------------------
RTPSMessenger::RTPSMessenger(
   string broker_URL,string topicName,
   bool include_sender_and_timestamp_info_flag,bool useQueue) :
   Messenger(broker_URL,topicName,include_sender_and_timestamp_info_flag,
             useQueue)
{
//   cout << "inside RTPSMessenger constructor #1" << endl;
   allocate_member_objects();
   initialize_member_objects();
}

// ---------------------------------------------------------------------
RTPSMessenger::RTPSMessenger(
   string broker_URL,string topicName,string message_sender_ID,
   bool include_sender_and_timestamp_info_flag,bool useQueue) :
   Messenger(broker_URL,topicName,message_sender_ID,
             include_sender_and_timestamp_info_flag,useQueue)
{
//   cout << "inside RTPSMessenger constructor #2" << endl;
   
   allocate_member_objects();
   initialize_member_objects();
}

RTPSMessenger::~RTPSMessenger()
{
//   cout << "inside RTPSMessenger destructor" << endl;
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

// Called from the consumer when a message arrives

void RTPSMessenger::onMessage( const Message* msg_ptr )
{
   cout << "inside RTPSMessenger::onMessage()" << endl;
   cout << "msg_sender_ID = " << get_msg_sender_ID ()<< endl;

   try {
      const BytesMessage* BytesMessage_ptr =
         dynamic_cast< const BytesMessage* >( msg_ptr );

// Receive & stick message in the mailbox for later retrieval:

      if ( BytesMessage_ptr != NULL ) 
      {
//         cout << "BytesMessage_ptr = " << BytesMessage_ptr << endl;

         string msg_producer_ID="";
         if (include_sender_and_timestamp_info_flag)
         {
            msg_producer_ID=
               BytesMessage_ptr->getStringProperty("Message_sender_ID");
//            cout << "msg_producer_ID = " << msg_producer_ID << endl;
         }

// Ignore self-messages!

         if (msg_producer_ID != get_msg_sender_ID())
         {
            string queue_name=get_topicName();
//            cout << "queue_name = " << queue_name << endl;
            if (queue_name=="ROI")
            {
               string roi_msg_str=Messenger::convert_BytesMessage_to_string(
                  BytesMessage_ptr);
               reconstruct_ROI_message(roi_msg_str);
            }
         } // msg_producer != curr messenger conditional
      }
   } catch (CMSException& e) 
      {
         e.printStackTrace();
      }
}

// ==========================================================================
// RTPS message reconstruction member functions
// ==========================================================================

void RTPSMessenger::reconstruct_ROI_message(string roiMessage)
{
   cout << "inside RTPSMessenger::reconstruct_ROI_message()" << endl;
//   cout << "Input roiMessage = " << roiMessage << endl;

   stringstream buffer;
   buffer.clear();
   buffer.str(roiMessage);

   sMessageWrapper wrapper;
   buffer >> wrapper;

   sROICommand new_msg;
   wrapper >> new_msg;

   cout.precision(12);
   cout << "new_msg.regionID = " << new_msg.regionID << endl;
   cout << "new_msg.utmCoord.Easting = " << new_msg.utmCoord.Easting << endl;
   cout << "new_msg.utmCoord.Northing = " << new_msg.utmCoord.Northing 
        << endl;
   cout << "new_msg.trackID = " << new_msg.trackID << endl;
   cout << "new_msg.trackUpdatePriority = " 
        << new_msg.trackUpdatePriority << endl;
}

// ---------------------------------------------------------------------
void RTPSMessenger::reconstruct_SystemStatus_message(
   string systemstatusMessage)
{
   cout << "inside RTPSMessenger::reconstruct_SystemStatus_message()" << endl;
   cout << "Input systemstatusMessage = " << systemstatusMessage << endl;

   stringstream buffer;
   buffer.clear();
   buffer.str(systemstatusMessage);

   sMessageWrapper wrapper;
   buffer >> wrapper;

   sSystemStatus new_msg;
   wrapper >> new_msg;

   cout.precision(12);
   cout << "new_msg.running = " << new_msg.running << endl;
   cout << "new_msg.utmCoord.Easting = " << new_msg.utmCoord.Easting << endl;
   cout << "new_msg.utmCoord.Northing = " << new_msg.utmCoord.Northing 
        << endl;
   cout << "new_msg.altitude = " << new_msg.altitude << endl;
   cout << "new_msg.heading = " << new_msg.heading << endl;

   cout << "new_msg.NFOVAvailable = " << new_msg.NFOVAvailable << endl;
   cout << "new_msg.currentNFOVCommandID = " 
        << new_msg.currentNFOVCommandID << endl;

   cout << "new_msg.numValidROIS = " << new_msg.numValidROIS << endl;

   for (int r=0; r<MAX_NUM_ROIS; r++)
   {
      cout << "r = " << r << " roiID[r] = " << new_msg.roiId[r] << endl;      
      cout << "r = " << r << " roiState[r] = " << new_msg.roiState[r] 
           << endl;  
   }


}
