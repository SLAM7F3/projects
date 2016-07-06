#include <iostream>
#include <QtGui>
#include "listener.h"

using std::cout;
using std::endl;

Listener::Listener( QWidget* parent ) :QTextEdit( parent )
{
   _socket = new QTcpSocket( this );

   connect( _socket, SIGNAL( error( QAbstractSocket::SocketError ) ), this, SLOT( slotError( QAbstractSocket::SocketError ) ) );


   connect( _socket, SIGNAL( connected() ), this, SLOT( slotConnected() ) );
   connect( _socket, SIGNAL( hostFound() ), this, SLOT( slotHostFound() ) );

   qDebug() << "Connecting to host...";
//    _socket->connectToHost("www.klaralvdalens-datakonsult.se",80);
   _socket->connectToHost("155.34.162.78",4040);

   connect( _socket, SIGNAL(readyRead()), this, SLOT(readData()) );
}

void Listener::readData()
{
   qDebug() << "inside Listener::readData()";
   
   QTextStream stream( _socket );
   QString txt = stream.readAll();
   cout << "txt = " << txt.toStdString() << endl;

   append( txt );

   qDebug() << "at end of Listener::readData()";
}

void Listener::slotError( QAbstractSocket::SocketError err )
{
   qDebug() << "Error:" << err << _socket->errorString();
}

void Listener::slotConnected()
{
   qDebug( "Connected, sending request..." );
   QTextStream stream( _socket );
   stream.setCodec( QTextCodec::codecForName( "latin1") );
   stream << "GET http://155.34.162.78:4040/global/?sam_type=Unknown&country=South+Korea&ioc=1950&range=5-10&command=getSamInfo HTTP/1.0\r\n\r\n" << flush;
//   stream << "GET http://www.klaralvdalens-datakonsult.se/ HTTP/1.0\r\n\r\n" << flush;

   qDebug( "At end of Listener::slotConnected()" );
}

void Listener::slotHostFound()
{
   qDebug("Host found");
}
