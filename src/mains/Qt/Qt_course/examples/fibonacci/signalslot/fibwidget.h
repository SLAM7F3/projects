#ifndef FIBWIDGET_H
#define FIBWIDGET_H

#include <QWidget>
class FibThread;
class QListWidget;

class FibWidget : public QWidget
{
    Q_OBJECT
public:
    FibWidget( QWidget* parent = 0 );

public slots:
    void slotAddFib( int fib );
    void slotQuit();

private:
    QListWidget* _listbox;
    FibThread* _workerThread;
};



#endif /* FIBWIDGET_H */

