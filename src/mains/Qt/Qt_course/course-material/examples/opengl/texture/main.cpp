#include <QtOpenGL>

class MyGLWidget : public QGLWidget {
  Q_OBJECT
public:
  MyGLWidget( QWidget* parent )
    : QGLWidget(parent),
      xRot(0),yRot(0)
  {
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(rotateOneStep()));
    timer->start(20);
  }

  ~MyGLWidget() 
  { 
    makeCurrent();
    deleteTexture(texid);
  }
public slots:
  void rotateOneStep() 
  {
	xRot += 4;
	yRot += 4;
	updateGL();
  }
protected:
  virtual void initializeGL() 
  {
	glEnable(GL_TEXTURE_2D);
	QPixmap screenshot = QPixmap::grabWindow( QApplication::desktop()->winId() );
	texid = bindTexture( screenshot );
  }

  virtual void paintGL()
  {
    glClear( GL_COLOR_BUFFER_BIT );

    glLoadIdentity();
    glTranslatef( 0.0, 0.0, -5.0 );

    glRotatef( xRot, 1.0, 0.0, 0.0 );
    glRotatef( yRot, 0.0, 1.0, 0.0 );
	
	//if using more than one texture, call
	//glBindTexture( GL_TEXTURE_2D, texid );
	glBegin( GL_POLYGON );
	glTexCoord2d(0.,0.); glVertex3f(  -1.0,  -1.0, 0 );
	glTexCoord2d(1.,0.); glVertex3f(   1.0,  -1.0, 0 );
	glTexCoord2d(1.,1.); glVertex3f(   1.0,   1.0, 0 );
	glTexCoord2d(0.,1.); glVertex3f(  -1.0,   1.0, 0 );
	glEnd();	
  }

  virtual void resizeGL( int w, int h )
  {
    glViewport( 0, 0, (GLint)w, (GLint)h );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glFrustum( -1.0, 1.0, -1.0, 1.0, 3.5, 6.5 );
    glMatrixMode( GL_MODELVIEW );
  }

private:
  int xRot;
  int yRot;

  GLuint texid;
};

int main( int argc, char** argv ) {
    QApplication app( argc, argv );
    
    MyGLWidget w(0);
    w.resize(700,700);
    w.show();
	
    return app.exec();
}

#include "main.moc"
