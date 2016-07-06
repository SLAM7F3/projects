// ==========================================================================
// PIXELBUFFER class member function definitions
// ==========================================================================
// Last modified on 3/3/12; 3/4/12
// ==========================================================================

#include <iostream>
#include "osg/osgSceneGraph/CaptureImageCallback.h"
#include "osg/osgSceneGraph/PixelBuffer.h"
#include "math/threevector.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void PixelBuffer::allocate_member_objects()
{
   pBufferCamera_refptr = new Producer::Camera;
   pbufferSceneHandler_refptr = new osgProducer::OsgSceneHandler;
   CaptureImageCallback_ptr=new CaptureImageCallback;
}		       

void PixelBuffer::initialize_member_objects()
{
   root_ptr=NULL;
}		       

PixelBuffer::PixelBuffer()
{	
//   cout << "inside PixelBuffer constructor" << endl;
   allocate_member_objects();
   initialize_member_objects();
   initialize_BufferCamera_and_SceneHandler();
}		       

void PixelBuffer::initialize_BufferCamera_and_SceneHandler()
{
   pBufferCamera_refptr->getRenderSurface()->setDrawableType( 
      Producer::RenderSurface::DrawableType_PBuffer );
   pBufferCamera_refptr->setClearColor(0,0,0,0);
   pBufferCamera_refptr->addPostDrawCallback( CaptureImageCallback_ptr );


   pbufferSceneHandler_refptr->getSceneView()->setDefaults();
   pbufferSceneHandler_refptr->getSceneView()->setDrawBufferValue(GL_FRONT);
   pBufferCamera_refptr->setSceneHandler( pbufferSceneHandler_refptr.get());

   set_WindowRectangle(1024,1024);
   set_camera_FOVs(36,36);
}		       

PixelBuffer::~PixelBuffer()
{	
   delete CaptureImageCallback_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const PixelBuffer& P)
{
   return outstream;
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

void PixelBuffer::set_root_ptr(osg::Group* root)
{
   pbufferSceneHandler_refptr->getSceneView()->setSceneData( root );
}

void PixelBuffer::set_WindowRectangle(int width,int height)
{
   pBufferCamera_refptr->getRenderSurface()->setWindowRectangle(
      0, 0, width,height);
}

void PixelBuffer::set_camera_posn_and_pointing(
   const threevector& camera_posn,const threevector& Uhat,
   const threevector& Vhat)
{
   threevector What=Uhat.cross(Vhat);
   pBufferCamera_refptr->setViewByLookat( 
      camera_posn.get(0),camera_posn.get(1),camera_posn.get(2),
      camera_posn.get(0)-What.get(0),camera_posn.get(1)-What.get(1),
      camera_posn.get(2)-What.get(2),
      Vhat.get(0),Vhat.get(1),Vhat.get(2));
}

void PixelBuffer::set_camera_FOVs(double horiz_FOV,double vert_FOV)
{
   double nearClip=1;
   double farClip=1E6;
   pBufferCamera_refptr->getLens()->setPerspective(
      horiz_FOV,vert_FOV,nearClip,farClip);
}

// ==========================================================================
// PixelBuffer manipulation member functions
// ==========================================================================

void PixelBuffer::TakeSnapshot(string output_filename)
{
   CaptureImageCallback_ptr->set_output_filename(output_filename);
   pBufferCamera_refptr->frame();
}


