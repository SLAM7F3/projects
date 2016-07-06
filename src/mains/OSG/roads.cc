// ========================================================================
// Program ROADS is a variant of program VIEWCITIES which is a lab for
// incorporating MASSGIS roads into the Boston ladar set.

//   roads /data/alirt2/Boston_2005/boston_30cm_tiles/fusion_osga/*.osga

// ========================================================================
// Last updated on 1/19/07; 1/23/07; 2/14/07; 2/22/07; 8/20/07
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgUtil/SceneView>
#include <osgProducer/Viewer>
#include <osgDB/WriteFile>

#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGraphicals/AnimationKeyHandler.h"
#include "osg/AnimationPathCreator.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CentersKeyHandler.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "astro_geo/Clock.h"
#include "color/colorfuncs.h"
#include "osg/osgSceneGraph/ColorMap.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "general/filefuncs.h"
#include "astro_geo/geofuncs.h"
#include "osg/osgGrid/GridKeyHandler.h"
#include "astro_geo/latlong2utmfuncs.h"
#include "osg/osgGrid/LatLongGridsGroup.h"
#include "osg/ModeController.h"
#include "osg/ModeKeyHandler.h"
#include "osg/osgModels/ModelsGroup.h"
#include "osg/osg2D/Moviefuncs.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgAnnotators/ObsFrustaGroup.h"
#include "osg/osgfuncs.h"
#include "passes/PassesGroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "geometry/polyline.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgGIS/postgis_database.h"
#include "passes/TextDialogBox.h"
#include "time/timefuncs.h"

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

// Instantiate clock to keep track of real time:

   Clock clock;
   clock.current_local_time_and_UTC();

// Instantiate database object to send data to and retrieve data from
// an external Postgres database:

   string hostname="localhost";
//   string hostname="fusion1";
// string hostname="sks";
//   string database_name="world_model";
//   string database_name="isds_clone";
   string database_name="isdsid_dev";
//   string username="cho";
   string username="sks";
   postgis_database worldmodel_db(hostname,database_name,username);

   hostname="fusion1";
   database_name="babygis";
   username="cho";
   postgis_database babygis_db(hostname,database_name,username);

   string TableName="eotmajroads";
   babygis_db.read_table(TableName);

// Initialize constants and parameters read in from command line as
// well as ascii text file:

   const int ndims=3;
   string colormap_dir(getenv("OSG_FILE_PATH"));
   colormap_dir += "/3D_colormaps/";
//   int p_map=2;		// small_hue_value
//   int p_map=7;		// wrap1
//   int p_map=8;		// wrap2
   int p_map=9;		// wrap3
//   int p_map=10;		// wrap4
//   int p_map=11;		// wrap8
//   int p_map=12;		// wrap16
   ColorMap colormap(colormap_dir,p_map);

// Read input ladar point cloud file:
   
   PassesGroup passes_group(&arguments);
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();

// Construct the viewer and set it up with sensible event handlers:

   osgProducer::Viewer viewer(arguments);
   viewer.setClearColor(osg::Vec4(0,0,0,0));
   viewer.setUpViewer(osgProducer::Viewer::ESCAPE_SETS_DONE);
//   viewer.setUpViewer(osgProducer::Viewer::SKY_LIGHT_SOURCE |
//                      osgProducer::Viewer::ESCAPE_SETS_DONE);
//   viewer.setUpViewer(osgProducer::Viewer::HEAD_LIGHT_SOURCE |
//                      osgProducer::Viewer::ESCAPE_SETS_DONE);

// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate a mode controller and key event handler:

   ModeController* ModeController_ptr=new ModeController();
   viewer.getEventHandlerList().push_back( 
      new ModeKeyHandler(ModeController_ptr) );
   root->addChild(osgfunc::create_Mode_HUD(ndims,ModeController_ptr));

// Instantiate animation controller & key handler:

   int n_animation_frames=360;
   AnimationController* AnimationController_ptr=new AnimationController(
      n_animation_frames);
   root->addChild(AnimationController_ptr->get_OSGgroup_ptr());
   
   AnimationKeyHandler* AnimationKeyHandler_ptr=
      new AnimationKeyHandler(ModeController_ptr,AnimationController_ptr);
   viewer.getEventHandlerList().push_back( AnimationKeyHandler_ptr);
   bool display_movie_state=false;
//   bool display_movie_state=true
   bool display_movie_number=false;
//   bool display_movie_number=true;
   root->addChild(Moviefunc::create_Imagenumber_HUD(
      AnimationController_ptr,display_movie_state,display_movie_number));

// Add a custom manipulator to the event handler list:

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,&window_mgr);
   window_mgr.set_CameraManipulator(CM_3D_ptr);

