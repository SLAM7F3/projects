#include <QtGui>
#include <QtNetwork>

int main( int argc, char** argv )
{
    QApplication app( argc, argv );

    {
        QUdpSocket socket;
        QByteArray data;
        QTextStream stream( &data );
        stream << "Hello world, here comes a UDP package" << flush;
        socket.writeDatagram( data, QHostAddress("127.0.0.1"), 4242 );
    }

    {
        QUdpSocket socket;
        socket.connectToHost( QHostAddress("127.0.0.1"), 4242 );
        QTextStream stream( &socket );
        stream << "Hello World, here comes a second UDP package" << flush;
    }
}
