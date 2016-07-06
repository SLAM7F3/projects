// ========================================================================
// Program PHOTOS3D displays a series of quasi-georegistered JPEG
// photos within a 3D environment.  It can also (hopefully) display a
// longitude/latitude base grid as well as a "Google Earth" aerial
// image texture.  We wrote this program for testing Ricoh camera
// output as well as eventual mass insertion of photos into 3D point
// clouds.

/*

  photos3d --surface_texture sandiego_EO.pkg 

  photos3d --surface_texture sandiego_EO.pkg --region_filename ricoh29.pkg 

  photos3d --surface_texture sandiego_EO.pkg \
   --region_filename ricoh29.pkg --region_filename ricoh30.pkg \
   --region_filename ricoh31.pkg --region_filename ricoh32.pkg 

  photos3d --surface_texture sandiego_EO.pkg \
   --region_filename ricoh29.pkg --region_filename ricoh30.pkg \
   --region_filename ricoh31.pkg --region_filename ricoh32.pkg \
   --region_filename ricoh33.pkg --region_filename ricoh34.pkg \
   --region_filename ricoh35.pkg --region_filename ricoh36.pkg \
   --region_filename ricoh37.pkg --region_filename ricoh38.pkg 

// As of 8/7/07, the following inverted version of the argument list
// still prevents the underlying latlong grid from appearing:

   photos3d --region_filename ricoh29.pkg --region_filename ricoh30.pkg \
   --region_filename ricoh31.pkg --region_filename ricoh32.pkg \
   --region_filename ricoh33.pkg --region_filename ricoh34.pkg \
   --region_filename ricoh35.pkg --region_filename ricoh36.pkg \
   --region_filename ricoh37.pkg --region_filename ricoh38.pkg \
   --surface_texture sandiego_EO.pkg 

*/

// ========================================================================
// Last updated on 9/29/07; 10/11/07; 10/15/07; 10/26/07; 4/12/09
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CentersKeyHandler.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "astro_geo/Clock.h"
#include "color/colorfuncs.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "astro_geo/geomagnet.h"
#include "astro_geo/geopoint.h"
#include "osg/osgGrid/GridKeyHandler.h"
#include "osg/osgGrid/LatLongGridsGroup.h"
#include "osg/ModeController.h"
#include "osg/osgModels/MODELSGROUP.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "video/photograph.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "passes/TextDialogBox.h"
#include "time/timefuncs.h"
#include "osg/osgWindow/ViewerManager.h"

#include "templates/mytemplates.h"

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

// Read input texture and video files:

   const int ndims=3;
   PassesGroup passes_group(&arguments);
   int texturepass_ID=passes_group.get_curr_texturepass_ID();
//   cout << "textpurepass_ID = " << texturepass_ID << endl;
   vector<int> videopass_ID;
   for (unsigned int n=0; n<passes_group.get_n_passes(); n++)
   {
      if (passes_group.get_pass_ptr(n)->get_passtype()==Pass::video)
      {
         videopass_ID.push_back(n);
//         cout << "n = " << n << " videopass_ID = " << videopass_ID.back()
//              << endl;
      }
   }

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("3D imagery");
//   window_mgr_ptr->getUsage(*arguments.getApplicationUsage());

// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

   Operations operations(ndims,window_mgr_ptr,passes_group);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   root->addChild(operations.get_OSGgroup_ptr());

// Add a custom manipulator to the event handler list:

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate groups to hold multiple latitude-longitude grids,
// surface textures, and associated earth regions:
   
   LatLongGridsGroup latlonggrids_group(
      ndims,passes_group.get_pass_ptr(texturepass_ID),CM_3D_ptr);
   root->addChild(latlonggrids_group.get_OSGgroup_ptr());

   EarthRegionsGroup earth_regions_group(
      passes_group.get_pass_ptr(texturepass_ID),&latlonggrids_group);
   earth_regions_group.generate_regions(passes_group);
   root->addChild(earth_regions_group.get_OSGgroup_ptr());

   LatLongGrid* latlonggrid_ptr=latlonggrids_group.get_Grid_ptr(0);
   threevector* grid_origin_ptr=latlonggrid_ptr->get_world_origin_ptr();
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new GridKeyHandler(ModeController_ptr,latlonggrid_ptr));

   CM_3D_ptr->set_Grid_ptr(latlonggrid_ptr);

