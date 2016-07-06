#include "editor.h"
#include <QApplication>

int main( int argc, char** argv )
{
    QApplication app( argc, argv );
    Editor* editor = new Editor(0);
    editor->show();
    return app.exec();
}
