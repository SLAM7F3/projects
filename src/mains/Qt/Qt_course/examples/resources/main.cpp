#include <QtGui>

class ImageButton :public QPushButton
{
    Q_OBJECT

public:
    ImageButton( const QString& name, const QString& imageFile, const QString& soundFile, QWidget* parent = 0 )
        :QPushButton( parent ), _soundFile( soundFile ), _name( name )
    {
        _pixmap = QPixmap( imageFile );
        setIcon( _pixmap );
        connect( this, SIGNAL( clicked() ), this, SLOT(sayName() ) );
        int h = QFontMetrics( font() ).height();
        setFixedSize( _pixmap.size() + QSize( 6,6 + h + 2 ) );
    }

protected:
    virtual void paintEvent( QPaintEvent* e )
    {
        QPushButton::paintEvent( e );
        QPainter p( this );
        p.drawPixmap( 3,3, _pixmap);
        int h = QFontMetrics( font() ).height();
        p.drawText( QRect( 3, 3 + _pixmap.height() + 2, width()-6, h ), Qt::AlignCenter, _name );
    }

protected slots:
    void sayName()
    {
        QFile file( _soundFile );
        file.open( QIODevice::ReadOnly );
        QTextStream stream( &file );
        QString txt = stream.readAll();
        qDebug() << txt;
    }

private:
    QString _soundFile;
    QPixmap _pixmap;
    QString _name;
};

int main( int argc, char** argv )
{
    QApplication app(argc,argv);

    if ( argc> 1 && argv[1] == QLatin1String("-dk") ) {
        qDebug("And now in danish.");
        QLocale::setDefault( QLocale( QLocale::Danish ) );

        QTranslator* translator = new QTranslator;
        translator->load( ":/translations/resources_da.qm" );
        qApp->installTranslator( translator );
    }

    ImageButton* but1 = new ImageButton( QObject::tr("Bird"), ":/images/bird.png", ":/sounds/bird.txt" );
    ImageButton* but2 = new ImageButton( QObject::tr("Dog"), ":/images/dog.png", ":/sounds/dog.txt" );

    QWidget* box = new QWidget;
    box->resize( 300, 200 );

    QHBoxLayout* lay = new QHBoxLayout( box );
    lay->addWidget( but1, 0, Qt::AlignCenter );
    lay->addWidget( but2, 0, Qt::AlignCenter );

    box->show();
    return app.exec();
}

#include "main.moc"
