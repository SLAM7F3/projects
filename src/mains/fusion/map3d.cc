// ========================================================================
// Program MAP3D
// ========================================================================
// Last updated on 9/19/06; 9/25/06; 10/30/06; 11/22/06; 12/19/06
// ========================================================================

#include <string>
#include <vector>
#include "osg/osgGrid/AlirtGridsGroup.h"
#include "osg/AnimationPathCreator.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "osg/osgSceneGraph/ColorMap.h"
#include "osg/Custom2DManipulator.h"
#include "osg/Custom3DManipulator.h"
#include "general/filefuncs.h"
#include "osg/osgGrid/GridKeyHandler.h"
#include "osg/ModeController.h"
#include "osg/ModeKeyHandler.h"
#include "osg/osg2D/MovieKeyHandler.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osg2D/MoviePickHandler.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgfuncs.h"
#include "passes/PassesGroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "general/stringfuncs.h"
#include "osg/osgGraphicals/Transformer.h"
#include "video/VideoController.h"

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
   const int ndims=2;
   string colormap_dir(getenv("OSG_FILE_PATH"));
   colormap_dir += "/3D_colormaps/";
   ColorMap colormap(colormap_dir);

// Read input map video and point cloud data:

   PassesGroup passes_group;
   string map_video_filename="./roadways.vid";
   int mappass_ID=passes_group.add_pass(map_video_filename);

// Construct the viewer and set it up with sensible event handlers:

   osgProducer::Viewer viewer(arguments);
   viewer.setClearColor(osg::Vec4(0,0,0,0));
   viewer.setUpViewer( osgProducer::Viewer::ESCAPE_SETS_DONE );

// Initialize viewer window:

   Producer::RenderSurface* rs_ptr =
      viewer.getCameraConfig()->getCamera(0)->getRenderSurface();
   string window_title="3D ladar imagery";
   osgfunc::initialize_window(rs_ptr,window_title);

// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate a mode controller and mode key event handler:

   ModeController* ModeController_ptr=new ModeController();
   viewer.getEventHandlerList().push_back( 
      new ModeKeyHandler(ModeController_ptr) );
   root->addChild(osgfunc::create_Mode_HUD(ndims,ModeController_ptr));

// Add a custom manipulator to the event handler list:

   osgGA::Custom3DManipulator* CM_3D_ptr=
      new osgGA::Custom3DManipulator(ModeController_ptr);
   window_mgr.set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold "holodeck" grid:

   AlirtGridsGroup alirtgrids_group(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));

   double min_X=-47080.2;
   double max_X=-46929.3;
   double min_Y=45.5;
   double max_Y=993.9;
   double min_Z=-41.80; // meters (for HAFB.xyzp)
   AlirtGrid* grid_ptr=alirtgrids_group.generate_new_Grid(
      min_X,max_X,min_Y,max_Y,min_Z);
   threevector* grid_origin_ptr=grid_ptr->get_world_origin_ptr();
   root->addChild(alirtgrids_group.get_OSGgroup_ptr());

   viewer.getEventHandlerList().push_back(
      new GridKeyHandler(ModeController_ptr,grid_ptr));

// Instantiate a transformer in order to convert between screen and
// world space coordinate systems:

   Transformer transformer(&viewer);

// Instantiate group to hold pointcloud information:

   PointCloudsGroup clouds_group(
      &arguments,passes_group.get_pass_ptr(mappass_ID),
      &colormap,grid_origin_ptr);
   clouds_group.generate_new_Cloud();
   viewer.getEventHandlerList().push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_ptr));
   root->addChild(clouds_group.get_OSGgroup_ptr());

// Instantiate a MyDatabasePager to handle paging of files from disk:

   viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager(
      clouds_group.get_SetupGeomVisitor_ptr(),
      clouds_group.get_ColorGeodeVisitor_ptr());

// Instantiate a map image movie display:

   MoviesGroup mapimage_group(3,passes_group.get_pass_ptr(mappass_ID));
   Movie* mapimage_ptr=mapimage_group.generate_new_Movie();
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
   viewer.getEventHandlerList().push_back(MoviePickHandler_ptr);

   viewer.getEventHandlerList().push_back( 
      new MovieKeyHandler(mapimage_ptr,ModeController_ptr,
                          mapimage_controller_ptr,&mapimage_group));

// Instantiate CentersGroup and center pick handler to handle mid
// point selection:

   CentersGroup centers_group(
      3,passes_group.get_pass_ptr(mappass_ID));
   CenterPickHandler* CenterPickHandler_ptr=new CenterPickHandler(
      3,passes_group.get_pass_ptr(mappass_ID),
      CM_3D_ptr,&centers_group,ModeController_ptr,
      &transformer,grid_origin_ptr);
   viewer.getEventHandlerList().push_back(CenterPickHandler_ptr);

// Set scale, rotation and translation info needed to transform movie
// window so that it lies approximately within the z-grid:

   threevector grid_origin=grid_ptr->get_world_origin();
   mapimage_ptr->project_onto_worldspace_grid(0,mappass_ID,grid_origin);

// Attach the scene graphs to the viewer:

   viewer.setSceneData( root );

// Create the windows and run the threads:

   viewer.realize();

// Add an animation path creator to the event handler list AFTER the
// viewer has been realized:

   AnimationPathCreator* animation_path_handler = 
      new AnimationPathCreator(&viewer);
   viewer.getEventHandlerList().push_back(animation_path_handler);

   viewer.getUsage(*arguments.getApplicationUsage());

   while (!viewer.done())
   {
      viewer.sync();
      viewer.update();
      viewer.frame();
   }

   viewer.sync();

   return 0;
}

