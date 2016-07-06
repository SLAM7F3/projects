// ========================================================================
// Program SAURON_CLIENT is a variant of DBSAURON.  It opens a single
// 3D imagery window which displays a DTED-2 map as a point cloud
// colored according to height.  SAURON_CLIENT also instantiates an
// ActiveMQ messenger which continuously listens for node updates from
// Michael Yee's GraphViewer.  When a node message is received, the
// aircraft position and orientation are retrieved from the
// platforms_metadata table in the imagery database.  Similarly, the
// FLIR camera's pose is retrieved from the sensors_metadata table in
// the imagery database.  The LOST aircraft model and OBSFRUSTUM are
// then updated in the 3D viewer map display.
// ========================================================================
// Last updated on 5/4/12; 5/14/12; 5/16/12
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "osg/osg2D/ColorbarHUD.h"
#include "color/colorfuncs.h"
#include "osg/CompassHUD.h"
#include "postgres/databasefuncs.h"
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
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "math/rpy.h"
#include "osg/osgAnnotators/SignPostsGroup.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgTiles/TilesGroup.h"
#include "osg/osgWindow/ViewerManager.h"

#include "geometry/polyline.h"
#include "general/outputfuncs.h"

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

   int campaign_ID=1;	// Tstorm 4.0

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
   gis_databases_group* gis_databases_group_ptr=new gis_databases_group;

   postgis_database* worldgis_database_ptr=postgis_databases_group_ptr->
      generate_postgis_database_from_GISlayer_IDs(passes_group,GISlayer_IDs,0);
   worldgis_database_ptr->set_flat_grid_flag(true);
   gis_database* FLIR_database_ptr=gis_databases_group_ptr->
      generate_gis_database_from_GISlayer_IDs(passes_group,GISlayer_IDs,1);
   gis_database* images_database_ptr=gis_databases_group_ptr->
      generate_gis_database_from_GISlayer_IDs(passes_group,GISlayer_IDs,2);

// Instantiate ActiveMQ messengers:

//   string broker_URL="tcp://127.0.0.1:61616";
   string broker_URL="tcp://"+passes_group.get_broker_URL();
   cout << "ActiveMQ broker_URL = " << broker_URL << endl;
   string message_sender_ID="FRAMESAURON";

//   string message_queue_channel_name="127.0.0.1";
//   string message_queue_channel_name="155.34.162.244";
   string message_queue_channel_name=passes_group.
      get_message_queue_channel_name();
   cout << "message_queue_channel_name = " << message_queue_channel_name
        << endl;

   Messenger viewer_messenger( 
      broker_URL,message_queue_channel_name,message_sender_ID);

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_3D_ptr=new ViewerManager();
   window_mgr_3D_ptr->initialize_window("3D imagery");

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

   osg::Group* root_3D = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

//   bool display_movie_state=true;
//   bool display_movie_number=true;
   bool display_movie_world_time=true;
//   bool hide_Mode_HUD_flag=true;
   bool display_movie_state=false;
   bool display_movie_number=false;
//   bool display_movie_world_time=false;
   bool hide_Mode_HUD_flag=false;

   Operations operations(
      3,window_mgr_3D_ptr,passes_group,
      display_movie_state,display_movie_number,display_movie_world_time,
      hide_Mode_HUD_flag);
   root_3D->addChild(operations.get_OSGgroup_ptr());

   ModeController* ModeController_3D_ptr=operations.get_ModeController_ptr();
   ModeController_3D_ptr->setState(ModeController::RUN_MOVIE);

// Instantiate AnimationController which acts as master game clock:

   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   
// Add a custom manipulators to the event handler list:

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_3D_ptr,window_mgr_3D_ptr);
   window_mgr_3D_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold all decorations:

   Decorations decorations(
      window_mgr_3D_ptr,ModeController_3D_ptr,CM_3D_ptr);
   decorations.set_disable_keyhandlers_flag(false);
   decorations.set_disable_pickhandlers_flag(false);

