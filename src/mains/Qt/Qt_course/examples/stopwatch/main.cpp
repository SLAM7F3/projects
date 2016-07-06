#include <QtGui>

class StopWatch :public QLabel
{
    Q_OBJECT

public:
    StopWatch( QWidget* parent = 0 ) : QLabel( parent )
    {
        QTimer* timer = new QTimer( this );
        timer->start( 1000 );
        connect( timer, SIGNAL( timeout() ), this, SLOT( shot() ) );
        _secs = 0;
        setText("0:00");
        setAlignment( Qt::AlignCenter );
    }

protected slots:
    void shot()
    {
        _secs += 1;
        setText( QString().sprintf("%d:%02d", _secs / 60,  _secs %60) );
    }

private:
    int _secs;
};

int main(int argc, char** argv)
{
    QApplication app( argc, argv );
    StopWatch* watch = new StopWatch;

    watch->show();
    return app.exec();
}

#include "main.moc"
