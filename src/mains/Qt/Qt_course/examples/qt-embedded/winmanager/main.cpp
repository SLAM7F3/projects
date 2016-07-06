#include <qapplication.h>
#include <qtextedit.h>
#include <qdecorationdefault_qws.h>
#include <qpainter.h>
#include <qdrawutil.h>
#include <qwindowsystem_qws.h>
#include <qmap.h>
#include <qlabel.h>
#include <qmenu.h>

/* XPM */
static const char * const icon[] = {
/* width height ncolors chars_per_pixel */
    "16 16 11 1",
/* colors */
    "  c #000000",
    ". c #336600",
    "X c #666600",
    "o c #99CC00",
    "O c #999933",
    "+ c #333300",
    "@ c #669900",
    "# c #999900",
    "$ c #336633",
    "% c #666633",
    "& c #99CC33",
/* pixels */
    "oooooooooooooooo",
    "oooooooooooooooo",
    "ooooo#.++X#ooooo",
    "ooooX      Xoooo",
    "oooX  XO#%  X&oo",
    "oo#  Ooo&@O  Ooo",
    "oo. Xoo#+ @X Xoo",
    "oo+ OoO+ +O# +oo",
    "oo+ #O+  +## +oo",
    "oo. %@ ++ +. Xoo",
    "oo#  O@OO+   #oo",
    "oooX  X##$   Ooo",
    "ooooX        Xoo",
    "oooo&OX++X#OXooo",
    "oooooooooooooooo",
    "oooooooooooooooo"
};

static int titleHeight = 20;
static int bd = 4;

class MyDecoration : public QObject, public QDecoration
{
    Q_OBJECT
public:
    MyDecoration() : QObject( 0 ), QDecoration() {}

    virtual QRegion region( const QWidget* , const QRect& rect, int decorationRegion )
    {
        switch( decorationRegion ) {
        case QDecoration::Left: // These disables resizing.
        case QDecoration::Right:
        case QDecoration::Top:
        case QDecoration::Bottom:
        case QDecoration::TopLeft:
        case QDecoration::TopRight:
        case QDecoration::BottomLeft:
        case QDecoration::BottomRight:
        case QDecoration::Minimize:
        case QDecoration::Maximize:
        case QDecoration::Normalize:
        case QDecoration::Close:
        case QDecoration::Title: // This disables moving
            return QRegion(); // 0 pixels for this.
        case QDecoration::Menu: // There is only one button, it's the one that shows the menu
            return QRegion( rect.left(), rect.top() - titleHeight, titleHeight, titleHeight );
        case QDecoration::All:
            return QRegion( rect.left()-bd, rect.top()-titleHeight-bd, rect.width()+2*bd,
                            rect.height()+titleHeight+2*bd) - rect;
        case QDecoration::None: return QRegion();
        }
        return QRegion();
    }

    virtual bool paint(QPainter *painter, const QWidget* widget, int decorationRegion = All,
                       DecorationState state = Normal)
    {
        const QPalette pal = widget->palette();
        const QRect rect = widget->rect();

        // Draw the border.
        if ( (decorationRegion & Borders) && state == Normal )
        {
            painter->setPen( QPen(pal.background(), bd) );
            painter->setBrush( pal.background() );
            painter->drawRect( -bd+1, -titleHeight-bd+1,
                               rect.width()+2*bd-2, rect.height()+titleHeight+2*bd-2);
        }

        // Draw the blue bar and the title in it
        if ((decorationRegion & Title) && state == Normal) {
            qDrawWinPanel(painter, rect.left() + titleHeight, rect.top()-titleHeight,
                          rect.width()-titleHeight, titleHeight, pal, true,
                          &pal.brush(QPalette::Highlight));

            painter->setPen( pal.color(QPalette::HighlightedText) );
            painter->drawText( rect.left() + titleHeight, rect.top()-titleHeight,
                               rect.width()-titleHeight, titleHeight,
                               Qt::AlignCenter, widget->windowTitle());
        }

        // Draw the menu button
        if (decorationRegion & Menu) {
            painter->drawPixmap( rect.left(), rect.top() - titleHeight, QPixmap( icon ) );
        }

        return true;
    }

    // Replace the system menu with our own
    virtual void buildSysMenu(QWidget *, QMenu *menu)
    {
        _map.clear();
        const QList<QWSWindow *> list = qwsServer->clientWindows();
        QStringList captions;
        foreach ( QWSWindow * window, list ) {
            if ( window->isVisible() ) {
                captions << window->caption();
                _map.insert( window->caption(), window );
            }
        }

        captions.sort();

        foreach( QString caption, captions ) {
            QDecorationAction* action = new QDecorationAction( caption, menu, None );
            menu->addAction( action );
            connect( action, SIGNAL( activated() ), this, SLOT( switchWindow() ) );
        }
    }

protected slots:
    void switchWindow()
    {
        QDecorationAction* action = static_cast<QDecorationAction*>( sender() );
        _map[action->text()]->raise();
    }

private:
    QMap<QString, QWSWindow*> _map;
};

int main( int argc, char** argv )
{
    QApplication app( argc, argv, QApplication::GuiServer );

    // Notice due to a bug in Qtopia Core 4.1, we need to call this method
    // before creating any widgets (bug #100520)
    qApp->qwsSetDecoration( new MyDecoration() );

    QTextEdit* edit = new QTextEdit(0);
    edit->setWindowTitle("Editor 1");
    edit->setPlainText("<qt>This is editor <b>number one</b></qt>");
    edit->show();
    edit->showMaximized();

    edit = new QTextEdit(0);
    edit->setWindowTitle("Editor 2");
    edit->setPlainText("<qt>This is <font color=\"red\">another editor.</font></qt>");
    edit->show();
    edit->showMaximized();


    return app.exec();
}

#include "main.moc"
