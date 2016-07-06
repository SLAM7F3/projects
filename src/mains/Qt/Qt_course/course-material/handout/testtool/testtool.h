#ifndef TESTTOOL_H
#define TESTTOOL_H

#include <QMainWindow>
#include <QString>
class QPushButton;
class QStackedWidget;

class TestTool :public QMainWindow
{
    Q_OBJECT

public:
    TestTool( QWidget* parent = 0);
    void load( QString fileName );

public slots:
    void slotOpenFile();
    void forward();
    void backward();

protected:
    void addPage( const QString& name, const QString& objectives,
                  const QString& input, const QString& output );
    int nextId() { return _maxId++; }
    void setState();


private:
    QStackedWidget* _widgetStack;
    int _maxId, _curId;
    QPushButton *_forwardBut, *_backwardBut;
};

#endif /* TESTTOOL_H */

