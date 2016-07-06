#ifndef FIBWIDGET_H
#define FIBWIDGET_H

#include <QWidget>

class QListWidget;
class FibThread;

class FibWidget : public QWidget
{
    Q_OBJECT
public:
    FibWidget( QWidget* parent = 0 );

protected:
    void customEvent( QEvent* );

private slots:
    void slotQuit();

private:
    QListWidget* _listbox;
    FibThread* _workerThread;
};



#endif /* FIBWIDGET_H */

