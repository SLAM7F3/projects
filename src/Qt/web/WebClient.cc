// ==========================================================================
// WEBCLIENT class file
// ==========================================================================
// Last updated on 3/18/08; 3/24/08; 4/28/09
// ==========================================================================

#include <iostream>
#include <QtXml/QtXml>
#include <QtNetwork/QtNetwork>
#include "general/stringfuncs.h"
#include "Qt/web/WebClient.h"

using std::cout;
using std::endl;
using std::string;

// ---------------------------------------------------------------------
void WebClient::allocate_member_objects()
{
   socket_ptr = new QTcpSocket(this);
}		       

void WebClient::initialize_member_objects()
{
   returned_output="";
}

WebClient::WebClient(string host_IP,qint16 port,QObject* parent) 
   : QObject( parent )
{
//   cout << "inside WebClient constructor()" << endl;
   allocate_member_objects();
   initialize_member_objects();

   host_IP_address=host_IP;
   host_address=QHostAddress(host_IP_address.c_str());
   port_number=port;
//   cout << "host IP address = " << this->host_IP_address << endl;
//   cout << "port number = " << port_number << endl;

   connect( socket_ptr, SIGNAL( error( QAbstractSocket::SocketError ) ), 
            this, SLOT( slotError( QAbstractSocket::SocketError ) ) );
   connect( socket_ptr, SIGNAL( connected() ), 
            this, SLOT( slotConnected() ) );
   connect( socket_ptr, SIGNAL( hostFound() ), 
            this, SLOT( slotHostFound() ) );

//   qDebug() << "Connecting to host...";
   socket_ptr->connectToHost(host_address,port_number);

   connect( socket_ptr, SIGNAL(readyRead()), this, SLOT(readData()) );
}

// ---------------------------------------------------------------------
void WebClient::slotError( QAbstractSocket::SocketError err )
{
   cout << "inside WebClient::slotError()" << endl;
   qDebug() << "Error:" << err << socket_ptr->errorString();
}

// ---------------------------------------------------------------------
void WebClient::slotConnected()
{
//   cout << "inside WebClient::slotConnected()" << endl;
   QTextStream stream( socket_ptr );
   stream << QString(GET_command.c_str()) << flush;
}

// ---------------------------------------------------------------------
void WebClient::slotHostFound()
{
//   cout << "inside WebClient::slotHostFound()" << endl;
//   qDebug("Host found");
}

// ---------------------------------------------------------------------
void WebClient::readData()
{
//   qDebug() << "inside WebClient::readData()";
   QTextStream outstream( socket_ptr );
   returned_output += outstream.readAll().toStdString();
//   cout << "returned_output.size() = " << returned_output.size() << endl;
//   cout << "Returned output = " << returned_output << endl;
}

// ---------------------------------------------------------------------
bool WebClient::returned_output_contains_substring(string substring)
{
//   cout << "inside WebClient::returned_output_contains_substring()" << endl;
   int posn=stringfunc::first_substring_location(returned_output,substring);
//   cout << "posn = " << posn << endl;
   return (posn >= 0);
}

// ---------------------------------------------------------------------
string& WebClient::get_returned_output()
{

// As of Friday, Mar 14 at 12:40 pm, we instantiate a new WebClient
// for each individual GET call.  Qt should delete the dynamically
// allocated WebClient object once the data has been read out from the
// current WebClient object:

//   this->deleteLater();
   return returned_output;
}
