// ========================================================================
// Program SAURON is a variant of FLIRSIM.
// ========================================================================
// Last updated on 10/11/11; 10/12/11; 10/15/11
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "osg/osgGeometry/ArrowsGroup.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CentersKeyHandler.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "osg/osg2D/ColorbarHUD.h"
#include "color/colorfuncs.h"
#include "osg/CompassHUD.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgGrid/EarthGrid.h"
#include "osg/osgEarth/EarthsGroup.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "general/filefuncs.h"
#include "osg/osgGrid/LatLongGridKeyHandler.h"
#include "osg/osgGrid/LatLongGridsGroup.h"
#include "messenger/Messenger.h"
#include "osg/ModeController.h"
#include "osg/ModeKeyHandler.h"
#include "osg/osgModels/LOSMODELSGROUP.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osg2D/MovieKeyHandler.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgWindow/MyViewerEventHandler.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "osg/osg3D/ReferenceFrameHUD.h"
#include "math/rpy.h"
#include "osg/osgAnnotators/SignPostsGroup.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgTiles/TilesGroup.h"
#include "osg/osgWindow/ViewerManager.h"

#include "geometry/polyline.h"
#include "general/outputfuncs.h"
#include "osg/osgSceneGraph/DepthPartitionNode.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ofstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);

// Read input data files:
   
   const int ndims=3;
   string OSG_data_dir(getenv("OSG_FILE_PATH"));
   OSG_data_dir += "/";

   PassesGroup passes_group(&arguments);

   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
   cout << "cloudpass_ID = " << cloudpass_ID << endl;
   Pass* cloudpass_ptr=passes_group.get_pass_ptr(cloudpass_ID);
   vector<int> GISlayer_IDs=passes_group.get_GISlayer_IDs();

// Instantiate postgis database objects to send data to and retrieve
// data from external Postgres database:

   postgis_databases_group* postgis_databases_group_ptr=
      new postgis_databases_group;
   postgis_database* postgis_db_ptr=postgis_databases_group_ptr->
      generate_postgis_database_from_GISlayer_IDs(
         passes_group,GISlayer_IDs);
   postgis_db_ptr->set_flat_grid_flag(true);

// Instantiate ActiveMQ messengers:

   string broker_URL="tcp://127.0.0.1:61616";
   cout << "ActiveMQ broker_URL = " << broker_URL << endl;
   string message_sender_ID="FLIRSIM";

   string message_queue_channel_name="viewer_update";
   Messenger viewer_messenger( 
      broker_URL,message_queue_channel_name,message_sender_ID);
   Messenger EarthRegions_GoogleEarth_messenger( 
      broker_URL,message_queue_channel_name,message_sender_ID);

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_2D_ptr=new ViewerManager();
   WindowManager* window_mgr_3D_ptr=new ViewerManager();
   string window_2D_title="2D video";
   string window_3D_title="3D viewer";

   window_mgr_2D_ptr->initialize_dual_windows(
      window_2D_title,window_3D_title,window_mgr_3D_ptr);

   ViewerManager* ViewerManager_ptr=dynamic_cast<ViewerManager*>(
      window_mgr_3D_ptr);
   ViewerManager_ptr->get_MyViewerEventHandler_ptr()->
      set_Viewer_Messenger_ptr(&viewer_messenger);
   string tomcat_subdir="/usr/local/apache-tomcat/webapps/LOST/";
   ViewerManager_ptr->get_MyViewerEventHandler_ptr()->
      set_tomcat_subdir(tomcat_subdir);
   ViewerManager_ptr->get_MyViewerEventHandler_ptr()->
      purge_flash_movies();

// Create OSG root_3D node:

   osg::Group* root_2D = new osg::Group;
   osg::Group* root_3D = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

   bool display_movie_state=true;
   bool display_movie_number=true;
   bool display_movie_world_time=true;
