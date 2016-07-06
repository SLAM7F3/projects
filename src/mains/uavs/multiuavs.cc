// ========================================================================
// Program MULTIUAVS implements a simple real-time simulation for up
// to 4 UAVs.  Time and distance scales hardwired into this simulation
// are reasonable for predator UAVs.  

//     	multiuavs --surface_texture ./packages/sanclemente_EO.pkg

//     	multiuavs --surface_texture ./packages/baghdad.pkg

// ========================================================================
// Last updated on 2/14/08; 2/18/08; 2/19/08; 2/21/08
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgText/Text>

#include "osg/osgGraphicals/AnimationController.h"
#include "astro_geo/latlong2utmfuncs.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "math/constant_vectors.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "osg/osgGrid/GridKeyHandler.h"
#include "osg/osgGrid/LatLongGridsGroup.h"
#include "messenger/Messenger.h"
#include "osg/ModeController.h"
#include "numrec/nrfuncs.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgGeometry/PyramidsGroup.h"
#include "robots/robots_group.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgWindow/ViewerManager.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ifstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

//   nrfunc::init_time_based_seed();

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);

// Read input ladar point cloud file:

   const int ndims=3;
   PassesGroup passes_group(&arguments);
   int texturepass_ID=passes_group.get_curr_texturepass_ID();

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("3D imagery");
//   window_mgr_ptr->getUsage(*arguments.getApplicationUsage());

// Instantiate UAV messenger:

//   string broker_URL="tcp://127.0.0.1:61616";
//   string broker_URL="tcp://155.34.162.148:61616";
//   string broker_URL="tcp://155.34.162.230:61616";
//   string broker_URL="tcp://155.34.125.216:61616";	// family day
//   string broker_URL="tcp://155.34.135.239:61616";	// G104 conf room

   int pass_ID=passes_group.get_n_passes()-1;
   string broker_URL=passes_group.get_pass_ptr(pass_ID)->
      get_PassInfo_ptr()->get_ActiveMQ_hostname();
   cout << "ActiveMQ broker_URL = " << broker_URL << endl;

   string UAV_message_queue_channel_name="UAV";
   Messenger UAV_messenger( 
      broker_URL, UAV_message_queue_channel_name );

// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

   bool display_movie_state=true;
   bool display_movie_number=true;
   bool display_movie_world_time=true;
   Operations operations(
      ndims,window_mgr_ptr,passes_group,display_movie_state,
      display_movie_number,display_movie_world_time);
   operations.get_ImageNumberHUD_ptr()->set_text_color(colorfunc::cream);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   root->addChild(operations.get_OSGgroup_ptr());

// Add a custom manipulator to the event handler list:

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);

   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate groups to hold multiple surface textures,
// latitude-longitude grids and associated earth regions:

   LatLongGridsGroup latlonggrids_group(
      ndims,passes_group.get_pass_ptr(texturepass_ID),CM_3D_ptr);
   root->addChild(latlonggrids_group.get_OSGgroup_ptr());

   EarthRegionsGroup earth_regions_group(
      passes_group.get_pass_ptr(texturepass_ID),&latlonggrids_group);
   root->addChild(earth_regions_group.get_OSGgroup_ptr());

   earth_regions_group.generate_regions(passes_group);

// Display latlonggrid underneath a surface texture only for algorithm
// development purposes:

   LatLongGrid* latlonggrid_ptr=latlonggrids_group.get_Grid_ptr(0);
   threevector* grid_origin_ptr=latlonggrid_ptr->get_world_origin_ptr();
   CM_3D_ptr->set_Grid_ptr(latlonggrid_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new GridKeyHandler(ModeController_ptr,latlonggrid_ptr));

// Instantiate group to hold all decorations:

   Decorations decorations(window_mgr_ptr,ModeController_ptr,CM_3D_ptr,
                           grid_origin_ptr);

// Instantiate a PointFinder;

   PointFinder pointfinder;
   CM_3D_ptr->set_PointFinder(&pointfinder);