//   double theta=latlonggrid_ptr->
//      get_or_compute_UTM_to_latlong_gridlines_rot_angle();
//   cout << "theta = " << theta 
//        << " theta*180/PI = " << theta*180/PI << endl;

// Instantiate group to hold all decorations:
   
   Decorations decorations(
      window_mgr_ptr,ModeController_ptr,CM_3D_ptr,grid_origin_ptr);

// Instantiate a PointFinder;

   PointFinder pointfinder;
   CM_3D_ptr->set_PointFinder(&pointfinder);

// Instantiate OBSFRUSTAGROUP decoration group:

   if (videopass_ID.size() > 0)
   {
      decorations.add_OBSFRUSTA(
         passes_group.get_pass_ptr(videopass_ID.front()),
         AnimationController_ptr);
   }

// Instantiate an individual ObsFrustum for every input video.  Each
// contains a separate movie object.

   double curr_t=decorations.get_OBSFRUSTAGROUP_ptr()->get_curr_t();
   int pass_number=decorations.get_OBSFRUSTAGROUP_ptr()->get_passnumber();
   for (unsigned int n=0; n<passes_group.get_n_passes(); n++)
   {
      if (passes_group.get_pass_ptr(n)->get_passtype()==Pass::video)
      {
         string curr_video_filename=passes_group.get_pass_ptr(n)->
            get_first_filename();

// Group together all still images within common subgroup:

         int OSGsubPAT_number=0;	
         OBSFRUSTUM* OBSFRUSTUM_ptr=
            decorations.get_OBSFRUSTAGROUP_ptr()->generate_movie_OBSFRUSTUM(
               curr_video_filename,OSGsubPAT_number);
         OBSFRUSTUM_ptr->instantiate_OSG_Pyramids();

         OBSFRUSTUM_ptr->set_display_camera_model_flag(
            curr_t,pass_number,
//            true);
            false);
         
// Manipulate camera corresponding to ObsFrustum's movie:

         camera* camera_ptr=OBSFRUSTUM_ptr->get_Movie_ptr()->get_camera_ptr();
         camera_ptr->set_world_posn(*grid_origin_ptr);

//         double f=-1.5;
         double f=passes_group.get_pass_ptr(n)->get_PassInfo_ptr()->
         get_focal_length();
//      cout << "Enter focal length parameter f:" << endl;
//      cin >> f;
//      cout << "focal length = " << f << endl;

         camera_ptr->set_internal_params(
            f,f,camera_ptr->get_u0(),camera_ptr->get_v0());
         camera_ptr->construct_projection_matrix();

         double frustum_sidelength=-1;
         double movie_downrange_distance=50;
         OBSFRUSTUM_ptr->initialize_frustum_with_movie(
            frustum_sidelength,movie_downrange_distance);

         double magnetic_yaw=
            passes_group.get_pass_ptr(n)->get_PassInfo_ptr()->
            get_magnetic_yaw();
         double el=passes_group.get_pass_ptr(n)->get_PassInfo_ptr()->
            get_relative_el();
         double roll=passes_group.get_pass_ptr(n)->get_PassInfo_ptr()->
            get_relative_roll();

//      double az=0;
//      double el=0;
//      double roll=0;
//      cout << "Enter camera azimuth:" << endl;
//      cin >> az;
//      cout << "Enter camera elevation:" << endl;
//      cin >> el;
//      cout << "Enter camera roll:" << endl;
//      cin >> roll;
//      cout << "az = " << az << " el = " << el << " roll = " << roll << endl;

// Extract camera's geolocation and date from EXIF metadata:

         photograph photo(curr_video_filename);
//         cout << "photo = " << photo << endl;
         geopoint camera_geopoint=photo.get_geolocation();
         double decimal_year=photo.get_clock().get_decimal_year();

         double camera_alt=15;
         threevector camera_posn(camera_geopoint.get_UTM_easting(),
                                 camera_geopoint.get_UTM_northing(),
                                 camera_alt);

// Correct for difference between magnetic and true north in camera's
// absolute yaw:

         geomagnet curr_geomagnet;
         curr_geomagnet.set_geolocation(camera_geopoint);
         curr_geomagnet.get_geolocation().set_time(decimal_year);
         curr_geomagnet.compute_magnetic_field_components();
         double delta_yaw=curr_geomagnet.get_delta_yaw();
//         cout << "Delta_yaw = " << delta_yaw << endl;
         double yaw=magnetic_yaw+delta_yaw;

// Recall yaw is defined wrt north direction and increased in the
// positive east direction:

         double az=90-yaw;

         az *= PI/180;
         el *= PI/180;
         roll *= PI/180;

         OBSFRUSTUM_ptr->absolute_position_and_orientation(
            curr_t,pass_number,camera_posn,az,el,roll);

// Compute instantaneous above_Zplane pyramid.  Then build
// instantaneous ViewingPyramidAboveZplane graphical from this pyramid
// object:

         OBSFRUSTUM_ptr->generate_viewing_pyramid_ptr();
         double z_ground=0;
         OBSFRUSTUM_ptr->compute_viewing_pyramid_above_Zplane(
            z_ground,OBSFRUSTUM_ptr->get_viewing_pyramid_ptr());

         OBSFRUSTUM_ptr->generate_Pyramid_geodes();
         OBSFRUSTUM_ptr->get_ViewingPyramid_ptr()->build_current_pyramid(
            curr_t,pass_number,
            OBSFRUSTUM_ptr->get_viewing_pyramid_ptr());
         OBSFRUSTUM_ptr->get_ViewingPyramidAboveZplane_ptr()->
            build_current_pyramid(
               curr_t,pass_number,
               OBSFRUSTUM_ptr->get_viewing_pyramid_above_zplane_ptr());

         OBSFRUSTUM_ptr->set_typical_pyramid_edge_widths();

//         const double volume_alpha=0.1;
//         const double volume_alpha=0.01;
         const double volume_alpha=0.0;

         OBSFRUSTUM_ptr->set_permanent_color(
            colorfunc::get_color(n),volume_alpha);
         OBSFRUSTUM_ptr->set_selected_color(colorfunc::white,volume_alpha);
         OBSFRUSTUM_ptr->set_color(
            OBSFRUSTUM_ptr->get_permanent_color(),volume_alpha);

      } // video passtype conditional
   } // loop over index n labeling all input passes

   
// Attach scene graph to the viewer:

   root->addChild(decorations.get_OSGgroup_ptr());
   window_mgr_ptr->setSceneData(root);

// Open text dialog box to display feature information:

//   decorations.get_FeaturesGroup_ptr()->get_TextDialogBox_ptr()->
//      open("Feature Information");
//   decorations.get_FeaturesGroup_ptr()->update_feature_text();

// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_ptr->realize();

//   timefunc::initialize_timeofday_clock();
//   osg::FrameStamp* FrameStamp_ptr=window_mgr_ptr->getFrameStamp();

//   cout << "Before entering infinite viewer loop" << endl;
//   outputfunc::enter_continue_char();

   while( !window_mgr_ptr->done() )
   {
      window_mgr_ptr->process();
   }

//   decorations.get_FeaturesGroup_ptr()->get_TextDialogBox_ptr()->close();

}
