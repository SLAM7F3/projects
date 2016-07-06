#include <qapplication.h>
#include <qcopchannel_qws.h>
#include <qdatastream.h>
#include <qstring.h>
#include <qmultilineedit.h>
class Shell :public QMultiLineEdit
{
  Q_OBJECT
public:
  Shell( QWidget* parent ) :QMultiLineEdit( parent, "edit" )
  {
    QCopChannel* chanel = new QCopChannel( "System/Shell", this,"chanel" );
    connect( chanel, SIGNAL(received( const QCString &, const QByteArray & ) ),
             this, SLOT( process(const QCString &, const QByteArray &) ));
  }

protected slots:
  void process(const QCString &msg, const QByteArray &data)
  {
    QDataStream stream( data, IO_ReadOnly );
    if ( msg == "execute(QString,QString)" ) {
      QString cmd, arg;
      stream >> cmd >> arg;
      append( QString("execute(") + cmd + "," + arg + ")\n");
    }
    else if ( msg == "delete(QString)" ) {
      QString filename;
      stream >> filename;
      append( QString("delete(")+ filename + ")");
    }
    else {
      append(QString("Wrong message: ") + QString(msg));
    }
  }
};


int main( int argc, char** argv )
{
  QApplication app( argc, argv, QApplication::GuiServer );
  Shell* shell1 = new Shell(0);

  shell1->show();
  return app.exec();
}


#include "main.moc"
