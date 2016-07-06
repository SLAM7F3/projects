// ========================================================================
// Program GEOREGPANO reads in package files for 10 WISP panoramic
// video panels containing reasonable initial estimates for the
// cameras' calibration parameters.  For example, here are the
// contents of an initial package file for Deer Island WISP panel 0:

/*

/home/cho/programs/c++/svn/projects/src/mains/photosynth/bundler/DIME/Feb2013_DeerIsland/images/panels/DeerIsland_p0_fullres_00000.png
--Uaxis_focal_length -2.78243
--Vaxis_focal_length -2.78243
--U0 0.90909
--V0 0.5
--relative_az -78.32587768
--relative_el -0.3337800907
--relative_roll -2.456601016
--camera_x_posn 339104.8525
--camera_y_posn 4690479.106
--camera_z_posn 10.56898515
--frustum_sidelength 18

*/

// It also reads in manually established tiepoints selected between a
// ladar point cloud and WISP panels whose sky-sea horizons have been
// reset to their vertical midpoints via program RESET_HORIZON.  After
// iterating over possible WISP camera position threevectors,
// GEOREGPANO computes the global azimuthal rotation which optimally
// aligns maps the projected 3D and their 2D tiepoint counterparts.
// After a georegistered set of translated and rotated
// package files are written to disk, they can be directly
// imported into program VIEWBUNDLER.

/* 

./georegpano \
--region_filename ./bundler/DIME/Feb2013_DeerIsland/packages/panels/DeerIsland_p0_fullres_00000.pkg \
--region_filename ./bundler/DIME/Feb2013_DeerIsland/packages/panels/DeerIsland_p1_fullres_00000.pkg \
--region_filename ./bundler/DIME/Feb2013_DeerIsland/packages/panels/DeerIsland_p2_fullres_00000.pkg \
--region_filename ./bundler/DIME/Feb2013_DeerIsland/packages/panels/DeerIsland_p3_fullres_00000.pkg \
--region_filename ./bundler/DIME/Feb2013_DeerIsland/packages/panels/DeerIsland_p4_fullres_00000.pkg \
--region_filename ./bundler/DIME/Feb2013_DeerIsland/packages/panels/DeerIsland_p5_fullres_00000.pkg \
--region_filename ./bundler/DIME/Feb2013_DeerIsland/packages/panels/DeerIsland_p6_fullres_00000.pkg \
--region_filename ./bundler/DIME/Feb2013_DeerIsland/packages/panels/DeerIsland_p7_fullres_00000.pkg \
--region_filename ./bundler/DIME/Feb2013_DeerIsland/packages/panels/DeerIsland_p8_fullres_00000.pkg \
--region_filename ./bundler/DIME/Feb2013_DeerIsland/packages/panels/DeerIsland_p9_fullres_00000.pkg \
--region_filename ./bundler/DIME/Feb2013_DeerIsland/packages/panels/dummy.pkg \
--initial_mode Manipulate_Fused_Data_Mode 

# Note added on 1/16/2011:

# Recall Graphical observation is automatically instantiated and filled
# with zero UVW values whenever a new Graphical is generated.  In order to 
# avoid having specious zero-valued Features corresponding to pass#4, we 
# read in a dummy pass#5.  Ladar XYZ coordinates for all of the features
# corresponds to actual pass#5.  So the dummy zero values will be
# overwritten by the ladar coordinates, and pass# 4 will not have any
# specious zero-valued feature entries.

*/

// Used program FUSION with 2D/3D panel 0 features to come up with
// initial camera position estimate for Nov 2011 WISP data:

// # Camera world X = 312112.529619
// # Camera world Y = 4703879.67189
// # Camera world Z = 19.8137013222

//  For Nov 2011 WISP imagery, f=-2.40324
//  FOV_v = 23.5057 degs
//  U0 = 0.5*aspect_ratio = 0.780859


// Output from program UVCORRECT_PANOS generated on 3/25/13:

// <v_avg> = 0.493137
// Delta_v = -0.00686329
// pv_horizon = 1115
// delta_vert_pixels = -15.0992
// delta_vert_theta = -0.135893 degs
// theta = 89.8641
// WISP altitude above sea-level a = 17.9196 meters
// Range to horizon r = 15110.7 meters

// ========================================================================
// Last updated on 3/13/13; 3/16/13; 4/25/13
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include "video/camera.h"
#include "video/camerafuncs.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgFeatures/FeaturesGroup.h"
#include "osg/ModeController.h"
#include "osg/osgOperations/Operations.h"
#include "numerical/param_range.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgWindow/ViewerManager.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::string;
using std::vector;