//   bool hide_Mode_HUD_flag=true;
//   bool display_movie_state=false;
//   bool display_movie_number=false;
//   bool display_movie_world_time=false;
   bool hide_Mode_HUD_flag=false;

   Operations operations(
      3,window_mgr_3D_ptr,passes_group,
      display_movie_state,display_movie_number,display_movie_world_time,
      hide_Mode_HUD_flag);
   root_3D->addChild(operations.get_OSGgroup_ptr());
   root_2D->addChild(operations.get_OSGgroup_ptr());

   ModeController* ModeController_3D_ptr=operations.get_ModeController_ptr();
   ModeController_3D_ptr->setState(ModeController::RUN_MOVIE);

// Instantiate AnimationController which acts as master game clock:

   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   AnimationController_ptr->setDelay(0.10);
   AnimationController_ptr->setState(AnimationController::PLAY);
   
   ModeController* ModeController_2D_ptr=new ModeController();
   ModeController_2D_ptr->setState(ModeController::GENERATE_AVI_MOVIE);
   window_mgr_2D_ptr->get_EventHandlers_ptr()->push_back( 
      new ModeKeyHandler(ModeController_2D_ptr) );
   root_2D->addChild(osgfunc::create_Mode_HUD(2,ModeController_2D_ptr));

// Add a custom manipulators to the event handler list:

   osgGA::Custom2DManipulator* CM_2D_ptr = new 
      osgGA::Custom2DManipulator(ModeController_2D_ptr,window_mgr_2D_ptr);
   window_mgr_2D_ptr->set_CameraManipulator(CM_2D_ptr);

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_3D_ptr,window_mgr_3D_ptr);
   window_mgr_3D_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold all decorations:

   Decorations decorations_2D(
      window_mgr_2D_ptr,ModeController_2D_ptr,CM_2D_ptr);

   Decorations decorations_3D(
      window_mgr_3D_ptr,ModeController_3D_ptr,CM_3D_ptr);
   decorations_3D.set_disable_keyhandlers_flag(false);
   decorations_3D.set_disable_pickhandlers_flag(false);

// Instantiate group to hold movie:

   MoviesGroup* MoviesGroup_ptr=new MoviesGroup(
      2,passes_group.get_pass_ptr(cloudpass_ID),
      AnimationController_ptr);
   MoviesGroup_ptr->set_OSGgroup_nodemask(1);
   root_2D->addChild( MoviesGroup_ptr->get_OSGgroup_ptr() );

   MovieKeyHandler* MoviesKeyHandler_ptr=
      new MovieKeyHandler(ModeController_2D_ptr,MoviesGroup_ptr);
   window_mgr_2D_ptr->get_EventHandlers_ptr()->push_back(MoviesKeyHandler_ptr);

// Instantiate "holodeck" Earth grid:

   EarthGrid* earthgrid_ptr = new EarthGrid(colorfunc::grey);
   threevector* earthgrid_origin_ptr=earthgrid_ptr->get_world_origin_ptr();

// Instantiate an EarthsGroup to hold "blue marble" coordinate system
// information:

   bool flat_grid_flag=true;
   EarthsGroup earths_group(
      passes_group.get_pass_ptr(cloudpass_ID),operations.get_Clock_ptr(),
      earthgrid_origin_ptr,flat_grid_flag);
   earths_group.set_CM_3D_ptr(CM_3D_ptr);
   root_3D->addChild(earths_group.get_OSGgroup_ptr());

   Earth* Earth_ptr=earths_group.generate_new_Earth(postgis_db_ptr);

// Comment out next communication section with thin client for debugging
// purposes

// FAKE FAKE: As of Fri 10/3/2011, we hardwire HAFB minimap into this program.

   CM_3D_ptr->set_min_camera_height_above_grid(50);	// meters  HAFB

   cloudpass_ptr->set_UTM_zonenumber(19);	// Boston
   cloudpass_ptr->set_northern_hemisphere_flag(true);

   PassInfo* cloudpassinfo_ptr=cloudpass_ptr->get_PassInfo_ptr();
   cloudpassinfo_ptr->set_longitude_lo(-71.41);
   cloudpassinfo_ptr->set_longitude_hi(-71.14);
   cloudpassinfo_ptr->set_latitude_lo(42.42);
   cloudpassinfo_ptr->set_latitude_hi(42.50);
   string map_countries_name="HAFB";

