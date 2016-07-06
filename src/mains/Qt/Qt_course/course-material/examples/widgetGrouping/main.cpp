#include <QtGui>

int main( int argc, char** argv)
{
  QApplication app(argc, argv);

  QLabel* nameLabel = new QLabel( "Username: " );
  QLineEdit* nameEdit = new QLineEdit;

  QLabel* passwordLabel = new QLabel( "Password: " );
  QLineEdit* passwordEdit = new QLineEdit;

  QHBoxLayout* nameLay = new QHBoxLayout;
  nameLay->addWidget( nameLabel );
  nameLay->addWidget( nameEdit );

  QHBoxLayout* passwordLay = new QHBoxLayout;
  passwordLay->addWidget( passwordLabel );
  passwordLay->addWidget( passwordEdit );

  QVBoxLayout* topLay = new QVBoxLayout;
  topLay->addLayout( nameLay );
  topLay->addLayout( passwordLay );

  QWidget* top = new QWidget;
  top->setLayout( topLay );

  top->show();
  return app.exec();
}



