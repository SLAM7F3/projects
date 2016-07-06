// ========================================================================
// Program PATHFINDER reads in a set of ALIRT geotif tiles.  It 
// asks the user to enter relative weights for terms within a footpath
// construction cost function.  PATHFINDER performs an A* search for
// the best path between user-specified start and stop points within
// the 3D terrain.

// From within /data_second_disk/TOC11/FOB_Blessing/FOB_Blessing_tiles/z_tif_tiles/tiles/filtered_Z_tiles/osga

//  ~cho/programs/c++/svn/projects/src/mains/pathplanning/pathfinder *.osga

// Press "A" to change from VIEW DATA mode to INSERT ANNOTATION mode.
// Within INSERT ANNOTATION mode, press shift and left-mouse click
// within 3D viewer window in order to pick a starting point for a
// footpath.  Enter a label such as "start" within the terminal window
// when queried to "Enter new text label for SignPost".  Repeat this
// procedure to enter a stopping point for the footpath.

// Press "A" twice in order to change from VIEW data to
// MANIPULATE_ANNOTATION mode.  Then 'a' key to compute Astar path
// between starting and stopping locations.  Enter relative weights
// for terms within the footpath cost function in the terminal window
// when queried.  PATHFINDER program will then take several seconds to
// minutes to compute and display the A* path between the starting and
// stopping points.

// Once a path has been found, press L twice to enter into
// MANIPULATE_POLYLINE mode.  Then press 'e' to export the A* path to
// an output text file.  The exported path may subsequently be
// imported into program QTTOC11 for red actor path network discovery.

// Note: The footpath cost function is specified within our
// MapSearchNode::GetCost() member function.  As of 2011, it contains
// 3 terms.  The first two cost function terms are generic for all
// footpaths.  In contrast, the 3rd term is specialized for the TOC11
// project...

// ========================================================================
// Last updated on 5/27/11; 5/28/11; 6/19/11; 1/16/13
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
#include "osg/osgGraphicals/CentersKeyHandler.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "astro_geo/Clock.h"
#include "osg/CompassHUD.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "osg/osgGrid/GridKeyHandler.h"
#include "osg/osgGrid/LatLongGridsGroup.h"
#include "messenger/Messenger.h"
#include "osg/ModeController.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "osg/osgAnnotators/PowerPointsGroup.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgTiles/TilesGroup.h"
#include "time/timefuncs.h"
#include "osg/osgWindow/ViewerManager.h"


#include "graphs/graph.h"
#include "graphs/graphfuncs.h"
#include "graphs/node.h"
#include "osg/osgAnnotators/PathFinder.h"
#include "osg/osgAnnotators/PathFinderKeyHandler.h"
#include "image/raster_parser.h"
#include "image/terrainfuncs.h"


#include "general/outputfuncs.h"

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
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
   vector<int> GISlayer_IDs=passes_group.get_GISlayer_IDs();

// Instantiate postgis database objects to send data to and retrieve
// data from external Postgres database:

   postgis_databases_group* postgis_databases_group_ptr=
      new postgis_databases_group;
   postgis_database* postgis_db_ptr=postgis_databases_group_ptr->
      generate_postgis_database_from_GISlayer_IDs(passes_group,GISlayer_IDs);

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("3D imagery");

   int pass_ID=passes_group.get_n_passes()-1;
   string broker_URL=passes_group.get_pass_ptr(pass_ID)->
      get_PassInfo_ptr()->get_ActiveMQ_hostname();
   cout << "ActiveMQ broker_URL = " << broker_URL << endl;

   string ppt_message_queue_channel_name="powerpoint";
   Messenger ppt_messenger( broker_URL, ppt_message_queue_channel_name );

   string wiki_message_queue_channel_name="wiki";
   Messenger wiki_messenger( broker_URL, wiki_message_queue_channel_name );

// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

   bool display_movie_state=false;
//   bool display_movie_state=true;
   bool display_movie_number=false;
//   bool display_movie_number=true;
   bool display_movie_world_time=false;
   bool hide_Mode_HUD_flag=false;
//   bool hide_Mode_HUD_flag=true;
   Operations operations(
      ndims,window_mgr_ptr,passes_group,
      display_movie_state,display_movie_number,display_movie_world_time,
      hide_Mode_HUD_flag);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   AnimationController_ptr->setDelay(0);
   root->addChild(operations.get_OSGgroup_ptr());

// Instantiate clock pointer to keep track of real time:

   Clock* clock_ptr=AnimationController_ptr->get_clock_ptr();
   clock_ptr->current_local_time_and_UTC();

// Add a custom manipulator to the event handler list:

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

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
   double zmin=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().zMin();
   double zmax=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().zMax();
   if (postgis_db_ptr != NULL)
      postgis_db_ptr->pushback_gis_bbox(xmin,xmax,ymin,ymax);

// Instantiate a PointFinder;

   PointFinder pointfinder(&clouds_group);
   if (!passes_group.get_pick_points_on_Zplane_flag())
   {
      CM_3D_ptr->set_PointFinder(&pointfinder);
   }
   
