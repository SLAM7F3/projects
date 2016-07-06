#include <QtGui>
#include "listener.h"

Listener::Listener( QWidget* parent ) :QTextEdit( parent )
{
    _socket = new QTcpSocket( this );
    connect( _socket, SIGNAL( error( QAbstractSocket::SocketError ) ), 
             this, SLOT( slotError( QAbstractSocket::SocketError ) ) );

    connect( _socket, SIGNAL( connected() ), this, SLOT( slotConnected() ) );

    connect( _socket, SIGNAL( hostFound() ), this, SLOT( slotHostFound() ) );

    qDebug() << "Connecting to host...";

    _socket->connectToHost("www.klaralvdalens-datakonsult.se",80);


    connect( _socket, SIGNAL(readyRead()), this, SLOT(readData()) );
}

void Listener::readData()
{
    QTextStream stream( _socket );
    QString txt = stream.readAll();
    append( txt );
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
    stream << "GET http://www.klaralvdalens-datakonsult.se/ HTTP/1.0\r\n\r\n" << flush;
}

void Listener::slotHostFound()
{
    qDebug("Host found");
}
