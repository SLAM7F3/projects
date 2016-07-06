// ========================================================================
// Program SPASE_IMAGEPLANES takes in the canonical SPASE model XYZP
// file ("spase_model.xyzp") as well as a G99 video file containing
// ISAR satellite imagery ("reordered_sar.vid").  The orientations of
// the ISAR image planes relative to the SPASE model are hardwired
// into this program via azimuth and elevation information.  This
// program instantiates a time varying bordered image plane which
// displays the ISAR data as a 4D movie relative to a world grid.

//		spase_imageplanes --region_filename spase.pkg

// 	  	spase_imageplanes spase_model.xyzp reordered_sar.vid

// ========================================================================
// Last updated on 12/30/07; 2/19/08; 10/29/08
// ========================================================================

#include <string>
#include <vector>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>

#include "osg/osgGrid/AlirtGridsGroup.h"
#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "astro_geo/Clock.h"
#include "osg/Custom3DManipulator.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgSceneGraph/DepthPartitionNode.h"
#include "osg/osgSceneGraph/DistanceAccumulator.h"
#include "osg/osgGrid/EarthGrid.h"
#include "astro_geo/Ellipsoid_model.h"
#include "osg/osgGrid/GridKeyHandler.h"
#include "osg/osgAnnotators/ImageFramesGroup.h"
#include "osg/ModeController.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "numrec/nrfuncs.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osgSpace/PlanetsGroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "math/rotation.h"
#include "space/satelliteorbit.h"
#include "space/spasefuncs.h"
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
   int seed=-2000;
   nrfunc::init_default_seed(seed);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);

   Clock clock;

   int year=2006;
   int month=6;
   int day=14;
   int UTC_hour=18;
   int minutes=18;
   double secs=12;
   clock.set_UTM_zone_time_offset(19);		// Boston & HAX
   clock.set_daylight_savings_flag(true);
   clock.set_UTC_time(year,month,day,UTC_hour,minutes,secs);

// Initialize constants and parameters read in from command line as
// well as ascii text file:

   const int ndims=3;
   string OSG_data_dir(getenv("OSG_FILE_PATH"));
   OSG_data_dir += "/";

   PassesGroup passes_group(&arguments);
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
   int videopass_ID=passes_group.get_videopass_ID();

   string solarsystem_dir=OSG_data_dir+"SolarSystem/";
   string earth_filename=solarsystem_dir+"earth_bright60.osga";
   int earthpass_ID=passes_group.generate_new_pass(earth_filename);
   passes_group.get_pass_ptr(earthpass_ID)->set_passtype(Pass::earth);

//   cout << "cloudpass_ID = " << cloudpass_ID << endl;
//   cout << "videopass_ID = " << videopass_ID << endl;
//   cout << "earthpass_ID = " << earthpass_ID << endl;

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("3D imagery");
//   window_mgr_ptr->getUsage(*arguments.getApplicationUsage());

// Create a DepthPartitionNode to manage partitioning of the scene

   DepthPartitionNode* root = new DepthPartitionNode;
   root->setActive(true);     // Control whether the node analyzes the scene
//   cout << "max_depth = " << root->getMaxTraversalDepth() << endl;

// Instantiate Operations object to handle mode, animation and image
// number control:

   bool display_movie_state=true;
   bool display_movie_number=true;
   Operations operations(ndims,window_mgr_ptr,display_movie_state,
                         display_movie_number);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   root->addChild(operations.get_OSGgroup_ptr());

