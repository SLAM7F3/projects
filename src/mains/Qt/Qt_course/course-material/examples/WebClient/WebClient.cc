// ==========================================================================
// WEBCLIENT class file
// ==========================================================================
// Last updated on 3/13/08
// ==========================================================================

#include <iostream>
#include "WebClient.h"

using std::cout;
using std::endl;

// ---------------------------------------------------------------------
void WebClient::allocate_member_objects()
{
   socket_ptr = new QTcpSocket(this);
}		       

void WebClient::initialize_member_objects()
{
}

WebClient::WebClient(std::string host_IP,qint16 port,QObject* parent) 
   : QObject( parent )
{
   allocate_member_objects();
   initialize_member_objects();

   host_IP_address=host_IP;
   host_address=QHostAddress(host_IP_address.c_str());
   port_number=port;

   cout << "host IP address = " << this->host_IP_address << endl;
   cout << "port number = " << port_number << endl;

   connect( socket_ptr, SIGNAL( error( QAbstractSocket::SocketError ) ), 
            this, SLOT( slotError( QAbstractSocket::SocketError ) ) );
   connect( socket_ptr, SIGNAL( connected() ), 
            this, SLOT( slotConnected() ) );
   connect( socket_ptr, SIGNAL( hostFound() ), 
            this, SLOT( slotHostFound() ) );

   qDebug() << "Connecting to host...";
   socket_ptr->connectToHost(host_address,port_number);

   connect( socket_ptr, SIGNAL(readyRead()), this, SLOT(readData()) );
}


// ---------------------------------------------------------------------
void WebClient::readData()
{
   qDebug() << "inside WebClient::readData()";
   
   QTextStream stream( socket_ptr );
   QString txt = stream.readAll();
   cout << "txt = " << txt.toStdString() << endl;
}

// ---------------------------------------------------------------------
void WebClient::slotError( QAbstractSocket::SocketError err )
{
   qDebug() << "Error:" << err << socket_ptr->errorString();
}

// ---------------------------------------------------------------------
void WebClient::slotConnected()
{
   qDebug( "Connected, sending request..." );
   QTextStream stream( socket_ptr );
   stream << "GET http://155.34.162.78:4040/global/?sam_type=Unknown&country=South+Korea&ioc=1950&range=5-10&command=getSamInfo HTTP/1.0\r\n\r\n" << flush;
}

// ---------------------------------------------------------------------
void WebClient::slotHostFound()
{
   qDebug("Host found");
}
