// ========================================================================
// Header file for MESSAGESERVER class which 
// ========================================================================
// Last updated on 8/2/11; 2/26/12
// ========================================================================

#ifndef __MESSAGESERVER_H__
#define __MESSAGESERVER_H__

#include <map>
#include <vector>
#include <QtXml/QtXml>
#include <QtNetwork/QHttp>

#include "Qt/web/BasicServer.h"
#include "messenger/Messenger.h"

class MessageServer : public BasicServer
{

   typedef std::map<std::string,Messenger*> MESSENGER_MAP;

   Q_OBJECT
    
      public:

// Note: Avoid using port 8080 which is the default for TOMCAT.

   MessageServer(
      std::string host_IP_address, qint16 port,QObject* parent=NULL);
   ~MessageServer();

// Set & get member functions:

// ActiveMQ message handling member functions:

   int get_n_Messengers() const;
   Messenger* get_Messenger_ptr(std::string topic_name);
   const Messenger* get_Messenger_ptr(std::string topic_name) const;

// Thin client communications member functions:

   void generate_new_Messenger(std::string topic_name);
   bool destroy_Messenger(std::string topic_name);
   bool update_messenger_topic_name(std::string& response_msg);

   void issue_message(
      std::string command,std::string key,std::string value,
      std::string topic,std::string client_name);

   protected slots:
        
  protected:


// HTTP processing member functions:

  private:

   string broker_URL,message_sender_ID;
   MESSENGER_MAP* Messenger_map_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

#endif // __MESSAGESERVER_H__
