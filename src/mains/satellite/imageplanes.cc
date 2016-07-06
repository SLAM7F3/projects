// ========================================================================
// Program IMAGEPLANES takes in a canonical satellite model XYZP file
// as well as a G99 video file containing ISAR satellite imagery.  The
// rotations, translations and scale factors needed to align the ISAR
// image planes with the model are read in from an imagecdf file.
// This program instantiates a time varying bordered image plane which
// displays the ISAR data as a 4D movie.  It also displays a "blue
// marble" earth model, a sun model located at its correct
// astronomical position given a Universal Time and a random
// background starfield.  The satellite's orbit is represented by an
// orange polyline encircling the earth.

// Invoke this program via a command like

//	imageplanes RH2.xyzp RH2_jet.vid RH2.imagecdf

//	imageplanes RH2.xyzp RH2_hielev.vid RH2_hielev.imagecdf

//	imageplanes RH2.xyzp RH2_midelev_jet.vid RH2_midelev.imagecdf

//	imageplanes RH2.xyzp RH2_loelev_jet.vid RH2_loelev.imagecdf

//	imageplanes RH2.xyzp RH2_loelev_inrange_jet.vid RH2_loelev_inrange.imagecdf

// We last verified that this program ran OK on 12/6/06.

// ========================================================================
// Last updated on 12/6/06; 12/16/06; 12/19/06; 12/20/06; 12/26/06; 12/29/06;
//  1/16/07; 2/4/07; 2/9/07; 2/14/07; 3/12/07; 4/23/07; 6/17/07; 8/20/07;
//  9/21/07; 10/15/07; 2/19/08
// ========================================================================

#include <set>
#include <string>
#include <vector>
#include <osgUtil/Optimizer>

#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "astro_geo/Clock.h"
#include "math/constant_vectors.h"
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
#include "general/outputfuncs.h"
#include "passes/PassesGroup.h"
#include "osg/osgSpace/PlanetsGroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "space/satellite.h"
#include "space/satellitefuncs.h"
#include "space/satellitepass.h"
#include "space/satelliteorbit.h"

#include "osg/osgWindow/ViewerManager.h"

using std::cin;
using std::cout;
using std::endl;
using std::pair;
using std::string;
using std::vector;

// ========================================================================
int main( int argc, char** argv )
{
   int seed=-2000;
   nrfunc::init_default_seed(seed);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);

// Instantiate clock to keep track of real time:

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
   string solarsystem_dir=OSG_data_dir+"SolarSystem/";
   string earth_filename=solarsystem_dir+"earth_bright60.osga";
   int earthpass_ID=passes_group.generate_new_pass(earth_filename);
   passes_group.get_pass_ptr(earthpass_ID)->set_passtype(Pass::earth);
   int videopass_ID=passes_group.get_videopass_ID();
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
   string imagecdf_filename(argv[3]);

   cout << "imagecdf_filename = " << imagecdf_filename << endl;

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

// Instantiate "holodeck" earth grid:

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

// Insert "holodeck" grid in XY equatorial plane:

   osg::BoundingBox bbox(EarthGraph_ptr->get_xyz_bbox());
//   cout << "earthgrid origin = " << *earthgrid_origin_ptr << endl;

// Instantiate group to hold pointcloud information:

   PointCloudsGroup clouds_group(
      passes_group.get_pass_ptr(cloudpass_ID),grid_origin_ptr);
   PointCloud* cloud_ptr=clouds_group.generate_new_Cloud();
   clouds_group.set_pt_size(clouds_group.get_pt_size()+2);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_ptr));

// Initialize ALIRT grid based upon cloud's bounding box:

//   double fictitious_sat_magnification=5;   
   double fictitious_sat_magnification=1000;   
//   double fictitious_sat_magnification=50000;

//   const double x_magnification_factor=2.5;
//   const double y_magnification_factor=1.5;
//   const double x_magnification_factor=2;	// OK for AK model
//   const double y_magnification_factor=1.3;	// OK for AK model
   const double x_magnification_factor=1.5;	// OK for RH2 model
   const double y_magnification_factor=1.2;	// OK for RH2 model
   double min_X=x_magnification_factor*cloud_ptr->get_min_value(0);
   double max_X=x_magnification_factor*cloud_ptr->get_max_value(0);
   double min_Y=y_magnification_factor*cloud_ptr->get_min_value(1);
   double max_Y=y_magnification_factor*cloud_ptr->get_max_value(1);
   double min_Z=cloud_ptr->get_min_value(2);

   decorations.get_AlirtGridsGroup_ptr()->initialize_grid(
      grid_ptr,min_X,max_X,min_Y,max_Y,min_Z);
   grid_ptr->set_axes_labels("Relative Model X (meters)",
                             "Relative Model Y (meters)");
