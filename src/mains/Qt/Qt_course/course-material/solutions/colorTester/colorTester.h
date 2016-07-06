#ifndef COLORTESTER_H
#define COLORTESTER_H

#include <QWidget>
class QLabel;

class ColorTester :public QWidget
{
    Q_OBJECT

public:
    ColorTester( QWidget* parent = 0 );

public slots:
    void slotSelectColor();

private:
    QLabel* _colorLabel;
};


#endif /* COLORTESTER_H */