/*

// FAKE FAKE: As of Fri Sep 30, 2011, we hardwire CA/AZ region into
// this program !

   CM_3D_ptr->set_min_camera_height_above_grid(5000);	// meters  AZ

   cloudpass_ptr->set_UTM_zonenumber(12);	// AZ
   cloudpass_ptr->set_northern_hemisphere_flag(true);

   PassInfo* cloudpassinfo_ptr=cloudpass_ptr->get_PassInfo_ptr();
   cloudpassinfo_ptr->set_longitude_lo(-125);
   cloudpassinfo_ptr->set_longitude_hi(-109);
   cloudpassinfo_ptr->set_latitude_lo(30);
   cloudpassinfo_ptr->set_latitude_hi(42);
   string map_countries_name="CalifAriz";
*/

// Read in OSGA files for fused DTED-EO map from subdirectories whose
// names match those passed from thin client:


//   cout << "map_countries_name = " << map_countries_name << endl;
   cloudpass_ptr->get_filenames().clear();
   string osga_subdir="/data/DTED/"+map_countries_name+"/osga/z_and_p/";
   vector<string> osga_filenames=
      filefunc::files_in_subdir_matching_substring(osga_subdir,".osga");
   for (int f=0; f<osga_filenames.size(); f++)
   {
      cloudpass_ptr->get_filenames().push_back(osga_filenames[f]);
//      cout << "file = " << cloudpass_ptr->get_filenames().back() << endl;
   }
   
// Instantiate groups to hold multiple point clouds,
// latitude-longitude grids and associated earth regions:

   PointCloudsGroup clouds_group(
      passes_group.get_pass_ptr(cloudpass_ID));
   clouds_group.set_point_transition_altitude_factor(100);
   clouds_group.set_Terrain_Manipulator_ptr(CM_3D_ptr);
//   clouds_group.set_auto_resize_points_flag(false);
   clouds_group.set_auto_resize_points_flag(true);
   decorations_3D.set_PointCloudsGroup_ptr(&clouds_group);
   window_mgr_3D_ptr->get_EventHandlers_ptr()->push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_3D_ptr,CM_3D_ptr));

   LatLongGridsGroup latlonggrids_group(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),CM_3D_ptr);
   EarthRegionsGroup earth_regions_group(
      passes_group.get_pass_ptr(cloudpass_ID),      
      &clouds_group,&latlonggrids_group,MoviesGroup_ptr,Earth_ptr);
   earth_regions_group.set_specified_UTM_zonenumber(
      cloudpass_ptr->get_UTM_zonenumber());
   earth_regions_group.set_northern_hemisphere_flag(
      cloudpass_ptr->get_northern_hemisphere_flag());
   earth_regions_group.pushback_Messenger_ptr(
      &EarthRegions_GoogleEarth_messenger);
   earth_regions_group.assign_EarthRegionsGroup_Messenger_ptrs();
   bool place_onto_bluemarble_flag=false;

   earth_regions_group.generate_regions(
      passes_group,place_onto_bluemarble_flag);
   PointCloud* PointCloud_ptr=clouds_group.get_Cloud_ptr(0);
   LatLongGrid* LatLongGrid_ptr=latlonggrids_group.get_Grid_ptr(0);
   PointCloud_ptr->set_Grid_ptr(LatLongGrid_ptr);
   clouds_group.set_dependent_coloring_var(0);	// Z -> hue; P -> value