//   grid_ptr->set_delta_xy(1,1);
   grid_ptr->set_delta_xy(2,2);
   grid_ptr->set_axis_char_label_size(0.6);	// OK for AK model
   grid_ptr->set_tick_char_label_size(0.6);	// OK for AK model
//   grid_ptr->set_axis_char_label_size(1.0);
//   grid_ptr->set_tick_char_label_size(1.0);
   grid_ptr->set_rot_x_axis_label_flag(false);
   grid_ptr->set_ticks_in_xy_plane_flag(true);
   grid_ptr->update_grid();

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

// Parse imagecdf header information.  For each image, save image
// frame's rotation, scaling and translation information into the
// following STL vectors which will subsequently be used to display
// the image planes in the model's reference frame:

   vector<pair<int,int> > image_size;
   vector<twovector> meters_per_pixel,center_shift,trans;
   vector<genmatrix*> Rmodel_ptrs;

   string sat_name="RH2";
   satellitepass pass(sat_name);
   bool regularize_images=false;	 
   if (pass.initialize_pass(imagecdf_filename,regularize_images))
   {
      vector<satelliteimage*> satimages_ptr=pass.get_satimage_ptrs();
      for (int n=0; n<satimages_ptr.size(); n++)
      {
         satelliteimage* currimage_ptr=satimages_ptr[n];
         pair<int,int> image_center=currimage_ptr->get_p_center();
         twoDarray* ztwoDarray_ptr=currimage_ptr->get_z2Darray_orig_ptr();
         double dx=ztwoDarray_ptr->get_deltax();
         double dy=ztwoDarray_ptr->get_deltay();

         image_size.push_back(pair<int,int>(ztwoDarray_ptr->get_mdim(),
                                            ztwoDarray_ptr->get_ndim()));
         meters_per_pixel.push_back(twovector(dx,dy));
         center_shift.push_back(twovector(image_center.first*dx,
                                          image_center.second*dy));
         trans.push_back(twovector(0,0));
         Rmodel_ptrs.push_back(
            satellitefunc::XELIAS_imageframe_to_model_rotation_matrix(
               currimage_ptr->get_target_ptr()->get_R0_rotate()));

         cout << "image = " << n << endl;
         cout << "dims in pixels = " << image_size.back().first << " x"
              << image_size.back().second << endl;
         cout << "meters_per_pixel = " << meters_per_pixel.back() << endl;
         cout << "center_shift = " << center_shift.back() << endl;
         cout << "Rmodel = " << *(Rmodel_ptrs.back()) << endl;
      } // loop over index n labeling images within pass
   }

   for (int i=0; i<movie_ptr->get_Nimages(); i++)
   {
      double curr_time=double(i);
      genmatrix Rimageplane(3,3);
      Rimageplane=Rmodel_ptrs[i]->transpose();

// Scale, rotation and translation info needed to transform movie
// window:

      twovector UVsize_in_meters(
         movie_ptr->getWidth()*meters_per_pixel[i].get(0),
         movie_ptr->getHeight()*meters_per_pixel[i].get(1));
      movie_ptr->rotate_scale(
         curr_time,videopass_ID,Rimageplane,UVsize_in_meters);

//      const double radial_displacement_along_phat_axis=30.0;	 // meters
      const double radial_displacement_along_phat_axis=-30.0;	 // meters
      threevector curr_trans=movie_ptr->translate(
         curr_time,videopass_ID,meters_per_pixel[i],center_shift[i],
         trans[i],radial_displacement_along_phat_axis);
    
// Recall that we effectively take the AK model's Z=0 plane to lie at
// the bottom of the IGES model.  In constrast, the IGES model was
// built with its Z=0 plane located approximately 4.5 meters within
// its interior.  In order to take this discrepancy into account, we
// need to shift the imageplane in the +Zhat direction by 4.5
// meters...

      threevector Ztrans=cloud_ptr->get_min_value(2)*z_hat;
      threevector origin=*grid_origin_ptr-Ztrans-curr_trans;

// If the IGES model lies within the center of the display grid, we
// need to translate the image plane in XY worldspace by the
// threevector rho defined below:

      threevector rho=(grid_ptr->get_world_middle()- (*grid_origin_ptr) );
      origin += rho;
      movie_ptr->set_frame_origin(origin);

      movie_ptr->transform_UV_to_XYZ_coords(curr_time,videopass_ID);
      imageframes_group.get_ImageFrame_ptr(0)->
         transform_linesegments(curr_time,videopass_ID,movie_ptr);

//      cout << "t = " << curr_time << endl;
//      cout << "Uhat = " << movie_ptr->get_Uhat() << endl;
//      cout << "Vhat = " << movie_ptr->get_Vhat() << endl;
//      cout << "Uhat.Vhat = " << movie_ptr->get_Uhat().dot(
//         movie_ptr->get_Vhat()) << endl;
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
      avg_origin_posn,orbit_posn_ECI,colorfunc::get_OSG_color(
         colorfunc::orange));
   decorations.get_PolyLinesGroup_ptr()->set_width(2);

