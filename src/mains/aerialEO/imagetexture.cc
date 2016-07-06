// ========================================================================
// Program IMAGETEXTURE is a playground which places an image with
// specified geocoordinate corners onto a LatLongGrid whose extents
// are independently specified.

//  	   imagetexture --surface_texture ./packages/CH.pkg 

// ========================================================================
// Last updated on 5/6/09; 5/20/09; 5/23/09; 8/14/09
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include "osg/osgGraphicals/AnimationController.h"
#include "astro_geo/Clock.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "geometry/ellipse.h"
#include "astro_geo/geopoint.h"
#include "osg/osgGrid/GridKeyHandler.h"
#include "osg/osgGrid/LatLongGridsGroup.h"
#include "osg/ModeController.h"
#include "osg/osgModels/MODELSGROUP.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osg2D/MovieKeyHandler.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGIS/postgis_database.h"
#include "general/stringfuncs.h"
#include "passes/TextDialogBox.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "time/timefuncs.h"
#include "osg/osgWindow/ViewerManager.h"

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

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   int texturepass_ID=passes_group.get_curr_texturepass_ID();
   int videopass_ID=passes_group.get_videopass_ID();

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("3D imagery");
//   window_mgr_ptr->getUsage(*arguments.getApplicationUsage());

// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate clock to keep track of real time:

   Clock clock;
   clock.current_local_time_and_UTC();

// Instantiate Operations object to handle mode, animation and image
// number control:

   bool display_movie_state=false;
   bool display_movie_number=false;
   Operations operations(ndims,window_mgr_ptr,display_movie_state,
                         display_movie_number);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   root->addChild(operations.get_OSGgroup_ptr());

// Add a custom manipulator to the event handler list:

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold movie:

   AnimationController* movie_anim_controller_ptr=new AnimationController;
   movie_anim_controller_ptr->set_master_AnimationController_ptr(
      AnimationController_ptr);
   MoviesGroup movies_group(
      ndims,passes_group.get_pass_ptr(texturepass_ID),
      movie_anim_controller_ptr);
   
   movies_group.set_OSGgroup_nodemask(1);
   root->addChild( movies_group.get_OSGgroup_ptr() );

   MovieKeyHandler* MoviesKeyHandler_ptr=
      new MovieKeyHandler(ModeController_ptr,&movies_group);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(MoviesKeyHandler_ptr);

// Instantiate groups to hold multiple surface textures,
// latitude-longitude grids and associated earth regions:

   LatLongGridsGroup latlonggrids_group(
      ndims,passes_group.get_pass_ptr(texturepass_ID),CM_3D_ptr);
   root->addChild(latlonggrids_group.get_OSGgroup_ptr());

   EarthRegionsGroup earth_regions_group(
      passes_group.get_pass_ptr(texturepass_ID),&latlonggrids_group,
      &movies_group);
   earth_regions_group.set_movie_alpha(1.0);
   earth_regions_group.generate_regions(passes_group);
   root->addChild(earth_regions_group.get_OSGgroup_ptr());

// Display latlonggrid underneath a surface texture only for algorithm
// development purposes:

   LatLongGrid* latlonggrid_ptr=latlonggrids_group.get_Grid_ptr(0);
   threevector* grid_origin_ptr=latlonggrid_ptr->get_world_origin_ptr();

   CM_3D_ptr->set_Grid_ptr(latlonggrid_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new GridKeyHandler(ModeController_ptr,latlonggrid_ptr));

// Instantiate a PointFinder;

   PointFinder pointfinder;
   CM_3D_ptr->set_PointFinder(&pointfinder);

// Instantiate group to hold all decorations:

   Decorations decorations(
      window_mgr_ptr,ModeController_ptr,CM_3D_ptr,grid_origin_ptr);

// Attach scene graph to the viewer:

   root->addChild(decorations.get_OSGgroup_ptr());
   window_mgr_ptr->setSceneData(root);

// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_ptr->realize();

//   outputfunc::enter_continue_char();

   while( !window_mgr_ptr->done() )
   {
      window_mgr_ptr->process();
   }

   delete window_mgr_ptr;
}
