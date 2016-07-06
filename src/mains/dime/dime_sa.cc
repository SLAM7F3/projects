// ========================================================================
// Program DIME_SA is a variant of program LOS.  It displays a 3D DTED
// map of the southern US with a large and medium city GIS layer.  FSF
// and Athena ship tracks are displayed as cyan and purple curves,
// while aircraft tracks are displayed as red to green curves.



/*

cd /home/cho/programs/c++/svn/projects/src/mains/dime
/home/cho/programs/c++/svn/projects/src/mains/dime/dime_sa \
--region_filename ./packages/florida.pkg \
--GIS_layer ./packages/world_GIS.pkg \
--initial_mode Manipulate_Fused_Data_Mode 

*/



// ========================================================================
// Last updated on 6/26/13; 6/27/13; 7/16/13
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "osg/osgGeometry/ArrowsGroup.h"
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

   string bundler_subdir="./bundler/DIME/";
   string MayFieldtest_subdir=bundler_subdir+"May2013_Fieldtest/";
   string FSFdate_subdir=MayFieldtest_subdir+"05202013/";

   int scene_ID;
   cout << "Enter scene ID:" << endl;
   cin >> scene_ID;
   string scene_ID_str=stringfunc::integer_to_string(scene_ID,2);
   string bundler_IO_subdir=FSFdate_subdir+"Scene"+scene_ID_str+"/";
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;


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

//   bool display_movie_state=true;
   bool display_movie_state=false;
//   bool display_movie_number=true;
   bool display_movie_number=false;
//   bool display_movie_world_time=true;
   bool display_movie_world_time=false;
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
//   clouds_group.set_dependent_coloring_var(1);	// fused Z & P
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

// Construct ColorbarHUDs for terrain:

   double hue_start=240;	// blue
   double hue_stop=0;		// red
   double scalar_value_start=0;	// km
   double scalar_value_stop=7;	// km
   string title="Terrain altitude (kms)";

   ColorbarHUD* ColorbarHUD_ptr=new ColorbarHUD(
      hue_start,hue_stop,scalar_value_start,scalar_value_stop,title);
   clouds_group.set_ColorbarHUD_ptr(ColorbarHUD_ptr);
   
//   root->addChild(ColorbarHUD_ptr->getProjection());
   
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

      cout << "specified UTM zonenumber = "
           << earth_regions_group.get_specified_UTM_zonenumber()
           << endl;
      
      PassInfo* cloudpassinfo_ptr=cloudpass_ptr->get_PassInfo_ptr();

