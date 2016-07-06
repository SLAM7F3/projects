#include<QtGui>
int main(int argc, char** argv)
{
    QApplication a(argc, argv);

    QCheckBox *cb1 = new QCheckBox( "b1" );
    QCheckBox *cb2 = new QCheckBox( "b2" );
    QCheckBox *cb3 = new QCheckBox( "b3" );


    QWidget* top = new QWidget();
    QHBoxLayout *lay = new QHBoxLayout( top );
    lay->addWidget( cb1 );
    lay->addWidget( cb2 );
    lay->addWidget( cb3 );

    cb2->hide();
    top->show();
    top->hide();
    top->show();

    a.exec();
}