// Instantiate group to hold movie:

   MoviesGroup* MoviesGroup_ptr=new MoviesGroup(
      2,passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
   MoviesGroup_ptr->set_OSGgroup_nodemask(1);

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

   Earth* Earth_ptr=earths_group.generate_new_Earth(worldgis_database_ptr);

// Retrieve 3D map metadata from FLIR database based upon input
// campaign ID:

   bool northern_hemisphere_flag;
   string campaign_name,DTED_map_name;
   int UTM_zonenumber,flight_number;
   double map_min_lon,map_max_lon,map_min_lat,map_max_lat;
   databasefunc::retrieve_campaign_metadata_from_database(
      FLIR_database_ptr,campaign_ID,
      campaign_name,UTM_zonenumber,northern_hemisphere_flag,DTED_map_name,
      map_min_lon,map_max_lon,map_min_lat,map_max_lat);


//   cout << "UTM_zonenumber = " << UTM_zonenumber << endl;
//   cout << "northern flag = " << northern_hemisphere_flag << endl;
//   cout << "DTED_map_name = " << DTED_map_name << endl;
//   cout << "campaign_name = " << campaign_name << endl;
//   cout << "min_lon = " << map_min_lon << " max_lon = " << map_max_lon
//        << endl;
//   cout << "min_lat = " << map_min_lat << " max_lat = " << map_max_lat
//        << endl;

   CM_3D_ptr->set_min_camera_height_above_grid(5000);	// meters  AZ

   cloudpass_ptr->set_UTM_zonenumber(UTM_zonenumber);	
   cloudpass_ptr->set_northern_hemisphere_flag(northern_hemisphere_flag);

   PassInfo* cloudpassinfo_ptr=cloudpass_ptr->get_PassInfo_ptr();
   cloudpassinfo_ptr->set_longitude_lo(map_min_lon);
   cloudpassinfo_ptr->set_longitude_hi(map_max_lon);
   cloudpassinfo_ptr->set_latitude_lo(map_min_lat);
   cloudpassinfo_ptr->set_latitude_hi(map_max_lat);

// Read in OSGA files for fused DTED-EO map from subdirectories whose
// names match those passed from thin client:

   cloudpass_ptr->get_filenames().clear();
   string osga_subdir="/data/DTED/"+DTED_map_name+"/osga/z_and_p/";
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
   decorations.set_PointCloudsGroup_ptr(&clouds_group);
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
   decorations.set_grid_origin_ptr(grid_origin_ptr);
   window_mgr_3D_ptr->get_EventHandlers_ptr()->push_back(
      new LatLongGridKeyHandler(ModeController_3D_ptr,LatLongGrid_ptr));

// Construct ColorbarHUD for terrain:

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
   clouds_group.set_ColorbarHUD_ptr(ColorbarHUD_ptr);
   
// Extract city and country border information from PostGis database:

   double xmin=PointCloud_ptr->get_xyz_bbox().xMin();
   double xmax=PointCloud_ptr->get_xyz_bbox().xMax();
   double ymin=PointCloud_ptr->get_xyz_bbox().yMin();
   double ymax=PointCloud_ptr->get_xyz_bbox().yMax();
   cout << "xmin = " << xmin << " xmax = " << xmax
        << " ymin = " << ymin << " ymax = " << ymax << endl;
   
   if (worldgis_database_ptr != NULL)
      worldgis_database_ptr->pushback_gis_bbox(xmin,xmax,ymin,ymax);

   if (GISlayer_IDs.size() > 0)
   {
      vector<string> GISlines_tablenames=
         passes_group.get_pass_ptr(GISlayer_IDs[0])->
         get_PassInfo_ptr()->get_gislines_tablenames();

      earth_regions_group.set_specified_UTM_zonenumber(
         cloudpass_ptr->get_UTM_zonenumber());
      worldgis_database_ptr->set_specified_UTM_zonenumber(
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
// this OSG group to decorations OSG group:

      decorations.get_OSGgroup_ptr()->addChild(
         Earth_ptr->get_drawable_group_ptr());
   }
 
// Instantiate a MyDatabasePager to handle paging of files from disk:

   viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager(
      clouds_group.get_SetupGeomVisitor_ptr(),
      clouds_group.get_ColorGeodeVisitor_ptr());

// Instantiate OBSFRUSTA decoration group:

   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr=decorations.add_OBSFRUSTA(
      passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);

// Instantiate SignPostsGroup to hold ground target locations:

   SignPostsGroup* GroundTarget_SignPostsGroup_ptr=
      decorations.add_SignPosts(
         ndims,passes_group.get_pass_ptr(cloudpass_ID));
   GroundTarget_SignPostsGroup_ptr->set_CM_3D_ptr(CM_3D_ptr);

// Instantiate PolyLine and MODEL decorations groups:

   PolyLinesGroup* Flight_PolyLinesGroup_ptr=decorations.add_PolyLines(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   Flight_PolyLinesGroup_ptr->set_width(5);

   LOSMODELSGROUP* Aircraft_MODELSGROUP_ptr=decorations.add_LOSMODELS(
      passes_group.get_pass_ptr(cloudpass_ID),
      Flight_PolyLinesGroup_ptr,NULL,&operations,MoviesGroup_ptr);
   Aircraft_MODELSGROUP_ptr->set_AircraftModelType(MODELSGROUP::LiMIT);
   Aircraft_MODELSGROUP_ptr->set_EarthRegionsGroup_ptr(&earth_regions_group);
   Aircraft_MODELSGROUP_ptr->set_ColorGeodeVisitor_ptr(
      clouds_group.get_ColorGeodeVisitor_ptr());
   Aircraft_MODELSGROUP_ptr->set_CM_3D_ptr(CM_3D_ptr);
   Aircraft_MODELSGROUP_ptr->set_GroundTarget_SignPostsGroup_ptr(
      GroundTarget_SignPostsGroup_ptr);
   Aircraft_MODELSGROUP_ptr->set_images_database_ptr(images_database_ptr);
   Aircraft_MODELSGROUP_ptr->set_PointCloudsGroup_ptr(&clouds_group);
   Aircraft_MODELSGROUP_ptr->set_tomcat_subdir(tomcat_subdir);

   Aircraft_MODELSGROUP_ptr->pushback_Messenger_ptr(&viewer_messenger);
   Aircraft_MODELSGROUP_ptr->assign_MODELSGROUP_Messenger_ptrs();
   Aircraft_MODELSGROUP_ptr->set_raytrace_ground_targets_flag(true);

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
   string geotif_subdir="/data/DTED/"+DTED_map_name+"/geotif/";
   TilesGroup_ptr->set_geotif_subdir(geotif_subdir);
   TilesGroup_ptr->set_geotif_Ztiles_subdir("/data/DTED/all_z_geotif_tiles/");
   Aircraft_MODELSGROUP_ptr->set_TilesGroup_ptr(TilesGroup_ptr);
 
// Purge contents LOS geotiff files left over from previous running of
// this main program:

   TilesGroup_ptr->purge_tile_files();

   root_3D->addChild(clouds_group.get_OSGgroup_ptr());
   root_3D->addChild(latlonggrids_group.get_OSGgroup_ptr());
   root_3D->addChild(decorations.get_OSGgroup_ptr());
   root_3D->addChild(ColorbarHUD_ptr->getProjection());

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

   Aircraft_MODELSGROUP_ptr->set_instantiate_OBSFRUSTAGROUP_flag(false);
   MODEL* Aircraft_MODEL_ptr=Aircraft_MODELSGROUP_ptr->generate_LiMIT_MODEL();
   Aircraft_MODEL_ptr->set_dynamically_compute_OBSFRUSTUM_flag(false);
   Aircraft_MODEL_ptr->set_OBSFRUSTAGROUP_ptr(OBSFRUSTAGROUP_ptr);

//   OBSFRUSTAGROUP* MODEL_OBSFRUSTAGROUP_ptr=
//      Aircraft_MODEL_ptr->get_OBSFRUSTAGROUP_ptr();
//   Aircraft_MODEL_ptr->set_OBSFRUSTAGROUP_ptr(OBSFRUSTAGROUP_ptr);

   photogroup* photogroup_ptr=new photogroup;
   OBSFRUSTAGROUP_ptr->set_photogroup_ptr(photogroup_ptr);

// ========================================================================

// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_3D_ptr->setSceneData(root_3D);
   window_mgr_3D_ptr->realize();

   while( !window_mgr_3D_ptr->done())
   {
      window_mgr_3D_ptr->process();
   }
   delete window_mgr_3D_ptr;

   delete postgis_databases_group_ptr;
}

