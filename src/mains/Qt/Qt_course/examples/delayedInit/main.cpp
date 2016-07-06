#include <QtGui>

class Sender :public QPushButton
{
    Q_OBJECT
public:
    Sender( QWidget* parent = 0 ) :QPushButton( "Press me", parent ), _counter(0) {
        connect( this, SIGNAL(clicked()), this, SLOT(emitData()) );
        QMetaObject::invokeMethod( this, "emitData", Qt::QueuedConnection );
        // Trying to do an emit here would not help, as the connection has not yet been
        // set up.
    }

signals:
    void data( int );

protected slots:
    void emitData()
    {
        emit data(_counter++);
    }

private:
    int _counter;
};

class Receiver :public QTextEdit
{
    Q_OBJECT
public:
    Receiver( QWidget* parent = 0 ) :QTextEdit( parent ) {};

public slots:
    void data(int i)
    {
        append( QString::number(i) );
    }
};


int main( int argc, char** argv )
{
    QApplication app(argc,argv);

    Sender* sender = new Sender;
    Receiver* receiver = new Receiver;

    QWidget* box = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout( box );
    layout->addWidget( sender );
    layout->addWidget( receiver );

    QObject::connect( sender, SIGNAL(data(int)), receiver, SLOT(data(int)) );

    box->show();
    return app.exec();
}


#include "main.moc"
