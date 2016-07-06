// ========================================================================
// Program LADARRAYTRACE

/*

cd ~/programs/c++/svn/projects/src/mains/afghanistan/
./ladarraytrace \
--region_filename ./packages/FOB.pkg \
--world_start_UTC 2010,7,10,10,6,3 \
--world_stop_UTC 2010,7,10,10,6,46 \
--world_time_step 0.2 \
--initial_mode Manipulate_Fused_Data_Mode 

*/

// ========================================================================
// Last updated on 4/18/11; 4/19/11; 4/23/11; 4/24/11
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include "osg/osgGraphicals/AnimationController.h"
#include "video/camera.h"
#include "osg/CompassHUD.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "osg/osgGrid/GridKeyHandler.h"
#include "osg/osgSceneGraph/HiresDataVisitor.h"
#include "osg/osgGrid/LatLongGridsGroup.h"
#include "osg/ModeController.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osg2D/MovieKeyHandler.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "numrec/nrfuncs.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "osg/osgAnnotators/SignPostsGroup.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgTiles/TilesGroup.h"
#include "osg/osgWindow/ViewerManager.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::string;
using std::vector;

// ==========================================================================
int main( int argc, char** argv )
{
   
   nrfunc::init_time_based_seed();

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
   int videopass_ID=passes_group.get_videopass_ID();
   vector<int> GISlayer_IDs=passes_group.get_GISlayer_IDs();
//   cout << "cloudpass_ID = " << cloudpass_ID << endl;
//   cout << "videopass_ID = " << videopass_ID << endl;

// Instantiate postgis database objects to send data to and retrieve
// data from external Postgres database:

   postgis_databases_group* postgis_databases_group_ptr=
      new postgis_databases_group;
   postgis_database* postgis_db_ptr=postgis_databases_group_ptr->
      generate_postgis_database_from_GISlayer_IDs(passes_group,GISlayer_IDs);

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("3D imagery");
//   window_mgr_ptr->getUsage(*arguments.getApplicationUsage());

// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

//   bool display_movie_state=true;
//   bool display_movie_number=true;
//   bool display_movie_world_time=true;
   bool display_movie_state=false;		// viewgraphs
   bool display_movie_number=false;		// viewgraphs
   bool display_movie_world_time=false;       // viewgraphs
   bool hide_Mode_HUD_flag=false;
//   bool hide_Mode_HUD_flag=true;

   Operations operations(
      ndims,window_mgr_ptr,passes_group,display_movie_state,
      display_movie_number,display_movie_world_time,hide_Mode_HUD_flag);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();

// Instantiate AnimationController which acts as master game clock:

   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   AnimationController_ptr->setDelay(0.1);

// Specify start, stop and step times for master game clock:

   operations.set_master_world_start_UTC(
      passes_group.get_world_start_UTC_string());
   operations.set_master_world_stop_UTC(
      passes_group.get_world_stop_UTC_string());
   operations.set_delta_master_world_time_step_per_master_frame(
      passes_group.get_world_time_step());

   AnimationController_ptr->set_world_time_params(
      operations.get_master_world_start_time(),
      operations.get_master_world_stop_time(),
      operations.get_delta_master_world_time_step_per_master_frame());

// Add a custom manipulator to the event handler list:

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

   HiresDataVisitor* HiresDataVisitor_ptr=new HiresDataVisitor();

// Instantiate compass:

   CompassHUD* CompassHUD_ptr=new CompassHUD(colorfunc::white);
   root->addChild(CompassHUD_ptr->getProjection());
   CM_3D_ptr->set_CompassHUD_ptr(CompassHUD_ptr);

// Instantiate group to hold all decorations:
   
   Decorations decorations(
      window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

// Instantiate groups to hold multiple point clouds,
// latitude-longitude grids and associated earth regions:

   PointCloudsGroup clouds_group(
      passes_group.get_pass_ptr(cloudpass_ID));
   clouds_group.set_Terrain_Manipulator_ptr(CM_3D_ptr);
   clouds_group.set_auto_resize_points_flag(false);
//   clouds_group.set_auto_resize_points_flag(true);
   decorations.set_PointCloudsGroup_ptr(&clouds_group);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_ptr,CM_3D_ptr));

   LatLongGridsGroup latlonggrids_group(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),CM_3D_ptr);
   EarthRegionsGroup earth_regions_group(
      passes_group.get_pass_ptr(cloudpass_ID),      
      &clouds_group,&latlonggrids_group);
   earth_regions_group.generate_regions(passes_group);

   LatLongGrid* LatLongGrid_ptr=latlonggrids_group.get_Grid_ptr(0);
   threevector* grid_origin_ptr=LatLongGrid_ptr->get_world_origin_ptr();
//   cout << "*grid_origin_ptr = " << *grid_origin_ptr << endl;
   CM_3D_ptr->set_Grid_ptr(LatLongGrid_ptr);

   decorations.set_grid_origin_ptr(grid_origin_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new GridKeyHandler(ModeController_ptr,LatLongGrid_ptr));

   double xmin=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().xMin();
   double xmax=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().xMax();
   double ymin=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().yMin();
   double ymax=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().yMax();
   if (postgis_db_ptr != NULL)
      postgis_db_ptr->pushback_gis_bbox(xmin,xmax,ymin,ymax);

