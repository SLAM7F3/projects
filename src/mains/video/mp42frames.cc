// ========================================================================
// Program MP42FRAMES extracts individual frames from MP4 video clips
// as PNG files.  
// ========================================================================
// Last updated on 9/18/09
// ========================================================================

#include <vector>
#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "osg/Custom2DManipulator.h"
#include "osg/osg2D/ImageNumberHUD.h"
#include "osg/ModeController.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osg2D/MovieKeyHandler.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgWindow/ViewerManager.h"

// ========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;

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
//   bool display_movie_world_time=true;
//   bool display_movie_state=false;
//   bool display_movie_number=false;
   bool display_movie_world_time=false;
   Operations operations(
      ndims,window_mgr_ptr,passes_group,display_movie_state,
      display_movie_number,display_movie_world_time);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   ModeController_ptr->setState(ModeController::RUN_MOVIE);
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   root->addChild(operations.get_OSGgroup_ptr());

// Add a custom manipulator to the event handler list:

   osgGA::Custom2DManipulator* CM_2D_ptr = 
      new osgGA::Custom2DManipulator(ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->set_CameraManipulator(CM_2D_ptr);

// Instantiate group to hold movie:

   MoviesGroup movies_group(
      ndims,passes_group.get_pass_ptr(videopass_ID),
      AnimationController_ptr);
   movies_group.set_extract_frames_flag(true);

   int first_framenumber_to_extract,last_framenumber_to_extract,skip;
   cout << "Enter first framenumber to extract:" << endl;
   cin >> first_framenumber_to_extract;
   cout << "Enter last framenumber to extract:" << endl;
   cin >> last_framenumber_to_extract;
   movies_group.set_first_framenumber_to_extract(
      first_framenumber_to_extract);
   movies_group.set_last_framenumber_to_extract(
      last_framenumber_to_extract);
   cout << "Enter framenumber skip:" << endl;
   cin >> skip;
   movies_group.set_framenumber_skip(skip);

   string movie_filename=
      passes_group.get_videopass_ptr()->get_first_filename();
   texture_rectangle* texture_rectangle_ptr=
      movies_group.generate_new_texture_rectangle(movie_filename);
//   Movie* movie_ptr=
      movies_group.generate_new_Movie(texture_rectangle_ptr);

   root->addChild( movies_group.get_OSGgroup_ptr() );

   MovieKeyHandler* MoviesKeyHandler_ptr=
      new MovieKeyHandler(ModeController_ptr,&movies_group);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(MoviesKeyHandler_ptr);

// Instantiate CentersGroup and center pick handler to handle mid
// point selection:

   CentersGroup centers_group(
      ndims,passes_group.get_pass_ptr(videopass_ID));
   CenterPickHandler* CenterPickHandler_ptr=new CenterPickHandler(
      ndims,passes_group.get_pass_ptr(videopass_ID),
      CM_2D_ptr,&centers_group,ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(CenterPickHandler_ptr);

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

