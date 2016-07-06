// ========================================================================
// Program VIDEO3D takes in a G99 .vid file as a command line
// argument.  It reads in aircraft position and attitude information
// as functions of time from "TPA_filtered.txt".  The locations of
// video frames' corners projected onto a rectangle that lies just
// barely below the world grid (which were assumed to have been
// previously calculated by main program BACKPROJECT) is also read in
// from "UV_corners.txt".

// VIDEO3D displays the world grid and uses a blue-colored version of
// the OSG Cessna model to display the aircraft's location and
// attitude.  It draws 4 pyramidal lines from the aircraft's location
// down to the instantaneous rectangular field-of-view.  The video
// sequence is displayed within the moving rectangle which lies only
// approximately within the z-plane of the world grid.  This output
// demonstrates that sensor motion can be (approximately) removed and
// video can be stabilized by projecting it into world-space.

// From within src/mains/fusion, chant 

// 		    video3d HAFB_overlap_corrected_grey.vid

// to see best example of this program.

// ========================================================================
// Last updated on 9/21/07; 10/11/07; 10/15/07
// ========================================================================

#include <string>
#include <vector>

#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgGrid/GridKeyHandler.h"
#include "osg/ModeController.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgWindow/ViewerManager.h"

#include "general/outputfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

// ========================================================================
int main( int argc, char** argv )
{

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   int videopass_ID=passes_group.get_videopass_ID();

//   bool include_map_underlay=true;
//   bool include_map_underlay=false;
   string map_video_filename="./just_roads.vid";
//   int mappass_ID=passes_group.add_pass(map_video_filename);

// Construct the viewer and instantiate a ViewerManager:

   osgProducer::Viewer viewer(arguments);
   WindowManager* window_mgr_ptr=new ViewerManager(&viewer);
   window_mgr_ptr->initialize_window("3D imagery");
//   viewer.getUsage(*arguments.getApplicationUsage());

// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

   bool display_movie_state=true;
   bool display_movie_number=true;
   Operations operations(ndims,window_mgr_ptr,display_movie_state,
                         display_movie_number);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   AnimationController_ptr->set_nframes(289);
   root->addChild(operations.get_OSGgroup_ptr());

// Add a custom manipulator to the event handler list:

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold all decorations:
   
   Decorations decorations(
      window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

// Instantiate AlirtGrid decorations group:

   AlirtGrid* grid_ptr=decorations.add_AlirtGrid(
      ndims,passes_group.get_pass_ptr(videopass_ID));
   threevector* grid_origin_ptr=grid_ptr->get_world_origin_ptr();
   CM_3D_ptr->set_Grid_ptr(grid_ptr);
   decorations.set_grid_origin_ptr(grid_origin_ptr);

   double min_X=-47763.3;
   double max_X=-46027.9;
   double min_Y=-742.8;
   double max_Y=2363.5;
   double min_Z=-41.80; // meters (for HAFB.xyzp)
   decorations.get_AlirtGridsGroup_ptr()->initialize_grid(
      grid_ptr,min_X,max_X,min_Y,max_Y,min_Z);
   grid_ptr->set_delta_xy(250,250);
//   cout << "*grid_origin_ptr = " << *grid_origin_ptr << endl;

// Instantiate a PointFinder;

   PointFinder pointfinder;
   CM_3D_ptr->set_PointFinder(&pointfinder);



/*
   Movie* mapimage_ptr=NULL;
   if (include_map_underlay)
   {

// Instantiate a map image movie display:

      MoviesGroup mapimage_group(3,passes_group.get_pass_ptr(mappass_ID));
      double alpha=0.45;
      mapimage_ptr=mapimage_group.generate_new_Movie(
         map_video_filename,alpha);
      mapimage_ptr->set_PAT_pivot(mapimage_ptr->get_midpoint());

      VideoController* mapimage_controller_ptr = new VideoController(
         3,passes_group.get_pass_ptr(mappass_ID),mapimage_ptr,
         &mapimage_group);
      mapimage_group.get_OSGgroup_ptr()->setUpdateCallback(
         new AbstractOSGCallback<VideoController>(
            mapimage_controller_ptr, &VideoController::update));
      root->addChild( mapimage_group.get_OSGgroup_ptr() );

// Add a movie pick and key event handler to deal with the map image
// display:

      MoviePickHandler* MoviePickHandler_ptr=new MoviePickHandler(
         3,passes_group.get_pass_ptr(mappass_ID),CM_3D_ptr,
         &mapimage_group,ModeController_ptr);
      window_mgr_ptr->get_EventHandlers_ptr()->push_back(MoviePickHandler_ptr);

      window_mgr_ptr->get_EventHandlers_ptr()->push_back( 
         new MovieKeyHandler(mapimage_ptr,ModeController_ptr,
                             mapimage_controller_ptr,&mapimage_group));

//      cout << "&movies_group = " << &movies_group << endl;
//      cout << "&mapimage_group = " << &mapimage_group << endl;
//      outputfunc::enter_continue_char();
   }
*/
   
// Instantiate model decorations group.  Read in aircraft filtered
// position and attitude as functions of time.  Then generate
// ObsFrustum with HAFB video pass displayed at its base:

   decorations.add_Models(
      passes_group.get_pass_ptr(videopass_ID),AnimationController_ptr);
   double z_rot_angle=0;
   threevector first_frame_aircraft_posn(-47657.47+500,2300.739-600);
   decorations.get_ModelsGroup_ptr()->generate_HAFB_video_pass_model(
      z_rot_angle,first_frame_aircraft_posn,AnimationController_ptr);

// Instantiate CentersGroup and center pick handler to handle mid
// point selection:

   CentersGroup centers_group(
      3,passes_group.get_pass_ptr(videopass_ID));
   CenterPickHandler* CenterPickHandler_ptr=new CenterPickHandler(
      3,passes_group.get_pass_ptr(videopass_ID),
      CM_3D_ptr,&centers_group,ModeController_ptr,window_mgr_ptr,
      grid_origin_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      CenterPickHandler_ptr);

// Attach the scene graphs to the viewer:

   root->addChild(decorations.get_OSGgroup_ptr());
   viewer.setSceneData( root );

// Create the windows and run the threads.  Viewer's realize method
// calls the CUstomManipulator's home() method:

   viewer.realize();

   while (!viewer.done())
   {
      window_mgr_ptr->process();
   }

}