// Instantiate a MyDatabasePager to handle paging of files from disk:

   viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager(
      clouds_group.get_SetupGeomVisitor_ptr(),
      clouds_group.get_ColorGeodeVisitor_ptr());
   clouds_group.get_HiresDataVisitor_ptr()->setDatabasePager(
      MyDatabasePager_ptr);

// Instantiate OBSFRUSTAGROUP decoration group:

   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr=decorations.add_OBSFRUSTA(
      passes_group.get_pass_ptr(0),AnimationController_ptr);

// Instantiate group to hold movie:

   MoviesGroup movies_group(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),
      AnimationController_ptr);

// Instantiate a PointFinder;

   PointFinder pointfinder(&clouds_group);
   CM_3D_ptr->set_PointFinder(&pointfinder);

// Instantiate SignPostsGroup to hold ground target locations:

   SignPostsGroup* GroundTarget_SignPostsGroup_ptr=
      decorations.add_SignPosts(ndims,passes_group.get_pass_ptr(cloudpass_ID));

// Instantiate LOSMODELSGROUP decoration groups:

   LOSMODELSGROUP* Aircraft_MODELSGROUP_ptr=decorations.add_LOSMODELS(
      passes_group.get_pass_ptr(cloudpass_ID),
      NULL,NULL,&operations,&movies_group);
   Aircraft_MODELSGROUP_ptr->set_AircraftModelType(MODELSGROUP::LiMIT);
   Aircraft_MODELSGROUP_ptr->set_EarthRegionsGroup_ptr(&earth_regions_group);
   Aircraft_MODELSGROUP_ptr->set_CM_3D_ptr(CM_3D_ptr);
   Aircraft_MODELSGROUP_ptr->set_ColorGeodeVisitor_ptr(
      clouds_group.get_ColorGeodeVisitor_ptr());
   Aircraft_MODELSGROUP_ptr->set_compute_zface_height_flag(false);
   Aircraft_MODELSGROUP_ptr->set_GroundTarget_SignPostsGroup_ptr(
      GroundTarget_SignPostsGroup_ptr);
   Aircraft_MODELSGROUP_ptr->set_PointCloudsGroup_ptr(&clouds_group);
   Aircraft_MODELSGROUP_ptr->set_raytrace_ground_targets_flag(false);
   Aircraft_MODELSGROUP_ptr->set_ladar_height_data_flag(true);

   double min_ground_sensor_range=0*1000;	// meters
//   double max_ground_sensor_range=2*1000;	// meters
   double max_ground_sensor_range=50*1000;	// meters
   MODEL* empty_MODEL_ptr=Aircraft_MODELSGROUP_ptr->generate_empty_MODEL(
      min_ground_sensor_range,max_ground_sensor_range);
   empty_MODEL_ptr->set_OBSFRUSTAGROUP_ptr(OBSFRUSTAGROUP_ptr);
   empty_MODEL_ptr->set_raytrace_occluded_ground_regions_flag(true);
   Aircraft_MODELSGROUP_ptr->set_OSGsubPAT_nodemask(1,0);

// Instantiate TilesGroup to perform raytracing computations:

   TilesGroup* TilesGroup_ptr=new TilesGroup();
   string map_countries_name="FOB_Blessing";
   string geotif_subdir="/data/DTED/"+map_countries_name+"/geotif/";
   TilesGroup_ptr->set_geotif_subdir(geotif_subdir);
   TilesGroup_ptr->set_ladar_height_data_flag(true);

   Aircraft_MODELSGROUP_ptr->set_TilesGroup_ptr(TilesGroup_ptr);
   TilesGroup_ptr->purge_tile_files();

// Set fixed_to_mutable_colors_flag=true for ladar coloring purposes:

   clouds_group.get_ColorGeodeVisitor_ptr()->
      set_fixed_to_mutable_colors_flag(true);

// Optimize the scene graph, remove redundent nodes and states, and
// then attach it to the viewer:

   root->addChild(operations.get_OSGgroup_ptr());
   root->addChild(decorations.get_OSGgroup_ptr());
   root->addChild( movies_group.get_OSGgroup_ptr() );
   root->addChild(clouds_group.get_OSGgroup_ptr());
   root->addChild(latlonggrids_group.get_OSGgroup_ptr());

   window_mgr_ptr->setSceneData(root);

// HAFB minimap [Afghanistan tile 3_3 ]has very [moderately] dark intensity
// values.  So for intensity display purposes, we artificially
// brighten all p-values by the following intensity magnification
// factor:

   double intensity_magnification=5;	// HAFB mini map
//   double intensity_magnification=1.3;	// FOB tile 3.3
//   cout << "Enter intensity magnification factor:" << endl;
//   cin >> intensity_magnification;

   clouds_group.get_ColorGeodeVisitor_ptr()->
      set_probabilities_magnification(intensity_magnification);

// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_ptr->realize();

   while( !window_mgr_ptr->done() )
   {
      window_mgr_ptr->process();
   }

   delete window_mgr_ptr;
}

