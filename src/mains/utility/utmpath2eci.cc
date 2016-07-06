// ========================================================================
// Program UTMPATH2ECI takes in an animation path generated in UTM
// coordinate space.  It generates a corresponding path within ECI
// coordinates which can be used to perform a "blue marble"
// flythrough.
// ========================================================================
// Last updated on 2/7/07; 2/20/07; 3/12/07; 8/20/07
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osg/Light>
#include <osg/LightSource>
#include <osg/LightModel>
#include <osgGA/NodeTrackerManipulator>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>

#include "osg/osgGrid/AlirtGridsGroup.h"
#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGraphicals/AnimationKeyHandler.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "astro_geo/Clock.h"
#include "color/colorfuncs.h"
#include "osg/osgSceneGraph/ColorMap.h"
#include "osg/Custom3DManipulator.h"
#include "osg/osgSceneGraph/DepthPartitionNode.h"
#include "osg/osgEarth/EarthManipulator.h"
#include "osg/osgGrid/EarthGrid.h"
#include "osg/osgEarth/EarthsGroup.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "astro_geo/Ellipsoid_model.h"
#include "general/filefuncs.h"
#include "osg/osgGrid/GridKeyHandler.h"
#include "astro_geo/latlong2utmfuncs.h"
#include "osg/osgGrid/LatLongGridsGroup.h"
#include "osg/ModeController.h"
#include "osg/ModeKeyHandler.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgfuncs.h"
#include "passes/PassesGroup.h"
#include "osg/osgSpace/PlanetsGroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/Transformer.h"
#include "osg/ViewerManager.h"

#include "general/outputfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

// ========================================================================
int main( int argc, char** argv )
{

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);

// Instantiate clock to keep track of real time:

   Clock clock;
//   clock.current_local_time_and_UTC();

   int year=2006;
   int month=8;
   int day=24;
//   int local_hour=12;

//   cout << "Enter local hour:" << endl;
//   cin >> local_hour;

//   int UTC_hour=11;
   int UTC_hour=20;
//   cout << "Enter UTC hour:" << endl;
//   cin >> UTC_hour;
   int minutes=0;
   double secs=0;

//   clock.set_UTM_zone_time_offset(19);	// Boston
//   clock.set_daylight_savings_flag(true);
//   clock.set_local_time(year,month,day,local_hour,minutes,secs);
   clock.set_UTC_time(year,month,day,UTC_hour,minutes,secs);

// Initialize constants and parameters read in from command line as
// well as ascii text file:

   const int ndims=3;
   string OSG_data_dir(getenv("OSG_FILE_PATH"));
   OSG_data_dir += "/";
   string colormap_dir=OSG_data_dir+"3D_colormaps/";
//   int p_map=2;		// small_hue_value
//   int p_map=7;	// wrap1
//   int p_map=8;	// wrap2
   int p_map=9;	// wrap3
   ColorMap colormap(colormap_dir,p_map);

// Read input ladar point cloud file:

   PassesGroup passes_group(&arguments);
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();

   string solarsystem_dir=OSG_data_dir+"SolarSystem/";
   string earth_filename=solarsystem_dir+"hires_earth.osga";
   int earthpass_ID=passes_group.generate_new_pass(earth_filename);
   passes_group.get_pass_ptr(earthpass_ID)->set_passtype(Pass::earth);
   cout << " earthpass_ID = " << earthpass_ID 
        << " cloudpass_ID = " << cloudpass_ID << endl;

// Construct the viewer and set it up with sensible event handlers:

   osgProducer::Viewer viewer(arguments);
   viewer.setClearColor(osg::Vec4(0,0,0,0));
   viewer.setUpViewer( osgProducer::Viewer::ESCAPE_SETS_DONE );

// Create a DepthPartitionNode to manage partitioning of the scene

   DepthPartitionNode* root = new DepthPartitionNode;
   root->setActive(true);     // Control whether the node analyzes the scene
