#include <QtGui>

class Data
{
public:
    Data() : _i(-1), _p(0), _s() {}
    Data( int i, int* p, const QString& s ) : _i(i), _p(p), _s(s)
    {
    }

    int _i;
    int* _p;
    QString _s;
};
Q_DECLARE_TYPEINFO( Data, Q_MOVABLE_TYPE );

class Sender :public QPushButton
{
    Q_OBJECT
public:
    Sender( QWidget* parent = 0 ) : QPushButton( "Push me", parent )
    {
        connect( this, SIGNAL( clicked() ), this, SLOT( sendData() ) );
        _i = 42;
    }

signals:
    void data( const Data& );

protected slots:
    void sendData()
    {
        Data d( 10, &_i, "hej" );
        emit data( d );
    }

private:
    int _i;
};


class Receiver : public QObject {
    Q_OBJECT

public:
    Receiver( QObject* parent = 0) :QObject( parent )
    {
    }

public slots:
    void data( const Data& data )
    {
        qDebug() << "Receiver says: " << data._i << *data._p << data._s;
    }
};

class MyThread :public QThread
{
    Q_OBJECT

public:
    MyThread( Sender* s )
        : _s( s )
    {
        // Anything created here is on the thread creating the MyThread instance, not on the MyThread thread!
    }

    void run()
    {
        Receiver* r = new Receiver;
        QObject::connect( _s, SIGNAL( data( const Data& ) ), r, SLOT( data( const Data& ) ), Qt::QueuedConnection  );
        exec();
    }

private:
    Sender* _s;
};

int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    qRegisterMetaType<Data>("Data");

    Sender* sender = new Sender;

    MyThread* t = new MyThread( sender );
    t->start();

    sender->show();
    return app.exec();
}

#include "main.moc"
