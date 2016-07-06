#include <QtGui>

class MasterBut :public QPushButton
{
public:
    MasterBut( QString txt, QPushButton* other, QWidget* parent = 0 )
        : QPushButton( txt, parent), _other(other) {}
protected:
    virtual bool event ( QEvent * e )
    {
        if (  QEvent::MouseButtonPress <= e->type() && e->type() <= QEvent::KeyRelease )
            qApp->sendEvent( _other, e );
        return QPushButton::event( e );
    }
private:
    QPushButton* _other;
};


int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QPushButton* slave = new QPushButton( "Slave" );
    MasterBut* master = new MasterBut( "Master", slave );

    QWidget* top = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout( top );
    layout->addWidget( master );
    layout->addWidget( slave );
    top->show();

    return app.exec();
}


