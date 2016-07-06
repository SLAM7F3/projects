#include <QtGui>

class MyScrollArea :public QAbstractScrollArea
{
public:
  MyScrollArea( QWidget* parent ) :QAbstractScrollArea( parent )
  {
	horizontalScrollBar()->setRange(0,(1<<31)-1000);
	verticalScrollBar()->setRange(0,(1<<31)-1000);
    verticalScrollBar()->setSingleStep(fontMetrics().lineSpacing());
  }

protected:
  virtual void resizeEvent( QResizeEvent* ev )
  {
	// Set up correct page-steps
	horizontalScrollBar()->setPageStep( horizontalScrollBar()->maximum()/ev->size().width() );
	verticalScrollBar()->setPageStep( verticalScrollBar()->maximum()/ev->size().height() );
  }

  virtual void paintEvent( QPaintEvent* ev )
  {
	// Find the boundaries in "model space"
	int cx = horizontalScrollBar()->value()+ev->rect().left();
	int cy = verticalScrollBar()->value()+ev->rect().top();
	int cw = ev->rect().width();
	int ch = ev->rect().height();

	// Create painter and transform into "model space"
	QPainter p(viewport());
	p.translate( -horizontalScrollBar()->value(),
				 -verticalScrollBar()->value() );
    QFontMetrics fm=p.fontMetrics();

	// Find rows and columns at location
    int rowheight=fm.lineSpacing();
    int toprow=cy/rowheight;
    int bottomrow=(cy+ch+rowheight-1)/rowheight;
    int colwidth=fm.width("00000000,000000000 ")+3;
    int leftcol=cx/colwidth;
    int rightcol=(cx+cw+colwidth-1)/colwidth;

    QString str;
    for (int r=toprow; r<=bottomrow; r++) {
      int py=r*rowheight;
      for (int c=leftcol; c<=rightcol; c++) {
		    int px=c*colwidth;
		    str.sprintf("%d,%d",c,r);
		    p.drawText(px+3, py+fm.ascent(), str);
      }
    }
  }
  virtual void scrollContentsBy( int dx, int dy )
  {
	viewport()->scroll(dx,dy);
  }
};

int main( int argc, char** argv )
{
  QApplication app( argc, argv );

  MyScrollArea* scroll = new MyScrollArea( 0 );
  scroll->show();

  return app.exec();
}
