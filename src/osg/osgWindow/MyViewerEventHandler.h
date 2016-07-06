// ==========================================================================
// Header file for MyViewerEventHandler class 
// ==========================================================================
// Last modified on 5/17/10; 5/19/10; 12/21/10; 2/28/11; 4/14/16
// ==========================================================================

#ifndef MYVIEWEREVENTHANDLER_H
#define MYVIEWEREVENTHANDLER_H

#include <osgGA/GUIEventHandler>
#include <osg/Node>
#include <osgProducer/Viewer>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "astro_geo/Clock.h"
#include "osg/osgWindow/CustomAnimationPathManipulator.h"
#include "osg/CustomManipulator.h"
#include "messenger/Messenger.h"
#include "osg/osgWindow/SnapImageDrawCallback.h"

class Messenger;
class WindowManager;

namespace osgProducer {

   class OSGPRODUCER_EXPORT MyViewerEventHandler : 
      public osgGA::GUIEventHandler
   {
      public:
    
         MyViewerEventHandler(WindowManager* WM_ptr);

// Set & get member functions:

         void set_auto_generate_movies_flag(bool flag);
         void set_CustomManipulator_ptr(osgGA::CustomManipulator* CM_ptr);
         void setWriteImageOnNextFrame(bool writeImageOnNextFrame);
         void setWriteImageFileName(const std::string& filename);
         void set_frame_cycle_size(int fcs);
         void set_horiz_scale_factor(double f);
         std::string get_flv_movie_path() const;
         void set_Viewer_Messenger_ptr(Messenger* messenger_ptr);
         void set_tomcat_subdir(std::string subdir);
         osgGA::CustomAnimationPathManipulator* 
            get_CustomAnimationPathManipulator_ptr();
         const osgGA::CustomAnimationPathManipulator* 
            get_CustomAnimationPathManipulator_ptr() const;

// Event handling member functions:

         virtual bool handle(
            const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa);

         virtual void accept(osgGA::GUIEventHandlerVisitor& gehv);

// Movie recording member functions:

         bool begin_recording_movie();
         bool end_recording_movie();
         bool get_recording_flag() const;
         double get_total_movie_duration() const;
         std::string retrieve_movie_frame_file(double frame_time);
         void report_progress(double progress);
         void purge_flash_movies();

     protected:

     private:

         bool auto_generate_movies_flag;
         bool _recording,_firstTimeTogglingFullScreen;
         int max_snapped_images,n_snapped_images;
         double fps,total_movie_duration;
         std::string output_imagery_subdir,flv_movie_path,avi_movie_filename;
//         std::string finished_movie_subdir;
         std::string tomcat_subdir;
         std::string image_suffix;
         Clock clock;
         osgProducer::OsgCameraGroup* _cg;
         WindowManager* WindowManager_ptr;
         Messenger* Viewer_Messenger_ptr;

         osgGA::CustomManipulator* CustomManipulator_ptr;
         osg::ref_ptr<osgGA::CustomAnimationPathManipulator> apm_refptr;

         SnapImageDrawCallback* SnapImageDrawCallback_ptr;
         typedef std::vector<SnapImageDrawCallback*> 
            SnapImageDrawCallbackList;
         SnapImageDrawCallbackList  _snapImageDrawCallbackList;

         void allocate_member_objects();
         void initialize_member_objects();

         void setRecording(bool recording_or_not);
         bool generate_movie(std::string video_codec);
         bool generate_flash_movie();
   };
   
// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

   inline void MyViewerEventHandler::set_auto_generate_movies_flag(bool flag)
      {
         auto_generate_movies_flag=flag;
      }

   inline void MyViewerEventHandler::set_Viewer_Messenger_ptr(
      Messenger* messenger_ptr)
      {
         Viewer_Messenger_ptr=messenger_ptr;
      }

   inline void MyViewerEventHandler::set_CustomManipulator_ptr(
      osgGA::CustomManipulator* CM_ptr)
      {
         CustomManipulator_ptr=CM_ptr;
      }

   inline bool MyViewerEventHandler::get_recording_flag() const
      {
         return _recording;
      }

   inline double MyViewerEventHandler::get_total_movie_duration() const
      {
         return total_movie_duration;
      }

   inline std::string MyViewerEventHandler::get_flv_movie_path() const
      {
//         std::cout << "inside MyViewerEventHandler::get_flv_movie_path()"
//                   << std::endl;
//         std::cout << "flv_movie_path = " << flv_movie_path << std::endl;
         return flv_movie_path;
      }

   inline void MyViewerEventHandler::set_tomcat_subdir(std::string subdir)
      {
         tomcat_subdir=subdir;
      }

   inline osgGA::CustomAnimationPathManipulator* MyViewerEventHandler::
      get_CustomAnimationPathManipulator_ptr()
   {
      if (apm_refptr.valid())
      {
         return apm_refptr.get();
      }
      else
      {
         return NULL;
      }
   }

   inline const osgGA::CustomAnimationPathManipulator* MyViewerEventHandler::
      get_CustomAnimationPathManipulator_ptr() const
   {
      if (apm_refptr.valid())
      {
         return apm_refptr.get();
      }
      else
      {
         return NULL;
      }
   }
   
} // osgProducer namespace


#endif //  MYVIEWEREVENTHANDLER_H