//   cout << "max_depth = " << root->getMaxTraversalDepth() << endl;

// Instantiate a mode controller and mode key event handler:

   ModeController* ModeController_ptr=new ModeController();
   viewer.getEventHandlerList().push_back( 
      new ModeKeyHandler(ModeController_ptr) );
   root->addChild(osgfunc::create_Mode_HUD(ndims,ModeController_ptr));

// Instantiate "holodeck" ALIRT grid:

   AlirtGridsGroup alirtgrids_group(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   AlirtGrid* grid_ptr=alirtgrids_group.generate_new_Grid();
   threevector* grid_origin_ptr=grid_ptr->get_world_origin_ptr();
   viewer.getEventHandlerList().push_back(
      new GridKeyHandler(ModeController_ptr,grid_ptr));
   grid_ptr->get_geode_ptr()->setName("AlirtGrid");

   EarthGrid* earthgrid_ptr = new EarthGrid(colorfunc::grey);
   threevector* earthgrid_origin_ptr=earthgrid_ptr->get_world_origin_ptr();
   viewer.getEventHandlerList().push_back(
      new GridKeyHandler(ModeController_ptr,earthgrid_ptr));
//   root->addChild(earthgrid_ptr->get_geode_ptr());

// Instantiate solar system group to hold sun and planets:

   PlanetsGroup planets_group(
      passes_group.get_pass_ptr(earthpass_ID),&clock,earthgrid_origin_ptr);
   DataGraph* EarthGraph_ptr=planets_group.generate_EarthGraph();

// Instantiate a transformer in order to convert between screen and
// world space coordinate systems:

   Transformer transformer(&planets_group);

// Instantiate an EarthsGroup to hold "blue marble" coordinate system
// information:

   EarthsGroup earths_group(
      passes_group.get_pass_ptr(earthpass_ID),&clock,earthgrid_origin_ptr);
   Earth* Earth_ptr=earths_group.generate_new_Earth(&transformer);
   root->addChild(earths_group.get_OSGgroup_ptr());

// Instantiate WindowCoordConverter:

   ViewerManager window_mgr;
   window_mgr.set_Viewer_ptr(&viewer);
   window_mgr.initialize_window("3D imagery");

// Add a custom manipulator to the event handler list:

   osgGA::EarthManipulator* CM_3D_ptr=new osgGA::EarthManipulator(
      ModeController_ptr,Earth_ptr->get_Ellipsoid_model_ptr(),&clock,
      &window_mgr);
   window_mgr.set_CameraManipulator(CM_3D_ptr);
   CM_3D_ptr->set_Transformer(&transformer);

// Instantiate groups to hold multiple point clouds,
// latitude-longitude grids and associated earth regions:

   PointCloudsGroup clouds_group(
      &arguments,passes_group.get_pass_ptr(cloudpass_ID),&colormap);
   LatLongGridsGroup latlonggrids_group(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),CM_3D_ptr);
   EarthRegionsGroup earth_regions_group(
      passes_group.get_pass_ptr(earthpass_ID),
      &clouds_group,&latlonggrids_group,Earth_ptr);
   earth_regions_group.generate_regions(passes_group);

   EarthRegion* earthregion_ptr=earth_regions_group.get_region(0);
   DataGraph* DataGraph_ptr=earthregion_ptr->get_DataGraph_ptr();
   LatLongGrid* LatLongGrid_ptr=earthregion_ptr->get_LatLongGrid_ptr();

// First counter translation embedded within cloud's topmost
// MatrixTransform:

   threevector pointcloud_origin_long_lat_alt;
   osg::MatrixTransform* TransTransform_ptr=
      Earth_ptr->generate_datagraph_translation_MatrixTransform(
         dynamic_cast<PointCloud*>(DataGraph_ptr),
         pointcloud_origin_long_lat_alt);

