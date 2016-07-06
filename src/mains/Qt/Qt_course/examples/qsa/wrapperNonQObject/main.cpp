#include <qapplication.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qsinterpreter.h>
#include <qswrapperfactory.h>
#include <qsobjectfactory.h>
#include <qlistview.h>

class QListViewItemWrapper :public QObject {
    Q_OBJECT

public:
    QListViewItemWrapper( QListViewItem* item ) : _item( item ) {}

public slots:
    QString text( int col ) {
        return _item->text( col );
    }

private:
    QListViewItem* _item;
};


class QListViewItemFactory :public QSWrapperFactory {
public:
    QListViewItemFactory() {
        registerWrapper( "QListViewItem" );
    }

    virtual QObject * create ( const QString & className, void * ptr ) {
        if ( className == QString::fromLatin1( "QListViewItem" ) ) {
            return new QListViewItemWrapper( static_cast<QListViewItem*>( ptr ) );
        }
        return 0;
    }
};


// This is only needed to create instances of QListViewItems from QSA
// Watch out for the license, if you do so.
class ItemFactory : public QSObjectFactory {
public:
    ItemFactory() {
        registerClass("QListViewItem");
    }

    QObject* create(const QString &name, const QSArgumentList &args, QObject* ctx) {
        QObject* listView = args[0].qobject();
        if (!listView || !listView->inherits("QListView")) {
            interpreter()->throwError("Argument 1 is not present or not of type QListView");
            return 0;
        }
        QString str1 = args[1].variant().toString();
        QString str2 = args[2].variant().toString();
        return new QListViewItemWrapper( new QListViewItem((QListView*)listView, str1, str2 ));
    }
};


int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    QListView* view = new QListView( 0, "listview" );
    view->addColumn( "Col 1" );
    view->addColumn( "Col 2" );
    new QListViewItem( view, "Item 1A", "Item 1B" );
    new QListViewItem( view, "Item 2A", "Item 2B" );
    new QListViewItem( view, "Item 3A", "Item 3B" );
    new QListViewItem( view, "Item 4A", "Item 4B" );
    view->show();

    // Read the script file
    QFile f("script");
    f.open( IO_ReadOnly );
    QTextStream stream(&f);
    QString script = stream.read();

    // Register the Table
    QSInterpreter* interpreter = QSInterpreter::defaultInterpreter();
    interpreter->addWrapperFactory( new QListViewItemFactory );
    interpreter->addObjectFactory( new ItemFactory );
    interpreter->addTransientObject( view );

    if ( !interpreter->checkSyntax( script ) )
        qDebug("Syntax Error: %s", interpreter->errorMessage().latin1());
    else
        interpreter->evaluate( script );


    return app.exec();
}

#include "main.moc"
