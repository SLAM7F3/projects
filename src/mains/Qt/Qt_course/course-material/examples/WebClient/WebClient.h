// ========================================================================
// WEBCLIENT header file
// ========================================================================
// Last updated on 3/13/08
// ========================================================================

#ifndef WEBCLIENT_H
#define WEBCLIENT_H

#include <string>
#include <QtNetwork/QtNetwork>

class WebClient :public QObject
{
   Q_OBJECT

      public:

   WebClient(std::string host_IP, qint16 port, QObject* parent=NULL);

   private slots:

   void readData();
   void slotError( QAbstractSocket::SocketError );
   void slotConnected();
   void slotHostFound();

  private:

   QTcpSocket* socket_ptr;
   QHostAddress host_address;
   std::string host_IP_address;
   int port_number;

   void allocate_member_objects();
   void initialize_member_objects();

};

#endif // WEBCLIENT_H

