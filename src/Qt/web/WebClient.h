// ========================================================================
// WEBCLIENT header file
// ========================================================================
// Last updated on 3/18/08; 3/24/08; 4/29/08
// ========================================================================

#ifndef WEBCLIENT_H
#define WEBCLIENT_H

#include <string>
#include <QtNetwork/QtNetwork>
#include <QtCore/QSignalMapper>
 
class WebClient : public QObject
{
   Q_OBJECT

      public:

   WebClient(std::string host_IP, qint16 port, QObject* parent=NULL);

// Set & get member functions

   void set_GET_command(std::string cmd);
   std::string& get_returned_output();
   bool returned_output_contains_substring(std::string substring);

  protected:

   private slots:

   void slotError( QAbstractSocket::SocketError );
   void slotConnected();
   void slotHostFound();
   void readData();

  private:

   QTcpSocket* socket_ptr;
   QHostAddress host_address;

   std::string host_IP_address;
   int port_number;

   std::string GET_command,returned_output;

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void WebClient::set_GET_command(std::string cmd)
{
   GET_command=cmd;
}

#endif // WEBCLIENT_H

