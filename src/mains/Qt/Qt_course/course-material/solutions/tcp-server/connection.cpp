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
    connect( _socket, SIGNAL(readyRead()), this, SLOT(slotRead()) );
    connect( _socket, SIGNAL(disconnected()), this, SLOT(close()));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget( _in );
    layout->addWidget( _out );
    setLayout( layout );

    QTextStream stream( _socket );
    stream << "Hello World\n";
}



void ConnectionWindow::slotRead()
{
    while ( _socket->canReadLine() ) {
        QString line = _socket->readLine();
        line = line.mid(0,line.length()-2);
        _in->append ( line );
    }
}

void ConnectionWindow::slotSendLine()
{
    QTextStream stream( _socket );
    QString txt = _out->text();
    stream << txt << endl;
    _out->clear();
}
