// ========================================================================
// Program EARTH is a playground for working with a draped ellipsoid
// representation of the earth which is endowed with an ECI coordinate
// system.  Pointclouds (of Lowell and Boston) can be mapped onto the
// ellipsoid's surface.

//		       earth lowell.xyzp

//		       earth cambridge.osga

//  earth /data/alirt2/Boston_2005/boston_30cm_tiles/fusion_osga/*.osga

//  earth /data/alirt2/Boston_2005/boston_30cm_tiles/fusion_osga/*.osga --newpass baghdad*.osga 


//  earth /data/alirt2/Boston_2005/boston_30cm_tiles/fusion_osga/*.osga --newpass /data/alirt2/Boston_2005/boston_30cm_tiles/boston_osga/copley.vid

//  earth /data/alirt2/Boston_2005/boston_30cm_tiles/fusion_osga/*.osga --newpass baghdad*.osga --newpass /data/alirt2/Boston_2005/boston_30cm_tiles/boston_osga/copley.vid

// earth /data/alirt2/Boston_2005/boston_30cm_tiles/fusion_osga/*.osga --newpass baghdad*.osga --newpass /home/cho/programs/c++/svn/projects/src/mains/OSG/HAFB_all.vid

//  earth /data/alirt2/Boston_2005/boston_30cm_tiles/fusion_osga/*.osga --newpass baghdad*.osga --newpass /home/cho/programs/c++/svn/projects/src/mains/OSG/HAFB_all.vid --region_filename sanclemente_EO.pkg

//  earth /data/alirt2/Boston_2005/boston_30cm_tiles/fusion_osga/*.osga --newpass /media/usbdisk/new_york/rtv/osga/height/*.osga

//  earth --region_filename boston.pkg --region_filename baghdad.pkg --newpass /home/cho/programs/c++/svn/projects/src/mains/OSG/HAFB_all.vid --region_filename sanclemente_EO.pkg

//  earth --region_filename boston.pkg --region_filename /home/cho/programs/c++/svn/projects/src/mains/newyork/nyc_rtv.pkg --region_filename baghdad.pkg --newpass /home/cho/programs/c++/svn/projects/src/mains/OSG/HAFB_all.vid --region_filename sanclemente_EO.pkg

//  earth --region_filename boston.pkg --region_filename baghdad.pkg --newpass /home/cho/programs/c++/svn/projects/src/mains/OSG/HAFB_all.vid --region_filename sanclemente_EO.pkg

//  earth --region_filename fused_baghdad.pkg --newpass /home/cho/programs/c++/svn/projects/src/mains/OSG/HAFB_all.vid 


// ========================================================================
// Last updated on 6/14/07; 6/16/07; 6/17/07; 6/26/07
// ========================================================================

#include <string>
#include <vector>
#include <osg/Light>
#include <osg/LightSource>
#include <osg/LightModel>
#include <osgGA/NodeTrackerManipulator>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgUtil/SceneView>

#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGraphicals/AnimationKeyHandler.h"
#include "osg/AnimationPathCreator.h"
#include "astro_geo/Clock.h"
#include "color/colorfuncs.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgSceneGraph/DepthPartitionNode.h"
#include "osg/osgGrid/EarthGrid.h"
#include "osg/osgEarth/EarthsGroup.h"
#include "osg/osgEarth/EarthKeyHandler.h"
#include "osg/osgEarth/EarthManipulator.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "astro_geo/Ellipsoid_model.h"
#include "general/filefuncs.h"
#include "osg/osgGrid/GridKeyHandler.h"
#include "astro_geo/latlong2utmfuncs.h"
#include "osg/osgGrid/LatLongGridsGroup.h"
#include "osg/ModeController.h"
#include "osg/ModeKeyHandler.h"
#include "osg/osgModels/ModelsGroup.h"
#include "osg/osg2D/Moviefuncs.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgModels/ObsFrustaGroup.h"
#include "osg/osgfuncs.h"
#include "passes/PassesGroup.h"
#include "osg/osgSpace/PlanetsGroup.h"
#include "osg/osgSpace/PlanetKeyHandler.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGIS/postgis_database.h"
#include "osg/osgWindow/ViewerManager.h"

#include "general/outputfuncs.h"

// ========================================================================
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

// Instantiate clock to keep track of real time:

   Clock clock;
   clock.current_local_time_and_UTC();

