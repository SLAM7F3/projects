#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItemGroup>
#include <QTime>
#include <QTimeLine>
#include <QDial>
#include <QSlider>
#include <QTimer>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGLWidget>
#include <QPushButton>

#include <QDebug>

#include "clockdialitem.h"

class Timetrix : public QWidget {
  Q_OBJECT
public: 
  explicit Timetrix(QWidget* parent=0)
    : QWidget(parent)
  {
    m_rotationDial = new QDial;
    m_rotationDial->setRange(-180,180);
    m_rotationDial->setWrapping(true);
    connect(m_rotationDial,SIGNAL(valueChanged(int)),
	    this, SLOT(slotTransform()));

    m_zoomSlider = new QSlider;
    m_zoomSlider->setRange(1,100);
    m_zoomSlider->setValue(20);
    connect(m_zoomSlider,SIGNAL(valueChanged(int)),
	    this, SLOT(slotTransform()));

    QPushButton* animateButton = new QPushButton(tr("Animate"));
    animateButton->setCheckable(true);
    connect( animateButton, SIGNAL(toggled(bool)),
	     this, SLOT(slotAnimate(bool)));

    m_scene = new QGraphicsScene(this);
    m_scene->setBackgroundBrush( Qt::black );

    const int BCOUNT = 50;
    const int DCOUNT = 100;
    
    for( int i = 0; i < DCOUNT; ++i ) {
      for( int j = 0; j < DCOUNT; ++j ) {
	ClockDialItem* dial = new ClockDialItem(0,m_scene);
	dial->setColor( QColor( 0,50,0 ) );
	dial->setHoverColor( QColor( 50, 50,0) );
	dial->setRadius(30);
	dial->setPos(i*80,j*80);
	m_dials << dial;
      }
    }
    for( int i = 0; i < BCOUNT; ++i ) {
      for( int j = 0; j < BCOUNT; ++j ) {
	ClockDialItem* dial = new ClockDialItem(0,m_scene);
	dial->setColor( QColor( 0,200,0 ) );
	dial->setHoverColor( QColor( 200, 200,0) );
	dial->setRadius(50);
	dial->setPos(i*160,j*160);
	m_dials << dial;
      }
    }

    m_scene->setSceneRect( QRectF(0.,0.,BCOUNT*160,BCOUNT*160) );

    connect( &timer, SIGNAL(timeout()), this, SLOT(slotUpdateTime()));

    m_view = new QGraphicsView(scene());
    m_view->setRenderHint(QPainter::Antialiasing);
    //m_view->setViewport( new QGLWidget );

    timeline.setCurveShape(QTimeLine::EaseInOutCurve);
    timeline.setLoopCount(1);
    timeline.setDuration( 10000 );
    timeline.setFrameRange( -45, 45 );

    connect( &timeline, SIGNAL(frameChanged(int)),
	     this, SLOT(slotAnimateRotation(int)));
    connect( &timeline, SIGNAL(finished()),
	     this, SLOT(slotUpdateTimeline()), Qt::QueuedConnection);

    QHBoxLayout* topLayout = new QHBoxLayout;
    QVBoxLayout* toolLayout = new QVBoxLayout;
    topLayout->addLayout(toolLayout);
    toolLayout->addWidget(m_rotationDial);
    toolLayout->addWidget(m_zoomSlider);
    toolLayout->addWidget(animateButton);
    topLayout->addWidget(m_view);
    setLayout(topLayout);
    slotTransform();
  }

  QGraphicsScene* scene() const { return m_scene; }
private slots:
  void slotUpdateTime()
  {
    m_view->update();
  }
  void slotTransform()
  {
    int f = m_rotationDial->value();
    int z = m_zoomSlider->value();
    m_view->resetMatrix();
    m_view->rotate(f);
    m_view->scale(z/10.,z/10. );
  }
  void slotAnimateRotation(int f)
  {
    m_rotationDial->setValue(f);
    int z = m_zoomSlider->value();
    m_view->resetMatrix();
    m_view->rotate(f);
    m_view->scale(z/10.,z/10. );
  }

  void slotAnimate(bool b)
  {
    if(b) {
      timeline.start();
    } else {
      timeline.stop();
    }
  }
  void slotUpdateTimeline()
  {
    timeline.toggleDirection();
    timeline.start();
  }

private:
  QTimer timer;
  QTimeLine timeline;
  QGraphicsScene* m_scene;
  QVector<ClockDialItem*> m_dials;
  QDial* m_rotationDial;
  QSlider* m_zoomSlider;  
  QGraphicsView* m_view;
};

int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    Timetrix p;
    p.show();
    return app.exec();
}

#include "main.moc"
