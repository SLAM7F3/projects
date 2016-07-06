#include <QtGui>
#include <QtNetwork>

class Server :public QUdpSocket
{
    Q_OBJECT

public:
    Server( QObject* parent = 0 ) :QUdpSocket( parent ) {
        bind( QHostAddress::LocalHost, 4242 );
        connect( this, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()) );
    }

private slots:
    void readPendingDatagrams() {
        while ( hasPendingDatagrams()) {
            QByteArray datagram;
            datagram.resize( pendingDatagramSize() );
            QHostAddress sender;
            quint16 senderPort;
            readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
            QTextStream stream( datagram );
            QString str = stream.readAll();
            qDebug("Data from %s (port %d): %s", sender.toString().toLatin1().data(), senderPort, str.toLatin1().data());
        }
    }
};

int main( int argc, char** argv )
{
    QApplication app( argc, argv, false );
    new Server();
    return app.exec();
}

#include "main.moc"
