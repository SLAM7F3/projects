#ifndef FIBTHREAD_H
#define FIBTHREAD_H

#include <QThread>

class FibThread : public QThread
{
    Q_OBJECT

public:
    FibThread( QObject * receiver, QObject* parent = 0 );

    virtual void run();

    void stop();

private:
    int fibonacci( int );

    QObject * _receiver;
    volatile bool _stop;
};


#endif /* FIBTHREAD_H */

