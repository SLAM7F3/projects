#include <QtGui/QApplication>
#include "flashviewer.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FlashViewer w;
    w.show();
    a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));
    return a.exec();
}
