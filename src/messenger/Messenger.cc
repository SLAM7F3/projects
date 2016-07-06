// ==========================================================================
// Messenger class member function definitions
// ==========================================================================
// Last modified on 2/14/12; 2/17/12; 1/23/14; 4/5/14
// ==========================================================================

#include <unistd.h> // for sleep() calls
#include "messenger/Messenger.h"
#include "general/stringfuncs.h"
#include "time/timefuncs.h"

#include "general/outputfuncs.h"

using namespace activemq::core;
// using namespace activemq::util;

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

void Messenger::allocate_member_objects()
{
}

void Messenger::initialize_member_objects()
{
   mailbox_locked_flag=clearing_mailbox_flag=false;
   n_sent_messages=n_received_messages=0;
   connection_ptr = NULL;
   session_ptr = NULL;
   destination_ptr = NULL;
   consumer_ptr = NULL;
   producer_ptr=NULL;
   clear_cancelled_operation();
}

// ---------------------------------------------------------------------
Messenger::Messenger(
   string broker_URL,string topicName,
   bool include_sender_and_timestamp_info_flag,bool useQueue) 
{
//   cout << "inside Messenger constructor #1" << endl;
   allocate_member_objects();
   initialize_member_objects();
   this->include_sender_and_timestamp_info_flag=
      include_sender_and_timestamp_info_flag;

   msg_sender_ID = topicName;
   msg_sender_ID = topicName+"_"+timefunc::getcurrdate();
   finish_construction(broker_URL,topicName,useQueue);
}

// ---------------------------------------------------------------------
Messenger::Messenger(
   string broker_URL,string topicName,string message_sender_ID,
   bool include_sender_and_timestamp_info_flag,bool useQueue)
{
//   cout << "inside Messenger constructor" << endl;
   
   allocate_member_objects();
   initialize_member_objects();
   this->include_sender_and_timestamp_info_flag=
      include_sender_and_timestamp_info_flag;

// Assign time-dependent msg_sender_ID if input message_sender_ID
// doesn't equal "ALLOW_SELF_MESSAGES":

   msg_sender_ID = message_sender_ID;
   if (message_sender_ID != "ALLOW_SELF_MESSAGES" &&
       message_sender_ID != "DISALLOW_SELF_MESSAGES")
   {
      msg_sender_ID = message_sender_ID;
      msg_sender_ID = message_sender_ID+"_"+timefunc::getcurrdate();
   }

//   cout << "msg_sender_ID = " << msg_sender_ID << endl;
   finish_construction(broker_URL,topicName,useQueue);
}
 
// ---------------------------------------------------------------------
void Messenger::finish_construction(
   string broker_URL,string topicName,bool useQueue)
{
//   cout << "inside Messenger::finish_construction" << endl;
//   cout << "include_sender_and_timestamp_info_flag = "
//        << include_sender_and_timestamp_info_flag << endl;

   brokerURI = broker_URL +
      "?wireFormat=openwire"
      "&transport.useAsyncSend=true";
//        "&transport.commandTracingEnabled=true"
//        "&transport.tcpTracingEnabled=true";
//        "&wireFormat.tightEncodingEnabled=true";
//   cout << "brokerURI = " << brokerURI << endl;
   
   this->topicName = topicName;
   this->useQueue = useQueue;
   
/*
   if (include_sender_and_timestamp_info_flag)
   {
      Property P("Message_sender_ID",msg_sender_ID);
      general_string_properties.push_back(P);
   }
*/

   start_operations();
}

Messenger::~Messenger()
{
//   cout << "inside Messenger destructor" << endl;
   stop_operations();
}
 
// ---------------------------------------------------------------------
void Messenger::start_operations()
{
//   cout << "inside Messenger::start_operations" << endl;
   messengerThread_ptr=new Thread(this);
   messengerThread_ptr->start();
   sleep(1.0);	// secs
}