/*
   int year=2006;
   int month=8;
   int day=24;
//   int local_hour=12;
//   cout << "Enter local hour:" << endl;
//   cin >> local_hour;
//   int UTC_hour=11;
   int UTC_hour=17;
//   cout << "Enter UTC hour:" << endl;
//   cin >> UTC_hour;
   int minutes=0;
   double secs=0;

//   clock.set_UTM_zone_time_offset(19);	// Boston
//   clock.set_daylight_savings_flag(true);
//   clock.set_local_time(year,month,day,local_hour,minutes,secs);
   clock.set_UTC_time(year,month,day,UTC_hour,minutes,secs);
*/

// Instantiate database object to send data to and retrieve data from
// an external Postgres database:

//   string hostname="localhost";
//   string hostname="fusion1";
   string hostname="sks";
//   string database_name="world_model";
//   string database_name="isdsid_dev";
   string database_name="isdsid_stage_sar";	// 2/7/07 dry run
//   string username="junk";
   string username="sks";
   postgis_database worldmodel_db(hostname,database_name,username);

//   hostname="fusion1";
   hostname="localhost";
//   database_name="babygis";
   database_name="isds_gis";
//   username="junk";
   username="cho";
   postgis_database babygis_db(hostname,database_name,username);

//   string TableName="north_american_boundaries";
   string TableName="country_borders";
   babygis_db.read_table(TableName);
   const double xmin = -181;	// degs
   const double xmax = 181;	// degs
   const double ymin = -80;	// degs
   const double ymax = 80;	// degs
   babygis_db.pushback_gis_bbox(xmin,xmax,ymin,ymax);
//   babygis_db.setup_coordinate_transformation("NAD83");

// Read input data files:

   const int ndims=3;
   string OSG_data_dir(getenv("OSG_FILE_PATH"));
   OSG_data_dir += "/";

   PassesGroup passes_group(&arguments);
   int texturepass_ID=passes_group.get_curr_texturepass_ID();
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
   int videopass_ID=passes_group.get_curr_videopass_ID();
   PassInfo* PassInfo_ptr=passes_group.get_passinfo_ptr(cloudpass_ID);

   string solarsystem_dir=OSG_data_dir+"SolarSystem/";
   string earth_filename=solarsystem_dir+"earth_bright60.osga";
   int earthpass_ID=passes_group.generate_new_pass(earth_filename);
   passes_group.get_pass_ptr(earthpass_ID)->set_passtype(Pass::earth);
   cout << "earthpass_ID = " << earthpass_ID << endl;
   cout << "cloudpass_ID = " << cloudpass_ID << endl;
   cout << "videopass_ID = " << videopass_ID << endl;
   cout << "texturepass_ID = " << texturepass_ID << endl;

// Construct the viewer and set it up with sensible event handlers:

   osgProducer::Viewer viewer(arguments);
   viewer.setClearColor(osg::Vec4(0,0,0,0));
   viewer.setUpViewer( osgProducer::Viewer::ESCAPE_SETS_DONE );

// Initialize viewer window:

   Producer::RenderSurface* rs_ptr =
      viewer.getCameraConfig()->getCamera(0)->getRenderSurface();
   string window_title="Earth";
   osgfunc::initialize_window(rs_ptr,window_title);

// Create a DepthPartitionNode to manage partitioning of the scene

   DepthPartitionNode* root = new DepthPartitionNode;
   root->setActive(true);     // Control whether the node analyzes the scene
//   cout << "max_depth = " << root->getMaxTraversalDepth() << endl;

// Instantiate a mode controller and mode key event handler:

   ModeController* ModeController_ptr=new ModeController();
   ModeController_ptr->setState(ModeController::MANIPULATE_EARTH);
   viewer.getEventHandlerList().push_back( 
      new ModeKeyHandler(ModeController_ptr) );
   root->addChild(osgfunc::create_Mode_HUD(ndims,ModeController_ptr));

// Instantiate animation controller & key handler:

   AnimationController* AnimationController_ptr=new AnimationController();
   root->addChild(AnimationController_ptr->get_OSGgroup_ptr());
   AnimationKeyHandler* AnimationKeyHandler_ptr=
      new AnimationKeyHandler(ModeController_ptr,AnimationController_ptr);
   viewer.getEventHandlerList().push_back( AnimationKeyHandler_ptr);
//   bool display_movie_state=false;
   bool display_movie_state=true;
//   bool display_movie_number=false;
   bool display_movie_number=true;
//   root->addChild(Moviefunc::create_Imagenumber_HUD(
//      AnimationController_ptr,display_movie_state,display_movie_number));

