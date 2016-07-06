// ========================================================================
// CaptureImageCallback class member function definitions
// ========================================================================
// Last updated on 3/4/12
// ========================================================================

#include <iostream>
#include "osg/osgSceneGraph/CaptureImageCallback.h"

using std::cout;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void CaptureImageCallback::allocate_member_objects()
{
}

void CaptureImageCallback::initialize_member_objects()
{
}
		
CaptureImageCallback::CaptureImageCallback() 
{ 
   allocate_member_objects();
   initialize_member_objects();
   
}

CaptureImageCallback::~CaptureImageCallback()
{
}

// ------------------------------------------------------------------------
void CaptureImageCallback::operator()(const Producer::Camera& cam)
{
//   cout << "inside CaptureImageCallback::operator()" << endl;
   int x, y;
   unsigned int w, h;
   cam.getProjectionRectangle(x,y,w,h);

   osg::ref_ptr<osg::Image>image = new osg::Image;
   image->allocateImage( w, h, 1, GL_RGB, GL_UNSIGNED_BYTE);
   image->readPixels( 0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE);
   osgDB::writeImageFile( *(image.get()), output_filename );

   cout << "Captured image written to "+output_filename << endl;
}
