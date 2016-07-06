#include <qwindowsstyle.h>

class TestStyle: public QWindowsStyle
{
public:
    TestStyle() : QWindowsStyle() {}

    void drawPrimitive( PrimitiveElement pe,
                        const QStyleOption *opt, QPainter *p,
                        const QWidget *w = 0 ) const;

    void drawControl( ControlElement element, const QStyleOption *opt, QPainter *p,
                      const QWidget *w = 0 ) const;

    int pixelMetric(PixelMetric pm, const QStyleOption *option = 0, const QWidget *widget = 0) const;
};
