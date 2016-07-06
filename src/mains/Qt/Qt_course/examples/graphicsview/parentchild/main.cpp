#include <QApplication>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsView>

class TestItem : public QGraphicsItem {
public:
  explicit TestItem(QGraphicsItem* parent=0,QGraphicsScene* scene=0)
    : QGraphicsItem(parent,scene), 
      m_childitem1(new QGraphicsRectItem(this,scene)),
      m_childitem2(new QGraphicsRectItem(this,scene))
  {
    m_childitem1->setRect(0,0,100,100);
    m_childitem1->setBrush(Qt::blue);
    m_childitem2->setRect(0,0,100,100);
    m_childitem2->setBrush(Qt::green);
    layoutChildren();
  }

  void setSize(const QSizeF& s)
  {
    m_size = s;
    prepareGeometryChange();
    layoutChildren();
  }
  QSizeF size() const { return m_size; }

  virtual QRectF boundingRect() const
  {
    return QRectF(pos(),size());
  }

  virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget=0)
  {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->fillRect(boundingRect(),Qt::red);
  }
protected:
#if 0
  // This made no diff either...
  virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value)
  {
    if( change == ItemPositionChange ) {
      layoutChildren();
    }
    return QGraphicsItem::itemChange(change,value);
  }
#endif
private:
  void layoutChildren()
  {
    m_childitem1->setPos(0,0);
    m_childitem2->setPos(QPointF(size().width()-100,size().height()-100));
  }

  QGraphicsRectItem* m_childitem1;
  QGraphicsRectItem* m_childitem2;
  QSizeF m_size;
};

int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    /* Shows blue and green rect at wrong position: */
    QGraphicsScene* scene = new QGraphicsScene;
    TestItem* ti = new TestItem; 
    ti->setPos( QPoint(200,200) );
    ti->setSize( QSizeF(500,500 ) );
    scene->addItem(ti);
    
    QGraphicsView view;
    view.setScene(scene);
    view.show();

    return app.exec();
}
