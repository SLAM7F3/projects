// ========================================================================
// Program LOS is a variant of program TESTCITIES.  It is an
// experimental playground for working with Afghanistan DTED data.
// ========================================================================
// Last updated on 9/28/09; 10/7/09; 5/12/10; 12/21/10
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
#include "osg/osgModels/LOSMODELSGROUP.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osg2D/MovieKeyHandler.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgTiles/TilesGroup.h"
#include "osg/osgWindow/ViewerManager.h"

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

// Read input data files:
   
   const int ndims=3;
   string OSG_data_dir(getenv("OSG_FILE_PATH"));
   OSG_data_dir += "/";

   PassesGroup passes_group(&arguments);
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
//   int texturepass_ID=passes_group.get_curr_texturepass_ID();
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
   
// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("3D imagery");
   window_mgr_ptr->set_auto_generate_movies_flag(true);
//   window_mgr_ptr->set_horiz_scale_factor(0.5);
   window_mgr_ptr->set_horiz_scale_factor(0.66);
//   window_mgr_ptr->set_horiz_scale_factor(0.75);

// Instantiate ActiveMQ messengers:

   string broker_URL="tcp://127.0.0.1:61616";
//   cout << "ActiveMQ broker_URL = " << broker_URL << endl;
   string viewer_message_queue_channel_name="viewer_update";
   Messenger viewer_messenger( 
      broker_URL, viewer_message_queue_channel_name );
   string progress_message_queue_channel_name="progress";
   Messenger progress_messenger( 
      broker_URL, progress_message_queue_channel_name );

// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

   bool display_movie_state=true;
   bool display_movie_number=true;
   bool display_movie_world_time=true;
   bool hide_Mode_HUD_flag=false;
   Operations operations(
      ndims,window_mgr_ptr,passes_group,
      display_movie_state,display_movie_number,display_movie_world_time,
      hide_Mode_HUD_flag);
   root->addChild(operations.get_OSGgroup_ptr());

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();

// Instantiate AnimationController which acts as master game clock:

   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();

// Set master game clock's starting time to current UTC, simulation's
// duration and time step:

   double n_simulation_hours=1;     
   double world_time_step_in_secs_per_frame=10*60;	// secs
//   double world_time_step_in_secs_per_frame=15*60;	// secs
   operations.set_current_master_clock_time_duration_and_step(
      n_simulation_hours,world_time_step_in_secs_per_frame);
   
// Add a custom manipulator to the event handler list:

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);
   CM_3D_ptr->set_min_camera_height_above_grid(5000);	// meters
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate compass:

   CompassHUD* CompassHUD_ptr=new CompassHUD(colorfunc::white);
   root->addChild(CompassHUD_ptr->getProjection());
   CM_3D_ptr->set_CompassHUD_ptr(CompassHUD_ptr);

// Instantiate group to hold all decorations:

   Decorations decorations(window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

// Instantiate group to hold movie:

   AnimationController* movie_anim_controller_ptr=new AnimationController;
   movie_anim_controller_ptr->set_master_AnimationController_ptr(
      AnimationController_ptr);
//   PassInfo* texture_passinfo_ptr=
//      passes_group.get_passinfo_ptr(texturepass_ID);
//   cout << "*texture_passinfo_ptr = " << *texture_passinfo_ptr << endl;

   MoviesGroup movies_group(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),
      movie_anim_controller_ptr);
   movies_group.set_OSGgroup_nodemask(1);
   root->addChild( movies_group.get_OSGgroup_ptr() );

   MovieKeyHandler* MoviesKeyHandler_ptr=
      new MovieKeyHandler(ModeController_ptr,&movies_group);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(MoviesKeyHandler_ptr);

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
   bool place_onto_bluemarble_flag=false;
   earth_regions_group.generate_regions(
      passes_group,place_onto_bluemarble_flag);

   PointCloud* PointCloud_ptr=clouds_group.get_Cloud_ptr(0);
   LatLongGrid* LatLongGrid_ptr=latlonggrids_group.get_Grid_ptr(0);
   PointCloud_ptr->set_Grid_ptr(LatLongGrid_ptr);
   clouds_group.set_dependent_coloring_var(1);	// fused Z & P
//   LatLongGrid_ptr->set_depth_buffering_off_flag(false);
   LatLongGrid_ptr->set_depth_buffering_off_flag(true);
   LatLongGrid_ptr->set_init_linewidth(2.0);
   LatLongGrid_ptr->set_text_size_prefactor(9.0E-4);
