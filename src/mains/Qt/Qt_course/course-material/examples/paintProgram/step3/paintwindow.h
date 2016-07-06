#ifndef PAINT_H
#define PAINT_H

#include <QMainWindow>
class QAction;
class ScribbleArea;

class PaintWindow :public QMainWindow
{
  Q_OBJECT

public:
  explicit PaintWindow( QWidget* parent=0 );

protected:
  void setupFileMenu();
  void setupColorMenu();
  void setupHelpMenu();

protected slots:
  void slotChangeColor( QAction* action );
  void slotAbout();
  void slotAboutQt();
  void slotLoad();
  void slotSave();

signals:
  void colorChange( const QColor& color );

private:
  ScribbleArea* _scribbleArea;
  QAction *_black, *_red, *_blue, *_green, *_yellow;
};

#endif /* PAINT_H */