// Add a custom manipulator to the event handler list:

   osgGA::Custom3DManipulator* CM_3D_ptr=new osgGA::Custom3DManipulator(
      ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold all decorations:
   
   Decorations decorations(
      window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

// Instantiate "holodeck" grids:

   EarthGrid* earthgrid_ptr = new EarthGrid(colorfunc::brightpurple);
   threevector* earthgrid_origin_ptr=earthgrid_ptr->get_world_origin_ptr();
   decorations.set_grid_origin_ptr(earthgrid_origin_ptr);
//   root->addChild(earthgrid_ptr->get_geode_ptr());

// Instantiate AlirtGrid decorations group:

   AlirtGrid* grid_ptr=decorations.add_AlirtGrid(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   threevector* grid_origin_ptr=grid_ptr->get_world_origin_ptr();
   grid_ptr->set_curr_color(osg::Vec4(0.6,0.6,0.6,0.8));

// Instantiate a MyDatabasePager to handle paging of files from disk:

   viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager();

// Instantiate PlanetsGroup to hold sun and planets:

   PlanetsGroup planets_group(
      passes_group.get_pass_ptr(earthpass_ID),&clock,earthgrid_origin_ptr);
   root->addChild(planets_group.get_EarthSpinTransform_ptr());

// Generate the earth as well the solar system:

   DataGraph* EarthGraph_ptr=planets_group.generate_EarthGraph();
   osg::Group* solarsystem_ptr=planets_group.generate_solarsystem(
      earthgrid_ptr);
   root->addChild(solarsystem_ptr);

// Instantiate a PointFinder;

   PointFinder pointfinder(&planets_group);
   CM_3D_ptr->set_PointFinder(&pointfinder);

// Generate random background star field:

   root->addChild(planets_group.generate_starfield(earthgrid_ptr));

// Instantiate group to hold pointcloud information:

   PointCloudsGroup clouds_group(
      passes_group.get_pass_ptr(cloudpass_ID),grid_origin_ptr);
   PointCloud* cloud_ptr=clouds_group.generate_new_Cloud();
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_ptr));

// Initialize ALIRT grid based upon cloud's bounding box:

   decorations.get_AlirtGridsGroup_ptr()->initialize_grid(
      grid_ptr,clouds_group.get_xyz_bbox());

   bool SPASE_flag=true;
   grid_ptr->initialize_satellite_grid(SPASE_flag);

// Instantiate group to hold movie:

   MoviesGroup movies_group(
      ndims,passes_group.get_pass_ptr(videopass_ID),
      AnimationController_ptr);
   const double alpha=0.5;
//   const double alpha=1.0;
   Movie* movie_ptr=movies_group.generate_new_Movie(passes_group,alpha);
   AnimationController_ptr->set_nframes(movie_ptr->get_Nimages());
   root->addChild( movies_group.get_OSGgroup_ptr() );

// Instantiate ImageFrame objects to border time-varying image planes:

   ImageFramesGroup imageframes_group(
      passes_group.get_pass_ptr(videopass_ID),AnimationController_ptr);
   ImageFrame* curr_imageframe_ptr=
      imageframes_group.generate_new_ImageFrame();
   const double fictitious_sat_magnification=10000;
   curr_imageframe_ptr->set_axes_labels("Cross Range","Range",
                                        fictitious_sat_magnification);
   root->addChild(imageframes_group.get_OSGgroup_ptr());

// Instantiate CentersGroup and center pick handler to handle mid
// point selection:

   CentersGroup centers_group(
      3,passes_group.get_pass_ptr(cloudpass_ID));
   CenterPickHandler* CenterPickHandler_ptr=new CenterPickHandler(
      3,passes_group.get_pass_ptr(cloudpass_ID),
      CM_3D_ptr,&centers_group,ModeController_ptr,window_mgr_ptr,
      grid_origin_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      CenterPickHandler_ptr);

// Rotation matrix needed to bring canonical model into the physical
// orientation at the compact test range where Weber Hoen took his SAR
// data:

   vector<double> azimuth,elevation;

   azimuth.push_back(180+2);
   azimuth.push_back(180+1);
   azimuth.push_back(180+2);

   azimuth.push_back(180+2);
   azimuth.push_back(180+3);
   azimuth.push_back(180+3);

   azimuth.push_back(170+3);
   azimuth.push_back(170+6);
   azimuth.push_back(165+8);

   elevation.push_back(20);
   elevation.push_back(17.5);
   elevation.push_back(15);

   elevation.push_back(12.5);
   elevation.push_back(10);
   elevation.push_back(5);

   elevation.push_back(10);
   elevation.push_back(25);
   elevation.push_back(30);

   vector<threevector> trans,scale;

   trans.push_back(threevector(-0.456,-0.587));
   trans.push_back(threevector(-0.458,-0.651));
   trans.push_back(threevector(-0.455,-0.729));

   trans.push_back(threevector(-0.439,-0.791));
   trans.push_back(threevector(-0.436,-0.914));
   trans.push_back(threevector(-0.418,-1.082));

   trans.push_back(threevector(-0.483,-0.894));
   trans.push_back(threevector(-0.462,-1.065));
   trans.push_back(threevector(-0.546,-0.952));

   scale.push_back(threevector(1.687,1.624,1));
   scale.push_back(threevector(1.671,1.540,1));
   scale.push_back(threevector(1.669,1.528,1));

   scale.push_back(threevector(1.614,1.469,1));
   scale.push_back(threevector(1.622,1.577,1));
   scale.push_back(threevector(1.536,1.577,1));

   scale.push_back(threevector(1.669,1.520,1));
   scale.push_back(threevector(1.688,1.558,1));
   scale.push_back(threevector(1.933,1.719,1));

   for (int i=movie_ptr->get_first_framenumber(); 
        i <= movie_ptr->get_last_framenumber(); i++)
   {
      double curr_time=double(i);
      rotation Rmodel,Rimageplane;
      spasefunc::wireframe_rotation_corresponding_to_mount_az_el(
         azimuth[i],elevation[i],Rmodel);
      Rimageplane=Rmodel.transpose();

// Scale, rotation and translation info needed to transform movie
// window:

      threevector Uhat,Vhat;
      Rimageplane.get_column(0,Uhat);
      Rimageplane.get_column(1,Vhat);
      movie_ptr->set_UVW_dirs(curr_time,videopass_ID,Uhat,Vhat);
      movie_ptr->set_UVW_scales(
         curr_time,videopass_ID,movie_ptr->get_maxU()*scale[i].get(0),
         movie_ptr->get_maxV()*scale[i].get(1));
      threevector What(Uhat.cross(Vhat));
      const double radius=0.75;	 // meters

      threevector curr_trans=trans[i].get(0)*Uhat+trans[i].get(1)*Vhat;
      threevector imageplane_origin=radius*What+curr_trans;

      plane imageplane(What,imageplane_origin);
      movie_ptr->set_frame_origin(imageplane_origin);

      movie_ptr->transform_UV_to_XYZ_coords(curr_time,videopass_ID);
      imageframes_group.get_ImageFrame_ptr(0)->
         transform_linesegments(curr_time,videopass_ID,movie_ptr);

//      cout << "t = " << curr_time << endl;
//      cout << "Uhat = " << Uhat << endl;
//      cout << "Vhat = " << Vhat << endl;
//      cout << "Uhat.Vhat = " << Uhat.dot(Vhat) << endl;

   } // loop over index i labeling video image number
 
   
// Reconstruct satellite orbits:

   satelliteorbit orbit;
   orbit.set_a_semimajor(7055644);
   orbit.set_eccentricity(0.0013801821);
   orbit.set_omega_avg(threevector(
      0.0010319554 , 0.00021774134 , -0.00015100841));
   orbit.set_Runge_Lenz_vector(threevector(
      1.1774995E11,-2.1714846E11,4.915657E11));
   vector<threevector> orbit_posn_ECI=orbit.target_locations();

// Instantiate PolyLines decorations group to represent satellite orbits:

   decorations.add_PolyLines(ndims,passes_group.get_pass_ptr(cloudpass_ID));
   threevector avg_origin_posn=decorations.get_PolyLinesGroup_ptr()->
      compute_vertices_average(orbit_posn_ECI);
   decorations.get_PolyLinesGroup_ptr()->generate_new_PolyLine(
      avg_origin_posn,orbit_posn_ECI,
      colorfunc::get_OSG_color(colorfunc::orange));
   decorations.get_PolyLinesGroup_ptr()->set_width(2);

// Rotate and translate satellite point cloud and image frames to
// bring them to correct location in ECI space.  Magnify both so that
// they can be seen against the much larger earth background.  Insert
// a MatrixTransform into scenegraph and then attach point cloud and
// imageframes group onto it.
  
//   threevector sat_COM(0,0,0);
   threevector sat_COM(-734112.23 , 6002091.7 , 3637674.2);
//   threevector sat_COM(-618416.75 , 5742186.6 , 4053604.5);
   osg::MatrixTransform* SatelliteTransform_ptr=osgfunc::
      generate_scale_and_trans(fictitious_sat_magnification,sat_COM);
   SatelliteTransform_ptr->addChild(clouds_group.get_OSGgroup_ptr());
   SatelliteTransform_ptr->addChild(movies_group.get_OSGgroup_ptr());
   SatelliteTransform_ptr->addChild(imageframes_group.get_OSGgroup_ptr());
   SatelliteTransform_ptr->addChild(
      decorations.get_AlirtGridsGroup_ptr()->get_OSGgroup_ptr());
   root->addChild(SatelliteTransform_ptr);
   
// Instantiate signposts decorations group:

   bool include_into_Decorations_OSGgroup_flag=true;
   Ellipsoid_model earth_Ellipsoid_model;
   decorations.add_SignPosts(
      passes_group.get_pass_ptr(earthpass_ID),&clock,
      &earth_Ellipsoid_model,include_into_Decorations_OSGgroup_flag,
      planets_group.get_EarthSpinTransform_ptr());
   SignPostsGroup* SignPostsGroup_ptr=decorations.get_SignPostsGroup_ptr(0);
   SignPostsGroup_ptr->set_common_geometrical_size(10000);

   double longitude=-71.487166;	// degs		HAX
   double latitude=42.622833;	// degs		HAX
   double altitude=101;		// meters
   SignPost* curr_SignPost_ptr=SignPostsGroup_ptr->
      generate_new_SignPost_on_earth(longitude,latitude,altitude);
   curr_SignPost_ptr->set_label("HAX");

// Attach scene graph to the viewer:

   root->addChild(decorations.get_OSGgroup_ptr());
   window_mgr_ptr->setSceneData(root);

// Create the windows and run the threads.  Viewer's realize method
// calls the CUstomManipulator's home() method:

   window_mgr_ptr->realize();

// Set initial camera view to grid's midpoint:

   CM_3D_ptr->set_worldspace_center(sat_COM);
   CM_3D_ptr->set_eye_to_center_distance(0.5*sat_COM.magnitude());
//   CM_3D_ptr->set_eye_to_center_distance(12*sat_COM.magnitude());
   CM_3D_ptr->update_M_and_Minv();

   while (!window_mgr_ptr->done())
   {
      window_mgr_ptr->process();
   }
}