//   LatLongGrid_ptr->set_text_size_prefactor(1.0E-3);
   threevector* grid_origin_ptr=LatLongGrid_ptr->get_world_origin_ptr();
   cout << "*grid_origin_ptr = " << *grid_origin_ptr << endl;
   CM_3D_ptr->set_Grid_ptr(LatLongGrid_ptr);
   decorations.set_grid_origin_ptr(grid_origin_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new LatLongGridKeyHandler(ModeController_ptr,LatLongGrid_ptr));

// Construct ColorbarHUDs for terrain, raytracing occlusion results
// and SAM threats:

   double hue_start=240;	// blue
   double hue_stop=0;		// red
   double scalar_value_start=0;	// km
   double scalar_value_stop=7;	// km
   string title="Terrain altitude (kms)";

   ColorbarHUD* ColorbarHUD_ptr=new ColorbarHUD(
      hue_start,hue_stop,scalar_value_start,scalar_value_stop,title);

   hue_start=120;	// green
   hue_stop=0;		// red
   scalar_value_start=0;
   scalar_value_stop=100;
   title="Occlusion Percentage";

   ColorbarHUD_ptr->pushback_hue_start(hue_start);
   ColorbarHUD_ptr->pushback_hue_stop(hue_stop);
   ColorbarHUD_ptr->pushback_scalar_value_start(scalar_value_start);
   ColorbarHUD_ptr->pushback_scalar_value_stop(scalar_value_stop);
   ColorbarHUD_ptr->pushback_title(title);

   hue_start=300;		// purple
   hue_stop=300;	       	// purple
   scalar_value_start=0;	// km**2
   scalar_value_stop=100;	// km**2
   title="Ground area in SAM range of aircraft (sq km)";

   ColorbarHUD_ptr->pushback_hue_start(hue_start);
   ColorbarHUD_ptr->pushback_hue_stop(hue_stop);
   ColorbarHUD_ptr->pushback_scalar_value_start(scalar_value_start);
   ColorbarHUD_ptr->pushback_scalar_value_stop(scalar_value_stop);
   ColorbarHUD_ptr->pushback_title(title);

   clouds_group.set_ColorbarHUD_ptr(ColorbarHUD_ptr);
   
   root->addChild(ColorbarHUD_ptr->getProjection());
   
// Extract city and country border information from PostGis database:

   double xmin=PointCloud_ptr->get_xyz_bbox().xMin();
   double xmax=PointCloud_ptr->get_xyz_bbox().xMax();
   double ymin=PointCloud_ptr->get_xyz_bbox().yMin();
   double ymax=PointCloud_ptr->get_xyz_bbox().yMax();
//   cout << "xmin = " << xmin << " xmax = " << xmax
//        << " ymin = " << ymin << " ymax = " << ymax << endl;

   if (postgis_db_ptr != NULL)
      postgis_db_ptr->pushback_gis_bbox(xmin,xmax,ymin,ymax);

   if (GISlayer_IDs.size() > 0)
   {
      vector<string> GISlines_tablenames=
         passes_group.get_pass_ptr(GISlayer_IDs[0])->
         get_PassInfo_ptr()->get_gislines_tablenames();

      postgis_db_ptr->set_specified_UTM_zonenumber(
         earth_regions_group.get_specified_UTM_zonenumber());
      PassInfo* cloudpassinfo_ptr=cloudpass_ptr->get_PassInfo_ptr();

//      Earth_ptr->set_init_border_width(3); 
      Earth_ptr->set_init_border_width(5);
      Earth_ptr->retrieve_borders_from_PostGIS_database(
         GISlines_tablenames[0],
         cloudpassinfo_ptr->get_longitude_lo(),
         cloudpassinfo_ptr->get_longitude_hi(),
         cloudpassinfo_ptr->get_latitude_lo(),
         cloudpassinfo_ptr->get_latitude_hi(),
         colorfunc::white);
      Earth_ptr->get_borders_group_ptr()->set_DataGraph_ptr(PointCloud_ptr);

      Earth_ptr->retrieve_cities_from_PostGIS_database(
         cloudpassinfo_ptr->get_longitude_lo(),
         cloudpassinfo_ptr->get_longitude_hi(),
         cloudpassinfo_ptr->get_latitude_lo(),
         cloudpassinfo_ptr->get_latitude_hi(),
         colorfunc::white);

      decorations.get_OSGgroup_ptr()->addChild(
         Earth_ptr->get_borders_group_ptr()->get_OSGgroup_ptr());
      decorations.get_OSGgroup_ptr()->addChild(
         Earth_ptr->get_countries_group_ptr()->get_OSGgroup_ptr());
      decorations.get_OSGgroup_ptr()->addChild(
         Earth_ptr->get_cities_group_ptr()->get_OSGgroup_ptr());
   }

