// ==========================================================================
// Header file for PIXELBUFFER class
// ==========================================================================
// Last modified on 3/3/12; 3/4/12
// ==========================================================================

#ifndef PIXELBUFFER_H
#define PIXELBUFFER_H

#include <iostream>
#include <string>
#include <vector>
#include <Producer/Camera>
#include <osg/Group>
#include <osgProducer/OsgSceneHandler>

class CaptureImageCallback;
class threevector;

class PixelBuffer
{

  public:

// Initialization, constructor and destructor functions:

   PixelBuffer();
   ~PixelBuffer();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const PixelBuffer& P);

// Set & get member functions:

   void set_root_ptr(osg::Group* root);
   void set_WindowRectangle(int width,int height);
   void set_camera_posn_and_pointing(
      const threevector& camera_posn,const threevector& Uhat,
      const threevector& Vhat);
   void set_camera_FOVs(double horiz_FOV,double vert_FOV);

// PixelBuffer manipulation member functions:

   void TakeSnapshot(std::string output_filename);

  private:

   osg::Group* root_ptr;
   Producer::ref_ptr<Producer::Camera> pBufferCamera_refptr;
   osg::ref_ptr<osgProducer::OsgSceneHandler> pbufferSceneHandler_refptr;

   CaptureImageCallback* CaptureImageCallback_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const PixelBuffer& P);

   void initialize_BufferCamera_and_SceneHandler();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

#endif // PixelBuffer.h