// Instantiate groups to hold multiple point clouds,
// latitude-longitude grids and associated earth regions:

   PointCloudsGroup clouds_group(
      &arguments,passes_group.get_pass_ptr(cloudpass_ID),&colormap);
   LatLongGridsGroup latlonggrids_group(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),CM_3D_ptr);
   EarthRegionsGroup earth_regions_group(
      passes_group.get_pass_ptr(cloudpass_ID),      
      &clouds_group,&latlonggrids_group);
   earth_regions_group.generate_regions(passes_group);

   LatLongGrid* latlonggrid_ptr=latlonggrids_group.get_Grid_ptr(0);
   threevector* grid_origin_ptr=latlonggrid_ptr->get_world_origin_ptr();

   viewer.getEventHandlerList().push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_ptr));
   viewer.getEventHandlerList().push_back(
      new GridKeyHandler(ModeController_ptr,latlonggrid_ptr));

// Instantiate WindowCoordConverter:

   ViewerManager window_mgr;
   window_mgr.set_Viewer_ptr(&viewer);
   window_mgr.initialize_window("3D imagery");

// Instantiate a PointFinder;

   PointFinder pointfinder(&clouds_group);
   CM_3D_ptr->set_PointFinder(&pointfinder);

// Instantiate group to hold all decorations:

   Decorations decorations(
      &window_mgr,ModeController_ptr,CM_3D_ptr,
      grid_origin_ptr);

// Instantiate a MyDatabasePager to handle paging of files from disk:

   viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager(
      clouds_group.get_SetupGeomVisitor_ptr(),
      clouds_group.get_ColorGeodeVisitor_ptr());

// Instantiate signposts, features, army symbol and sphere segments
// decoration groups:

   vector<postgis_database*> databases;
   databases.push_back(&worldmodel_db);
   databases.push_back(&babygis_db);

   decorations.add_SignPosts(
      passes_group.get_pass_ptr(cloudpass_ID),databases);
   for (int n=0; n<decorations.get_n_SignPostsGroups(); n++)
   {
      decorations.get_SignPostsGroup_ptr(n)->set_EarthRegionsGroup_ptr(
         &earth_regions_group);
      decorations.get_SignPostsGroup_ptr(n)->set_Clock_ptr(&clock);
   }

   decorations.add_Features(ndims,passes_group.get_pass_ptr(cloudpass_ID));
   decorations.get_FeaturesGroup_ptr()->set_EarthRegionsGroup_ptr(
      &earth_regions_group);
   decorations.add_ArmySymbols(passes_group.get_pass_ptr(cloudpass_ID));
   decorations.add_ObsFrusta(
      passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);

// Instantiate PolyLines decorations group to hold roadways:

   decorations.add_PolyLines(ndims,passes_group.get_pass_ptr(cloudpass_ID));
   PolyLinesGroup* roadlines_group_ptr=decorations.get_PolyLinesGroup_ptr();
   roadlines_group_ptr->set_width(3);

   babygis_db.set_PolyLinesGroup_ptr(roadlines_group_ptr);
   babygis_db.setup_coordinate_transformation("WGS84",2001,19,true);

   const double xmin = 324211;	// meters
   const double xmax = 336667;	// meters
   const double ymin = 4.68985e+06;	// meters
   const double ymax = 4.69363e+06;	// meters

   threevector reference_vertex(xmin,ymin,0);
   babygis_db.set_reference_vertex(reference_vertex);
   babygis_db.set_altitude(10);	// meters
   babygis_db.pushback_gis_bbox(xmin,xmax,ymin,ymax);

   int geom_name_column=10;
   babygis_db.parse_table_contents(geom_name_column);

   const osg::Vec4 road_color(1,1,1,1);
   roadlines_group_ptr->set_uniform_color(road_color);
   roadlines_group_ptr->attach_bunched_geometries_to_OSGsubPAT(
      reference_vertex,0);

//   set_altitude_dependent_polyline_width();
   roadlines_group_ptr->update_display();

   cout << "# polylines = " 
        << roadlines_group_ptr->get_n_Graphicals() << endl;

