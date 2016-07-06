#ifndef CLOCKDIALITEM_H
#define CLOCKDIALITEM_H

#include <QGraphicsItem>
#include <QColor>

class QGraphicsLineItem;

class ClockDialItem : public QGraphicsItem {
public:
  ClockDialItem(QGraphicsItem* parent=0,QGraphicsScene* scene=0);
  virtual ~ClockDialItem();
  virtual QRectF boundingRect() const;
  virtual void paint( QPainter* painter, 
		      const QStyleOptionGraphicsItem* option, 
		      QWidget* widget = 0 );
  void setRadius(qreal);
  qreal radius() const;

  void setColor( const QColor& c );
  QColor color() const;

  void setHoverColor( const QColor& c);
  QColor hoverColor() const;

protected:
  virtual void hoverEnterEvent( QGraphicsSceneHoverEvent * event );
  virtual void hoverLeaveEvent( QGraphicsSceneHoverEvent * event );

private:
  qreal m_radius;
  QColor m_color;
  QColor m_hovercolor;
  bool m_hover;
};

#endif /* CLOCKDIALITEM_H */

