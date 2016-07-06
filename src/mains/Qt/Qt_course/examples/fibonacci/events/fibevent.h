#ifndef FIBEVENT_H
#define FIBEVENT_H

#include <qevent.h>

class FibEvent : public QEvent
{
public:
    FibEvent( int fib )
        : QEvent( TYPE ),
          _fib( fib ) {}

    int fib() const { return _fib; }

    // use a random value between QEvent::User and QEvent::MaxUser to
    // avoid clashes with other QEvent subclasses:
    static const QEvent::Type TYPE = static_cast<QEvent::Type>( User + 4242 );

private:
    int _fib;
};



#endif /* FIBEVENT_H */

