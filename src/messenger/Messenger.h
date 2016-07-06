// ==========================================================================
// Header file for Messenger class
// ==========================================================================
// Last modified on 6/9/11; 2/17/12; 1/23/14; 4/5/14; 4/8/14
// ==========================================================================

#ifndef MESSENGER_H
#define MESSENGER_H


// DELL:

#include <activemq/concurrent/Thread.h>
#include <activemq/concurrent/Runnable.h>
#include <activemq/util/Integer.h>
#include <activemq/util/Config.h>

/*
// MAC

#include <decaf/lang/Thread.h>
#include <decaf/lang/Runnable.h>
#include <decaf/lang/Integer.h>
#include <decaf/util/Config.h>
*/


#include <activemq/core/ActiveMQConnectionFactory.h>

#include <cms/Connection.h>
#include <cms/Session.h>
#include <cms/TextMessage.h>
#include <cms/BytesMessage.h>
#include <cms/MapMessage.h>
#include <cms/ExceptionListener.h>
#include <cms/MessageListener.h>
#include <stdlib.h>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "messenger/message.h"

// DELL
using namespace activemq::concurrent;

// MAC:  
// using namespace decaf::lang;


using namespace cms;

class Messenger : public ExceptionListener,
                  public MessageListener,
                  public Runnable
{

  public:
   
   typedef std::pair<std::string,std::string> Property;

    Messenger(std::string brokerURI,std::string topicName,
              bool include_sender_and_timestamp_info_flag=true,
              bool useQueue=false);
    Messenger(std::string brokerURI,std::string topicName,
              std::string message_sender_ID,
              bool include_sender_and_timestamp_info_flag=true,
              bool useQueue=false);
    virtual ~Messenger();
    void start_operations();
    void stop_operations();
    
// Set & get member functions:

    unsigned int get_n_messages_in_mailbox() const;
    unsigned int get_n_curr_messages() const;
    unsigned int get_n_sent_messages() const;
    unsigned int get_n_received_messages() const;
    std::string get_topicName() const;
    void set_msg_sender_ID(std::string ID);
    std::string get_msg_sender_ID() const;
    std::string get_ClientID() const;

// Send bytes message member functions:

    void sendBytesMessage(std::string inputstring);
    void sendBytesMessage(const char* bytes, unsigned int bytesSize);
    BytesMessage* generate_BytesMessage(
       const char* bytes, unsigned int bytesSize);
    int get_nbytes_in_BytesMessage(const BytesMessage* BytesMessage_ptr);
    unsigned char* convert_BytesMessage_to_unsigned_chars(
       const BytesMessage* BytesMessage_ptr);
    std::string convert_BytesMessage_to_string(
       const BytesMessage* BytesMessage_ptr);

// Send text message member functions:

    void sendTextMessage
       (std::string textMsg,bool print_msg_flag=false);
    void sendTextMessage( 
       std::string textMsg, std::string key, std::string value,
       bool print_msg_flag=false);
    void sendTextMessage( 
       std::string textMsg,
       const std::vector<Property>& particular_string_properties,
       bool print_msg_flag=false);
    TextMessage* generate_TextMessage( 
       std::string textMsg,
       const std::vector<Property>& particular_string_properties);

    bool connected_to_broker_flag() const;
    virtual void run();
    void changeTopic(std::string topic_name,bool useQueue=false);

    // Called from the consumer when a message arrives
    virtual void onMessage( const Message* message );

    // If something bad happens you see it here as this class is also been
    // registered as an ExceptionListener with the connection.

    virtual void onException( const CMSException& ex AMQCPP_UNUSED);

    bool copy_messages_and_purge_mailbox();
    message* get_message_ptr(unsigned int n);
    const message* get_message_ptr(unsigned int n) const;

// Broadcast member functions:

    void broadcast_subpacket(std::string command);
    void broadcast_subpacket(
       std::string command,const std::vector<Property>& properties);
    void broadcast_progress(
       double curr_progress_frac,std::string progress_type);
    void broadcast_finished_progress(std::string progress_type);
    void broadcast_clear_progress(std::string progress_type);
    void broadcast_current_framenumber(double curr_t);

// Cancel operation member functions:

    void clear_cancelled_operation();
    std::string check_for_cancel_operation_message();

  protected:

    bool include_sender_and_timestamp_info_flag;

  private:

    bool mailbox_locked_flag,clearing_mailbox_flag;
    unsigned int n_sent_messages,n_received_messages;
    Connection* connection_ptr;
    Session* session_ptr;
    Destination* destination_ptr;
    Thread* messengerThread_ptr;
    MessageConsumer* consumer_ptr;
    MessageProducer* producer_ptr;
    bool useQueue;
    std::string brokerURI,topicName,ClientID;
    std::string msg_sender_ID,cancelled_operation;
//     std::vector<Property> general_string_properties;

    std::vector<message> mailbox;
    std::vector<message> curr_messages;

    void allocate_member_objects();
    void initialize_member_objects();
    void finish_construction(
       std::string broker_URL,std::string topicName,bool useQueue);
    void print_sent_message_contents(TextMessage* textMessage_ptr);
    void print_sent_message_contents(const BytesMessage* BytesMessage_ptr);
    void cleanup();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline std::string Messenger::get_topicName() const
{
   return topicName;
}

inline void Messenger::set_msg_sender_ID(std::string ID)
{
   msg_sender_ID=ID;
}

inline std::string Messenger::get_msg_sender_ID() const
{
   return msg_sender_ID;
}

inline void Messenger::clear_cancelled_operation() 
{
   cancelled_operation="";
}

inline std::string Messenger::get_ClientID() const
{
   return ClientID;
}


#endif  // Messenger.h
