// ========================================================================
// Program PTRAYTRACE performs line-of-sight raytracing from any source
// location within a 3D ALIRT map.  The user selects the transmitter location.
// PTRAYTRACE then checks line-of-sight visibility to each ground
// point within a certain radius of the transmitter.  
// After performing LOST-like raytracing, terrain regions around the
// transmitter which are visible {partially occluded} [occluded] 
// are shaded in green {yellow,orange} [red].  Terrain locations
// outside some radius beyond the transmitter are shaded grey.

/*

./ptraytrace \
--region_filename ./packages/miniHAFB.pkg \
--initial_mode Manipulate_Fused_Data_Mode 

*/

// ========================================================================
// Last updated on 6/30/11; 7/1/11; 7/3/11
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include "osg/CompassHUD.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "osg/osgGrid/GridKeyHandler.h"
#include "osg/osgSceneGraph/HiresDataVisitor.h"
#include "osg/osgGrid/LatLongGridsGroup.h"
#include "osg/ModeController.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgTiles/ray_tracer.h"
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

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
//   cout << "cloudpass_ID = " << cloudpass_ID << endl;

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
//   clouds_group.set_auto_resize_points_flag(false);
   clouds_group.set_auto_resize_points_flag(true);
   decorations.set_PointCloudsGroup_ptr(&clouds_group);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_ptr,CM_3D_ptr));

   LatLongGridsGroup latlonggrids_group(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),CM_3D_ptr);
   EarthRegionsGroup earth_regions_group(
      passes_group.get_pass_ptr(cloudpass_ID),      
      &clouds_group,&latlonggrids_group);
   earth_regions_group.generate_regions(passes_group);
   double Zmax=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().zMax();
   cout << "Zmax = " << Zmax << endl;

   LatLongGrid* LatLongGrid_ptr=latlonggrids_group.get_Grid_ptr(0);
   threevector* grid_origin_ptr=LatLongGrid_ptr->get_world_origin_ptr();
//   cout << "*grid_origin_ptr = " << *grid_origin_ptr << endl;
   CM_3D_ptr->set_Grid_ptr(LatLongGrid_ptr);

   decorations.set_grid_origin_ptr(grid_origin_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new GridKeyHandler(ModeController_ptr,LatLongGrid_ptr));

// Instantiate a PointFinder;

   PointFinder pointfinder(&clouds_group);
   CM_3D_ptr->set_PointFinder(&pointfinder);
   
// Instantiate SignPost decoration group:

   SignPostsGroup* SignPostsGroup_ptr=
      decorations.add_SignPosts(ndims,passes_group.get_pass_ptr(cloudpass_ID));
   SignPostsGroup_ptr->set_ladar_height_data_flag(true);
   SignPostsGroup_ptr->set_altitude_dependent_size_flag(false);
   SignPostsGroup_ptr->set_raytrace_visible_terrain_flag(true);
   SignPostsGroup_ptr->set_CM_3D_ptr(CM_3D_ptr);
   SignPostsGroup_ptr->set_ColorGeodeVisitor_ptr(
      clouds_group.get_ColorGeodeVisitor_ptr());

// Instantiate Feature decoration group:

   decorations.add_Features(ndims,passes_group.get_pass_ptr(cloudpass_ID));
   decorations.get_FeaturePickHandler_ptr()->
      set_surface_picking_flag(false);
   decorations.get_FeaturePickHandler_ptr()->
      set_Zplane_picking_flag(false);

// Instantiate a MyDatabasePager to handle paging of files from disk:

   viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager(
      clouds_group.get_SetupGeomVisitor_ptr(),
      clouds_group.get_ColorGeodeVisitor_ptr());
   clouds_group.get_HiresDataVisitor_ptr()->setDatabasePager(
      MyDatabasePager_ptr);

// Instantiate TilesGroup to perform raytracing computations:

   TilesGroup* TilesGroup_ptr=new TilesGroup();
   string map_countries_name="HAFB";
   string geotif_subdir="/data/DTED/"+map_countries_name+"/geotif/";
   TilesGroup_ptr->set_geotif_subdir(geotif_subdir);
   TilesGroup_ptr->set_ladar_height_data_flag(true);
   TilesGroup_ptr->purge_tile_files();

   string ladar_tile_filename=geotif_subdir+"Ztiles/"+
      "larger_flightfacility_ladarmap.tif";
   TilesGroup_ptr->load_ladar_height_data_into_ztwoDarray(ladar_tile_filename);

   double min_ground_sensor_range=0*1000;	// meters
   double max_ground_sensor_range=1*1000;	// meters

   ray_tracer* ray_tracer_ptr=new ray_tracer();
   ray_tracer_ptr->set_max_zground(Zmax);
   ray_tracer_ptr->set_TilesGroup_ptr(TilesGroup_ptr);
   SignPostsGroup_ptr->set_raytracer_ptr(ray_tracer_ptr);

// Set fixed_to_mutable_colors_flag=true for ladar coloring purposes:

   clouds_group.get_ColorGeodeVisitor_ptr()->
      set_fixed_to_mutable_colors_flag(true);

// Optimize the scene graph, remove redundent nodes and states, and
// then attach it to the viewer:

   root->addChild(operations.get_OSGgroup_ptr());
   root->addChild(decorations.get_OSGgroup_ptr());
   root->addChild(clouds_group.get_OSGgroup_ptr());
   root->addChild(latlonggrids_group.get_OSGgroup_ptr());

   window_mgr_ptr->setSceneData(root);

// The HAFB ALIRT minimap has very dark intensity values.  So for
// intensity display purposes, we artificially brighten all p-values
// by the following intensity magnification factor:

//   double intensity_magnification=1.0;
   double intensity_magnification=5.0;
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

