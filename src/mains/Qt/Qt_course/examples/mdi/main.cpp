#include <QtGui>

class Editor :public QMainWindow
{
    Q_OBJECT
public:
    Editor()
    {
        _workspace = new QWorkspace;
        setCentralWidget( _workspace );

        // File Menu
        QMenu* file = new QMenu( "&File" );
        QAction* newWindow = new QAction( "New", this );
        file->addAction( newWindow );
        connect( newWindow, SIGNAL( triggered() ), this, SLOT( newWindow() ) );

        QAction* print = new QAction( "Print", this );
        connect( print, SIGNAL( triggered() ), this, SLOT( print() ) );
        file->addAction( print );

        menuBar()->addMenu( file );

        // Window Menu
        _windows = new QMenu( "&Windows" );
        menuBar()->addMenu( _windows );
        connect( _windows, SIGNAL( aboutToShow() ), this, SLOT( populateWindows() ) );
        connect( _windows, SIGNAL( triggered( QAction* ) ), this, SLOT( showWindow( QAction* ) ) );


    }

protected slots:
    void newWindow()
    {
        static int count = 0;
        QTextEdit* edit = new QTextEdit;
        _workspace->addWindow( edit );
        edit->setWindowTitle( QString( "Editor %1").arg( ++count ) );
        edit->show();
    }

    void populateWindows()
    {
        _windows->clear();
        const QWidgetList windows = _workspace->windowList();
        foreach ( QWidget* widget, windows ) {
            QAction* action =_windows->addAction( widget->windowTitle() );
            action->setData( (qlonglong) widget );
        }
    }

    void showWindow( QAction* action )
    {
        QWidget* widget = reinterpret_cast<QWidget*>( qvariant_cast<qlonglong>(action->data()));
        _workspace->setActiveWindow( widget );
    }

    void print()
    {
        QWidget* widget = _workspace->activeWindow();
        if ( !widget )
            return;

        QTextEdit* edit = static_cast<QTextEdit*>( widget );
        QMessageBox::information( this, "poor mans printing", edit->toPlainText() );
    }

private:
    QWorkspace* _workspace;
    QMenu* _windows;

};

int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    Editor* editor = new Editor;
    editor->show();

    return app.exec();
}

#include "main.moc"
