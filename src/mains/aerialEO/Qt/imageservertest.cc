// ========================================================================
// Program IMAGESERVERTEST reads in Activity Region #1 Bluegrass
// Constant Hawk video. It also instantiates an ImageServer which
// continuously waits for GET_NEXT_IMAGE requests coming from a
// client.  When the ImageServer receives such a request, it extracts 
// a 1000x1000 chip from the current video frame and returns that to the
// server using Ross Anderson's image header.

// This program emulates transmission of Real-Time Persistent
// Surveillance imagery coming from the air down to the ground
// analysis station in the form of JPEG chips.

// ========================================================================
// Last updated on 10/11/09; 5/6/10; 7/12/10
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <QtCore/QtCore>

#include "osg/osgGraphicals/AnimationController.h"
#include "astro_geo/Clock.h"
#include "osg/Custom2DManipulator.h"
#include "osg/osgOrganization/Decorations.h"
#include "Qt/web/ImageServer.h"
#include "messenger/Messenger.h"
#include "osg/ModeController.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osg2D/MovieKeyHandler.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "general/sysfuncs.h"
#include "track/tracks_group.h"
#include "osg/osgWindow/ViewerManager.h"

#include "general/outputfuncs.h"
#include "templates/mytemplates.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Initialize Qt application:

   QCoreApplication app(argc,argv);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=2;
   PassesGroup passes_group(&arguments);
   int videopass_ID=passes_group.get_videopass_ID();

   string ImageServer_URL=passes_group.get_LogicServer_URL();
   cout << "ImageServer_URL = " << ImageServer_URL << endl;
   string ImageServer_hostname=stringfunc::get_hostname_from_URL(
      ImageServer_URL);
   int ImageServer_portnumber=stringfunc::get_portnumber_from_URL(
      ImageServer_URL);
   cout << "ImageServer_hostname = " << ImageServer_hostname
        << " ImageServer_portnumber = " << ImageServer_portnumber
        << endl;

// Instantiate ActiveMQ messengers:

   string broker_URL="tcp://127.0.0.1:61616";
//   int pass_ID=passes_group.get_n_passes()-1;
//   string broker_URL=passes_group.get_pass_ptr(pass_ID)->
//      get_PassInfo_ptr()->get_ActiveMQ_hostname();
   cout << "ActiveMQ broker_URL = " << broker_URL << endl;
   string message_sender_ID="IMAGESERVERTEST";

   bool include_sender_and_timestamp_info_flag=false;

//   string blueforce_car_message_queue_channel_name="blueforce_car";
   string blueforce_car_message_queue_channel_name="viewer_update";
   Messenger blueforce_car_transmitter_messenger( 
      broker_URL,blueforce_car_message_queue_channel_name,message_sender_ID,
      include_sender_and_timestamp_info_flag);

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("2D imagery");

// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

   bool display_movie_state=true;
   bool display_movie_number=true;
   bool display_movie_world_time=true;
//   bool display_movie_state=false;
//   bool display_movie_number=false;
//   bool display_movie_world_time=false;
   Operations operations(
      ndims,window_mgr_ptr,passes_group,display_movie_state,
      display_movie_number,display_movie_world_time);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   root->addChild(operations.get_OSGgroup_ptr());

// Utilize any frame-to-worldtime information entered via input
// package files:

   PassInfo* video_passinfo_ptr=
      passes_group.get_passinfo_ptr(videopass_ID);
   if (video_passinfo_ptr != NULL)
   {
      AnimationController_ptr->correlate_frames_to_world_times(
         video_passinfo_ptr->get_frame_times());
   }

// Add a custom manipulator to the event handler list:

   osgGA::Custom2DManipulator* CM_2D_ptr = 
      new osgGA::Custom2DManipulator(ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->set_CameraManipulator(CM_2D_ptr);

// Instantiate group to hold all decorations:
   
   Decorations decorations(
      window_mgr_ptr,ModeController_ptr,CM_2D_ptr);

// Instantiate group to hold movie:

   MoviesGroup movies_group(
      ndims,passes_group.get_pass_ptr(videopass_ID),AnimationController_ptr);
   Movie* Movie_ptr=movies_group.generate_new_Movie(passes_group);

// FAKE FAKE:  Thurs, May 6, 2010
// Next line added for bots n dogs alg develop only...

//   string photo_subdir="/data/EO/ground_photos/2009/Boston/MIT_30K/MIT_Results_2328/images/junk/";
//   string photo_subdir="./test_images/";
   string photo_subdir="/data/tech_challenge/webcam_images/";
   Movie_ptr->set_watch_subdirectory(photo_subdir);

   AnimationController_ptr->set_nframes(Movie_ptr->get_Nimages());
   AnimationController_ptr->setDelay(0.75);
   root->addChild( movies_group.get_OSGgroup_ptr() );

   MovieKeyHandler* MoviesKeyHandler_ptr=
      new MovieKeyHandler(ModeController_ptr,&movies_group);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(MoviesKeyHandler_ptr);

   decorations.set_DataNode_ptr(Movie_ptr->getGeode());

   root->addChild(decorations.get_OSGgroup_ptr());

// Attach the scene graph to the viewer:

   window_mgr_ptr->setSceneData(root);

// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_ptr->realize();

// Instantiate Imageserver:

   ImageServer* ImageServer_ptr=new ImageServer(
      ImageServer_hostname,ImageServer_portnumber);
   ImageServer_ptr->set_Movie_ptr(Movie_ptr);
   ImageServer_ptr->pushback_Messenger_ptr(
      &blueforce_car_transmitter_messenger);

   int sensor_ID=4;
   bool iPhone_beacon_flag=true;
//   bool iPhone_beacon_flag=false;
   ImageServer_ptr->set_sensor_ID(sensor_ID);
   ImageServer_ptr->set_iPhone_beacon_flag(iPhone_beacon_flag);

   tracks_group iPhone_tracks_group;
   track* iPhone_track_ptr=iPhone_tracks_group.generate_new_track(sensor_ID);
   iPhone_track_ptr->set_description("IPHONE");
   ImageServer_ptr->set_iPhone_track_ptr(iPhone_track_ptr);

// ========================================================================

   while( !window_mgr_ptr->done() )
   {
      window_mgr_ptr->process();
//      for (int iter=0; iter<10; iter++)
      {
         app.processEvents();
      }
   }

   delete window_mgr_ptr;
}

