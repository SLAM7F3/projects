#ifndef PENDULUMITEM_H
#define PENDULUMITEM_H

#include <QGraphicsItemGroup>

class QGraphicsLineItem;
class QGraphicsEllipseItem;

class PendulumItem : public QGraphicsItem {
public:
  PendulumItem( QGraphicsItem* parent=0, QGraphicsScene* scene=0);
  virtual ~PendulumItem();

  virtual QRectF boundingRect() const;
  virtual void paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0 );

private:
  QGraphicsLineItem* m_rod;
  QGraphicsEllipseItem* m_weight;
};

#endif /* PENDULUMITEM_H */

