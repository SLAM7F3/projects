// ==========================================================================
// MESSAGESERVER class file
// ==========================================================================
// Last updated on 8/2/11; 2/26/12; 2/27/12
// ==========================================================================

#include <iostream>

// Include qapplication in order to access qApp macro which obviates
// having to pass app pointer from main Qt program...

#include <Qt/qapplication.h>
#include "Qt/web/MessageServer.h"

using std::cout;
using std::endl;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
void MessageServer::allocate_member_objects()
{
   Messenger_map_ptr=new MESSENGER_MAP;
}		       

void MessageServer::initialize_member_objects()
{
    broker_URL="tcp://127.0.0.1:61616";
    message_sender_ID="MessageServer";
}

MessageServer::MessageServer(
   string host_IP_address,qint16 port, QObject* parent) :
   BasicServer(host_IP_address,port,parent)
{
   allocate_member_objects();
   initialize_member_objects();
}

// ---------------------------------------------------------------------
MessageServer::~MessageServer()
{
   delete Messenger_map_ptr;
}

// ==========================================================================
// Thin client communications member functions
// ==========================================================================

// Member function generate_new_Messenger()

void MessageServer::generate_new_Messenger(string topic_name)
{
//   cout << "inside MessageServer::generate_new_Messenger()" << endl;
//   cout << "topic_name = " << topic_name << endl;

   Messenger* curr_Messenger_ptr=new Messenger( 
      broker_URL,topic_name,message_sender_ID);
   (*Messenger_map_ptr)[topic_name]=curr_Messenger_ptr;
}

bool MessageServer::destroy_Messenger(string topic_name)
{
//   cout << "inside MessageServer::destroy_Messenger()" << endl;

   MESSENGER_MAP::iterator iter=Messenger_map_ptr->find(topic_name);
   if (iter != Messenger_map_ptr->end()) 
   {
      Messenger* curr_Messenger_ptr=iter->second;
      delete curr_Messenger_ptr;
      Messenger_map_ptr->erase(iter);
      return true;
   }
   return false;
}

// ---------------------------------------------------------------------
// Member function update_messenger_topic_name()

bool MessageServer::update_messenger_topic_name(string& response_msg)
{
//   cout << "inside MessageServer::update_messenger_topic_name()" << endl;

   int n_args=KeyValue.size();
   string topic_name="";
   for (int k=0; k<n_args; k++)
   {
//      cout << "k = " << k 
//           << " KeyValue[k].first = " << KeyValue[k].first 
//           << " KeyValue[k].second = " << KeyValue[k].second
//           << endl;
      if (KeyValue[k].first=="TopicName")
      {
         topic_name=KeyValue[k].second;
      }
   } // loop over index k labeling KeyValue key possibilities
//   cout << "topic_name = " << topic_name << endl;
//   outputfunc::enter_continue_char();

// Check if topic name already exists within *Messenger_map_ptr.  If
// not, instantiate new Messenger and pair it with the new topic name:

   if (topic_name.size() < 1) return false;

   MESSENGER_MAP::iterator iter=Messenger_map_ptr->find(topic_name);
   if (iter==Messenger_map_ptr->end()) 
   {
      generate_new_Messenger(topic_name);
   }

   return true;
}

// ==========================================================================
// ActiveMQ Message handling member functions
// ==========================================================================

int MessageServer::get_n_Messengers() const
{
   return Messenger_map_ptr->size();
}

// ---------------------------------------------------------------------
Messenger* MessageServer::get_Messenger_ptr(string topic_name)
{
//   cout << "inside MessageServer::get_Messenger_ptr()" << endl;

   MESSENGER_MAP::iterator iter=Messenger_map_ptr->find(topic_name);
   if (iter != Messenger_map_ptr->end()) 
   {
      return iter->second;
   }
   else 
   {
      return NULL;
   }
}

const Messenger* MessageServer::get_Messenger_ptr(string topic_name) const
{
//   cout << "inside MessageServer::get_Messenger_ptr()" << endl;

   MESSENGER_MAP::iterator iter=Messenger_map_ptr->find(topic_name);
   if (iter != Messenger_map_ptr->end()) 
   {
      return iter->second;
   }
   else 
   {
      return NULL;
   }
}

// ---------------------------------------------------------------------
// Member function issue_message() is a high-level method which
// broadcasts an ActiveMQ message containing a command and single
// key-value pair

void MessageServer::issue_message(
   string command,string key,string value,string topic,string client_name)
{
   cout << "inside MessageServer::issue_message()" << endl;
//   cout << "command = " << command << endl;
//   cout << "key = " << key << endl;
//   cout << "value = " << value << endl;
//   cout << "topic = " << topic << endl;
//   cout << "client_name = " << client_name << endl;

   Messenger* Messenger_ptr=get_Messenger_ptr(topic);
//   cout << "Messenger_ptr = " << Messenger_ptr << endl;
   
   if (Messenger_ptr==NULL ||
       !Messenger_ptr->connected_to_broker_flag()) return;

// Recall that ActiveMQ messages consist of a single command string
// along with an STL vector of key-value string pair properties:

   typedef pair<string,string> property;
   vector<property> properties;

   if (key.size() > 0 || value.size() > 0)
      properties.push_back(property(key,value));

// If client_name is non_empty, temporarily reset message sender ID to
// client_name.  After sending the message, reset message sender ID to
// its original value.  We implemented this swapping in Feb 2012 for
// Timeline client messaging purposes...

   string orig_msg_sender_ID=Messenger_ptr->get_msg_sender_ID();
   if (client_name.size() > 0)
   {
      Messenger_ptr->set_msg_sender_ID(client_name);
   }
   Messenger_ptr->sendTextMessage(command,properties);

//   cout << "properties.size() = " << properties.size() << endl;
   Messenger_ptr->set_msg_sender_ID(orig_msg_sender_ID);

   cout << "at end of MessageServer::issue_message()" << endl;
}