//   LatLongGrid_ptr->set_depth_buffering_off_flag(false);
   LatLongGrid_ptr->set_depth_buffering_off_flag(true);
   LatLongGrid_ptr->set_init_linewidth(2.0);
   LatLongGrid_ptr->set_text_size_prefactor(9.0E-4);
   threevector* grid_origin_ptr=LatLongGrid_ptr->get_world_origin_ptr();
   cout << "*grid_origin_ptr = " << *grid_origin_ptr << endl;
   CM_3D_ptr->set_Grid_ptr(LatLongGrid_ptr);
   decorations_3D.set_grid_origin_ptr(grid_origin_ptr);
   window_mgr_3D_ptr->get_EventHandlers_ptr()->push_back(
      new LatLongGridKeyHandler(ModeController_3D_ptr,LatLongGrid_ptr));

   ReferenceFrameHUD* ReferenceFrameHUD_ptr=new ReferenceFrameHUD();

// Construct ColorbarHUDs for terrain, raytracing occlusion results
// and SAM threats:


   ColorMap* height_ColorMap_ptr=PointCloud_ptr->get_z_ColorMap_ptr();
   double z_max=height_ColorMap_ptr->get_max_threshold(2);
   double z_min=height_ColorMap_ptr->get_min_threshold(2);
   cout << "z_max = " << z_max << " z_min = " << z_min << endl;
   
   double hue_start=240;	// blue
   double hue_stop=30;		// orange
   const double ft_per_km=0.0032808399;
   double scalar_value_start=0;
   double scalar_value_stop=basic_math::round(z_max * ft_per_km);
   string title="Terrain altitude (kft)";

   ColorbarHUD* ColorbarHUD_ptr=new ColorbarHUD(
      hue_start,hue_stop,scalar_value_start,scalar_value_stop,title);

   hue_start=0;		// red
   hue_stop=120;	// green
   scalar_value_start=0;
   scalar_value_stop=100;
   title="Visibility Percentage";

   ColorbarHUD_ptr->pushback_hue_start(hue_start);
   ColorbarHUD_ptr->pushback_hue_stop(hue_stop);
   ColorbarHUD_ptr->pushback_scalar_value_start(scalar_value_start);
   ColorbarHUD_ptr->pushback_scalar_value_stop(scalar_value_stop);
   ColorbarHUD_ptr->pushback_title(title);

   hue_start=hue_stop=0;	// red
   scalar_value_start=0;	// km**2
   scalar_value_stop=100;	// km**2
   title="Ground area in SAM range of aircraft (sq km)";

   ColorbarHUD_ptr->pushback_hue_start(hue_start);
   ColorbarHUD_ptr->pushback_hue_stop(hue_stop);
   ColorbarHUD_ptr->pushback_scalar_value_start(scalar_value_start);
   ColorbarHUD_ptr->pushback_scalar_value_stop(scalar_value_stop);
   ColorbarHUD_ptr->pushback_title(title);

   clouds_group.set_ColorbarHUD_ptr(ColorbarHUD_ptr);
   
