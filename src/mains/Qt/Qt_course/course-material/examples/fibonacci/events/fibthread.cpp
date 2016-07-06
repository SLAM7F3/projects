#include <QtCore>
#include "fibthread.h"
#include "fibevent.h"

const int MAXFIB = 1000;

FibThread::FibThread( QObject* receiver, QObject* parent ) :
    QThread( parent ), _receiver( receiver ), _stop( false )
{

}

void FibThread::stop()
{
    // doesn't need a lock; we don't care whether the thread continues
    // to read false for a few more cylces. Eventually, it will see
    // the value true:
    _stop = true;
}

void FibThread::run()
{
    // This method implements computing the first 1000 fibonacci
    // numbers and communicates them to the GUI thread. This is not
    // the best implementation (caching, among other things, is
    // missing), but since this is about thread communication and not
    // about efficient algorithms, we'll gladly gloss over that.
    for ( int i = 1; i < MAXFIB; ++i ) {
        int fib = fibonacci( i );
        if ( _stop )
            return; // if stop is set, the number might be invalid
        QCoreApplication::postEvent( _receiver, new FibEvent( fib ) );
    }
}


int FibThread::fibonacci( int i )
{
    if ( _stop )
        return -1; // faster reaction to _stop
    if ( i == 0 || i == 1 )
        return 1;
    return fibonacci( i-1 ) + fibonacci( i-2 );
}
