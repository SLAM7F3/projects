#include<QtGui>

int main( int argc, char* argv[] )
{
   QApplication a( argc, argv );
   QWidget *top = new QWidget;
   QVBoxLayout* topLayout = new QVBoxLayout( top );

   // Without any modifications
   {
      QPushButton* a = new QPushButton( "But A" );
      QPushButton* b = new QPushButton( "But B" );
      QPushButton* c = new QPushButton( "But C" );

      QWidget* w = new QWidget;
      topLayout->addWidget( w );

      QHBoxLayout *layout = new QHBoxLayout( w );
      layout->addWidget( a );
      layout->addWidget( b );
      layout->addWidget( c );
   }


   // Exercise 1
   {
      QPushButton* a = new QPushButton( "But A" );
      QPushButton* b = new QPushButton( "But B" );
      QPushButton* c = new QPushButton( "But C" );

      QWidget* w = new QWidget;
      topLayout->addWidget( w );

      QHBoxLayout *layout = new QHBoxLayout( w );
      layout->addWidget( a,2 );
      layout->addWidget( b,1 );
      layout->addWidget( c,1 );
   }

   // Exercise 2
   {
      QPushButton* a = new QPushButton( "But A" );
      QPushButton* b = new QPushButton( "But B" );
      QPushButton* c = new QPushButton( "But C" );

      QWidget* w = new QWidget;
      topLayout->addWidget( w );

      QHBoxLayout *layout = new QHBoxLayout( w );
      layout->addWidget( a,2 );
      layout->addWidget( b,0 );
      layout->addWidget( c,0 );
   }

   // Exercise 3
   {
      QPushButton* a = new QPushButton( "But A" );
      QPushButton* b = new QPushButton( "But B" );
      QPushButton* c = new QPushButton( "But C" );

      QWidget* w = new QWidget;
      topLayout->addWidget( w );

      QHBoxLayout *layout = new QHBoxLayout( w );
      layout->addWidget( a,0 );
      layout->addWidget( b,0 );
      layout->addWidget( c,0 );
      layout->addStretch();
   }


   // Exercise 4
   {
      QPushButton* a = new QPushButton( "But A" );
      QPushButton* b = new QPushButton( "But B" );
      QPushButton* c = new QPushButton( "But C" );

      QWidget* w = new QWidget;
      topLayout->addWidget( w );

      QHBoxLayout *layout = new QHBoxLayout( w );
      layout->addWidget( a,0 );
      layout->addStretch();
      layout->addWidget( b,0 );
      layout->addStretch();
      layout->addWidget( c,0 );
   }

   top->resize(600,400);
   top->show();

   return a.exec();
}

