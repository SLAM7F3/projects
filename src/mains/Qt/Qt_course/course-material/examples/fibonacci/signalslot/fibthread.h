#ifndef FIBTHREAD_H
#define FIBTHREAD_H

#include <QtCore>

class FibThread : public QThread
{
    Q_OBJECT

public:
    FibThread( QObject* parent = 0 );

    virtual void run();

    void stop();

    // This signal must be connected to by a queued connection, since
    // we'll be emitting it from inside run(), but both the receiver
    // and this QThread instance are usually created in the GUI
    // thread, so the default would be to use a direct
    // connection. Note that because of this it's usually a bad idea
    // to define signals, slots, and data members in QThread
    // subclasses. We do it here solely for simplicity.
signals:
    void fibFound( int );

private:
    int fibonacci( int );
    volatile bool _stop;
};


#endif /* FIBTHREAD_H */

