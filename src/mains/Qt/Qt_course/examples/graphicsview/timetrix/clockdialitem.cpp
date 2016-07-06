#include "clockdialitem.h"

#include <QPainter>
#include <QPen>
#include <QGraphicsLineItem>
#include <QStyleOptionGraphicsItem>
#include <QTime>

#include <QDebug>

ClockDialItem::ClockDialItem(QGraphicsItem* parent,QGraphicsScene* scene)
  : QGraphicsItem(parent,scene), m_hover(false)
{
  setColor(Qt::green);
  setRadius(100.);
  setAcceptsHoverEvents(true);
}

ClockDialItem::~ClockDialItem()
{
}

void ClockDialItem::setRadius(qreal r)
{
  m_radius = r;
  update();
}

qreal ClockDialItem::radius() const
{
  return m_radius;
}

void ClockDialItem::setColor( const QColor& c )
{
  m_color = c;
  update();
}

QColor ClockDialItem::color() const
{
  return m_color;
}

void ClockDialItem::setHoverColor( const QColor& c)
{
  m_hovercolor = c;
  update();
}

QColor ClockDialItem::hoverColor() const
{
  return m_hovercolor;
}

void ClockDialItem::hoverEnterEvent( QGraphicsSceneHoverEvent * event )
{
  Q_UNUSED(event);
  m_hover = true;
  update();
}

void ClockDialItem::hoverLeaveEvent( QGraphicsSceneHoverEvent * event )
{
  Q_UNUSED(event);
  m_hover = false;
  update();
}


QRectF ClockDialItem::boundingRect() const
{
  qreal pw = 3;
  return QRectF( -radius()-pw/2, -radius()-pw/2,
		 2*radius()+pw, 2*radius()+pw );
}

void ClockDialItem::paint( QPainter* painter, 
			   const QStyleOptionGraphicsItem* option, 
			   QWidget* widget )
{
  if( option->levelOfDetail < 10./radius() ) return;
  painter->setPen(QPen(QColor(30,30,30), 2));
  painter->setBrush(QColor(20,20,20));
  painter->drawEllipse( QRectF(-radius(),-radius(),2*radius(), 2*radius()) );
  if( m_hover ) painter->setPen( QPen( hoverColor(), 
				 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ) );
  else painter->setPen( QPen( color(), 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ) );
  painter->setBrush( Qt::NoBrush );

  painter->save();
  painter->rotate(180);
  painter->drawLine( QLineF(0.,radius()/2., 0., radius()-3) );
  for( int i = 0; i < 11; ++i ) {
    painter->rotate( 30 );
    painter->drawLine( QLineF(0.,radius()*2./3., 0., radius()-3) );
  }
  painter->restore();

  QTime t(QTime::currentTime());
  painter->save();  
  painter->rotate( 360*(60*t.hour()+t.minute())/(12*60) - 90. );
  painter->drawLine(0.,0.,radius()/3.,0.);
  painter->restore();
  painter->save();
  painter->rotate( 360*(t.minute())/60 - 90. );
  painter->drawLine(0.,0.,radius()/2.,0.);
  painter->restore();
}
