#ifndef PAINT_H
#define PAINT_H

#include "scribbleArea.h"
#include <QMainWindow>
class QAction;
class ConfigDialog;

class PaintWindow :public QMainWindow
{
    Q_OBJECT

public:
    PaintWindow( QWidget* parent );

protected:
    void setupFileMenu();
    void setupColorMenu();
    void setupHelpMenu();
    void setupShapesMenu( );

protected slots:
    void slotChangeColor( QAction* action );
    void slotAbout();
    void slotAboutQt();
    void slotLoad();
    void slotSave();
    void slotConfigure();
    void slotShapeMenu( QAction* );

signals:
    void colorChange( const QColor& color );
    void load( const QString& );
    void save( const QString& );
    void print();
    void shapeChange( int );
    void drawPixmap( const QPixmap& );

private:
    ScribbleArea* _scribbleArea;
    QAction *_black, *_red, *_blue, *_green, *_yellow;
    QAction *_line, *_rect, *_ellipse, *_image;

    ConfigDialog* _configDialog;
};

#endif /* PAINT_H */

