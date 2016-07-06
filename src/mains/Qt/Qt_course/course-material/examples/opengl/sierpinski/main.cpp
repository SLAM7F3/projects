#include <QtOpenGL>

class SierpinskiWidget : public QGLWidget
{
public:
  virtual void paintGL();
  virtual void initializeGL();
  virtual void resizeGL( int w, int h );
};


void SierpinskiWidget::paintGL()
{
  typedef GLfloat point2[2];
  point2 vertices[3] = {{ 0.,0.},{width()/2.,height()},{width(),0.}}; // triangle
  point2 p = { width()/2.0, height()/2.0 }; // initial point

  glClear( GL_COLOR_BUFFER_BIT );

  /* Compute and plot 100000 new points */
  for( int k = 0; k < 100000; k++ ) {
	int j = rand() % 3; // pick a random vertex

	// Compute the point between the vertex and the old point
	p[0] = ( p[0] + vertices[j][0] ) / 2.0;
	p[1] = ( p[1] + vertices[j][1] ) / 2.0;

	// Plot new point
	glBegin( GL_POINTS );
	glVertex2fv( p );
	glEnd();
  }

  glFlush();
}


void SierpinskiWidget::initializeGL()
{
  glClearColor( 1.0, 1.0, 1.0, 1.0 ); 
  glColor3f( 1.0, 0.0, 0.0 );
}


void SierpinskiWidget::resizeGL( int w, int h )
{
  glViewport(0,0,w,h);
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  gluOrtho2D( 0., w, 0., h );
  glMatrixMode( GL_MODELVIEW );

  updateGL();
}

int main( int argc, char **argv )
{
    QApplication a(argc,argv);			

    SierpinskiWidget w;
    w.resize( 400, 350 );
    w.show();
    return a.exec();
}
