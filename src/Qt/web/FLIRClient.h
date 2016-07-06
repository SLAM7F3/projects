// ========================================================================
// FLIRCLIENT header file
// ========================================================================
// Last updated on 10/20/11; 10/26/11
// ========================================================================

#ifndef FLIRCLIENT_H
#define FLIRCLIENT_H

#include <string>
#include <QtNetwork/QtNetwork>
#include <QtCore/QSignalMapper>

class FLIRClient : public QObject
{
   Q_OBJECT

      public:

   FLIRClient(
      std::string Server_hostname,int Server_portnumber,QObject* parent=NULL);
   ~FLIRClient();

// Set & get member functions

   std::string get_message_string();

  protected:

   private slots:

   void slotError( QAbstractSocket::SocketError );
   void slotHostFound();
   void slotConnected();
   void readSocket();

  private:

   std::string message_string;
   quint32 packet_size;	// packet header data

   QTcpSocket* socket_ptr;
   QDataStream* instream_ptr;

   void allocate_member_objects();
   void initialize_member_objects();

   void process_Qt_events(int n_iters);

   void decompose_message_string(std::string message);
};



// ==========================================================================
// Inlined methods:
// ==========================================================================

inline std::string FLIRClient::get_message_string()
{
   return message_string;
}


#endif // FLIRCLIENT_H

