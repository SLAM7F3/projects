// ========================================================================
// Program RAYTRACE computes the line-of-sight between a moving
// airborne transmitter and a fixed ground receiver.  The ground takes
// the form of a DTED surface map (and not point cloud!).  The
// aircraft's flight path can be manually entered as a polyline above
// the DTED map.  The the ground receiver can also be manually placed
// anywhere on the DTED map.  This program then evenly samples the
// airborne transmitter's position over time and animates its motion.
// If the instantaneous line-of-sight between the transmitter and
// receiver is blocked, the line segment is colored red, and it
// terminates at the point where the segment pierces the ground.  On
// the other hand, if the line-of-sight is clear, the segment is
// colored green.  For a given flight path, this program computes the
// clear LOS percentage.  

// ~cho/programs/c++/svn/projects/src/mains/OSG/raytrace puget_l1.osga

// 		raytrace --region_filename ./packages/dted.pkg


// Press "A" to enter into INSERT ANNOTATION mode.  Then place the
// ground receiver (represented as a signpost) at the desired location
// on the DTED map

// Press "J" twice to enter into MANIPULATE MODEL mode.  Then press
// up/down arrows to adjust aircraft's altitude.  Drag Cessna model
// (which starts near (0,0) on the world grid) over the DTED map with
// the mouse.  Press 'p' to mark desired waypoints along the Cessna's
// flight path.  After the last waypoint has been entered, press 'f'
// to finish waypoint entry.  A light white line should then mark the
// Cessna's flight path above the DTED surface.

// Press "R" to enter into RUN MOVIE mode.  Press'p' to begin
// animating the Cessna and computing the lines-of-sight.  Press
// right/left arrows to step through individual frames in the animation.

// ========================================================================
// Last updated on 6/19/08; 6/27/08; 7/9/08; 7/10/08; 9/7/08
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "astro_geo/Clock.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgSceneGraph/HiresDataVisitor.h"
#include "osg/ModeController.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osg3D/RayTracer.h"
#include "osg/osg3D/RayTraceKeyHandler.h"
#include "osg/osgSceneGraph/SetupGeomVisitor.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgWindow/ViewerManager.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ifstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);
   
// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();

// Instantiate clock to keep track of real time:

   Clock clock;
   clock.current_local_time_and_UTC();

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("3D imagery");
//   window_mgr_ptr->getUsage(*arguments.getApplicationUsage());

// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

   bool display_movie_state=true;
   bool display_movie_number=true;
   bool display_movie_world_time=true;
   Operations operations(ndims,window_mgr_ptr,display_movie_state,
                         display_movie_number,display_movie_world_time);
   root->addChild(operations.get_OSGgroup_ptr());

   operations.get_ImageNumberHUD_ptr()->set_text_color(colorfunc::red);
   operations.get_ImageNumberHUD_ptr()->reset_text_size(1.1 , 0);
   operations.get_ImageNumberHUD_ptr()->reset_text_size(1.1 , 1);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();

// Instantiate AnimationController which acts as master game clock:

   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
//   AnimationController_ptr->setDelay(0);

// Specify start, stop and step times for master game clock:

   bool start_time_flag=true;
   operations.set_master_world_UTC(2007,9,27,16,8,54,start_time_flag);
   start_time_flag=false;
   operations.set_master_world_UTC(2007,9,27,16,59,0,start_time_flag);

   operations.set_delta_master_world_time_step_per_master_frame(0.5); // secs
   AnimationController_ptr->set_world_time_params(
      operations.get_master_world_start_time(),
      operations.get_master_world_stop_time(),
      operations.get_delta_master_world_time_step_per_master_frame());

// Add a custom manipulator to the event handler list:

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold all decorations:

   Decorations decorations(window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

// Instantiate group to hold "holodeck" grid:

   AlirtGrid* grid_ptr=decorations.add_AlirtGrid(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   threevector* grid_origin_ptr=grid_ptr->get_world_origin_ptr();
   CM_3D_ptr->set_Grid_ptr(grid_ptr);
   decorations.set_grid_origin_ptr(grid_origin_ptr);

// Instantiate group to hold pointcloud information:

   PointCloudsGroup clouds_group(
      passes_group.get_pass_ptr(cloudpass_ID),grid_origin_ptr);
   PointCloud* cloud_ptr=clouds_group.generate_new_Cloud();
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_ptr));
   root->addChild(clouds_group.get_OSGgroup_ptr());

// Instantiate a PointFinder;

   PointFinder pointfinder(&clouds_group);
   CM_3D_ptr->set_PointFinder(&pointfinder);

// Initialize ALIRT grid based upon cloud's bounding box:

   decorations.get_AlirtGridsGroup_ptr()->initialize_grid(
      grid_ptr,clouds_group.get_xyz_bbox());

// Instantiate a MyDatabasePager to handle paging of files from disk:

   viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager(
      clouds_group.get_SetupGeomVisitor_ptr(),
      clouds_group.get_ColorGeodeVisitor_ptr());
   clouds_group.get_HiresDataVisitor_ptr()->setDatabasePager(
      MyDatabasePager_ptr);

// Instantiate SignPost, LineSegment, PolyLine and MODEL decorations
// groups:

   decorations.add_SignPosts(ndims,passes_group.get_pass_ptr(cloudpass_ID));
   decorations.add_LineSegments(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);

   PolyLinesGroup* Flight_PolyLinesGroup_ptr=decorations.add_PolyLines(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   PolyLinePickHandler* Flight_PolyLinePickHandler_ptr=
      decorations.get_PolyLinePickHandler_ptr();
   Flight_PolyLinePickHandler_ptr->set_permanent_color(colorfunc::cyan);

   MODELSGROUP* Predator_MODELSGROUP_ptr=decorations.add_MODELS(
      passes_group.get_pass_ptr(cloudpass_ID),
      Flight_PolyLinesGroup_ptr,Flight_PolyLinePickHandler_ptr,
      AnimationController_ptr);
//   Predator_MODELSGROUP_ptr->set_aircraft_altitude(300);
   Predator_MODELSGROUP_ptr->set_aircraft_altitude(200);

// Instantiate a RayTracer to determine where LOS from aerial
// transmitter to ground receiver pierces DTED surface map:

   RayTracer* RayTracer_ptr=new RayTracer(
      passes_group.get_pass_ptr(cloudpass_ID),
      decorations.get_SignPostsGroup_ptr(0),
      Predator_MODELSGROUP_ptr,
      decorations.get_LineSegmentsGroup_ptr(),
      cloud_ptr,clouds_group.get_TreeVisitor_ptr(),
      AnimationController_ptr,grid_origin_ptr);
   root->addChild(RayTracer_ptr->get_OSGgroup_ptr());

   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new RayTraceKeyHandler(ModeController_ptr,RayTracer_ptr));

// Attach scene graph to viewer:

   root->addChild(decorations.get_OSGgroup_ptr());
   window_mgr_ptr->setSceneData(root);

// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_ptr->realize();

   while( !window_mgr_ptr->done() )
   {
      window_mgr_ptr->process();
   }

   delete window_mgr_ptr;
}

