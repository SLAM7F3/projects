#include <QtGui>
#include "fibwidget.h"
#include "fibthread.h"

FibWidget::FibWidget( QWidget* parent )
    : QWidget( parent )
{
    _listbox = new QListWidget( this );
    _listbox->setObjectName( "listbox" );

    QPushButton* quit = new QPushButton( tr( "Quit" ), this );
    quit->setObjectName( "quit" );
    connect( quit, SIGNAL( clicked() ), this, SLOT( slotQuit() ) );

    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->addWidget( _listbox );
    layout->addWidget( quit );

    _workerThread = new FibThread( this );
    // This must be a queued connection, since we'll be emitting from
    // another thread, but both _workerThread and this have been
    // created in the GUI thread, so the default would be to use a
    // direct connection. Note that because of this it's usually a bad
    // idea to define signals, slots, and data members in QThread
    // subclasses. We do it here solely for simplicity.
    connect( _workerThread, SIGNAL( fibFound( int ) ), this, SLOT( slotAddFib( int ) ),
             Qt::QueuedConnection );
    _workerThread->start( QThread::IdlePriority );
}

void FibWidget::slotAddFib( int fib )
{
    _listbox->addItem( QString::number( fib ) );
    // make sure that the last added item is always visible
    _listbox->scrollToItem( _listbox->item( _listbox->count()-1 ) );
}


void FibWidget::slotQuit()
{
    _workerThread->stop();
    _workerThread->wait(); // this could freeze the GUI in theory, but in practice it's OK
    delete _workerThread;

    qApp->quit();
}
