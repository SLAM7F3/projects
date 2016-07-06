// ========================================================================
// Program QTLOS is a variant of program LOS.  It is an experimental
// playground for working with DTED-2 data for Afghanistan, Korea,
// etc.
// ========================================================================
// Last updated on 12/21/10; 12/22/10; 12/24/10
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <QtCore/QtCore>
#include <QtGui/QApplication>

#include "osg/osgGeometry/ArrowsGroup.h"
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
#include "Qt/web/LOSServer.h"
#include "messenger/Messenger.h"
#include "osg/ModeController.h"
#include "osg/osgModels/LOSMODELSGROUP.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osg2D/MovieKeyHandler.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgWindow/MyViewerEventHandler.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgGIS/postgis_databases_group.h"
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

// Initialize Qt application:

   QApplication app(argc,argv);
//   QCoreApplication app(argc,argv);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);

// Read input data files:
   
   const int ndims=3;
   string OSG_data_dir(getenv("OSG_FILE_PATH"));
   OSG_data_dir += "/";

   PassesGroup passes_group(&arguments);
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
   Pass* cloudpass_ptr=passes_group.get_pass_ptr(cloudpass_ID);
   int texturepass_ID=passes_group.get_curr_texturepass_ID();
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
   string message_sender_ID="QTLOS";

   string message_queue_channel_name="viewer_update";
   Messenger viewer_messenger( 
      broker_URL,message_queue_channel_name,message_sender_ID);
   Messenger EarthRegions_GoogleEarth_messenger( 
      broker_URL,message_queue_channel_name,message_sender_ID);

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->set_thick_client_window_position();
   window_mgr_ptr->initialize_window("3D Viewer");

   window_mgr_ptr->set_auto_generate_movies_flag(true);
//   window_mgr_ptr->set_horiz_scale_factor(0.5);
   window_mgr_ptr->set_horiz_scale_factor(0.66);
//   window_mgr_ptr->set_horiz_scale_factor(0.75);

   ViewerManager* ViewerManager_ptr=dynamic_cast<ViewerManager*>(
      window_mgr_ptr);
   ViewerManager_ptr->get_MyViewerEventHandler_ptr()->
      set_Viewer_Messenger_ptr(&viewer_messenger);
   string tomcat_subdir="/usr/local/apache-tomcat/webapps/LOST/";
   ViewerManager_ptr->get_MyViewerEventHandler_ptr()->
      set_tomcat_subdir(tomcat_subdir);
   ViewerManager_ptr->get_MyViewerEventHandler_ptr()->
      purge_flash_movies();

// Instantiate LOSServer which receives get calls from web page
// buttons that can be mapped onto ModeController state changes:

   string LOSServer_hostname="127.0.0.1";
   int LOSServer_portnumber=4041;
   string LOSServer_URL;
   if (LOSServer_URL.size() > 0)
   {
      LOSServer_hostname=stringfunc::get_hostname_from_URL(LOSServer_URL);
      LOSServer_portnumber=stringfunc::get_portnumber_from_URL(
         LOSServer_URL);
   }
   cout << "LOSServer_hostname = " << LOSServer_hostname
        << " LOSServer_portnumber = " << LOSServer_portnumber
        << endl;

   LOSServer LOS_server(LOSServer_hostname,LOSServer_portnumber);

// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

//   bool display_movie_state=true;
//   bool display_movie_number=true;
//   bool display_movie_world_time=true;
   bool hide_Mode_HUD_flag=true;
   bool display_movie_state=false;
   bool display_movie_number=false;
   bool display_movie_world_time=false;
//   bool hide_Mode_HUD_flag=false;

   Operations operations(
      ndims,window_mgr_ptr,passes_group,
      display_movie_state,display_movie_number,display_movie_world_time,
      hide_Mode_HUD_flag);
   root->addChild(operations.get_OSGgroup_ptr());

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();

