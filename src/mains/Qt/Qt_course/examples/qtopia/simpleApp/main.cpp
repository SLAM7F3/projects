#include <QTextEdit>
#include <qtopia/qtopiaapplication.h>

class TextEdit : public QTextEdit
{
public:
    TextEdit( QWidget* parent = 0, Qt::WFlags f = 0 ) :
        QTextEdit( parent ) { setWindowFlags( f ); }
};


QTOPIA_ADD_APPLICATION("simpleApp", TextEdit)
QTOPIA_MAIN
