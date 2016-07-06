// ========================================================================
// Program VPLAYER is an OSG based viewer for frames ripped from video
// clips.  VPLAYER enables users to annotate individual frames and
// export all such annotations to CSV text files.

/*

./vplayer \
--newpass ./videos/jpg_frames/clip_0000_frame-00001.jpg \
--world_start_UTC 2013,9,27,16,8,54 \
--world_stop_UTC 2013,9,27,16,59,0 \
--world_time_step 0.1 \
--initial_mode Run_Movie_Mode 

*/


// ========================================================================
// Last updated on 7/18/13; 3/31/14; 4/1/14
// ========================================================================

#include <vector>
#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "astro_geo/Clock.h"
#include "osg/Custom2DManipulator.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osg2D/ImageNumberHUD.h"
#include "messenger/Messenger.h"
#include "osg/ModeController.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osg2D/MovieKeyHandler.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgWindow/ViewerManager.h"

// ========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;

   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);

// Initialize constants and parameters read in from command line as
// well as ascii text file:

   const int ndims=2;
   PassesGroup passes_group(&arguments);
   int videopass_ID=passes_group.get_videopass_ID();
//   cout << "videopass_ID = " << videopass_ID << endl;

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("2D imagery");

// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

   bool display_movie_state=true;
   bool display_movie_number=true;
   bool display_movie_world_time=false;
   bool display_movie_elapsed_time=true;
   Operations operations(
      ndims,window_mgr_ptr,passes_group,display_movie_state,
      display_movie_number,display_movie_world_time,
      display_movie_elapsed_time);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   ModeController_ptr->setState(ModeController::RUN_MOVIE);
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   root->addChild(operations.get_OSGgroup_ptr());

// Specify start, stop and step times for master game clock:

   operations.set_master_world_start_UTC(
      passes_group.get_world_start_UTC_string());
   operations.set_master_world_stop_UTC(
      passes_group.get_world_stop_UTC_string());
   operations.set_delta_master_world_time_step_per_master_frame(
      passes_group.get_world_time_step());
   
   operations.reset_AnimationController_world_time_params();

// Add a custom manipulator to the event handler list:

   osgGA::Custom2DManipulator* CM_2D_ptr = 
      new osgGA::Custom2DManipulator(ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->set_CameraManipulator(CM_2D_ptr);

// Instantiate group to hold all decorations:
   
   Decorations decorations(
      window_mgr_ptr,ModeController_ptr,CM_2D_ptr);

/*
// Instantiate points, polylines, polygons, linesegments, triangles,
// rectangles and features decorations group:

   decorations.add_Points(
      ndims,passes_group.get_pass_ptr(videopass_ID),
      AnimationController_ptr);
   PolyLinesGroup* PolyLinesGroup_ptr=
      decorations.add_PolyLines(ndims,passes_group.get_pass_ptr(
         videopass_ID));
   osgGeometry::PolygonsGroup* PolygonsGroup_ptr=decorations.add_Polygons(
      ndims,passes_group.get_pass_ptr(videopass_ID),AnimationController_ptr);
   PolygonsGroup_ptr->set_PolyLinesGroup_ptr(PolyLinesGroup_ptr);
   
   LineSegmentsGroup* LineSegmentsGroup_ptr=
      decorations.add_LineSegments(
         ndims,passes_group.get_pass_ptr(videopass_ID),
         AnimationController_ptr);
   decorations.add_Triangles(
      ndims,passes_group.get_pass_ptr(videopass_ID));
   decorations.add_Rectangles(
      ndims,passes_group.get_pass_ptr(videopass_ID));
*/

// Instantiate group to hold movie:

   MoviesGroup movies_group(
      ndims,passes_group.get_pass_ptr(videopass_ID),
//      decorations.get_PointsGroup_ptr(),
//      decorations.get_PolygonsGroup_ptr(),
      AnimationController_ptr);

   string movie_filename=
      passes_group.get_videopass_ptr()->get_first_filename();
//   cout << "movie_filename = " << movie_filename << endl;
   string image_subdir=filefunc::getdirname(movie_filename);
   if (image_subdir.size()==0) image_subdir="./";
   cout << "Looking for ripped video frames inside image_subdir = " 
        << image_subdir << endl;

   AnimationController_ptr->store_ordered_image_filenames(image_subdir);
   int number_of_images=AnimationController_ptr->
      get_n_ordered_image_filenames();
//   cout << "number_of_images = " << number_of_images << endl;

   texture_rectangle* texture_rectangle_ptr=
      movies_group.generate_new_texture_rectangle(movie_filename);
   Movie* movie_ptr=movies_group.generate_new_Movie(texture_rectangle_ptr);

// Scan through subdirectory containing video frames.  Set minimum and
// maximum photo numbers based upon the files' name:

   int min_photo_number=-1;
   int max_photo_number=-1;
   videofunc::find_min_max_photo_numbers(
      image_subdir,min_photo_number,max_photo_number);
   int Nimages=max_photo_number-min_photo_number+1;
   if (number_of_images > Nimages)
   {
      Nimages=number_of_images;
      min_photo_number=0;
      max_photo_number=Nimages-1;
   }

   cout << "min_video_frame_number = " << min_photo_number
        << " max_video_frame_number = " << max_photo_number
        << " N_video_frames = " << Nimages << endl;

//   AnimationController_ptr->set_first_framenumber(min_photo_number);
//   AnimationController_ptr->set_curr_framenumber(min_photo_number);
   AnimationController_ptr->set_nframes(Nimages);
   AnimationController_ptr->set_frame_counter_offset(min_photo_number);

   movie_ptr->get_texture_rectangle_ptr()->set_first_frame_to_display(
      min_photo_number);
   movie_ptr->get_texture_rectangle_ptr()->set_last_frame_to_display(
      max_photo_number);
   root->addChild( movies_group.get_OSGgroup_ptr() );

   MovieKeyHandler* MoviesKeyHandler_ptr=
      new MovieKeyHandler(ModeController_ptr,&movies_group);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(MoviesKeyHandler_ptr);

   cout << endl;
   cout << "******************************************************" << endl;
   cout << "Keyboard control notes:" << endl << endl;
   cout << "Note: Video window must be active before following keys will be recognized" 
        << endl << endl;
   cout << "'p' key toggles video player between 'play' and 'pause' modes " 
        << endl;
   cout << "'r' key reverses video's time direction" << endl;
   cout << "Right and left arrow keys step one video frame forward/backward"
        << endl;
   cout << "'a' key allows user to enter an annotation for current frame"
        << endl;
   cout << "'e' key exports all video annotations to output CSV file" 
        << endl;
   cout << "spacebar 'homes' video window to default settings" << endl;
   cout << endl;
   cout << "******************************************************" << endl;
   cout << "Mouse control notes:" << endl << endl;
   cout << "Left mouse button translates video frame in 2D" << endl;
   cout << "Center mouse button rotates video window in 2D" << endl;
   cout << "Right mouse button zooms video frame in/out" << endl;
   cout << "******************************************************" << endl;
   cout << endl;

   decorations.set_DataNode_ptr(movie_ptr->getGeode());

   root->addChild(decorations.get_OSGgroup_ptr());

// Attach the scene graph to the viewer:

   window_mgr_ptr->setSceneData(root);

// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_ptr->realize();

   while( !window_mgr_ptr->done() )
   {
      window_mgr_ptr->process();
   }
}

