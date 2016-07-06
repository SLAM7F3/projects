// ==========================================================================
// Header file for RTPSMessenger class
// ==========================================================================
// Last modified on 10/13/09; 10/15/09; 11/30/09
// ==========================================================================

#ifndef RTPS_MESSENGER_H
#define RTPS_MESSENGER_H

#include "messenger/Messenger.h"
#include "Qt/rtps/MessageWrapper.h"

class RTPSMessenger : public Messenger
{

public:

    RTPSMessenger(std::string brokerURI,std::string topicName,
                  bool include_sender_and_timestamp_info_flag=true,
                  bool useQueue=false);
    RTPSMessenger(std::string brokerURI,std::string topicName,
                  std::string message_sender_ID,
                  bool include_sender_and_timestamp_info_flag=true,
                  bool useQueue=false);
    virtual ~RTPSMessenger();
    
// Set & get member functions:

    virtual void onMessage( const Message* message );

// RTPS message reconstruction member functions:

    void reconstruct_ROI_message(std::string roi_message);
    void reconstruct_SystemStatus_message(std::string systemstatusMessage);

private:

    void allocate_member_objects();
    void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

/*
inline std::string RTPSMessenger::get_topicName() const
{
   return topicName;
}
*/


#endif  // RTPSMessenger.h