// Instantiate Polygons & PolyLines decoration groups:

   osgGeometry::PolygonsGroup* PolygonsGroup_ptr=decorations.add_Polygons(
      ndims,passes_group.get_pass_ptr(texturepass_ID),
      AnimationController_ptr);
   PolyLinesGroup* PolyLinesGroup_ptr=decorations.add_PolyLines(
      ndims,passes_group.get_pass_ptr(texturepass_ID));

   PolyLinesGroup_ptr->set_width(3);
   PolyLinePickHandler* PolyLinePickHandler_ptr=
      decorations.get_PolyLinePickHandler_ptr();
   PolyLinePickHandler_ptr->set_min_doubleclick_time_spread(0.05);
   PolyLinePickHandler_ptr->set_max_doubleclick_time_spread(0.2);

   PolyLinePickHandler_ptr->set_form_polygon_flag(true);
   PolyLinePickHandler_ptr->set_Polygon_color(colorfunc::red);
//   PolyLinePickHandler_ptr->set_Polygon_color(colorfunc::green);

// Hardwired KOZ Polygons:

   vector<threevector> vertex;
   double KOZ_distance=5000;
   vertex.push_back(threevector(0,0));
   vertex.push_back(threevector(KOZ_distance,0));
   vertex.push_back(threevector(KOZ_distance,KOZ_distance));
   vertex.push_back(threevector(0,KOZ_distance));
   polygon koz_poly1(vertex);

   threevector reference_origin(430000 , 3676000);
//    osgGeometry::Polygon* KOZ_Poly1_ptr=
      PolygonsGroup_ptr->generate_new_Polygon(
         reference_origin,koz_poly1);

   reference_origin=threevector(450910 , 3687086);
//   osgGeometry::Polygon* KOZ_Poly2_ptr=
      PolygonsGroup_ptr->generate_new_Polygon(
         reference_origin,koz_poly1);

// Instantiate boxes decoration group:

   decorations.add_Boxes(
      passes_group.get_pass_ptr(texturepass_ID));
   decorations.get_BoxesGroup_ptr()->set_wlh(20,30,40);
//   decorations.get_BoxesGroup_ptr()->set_wlh(2,2,2);
   root->addChild(decorations.get_BoxesGroup_ptr()->
                  createBoxLight(threevector(20,10,10)));

// Instantiate cones decoration group:

   decorations.add_Cones(passes_group.get_pass_ptr(texturepass_ID));
   root->addChild(decorations.get_ConesGroup_ptr()->
                  createConeLight(threevector(20,10,10)));

// Instantiate cylinders decoration group:

   CylindersGroup* CylindersGroup_ptr=
      decorations.add_Cylinders(passes_group.get_pass_ptr(texturepass_ID),
                                AnimationController_ptr);
//   decorations.get_CylindersGroup_ptr()->set_rh(30000,0.1);
   decorations.get_CylindersGroup_ptr()->set_rh(300,0.1);
   root->addChild(decorations.get_CylindersGroup_ptr()->
                  createCylinderLight(threevector(20,10,10)));

// Instantiate pyramids decoration group:

   decorations.add_Pyramids(passes_group.get_pass_ptr(texturepass_ID));

// Set world time:

   Clock* clock_ptr=AnimationController_ptr->get_clock_ptr();
   clock_ptr->current_local_time_and_UTC();
   double secs_since_epoch=clock_ptr->secs_elapsed_since_reference_date(
      clock_ptr->get_year(),clock_ptr->get_month(),clock_ptr->get_day());
 
// Set up world clock:

//   int n_timesteps=100;
//   int n_timesteps=1000;
//   int n_timesteps=3000;
//   int n_timesteps=5000;
   int n_timesteps=10000;	// genuine as of Mon at 7:40 am
