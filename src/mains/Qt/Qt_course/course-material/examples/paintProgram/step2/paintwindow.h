#ifndef PAINT_H
#define PAINT_H

#include <QMainWindow>

class QAction;
class ScribbleArea;
class QColor;

class PaintWindow :public QMainWindow
{
  Q_OBJECT

public:
  explicit PaintWindow( QWidget* parent=0 );

private:
  void setupFileMenu();
  void setupColorMenu();
  void setupHelpMenu();

private slots:
  void slotChangeColor( QAction* action );
  void slotAbout();
  void slotAboutQt();

signals:
  void colorChange( const QColor& color );

private:
  ScribbleArea* _scribbleArea;
  QAction *_black, *_red, *_blue, *_green, *_yellow;
};

#endif /* PAINT_H */

