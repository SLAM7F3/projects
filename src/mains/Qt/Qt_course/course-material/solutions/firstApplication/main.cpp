#include <QtGui>

int main(int argc, char** argv)
{
  QApplication app( argc, argv );

  QLabel* nameLabel = new QLabel( "Name:\t" );
  QLineEdit* nameEdit = new QLineEdit;
  nameEdit->setText("Jay K. Hacker");

  QLabel* streetLabel = new QLabel( "Street:\t" );
  QLineEdit* streetEdit = new QLineEdit;
  streetEdit->setText( "Santa Claus Street 24" );

  QLabel* countryLabel = new QLabel( "Country:");
  countryLabel->setAlignment( Qt::AlignCenter );

  QListWidget* countries = new QListWidget;
  QStringList items;
  items << "Denmark"
        << "Greenland"
        << "Sweden"
        << "Norway"
        << "Finland"
        << "France"
        << "Germany"
        << "Italy"
        << "Rusia"
        << "Estonia";
  countries->addItems( items );

  QLabel* commentLabel = new QLabel( "Comments:" );
  commentLabel->setAlignment( Qt::AlignCenter );

  QTextEdit* edit = new QTextEdit;
  edit->document()->setPlainText( "A programmer known world-wide" );

  QPushButton* ok = new QPushButton( "OK" );
  QPushButton* cancel = new QPushButton( "Cancel" );

  // Layout Managers
  QHBoxLayout* nameLayout = new QHBoxLayout;
  nameLayout->addWidget( nameLabel );
  nameLayout->addWidget( nameEdit );

  QHBoxLayout* streetLayout = new QHBoxLayout;
  streetLayout->addWidget( streetLabel );
  streetLayout->addWidget( streetEdit );

  QHBoxLayout* buttonLayout = new QHBoxLayout;
  buttonLayout->addWidget( ok );
  buttonLayout->addWidget( cancel );

  QVBoxLayout* layout = new QVBoxLayout;
  layout->addLayout( nameLayout );
  layout->addLayout( streetLayout );
  layout->addWidget( countryLabel );
  layout->addWidget( countries );
  layout->addWidget( commentLabel );
  layout->addWidget( edit );
  layout->addLayout( buttonLayout );

  QWidget* top = new QWidget;
  top->setLayout( layout );


  top->show();
  return app.exec();
}