// In June 2009, we realized that the OpenLayers map tool within
// Michael Yee's thin client appears to have a bug related to the
// shift key.  When the shift key is depressed to allow for rapid
// zooming into some area on the 2D map and subsequently released, the
// signal for the shift key release is not always successfully
// transmitted.  This causes our Custom3DManipulator class to prohibit
// translation within the thick client.  In order to avoid this
// catastrophic failure mode, we simply disable shift within our thick
// client for the QTLOS main program where it is not needed...

   ModeController_ptr->set_allow_manipulator_translation_flag(true);

// Instantiate AnimationController which acts as master game clock:

   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   AnimationController_ptr->setDelay(0.0);

// Set master game clock's starting time to current UTC, simulation's
// duration and time step:

   double n_simulation_hours=1;     
//   double world_time_step_in_secs_per_frame=5*60;	// secs
   double world_time_step_in_secs_per_frame=10*60;	// secs
//   double world_time_step_in_secs_per_frame=15*60;	// secs
   operations.set_current_master_clock_time_duration_and_step(
      n_simulation_hours,world_time_step_in_secs_per_frame);

// Add a custom manipulator to the event handler list:

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);
   CM_3D_ptr->set_min_camera_height_above_grid(5000);	// meters
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold all decorations:

   Decorations decorations(window_mgr_ptr,ModeController_ptr,CM_3D_ptr);
   decorations.set_disable_keyhandlers_flag(true);

// FAKE FAKE:  Weds Oct 28 at 5:20 pm
// Restore zooming into OBSFRUSTA via doubleclicking for demo purposes only!

   decorations.set_disable_pickhandlers_flag(true);
//   decorations.set_disable_pickhandlers_flag(false);

// Instantiate group to hold movie:

   AnimationController* movie_anim_controller_ptr=new AnimationController;
   movie_anim_controller_ptr->set_master_AnimationController_ptr(
      AnimationController_ptr);

   MoviesGroup movies_group(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),
      movie_anim_controller_ptr);
   movies_group.set_OSGgroup_nodemask(1);
   root->addChild( movies_group.get_OSGgroup_ptr() );

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
   root->addChild(earths_group.get_OSGgroup_ptr());

   Earth* Earth_ptr=earths_group.generate_new_Earth(postgis_db_ptr);

// Comment out next communication section with thin client for debugging
// purposes

// Wait for thin client to transmit 3D map borders, specified UTM zone
// and northern hemisphere flag before instantiating point cloud:

   cout << "LOS_server.get_map_selected_flag() = "
        << LOS_server.get_map_selected_flag() << endl;
   
   while (!LOS_server.get_map_selected_flag())
   {
      string command="SEND_THICKCLIENT_READY_FOR_MAP_INFORMATION";
      viewer_messenger.broadcast_subpacket(command);
      app.processEvents();
   }


   cloudpass_ptr->set_UTM_zonenumber(
      LOS_server.get_specified_UTM_zonenumber());
   cloudpass_ptr->set_northern_hemisphere_flag(
      LOS_server.get_northern_hemisphere_flag());
   cout << "specified_UTM_zone = " << cloudpass_ptr->get_UTM_zonenumber()
        << endl;

   PassInfo* cloudpassinfo_ptr=cloudpass_ptr->get_PassInfo_ptr();
   cloudpassinfo_ptr->set_longitude_lo(LOS_server.get_longitude_lo());
   cloudpassinfo_ptr->set_longitude_hi(LOS_server.get_longitude_hi());
   cloudpassinfo_ptr->set_latitude_lo(LOS_server.get_latitude_lo());
   cloudpassinfo_ptr->set_latitude_hi(LOS_server.get_latitude_hi());
   
   string map_countries_name=LOS_server.get_map_countries_name();
   
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
   decorations.set_PointCloudsGroup_ptr(&clouds_group);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_ptr,CM_3D_ptr));

   
   LatLongGridsGroup latlonggrids_group(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),CM_3D_ptr);
   EarthRegionsGroup earth_regions_group(
      passes_group.get_pass_ptr(cloudpass_ID),      
      &clouds_group,&latlonggrids_group,&movies_group,Earth_ptr);
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
   decorations.set_grid_origin_ptr(grid_origin_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new LatLongGridKeyHandler(ModeController_ptr,LatLongGrid_ptr));

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
  
