// ==========================================================================
// FLIRSERVER class file
// ==========================================================================
// Last updated on 10/24/11
// ==========================================================================

#include <iostream>
#include <set>
#include <vector>
#include <QtCore/QtCore>
#include "Qt/web/FLIRServer.h"
#include "general/stringfuncs.h"

using std::cout;
using std::endl;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
void FLIRServer::allocate_member_objects()
{
   listening_socket_ptr=new QTcpServer(this);
}		       

void FLIRServer::initialize_member_objects()
{
   Server_listening_flag=false;
   packet_size=0;
   instream_ptr=NULL;

   requestTimer.setInterval( 1000.0 );	// Timer goes off every 1000 ms = 1 sec
   requestTimer.start();
}

FLIRServer::FLIRServer(std::string host_IP, qint16 port, QObject* parent) 
   : QObject( parent )
{
   allocate_member_objects();
   initialize_member_objects();

// On 10/24/11, Ross Anderson taught us that server IP does NOT ever
// need to be passed in from a main program.

   port_number=port;
   cout << "port number = " << port_number << endl;
   
   setup_initial_signal_slot_connections();

   cout << "Server not yet listening" << endl;

// As of 6/26/08, we follow Ross Anderson's and Dave Ceddia's
// suggestion to set host_address to QHostAddress::Any so that Server
// can listen for client calls coming from any IP address and port:

   host_address=QHostAddress::Any;
   Server_listening_flag=listening_socket_ptr->listen(
      host_address,port_number);

   if (Server_listening_flag) 
      cout << "Server now listening" << endl;
}

// ---------------------------------------------------------------------
FLIRServer::~FLIRServer()
{
   listening_socket_ptr->close();
}

// ---------------------------------------------------------------------
// These first signal/slot relationships should be established BEFORE
// any network connections are made...

void FLIRServer::setup_initial_signal_slot_connections()
{
//   cout << "inside FLIRServer::setup_initial_signal_slot_connections()"
//        << endl;
   
   connect( listening_socket_ptr, SIGNAL( newConnection() ), 
            this, SLOT( incomingConnection() ) );
   connect( &requestTimer, SIGNAL(timeout()), this, SLOT(writeAllSockets()) );
}

// ---------------------------------------------------------------------
void FLIRServer::incomingConnection()
{
   cout << "inside FLIRServer::incomingConnection(), listening_socket_ptr = "
        << listening_socket_ptr << endl;
   
   if ( listening_socket_ptr == NULL ) return;
    
// Open a new, dedicated socket for the incoming connection and notify
// this upon available data:

   QTcpSocket* socket = listening_socket_ptr->nextPendingConnection();
   socket->setParent(this);
   
   connect( socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()) );
}

// ---------------------------------------------------------------------
// On 5/1/09, Dave Ceddia explained to us that a Server responds to a
// Client request by first establishing a bi-directional socket (which
// can be thought of as a doorway attached to the server; the address
// for this socket doorway is returned by sender() ).  Once the
// readyRead() signal is emitted by the new socket, the server reads
// the client's request in this member function readSocket().  The
// server can then respond to the request by writing information out
// to the socket for transmission back to the client.

void FLIRServer::writeAllSockets()
{
   QList<QTcpSocket*> children_list=findChildren<QTcpSocket*>();
   for (int i=0; i<children_list.size(); i++)
   {
      QTcpSocket* socket_ptr=children_list.at(i);
      writeCurrSocket(socket_ptr);
   }

//   iterate over all children , cast each to a socket , if not null then send them individuall to writecurrSocket

//   QTcpSocket* socket_ptr = qobject_cast<QTcpSocket *>( sender() );

}

// ---------------------------------------------------------------------
void FLIRServer::writeCurrSocket(QTcpSocket* socket_ptr)
{
   cout << "inside FLIRServer::writeCurrSocket()" << endl;

   if ( socket_ptr == NULL ) return;
//   cout << "socket_ptr = " << socket_ptr << endl;

// Write out packet to socket:

   QByteArray packet;
   QDataStream outstream(&packet,QIODevice::WriteOnly);
   outstream.setVersion( QDataStream::Qt_4_1 );
        
// Magic number indicates start of packet:

//   outstream << quint32(0xABCD);
   outstream << quint32(666);

// Packet length placeholder

   outstream << quint32(0);
   
// Header values for iPhone camera pictures:

   QString     phrase1("Hello ");
   QString     phrase2("World ! ");
   QString     tot_string=phrase1+phrase2;

   outstream << tot_string;

// Seek to beginning of Datastream.  Then overwrite second 4 bytes
// with genuine packet reduced size (which does not count the first 4
// bytes containing the magic number nor the second 4 bytes containing
// the packet length):

   outstream.device()->seek(4);
   quint32 reduced_packet_size=packet.size()-2*sizeof(quint32);

   outstream << reduced_packet_size;

   socket_ptr->write(packet);
}
