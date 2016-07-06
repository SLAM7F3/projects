// ========================================================================
// FLIRSERVER header file
// ========================================================================
// Last updated on 10/24/11
// ========================================================================

#ifndef FLIRSERVER_H
#define FLIRSERVER_H

#include <stdint.h>
#include <string>
#include <QtNetwork/QtNetwork>
#include <QtXml/QtXml>

class FLIRServer : public QObject
{
   Q_OBJECT
    
      public:

// Note: Avoid using port 8080 which is the default for TOMCAT.

   FLIRServer(std::string host_IP, qint16 port, QObject* parent = NULL );
   ~FLIRServer();

// Set & get member functions:

  protected:

   private slots:
        
   void incomingConnection();
   void writeAllSockets();
        
  private:
        
   bool Server_listening_flag;
   uint32_t packet_size;
   int port_number,image_number;
   std::string host_IP_address;

   QTimer requestTimer;
   QTcpServer* listening_socket_ptr;
   QHostAddress host_address;
   QDataStream* instream_ptr;

   void allocate_member_objects();
   void initialize_member_objects();

   void setup_initial_signal_slot_connections();
   void writeCurrSocket(QTcpSocket* socket_ptr);
   
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

#endif // FLIRSERVER_H
