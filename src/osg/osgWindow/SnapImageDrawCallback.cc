// ==========================================================================
// SnapImageDrawCallback class member function definitions
// ==========================================================================
// Last updated on 3/25/09; 5/11/10; 5/12/10
// ==========================================================================

#include <iostream>
#include <stdio.h>
#include <sstream>
#include "osg/osgWindow/SnapImageDrawCallback.h"
#include "general/stringfuncs.h"
#include "osg/osgWindow/WindowManager.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ostringstream;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void SnapImageDrawCallback::allocate_member_objects()
{
   image_refptr = new osg::Image;
}		       

void SnapImageDrawCallback::initialize_member_objects()
{
   _recording=_snapImageOnNextFrame=false;

// Important note added on 5/11/10: We learned the painful and hard
// way that the image_suffix defined below must match the image suffix
// used within MyViewerEventHandler for auto movie generation!

   image_suffix="png";
//   image_suffix="jpg";
//   image_suffix="rgb";

   frame_cycle_size=10;
   snapped_image_counter=0;
   horiz_scale_factor=1.0;
   WindowManager_ptr=NULL;
}		       

SnapImageDrawCallback::SnapImageDrawCallback()
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

SnapImageDrawCallback::~SnapImageDrawCallback()
{	
}

// ==========================================================================

void SnapImageDrawCallback::setRecordingOnOrOff(bool recording_or_not) 
{ 
   _recording=recording_or_not; 
   _frame_number = snapped_image_counter = 0;
//   if (_recording)
//   {
//      cout << "Enter starting ouput file number:" << endl;
//      cin >> snapped_image_counter;
//   }
}

// ---------------------------------------------------------------------
// This function is called on every traversal of the tree, (about 9 or
// 10 times per second).  If you are recording a video, it snaps a
// .rgb file of the screen, or if you have told it you wanted to take
// a screen capture, it will write out the one image here

void SnapImageDrawCallback::operator()(const Producer::Camera & camera)
{
//   cout << "inside SnapimageDrawCallback::operator()" << endl;
   
   if (!_snapImageOnNextFrame && !_recording) return;

   int x,y;
   unsigned int width,height;
   camera.getProjectionRectangle(x,y,width,height);
   image_refptr->readPixels(x,y,width,height, GL_RGB,GL_UNSIGNED_BYTE);

   if (_snapImageOnNextFrame && !_recording)
   {
      if (osgDB::writeImageFile(*image_refptr,output_filename))
      {
         cout << "Saved screen capture into " << output_filename << endl;
      }
      _snapImageOnNextFrame = false;
   }

   if (!nearly_equal(horiz_scale_factor,1.0))
   {
      image_refptr->scaleImage(
         horiz_scale_factor*image_refptr->s(),
         horiz_scale_factor*image_refptr->t(),image_refptr->r());
   }
   
   if (_recording)
   {
//      cout << "frame_cycle_size = " << frame_cycle_size << endl;
      if (_frame_number % frame_cycle_size == 0)
      {
         snapped_image_counter++;
         if (snapped_image_counter%10==0) 
            cout << snapped_image_counter << " " << flush;
          

         const int ndigits=4;
         
         string output_subdir="recorded_video/"+output_filename+"/";
         string output_image_filename=output_filename+
            stringfunc::integer_to_string(
               snapped_image_counter,ndigits)+"."+image_suffix;
         string full_pathname=output_subdir+output_image_filename;

         if (!osgDB::writeImageFile(*image_refptr,full_pathname))
         {
            cout << "Error! Could not write out image file!" << endl;
            setRecordingOnOrOff(false);
            return;
         }
      } // frame_number % frame_cycle_size ==0 conditional
            
      _frame_number++;
   } // _recording conditional
}
