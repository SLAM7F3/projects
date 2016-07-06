#include "pendulumitem.h"

#include <QGraphicsLineItem>
#include <QGraphicsEllipseItem>
#include <QPen>
#include <QRadialGradient>

static const int PENWIDTH = 3;

PendulumItem::PendulumItem(QGraphicsItem* parent, QGraphicsScene* scene)
  : QGraphicsItem(parent,scene)
{
  m_rod = new QGraphicsLineItem( 0,0,0,100, this,scene );
  m_rod->setPen( QPen( Qt::black, 3 ) );
  m_weight = new QGraphicsEllipseItem( -20, 100, 40, 40, this,scene );
  m_weight->setPen( QPen(Qt::black, 3 ));
  
  QRadialGradient g( 0, 120, 20, -10, 110 );
  g.setColorAt( 0.0, Qt::white );
  g.setColorAt( 0.5, Qt::yellow );
  g.setColorAt( 1.0, Qt::black );
  m_weight->setBrush(g);
}

PendulumItem::~PendulumItem()
{
}

QRectF PendulumItem::boundingRect() const
{
  return QRectF(-20-PENWIDTH/2,-PENWIDTH/2,40+PENWIDTH,140+PENWIDTH);
}

void PendulumItem::paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
  Q_UNUSED(painter);
  Q_UNUSED(option);
  Q_UNUSED(widget);
}