// Instantiate Earth grid:

   EarthGrid* earthgrid_ptr = new EarthGrid(colorfunc::grey);
   threevector* earthgrid_origin_ptr=earthgrid_ptr->get_world_origin_ptr();
   viewer.getEventHandlerList().push_back(
      new GridKeyHandler(ModeController_ptr,earthgrid_ptr));
//   root->addChild(earthgrid_ptr->get_geode_ptr());

// Instantiate PlanetsGroup to hold sun and planets:

   PlanetsGroup planets_group(
      passes_group.get_pass_ptr(earthpass_ID),&clock,earthgrid_origin_ptr);
   root->addChild(planets_group.get_EarthSpinTransform_ptr());

// Generate the earth as well the solar system:

   DataGraph* EarthGraph_ptr=planets_group.generate_EarthGraph();
   osg::Group* solarsystem_ptr=planets_group.generate_solarsystem(
      earthgrid_ptr);
   root->addChild(solarsystem_ptr);
   viewer.getEventHandlerList().push_back(
      new PlanetKeyHandler(&planets_group,ModeController_ptr));

// Instantiate a PointFinder;

   PointFinder pointfinder(&planets_group);

// Instantiate an EarthsGroup to hold "blue marble" coordinate system
// information:

   EarthsGroup earths_group(
      passes_group.get_pass_ptr(earthpass_ID),&clock,earthgrid_origin_ptr);
   planets_group.get_EarthSpinTransform_ptr()->
      addChild(earths_group.get_OSGgroup_ptr());
   Earth* Earth_ptr=earths_group.generate_new_Earth(&babygis_db);
   Earth_ptr->retrieve_borders_from_babygis_database();
   Earth_ptr->retrieve_cities_from_babygis_database();

// Add a custom manipulator to the event handler list:

   ViewerManager window_mgr;
   window_mgr.set_Viewer_ptr(&viewer);

   osgGA::EarthManipulator* CM_3D_ptr=new osgGA::EarthManipulator(
      ModeController_ptr,Earth_ptr->get_Ellipsoid_model_ptr(),&clock,
      &window_mgr);
   CM_3D_ptr->set_PointFinder(&pointfinder);
   Earth_ptr->set_EarthManipulator_ptr(CM_3D_ptr);

   window_mgr.set_CameraManipulator(CM_3D_ptr);
   viewer.getEventHandlerList().push_back(
      new EarthKeyHandler(Earth_ptr,CM_3D_ptr,ModeController_ptr));

// Instantiate group to hold all decorations:

   Decorations decorations(
      &window_mgr,ModeController_ptr,CM_3D_ptr,
      earthgrid_origin_ptr);

// Generate random background star field:

   root->addChild(planets_group.generate_starfield(earthgrid_ptr));

// Instantiate groups to hold multiple point clouds,
// latitude-longitude grids and associated earth regions:

   PointCloudsGroup clouds_group(
      &arguments,passes_group.get_pass_ptr(cloudpass_ID));
   pointfinder.pushback_DataGraphsGroup_ptr(&clouds_group);
   LatLongGridsGroup latlonggrids_group(
      ndims,passes_group.get_pass_ptr(earthpass_ID));
   EarthRegionsGroup earth_regions_group(
      passes_group.get_pass_ptr(earthpass_ID),
      &clouds_group,&latlonggrids_group,Earth_ptr);

   earth_regions_group.generate_regions(passes_group);
   earths_group.get_OSGgroup_ptr()->addChild(
      earth_regions_group.get_OSGgroup_ptr());

   viewer.getEventHandlerList().push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_ptr));

// Instantiate a MyDatabasePager to handle paging of files from disk:

   viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager(
      clouds_group.get_SetupGeomVisitor_ptr(),
      clouds_group.get_ColorGeodeVisitor_ptr());

// Instantiate signposts, features, army symbol, sphere segments and
// obsfrusta decoration groups:

   vector<postgis_database*> databases;
   databases.push_back(&worldmodel_db);
   databases.push_back(&babygis_db);

   SignPostsGroup* SignPostsGroup_ptr=decorations.add_SignPosts(
      passes_group.get_pass_ptr(earthpass_ID),
      &clock,Earth_ptr->get_Ellipsoid_model_ptr(),databases,&clouds_group);

   for (int n=0; n<decorations.get_n_SignPostsGroups(); n++)
   {
      decorations.get_SignPostsGroup_ptr(n)->set_EarthRegionsGroup_ptr(
         &earth_regions_group);
      decorations.get_SignPostsGroup_ptr(n)->set_Clock_ptr(&clock);
   }

