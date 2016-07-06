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
    // Radiobutton
    // -----------------------------------------
    case PE_IndicatorRadioButton:
    {
        // Background color: always Background, to fit seamlessly in the button's environment
        p->fillRect( opt->rect, opt->palette.background() );

        // Ellipse color.
        p->setPen( opt->palette.color( QPalette::Text ) );

        // Ellipse fill. 3 cases: enabled (down and normal), disabled
        QBrush fill;
        if ( opt->state & State_Sunken )
            fill = opt->palette.brush( QPalette::Button );
        else
            fill = opt->palette.brush( opt->state & State_Enabled ? QPalette::Base : QPalette::Background );
        p->setBrush( fill );
        p->drawEllipse( opt->rect.adjusted(0,0,-1,-1) );

        // Draw "checked" mark, using a smaller ellipse, with a fill but no edge color
        if ( opt->state & State_On )
        {
            QRect innerRect( opt->rect );
            const int lineWidth = 2;
            innerRect.adjust( lineWidth, lineWidth, -lineWidth, -lineWidth );
            p->setPen( Qt::NoPen );
            p->setBrush( opt->palette.brush( QPalette::Highlight ) );
            p->drawEllipse( innerRect );
        }
    }
    break;
    // Arrow down in the combobox
    case PE_IndicatorArrowDown:
    {
        // Draw the windows arrow, but red
        // No need to duplicate the arrow-drawing code, simply modify the color group
        // Reading QWindowsStyle::drawPrimitive shows that the ButtonText color is the one being used.
        QStyleOption modifiedOpt( *opt );
        modifiedOpt.palette.setColor( QPalette::ButtonText, Qt::red );
        QWindowsStyle::drawPrimitive( pe, &modifiedOpt, p, w );
    }
    break;
    // Button for scrollbars and spinboxes
    case PE_PanelButtonBevel:
    {
        QBrush fill;
        if (!(opt->state & State_Sunken) && (opt->state & State_On))
            fill = QBrush(opt->palette.light().color(), Qt::Dense4Pattern);
        else
            fill = opt->palette.brush(QPalette::Button);

        p->setBrush( fill );
        p->drawEllipse( opt->rect.adjusted(0,0,-1,-1) );
        break;
    }
    default:
        QWindowsStyle::drawPrimitive( pe, opt, p, w );
        break;
    }
}

int TestStyle::pixelMetric(PixelMetric pm, const QStyleOption *opt, const QWidget *widget) const
{
    int ret;
    switch( pm )
    {
    // Make checkboxes bigger
    case PM_IndicatorWidth:
    case PM_IndicatorHeight:
    // Make radiobuttons bigger
    case PM_ExclusiveIndicatorWidth:
    case PM_ExclusiveIndicatorHeight:
        return 18;
    // This adds some margin inside the pushbutton, to make it a bit higher.
    case PM_ButtonMargin:
        return 2*20;
    default:
        ret = QWindowsStyle::pixelMetric(pm, opt, widget);
        break;
    }

    return ret;
}

void TestStyle::drawControl(ControlElement element, const QStyleOption *opt, QPainter *p,
                            const QWidget *w ) const
{
    switch( element )
    {
    // Tricky: using CE_PushButton and simply drawing an ellipse means no text appears anymore,
    // so one might consider using CE_PushButtonBevel instead.
    // But then when the button is focused, the focus rect is still rectangular.
    // And reimplementing PE_FrameFocusRect as an ellipse, also affects the checkboxes, so that's no good.
    // Better reimplement CE_PushButton fully after all.
    case CE_PushButton:
    {
        // The easy part: the button itself ("the bevel")
        bool mouseOver = opt->state & State_MouseOver; // just a test for WA_Hover
        p->setPen( mouseOver ? Qt::red : opt->palette.color( QPalette::Foreground ) );
        p->setBrush( opt->palette.button() );
        p->drawEllipse( opt->rect.adjusted( 0, 0, -1, -1 ) );

        // The text - delegated
        drawControl(CE_PushButtonLabel, opt, p, w);

        // The focus "rect" (as an ellipse)
        if (opt->state & State_HasFocus) {
            p->setPen( opt->palette.color( QPalette::Foreground ) );
            p->setBrush( Qt::NoBrush );
            p->drawEllipse( opt->rect.adjusted( 2, 2, -3, -3 ) );
        }

        break;
    }
    default:
        QWindowsStyle::drawControl( element, opt, p, w );
    }
}