// Instantiate a MyDatabasePager to handle paging of files from disk:

   viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager(
      clouds_group.get_SetupGeomVisitor_ptr(),
      clouds_group.get_ColorGeodeVisitor_ptr());
   clouds_group.get_HiresDataVisitor_ptr()->setDatabasePager(
      MyDatabasePager_ptr);

// Instantiate Polyhedra decorations and Earth Regions groups:

   PolyhedraGroup* PolyhedraGroup_ptr=
      decorations.add_Polyhedra(passes_group.get_pass_ptr(cloudpass_ID));

// Instantiate signposts, features, army symbol and sphere segments
// decoration groups:

   SignPostsGroup* SignPostsGroup_ptr=decorations.add_SignPosts(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),
      NULL,NULL);
   SignPostsGroup_ptr->set_size(20);
   SignPostsGroup_ptr->pushback_Messenger_ptr(&wiki_messenger);
   SignPostsGroup_ptr->set_postgis_databases_group_ptr(
      postgis_databases_group_ptr);

   for (int n=0; n<decorations.get_n_SignPostsGroups(); n++)
   {
      decorations.get_SignPostsGroup_ptr(n)->set_EarthRegionsGroup_ptr(
         &earth_regions_group);
      decorations.get_SignPostsGroup_ptr(n)->set_Clock_ptr(clock_ptr);
   }

   decorations.add_Features(ndims,passes_group.get_pass_ptr(cloudpass_ID));
//   decorations.get_FeaturesGroup_ptr()->set_EarthRegionsGroup_ptr(
//      &earth_regions_group);
//   decorations.add_ArmySymbols(passes_group.get_pass_ptr(cloudpass_ID));

   decorations.add_SphereSegments(passes_group.get_pass_ptr(cloudpass_ID));

// Instantiate cylinders decoration group:

//   CylindersGroup* CylindersGroup_ptr=
      decorations.add_Cylinders(
         passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
   decorations.get_CylindersGroup_ptr()->set_rh(5,0.35);
   decorations.get_CylinderPickHandler_ptr()->set_text_size(8);
   decorations.get_CylinderPickHandler_ptr()->
      set_text_screen_axis_alignment_flag(false);

// Instantiate OBSFRUSTA decoration group:

   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr=decorations.add_OBSFRUSTA(
      passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
   OBSFRUSTAGROUP_ptr->set_z_ColorMap_ptr(
      clouds_group.get_Cloud_ptr(0)->get_z_ColorMap_ptr());
   OBSFRUSTAGROUP_ptr->set_ImageNumberHUD_ptr(
      operations.get_ImageNumberHUD_ptr());      
   decorations.get_OBSFRUSTAKeyHandler_ptr()->set_Grid_ptr(
      LatLongGrid_ptr);
   decorations.get_OBSFRUSTUMPickHandler_ptr()->set_Grid_ptr(
      LatLongGrid_ptr);
   decorations.get_OBSFRUSTAKeyHandler_ptr()->set_SignPostsGroup_ptr(
      SignPostsGroup_ptr);
   decorations.get_OBSFRUSTAKeyHandler_ptr()->set_PolyhedraGroup_ptr(
      PolyhedraGroup_ptr);

   double Zplane_altitude=20;	// meters (appropriate for NYC demo)
   bool multicolor_frusta_flag=false;
   bool initially_mask_all_frusta_flag=true;
   OBSFRUSTAGROUP_ptr->generate_still_imagery_frusta_from_projection_matrices(
      passes_group,Zplane_altitude,
      multicolor_frusta_flag,initially_mask_all_frusta_flag);
    
// Instantiate Polyhedra, Polygons & PolyLines decoration groups:

//   osgGeometry::PolygonsGroup*  PolygonsGroup_ptr=
   decorations.add_Polygons(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);

   PolyLinesGroup* PolyLinesGroup_ptr=
      decorations.add_PolyLines(ndims,passes_group.get_pass_ptr(cloudpass_ID));
   PolyLinesGroup_ptr->set_width(8);

   PolyLinePickHandler* PolyLinePickHandler_ptr=
      decorations.get_PolyLinePickHandler_ptr();
   PolyLinePickHandler_ptr->set_min_doubleclick_time_spread(0.05);
   PolyLinePickHandler_ptr->set_max_doubleclick_time_spread(0.25);
   PolyLinePickHandler_ptr->set_z_offset(5);

//   PolyLinePickHandler_ptr->set_form_polyhedron_skeleton_flag(true);
//   PolyLinePickHandler_ptr->set_form_polygon_flag(true);

   root->addChild(decorations.get_OSGgroup_ptr());
   root->addChild(clouds_group.get_OSGgroup_ptr());
   root->addChild(latlonggrids_group.get_OSGgroup_ptr());

// Attach scene graph to the viewer:

   window_mgr_ptr->setSceneData(root);

// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_ptr->realize();

// Generate DTED graph for from input terrain height map:
   
   PathFinder* PathFinder_ptr=new PathFinder(
      passes_group.get_pass_ptr(cloudpass_ID),
      SignPostsGroup_ptr,PolyLinesGroup_ptr,
      AnimationController_ptr,grid_origin_ptr);
   PathFinder_ptr->set_extremal_z_values(zmin,zmax);
   root->addChild(PathFinder_ptr->get_OSGgroup_ptr());

//   string subdir="/media/5f11a671-48d4-4fe7-9997-b6e2d7070af0/ALIRT/Haiti/";
//   string geotif_filename=subdir+"mountains_cropped.tif";
//   string subdir="/data_second_disk/TOC11/FOB_Blessing/FOB_Blessing_tiles/z_tif_tiles/tiles/filtered_Z_tiles/tif/";
//   string geotif_filename=subdir+"filtered_tile_3_3.tif";
//   PathFinder_ptr->import_DTED_height_map(geotif_filename);

   PathFinderKeyHandler* PathFinderKeyHandler_ptr=
      new PathFinderKeyHandler(ModeController_ptr,PathFinder_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      PathFinderKeyHandler_ptr);

// Instantiate TilesGroup to perform raytracing computations:

   TilesGroup* TilesGroup_ptr=new TilesGroup();
   double ROI_min_lon=70.692312114805;
   double ROI_max_lon=70.911461319565;
   double ROI_min_lat=34.905732075832;
   double ROI_max_lat=35.049501837535;

   TilesGroup_ptr->set_min_long(ROI_min_lon);
   TilesGroup_ptr->set_max_long(ROI_max_lon);
   TilesGroup_ptr->set_min_lat(ROI_min_lat);
   TilesGroup_ptr->set_max_lat(ROI_max_lat);
   TilesGroup_ptr->set_specified_UTM_zonenumber(42);
   TilesGroup_ptr->set_northern_hemisphere_flag(true);
   string geotif_subdir=
      "/data_second_disk/TOC11/FOB_Blessing/FOB_Blessing_tiles/z_tif_tiles/tiles/filtered_Z_tiles/tif/";
   string redactor_geotif_subdir=geotif_subdir+"red_actor_paths/";
   TilesGroup_ptr->set_geotif_subdir(redactor_geotif_subdir);

   double easting_lo=655300.000;
   double northing_lo=3862699.000;
   double easting_hi=675300.000;
   double northing_hi=3882699.000;

//   double easting_lo=655305.000;
//   double northing_lo=3862694.000;
//   double easting_hi=675305.000;
//   double northing_hi=3882694.000;

//   double easting_lo=655315.000;
//   double northing_lo=3862684.000;
//   double easting_hi=675315.000;
//   double northing_hi=3882684.000;
   
   double dx=1;	// meter
   twoDarray* DTED_ztwoDarray_ptr=TilesGroup_ptr->generate_subtile_twoDarray(
      dx,dx,easting_lo,easting_hi,northing_lo,northing_hi);
   DTED_ztwoDarray_ptr->clear_values();

   cout << "*DTED_ztwoDarray_ptr = " << *DTED_ztwoDarray_ptr << endl;
   vector<string> geotif_filenames=filefunc::files_in_subdir(
      redactor_geotif_subdir);
   cout << "redactor_geotif_subdir = " << redactor_geotif_subdir << endl;
   for (unsigned int j=0; j<geotif_filenames.size(); j++)
   {
      cout << "curr geotif filename = " << geotif_filenames[j] << endl;
      TilesGroup_ptr->read_geotif_subtile_height_data(
         geotif_filenames[j],DTED_ztwoDarray_ptr);
   }

/*
   int n_pixel_gaps=0;
   cout << "Counting number of pixel gaps" << endl;
   for (int px=0; px<DTED_ztwoDarray_ptr->get_mdim(); px++)
   {
      for (int py=0; py<DTED_ztwoDarray_ptr->get_ndim(); py++)
      {
         double curr_z=DTED_ztwoDarray_ptr->get(px,py);
         if (curr_z < 10) n_pixel_gaps++;
      }
   }
   cout << "n_pixel_gaps = " << n_pixel_gaps << endl;

// n_pixels_gaps = 80004

*/

   PathFinder_ptr->set_ztwoDarray_ptr(DTED_ztwoDarray_ptr);

// ========================================================================
// For NYC demo purposes, automatically retrieve all signposts from
// PostGIS database:

   colorfunc::Color signposts_color=colorfunc::white;
//   colorfunc::Color signposts_color=colorfunc::red;
   double SignPost_size=0.5;
   SignPostsGroup_ptr->retrieve_all_signposts_from_PostGIS_databases(
      signposts_color,SignPost_size);
   decorations.get_SignPostPickHandler_ptr()->
      set_allow_doubleclick_in_manipulate_fused_data_mode(true);

   while( !window_mgr_ptr->done() )
   {
      window_mgr_ptr->process();
    }
   delete window_mgr_ptr;

   delete postgis_databases_group_ptr;
}