/*
   SignPostsGroup_ptr->set_common_geometrical_size(100);
   
   vector<twovector> signpost_long_lat;
   signpost_long_lat.push_back(twovector(-118.5650902978,33.0206022036));
   // site 2
   signpost_long_lat.push_back(twovector(-118.488025099,32.9150077336)); 
   // site 6
   signpost_long_lat.push_back(twovector(-118.583973101,33.0212484756));
   // site 0
   signpost_long_lat.push_back(twovector(-118.51523921225,32.9084069935));
   // site 12
*/

   decorations.add_Features(
      ndims,passes_group.get_pass_ptr(earthpass_ID),
      &clock,Earth_ptr->get_Ellipsoid_model_ptr());
//   decorations.get_FeaturesGroup_ptr()->set_EarthRegionsGroup_ptr(
//      &earth_regions_group);

   decorations.add_ArmySymbols(
      passes_group.get_pass_ptr(earthpass_ID),&clock,
      Earth_ptr->get_Ellipsoid_model_ptr());

   decorations.add_SphereSegments(
      passes_group.get_pass_ptr(earthpass_ID),
      &clock,Earth_ptr->get_Ellipsoid_model_ptr());
   
   ObsFrustaGroup* ObsFrustaGroup_ptr=NULL;
   bool include_into_Decorations_OSGgroup_flag=false;
   if (videopass_ID >= 0)
   {
      decorations.add_ObsFrusta(
         passes_group.get_pass_ptr(videopass_ID),AnimationController_ptr,
         include_into_Decorations_OSGgroup_flag);
      ObsFrustaGroup_ptr=decorations.get_ObsFrustaGroup_ptr();
//      ObsFrustaGroup_ptr->generate_Copley_ObsFrustum();   
   }

// Add GMTI targets into San Clemente earth_region:

   EarthRegion* GMTI_region_ptr=earth_regions_group.get_EarthRegion_ptr(0);
   GMTI_region_ptr->add_GMTI_target(geopoint(-118.5650902978,33.0206022036));
   // site 2
   GMTI_region_ptr->add_GMTI_target(geopoint(-118.488025099,32.9150077336)); 
   // site 6
   GMTI_region_ptr->add_GMTI_target(geopoint(-118.583973101,33.0212484756));
   // site 0
   GMTI_region_ptr->add_GMTI_target(geopoint(-118.51523921225,32.9084069935));
   // site 12

// Instantiate model decorations group:

   include_into_Decorations_OSGgroup_flag=false;
   decorations.add_Models(
      passes_group.get_pass_ptr(texturepass_ID),AnimationController_ptr,
      include_into_Decorations_OSGgroup_flag);
   ModelsGroup* ModelsGroup_ptr=decorations.get_ModelsGroup_ptr();

   int n_total_frames=720*2;
//   int n_total_frames=720*3;

// Generate LiMIT model, racetrack orbit and TWO ObsFrusta:

   int LiMIT_OSGsubPAT_number;
   Model* LiMIT_ptr=ModelsGroup_ptr->generate_LiMIT_Model(
      LiMIT_OSGsubPAT_number);
   geopoint LiMIT_ellipse_center(true,11,353512.872107285,3648382.98542531);

   ModelsGroup_ptr->generate_elliptical_LiMIT_racetrack_orbit(
      n_total_frames,LiMIT_ellipse_center,LiMIT_ptr);
   ObsFrustum* LiMIT_FOV_ObsFrustum_ptr=
      ModelsGroup_ptr->generate_LiMIT_FOV_ObsFrustrum(
         LiMIT_OSGsubPAT_number,LiMIT_ptr);
   ModelsGroup_ptr->generate_LiMIT_instantaneous_dwell_ObsFrustrum(
      LiMIT_OSGsubPAT_number,LiMIT_ptr,LiMIT_FOV_ObsFrustum_ptr,
      GMTI_region_ptr);

// Translate & rotate LiMIT model, orbit and ObsFrusta from UTM space
// to blue marble surface:

   int region_ID=0;
   threevector LiMIT_UTM_translation(
      LiMIT_ellipse_center.get_UTM_easting(),
      LiMIT_ellipse_center.get_UTM_northing());
   earth_regions_group.UTM_to_surface_transform(
      region_ID,LiMIT_UTM_translation,
      LiMIT_ellipse_center.get_longitude(),
      LiMIT_ellipse_center.get_latitude(),0,
      ModelsGroup_ptr->get_OSGsubPAT_ptr(LiMIT_OSGsubPAT_number));

