#include <QtGui>
#include "bookstore.h"

int main( int argc, char** argv ) {
    QApplication app( argc, argv );
    BookStore* bookStore = new BookStore;
    bookStore->show();

    return app.exec();
}
