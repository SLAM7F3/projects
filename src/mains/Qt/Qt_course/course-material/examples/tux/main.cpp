#include <QtGui>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QLabel topLevelLabel;
    QPixmap pixmap(":/images/tux.png");
    topLevelLabel.setPixmap(pixmap);
    topLevelLabel.setMask(pixmap.mask());
    topLevelLabel.show();
    return app.exec();
}
