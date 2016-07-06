#include <QTextEdit>
#include <qtopia/qtopiaapplication.h>

class MyMainWindow :public QTextEdit
{
public:
    MyMainWindow (QWidget* parent = 0, Qt::WFlags f = 0 )
        :QTextEdit( parent )
        {
            setWindowFlags( f );
        }

};

QTOPIA_ADD_APPLICATION( "simpleApp", MyMainWindow );
QTOPIA_MAIN
