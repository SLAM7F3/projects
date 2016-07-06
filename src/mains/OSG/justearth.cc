// ========================================================================
// Program JUSTEARTH is a playground for working with a draped
// ellipsoid representation of the earth which is endowed with an ECI
// coordinate system.  

//		justearth 

//		justearth --GIS_layer ./packages/world_GIS.pkg

// ========================================================================
// Last updated on 11/27/07; 10/1/08; 3/9/09
// ========================================================================

#include <string>
#include <vector>
#include <osg/Light>
#include <osg/LightSource>
#include <osg/LightModel>
#include <osgGA/NodeTrackerManipulator>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>

#include "astro_geo/Clock.h"
#include "color/colorfuncs.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgSceneGraph/DepthPartitionNode.h"
#include "osg/osgEarth/EarthsGroup.h"
#include "osg/osgEarth/EarthKeyHandler.h"
#include "osg/osgEarth/EarthManipulator.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "osg/osgGrid/EarthGrid.h"
#include "astro_geo/Ellipsoid_model.h"
#include "osg/osgGrid/GridKeyHandler.h"
#include "osg/ModeController.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osgSpace/PlanetsGroup.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGIS/postgis_database.h"
#include "passes/TextDialogBox.h"
#include "osg/osgWindow/ViewerManager.h"
#include "general/outputfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

// ========================================================================
int main( int argc, char** argv )
{

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

   vector<int> GISlayer_IDs=passes_group.get_GISlayer_IDs();

// Instantiate database object to send data to and retrieve data from
// an external PostGIS database:

   postgis_database* postgis_db_ptr=NULL;

   if (GISlayer_IDs.size() > 0)
   {
      string hostname=passes_group.get_pass_ptr(GISlayer_IDs[0])->
         get_PassInfo_ptr()->get_PostGIS_hostname();
      string database_name=passes_group.get_pass_ptr(GISlayer_IDs[0])->
         get_PassInfo_ptr()->get_PostGIS_database_name();
      string username=passes_group.get_pass_ptr(GISlayer_IDs[0])->
         get_PassInfo_ptr()->get_PostGIS_username();
      postgis_db_ptr=new postgis_database(hostname,database_name,username);
   
      const double xmin = -181;	// degs
      const double xmax = 181;	// degs
      const double ymin = -80;	// degs
      const double ymax = 80;	// degs
      postgis_db_ptr->pushback_gis_bbox(xmin,xmax,ymin,ymax);
   }

// Instantiate clock to keep track of real time:

   Clock clock;
   clock.current_local_time_and_UTC();

/*
   int year=2006;
   int month=8;
//   int day=9;
   int day=24;
//   int local_hour=12;

//   cout << "Enter local hour:" << endl;
//   cin >> local_hour;

   int UTC_hour=11;
//   int UTC_hour=20;
   cout << "Enter UTC hour:" << endl;
   cin >> UTC_hour;
   int minutes=0;
   double secs=0;

//   clock.set_UTM_zone_time_offset(19);	// Boston
//   clock.set_daylight_savings_flag(true);
//   clock.set_local_time(year,month,day,local_hour,minutes,secs);
   clock.set_UTC_time(year,month,day,UTC_hour,minutes,secs);

*/
// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("3D Earth");
//   window_mgr_ptr->getUsage(*arguments.getApplicationUsage());

// Create a DepthPartitionNode to manage partitioning of the scene

   DepthPartitionNode* root = new DepthPartitionNode;
   root->setActive(true);     // Control whether the node analyzes the scene
//   cout << "max_depth = " << root->getMaxTraversalDepth() << endl;

// Instantiate Operations object to handle mode, animation and image
// number control:

   Operations operations(ndims,window_mgr_ptr);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   root->addChild(operations.get_OSGgroup_ptr());

// Instantiate "holodeck" Earth grid:

   EarthGrid* earthgrid_ptr = new EarthGrid(colorfunc::grey);
   threevector* earthgrid_origin_ptr=earthgrid_ptr->get_world_origin_ptr();
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new GridKeyHandler(ModeController_ptr,earthgrid_ptr));
//   root->addChild(earthgrid_ptr->get_geode_ptr());