/*
// ---------------------------------------------------------------------
// FAKE FAKE: Tues April 13, 2010

// Section added into main program in order to export part of
// Afghanistan border intersecting a pre-defined bounding box:

// Hardwire experimental border limits...

      min_lon=69.06005;
      max_lon=72.06005;
      min_lat=32.6425;
      max_lat=35.6425;
      geopoint lower_left(min_lon,min_lat,0,
                          LOS_server.get_specified_UTM_zonenumber());
      geopoint upper_right(max_lon,max_lat,0,
                          LOS_server.get_specified_UTM_zonenumber());
      const double delta=100;	// meters

      bounding_box bbox(lower_left.get_UTM_easting()+delta,
                        upper_right.get_UTM_easting()-delta,
                        lower_left.get_UTM_northing()+delta,
                        upper_right.get_UTM_northing()-delta);
//      cout << "bbox = " << bbox << endl;
//      outputfunc::enter_continue_char();
*/

      Earth_ptr->retrieve_borders_from_PostGIS_database(
         "country_borders",min_lon,max_lon,min_lat,max_lat,
         colorfunc::white);
      Earth_ptr->retrieve_borders_from_PostGIS_database(
         "state_borders",min_lon,max_lon,min_lat,max_lat,
         colorfunc::white);
      Earth_ptr->get_borders_group_ptr()->set_DataGraph_ptr(PointCloud_ptr);

// ---------------------------------------------------------------------
/*
      vector<twovector> restricted_border_points;
      PolyLinesGroup* borders_group_ptr=Earth_ptr->get_borders_group_ptr();
      for (int b=0; b<borders_group_ptr->get_n_Graphicals(); b++)
      {
         PolyLine* curr_PolyLine_ptr=borders_group_ptr->get_PolyLine_ptr(b);
         polyline* curr_polyline_ptr=curr_PolyLine_ptr->get_polyline_ptr();
//         cout << "*curr_polyline_ptr = "
//              << *curr_polyline_ptr << endl;

         for (int v=0; v<curr_polyline_ptr->get_n_vertices(); v++)
         {
            threevector curr_vertex(curr_polyline_ptr->get_vertex(v));
//            cout << "v = " << v << " curr_vertex = " << curr_vertex << endl;
            if (bbox.point_inside(curr_vertex.get(0),curr_vertex.get(1)))
            {
               restricted_border_points.push_back(
                  twovector(curr_vertex.get(0),curr_vertex.get(1)));
            }
         } // loop over index v labeling border vertices
      } // loop over index b labeling country borders

//      cout << "restricted_border_points.size() = "
//           << restricted_border_points.size() << endl;
//      cout << "bbox = " << bbox << endl;
//      outputfunc::enter_continue_char();
      
      string border_filename="border_points.txt";
      ofstream border_stream;
      border_stream.precision(12);
      filefunc::openfile(border_filename,border_stream);
      for (int i=0; i<restricted_border_points.size(); i++)
      {
         const int nprecision=4;
         border_stream << stringfunc::number_to_string(
            restricted_border_points[i].get(0),nprecision) 
                       << "   "
                       << stringfunc::number_to_string(
                          restricted_border_points[i].get(1),nprecision)
                       << endl;
      }
      filefunc::closefile(border_filename,border_stream);
*/

// ---------------------------------------------------------------------

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

