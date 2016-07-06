// ========================================================================
// Program EARTH is a playground for working with a draped ellipsoid
// representation of the earth which is endowed with an ECI coordinate
// system.  Pointclouds (of Lowell and Boston) can be mapped onto the
// ellipsoid's surface.


//  earth --GIS_layer world_GIS.pkg --region_filename ./packages/baghdad.pkg 


//  earth --GIS_layer world_GIS.pkg --region_filename boston.pkg --region_filename /home/cho/programs/c++/svn/projects/src/mains/newyork/nyc_rtv.pkg --region_filename baghdad.pkg --newpass /home/cho/programs/c++/svn/projects/src/mains/OSG/HAFB_all.vid --surface_texture sanclemente_EO.pkg


//  earth --GIS_layer world_GIS.pkg --region_filename boston.pkg --region_filename /home/cho/programs/c++/svn/projects/src/mains/newyork/nyc_rtv.pkg --region_filename baghdad.pkg --newpass /home/cho/programs/c++/svn/projects/src/mains/OSG/HAFB_all.vid --surface_texture sanclemente_EO.pkg --GIS_layer SKS_GIS.pkg

// ========================================================================
// Last updated on 3/20/09; 5/11/10; 5/12/10
// ========================================================================

#include <string>
#include <vector>
#include <osg/Light>
#include <osg/LightSource>
#include <osg/LightModel>
#include <osgGA/NodeTrackerManipulator>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>

#include "osg/osgGraphicals/AnimationController.h"
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
#include "osg/osgGrid/GridKeyHandler.h"
#include "osg/osgGrid/LatLongGridsGroup.h"
#include "osg/ModeController.h"
#include "osg/osgModels/ModelsGroup.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgModels/ObsFrustaGroup.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osgSpace/PlanetsGroup.h"
#include "osg/osgSpace/PlanetKeyHandler.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "passes/TextDialogBox.h"
#include "osg/osgWindow/ViewerManager.h"

#include "general/outputfuncs.h"
#include "templates/mytemplates.h"

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
   const int ndims=3;
   string OSG_data_dir(getenv("OSG_FILE_PATH"));
   OSG_data_dir += "/";

   PassesGroup passes_group(&arguments);
   string solarsystem_dir=OSG_data_dir+"SolarSystem/";
   string earth_filename=solarsystem_dir+"earth_bright60.osga";
   int earthpass_ID=passes_group.generate_new_pass(
      earth_filename,Pass::earth,Pass::osga);

   int texturepass_ID=passes_group.get_curr_texturepass_ID();
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
   int videopass_ID=passes_group.get_videopass_ID();
   vector<int> GISlayer_IDs=passes_group.get_GISlayer_IDs();

   cout << "earthpass_ID = " << earthpass_ID << endl;
   cout << "cloudpass_ID = " << cloudpass_ID << endl;
   cout << "videopass_ID = " << videopass_ID << endl;
   cout << "texturepass_ID = " << texturepass_ID << endl;
   cout << "GISlayer_IDs = " << endl;
   templatefunc::printVector(GISlayer_IDs);

// Instantiate clock to keep track of real time:

   Clock clock;
   clock.current_local_time_and_UTC();

/*
// Used year=2006, month=8, day=24, UTC_hour=11, minutes=secs=0 to
// generate fused Baghdad ladar/sat EO movie on 9/6/07.  Should have
// picked a different UTC_hour so that western hemisphere & Africa are
// clearly on night-side of terminator while Iraq is on day-side of
// terminator...

   int year=2006;
   int month=8;
   int day=24;
//  int local_hour=12;
//   cout << "Enter local hour:" << endl;
//   cin >> local_hour;
   int UTC_hour=11;
//   int UTC_hour=17;
//   cout << "Enter UTC hour:" << endl;
//   cin >> UTC_hour;
   int minutes=0;
   double secs=0;

//   clock.set_UTM_zone_time_offset(19);	// Boston
//   clock.set_daylight_savings_flag(true);
//   clock.set_local_time(year,month,day,local_hour,minutes,secs);
   clock.set_UTC_time(year,month,day,UTC_hour,minutes,secs);
*/

// Instantiate postgis database objects to send data to and retrieve
// data from external Postgres database:

   postgis_databases_group* postgis_databases_group_ptr=
      new postgis_databases_group;
   postgis_database* postgis_db_ptr=postgis_databases_group_ptr->
      generate_postgis_database_from_GISlayer_IDs(passes_group,GISlayer_IDs);

   const double xmin=417197.107472;
   const double xmax=451175.746614;
   const double ymin=3676472.7687;
   const double ymax=3689774.9008;
//   const double xmin = -181;	// degs
//   const double xmax = 181;	// degs
//   const double ymin = -80;	// degs
//   const double ymax = 80;	// degs
   if (postgis_db_ptr != NULL)
      postgis_db_ptr->pushback_gis_bbox(xmin,xmax,ymin,ymax);
   
// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("3D Earth");

// Create a DepthPartitionNode to manage partitioning of the scene

   DepthPartitionNode* root = new DepthPartitionNode;
   root->setActive(true);     // Control whether the node analyzes the scene
//   cout << "max_depth = " << root->getMaxTraversalDepth() << endl;

// Instantiate Operations object to handle mode, animation and image
// number control:

   Operations operations(ndims,window_mgr_ptr,passes_group);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   root->addChild(operations.get_OSGgroup_ptr());

