#ifndef PUZZLE_H
#define PUZZLE_H
#include <QWidget>
#include <QPixmap>
#include <QRegion>
#include <QList>
class QMouseEvent;
class QPaintEvent;

class Puzzle :public QWidget
{
    Q_OBJECT

public:
    Puzzle( QWidget* parent = 0 );

protected:
    virtual void paintEvent( QPaintEvent* );

protected slots:
    void showNext();

private:
    QPixmap _image;
    QPixmap _question;
    QList<QRegion> _regions;
    int _count;
};

#endif /* PUZZLE_H */