// ==========================================================================
int main( int argc, char** argv )
{

/*   
   double f,FOV_v,FOV_u=36*PI/180;
   double aspect_ratio=1999.0/1280.0;
   camerafunc::f_and_vert_FOV_from_horiz_FOV_and_aspect_ratio(
      FOV_u,aspect_ratio,f,FOV_v);
   cout << "f = " << f 
        << " FOV_v = " << FOV_v*180/PI << endl;
//   cout << "aspect_ratio = " << aspect_ratio 
//        << " 0.5*aspect_ratio = " << 0.5*aspect_ratio << endl;

//	f=-2.40324
//	FOV_v = 23.5057 degs
// 	U0 = 0.5*aspect_ratio = 0.780859
*/

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   string image_list_filename=passes_group.get_image_list_filename();
//   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
//   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string packages_subdir=bundler_IO_subdir+"packages/";
   cout << "packages_subdir = " << packages_subdir << endl;

   int n_passes=passes_group.get_n_passes();
   cout << "n_passes = " << n_passes << endl;

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("3D imagery");

// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

   Operations operations(ndims,window_mgr_ptr,passes_group);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   root->addChild(operations.get_OSGgroup_ptr());

// Add a custom manipulator to the event handler list:

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold all decorations:
   
   Decorations decorations(window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group);

   string order_filename="./packages/calib/panels_order.dat";
   photogroup_ptr->set_photo_order(order_filename);
   
   int n_photos(photogroup_ptr->get_n_photos());
   cout << "n_photos = " << n_photos << endl;

// Instantiate FeaturesGroup to hold small number of manually
// extracted ladar and corresponding panoramic stills features:

   FeaturesGroup* manual_FeaturesGroup_ptr=new FeaturesGroup(
      ndims,passes_group.get_pass_ptr(n_passes-1),CM_3D_ptr);
   string manual_features_subdir=bundler_IO_subdir+
      "stable_frames/orig_panels/frame_00000/";
   string manual_features_filename=manual_features_subdir+
      "features_combined_DeerIsland.txt";
   cout << "manual_features_filename = " << manual_features_filename
        << endl;

// Important note added on 4/25/13: Needed to reset bool
// convert_passnumber_flag from true to false in
// FeaturesGroup::read_feature_info_fromfile()!

   manual_FeaturesGroup_ptr->read_feature_info_from_file(
      manual_features_filename);
   manual_FeaturesGroup_ptr->write_feature_html_file(11); // Deer Island WISP

   photogroup_ptr->save_initial_camera_f_az_el_roll_params();

   int n_features=manual_FeaturesGroup_ptr->get_n_Graphicals();


   threevector init_camera_posn=photogroup_ptr->get_photograph_ptr(0)->
      get_camera_ptr()->get_world_posn();
   double init_azimuth=0;

// Initialize camera's altitude to height above sea level determined via
// horizon fitting:

   double z_wisp=19;	// meters above sea level
   init_camera_posn.put(2,z_wisp);
   cout << "init_camera_posn = " << init_camera_posn << endl;

   param_range camera_X(
      init_camera_posn.get(0)-10, init_camera_posn.get(0)+10,5);
   param_range camera_Y(
      init_camera_posn.get(1)-10, init_camera_posn.get(1)+10,5);
   param_range camera_Z(
      init_camera_posn.get(2)-5, init_camera_posn.get(2)+1,7);
//      init_camera_posn.get(2)-0.5, init_camera_posn.get(2)+1,5);
   param_range azimuth(-15*PI/180,17*PI/180,19);

// For WISP 360, focal length is already known exactly!

   double scalefactor=1.0;

   double curr_t=0;
   int n_panels=10;
   double min_projected_error_sum=POSITIVEINFINITY;

//   int n_iters=1;
   int n_iters=2;

   cout << "Enter number of search iterations to perform:" << endl;
   cin >> n_iters;
   for (int iter=0; iter<n_iters; iter++)
   {
      cout << "iter = " << iter << " of " << n_iters << endl;
      cout << "dX = " << camera_X.get_delta()
           << " dY = " << camera_X.get_delta()
           << " dZ = " << camera_Z.get_delta() << endl;
      cout << "d_az = " << azimuth.get_delta()*180/PI << endl << endl;

// ========================================================================
// Begin while loop over camera position parameters
// ========================================================================

      while (camera_X.prepare_next_value())
      {
         while (camera_Y.prepare_next_value())
         {
            while (camera_Z.prepare_next_value())
            {
               threevector camera_posn(camera_X.get_value(),
                  camera_Y.get_value(),camera_Z.get_value());
//               cout << "camera_posn = " << camera_posn << endl;

               while (azimuth.prepare_next_value())
               {
                  photogroup_ptr->restore_initial_camera_f_az_el_roll_params();

                  double projected_error_sum=0;
                  threevector XYZ,UVW,projected_UVW;
                  for (int f=0; f<n_features; f++)
                  {
                     Feature* curr_Feature_ptr=
                        manual_FeaturesGroup_ptr->get_Feature_ptr(f);

                     instantaneous_obs* obs_ptr=curr_Feature_ptr->
                        get_all_particular_time_observations(curr_t);
                     vector<int> curr_pass_numbers=obs_ptr->get_pass_numbers();
                     
                     int passnumber_2D=curr_pass_numbers.front();
                     int passnumber_3D=curr_pass_numbers.back();
//                     cout << "passnumber_2D = " << passnumber_2D << endl;
//                     cout << "passnumber_3D = " << passnumber_3D << endl;

                     curr_Feature_ptr->get_UVW_coords(
                        curr_t,passnumber_2D,UVW);
                     curr_Feature_ptr->get_UVW_coords(
                        curr_t,passnumber_3D,XYZ);
//                     cout << "UVW = " << UVW << endl;
//                     cout << "XYZ = " << XYZ << endl;

                     photograph* photo_ptr=photogroup_ptr->
                        get_photograph_ptr(passnumber_2D);
                     camera* camera_ptr=photo_ptr->get_camera_ptr();
                     camera_ptr->restore_initial_f_az_el_roll_params();
                     
                     double curr_az=camera_ptr->get_rel_az()+
                        azimuth.get_value();
                     double curr_el=camera_ptr->get_rel_el();
                     double curr_roll=camera_ptr->get_rel_roll();

                     camera_ptr->set_world_posn(camera_posn);
                     camera_ptr->set_Rcamera(curr_az,curr_el,curr_roll);
                     camera_ptr->construct_projection_matrix(false);

                     if (!camera_ptr->XYZ_in_front_of_camera(XYZ)) 
                     {
                        projected_error_sum += 1000;
                     }
                     else
                     {
                        camera_ptr->project_XYZ_to_UV_coordinates(
                           XYZ,projected_UVW);
                        threevector UVW_error=projected_UVW-UVW;
                        projected_error_sum += fabs(UVW_error.get(0))+
                           fabs(UVW_error.get(1));
                     }
                  } // loop over index f labeling feature tiepoints
                  
                  if (projected_error_sum < min_projected_error_sum)
                  {
                     min_projected_error_sum=projected_error_sum;
                     camera_X.set_best_value();
                     camera_Y.set_best_value();
                     camera_Z.set_best_value();
                     azimuth.set_best_value();
                     cout << "min_projected_error_sum = " 
                          << min_projected_error_sum << endl;

                     cout << " E = " << camera_X.get_best_value()
                          << " N = " << camera_Y.get_best_value()
                          << " Z = " << camera_Z.get_best_value()
                          << " az = " << azimuth.get_best_value()*180/PI
                          << endl << endl;
                  }

               } // azimuth while loop
            } // camera_Z while loop
         } // camera_Y while loop
      } // camera_X while loop

// ========================================================================
// End while loop over camera position parameters
// ========================================================================

      double frac=0.66;
      camera_X.shrink_search_interval(camera_X.get_best_value(),frac);
      camera_Y.shrink_search_interval(camera_Y.get_best_value(),frac);
      camera_Z.shrink_search_interval(camera_Z.get_best_value(),frac);
      azimuth.shrink_search_interval(azimuth.get_best_value(),frac);
   } // loop over iter index

   threevector best_camera_posn(
      camera_X.get_best_value(),camera_Y.get_best_value(),
      camera_Z.get_best_value());
   double best_az=azimuth.get_best_value();
   double best_el=0;
   double best_roll=0;

   cout << "min_projected_error_sum = " << min_projected_error_sum << endl;
   cout << "Best camera_X value = " << best_camera_posn.get(0) << endl;
   cout << "Best camera_Y value = " << best_camera_posn.get(1) << endl;
   cout << "Best camera_Z value = " << best_camera_posn.get(2) << endl;
   cout << "Best az_global = " << best_az*180/PI << endl;
   cout << "Best el_global = " << best_el*180/PI << endl;
   cout << "Best roll_global = " << best_roll*180/PI << endl;

   outputfunc::enter_continue_char();

// Force best_el=best_roll=0 in order to (approximately) align
// modified WISP panels with physical horizon:

   rotation best_R_global;
   best_R_global=best_R_global.rotation_from_az_el_roll(
      best_az,best_el,best_roll);
   cout << "best_R_global = " << best_R_global << endl;

// Before writing out best fit package files for each of the WISP
// cameras' panels, restore focal and rotation parameters to their
// initial values and then recompute them based upon best_scalefactor
// and best_R_global:

   photogroup_ptr->restore_initial_camera_f_az_el_roll_params();
   photogroup_ptr->globally_reset_camera_world_posn(best_camera_posn);
   photogroup_ptr->globally_rotate(best_R_global);

// Note added on 2/8/09: When we globally rescale and rotate
// imagespace rays onto worldspace rays, photographs are not
// reordered.  So photos_ordered_flag is set equal to false below:
 
   bool photos_ordered_flag=false;
   double frustum_sidelength=10;	// meters
//   double frustum_sidelength=25;	// meters
   photogroup_ptr->export_photo_parameters(
      packages_subdir,photos_ordered_flag,frustum_sidelength);

}