void Messenger::stop_operations()
{
//   cout << "inside Messenger::stop_operations()" << endl;
   cleanup();
   while (messengerThread_ptr != NULL)
   {
      messengerThread_ptr->join();
      delete messengerThread_ptr;
      messengerThread_ptr=NULL;
   }
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

unsigned int Messenger::get_n_messages_in_mailbox() const
{
   return mailbox.size();
}

unsigned int Messenger::get_n_curr_messages() const
{
   return curr_messages.size();
}

unsigned int Messenger::get_n_sent_messages() const
{
   return n_sent_messages;
}

unsigned int Messenger::get_n_received_messages() const
{
   return n_received_messages;
}

// ==========================================================================
// Send byte message member functions
// ==========================================================================

void Messenger::sendBytesMessage(string inputstring) 
{
   char* cstr=stringfunc::string_to_chars(inputstring);
   sendBytesMessage(cstr,inputstring.size()+1);
   delete [] cstr;
}

void Messenger::sendBytesMessage(const char* bytes, unsigned int bytesSize) 
{
//   cout << "inside Messenger::sendBytesMessage()" << endl;

   BytesMessage* msg_ptr=generate_BytesMessage(bytes, bytesSize);

   if (msg_ptr==NULL)    {
      cout << "Error in Messenger::sendByteMessage()" << endl;
      cout << "msg_ptr = NULL" << endl;
      return;
   }

//   print_sent_message_contents(msg_ptr);

   try
      {
         producer_ptr->send(msg_ptr);
         double microsecs_between_messages=1.0;
         usleep(microsecs_between_messages);
      }
   catch (CMSException& e) 
      { 
         cout << "Error thrown by *producer_ptr in trying to send Bytes msg"
              << endl;
         e.printStackTrace(); 
      }

// Delete dynamically generated message after it's been sent:

   delete msg_ptr;

   n_sent_messages++;
}

// ---------------------------------------------------------------------
BytesMessage* Messenger::generate_BytesMessage(
   const char* bytes, unsigned int bytesSize)  
{
//   cout << "inside Messenger::generate_BytesMessage()" << endl;
//   cout << "bytes = " << bytes << " bytesSize = " << bytesSize << endl;

   if (!connected_to_broker_flag()) return NULL;
   BytesMessage* msg_ptr = session_ptr->createBytesMessage(
      reinterpret_cast<const unsigned char*>(bytes), bytesSize);

/*
   for (unsigned int p=0; p<general_string_properties.size(); p++)
   {
      msg_ptr->setStringProperty(
         general_string_properties[p].first,
         general_string_properties[p].second);
   }
*/

   if (include_sender_and_timestamp_info_flag) 
   {
      msg_ptr->setStringProperty("Message_sender_ID",msg_sender_ID);
      msg_ptr->setStringProperty("Message_timestamp",timefunc::getcurrdate());
   }

   return msg_ptr;
} 

// ---------------------------------------------------------------------
// Recall every BytesMessage is terminated with a final null
// character.

int Messenger::get_nbytes_in_BytesMessage(
   const BytesMessage* BytesMessage_ptr)
{
//   cout << "inside Messenger::get_nbytes_in_BytesMessage()" << endl;

   if (BytesMessage_ptr==NULL) return 0;

   std::size_t body_length=BytesMessage_ptr->getBodyLength();
//   cout << "body_length = " << body_length << endl;
   return int(body_length);
}

// ---------------------------------------------------------------------
unsigned char* Messenger::convert_BytesMessage_to_unsigned_chars(
   const BytesMessage* BytesMessage_ptr)
{
//   cout << "inside Messenger::convert_BytesMessage_to_unsigned_chars()" 
//        << endl;

   if (BytesMessage_ptr==NULL) return NULL;
   return const_cast<unsigned char*>(BytesMessage_ptr->getBodyBytes());
}

// ---------------------------------------------------------------------
string Messenger::convert_BytesMessage_to_string(
   const BytesMessage* BytesMessage_ptr)
{
//   cout << "inside Messenger::convert_BytesMessage_to_string()" << endl;
   return stringfunc::unsigned_char_array_to_string(
      convert_BytesMessage_to_unsigned_chars(BytesMessage_ptr),
      get_nbytes_in_BytesMessage(BytesMessage_ptr));
}

// ---------------------------------------------------------------------
void Messenger::print_sent_message_contents(
   const BytesMessage* BytesMessage_ptr)
{
   cout << "inside Messenger::print_sent_message_contents(BytesMessage)" 
        << endl;

   if (BytesMessage_ptr==NULL) return;

   int nbytes=get_nbytes_in_BytesMessage(BytesMessage_ptr);
   unsigned char* buffer_cstr=
      convert_BytesMessage_to_unsigned_chars(BytesMessage_ptr);
   string buffer=stringfunc::unsigned_char_array_to_string(
      buffer_cstr,nbytes);
   cout << "buffer string = " << buffer << endl;

/*
   string text = BytesMessage_ptr->getText();
   string msg_producer_ID="";
//   string msg_producer_ID=
//      textMessage_ptr->getStringProperty("Message_sender_ID");

   vector<Property> properties;
   vector<string> PropertyNames=textMessage_ptr->getPropertyNames();
   for (unsigned int p=0; p<PropertyNames.size(); p++)
   {
      Property P(
         PropertyNames[p],
         textMessage_ptr->getStringProperty(PropertyNames[p]));
      properties.push_back(P);
   } // loop over index p labeling textMessage's properties

   message sent_message(text,msg_producer_ID,properties);
   cout << "Contents of sent message:  " << sent_message << endl;
*/

   cout << "n_sent_messages = " << get_n_sent_messages() << endl;
}

// ==========================================================================
// Send text message member functions
// ==========================================================================

void Messenger::sendTextMessage(string textMsg,bool print_msg_flag)
{
  //  cout << "inside Messenger::sendTextMessage() #1" << endl;
   vector<Property> particular_string_properties;
   sendTextMessage(textMsg,particular_string_properties,print_msg_flag);
}

// ---------------------------------------------------------------------
void Messenger::sendTextMessage( 
   string textMsg,string key,string value,bool print_msg_flag)
{
  //  cout << "inside Messenger::sendTextMessage() #2" << endl;

   Property curr_property(key,value);
   vector<Property> particular_string_properties;
   particular_string_properties.push_back(curr_property);
   sendTextMessage(textMsg,particular_string_properties,print_msg_flag);
}

// ---------------------------------------------------------------------
// This overloaded and most general version of sendTextMessage()
// dynamically creates a TextMessage, sends it via *producer_ptr and
// then deletes the TextMessage.

void Messenger::sendTextMessage( 
   string textMsg,const vector<Property>& particular_string_properties,
   bool print_msg_flag)
{
//   cout << "inside Messenger::sendTextMessage() #3" << endl;
//   cout << "TopicName = " << get_topicName << endl;
//   cout << "textMsg = " << textMsg << endl;

   TextMessage* msg_ptr=generate_TextMessage(
      textMsg,particular_string_properties);

   if (msg_ptr==NULL) 
   {
      cout << "Error in Messenger::sendTextMessage()" << endl;
      cout << "msg_ptr = NULL" << endl;
      return;
   }

   if (print_msg_flag) print_sent_message_contents(msg_ptr);

   try
      {
         producer_ptr->send(msg_ptr);
         double microsecs_between_messages=1.0;
         usleep(microsecs_between_messages);
      }
   catch (CMSException& e) 
      { 
         cout << "Error thrown by *producer_ptr in trying to send msg"
              << endl;
         e.printStackTrace(); 
      }

// Delete dynamically generated message after it's been sent:

   delete msg_ptr;

   n_sent_messages++;
}

// ---------------------------------------------------------------------
TextMessage* Messenger::generate_TextMessage( 
   string textMsg,const vector<Property>& particular_string_properties)
{
//   cout << "inside Messenger::generate_TextMessage()" << endl;
//   cout << "textMsg = " << textMsg << endl;

//   cout << "connected_to_broker_flag = " << connected_to_broker_flag()
//        << endl;
//   cout << "session_ptr = " << session_ptr << endl;
//   cout << "connection_ptr = " << connection_ptr << endl;
//   cout << "connected_to_broker_flag() = " << connected_to_broker_flag()
//       << endl;

   if (!connected_to_broker_flag()) return NULL;

   TextMessage* msg_ptr = session_ptr->createTextMessage( textMsg );

/*
   cout << "general_string_properties.size() = "
        << general_string_properties.size() << endl;

   for (unsigned int p=0; p<general_string_properties.size(); p++)
   {
      msg_ptr->setStringProperty(
         general_string_properties[p].first,
         general_string_properties[p].second);
   }
*/

// Incorporate time stamp into message prior to sending it:

   if (include_sender_and_timestamp_info_flag)
   {
      msg_ptr->setStringProperty("Message_sender_ID",msg_sender_ID);
      msg_ptr->setStringProperty("Message_timestamp",timefunc::getcurrdate());
   }
   
   for (unsigned int p=0; p<particular_string_properties.size(); p++)
   {
      msg_ptr->setStringProperty(
         particular_string_properties[p].first,
         particular_string_properties[p].second);
   }

   return msg_ptr;
}

// ---------------------------------------------------------------------
// Member function print_sent_message_contents takes in pointer
// testMessage_ptr to an activeMQ TextMessage.  This method parses and
// prints its contents.

void Messenger::print_sent_message_contents(TextMessage* textMessage_ptr)
{
//   cout << "inside Messenger::print_sent_message_contents()" << endl;

   if (textMessage_ptr==NULL) return;

   string text = textMessage_ptr->getText();
   string msg_producer_ID="";
//   string msg_producer_ID=
//      textMessage_ptr->getStringProperty("Message_sender_ID");

   vector<Property> properties;
   vector<string> PropertyNames=textMessage_ptr->getPropertyNames();
   for (unsigned int p=0; p<PropertyNames.size(); p++)
   {
      Property P(
         PropertyNames[p],
         textMessage_ptr->getStringProperty(PropertyNames[p]));
      properties.push_back(P);
   } // loop over index p labeling textMessage's properties

   message sent_message(text,msg_producer_ID,properties);
   cout << "Contents of sent message:  " << sent_message << endl;
   cout << "n_sent_messages = " << get_n_sent_messages() << endl;
}

// ---------------------------------------------------------------------
void Messenger::run() 
{
//   cout << "inside Messenger::run()" << endl;

   try {
      // Create a ConnectionFactory

//      cout << "brokerURI = " << brokerURI << endl;
      ActiveMQConnectionFactory* connectionFactory =
         new ActiveMQConnectionFactory( brokerURI );
//      cout << "connectionFactory = " << connectionFactory << endl;

      // Create a Connection
      connection_ptr = connectionFactory->createConnection();
//      cout << "connection_ptr = " << connection_ptr << endl;

      ClientID=connection_ptr->getClientID();
//      cout << "ClientID = " << ClientID << endl;

      delete connectionFactory;
      connection_ptr->start();

      connection_ptr->setExceptionListener(this);

      // Create a Session
      session_ptr = connection_ptr->createSession( Session::AUTO_ACKNOWLEDGE );
//      cout << "session_ptr = " << session_ptr << endl;

      changeTopic(topicName,useQueue);

   } catch (CMSException& e) {
      e.printStackTrace();
   }
}

// ---------------------------------------------------------------------
// Member function changeTopic() intentionally mimics the Java method
// which Michael Yee wrote on 6/9/11 to enable topic names to be
// dynamically chnaged

void Messenger::changeTopic(string topic_name,bool useQueue)
{
//   cout << "inside Messenger::changeTopic()" << endl;
//   cout << "topic_name = " << topic_name << endl;
//   cout << "topic_name.size() = " << topic_name.size() << endl;

// Create the destination (Topic or Queue)

   if( useQueue ) 
   {
      destination_ptr = session_ptr->createQueue( topic_name );
   } else 
   {
      destination_ptr = session_ptr->createTopic( topic_name );
   }

// Create the consumer

   if (consumer_ptr != NULL) consumer_ptr->close();

   string selector="";
   consumer_ptr = session_ptr->createConsumer( 
      destination_ptr , selector );
   consumer_ptr->setMessageListener( this );

// Create the producer

   if (producer_ptr != NULL) producer_ptr->close();

   producer_ptr = session_ptr->createProducer( destination_ptr );
   producer_ptr->setDeliveryMode( DeliveryMode::NON_PERSISTENT );

   cout.flush();
   cerr.flush();
}

// ---------------------------------------------------------------------
// Member function connected_to_broker_flag() should be called prior
// to attempting to send or receive ActiveMQ messages

bool Messenger::connected_to_broker_flag() const
{
//   cout << "inside Messenger::connected_to_broker_flag()" << endl;
//   bool flag=(connection_ptr != NULL && session_ptr != NULL);
//   cout << "flag = " << flag << endl;
   return (connection_ptr != NULL && session_ptr != NULL);
}

// ---------------------------------------------------------------------
// Called from the consumer when a message arrives

void Messenger::onMessage( const Message* msg_ptr )
{
//   cout << "inside Messenger::onMessage()" << endl;

   try {
      const TextMessage* textMessage_ptr =
         dynamic_cast< const TextMessage* >( msg_ptr );

// Receive & stick message in the mailbox for later retrieval:

      if ( textMessage_ptr != NULL ) 
      {
         string text = textMessage_ptr->getText();

         string msg_producer_ID="";
         if (include_sender_and_timestamp_info_flag)
         {
            msg_producer_ID=
               textMessage_ptr->getStringProperty("Message_sender_ID");
         }

// Loop over textMessage's properties and store them into STL vector
// for long-term storage within our message object:

         vector<Property> properties;

         vector<string> PropertyNames=textMessage_ptr->getPropertyNames();
         for (unsigned int p=0; p<PropertyNames.size(); p++)
         {
            Property P(
               PropertyNames[p],
               textMessage_ptr->getStringProperty(PropertyNames[p]));
            properties.push_back(P);
         } // loop over index p labeling textMessage's properties
         message curr_message(text,msg_producer_ID,properties);

// Ignore self-messages only if msg_sender_ID != "ALLOW_SELF_MESSAGES":

//         if ((msg_sender_ID != "ALLOW_SELF_MESSAGES" 
//              && msg_producer_ID==msg_sender_ID

//         cout << "msg_sender_ID = " << msg_sender_ID << endl;
//         cout << "msg_producer_ID = " << msg_producer_ID << endl;

         if (msg_sender_ID=="DISALLOW_SELF_MESSAGES" &&
             msg_producer_ID==msg_sender_ID)
         {
            cout << "Ignoring self-messsages" << endl;
         }
         else
         {
//            cout << "Message received from "+msg_producer_ID << endl;
//            cout << "Text message = " << curr_message.get_text_message()
//                 << endl;

            while (clearing_mailbox_flag)
            {
               usleep(100);
               cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
                    << endl;
               cout << "inside Messenger::onMessage(), mailbox purging flag=true"
                    << endl;
               cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
                    << endl;
            }
            mailbox_locked_flag=true;
            mailbox.push_back( curr_message );
//            cout << "mailbox message = "
//                 << mailbox.back() << endl;
//            cout << "get_n_messages_in_mailbox() = "
//                 << get_n_messages_in_mailbox() << endl;
            mailbox_locked_flag=false;
         }
      } // else, not a text message. ignore it.
   } catch (CMSException& e) 
      {
         e.printStackTrace();
      }
}

// ---------------------------------------------------------------------
void Messenger::onException( const CMSException& ex AMQCPP_UNUSED) 
{
   cout << "JMS Exception occurred.  Shutting down client." << endl;
}

// ---------------------------------------------------------------------
bool Messenger::copy_messages_and_purge_mailbox()
{
//   cout << "inside Messenger::copy_messages_and_purge_mailbox()" << endl;

   curr_messages.clear();
   for (unsigned int m=0; m<mailbox.size(); m++)
   {
      curr_messages.push_back(mailbox[m]);
      n_received_messages++;
   }

   const int max_counter_value=10;
   int counter=0;
   while (mailbox_locked_flag && counter < max_counter_value)
   {
      usleep(100);
      cout << "******************************************************" << endl;
      cout << "In Messenger::copy_messages_and_purge_mailbox(), mailbox locked"
           << endl;
      cout << "counter = " << counter << endl;
      cout << "******************************************************" << endl;
      counter++;
   }
   if (mailbox_locked_flag) return false;

   clearing_mailbox_flag=true;
   mailbox.clear();
   clearing_mailbox_flag=false;
   
//   cout << "At end of copy_messages_and_purge_mailbox()" << endl;
//   cout << "curr_messages.size() = " << curr_messages.size() << endl;
//   cout << "mailbox.size() = " << mailbox.size() << endl;
   return true;
}

// ---------------------------------------------------------------------
message* Messenger::get_message_ptr(unsigned int n) 
{
   if (n >= 0 && n < curr_messages.size())
   {
      return &(curr_messages[n]);
   }
   else
   {
      return NULL;
   }
}

const message* Messenger::get_message_ptr(unsigned int n) const
{
   if (n >= 0 && n < curr_messages.size())
   {
      return &(curr_messages[n]);
   }
   else
   {
      return NULL;
   }
}

// ---------------------------------------------------------------------
void Messenger::cleanup()
{
//   cout << "inside Messenger::cleanup()" << endl;

   //*************************************************
   // Always close destination, consumers and producers before
   // you destroy their sessions and connection.
   //*************************************************

   // Destroy resources.
   try{
      if( destination_ptr != NULL ) delete destination_ptr;
   }catch (CMSException& e) { e.printStackTrace(); }
   destination_ptr = NULL;

   try{
      if( consumer_ptr != NULL ) delete consumer_ptr;
   }catch (CMSException& e) { e.printStackTrace(); }
   consumer_ptr = NULL;

   try{
      if( producer_ptr != NULL ) delete producer_ptr;
   }catch (CMSException& e) { e.printStackTrace(); }
   producer_ptr = NULL;

   // Close open resources.
   try{
      if( session_ptr != NULL ) session_ptr->close();
      if( connection_ptr != NULL ) connection_ptr->close();
   }catch (CMSException& e) { e.printStackTrace(); }

   // Now Destroy them

   try{
      if( session_ptr != NULL ) delete session_ptr;
   }catch (CMSException& e) { e.printStackTrace(); }
   session_ptr = NULL;

   try{
      if( connection_ptr != NULL ) delete connection_ptr;
   }catch (CMSException& e) { e.printStackTrace(); }
   connection_ptr = NULL;
}

// ==========================================================================
// Broadcast member functions
// ==========================================================================

void Messenger::broadcast_subpacket(string command)
{
   vector<Property> properties;
   broadcast_subpacket(command,properties);
}

void Messenger::broadcast_subpacket(
   string command,const vector<Property>& properties)
{
//   cout << "inside Messenger::broadcast_subpacket()" << endl;
   
   sendTextMessage(command,properties);
}

// ---------------------------------------------------------------------
// Member function broadcast_progress() generates an output ActiveMQ
// message containing progress percentage information for thin client
// progress bar display purposes.

void Messenger::broadcast_progress(
   double curr_progress_frac,string progress_type)
{
//   cout << "inside Messenger::broadcast_progress()" << endl;
//   cout << "frac = " << curr_progress_frac 
//        << " type = " << progress_type << endl;

// Do NOT broadcast any more progress messages if operation has been
// cancelled!

   if (progress_type==cancelled_operation) return;

// Recall that ActiveMQ messages consist of a single command string
// along with an STL vector of key-value string pair properties:

   string command,key,value;
   vector<Property> properties;

   command="SEND_PROGRESS_UPDATE";	

   key="Progress type";
   value=progress_type;
   properties.push_back(Property(key,value));

   key="Progress percentage";
   value=stringfunc::number_to_string(curr_progress_frac*100);
   properties.push_back(Property(key,value));

//   cout << "Process percentage value = " << value << endl;
   broadcast_subpacket(command,properties);
}

void Messenger::broadcast_finished_progress(string task_name)
{
   broadcast_progress(1.0,task_name);
}

void Messenger::broadcast_clear_progress(string task_name)
{
   broadcast_progress(0.0,task_name);
}

// ---------------------------------------------------------------------
void Messenger::broadcast_current_framenumber(double curr_t)
{
//   cout << "inside Messenger::broadcast_current_framenumber()" << endl;

// Recall that ActiveMQ messages consist of a single command string
// along with an STL vector of key-value string pair properties:

   string command,key,value;
   vector<Messenger::Property> properties;

   command="SEND_CURRENT_FRAMENUMBER";	

   key="FrameNumber";
   value=stringfunc::number_to_string(curr_t);
   properties.push_back(Messenger::Property(key,value));

   broadcast_subpacket(command,properties);
}

// ==========================================================================
// Cancel operation member functions
// ==========================================================================

// Member function check_for_cancel_operation_message() listens for any 
// message whose text command = CANCEL.  If found, this method
// searches for a key-value pair where key="operation".  The
// corresponding value is stored within member string
// cancelled_operation.  

// We wrote this method in Nov 2010 to enable LOST thin client
// cancellation of expensive raytracing and skymap generation
// operations without having to use http get messages.  Since our
// OpenSceneGraph codes are not threaded and since they do NOT rely
// upon Qt, we use this ActiveMQ approach to instantaneously send and
// receive cancel commands within our C++ codes.

string Messenger::check_for_cancel_operation_message()
{
//    cout << "inside Messenger::check_for_cancel_operation_message()" << endl;

   int n_mailbox_messages=get_n_messages_in_mailbox();
//   cout << "n_mailbox_messages = " << n_mailbox_messages << endl;
   if (n_mailbox_messages==0) return cancelled_operation;


//   if (n_mailbox_messages > 1)
//   {
//      cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
//      cout << "get_n_messages_in_mailbox() = " 
//           << n_mailbox_messages << endl;
//      cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
//            outputfunc::enter_continue_char();
//   }

   if (copy_messages_and_purge_mailbox())
   {
//      cout << "n_curr_messages() from messenger = " 
//           << get_n_curr_messages() << endl;
      for (unsigned int i=0; i<get_n_curr_messages(); i++)
      {
         message* curr_message_ptr=get_message_ptr(i);
         if (curr_message_ptr != NULL)
         {
//            cout << "Message " << i << " : "
//                 << curr_message_ptr->get_text_message() << endl;
//            cout << "curr_message from messenger = " 
//                 << *curr_message_ptr << endl;

            if (curr_message_ptr->get_text_message()=="CANCEL")
            {
               curr_message_ptr->extract_and_store_property_keys_and_values();
               cancelled_operation=curr_message_ptr->
                  get_property_value("operation");
//               cout << "cancelled operation = " << cancelled_operation << endl;
//               outputfunc::enter_continue_char();
            }
            
         }
      } // loop over index i labeling received messages
   } // copy_messages_and_purge_mailbox conditional

   return cancelled_operation;
}



