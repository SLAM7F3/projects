#include "server.h"
#include "connection.h"

Server::Server( QObject* parent ) : QTcpServer( parent )
{
    // TODO: Start listening on port 4242
    connect( this, SIGNAL( newConnection() ), this, SLOT( slotConnection() ) );
}

void Server::slotConnection()
{
    // TODO: handle the incomming connection
}
