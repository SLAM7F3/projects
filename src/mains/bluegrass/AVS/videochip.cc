// ========================================================================
// Program VIDEOCHIP is a variant of VIDEO which stands up a video
// server.  A standard http get request containing longitude-latitude
// bounding box coordinates can be passed into the server.  A chipped
// subimage from the current frame corresponding to the bounding box
// region is then exported by this program.

/*

videochip --region_filename ./packages/ar1_texture_frames_all_orig.pkg \
--VideoServer_URL 127.0.0.1:4042 \
--initial_mode Run_Movie_Mode 

*/

// ========================================================================
// Last updated on 5/29/08; 10/12/09; 5/5/10
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <QtCore/QtCore>

#include "Qt/web/VideoServer.h"

#include "osg/osgGraphicals/AnimationController.h"
#include "osg/Custom2DManipulator.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osg2D/ImageNumberHUD.h"
#include "osg/ModeController.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osg2D/MovieKeyHandler.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osgWindow/ViewerManager.h"

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
   cout << "videopass_ID = " << videopass_ID << endl;

   string VideoServer_URL=passes_group.get_VideoServer_URL();
   cout << "VideoServer_URL = " << VideoServer_URL << endl;

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
   
   Decorations decorations(window_mgr_ptr,ModeController_ptr,CM_2D_ptr);

// Instantiate group to hold movie:

   MoviesGroup movies_group(
      ndims,passes_group.get_pass_ptr(videopass_ID),
      decorations.get_PointsGroup_ptr(),
      decorations.get_PolygonsGroup_ptr(),AnimationController_ptr);
   Movie* movie_ptr=movies_group.generate_new_Movie(passes_group);

/*
   string photo_subdir="/data/EO/ground_photos/2009/Boston/MIT_30K/MIT_Results_2328/images/junk/";
   movie_ptr->set_watch_subdirectory(photo_subdir);
*/

   bool twoD_flag=true;
   string photo_filename="/data/EO/ground_photos/2009/Boston/MIT_30K/MIT_Results_2328/images/SDC12825.rd.jpg";
   movie_ptr->reset_displayed_photograph(photo_filename,twoD_flag);

// Chip out sub-image from full video if video_corner_vertices are
// defined in input package and longitude-latitude bbox is specified
// here:

/*
   double min_long=-101.94;
   double max_long=-101.93;
   double min_lat=33.492;
   double max_lat=33.50;
   movie_ptr->georegister_subtexture_corners(
      min_long,max_long,min_lat,max_lat);
*/

//   twovector lower_left_texture_fracs(0.0 , 0.0);
//   twovector lower_right_texture_fracs(0.5 , 0.0);
//   twovector upper_right_texture_fracs(0.5 , 0.5);
//   twovector upper_left_texture_fracs(0.0 , 0.5);
//   movie_ptr->set_texture_fracs(
//      lower_left_texture_fracs,lower_right_texture_fracs,
//      upper_right_texture_fracs,upper_left_texture_fracs);

   AnimationController_ptr->set_nframes(movie_ptr->get_Nimages());
   root->addChild( movies_group.get_OSGgroup_ptr() );

   MovieKeyHandler* MoviesKeyHandler_ptr=
      new MovieKeyHandler(ModeController_ptr,&movies_group);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(MoviesKeyHandler_ptr);

   decorations.set_DataNode_ptr(movie_ptr->getGeode());

   root->addChild(decorations.get_OSGgroup_ptr());

// Attach the scene graph to the viewer:

   window_mgr_ptr->setSceneData(root);

// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_ptr->realize();

// Instantiate VideoServer which enables chip handling:

   string VideoServer_hostname=stringfunc::get_hostname_from_URL(
      VideoServer_URL);
   int VideoServer_portnumber=stringfunc::get_portnumber_from_URL(
      VideoServer_URL);
//   cout << "VideoServer_hostname = " << VideoServer_hostname
//        << " VideoServer_portnumber = " << VideoServer_portnumber
//        << endl;

   VideoServer video_server(
      VideoServer_hostname,VideoServer_portnumber);
   video_server.set_Movie_ptr(movie_ptr);

// ========================================================================

   while( !window_mgr_ptr->done() )
   {
      window_mgr_ptr->process();
      app.processEvents();
   }

   delete window_mgr_ptr;
}
