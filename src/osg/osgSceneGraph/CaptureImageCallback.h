// ========================================================================
// CaptureImageCallback header file
// ========================================================================
// Last updated on 3/4/12
// ========================================================================

#ifndef CAPTUREIMAGECALLBACK_H
#define CAPTUREIMAGECALLBACK_H

#include <Producer/Camera>
#include <osgDB/WriteFile>

class CaptureImageCallback : public Producer::Camera::Callback
{

  public:
		
   CaptureImageCallback();
   virtual ~CaptureImageCallback();

   void set_output_filename(std::string filename);

   void operator()(const Producer::Camera& cam);

  protected:
   
  private:
   
   std::string output_filename;

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void CaptureImageCallback::set_output_filename(std::string filename)
{
   output_filename=filename;
}

#endif // CaptureImageCallback.h
