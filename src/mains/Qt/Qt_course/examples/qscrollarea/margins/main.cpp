#include <QtGui>

class MyScrollArea :public QScrollArea
{
Q_OBJECT

public:
  MyScrollArea( QWidget* parent ) : QScrollArea( parent )
  {
	QWidget* contents = new QWidget();
	contents->setMinimumSize(100, 20*10);
	setWidget(contents);

    for (mNumCheckboxes = 0; mNumCheckboxes < 10; mNumCheckboxes++) {
      QCheckBox* b = new QCheckBox(QString("box #%1").arg(mNumCheckboxes), contents );
      b->move( 0, mNumCheckboxes*20);
    }

    mMore1 = new QPushButton( "More", this );
    mMore2 = new QPushButton( "More", this );

	// Make sure buttons are below scrollbars
	mMore1->lower();
	mMore2->lower();

    connect( mMore1, SIGNAL( clicked() ), this, SLOT(slotMore()) );
    connect( mMore2, SIGNAL( clicked() ), this, SLOT(slotMore()) );

    setViewportMargins(0,mMore1->sizeHint().height(),0,mMore2->sizeHint().height());
  }

protected slots:
  void slotMore()
  {
    QCheckBox* b = new QCheckBox(QString("box #%1").arg(mNumCheckboxes), widget() );
    b->move(0, mNumCheckboxes*20);
    b->show();
    mNumCheckboxes++;
    widget()->setMinimumSize( 100, 20*mNumCheckboxes );

	verticalScrollBar()->setValue(20*mNumCheckboxes);
  }


protected:
  virtual void resizeEvent( QResizeEvent* e )
  {
    QScrollArea::resizeEvent( e );
    mMore1->move( 0, 0 );
    mMore2->move( 0, 2+mMore1->sizeHint().height()+viewport()->size().height() );
  }

private:
  int mNumCheckboxes;
  QPushButton* mMore1;
  QPushButton* mMore2;
};


int main( int argc, char** argv )
{
  QApplication app( argc, argv );

  MyScrollArea* sa = new MyScrollArea( 0 );

  sa->show();

  return app.exec();
}

#include "main.moc"
