#include <QtGui>
#include "fibwidget.h"
#include "fibthread.h"
#include "fibevent.h"

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

    _workerThread = new FibThread( this, this );
    _workerThread->start( QThread::IdlePriority );
}

void FibWidget::customEvent( QEvent* event )
{
    // First make sure that this is infact a FibEvent
    if ( event->type() != FibEvent::TYPE ) {
        // It isn't. Call the base class implementation and return.
        QWidget::customEvent( event );
        return;
    }

    // It is a FibEvent. Do a cast.
    const FibEvent * fibevent = static_cast<FibEvent*>( event );

    // unpack the data, and display.
    _listbox->addItem( QString::number( fibevent->fib() ) );

    // Make sure the last addition is visible
    _listbox->scrollToItem( _listbox->item( _listbox->count()-1 ) );
}

void FibWidget::slotQuit()
{
    _workerThread->stop();
    _workerThread->wait(); // this could freeze the GUI in theory, but in practice it's OK
    delete _workerThread;

    qApp->quit();
}
