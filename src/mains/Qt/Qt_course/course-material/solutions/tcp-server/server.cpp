#include "server.h"
#include "connection.h"

Server::Server( QObject* parent ) : QTcpServer( parent )
{
    listen( QHostAddress::LocalHost, 4242 );
    connect( this, SIGNAL( newConnection() ), this, SLOT( slotConnection() ) );
}

void Server::slotConnection()
{
    while ( hasPendingConnections() ) {
        QTcpSocket* socket = nextPendingConnection();
        ConnectionWindow* con = new ConnectionWindow( socket, 0 );
        // Don't leak memory: ensure that the window is deleted when it's closed.
        con->setAttribute( Qt::WA_DeleteOnClose );
        con->show();
    }
}
