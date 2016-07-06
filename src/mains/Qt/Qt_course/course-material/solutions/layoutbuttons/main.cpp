#include<QtGui>

// This is how you would make the buttons take up the available space
// vertically: by changing their size policy.
class MyButton :public QPushButton
{
public:
    MyButton( QString label, QWidget *parent = 0 ) : QPushButton( label, parent )
    {
        QSizePolicy::Policy horizPolicy = QPushButton::sizePolicy().horizontalPolicy(); // preserve horizontal policy
        setSizePolicy( QSizePolicy( horizPolicy, QSizePolicy::MinimumExpanding ) );     // set vertical policy to MinimumExpanding
    }
};

int main( int argc, char* argv[] )
{
    QApplication a( argc, argv );
    QWidget *top = new QWidget;
    QVBoxLayout* topLayout = new QVBoxLayout( top );

    // Without any modifications
    {
        MyButton* a = new MyButton( "But A" );
        MyButton* b = new MyButton( "But B" );
        MyButton* c = new MyButton( "But C" );

        QWidget* w = new QWidget;
        topLayout->addWidget( w );

        QHBoxLayout *layout = new QHBoxLayout( w );
        layout->addWidget( a );
        layout->addWidget( b );
        layout->addWidget( c );
    }


    // Exercise 1
    {
        MyButton* a = new MyButton( "But A" );
        MyButton* b = new MyButton( "But B" );
        MyButton* c = new MyButton( "But C" );

        QWidget* w = new QWidget;
        topLayout->addWidget( w );

        QHBoxLayout *layout = new QHBoxLayout( w );
        layout->addWidget( a, 2 );
        layout->addWidget( b, 1 );
        layout->addWidget( c, 1 );
    }

    // Exercise 2
    {
        MyButton* a = new MyButton( "But A" );
        MyButton* b = new MyButton( "But B" );
        MyButton* c = new MyButton( "But C" );

        QWidget* w = new QWidget;
        topLayout->addWidget( w );

        QHBoxLayout *layout = new QHBoxLayout( w );
        layout->addWidget( a, 1 );
        layout->addWidget( b );
        layout->addWidget( c );
    }

    // Exercise 3
    {
        MyButton* a = new MyButton( "But A" );
        MyButton* b = new MyButton( "But B" );
        MyButton* c = new MyButton( "But C" );

        QWidget* w = new QWidget;
        topLayout->addWidget( w );

        QHBoxLayout *layout = new QHBoxLayout( w );
        layout->addWidget( a );
        layout->addWidget( b );
        layout->addWidget( c );
        layout->addStretch( 1 );
    }

    // Exercise 4
    {
        MyButton* a = new MyButton( "But A" );
        MyButton* b = new MyButton( "But B" );
        MyButton* c = new MyButton( "But C" );

        QWidget* w = new QWidget;
        topLayout->addWidget( w );

        QHBoxLayout *layout = new QHBoxLayout( w );
        layout->addWidget( a );
        layout->addStretch( 1 );
        layout->addWidget( b );
        layout->addStretch( 1 );
        layout->addWidget( c );
    }

    top->resize(600,400);
    top->show();

    return a.exec();
}

