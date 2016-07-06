#include <QtGui>

class HelpFilter :public QObject {
public:
  HelpFilter( QStatusBar* bar, const QString& txt )
  {
    _bar = bar;
    _txt = txt;
  }

private:
  QStatusBar* _bar;
  QString _txt;

protected:
  bool eventFilter( QObject* /*receiver*/, QEvent* event ) {
    if ( event->type() == QEvent::Enter ) {
      _bar->showMessage( _txt, 5000 );
    }
    else if ( event->type() == QEvent::Leave ) {
      _bar->clearMessage( );
    }

    return false;
  }
};


int main( int argc, char**argv)
{
  QApplication app(argc, argv);

  QMainWindow* win = new QMainWindow();

  QCheckBox* box = new QCheckBox( "CheckBox" );
  HelpFilter *boxFilter = new HelpFilter( win->statusBar(), "Text for checkbox" );
  box->installEventFilter( boxFilter );

  QLineEdit* edit= new QLineEdit;
  HelpFilter *editFilter = new HelpFilter( win->statusBar(), "Text for lineEdit" );
  edit->installEventFilter( editFilter );

  QWidget* top = new QWidget;
  win->setCentralWidget( top );

  QVBoxLayout* layout = new QVBoxLayout( top );
  layout->addWidget( box );
  layout->addWidget( edit );

  win->show();

  return app.exec();
}
