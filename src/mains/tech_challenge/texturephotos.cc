// ========================================================================
// Program TEXTUREPHOTOS is a playground which displays a dynamic
// LatLongGrid underneath a satellite image texture in 3D worldspace.
// It also visualizes 3D OBSFRUSTA whose parameters can all come from
// hardware (rather than expensive photosynth computations).

//  	   texturephotos --surface_texture ./packages/LL_EO.pkg

/*

/home/cho/programs/c++/svn/projects/src/mains/tech_challenge/texturephotos \
--surface_texture ./packages/LL_EO.pkg \
--region_filename ./packages/photo_00412.pkg \
--region_filename ./packages/photo_00414.pkg \
--initial_mode Manipulate_Fused_Data_Mode

*/


// ========================================================================
// Last updated on 8/8/10; 8/11/10
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
#include "astro_geo/geopoint.h"
#include "osg/osgGrid/GridKeyHandler.h"
#include "osg/osgGrid/LatLongGridsGroup.h"
#include "osg/ModeController.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "osg/osgGIS/postgis_database.h"
#include "general/stringfuncs.h"
#include "osg/osg3D/Terrain_Manipulator.h"
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

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   int texturepass_ID=passes_group.get_curr_texturepass_ID();
   int videopass_ID=passes_group.get_videopass_ID();

   cout << "texturepass_ID = " << texturepass_ID << endl;
   cout << "videopass_ID = " << videopass_ID << endl;
   
// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group);
   int n_photos=photogroup_ptr->get_n_photos();
   cout << "n_photos = " << n_photos << endl;

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
   Operations operations(ndims,window_mgr_ptr,passes_group,display_movie_state,
                         display_movie_number);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   root->addChild(operations.get_OSGgroup_ptr());

// Add a custom manipulator to the event handler list:

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate groups to hold multiple surface textures,
// latitude-longitude grids and associated earth regions:

   LatLongGridsGroup latlonggrids_group(
      ndims,passes_group.get_pass_ptr(texturepass_ID),CM_3D_ptr);
   EarthRegionsGroup earth_regions_group(
      passes_group.get_pass_ptr(texturepass_ID),&latlonggrids_group);

   bool place_onto_bluemarble_flag=false;
   bool generate_pointcloud_LatLongGrid_flag=false;
   bool display_SurfaceTexture_LatLongGrid_flag=true;
//   bool display_SurfaceTexture_LatLongGrid_flag=false;
   earth_regions_group.generate_regions(
      passes_group,place_onto_bluemarble_flag,
      generate_pointcloud_LatLongGrid_flag,
      display_SurfaceTexture_LatLongGrid_flag);
   
// Display latlonggrid underneath a surface texture only for algorithm
// development purposes:

   LatLongGrid* latlonggrid_ptr=latlonggrids_group.get_Grid_ptr(0);
   threevector* grid_origin_ptr=latlonggrid_ptr->get_world_origin_ptr();

   CM_3D_ptr->set_Grid_ptr(latlonggrid_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new GridKeyHandler(ModeController_ptr,latlonggrid_ptr));

// Instantiate group to hold all decorations:

   Decorations decorations(
      window_mgr_ptr,ModeController_ptr,CM_3D_ptr,grid_origin_ptr);
   decorations.set_grid_origin_ptr(grid_origin_ptr);

// Instantiate signposts, features, army symbol and sphere segments
// decoration groups:

   decorations.add_SignPosts(ndims,passes_group.get_pass_ptr(texturepass_ID));
   SignPostsGroup* SignPostsGroup_ptr=decorations.get_SignPostsGroup_ptr(0);
   SignPostsGroup_ptr->set_EarthRegionsGroup_ptr(&earth_regions_group);
   SignPostsGroup_ptr->set_Clock_ptr(&clock);
   SignPostsGroup_ptr->set_common_geometrical_size(100);

// Instantiate OBSFRUSTA decoration group:

   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr=decorations.add_OBSFRUSTA(
      passes_group.get_pass_ptr(videopass_ID),AnimationController_ptr);

// Instantiate an individual OBSFRUSTUM for every still image:

   bool multicolor_frusta_flag=false;
   bool thumbnails_flag=false;
//   bool thumbnails_flag=true;
   OBSFRUSTAGROUP_ptr->generate_still_imagery_frusta_for_photogroup(
      photogroup_ptr,multicolor_frusta_flag,thumbnails_flag);


// Attach scene graph to the viewer:

   root->addChild(latlonggrids_group.get_OSGgroup_ptr());
   root->addChild(earth_regions_group.get_OSGgroup_ptr());
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