// Next Z-rotate cloud/surface about its midpoint in order to optimally
// align the UTM subgrid with underlying lines of longitude and
// latitude on the earth ellipsoid:

   osg::MatrixTransform* UTM_to_LL_rot_transform_ptr=Earth_ptr->
      generate_UTM_to_latlong_grid_rot_MatrixTransform(
         LatLongGrid_ptr->
         get_or_compute_UTM_to_latlong_gridlines_rot_angle());

// Finally place datagraph onto blue marble's surface so that its XYZ
// axes are aligned with the local east, north and radial directions:

//   pointcloud_origin_long_lat_alt -= 
//      threevector(0,0,DataGraph_ptr->get_xyz_bbox().zMin());
   osg::MatrixTransform* cloud_SurfaceTransform_ptr=
      Earth_ptr->generate_earthsurface_MatrixTransform(
         pointcloud_origin_long_lat_alt);

   osg::MatrixTransform* EarthSpinTransform_ptr=planets_group.
      generate_earthrotation();

// Read in animation path generated within UTM coordinate space:

   string animpath_filename="fast_flythrough.path";
   cout << "Enter name of file containing animation path within UTM coords:"
        << endl;
   cin >> animpath_filename;
   filefunc::ReadInfile(animpath_filename);

// Prepare output file which will hold animation path in ECI
// coordinates:

   string ECI_animpath_filename="ECI_"+animpath_filename;
   ofstream outstream;
   filefunc::openfile(ECI_animpath_filename,outstream);

   vector<double> curr_anim_values;
   double time;
   osg::Vec3 posn;
   osg::Quat quat;
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      curr_anim_values.clear();
      curr_anim_values=stringfunc::string_to_numbers(filefunc::text_line[i]);
      time=curr_anim_values[0];
      posn=osg::Vec3(curr_anim_values[1],curr_anim_values[2],
                     curr_anim_values[3]);
      quat=osg::Quat(curr_anim_values[4],curr_anim_values[5],
                     curr_anim_values[6],curr_anim_values[7]);

//      cout << "time = " << time << endl;
//      cout << "posn = " << threevector(posn) << endl;
//      cout << "quat = " << quat.x() << "," << quat.y() << "," 
//           << quat.z() << "," << quat.w() << endl;

// First reconstruct 4x4 transformation matrix M from position and
// quaternion values read in from animation path:

      osg::Matrixd M;
      M.postMult(osg::Matrixd::rotate(quat));
      M.postMult(osg::Matrixd::translate(posn));

// Next multiply M by TransTransform, SurfaceTransform and
// EarthTransform 4x4 matrices to convert from UTM to ECI coordinates.
// Recall that 4-vectors are effectively treated within OSG as ROW
// vectors (and not columns!).  So we concatenate all these matrices
// by multiplication on the RIGHT:

      M.postMult(TransTransform_ptr->getMatrix());
      M.postMult(UTM_to_LL_rot_transform_ptr->getMatrix());
      M.postMult(cloud_SurfaceTransform_ptr->getMatrix());
      M.postMult(EarthSpinTransform_ptr->getMatrix());

//      cout << "M = " << endl;
//      osgfunc::print_matrix(M);

// Finally, decompose modified M back into a camera quaternion and
// translation within ECI coordinates.  Write new values to output
// animation path file:

      osg::Quat quat_ECI;
      quat_ECI.set(M);
      threevector posn_ECI(M(3,0),M(3,1),M(3,2));
      
      outstream.precision(12);
      outstream << time << " " 
                << posn_ECI.get(0) << " "
                << posn_ECI.get(1) << " "
                << posn_ECI.get(2) << " "
                << quat_ECI.x() << " "
                << quat_ECI.y() << " "
                << quat_ECI.z() << " "
                << quat_ECI.w() << endl;
   } // loop over index i labeling lines within input animation path file
   filefunc::closefile(ECI_animpath_filename,outstream);
}