// Example usage of polish(palette)
void TestStyle::polish( QPalette& palette )
{
    // Change the "active button" color to be more visible: take the highlight color
    QColor highlight = palette.color( QPalette::Active, QPalette::Highlight );
    palette.setColor( QPalette::Active, QPalette::Button, highlight );
}

void TestStyle::polish( QWidget* w )
{
    // Install an event filter on pushbuttons in order to call setMask in resizeEvents.
    // (This is what setAutoMask did in Qt2 and Qt3)
    if ( qobject_cast<QPushButton *>( w ) ) {
        w->installEventFilter( this );
        // Also test for mouse hovering like in the slide
        w->setAttribute( Qt::WA_Hover );
    }
    QWindowsStyle::polish( w );
}

bool TestStyle::eventFilter( QObject* obj, QEvent* ev )
{
    QPushButton* pb = qobject_cast<QPushButton *>( obj );
    if ( pb && ev->type() == QEvent::Resize ) {
        // The mask defines the shape of the button. This is important when
        // using a background pixmap, but also to ensure that clicks outside
        // the ellipse don't activate the button.
        QBitmap mask( pb->size() );
        mask.clear();
        QPainter p( &mask );
        p.setBrush( QBrush(Qt::color1) );
        p.drawEllipse( pb->rect().adjusted( 0,0,-1,-1 ) );
        pb->setMask( mask );
    }
    return QWindowsStyle::eventFilter( obj, ev );
}

void TestStyle::drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p,
                                   const QWidget *w ) const
{
    switch( cc )
    {
    case CC_ComboBox:
        if (const QStyleOptionComboBox *cmb = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {

            // qwindowsstyle.cpp buglet: the overall clearing is only done in the SC_ComboBoxArrow if().
            // The next 5 lines are a workaround for that.
            QBrush editBrush = (cmb->state & State_Enabled) ? cmb->palette.brush(QPalette::Base)
                               : cmb->palette.brush(QPalette::Background);
            if (cmb->frame)
                qDrawWinPanel(p, opt->rect, opt->palette, true, &editBrush);
            else
                p->fillRect(opt->rect, editBrush);

            // Draw elliptic 'arrow'
            if (cmb->subControls & SC_ComboBoxArrow) {
                QRect ar = visualRect(opt->direction, opt->rect, QCommonStyle::subControlRect(CC_ComboBox, cmb,
                                                                                              SC_ComboBoxArrow,
                                                                                              w));
                p->setPen( opt->palette.color( QPalette::Foreground ) );
                p->setBrush( opt->palette.button() );
                p->drawEllipse( ar.adjusted(0,0,-1,-1) );

                // Draw the arrow (code copied from qwindowsstyle)
                // Another solution is to draw the red arrow directly here.
                // Depends whether you want other down arrows (e.g. button with menu) to be red too.
                State flags = State_None;
                ar.adjust(2, 2, -2, -2);
                if (opt->state & State_Enabled)
                    flags |= State_Enabled;

                if (cmb->activeSubControls == SC_ComboBoxArrow)
                    flags |= State_Sunken;
                QStyleOption arrowOpt(0);
                arrowOpt.rect = ar;
                arrowOpt.palette = cmb->palette;
                arrowOpt.state = flags;
                drawPrimitive(PE_IndicatorArrowDown, &arrowOpt, p, w);
            }

            // Draw the rest (i.e. SC_ComboBoxEditField)
            QStyleOptionComboBox newopt( *cmb );
            newopt.subControls = cmb->subControls & ~SC_ComboBoxArrow;
            QWindowsStyle::drawComplexControl( cc, &newopt, p, w );

        }
        break;
    default:
        QWindowsStyle::drawComplexControl( cc, opt, p, w );
    }
}

