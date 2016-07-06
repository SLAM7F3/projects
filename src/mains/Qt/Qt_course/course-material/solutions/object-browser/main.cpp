#include <QtGui>
#include "objectbrowsermodel.h"
#include "ui_test.h"
#include "objectviewer.h"

int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    QDialog* dialog = new QDialog;
    Ui::Test* ui = new Ui::Test;
    ui->setupUi( dialog );
    dialog->show();

    ObjectViewer* view = new ObjectViewer;
    view->show();

    ObjectBrowserModel* model = new ObjectBrowserModel;
    view->setModel( model );


    return app.exec();
}