//   int n_timesteps=50000;	
   AnimationController_ptr->set_nframes(n_timesteps);
   vector<twovector> time_counter_samples;

   const double start_time=secs_since_epoch+0;		// secs
   const double stop_time=secs_since_epoch+24 * 3600;	// secs
   time_counter_samples.push_back(twovector(start_time , 0));
   time_counter_samples.push_back(twovector(stop_time , n_timesteps));
   AnimationController_ptr->correlate_frames_to_world_times(
      time_counter_samples);

// Instantiate group to hold robots.  Then instantiate individual robots:

   robots_group robotsgroup(CylindersGroup_ptr,AnimationController_ptr);
   robotsgroup.set_Messenger_ptr(&UAV_messenger);
   robotsgroup.get_groundspace().set_KOZ_PolygonsGroup_ptr(
      PolygonsGroup_ptr);

// Set earth region parameters for robots' basic coverage region:

   bool northern_hemisphere_flag;
   int UTM_zonenumber;
   double UTM_northing,UTM_easting;
   latlongfunc::LLtoUTM(
      latlonggrid_ptr->get_latitude_middle(),
      latlonggrid_ptr->get_longitude_middle(),
      UTM_zonenumber,northern_hemisphere_flag,
      UTM_northing,UTM_easting);

   threevector groundspace_origin=latlonggrid_ptr->get_world_middle();
   robotsgroup.get_groundspace().set_origin(groundspace_origin);
   robotsgroup.get_groundspace().set_UTM_zonenumber(UTM_zonenumber);
   robotsgroup.get_groundspace().set_northern_hemisphere_flag(
      northern_hemisphere_flag);

   const double genuine_max_predator_range=731000;	// meters
   const double max_predator_range=0.025*genuine_max_predator_range;	
   robotsgroup.get_groundspace().set_max_robot_dist_from_origin(
      max_predator_range);

// Draw circle representing maximum operating radius of 4 UAVs from
// ground space origin:

   int n_vertices=72;
   double dtheta=2*PI/n_vertices;
   vector<threevector> vertices;
   for (int n=0; n<=n_vertices; n++)
   {
      double curr_theta=n*dtheta;
      threevector curr_vertex(cos(curr_theta),sin(curr_theta));
      curr_vertex *= max_predator_range;
      curr_vertex += groundspace_origin;
      vertices.push_back(curr_vertex);
   }
   
   osg::Vec4 circle_color=colorfunc::get_OSG_color(colorfunc::white);
//   PolyLine* operating_region_PolyLine_ptr=
      PolyLinesGroup_ptr->generate_new_PolyLine(vertices,circle_color);

// Instantiate 4 robots:

//   int n_UAVS=1;
   int n_UAVS=4;
   const double predator_cruising_speed=37.6;	// meters/sec
   const double UAV_altitude=100;		// meters

   vector<double> theta;
   for (int n=0; n<n_UAVS; n++)
   {
      double x=0.75*max_predator_range*2*(nrfunc::ran1()-0.5);
      double y=0.75*max_predator_range*2*(nrfunc::ran1()-0.5);
      threevector p(x,y,UAV_altitude);
      p += groundspace_origin;

      double theta=2*PI*nrfunc::ran1();
      threevector v(cos(theta),sin(theta),0);
      v *= predator_cruising_speed;
      
      robot* curr_robot_ptr=robotsgroup.generate_new_robot();
      curr_robot_ptr->get_statevector().set_position(p);
      curr_robot_ptr->get_statevector().set_velocity(v);
   }

// Optimize the scene graph, remove redundent nodes and states, and
// then attach it to the viewer:

//   osgUtil::Optimizer optimizer;
//   optimizer.optimize(root);
   root->addChild(decorations.get_OSGgroup_ptr());

// Attach scene graph to the viewer:

   window_mgr_ptr->setSceneData(root);

// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_ptr->realize();

   while( !window_mgr_ptr->done() )
   {
      robotsgroup.propagate_all_statevectors();
      window_mgr_ptr->process();
   }
}