// Generate predator model, racetrack orbit and ObsFrustum:

   int predator_OSGsubPAT_number;
   Model* predator_ptr=ModelsGroup_ptr->generate_predator_Model(
      predator_OSGsubPAT_number);
   geopoint predator_racetrack_center(-118.5,32.91);	// San Clemente
   ModelsGroup_ptr->generate_predator_racetrack_orbit(
      n_total_frames,predator_racetrack_center.get_longitude(),
      predator_racetrack_center.get_latitude(),predator_ptr);
   ModelsGroup_ptr->generate_predator_ObsFrustrum(
      predator_OSGsubPAT_number,predator_ptr);

// Translate & rotate predator model, orbit and ObsFrusta from UTM
// space to blue marble surface:

   threevector predator_UTM_translation(
      predator_racetrack_center.get_UTM_easting(),
      predator_racetrack_center.get_UTM_northing());
   earth_regions_group.UTM_to_surface_transform(
      region_ID,predator_UTM_translation,
      predator_racetrack_center.get_longitude(),
      predator_racetrack_center.get_latitude(),0,
      ModelsGroup_ptr->get_OSGsubPAT_ptr(predator_OSGsubPAT_number));

// For clarity's sake, initially start with all model OSGsubPATs
// masked off.  Also enlarge the LiMIT and predator models beyond
// their default sizes.

   for (int n=0; n<ModelsGroup_ptr->get_n_OSGsubPATs(); n++)
   {
      ModelsGroup_ptr->set_OSGsubPAT_mask(n,0);      
   }
   ModelsGroup_ptr->change_scale(3);

/*
// Place ObsFrustaGroup containing Copley 2D image onto blue marble
// using same MatrixTransform as for entire Boston point cloud:

   if (ObsFrustaGroup_ptr != NULL)
   {
      earth_regions_group.insert_into_cloud_transform(
         region_ID,ObsFrustaGroup_ptr->get_OSGgroup_ptr());
   }
*/

   if (videopass_ID >= 0)
   {
      
// Read in aircraft filtered position and attitude as functions of
// time for HAFB video pass.  Then generate ObsFrustum with HAFB video
// pass displayed at its base:

      AnimationController* HAFB_AnimationController_ptr=
         new AnimationController();
      HAFB_AnimationController_ptr->setState(AnimationController::PLAY);
      HAFB_AnimationController_ptr->set_nframes(289);
      root->addChild(HAFB_AnimationController_ptr->get_OSGgroup_ptr());

      ModelsGroup* HAFB_ModelsGroup_ptr=decorations.add_Models(
         passes_group.get_pass_ptr(videopass_ID),
         HAFB_AnimationController_ptr);

      double z_rot_angle=-51*PI/180;
      threevector offset(20,210,150);
      threevector first_frame_aircraft_posn(offset);
      HAFB_ModelsGroup_ptr->generate_HAFB_video_pass_model(
         z_rot_angle,first_frame_aircraft_posn,HAFB_AnimationController_ptr);

      region_ID=1;
      double HAFB_center_longitude=44.238491;	// Baghdad
      double HAFB_center_latitude=33.271004;	// Baghdad
      threevector HAFB_UTM_translation(0,0,0);
      earth_regions_group.UTM_to_surface_transform(
         region_ID,HAFB_UTM_translation,
         HAFB_center_longitude,HAFB_center_latitude,0,
         HAFB_ModelsGroup_ptr->get_OSGsubPAT_ptr(1));
   }

// Attach scene graph to viewer:

   root->addChild(decorations.get_OSGgroup_ptr());
   viewer.setSceneData(root);

// Create the windows and run the threads:

   viewer.realize();
   osgUtil::SceneView* SceneView_ptr=viewer.getSceneHandlerList().front()->
      getSceneView();
   CM_3D_ptr->set_SceneView_ptr(SceneView_ptr);
   Earth_ptr->set_SceneView_ptr(SceneView_ptr);


// Enable user to place decorations within cities sitting on top of
// blue marble:

   decorations.set_DataNode_ptr(
      planets_group.get_EarthSpinTransform_ptr());

// Enable user to place decorations on blue marble surface:

//   decorations.set_DataNode_ptr(
//      planets_group.get_PlanetSpinTransform_ptr());

// Add an animation path creator to the event handler list AFTER the
// viewer has been realized:

   AnimationPathCreator* animation_path_handler = 
      new AnimationPathCreator(&viewer);
   viewer.getEventHandlerList().push_back(animation_path_handler);

   viewer.getUsage(*arguments.getApplicationUsage());

   while (!viewer.done())
   {
      viewer.sync();
      viewer.update();
      viewer.frame();
   }

   viewer.sync();
}