// Instantiate solar system group to hold sun and planets:

   PlanetsGroup planets_group(
      passes_group.get_pass_ptr(earthpass_ID),&clock,earthgrid_origin_ptr);
   root->addChild(planets_group.get_EarthSpinTransform_ptr());

// Generate the earth as well the solar system:

//   DataGraph* EarthGraph_ptr=
      planets_group.generate_EarthGraph();
   osg::Group* solarsystem_ptr=planets_group.generate_solarsystem(
      earthgrid_ptr);
   root->addChild(solarsystem_ptr);

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
      vector<string> GISlines_tablenames=
         passes_group.get_pass_ptr(GISlayer_IDs[0])->
         get_PassInfo_ptr()->get_gislines_tablenames();

      cout << "Before call to retrieve_borders_from_PostGIS_database()"
           << endl;
      cout << "GISlines_tablenames[0] = "
           << GISlines_tablenames[0] << endl;
      Earth_ptr->retrieve_borders_from_PostGIS_database(
         GISlines_tablenames[0]);

      cout << "Before call to retrieve_cities_from_PostGIS_database()" 
           << endl;
      Earth_ptr->retrieve_cities_from_PostGIS_database();
   }
   
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

   Decorations decorations(window_mgr_ptr,ModeController_ptr,CM_3D_ptr,
                           earthgrid_origin_ptr);

// Generate random background star field:

   root->addChild(planets_group.generate_starfield(earthgrid_ptr));

// Instantiate groups to hold multiple point clouds,
// latitude-longitude grids and associated earth regions:

   EarthRegionsGroup earth_regions_group(
      passes_group.get_pass_ptr(earthpass_ID),Earth_ptr);

// Instantiate a MyDatabasePager to handle paging of files from disk:

//   viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager();

// Instantiate group to hold signpost information:

//   SignPostsGroup* SignPostsGroup_ptr=
      decorations.add_SignPosts(
         passes_group.get_pass_ptr(earthpass_ID),
         &clock,Earth_ptr->get_Ellipsoid_model_ptr());

/*
   vector<twovector> signpost_long_lat;
   signpost_long_lat.push_back(twovector(-118.385100,33.185000));

   for (int s=0; s<signpost_long_lat.size(); s++)
   {
      twovector long_lat(signpost_long_lat[s]);
      SignPost* curr_SignPost_ptr=
         SignPostsGroup_ptr->generate_new_SignPost_on_earth(
            long_lat.get(0),long_lat.get(1),0);
      SignPostsGroup_ptr->generate_signpost_geode(curr_SignPost_ptr);
   }
*/

   decorations.add_Features(
      ndims,passes_group.get_pass_ptr(earthpass_ID),
      &clock,Earth_ptr->get_Ellipsoid_model_ptr());
   decorations.get_FeaturesGroup_ptr()->set_EarthRegionsGroup_ptr(
      &earth_regions_group);

/*
// Instantiate cylinders decoration group:

   decorations.add_Cylinders(
      passes_group.get_pass_ptr(earthpass_ID),&clock,Earth_ptr->
      get_Ellipsoid_model_ptr());
   decorations.get_CylindersGroup_ptr()->set_rh(10000,200);
*/

   decorations.add_ArmySymbols(
      passes_group.get_pass_ptr(earthpass_ID),&clock,
      Earth_ptr->get_Ellipsoid_model_ptr());

   decorations.add_SphereSegments(
      passes_group.get_pass_ptr(earthpass_ID),
      &clock,Earth_ptr->get_Ellipsoid_model_ptr());

// Attach scene graph to viewer:

   root->addChild(decorations.get_OSGgroup_ptr());
   window_mgr_ptr->setSceneData(root);

// Create the windows and run the threads.  Viewer's realize method
// calls the CUstomManipulator's home() method:

   window_mgr_ptr->realize();

   decorations.set_DataNode_ptr(
      planets_group.get_PlanetSpinTransform_ptr());

   while (!window_mgr_ptr->done())
   {
      window_mgr_ptr->process();
   }

   postgis_db_ptr->disconnect();

   delete earthgrid_ptr;
   delete window_mgr_ptr;
}