// Extract city and country border information from PostGis database:

   double xmin=PointCloud_ptr->get_xyz_bbox().xMin();
   double xmax=PointCloud_ptr->get_xyz_bbox().xMax();
   double ymin=PointCloud_ptr->get_xyz_bbox().yMin();
   double ymax=PointCloud_ptr->get_xyz_bbox().yMax();
   cout << "xmin = " << xmin << " xmax = " << xmax
        << " ymin = " << ymin << " ymax = " << ymax << endl;
   
   if (postgis_db_ptr != NULL)
      postgis_db_ptr->pushback_gis_bbox(xmin,xmax,ymin,ymax);

   if (GISlayer_IDs.size() > 0)
   {
      vector<string> GISlines_tablenames=
         passes_group.get_pass_ptr(GISlayer_IDs[0])->
         get_PassInfo_ptr()->get_gislines_tablenames();

      earth_regions_group.set_specified_UTM_zonenumber(
         cloudpass_ptr->get_UTM_zonenumber());
      postgis_db_ptr->set_specified_UTM_zonenumber(
         cloudpass_ptr->get_UTM_zonenumber());
      PassInfo* cloudpassinfo_ptr=cloudpass_ptr->get_PassInfo_ptr();

      Earth_ptr->set_init_border_width(3); 

      double min_lon=cloudpassinfo_ptr->get_longitude_lo();
      double max_lon=cloudpassinfo_ptr->get_longitude_hi();
      double min_lat=cloudpassinfo_ptr->get_latitude_lo();
      double max_lat=cloudpassinfo_ptr->get_latitude_hi();

      Earth_ptr->retrieve_borders_from_PostGIS_database(
         "country_borders",min_lon,max_lon,min_lat,max_lat,
         colorfunc::white);
      Earth_ptr->retrieve_borders_from_PostGIS_database(
         "state_borders",min_lon,max_lon,min_lat,max_lat,
         colorfunc::white);
      Earth_ptr->get_borders_group_ptr()->set_DataGraph_ptr(PointCloud_ptr);

// Recall country borders, country names and city names are all
// contained within *Earth_ptr->get_drawable_group_ptr().  So we add
// this OSG group to decorations_3D OSG group:

      decorations_3D.get_OSGgroup_ptr()->addChild(
         Earth_ptr->get_drawable_group_ptr());
   }
 
// Instantiate a MyDatabasePager to handle paging of files from disk:

   viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager(
      clouds_group.get_SetupGeomVisitor_ptr(),
      clouds_group.get_ColorGeodeVisitor_ptr());

// Instantiate a PointFinder;

   PointFinder pointfinder(&clouds_group);
   CM_3D_ptr->set_PointFinder(&pointfinder);

