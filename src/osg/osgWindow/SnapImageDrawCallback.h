// ==========================================================================
// Header file for SnapImageDrawCallback class
// ==========================================================================
// Last updated on 3/24/09; 3/25/09; 5/12/10
// ==========================================================================

#ifndef SNAPIMAGEDRAWCALLBACK_H
#define SNAPIMAGEDRAWCALLBACK_H

#include <string>
#include <osg/Image>
#include <osgDB/WriteFile>
#include <Producer/Camera>

class WindowManager;

class SnapImageDrawCallback: public Producer::Camera::Callback
{

  public:

// Initialization, constructor and destructor functions:

   SnapImageDrawCallback();
   ~SnapImageDrawCallback();

// Set & get member functions:

   int get_snapped_image_counter() const;
   std::string get_image_suffix() const;
   void setRecordingOnOrOff(bool recording_or_not);
   void setFileName(const std::string& filename);
   void set_frame_cycle_size(int fcs);
   void setSnapImageOnNextFrame(bool flag);
   void set_horiz_scale_factor(double f);

   virtual void operator()( const Producer::Camera & camera);

  private:

   bool _recording,_snapImageOnNextFrame;
   int _frame_number,frame_cycle_size,snapped_image_counter;
   double horiz_scale_factor;
   std::string output_filename,image_suffix;
   WindowManager* WindowManager_ptr;
   osg::ref_ptr<osg::Image> image_refptr;

   void allocate_member_objects();
   void initialize_member_objects();
}; 

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void SnapImageDrawCallback::setFileName(const std::string& filename) 
{ 
   output_filename = filename; 
}

inline void SnapImageDrawCallback::set_frame_cycle_size(int fcs) 
{ 
   frame_cycle_size=fcs; 
}

inline void SnapImageDrawCallback::setSnapImageOnNextFrame(bool flag) 
{ 
   _snapImageOnNextFrame = flag; 
}

inline int SnapImageDrawCallback::get_snapped_image_counter() const
{
   return snapped_image_counter;
}

inline std::string SnapImageDrawCallback::get_image_suffix() const
{
   return image_suffix;
}

inline void SnapImageDrawCallback::set_horiz_scale_factor(double f)
{
   horiz_scale_factor=f;
}


#endif  // SnapImageDrawCallback.h