/*
// Instantiate model decorations group:

   decorations.add_Models(
      passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
   string model_filename="./my_cessna.osg";
   Model* curr_model_ptr=decorations.get_ModelsGroup_ptr()->
      generate_new_Model(model_filename);
   
   double center_longitude=-71.09204305;	// MIT Dome
   double center_latitude=42.35978566;		// MIT Dome
   int racetrack_UTMzonenumber=19;
   bool racetrack_northern_hemisphere_flag=true;
   double center_easting,center_northing;
   latlongfunc::LLtoUTM(
      center_latitude,center_longitude,racetrack_UTMzonenumber,
      racetrack_northern_hemisphere_flag,center_northing,center_easting);

   threevector center_origin(center_easting,center_northing,0);
//   cout << "Center_origin = " << center_origin << endl;

   double radius=6000;
   double height_above_center=5000;
   double orbit_period=60;
   decorations.get_ModelsGroup_ptr()->generate_racetrack_orbit(
      center_origin,radius,height_above_center,orbit_period);

   const double cessna_scale_factor=10;
   curr_model_ptr->set_scale_attitude_posn(
      decorations.get_ModelsGroup_ptr()->get_passnumber(),cessna_scale_factor,
      decorations.get_ModelsGroup_ptr()->get_model_attitude(),
      decorations.get_ModelsGroup_ptr()->get_model_posn());

   AnimationController_ptr->set_nframes(
      decorations.get_ModelsGroup_ptr()->get_model_posn().size());
   AnimationController_ptr->set_last_framenumber(
      AnimationController_ptr->get_nframes()-1);

// Attach observation frusta to models group:

   ObsFrustaGroup* OFG_ptr=decorations.get_ObsFrustaGroup_ptr();
   decorations.get_ModelsGroup_ptr()->set_ObsFrustaGroup_ptr(OFG_ptr);
   // not sure if this last line is really needed...

   ObsFrustum* ObsFrustum_ptr=decorations.get_ObsFrustaGroup_ptr()->
      generate_new_ObsFrustum();
   decorations.get_ObsFrustaGroup_ptr()->instantiate_Graphical(
      ObsFrustum_ptr);
   decorations.get_ObsFrustaGroup_ptr()->insert_graphical_PAT_into_OSGsubPAT(
      ObsFrustum_ptr,0);
   ObsFrustum_ptr->get_PAT_ptr()->addChild(ObsFrustum_ptr->get_group_ptr());

   curr_model_ptr->set_ObsFrustum_ptr(ObsFrustum_ptr);

   const double delta_phi=90*PI/180;
   const double delta_theta = -45*PI/180;
   curr_model_ptr->orient_and_position_ObsFrustum(
      decorations.get_ModelsGroup_ptr()->get_passnumber(),
      decorations.get_ModelsGroup_ptr()->get_model_posn(),
      delta_phi,delta_theta);
*/

// Instantiate CentersGroup and center pick handler to handle mid
// point selection:

   CentersGroup centers_group(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),
      AnimationController_ptr,grid_origin_ptr);
   CenterPickHandler* CenterPickHandler_ptr=new CenterPickHandler(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),
      CM_3D_ptr,&centers_group,ModeController_ptr,&window_mgr,
      &transformer,grid_origin_ptr);
   viewer.getEventHandlerList().push_back(CenterPickHandler_ptr);
   viewer.getEventHandlerList().push_back(
      new CentersKeyHandler(ndims,CenterPickHandler_ptr,ModeController_ptr));

// Attach all 3D graphicals to CentersGroup SpinTransform which can
// perform global rotation about some axis coming out of a user
// selected center location:

   centers_group.get_SpinTransform_ptr()->addChild(
      decorations.get_OSGgroup_ptr());
   centers_group.get_SpinTransform_ptr()->addChild(
      clouds_group.get_OSGgroup_ptr());
   centers_group.get_SpinTransform_ptr()->addChild(
      latlonggrids_group.get_OSGgroup_ptr());
   root->addChild(centers_group.get_SpinTransform_ptr());

// Attach scene graph to the viewer:

   viewer.setSceneData(root);

// Open text dialog box to display feature information:

//   decorations.get_FeaturesGroup_ptr()->get_TextDialogBox_ptr()->
//      open("Feature Information");
//   decorations.get_FeaturesGroup_ptr()->update_feature_text();

// Viewer's realize method calls the Custom Manipulator's home() method:

   viewer.realize();
   osgUtil::SceneView* SceneView_ptr=viewer.getSceneHandlerList().front()->
      getSceneView();

   CM_3D_ptr->set_SceneView_ptr(SceneView_ptr);
   transformer.set_SceneView_ptr(SceneView_ptr);
   
// Add an animation path creator to the event handler list AFTER the
// viewer has been realized:

   AnimationPathCreator* animation_path_handler = 
      new AnimationPathCreator(&viewer);
   viewer.getEventHandlerList().push_back(animation_path_handler);

   viewer.getUsage(*arguments.getApplicationUsage());

// Set initial camera lateral posn to grid's midpoint and scale its
// altitude according to grid's maximal linear dimension:

   CM_3D_ptr->set_worldspace_center(latlonggrid_ptr->get_world_middle());
   CM_3D_ptr->set_eye_to_center_distance(
      2*max(latlonggrid_ptr->get_xsize(),latlonggrid_ptr->get_ysize()));
   CM_3D_ptr->update_M_and_Minv();

//   timefunc::initialize_timeofday_clock();
//   osg::FrameStamp* FrameStamp_ptr=viewer.getFrameStamp();

//   cout << "Before entering infinite viewer loop" << endl;
//   outputfunc::enter_continue_char();

   while( !viewer.done() )
   {

//      FrameStamp_ptr->setReferenceTime(timefunc::elapsed_timeofday_time());
//      cout << "t = " << FrameStamp_ptr->getReferenceTime() << endl;

      // Wait for all cull and draw threads to complete:
      viewer.sync();

        // Update the scene by traversing it with the update visitor
        // which will call all node update callbacks and animations:
      viewer.update();

      // Fire off the cull and draw traversals of the scene:
      viewer.frame();
   }

// Wait for all cull and draw threads to complete before exiting:

   viewer.sync();

//   decorations.get_FeaturesGroup_ptr()->get_TextDialogBox_ptr()->close();

   worldmodel_db.disconnect();
   babygis_db.disconnect();

   exit(0);
}
