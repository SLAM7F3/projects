#include <qapplication.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qpainter.h>

/**
 * This is the child widget which paints a semi-transparent blob
 * It doesn't draw all pixels inside its rect(), and doesn't use setAutoFillBackground(true),
 * so in Qt 4.1 the child widgets under it appear.
 */
class BlobWidget : public QWidget {
    Q_OBJECT
public:
    BlobWidget( QWidget* parent )
        : QWidget( parent ) {
            // Without this attribute clicks in the widget's rectangle
            // wouldn't be received by the push buttons.
            setAttribute(Qt::WA_TransparentForMouseEvents);
    }

    void paintEvent( QPaintEvent* ) {
        QPainter p( this );
        p.setPen( QPen( Qt::black, 3 ) );

        const QRect r = rect();
        p.fillRect( r, QColor( 10, 50, 180, 60 /*transparency*/ ) );
    }
};

/**
 * This is the window widget containing the pushbuttons.
 * The blob widget is a child of it, too.
 */
class MyWidget : public QWidget {
    Q_OBJECT
public:
    MyWidget( QWidget* parent )
        : QWidget( parent ),
          m_blobWidget( new BlobWidget( this ) ) {

        QGridLayout* layout = new QGridLayout( this );
        for ( int i = 0; i < 16; ++i )  {
            QPushButton* but =
                new QPushButton( QString::number(i),  this );
            layout->addWidget( but,  i/4, i%4 );
            connect( but, SIGNAL( clicked() ),
                     this, SLOT( selectButton() ) );
        }
        m_blobWidget->hide();
    }

protected slots:
    void selectButton()  {
        const QPushButton* selected = static_cast<const QPushButton*>( sender() );
        m_blobWidget->raise();
        m_blobWidget->show();
        m_blobWidget->setGeometry( selected->geometry().adjusted( -20, -20, 20, 20 ) );
    }

private:
    BlobWidget* m_blobWidget;
};

int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    MyWidget* widget = new MyWidget( 0 );
    widget->show();

    return app.exec();
}

#include "main.moc"
