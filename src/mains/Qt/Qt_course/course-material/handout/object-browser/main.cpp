#include <QtGui>
#include "ui_test.h"

int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    QDialog* dialog = new QDialog;
    Ui::Test* ui = new Ui::Test;
    ui->setupUi( dialog );
    dialog->show();

    // TODO: Create your model and view here.

    return app.exec();
}