// Rotate and translate satellite point cloud and image frames to
// bring them to correct location in ECI space.  Magnify both so that
// they can be seen against the much larger earth background.  Insert
// a MatrixTransform into scenegraph and then attach point cloud and
// imageframes group onto it.
  
   threevector sat_COM(-734112.23 , 6002091.7 , 3637674.2);
//   threevector sat_COM(-618416.75 , 5742186.6 , 4053604.5);
   threevector n_hat(0.96858407 , 0.20435635 , -0.14171586);
   threevector a_hat(0.10403086 , -0.8505549 , -0.51549389);
   threevector b_hat(n_hat.cross(-a_hat));
   genmatrix R_IGES2ECI(3,3),Rtrans(3,3);
   R_IGES2ECI.put_column(0,n_hat);
   R_IGES2ECI.put_column(1,-a_hat);
   R_IGES2ECI.put_column(2,b_hat);
   Rtrans=R_IGES2ECI.transpose();
   
//   cout << "R_IGES2ECI = " << R_IGES2ECI << endl;

   osg::MatrixTransform* SatelliteTransform_ptr=osgfunc::
      generate_rot_scale_and_trans(
         fictitious_sat_magnification,R_IGES2ECI,sat_COM);
   SatelliteTransform_ptr->addChild(clouds_group.get_OSGgroup_ptr());
   SatelliteTransform_ptr->addChild(movies_group.get_OSGgroup_ptr());
   SatelliteTransform_ptr->addChild(imageframes_group.get_OSGgroup_ptr());
   SatelliteTransform_ptr->addChild(grid_ptr->get_geode_ptr());
   root->addChild(SatelliteTransform_ptr);
   
// Instantiate signposts decorations group:

   decorations.add_SignPosts(
      ndims,passes_group.get_pass_ptr(earthpass_ID),
      planets_group.get_EarthSpinTransform_ptr());

   double longitude=-71.487166;	// degs		HAX
   double latitude=42.622833;	// degs		HAX
   double altitude=101;		// meters
   SignPost* curr_SignPost_ptr=decorations.get_SignPostsGroup_ptr(0)->
      generate_new_SignPost_on_earth(longitude,latitude,altitude);
   curr_SignPost_ptr->set_label("HAX");

// Attach scene graph to the viewer:

   root->addChild(decorations.get_OSGgroup_ptr());
   window_mgr_ptr->setSceneData(root);

// Create the windows and run the threads.  Viewer's realize method
// calls the CUstomManipulator's home() method:

   window_mgr_ptr->realize();

// Set initial camera view to grid's midpoint:

//   CM_3D_ptr->set_worldspace_center(grid_ptr->get_world_middle());
//   CM_3D_ptr->set_eye_to_center_distance(
//      2*max(grid_ptr->get_xsize(),grid_ptr->get_ysize()));
   CM_3D_ptr->set_worldspace_center(sat_COM);
   CM_3D_ptr->set_eye_to_center_distance(2*sat_COM.magnitude());
   CM_3D_ptr->update_M_and_Minv();

   while (!window_mgr_ptr->done())
   {
      window_mgr_ptr->process();
   }

   return 0;
}

