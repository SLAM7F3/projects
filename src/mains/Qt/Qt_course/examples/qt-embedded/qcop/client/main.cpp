#include <qapplication.h>
#include <qvbox.h>
#include <qpushbutton.h>
#include <qcopchannel_qws.h>
#include <qmessagebox.h>

class Client :public QVBox
{
  Q_OBJECT
public:
  Client( QWidget* parent ) :QVBox( parent )
  {
    QPushButton* but1 = new QPushButton( "Execute", this );
    QPushButton* but2 = new QPushButton( "Delete", this );
    QPushButton* but3 = new QPushButton( "Unknown message", this );
    QPushButton* but4 = new QPushButton( "Unknown service", this );
    connect( but1, SIGNAL( clicked() ), this, SLOT( sendExecute() ) );
    connect( but2, SIGNAL( clicked() ), this, SLOT( sendDelete() ) );
    connect( but3, SIGNAL( clicked() ), this, SLOT( sendUnknownMessage() ) );
    connect( but4, SIGNAL( clicked() ), this, SLOT( sendUnknownService() ) );
  }

protected slots:
  void sendExecute()
  {
    QByteArray ba;
    QDataStream stream( ba, IO_WriteOnly );
    stream << QString("cat") << QString("file.txt");
    QCopChannel::send( "System/Shell", "execute(QString,QString)", ba );
  }

  void sendDelete()
  {
    QByteArray ba;
    QDataStream stream( ba, IO_WriteOnly );
    stream << QString("file.txt");
    QCopChannel::send( "System/Shell", "delete(QString)", ba );
  }

  void sendUnknownMessage()
  {
    QByteArray ba;
    QDataStream stream( ba, IO_WriteOnly );
    stream << QString("file.txt");
    QCopChannel::send( "System/Shell", "blah(QString)", ba );
  }

  void sendUnknownService()
  {
    if ( !QCopChannel::isRegistered( "ShellService") ) {
      QMessageBox::warning(this, "Unknown Service", "Service unknown: ShellService");
    }

    QByteArray ba;
    QDataStream stream( ba, IO_WriteOnly );
    stream << QString("file.txt");
    QCopChannel::send( "ShellService", "delete(QString)", ba );
  }
}
;

int main( int argc, char** argv )
{
  QApplication app( argc, argv, QApplication::GuiClient );

  Client* client = new Client(0);
  client->show();

  return app.exec();
}


#include "main.moc"