// Instantiate Arrows decoration group:

   ArrowsGroup* ArrowsGroup_ptr=decorations_3D.add_Arrows(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   ArrowsGroup_ptr->set_CM_3D_ptr(CM_3D_ptr);

// Instantiate SignPostsGroup to hold ground target locations:

   SignPostsGroup* GroundTarget_SignPostsGroup_ptr=
      decorations_3D.add_SignPosts(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   GroundTarget_SignPostsGroup_ptr->set_CM_3D_ptr(CM_3D_ptr);
   GroundTarget_SignPostsGroup_ptr->set_common_geometrical_size(20);

// Instantiate FeaturesGroup:

   decorations_3D.add_Features(ndims,passes_group.get_pass_ptr(cloudpass_ID));
   decorations_3D.get_FeaturePickHandler_ptr()->
      set_surface_picking_flag(false);
   decorations_3D.get_FeaturePickHandler_ptr()->
      set_Zplane_picking_flag(false);
   
// Instantiate Polygons decorations group:

   osgGeometry::PolygonsGroup* PolygonsGroup_ptr=decorations_3D.add_Polygons(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);

// Instantiate PolyLines decorations group:

   PolyLinesGroup* PolyLinesGroup_ptr=decorations_3D.add_PolyLines(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   PolyLinesGroup_ptr->set_width(2);

   string footprints_subdir="./footprints/";
   vector<string> footprint_filenames=filefunc::files_in_subdir(
      footprints_subdir);
   for (int f=0; f<footprint_filenames.size(); f++)
   {
      vector<threevector> vertices;
      filefunc::ReadInfile(footprint_filenames[f]);
      for (int f=0; f<filefunc::text_line.size(); f++)
      {
         vector<double> column_values=
            stringfunc::string_to_numbers(filefunc::text_line[f]);
         vertices.push_back(
            threevector(column_values[0],column_values[1],column_values[2]));
      } // loop over index f 
      
      vertices.push_back(vertices.front());
      PolyLine* PolyLine_ptr=PolyLinesGroup_ptr->generate_new_PolyLine(
         vertices,colorfunc::get_OSG_color(colorfunc::pink));
   } // loop over index f labeling distinct footprints

// Instantiate OBSFRUSTA decoration group:

   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr=decorations_3D.add_OBSFRUSTA(
      passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);

// Instantiate ROI Polyhedra decoration group:

   PolyhedraGroup* ROI_PolyhedraGroup_ptr=
      decorations_3D.add_Polyhedra(passes_group.get_pass_ptr(cloudpass_ID));
   ROI_PolyhedraGroup_ptr->set_bbox_height(10000);	// meters
   ROI_PolyhedraGroup_ptr->set_altitude_dependent_volume_alphas_flag(true);
   ROI_PolyhedraGroup_ptr->set_CM_3D_ptr(CM_3D_ptr);
  
// Instantiate PolyLine and MODEL decorations_3D groups:

   PolyLinesGroup* Flight_PolyLinesGroup_ptr=decorations_3D.add_PolyLines(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   Flight_PolyLinesGroup_ptr->set_width(5);

   LOSMODELSGROUP* Aircraft_MODELSGROUP_ptr=decorations_3D.add_LOSMODELS(
      passes_group.get_pass_ptr(cloudpass_ID),
      Flight_PolyLinesGroup_ptr,NULL,&operations,MoviesGroup_ptr);
   Aircraft_MODELSGROUP_ptr->set_AircraftModelType(MODELSGROUP::LiMIT);
   Aircraft_MODELSGROUP_ptr->set_ArrowsGroup_ptr(ArrowsGroup_ptr);
   Aircraft_MODELSGROUP_ptr->set_EarthRegionsGroup_ptr(&earth_regions_group);
   Aircraft_MODELSGROUP_ptr->set_ColorGeodeVisitor_ptr(
      clouds_group.get_ColorGeodeVisitor_ptr());
   Aircraft_MODELSGROUP_ptr->set_CM_3D_ptr(CM_3D_ptr);
   Aircraft_MODELSGROUP_ptr->set_GroundTarget_SignPostsGroup_ptr(
      GroundTarget_SignPostsGroup_ptr);
//   Aircraft_MODELSGROUP_ptr->set_compute_zface_height_flag(true);
   Aircraft_MODELSGROUP_ptr->set_ROI_PolyhedraGroup_ptr(
      ROI_PolyhedraGroup_ptr);
   Aircraft_MODELSGROUP_ptr->set_PointCloudsGroup_ptr(&clouds_group);
   Aircraft_MODELSGROUP_ptr->set_ReferenceFrameHUD_ptr(
      ReferenceFrameHUD_ptr);
   Aircraft_MODELSGROUP_ptr->set_tomcat_subdir(tomcat_subdir);

   Aircraft_MODELSGROUP_ptr->pushback_Messenger_ptr(&viewer_messenger);
   Aircraft_MODELSGROUP_ptr->assign_MODELSGROUP_Messenger_ptrs();
   Aircraft_MODELSGROUP_ptr->set_raytrace_ground_targets_flag(true);

   Aircraft_MODELSGROUP_ptr->get_movers_group_ptr()->set_Messenger_ptr(
      &EarthRegions_GoogleEarth_messenger);

   Aircraft_MODELSGROUP_ptr->set_instantiate_OBSFRUSTUMPickHandler_flag(true);
//   Aircraft_MODELSGROUP_ptr->set_instantiate_OBSFRUSTUMPickHandler_flag(false);

// Instantiate TilesGroup to perform raytracing computations:

   TilesGroup* TilesGroup_ptr=new TilesGroup();
   TilesGroup_ptr->set_min_long(cloudpass_ptr->get_PassInfo_ptr()->
                                get_longitude_lo());
   TilesGroup_ptr->set_max_long(cloudpass_ptr->get_PassInfo_ptr()->
                                get_longitude_hi());
   TilesGroup_ptr->set_min_lat(cloudpass_ptr->get_PassInfo_ptr()->
                                get_latitude_lo());
   TilesGroup_ptr->set_max_lat(cloudpass_ptr->get_PassInfo_ptr()->
                                get_latitude_hi());
   TilesGroup_ptr->set_specified_UTM_zonenumber(
      cloudpass_ptr->get_UTM_zonenumber());
   TilesGroup_ptr->set_northern_hemisphere_flag(
      cloudpass_ptr->get_northern_hemisphere_flag());
   TilesGroup_ptr->set_tomcat_subdir(tomcat_subdir);
   string geotif_subdir="/data/DTED/"+map_countries_name+"/geotif/";
   TilesGroup_ptr->set_geotif_subdir(geotif_subdir);
   TilesGroup_ptr->set_geotif_Ztiles_subdir("/data/DTED/all_z_geotif_tiles/");
   Aircraft_MODELSGROUP_ptr->set_TilesGroup_ptr(TilesGroup_ptr);
 
// Purge contents LOS geotiff files left over from previous running of
// this main program:

   TilesGroup_ptr->purge_tile_files();

   root_3D->addChild(clouds_group.get_OSGgroup_ptr());
   root_3D->addChild(latlonggrids_group.get_OSGgroup_ptr());
   root_3D->addChild(decorations_3D.get_OSGgroup_ptr());
   root_3D->addChild(ReferenceFrameHUD_ptr->getProjection());
//   root_3D->addChild(ColorbarHUD_ptr->getProjection());

// Instantiate compass:

   CompassHUD* CompassHUD_ptr=new CompassHUD(colorfunc::white);
   root_3D->addChild(CompassHUD_ptr->getProjection());
   CM_3D_ptr->set_CompassHUD_ptr(CompassHUD_ptr);

// ========================================================================

   Earth_ptr->retrieve_US_cities_from_PostGIS_database(
      cloudpassinfo_ptr->get_longitude_lo(),
      cloudpassinfo_ptr->get_longitude_hi(),
      cloudpassinfo_ptr->get_latitude_lo(),
      cloudpassinfo_ptr->get_latitude_hi(),
      colorfunc::white);
   Earth_ptr->retrieve_cities_from_PostGIS_database(
      cloudpassinfo_ptr->get_longitude_lo(),
      cloudpassinfo_ptr->get_longitude_hi(),
      cloudpassinfo_ptr->get_latitude_lo(),
      cloudpassinfo_ptr->get_latitude_hi(),
      colorfunc::white);

// ========================================================================

   string meta_subdir="./";
   string aircraft_metadata_filename=meta_subdir+"aircraft_metadata.txt";
   string camera_metadata_filename=meta_subdir+"camera_metadata.txt";

   vector<double> curr_time;
   vector<threevector> curr_posn;
   vector<rpy> curr_RPY;
   
   filefunc::ReadInfile(aircraft_metadata_filename);
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      curr_time.push_back(values[1]);
      curr_posn.push_back(threevector(values[2],values[3],values[4]));
      curr_RPY.push_back(rpy(values[7],values[6],values[5]));
   }

   vector<twovector> curr_FOV;
   vector<threevector> curr_AER;
   filefunc::ReadInfile(camera_metadata_filename);
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      curr_FOV.push_back(twovector(values[1],values[2]));
      curr_AER.push_back(threevector(values[3],values[4],values[5]));
   }

   Aircraft_MODELSGROUP_ptr->set_instantiate_OBSFRUSTAGROUP_flag(false);
   Aircraft_MODELSGROUP_ptr->set_update_dynamic_aircraft_MODEL_flag(true);
   MODEL* Aircraft_MODEL_ptr=Aircraft_MODELSGROUP_ptr->generate_LiMIT_MODEL();
   Aircraft_MODEL_ptr->set_dynamically_compute_OBSFRUSTUM_flag(false);
   Aircraft_MODEL_ptr->set_OBSFRUSTAGROUP_ptr(OBSFRUSTAGROUP_ptr);

   Aircraft_MODELSGROUP_ptr->generate_Aircraft_MODEL_track_and_mover(
      Aircraft_MODEL_ptr);
   track* track_ptr=Aircraft_MODEL_ptr->get_track_ptr();

// Load aircraft positions and orientations as well as sensor
// orientations and fields-of-view into *track_ptr:

   for (int i=0; i<curr_time.size(); i++)
   {
      track_ptr->set_posn_rpy_sensor_aer_fov(
         curr_time[i],curr_posn[i],curr_RPY[i],curr_AER[i],curr_FOV[i]);
   }
//   track_ptr->compute_average_velocities();

// Set master game clock's start time, stop time and time step:

   operations.set_master_world_start_time(curr_time.front());
   operations.set_master_world_stop_time(curr_time.back());
   double world_time_step_in_secs_per_frame=1;		// secs
   operations.set_delta_master_world_time_step_per_master_frame(
      world_time_step_in_secs_per_frame);
   operations.reset_AnimationController_world_time_params();

   OBSFRUSTAGROUP* MODEL_OBSFRUSTAGROUP_ptr=
      Aircraft_MODEL_ptr->get_OBSFRUSTAGROUP_ptr();
//   MODEL_OBSFRUSTAGROUP_ptr->set_OSGgroup_nodemask(0);
   Aircraft_MODEL_ptr->set_OBSFRUSTAGROUP_ptr(OBSFRUSTAGROUP_ptr);

   photogroup* photogroup_ptr=new photogroup;
   OBSFRUSTAGROUP_ptr->set_photogroup_ptr(photogroup_ptr);

   int frame=0;
//   cout << "Enter frame number:" << endl;
//   cin >> frame;
//   AnimationController_ptr->set_curr_framenumber(frame);

   string frames_subdir=
      "/home/cho/programs/c++/svn/projects/src/mains/thunderstorm/video_data/20110511/flight1/";
//   string frames_subdir=
//      "/media/TS_Spiral4/Thunderstorm/Analog_Data/ops-5-11-11/flight-1/20110511_234525/";
////   string frames_subdir=
//      "/media/TS_Spiral4/Thunderstorm/Analog_Data/ops-5-11-11/flight-1/20110511_211231/";

   AnimationController_ptr->store_ordered_image_filenames(frames_subdir);
   int number_of_images=AnimationController_ptr->
      get_n_ordered_image_filenames();
   cout << "number_of_images = " << number_of_images << endl;
   AnimationController_ptr->set_nframes(number_of_images);

   string first_frame_filename=AnimationController_ptr->
      get_next_ordered_image_filename();
   cout << "first_frame_filename = " << first_frame_filename << endl;

   texture_rectangle* texture_rectangle_ptr=
      MoviesGroup_ptr->generate_new_texture_rectangle(first_frame_filename);

   int min_photo_number=0;
//   int min_photo_number=AnimationController_ptr->get_frame_counter_offset();
   int max_photo_number=
      min_photo_number+AnimationController_ptr->get_nframes()-1;
//   cout << " min_photo = " << min_photo_number
//        << " max_photo = " << max_photo_number << endl;
   
   texture_rectangle_ptr->set_first_frame_to_display(min_photo_number);
   texture_rectangle_ptr->set_last_frame_to_display(max_photo_number);

   MoviesGroup_ptr->destroy_all_Movies();
   Movie* movie_ptr=MoviesGroup_ptr->
      generate_new_Movie(texture_rectangle_ptr);

// ========================================================================

// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_3D_ptr->setSceneData(root_3D);
   window_mgr_2D_ptr->setSceneData(root_2D);

   window_mgr_3D_ptr->realize();
   window_mgr_2D_ptr->realize();

   int counter=0;
   while( !window_mgr_2D_ptr->done()  && !window_mgr_3D_ptr->done())
   {
      if (counter%3==0)
      {
         window_mgr_2D_ptr->process();
      }
      window_mgr_3D_ptr->process();
      counter++;
   }
   delete window_mgr_2D_ptr;
   delete window_mgr_3D_ptr;

   delete postgis_databases_group_ptr;
}