// Instantiate Earth grid:

   EarthGrid* earthgrid_ptr = new EarthGrid(colorfunc::grey);
   threevector* earthgrid_origin_ptr=earthgrid_ptr->get_world_origin_ptr();
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
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
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new PlanetKeyHandler(&planets_group,ModeController_ptr));

// Instantiate a PointFinder;

   PointFinder pointfinder(&planets_group);

// Instantiate an EarthsGroup to hold "blue marble" coordinate system
// information:

   EarthsGroup earths_group(
      passes_group.get_pass_ptr(earthpass_ID),&clock,earthgrid_origin_ptr);
   planets_group.get_EarthSpinTransform_ptr()->
      addChild(earths_group.get_OSGgroup_ptr());
   Earth* Earth_ptr=earths_group.generate_new_Earth(postgis_db_ptr);

   if (GISlayer_IDs.size() > 0)
   {
/*
      vector<string> GISpoints_tablenames=
         passes_group.get_pass_ptr(GISlayer_IDs[0])->
         get_PassInfo_ptr()->get_gispoints_tablenames();
      for (int t=0; t<GISpoints_tablenames.size(); t++)
      {
         cout << "t = " << t
              << " gispoints_tablenames = " 
              << GISpoints_tablenames[t] << endl;
         postgis_db_ptr->pushback_GISpoint_tablename(GISpoints_tablenames[t]);
      }
*/
    
      vector<string> GISlines_tablenames=
         passes_group.get_pass_ptr(GISlayer_IDs[0])->
         get_PassInfo_ptr()->get_gislines_tablenames();
      Earth_ptr->retrieve_borders_from_PostGIS_database(
         GISlines_tablenames[0]);
      Earth_ptr->retrieve_cities_from_PostGIS_database();
   } // GISlayer_IDs.size() > 0 conditional
   
// Add a custom manipulator to the event handler list:

   osgGA::EarthManipulator* CM_3D_ptr=new osgGA::EarthManipulator(
      ModeController_ptr,Earth_ptr->get_Ellipsoid_model_ptr(),&clock,
      window_mgr_ptr);
   CM_3D_ptr->set_PointFinder(&pointfinder);
   Earth_ptr->set_EarthManipulator_ptr(CM_3D_ptr);

   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new EarthKeyHandler(Earth_ptr,CM_3D_ptr,ModeController_ptr));

// Instantiate group to hold all decorations:

   Decorations decorations(
      window_mgr_ptr,ModeController_ptr,CM_3D_ptr,earthgrid_origin_ptr);

// Generate random background star field:

   root->addChild(planets_group.generate_starfield(earthgrid_ptr));

// Instantiate groups to hold multiple point clouds,
// latitude-longitude grids and associated earth regions:

   PointCloudsGroup clouds_group(passes_group.get_pass_ptr(cloudpass_ID));
   pointfinder.pushback_DataGraphsGroup_ptr(&clouds_group);
   LatLongGridsGroup latlonggrids_group(
      ndims,passes_group.get_pass_ptr(earthpass_ID),CM_3D_ptr);
   EarthRegionsGroup earth_regions_group(
      passes_group.get_pass_ptr(earthpass_ID),
      &clouds_group,&latlonggrids_group,Earth_ptr);

   earth_regions_group.generate_regions(passes_group);
   earths_group.get_OSGgroup_ptr()->addChild(
      earth_regions_group.get_OSGgroup_ptr());

   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_ptr));

// Instantiate a MyDatabasePager to handle paging of files from disk:

   viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager(
      clouds_group.get_SetupGeomVisitor_ptr(),
      clouds_group.get_ColorGeodeVisitor_ptr());

// Instantiate signposts, features, army symbol, sphere segments and
// obsfrusta decoration groups:

   SignPostsGroup* SignPostsGroup_ptr=decorations.add_SignPosts(
      passes_group.get_pass_ptr(earthpass_ID),
      &clock,Earth_ptr->get_Ellipsoid_model_ptr(),NULL,
      &clouds_group);
   SignPostsGroup_ptr->set_postgis_databases_group_ptr(
      postgis_databases_group_ptr);

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
      &clock,Earth_ptr->get_Ellipsoid_model_ptr(),100,
      0,2*PI,0,PI/2,false,true);
   
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
      ModelsGroup_ptr->set_OSGsubPAT_nodemask(n,0);      
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

// When generating movies with the Cessna flying around, we must
// increase the HAFB Animation Controller's delay factor by the same
// stretch factor as that which we apply to the animation path clock
// using main program mains/utility/SLOWPATH.  For Baghdad movie
// generation purposes, we took this stretch factor to equal 15:

//      HAFB_AnimationController_ptr->setDelay(
//         15*HAFB_AnimationController_ptr->getDelay());
//      cout << "HAFB_AnimationController.Delay = "
//           << HAFB_AnimationController_ptr->getDelay() << endl;
//      outputfunc::enter_continue_char();

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
   window_mgr_ptr->setSceneData(root);

// Create the windows and run the threads.  Viewer's realize method
// calls the CUstomManipulator's home() method:

   window_mgr_ptr->realize();

// Enable user to place decorations within cities sitting on top of
// blue marble:

   decorations.set_DataNode_ptr(
      planets_group.get_EarthSpinTransform_ptr());

// Enable user to place decorations on blue marble surface:

//   decorations.set_DataNode_ptr(
//      planets_group.get_PlanetSpinTransform_ptr());

   while (!window_mgr_ptr->done())
   {
      window_mgr_ptr->process();
   }
   delete window_mgr_ptr;

   delete postgis_databases_group_ptr;
}