//      Earth_ptr->set_init_border_width(3); 
      Earth_ptr->set_init_border_width(5);
      Earth_ptr->retrieve_borders_from_PostGIS_database(
//         GISlines_tablenames[0],
         "country_borders",
         cloudpassinfo_ptr->get_longitude_lo(),
         cloudpassinfo_ptr->get_longitude_hi(),
         cloudpassinfo_ptr->get_latitude_lo(),
         cloudpassinfo_ptr->get_latitude_hi(),
         colorfunc::white);
      Earth_ptr->retrieve_borders_from_PostGIS_database(
         "state_borders",
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

      int population_threshold=30000;
      Earth_ptr->retrieve_US_cities_from_PostGIS_database(
         cloudpassinfo_ptr->get_longitude_lo(),
         cloudpassinfo_ptr->get_longitude_hi(),
         cloudpassinfo_ptr->get_latitude_lo(),
         cloudpassinfo_ptr->get_latitude_hi(),
         colorfunc::white,population_threshold);

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
//   GroundTarget_SignPostsGroup_ptr->set_colors(
//      colorfunc::cyan,colorfunc::blue);

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

// Instantiate Polyhedra, Polygons & PolyLines decoration groups:

//   PolyhedraGroup* PolyhedraGroup_ptr=decorations.add_Polyhedra(
//      passes_group.get_pass_ptr(cloudpass_ID));

   osgGeometry::PolygonsGroup* PolygonsGroup_ptr=decorations.add_Polygons(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
//   OBSFRUSTAGROUP_ptr->set_PolygonsGroup_ptr(PolygonsGroup_ptr);

   PolyLinesGroup* PolyLinesGroup_ptr=decorations.add_PolyLines(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   PolygonsGroup_ptr->set_PolyLinesGroup_ptr(PolyLinesGroup_ptr);

//   PolyLinesGroup_ptr->set_width(3);
   PolyLinesGroup_ptr->set_width(5);
   PolyLinesGroup_ptr->set_multicolor_flags(true);
   PolyLinesGroup_ptr->set_ID_labels_flag(true);
   PolyLinesGroup_ptr->set_CM_3D_ptr(CM_3D_ptr);
   PolyLinesGroup_ptr->set_altitude_dependent_labels_flag(false);

   PolyLinePickHandler* PolyLinePickHandler_ptr=
      decorations.get_PolyLinePickHandler_ptr();
   PolyLinePickHandler_ptr->set_min_doubleclick_time_spread(0.05);
   PolyLinePickHandler_ptr->set_max_doubleclick_time_spread(0.25);
   PolyLinePickHandler_ptr->set_z_offset(5);

// Import FSF and Athena ship tracks and display as 3D polylines:   

   string Fieldtest_subdir=
      "/data/DIME/bundler/DIME/May2013_Fieldtest/05202013/";
   string SPAN_CPT_subdir=Fieldtest_subdir+"SPAN_CPT/";
   string AFT_PROPAK_subdir=Fieldtest_subdir+"AFT_PROPAK/";

   string Scene_subdir=Fieldtest_subdir+"Scene"+scene_ID_str+"//";
   string stable_frames_subdir=Scene_subdir+"stable_frames/";
   string ship_tracks_subdir=stable_frames_subdir;
   
//   int n_ships=1;
   int n_ships=2;
   for (int ship_ID=0; ship_ID<n_ships; ship_ID++)
   {
//      string track_filename=Fieldtest_subdir+"FSF_UTM_track.dat";
//      string track_filename=Fieldtest_subdir+"Athena_UTM_track.dat";
//      string track_filename=SPAN_CPT_subdir+"FSF_UTM_track.dat";
//      string track_filename=AFT_PROPAK_subdir+"FSF_UTM_track.dat";      
      string track_filename=ship_tracks_subdir+"FSF_UTM_track.dat";
      if (ship_ID==1)
      {
//         track_filename=SPAN_CPT_subdir+"FSF_UTM_track.dat";
         track_filename=ship_tracks_subdir+"Athena_UTM_track.dat";
      }
      filefunc::ReadInfile(track_filename);

      cout << "ship_ID = " << ship_ID
           << " text_line.size() = "
           << filefunc::text_line.size() << endl;

      int track_ID=-1;
      vector<threevector> V;
      for (unsigned int i=0; i<filefunc::text_line.size(); i++)
      {
         vector<double> column_values=stringfunc::string_to_numbers(
            filefunc::text_line[i]);

         track_ID=column_values[0];
//         double curr_time=column_values[1];
         double easting=column_values[2];
         double northing=column_values[3];
         double altitude=column_values[4];
//      altitude=20;	// meters
         altitude=50;	// meters
//      altitude=100;	// meters
         threevector curr_ship_posn(easting,northing,altitude);
         V.push_back(curr_ship_posn);

      } // loop over index i labeling filefunc::text_line
   
      vector<osg::Vec4> track_colors;

      hue_start=hue_stop=180;		// cyan for FSF
      if (ship_ID==1)
      {
         hue_start=hue_stop=300;	// purple for Athena
      }
      
      double v_start=1;
      double v_stop=0.2;
      for (unsigned int j=0; j<V.size(); j++)
      {
         double curr_frac=double(j)/(V.size()-1);
         double hue=hue_start+(hue_stop-hue_start)*curr_frac;
         double s=1;
         double v=v_start+(v_stop-v_start)*curr_frac;
         double r,g,b;
         colorfunc::hsv_to_RGB(hue,s,v,r,g,b);
         track_colors.push_back(osg::Vec4(r,g,b,1));
      }

      bool force_display_flag=false;
      bool single_polyline_per_geode_flag=true;
      int n_text_messages=1;
      int PolyLine_ID=track_ID;

//    osg::Vec4 track_color=colorfunc::get_OSG_color(colorfunc::red);
//   PolyLine* PolyLine_ptr=PolyLinesGroup_ptr->generate_new_PolyLine(
//      V.front(),V,track_color,
//      force_display_flag,single_polyline_per_geode_flag,
//      n_text_messages,PolyLine_ID);

      PolyLine* PolyLine_ptr=PolyLinesGroup_ptr->generate_new_PolyLine(
         V.front(),V,track_colors,
         force_display_flag,single_polyline_per_geode_flag,
         n_text_messages,PolyLine_ID);

   } // loop over ship_ID labeling FSF & Athena

// Import aerial tracks and display as 3D polylines:

   string aerial_tracks_subdir=bundler_IO_subdir+"aerial_tracks/";
   vector<string> aerial_track_filenames=filefunc::files_in_subdir(
      aerial_tracks_subdir);
   string aerial_track_filename=aerial_track_filenames.back();
   filefunc::ReadInfile(aerial_track_filename);

   cout << "aerial_track_filename = " << aerial_track_filename << endl;

   int track_ID=-10000;
   vector<threevector> V;
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);

      if ( (column_values[0] != track_ID && track_ID >= 0) ||
      i==filefunc::text_line.size()-1 )
      {
         int track_counter=track_ID-10;
         double min_track_hue=0;	// red
         double max_track_hue=120;	// green
         double delta_track_hue=75;

         double curr_track_hue=min_track_hue+track_counter*delta_track_hue;
         curr_track_hue=basic_math::phase_to_canonical_interval(
            curr_track_hue,min_track_hue,max_track_hue);
         double sat=1;
         double v_start=1;
         double v_stop=0.2;

         vector<osg::Vec4> track_colors;
         for (unsigned int j=0; j<V.size(); j++)
         {
            double curr_frac=double(j)/(V.size()-1);
            double v=v_start+(v_stop-v_start)*curr_frac;

            double r,g,b;
            colorfunc::hsv_to_RGB(curr_track_hue,sat,v,r,g,b);

            colorfunc::RGB curr_rgb(r,g,b);
            osg::Vec4 track_color=colorfunc::get_OSG_color(curr_rgb);
            track_colors.push_back(track_color);
         }

         bool force_display_flag=false;
         bool single_polyline_per_geode_flag=true;
         int n_text_messages=1;
         int PolyLine_ID=track_ID;

//         PolyLine* PolyLine_ptr=PolyLinesGroup_ptr->generate_new_PolyLine(
//            V.front(),V,track_color,
//            force_display_flag,single_polyline_per_geode_flag,
//            n_text_messages,PolyLine_ID);

         PolyLine* PolyLine_ptr=PolyLinesGroup_ptr->generate_new_PolyLine(
            V.front(),V,track_colors,
            force_display_flag,single_polyline_per_geode_flag,
            n_text_messages,PolyLine_ID);

         V.clear();
      }

      track_ID=column_values[0];
//      double curr_time=column_values[1];
      double easting=column_values[2];
      double northing=column_values[3];
      double altitude=column_values[4];
      threevector curr_aircraft_posn(easting,northing,altitude);
      V.push_back(curr_aircraft_posn);
   } // loop over index i labeling line within aerial tracks file
   
   cout << "PolyLinesGroup_ptr->get_n_Graphicals() = "
        << PolyLinesGroup_ptr->get_n_Graphicals() << endl;

   PolyLinesGroup_ptr->set_OSGgroup_nodemask(1);	
   clouds_group.set_OSGgroup_nodemask(0);		// hide point cloud

   root->addChild(clouds_group.get_OSGgroup_ptr());
   root->addChild(latlonggrids_group.get_OSGgroup_ptr());
   root->addChild(decorations.get_OSGgroup_ptr());

// Attach scene graph to the viewer:

   window_mgr_ptr->setSceneData(root);

// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_ptr->realize();

   while( !window_mgr_ptr->done() )
   {
      window_mgr_ptr->process();
   }
   delete window_mgr_ptr;

   delete postgis_databases_group_ptr;
}