// Instantiate Arrows decoration group:

   ArrowsGroup* ArrowsGroup_ptr=decorations.add_Arrows(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   ArrowsGroup_ptr->set_CM_3D_ptr(CM_3D_ptr);

// Instantiate SignPostsGroup to hold ground target locations:

   SignPostsGroup* GroundTarget_SignPostsGroup_ptr=decorations.add_SignPosts(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   GroundTarget_SignPostsGroup_ptr->set_CM_3D_ptr(CM_3D_ptr);

   decorations.add_Features(ndims,passes_group.get_pass_ptr(cloudpass_ID));
 
// Instantiate ROI Polyhedra decoration group:

   PolyhedraGroup* ROI_PolyhedraGroup_ptr=
      decorations.add_Polyhedra(passes_group.get_pass_ptr(cloudpass_ID));
   ROI_PolyhedraGroup_ptr->set_bbox_height(10000);	// meters
   ROI_PolyhedraGroup_ptr->set_altitude_dependent_volume_alphas_flag(true);
   ROI_PolyhedraGroup_ptr->set_CM_3D_ptr(CM_3D_ptr);
  
// Instantiate PolyLine and MODEL decorations groups:

   PolyLinesGroup* Flight_PolyLinesGroup_ptr=decorations.add_PolyLines(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   Flight_PolyLinesGroup_ptr->set_width(5);

   LOSMODELSGROUP* Aircraft_MODELSGROUP_ptr=decorations.add_LOSMODELS(
      passes_group.get_pass_ptr(cloudpass_ID),
      Flight_PolyLinesGroup_ptr,NULL,&operations,&movies_group);
   Aircraft_MODELSGROUP_ptr->set_AircraftModelType(MODELSGROUP::LiMIT);
   Aircraft_MODELSGROUP_ptr->set_ArrowsGroup_ptr(ArrowsGroup_ptr);
   Aircraft_MODELSGROUP_ptr->set_EarthRegionsGroup_ptr(&earth_regions_group);
   Aircraft_MODELSGROUP_ptr->set_ColorGeodeVisitor_ptr(
      clouds_group.get_ColorGeodeVisitor_ptr());
   Aircraft_MODELSGROUP_ptr->set_CM_3D_ptr(CM_3D_ptr);
   Aircraft_MODELSGROUP_ptr->set_GroundTarget_SignPostsGroup_ptr(
      GroundTarget_SignPostsGroup_ptr);
   Aircraft_MODELSGROUP_ptr->set_compute_zface_height_flag(true);
   Aircraft_MODELSGROUP_ptr->set_ROI_PolyhedraGroup_ptr(
      ROI_PolyhedraGroup_ptr);
   Aircraft_MODELSGROUP_ptr->set_PointCloudsGroup_ptr(&clouds_group);
   Aircraft_MODELSGROUP_ptr->set_tomcat_subdir(tomcat_subdir);
   Aircraft_MODELSGROUP_ptr->pushback_Messenger_ptr(&viewer_messenger);
   Aircraft_MODELSGROUP_ptr->assign_MODELSGROUP_Messenger_ptrs();
   Aircraft_MODELSGROUP_ptr->set_raytrace_ground_targets_flag(true);

   Aircraft_MODELSGROUP_ptr->get_movers_group_ptr()->set_Messenger_ptr(
      &EarthRegions_GoogleEarth_messenger);

   Aircraft_MODELSGROUP_ptr->set_instantiate_OBSFRUSTUMPickHandler_flag(true);
//   Aircraft_MODELSGROUP_ptr->set_instantiate_OBSFRUSTUMPickHandler_flag(false);

// On 6/1/09, Melissa Meyers suggested that we adopt 9000 m as a
// reasonable guestimate for the altitude of the G105 sensor.  As of
// 2010, we take aircraft altitude to be measured in feet rather than
// meters.  So we set the aircraft's default altitude to 30,000 ft =
// 9144 meters: 

//   Aircraft_MODELSGROUP_ptr->set_aircraft_altitude(9000);
   Aircraft_MODELSGROUP_ptr->set_aircraft_altitude(9144);

   double min_ground_sensor_range=30*1000;	// meters
   double max_ground_sensor_range=200*1000;	// meters
   double horiz_FOV=120;	// degs
   int roll_sgn=-1;		// right sided

   Aircraft_MODELSGROUP_ptr->compute_OBSFRUSTUM_parameters(
      min_ground_sensor_range,max_ground_sensor_range,horiz_FOV,roll_sgn);

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
   Aircraft_MODELSGROUP_ptr->set_TilesGroup_ptr(TilesGroup_ptr);
 
// Purge contents LOS geotiff files left over from previous running of
// this main program:

   TilesGroup_ptr->purge_tile_files();

   root->addChild(clouds_group.get_OSGgroup_ptr());
   root->addChild(latlonggrids_group.get_OSGgroup_ptr());
   root->addChild(decorations.get_OSGgroup_ptr());

// ========================================================================
// Flags to enable raytracing for ALIRT ladar imagery rather than
// DTED-2 height imagery:

//   bool ladar_height_data_flag=true;
   bool ladar_height_data_flag=false;
   
   if (ladar_height_data_flag)
   {
      GroundTarget_SignPostsGroup_ptr->set_ladar_height_data_flag(true);
      TilesGroup_ptr->set_ladar_height_data_flag(true);
      Aircraft_MODELSGROUP_ptr->set_ladar_height_data_flag(true);
      CM_3D_ptr->set_min_camera_height_above_grid(500);	// meters
      ROI_PolyhedraGroup_ptr->set_bbox_height(2000);	// meters

// Set fixed_to_mutable_colors_flag=true for ladar coloring purposes:

      clouds_group.get_ColorGeodeVisitor_ptr()->
         set_fixed_to_mutable_colors_flag(true);

// Create a DepthPartitionNode to manage partitioning of the scene

      DepthPartitionNode* dpn_node_ptr = new DepthPartitionNode;
      dpn_node_ptr->setActive(true); // Control whether node analyzes scene
      cout << "max_depth = " << dpn_node_ptr->getMaxTraversalDepth() << endl;
      dpn_node_ptr->addChild(root);

// Attach scene graph to the viewer:

      window_mgr_ptr->setSceneData(dpn_node_ptr);
   }
   else
   {
      root->addChild(ColorbarHUD_ptr->getProjection());

// Instantiate compass:

      CompassHUD* CompassHUD_ptr=new CompassHUD(colorfunc::white);
      root->addChild(CompassHUD_ptr->getProjection());
      CM_3D_ptr->set_CompassHUD_ptr(CompassHUD_ptr);
      LOS_server.set_CompassHUD_ptr(CompassHUD_ptr);

// Attach scene graph to the viewer:

      window_mgr_ptr->setSceneData(root);
   }

// ========================================================================

// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_ptr->realize();

   LOS_server.set_PointCloudsGroup_ptr(&clouds_group);
   LOS_server.set_northern_hemisphere_flag(
      earth_regions_group.get_northern_hemisphere_flag());
   LOS_server.set_Operations_ptr(&operations);
   LOS_server.set_FlightPolyLinesGroup_ptr(Flight_PolyLinesGroup_ptr);
   LOS_server.set_Aircraft_MODELSGROUP_ptr(Aircraft_MODELSGROUP_ptr);
   LOS_server.set_TilesGroup_ptr(TilesGroup_ptr);
   LOS_server.set_GroundTarget_SignPostsGroup_ptr(
      GroundTarget_SignPostsGroup_ptr);

   LOS_server.set_tomcat_subdir(tomcat_subdir);

   string command="SEND_THICKCLIENT_READY_FOR_USER_INPUT";
   viewer_messenger.broadcast_subpacket(command);

// ========================================================================

// In January 2010, we ran into major release review problems with
// trying to present the LOST demo at the MIT Tech Fair.  In order to
// satisfy the HAFB release reviewer, we had to suppress all country
// names, city names, borders and lat-lon grid displays.  The
// following few lines can be used to simply turn off these
// identifying geographic markers:

   bool MIT_demo_flag=false;
//   bool MIT_demo_flag=true;
   if (MIT_demo_flag)
   {
      latlonggrids_group.set_OSGgroup_nodemask(0);
//      Earth_ptr->get_drawable_group_ptr()->setNodeMask(0);
   }
   else
   {
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
   }

   int counter=0;
   while( !window_mgr_ptr->done() )
   {
      window_mgr_ptr->process();
      app.processEvents();
   }
   delete window_mgr_ptr;

//   if (postgis_db_ptr != NULL) postgis_db_ptr->disconnect();
   delete postgis_databases_group_ptr;
}
