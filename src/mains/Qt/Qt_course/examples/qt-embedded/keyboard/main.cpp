#include <qapplication.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qpushbutton.h>
#include <qsignalmapper.h>
#include <qwindowsystem_qws.h>
#include <qmultilineedit.h>

class SimpleKeyBoard :public QVBox
{
Q_OBJECT

public:
  SimpleKeyBoard( QWidget* parent ) :QVBox( parent,"SimpleKeyBoard", WStyle_Customize | WStyle_Tool | WStyle_StaysOnTop)
  {
    QSignalMapper* mapper = new QSignalMapper( this );
    connect( mapper, SIGNAL(mapped(const QString&)), this, SLOT(emitKey(const QString&)) );

    QStringList list1,list2,list3;
    list1 << "q" << "w" << "e" << "r" << "t" << "y" << "u" << "i" << "o" << "p";
    list2  << "a" << "s" << "d" << "f" << "g" << "h" << "j" << "k" << "l" << "-";
    list3  << "z" << "x" << "c" << "v" << "b" << "n" << "m" << "m" << "," << ".";

    mapList( list1, mapper );
    mapList( list2, mapper );
    mapList( list3, mapper );
  }

  void mapList( const QStringList& list, QSignalMapper* mapper )
  {
    QHBox* row = new QHBox( this );

    for ( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it ) {
      QPushButton* but = new QPushButton( *it, row );
      mapper->setMapping( but, *it );
      connect( but, SIGNAL(clicked()), mapper, SLOT(map()) );
    }
  }

protected slots:
  void emitKey( const QString& key )
  {
    int ucode = QChar( key[0] ).unicode();
    QWSServer::sendKeyEvent( ucode, 0, 0, true, false );
  }
};


int main(int argc, char** argv)
{
  QApplication app( argc, argv, QApplication::GuiServer );

  QMultiLineEdit* editor = new QMultiLineEdit(0);
  editor->show();
  editor->showMaximized();

  SimpleKeyBoard* keyboard = new SimpleKeyBoard(0);
  keyboard->show();

  // size and move the widgets.
  QWidget* desktop = app.desktop();
  int height = keyboard->sizeHint().height();
  int restHeight = desktop->size().height()-height;
  int width = desktop->size().width();
  keyboard->resize( width,height );
  keyboard->move(0, restHeight );
  qwsServer->setMaxWindowRect( QRect(0,0, width, restHeight) );

  return app.exec();
}


#include "main.moc"
