#include <QtGui>

class SliderWithTip :public QSlider
{
public:
    SliderWithTip( Qt::Orientation orient, QWidget* parent = 0 ) : QSlider( orient, parent ) {};

protected:
    virtual bool event( QEvent* e )
    {
        if ( e->type() == QEvent::ToolTip ) {
            QHelpEvent* he = static_cast<QHelpEvent*>(e);
            int length= width();
            int val = minimum() + (int) ((float) (maximum()-minimum())*((float)he->x()/length));

            QToolTip::showText( he->globalPos(), QString::number(val) );
            return true;
        }
        return QSlider::event( e );
    }
};



int main( int argc, char**argv)
{
    QApplication app(argc, argv);

    SliderWithTip* slider = new SliderWithTip( Qt::Horizontal );
    slider->setRange(0, 100);
    slider->setValue( 50 );

    QLCDNumber* lcd = new QLCDNumber;
    lcd->setNumDigits( 2 );

    QObject::connect(slider, SIGNAL(valueChanged(int)), lcd, SLOT(display(int)));

    QWidget* top = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout( top );
    layout->addWidget( slider );
    layout->addWidget( lcd );

    top->show();

    return app.exec();
}
