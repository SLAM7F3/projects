#include <QtGui>
#include <QTcpSocket>
#include "connection.h"

ConnectionWindow::ConnectionWindow( QTcpSocket* socket, QWidget* parent ) :QWidget( parent ),
                                                               _socket( socket )
{
    _in = new QTextEdit;
    _in->setReadOnly( true );

    _out = new QLineEdit;

    connect( _out, SIGNAL(returnPressed()), this, SLOT(slotSendLine()));
    // TODO: Set up signal/slot connection for the socket.

    // TODO: Close the window when the client is disconnected
    //       Tip: QWidget::close() is a slot.

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget( _in );
    layout->addWidget( _out );
    setLayout( layout );
}



void ConnectionWindow::slotRead()
{
    // TODO: Implement
}

void ConnectionWindow::slotSendLine()
{
    // TODO: Implement
}