//   colorfunc::Color contrasting_color=colorfunc::orange;
//   Earth_ptr->get_borders_group_ptr()->set_uniform_color(
//      colorfunc::get_OSG_color(contrasting_color));

//   Earth_ptr->get_cities_group_ptr()->reset_text_color(
//      0,colorfunc::get_OSG_color(contrasting_color));
//   Earth_ptr->get_countries_group_ptr()->reset_text_color(
//      0,colorfunc::get_OSG_color(contrasting_color));
//   Earth_ptr->get_cities_group_ptr()->reset_colors();
//   Earth_ptr->get_countries_group_ptr()->reset_colors();
   
//   postgis_db_ptr->set_city_color(contrasting_color);
//   postgis_db_ptr->set_country_name_color(contrasting_color);

// Instantiate a PointFinder;

   PointFinder pointfinder(&clouds_group);
   if (!passes_group.get_pick_points_on_Zplane_flag())
   {
      CM_3D_ptr->set_PointFinder(&pointfinder);
   }
   
// Instantiate a MyDatabasePager to handle paging of files from disk:

//   viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager(
//      clouds_group.get_SetupGeomVisitor_ptr(),
//      clouds_group.get_ColorGeodeVisitor_ptr());
   
// Instantiate Arrows decoration group:

   ArrowsGroup* ArrowsGroup_ptr=decorations.add_Arrows(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   ArrowsGroup_ptr->set_CM_3D_ptr(CM_3D_ptr);

// Instantiate SignPostsGroup to hold ground target locations:

   SignPostsGroup* GroundTarget_SignPostsGroup_ptr=decorations.add_SignPosts(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   GroundTarget_SignPostsGroup_ptr->set_CM_3D_ptr(CM_3D_ptr);
//   GroundTarget_SignPostsGroup_ptr->set_colors(
//      colorfunc::cyan,colorfunc::blue);

   const int n_ground_targets=1000;
   for (int n=0; n<n_ground_targets; n++)
   {
      string target_label="Target "+stringfunc::number_to_string(n);
      GroundTarget_SignPostsGroup_ptr->set_fixed_label_to_SignPost_ID(
         n,target_label);
   }

   decorations.add_Features(ndims,passes_group.get_pass_ptr(cloudpass_ID));

// Instantiate cylinders decoration group:

   CylindersGroup* CylindersGroup_ptr=
      decorations.add_Cylinders(
         passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
   CylindersGroup_ptr->set_rh(5,0.35);
   decorations.get_CylinderPickHandler_ptr()->set_text_size(8);
   decorations.get_CylinderPickHandler_ptr()->
      set_text_screen_axis_alignment_flag(false);

/*
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

   double Zplane_altitude=20;	// meters (appropriate for NYC demo)
   bool multicolor_frusta_flag=false;
   bool initially_mask_all_frusta_flag=true;
   OBSFRUSTAGROUP_ptr->generate_still_imagery_frusta_from_projection_matrices(
      passes_group,Zplane_altitude,
      multicolor_frusta_flag,initially_mask_all_frusta_flag);
*/

// Instantiate PolyLine and MODEL decorations groups:

   PolyLinesGroup* Flight_PolyLinesGroup_ptr=decorations.add_PolyLines(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   PolyLinePickHandler* Flight_PolyLinePickHandler_ptr=
      decorations.get_PolyLinePickHandler_ptr();
   Flight_PolyLinePickHandler_ptr->set_permanent_color(colorfunc::red);
   Flight_PolyLinePickHandler_ptr->set_generate_polyline_kdtree_flag(false);
   Flight_PolyLinesGroup_ptr->set_width(5);

//   bool instantiate_MODELPickHandler_flag=false;
   LOSMODELSGROUP* Aircraft_MODELSGROUP_ptr=decorations.add_LOSMODELS(
      passes_group.get_pass_ptr(cloudpass_ID),
      Flight_PolyLinesGroup_ptr,Flight_PolyLinePickHandler_ptr,
      &operations,&movies_group);
   decorations.get_LOSMODELSKeyHandler_ptr()->
      set_PointCloudsGroup_ptr(&clouds_group);

// FAKE FAKE : Tuesday, September 8, 2009 at 5:37 pm.
// Set double LiMIT lobe pattern to false to speed up alg development...

   Aircraft_MODELSGROUP_ptr->set_double_LiMIT_lobe_pattern_flag(false);

   Aircraft_MODELSGROUP_ptr->set_AircraftModelType(MODELSGROUP::LiMIT);
   Aircraft_MODELSGROUP_ptr->set_aircraft_altitude(9000);
   Aircraft_MODELSGROUP_ptr->set_ArrowsGroup_ptr(ArrowsGroup_ptr);
   Aircraft_MODELSGROUP_ptr->set_EarthRegionsGroup_ptr(&earth_regions_group);
   Aircraft_MODELSGROUP_ptr->set_ColorGeodeVisitor_ptr(
      clouds_group.get_ColorGeodeVisitor_ptr());
   Aircraft_MODELSGROUP_ptr->set_GroundTarget_SignPostsGroup_ptr(
      GroundTarget_SignPostsGroup_ptr);
   Aircraft_MODELSGROUP_ptr->set_compute_zface_height_flag(true);
   Aircraft_MODELSGROUP_ptr->set_instantiate_OBSFRUSTUMPickHandler_flag(true);
   Aircraft_MODELSGROUP_ptr->set_OBSFRUSTUM_az_extent(120*PI/180);
   Aircraft_MODELSGROUP_ptr->set_OBSFRUSTUM_el_extent(80*PI/180);
   Aircraft_MODELSGROUP_ptr->set_OBSFRUSTUM_roll(63*PI/180);
    Aircraft_MODELSGROUP_ptr->set_tomcat_subdir(
      "/usr/local/apache-tomcat/webapps/LOST/");
   Aircraft_MODELSGROUP_ptr->pushback_Messenger_ptr(&viewer_messenger);
//   Aircraft_MODELSGROUP_ptr->pushback_Messenger_ptr(&progress_messenger);
   Aircraft_MODELSGROUP_ptr->assign_MODELSGROUP_Messenger_ptrs();

// Instantiate TilesGroup to perform raytracing computations:

   TilesGroup* TilesGroup_ptr=new TilesGroup();
   TilesGroup_ptr->set_min_long(60);	// degs
   TilesGroup_ptr->set_max_long(78);	// degs
   TilesGroup_ptr->set_min_lat(30);	// degs
   TilesGroup_ptr->set_max_lat(42);	// degs
   TilesGroup_ptr->set_specified_UTM_zonenumber(42);	// Afghanistan
   TilesGroup_ptr->set_northern_hemisphere_flag(true);	
   TilesGroup_ptr->set_tomcat_subdir(
      "/usr/local/apache-tomcat/webapps/LOST/");
   TilesGroup_ptr->set_geotif_subdir("/data/DTED/Afghanistan/geotif/");
   Aircraft_MODELSGROUP_ptr->set_TilesGroup_ptr(TilesGroup_ptr);

// Purge contents LOS geotiff files left over from previous running of
// this main program:

   TilesGroup_ptr->purge_tile_files();
   
// Instantiate CentersGroup and center pick handler to handle mid
// point selection:

   CentersGroup centers_group(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),
      AnimationController_ptr,grid_origin_ptr);
   CenterPickHandler* CenterPickHandler_ptr=new CenterPickHandler(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),
      CM_3D_ptr,&centers_group,ModeController_ptr,window_mgr_ptr,
      grid_origin_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      CenterPickHandler_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new CentersKeyHandler(ndims,CenterPickHandler_ptr,ModeController_ptr));

// Attach all 3D graphicals to CentersGroup SpinTransform which can
// perform global rotation about some axis coming out of a user
// selected center location.  On 2/5/07, we learned (the painful and
// hard way!) that the order in which nodes are added to the
// SpinTransform is important for alpha-blending.  In particular, we
// must add decorations' OSGgroup AFTER adding clouds_group OSGgroup
// if alpha blending of 3D video imagery is to work...

   centers_group.get_SpinTransform_ptr()->addChild(
      clouds_group.get_OSGgroup_ptr());
   centers_group.get_SpinTransform_ptr()->addChild(
      latlonggrids_group.get_OSGgroup_ptr());
   centers_group.get_SpinTransform_ptr()->addChild(
      decorations.get_OSGgroup_ptr());

   root->addChild(centers_group.get_SpinTransform_ptr());
//   root->addChild(decorations.get_OSGgroup_ptr());

// Attach scene graph to the viewer:

   window_mgr_ptr->setSceneData(root);

// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_ptr->realize();

// ========================================================================
// For NYC demo purposes, automatically retrieve all signposts from
// PostGIS database:

//   colorfunc::Color signposts_color=colorfunc::white;
//   colorfunc::Color signposts_color=colorfunc::red;
//   double SignPost_size=0.5;
//   SignPostsGroup_ptr->retrieve_all_signposts_from_PostGIS_databases(
//      signposts_color,SignPost_size);
//   decorations.get_SignPostPickHandler_ptr()->
//      set_allow_doubleclick_in_manipulate_fused_data_mode(true);

   while( !window_mgr_ptr->done() )
   {
      window_mgr_ptr->process();
   }
   delete window_mgr_ptr;

   delete postgis_databases_group_ptr;
}
