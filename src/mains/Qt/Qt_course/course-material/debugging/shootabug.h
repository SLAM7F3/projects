#ifndef __shootabug
#define __shootabug

class ShootABug :public QObject 
{
public:
  virtual bool eventFilter( QObject* recv, QEvent* event )
  {
    if ( event->type() == QEvent::MouseButtonPress && 
         dynamic_cast<QMouseEvent*>(event)->state() == Qt::ControlButton ) {
      // Ctrl + right mouse click.

      qDebug("----------------------------------------------------");
      qDebug("Widget name : " + QString( recv->name() ) );
      qDebug("Widget class: " + QString( recv->className() ) );
      qDebug("\nObject info:");
      recv->dumpObjectInfo();
      qDebug("\nObject tree:");
      recv->dumpObjectTree();
      qDebug("----------------------------------------------------");
    }
    return false;
  }
};

#endif /* shootabug */

