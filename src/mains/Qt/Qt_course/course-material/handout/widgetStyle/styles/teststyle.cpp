#include <QtGui>
#include "teststyle.h"

void TestStyle::drawPrimitive( PrimitiveElement pe,
                               const QStyleOption *opt, QPainter *p,
                               const QWidget *w ) const
{
    switch( pe ) {
    // Checkbox
    // -----------------------------------------
    case PE_IndicatorCheckBox:
    {
        // Background color. 3 cases: enabled (down or normal), disabled
        QBrush fill;
        if ( opt->state & State_Sunken )
            fill = opt->palette.brush( QPalette::Button );
        else
            fill = opt->palette.brush( opt->state & State_Enabled ? QPalette::Base : QPalette::Background );
        p->fillRect( opt->rect, fill );

        // Rectangle color.
        p->setPen( opt->palette.color( QPalette::Text ) );
        p->drawRect( opt->rect.adjusted(0,0,-1,-1) );

        // Draw "checked" mark.
        if ( opt->state & State_On )
        {
            QRect innerRect( opt->rect );
            const int lineWidth = 2;
            innerRect.adjust( lineWidth, lineWidth, -lineWidth, -lineWidth );
            p->fillRect( innerRect, opt->palette.brush( QPalette::Highlight ) );
        }
    }
    break;
    default:
        QWindowsStyle::drawPrimitive( pe, opt, p, w );
        break;
    }
}

void TestStyle::drawControl( ControlElement element, const QStyleOption *opt, QPainter *p,
                             const QWidget *w ) const
{
    switch( element )
    {
    default:
        QWindowsStyle::drawControl( element, opt, p, w );
    }
}

int TestStyle::pixelMetric(PixelMetric pm, const QStyleOption *opt, const QWidget *widget) const
{
    switch( pm )
    {
    // Make checkboxes bigger
    case PM_IndicatorWidth:
    case PM_IndicatorHeight:
        return 18;
    default:
        return QWindowsStyle::pixelMetric(pm, opt, widget);
    }
}
