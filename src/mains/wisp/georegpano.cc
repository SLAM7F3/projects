// ========================================================================
// Program GEOREGPANO reads in package files for 5 WISP panoramic
// video panels containing reasonable initial estimates for the
// cameras' calibration parameters.  For example, here are the
// contents of an initial package file for panel0:

/*
/home/cho/programs/c++/svn/projects/src/mains/photosynth/bundler/wisp/images/panels/frame_p0_0000.png
--Uaxis_focal_length -2.789730163
--Vaxis_focal_length -2.789730163
--U0 0.90909
--V0 0.5
--relative_az -31.50603991
--relative_el -0.3835759755
--relative_roll -0.8453735472
--camera_x_posn 312108.1549
--camera_y_posn 4703878.117
--camera_z_posn 19.06621218
--frustum_sidelength 25

*/

// It also reads in manually established tiepoints selected between a
// ladar point cloud and the WISP panels.  After iterating over possible
// WISP camera position threevectors, GEOREGPANO computes 
// the global rotation which maps the rescaled image space ray
// constellation onto the ladar rays in absolute world space
// coordinates.  After a georegistered set of translated and rotated 
// package files are written to disk, they can be directly
// imported into program PANOVSLADAR in order to view the WISP
// panoramas a set of 3D OBSFRUSTA against the ladar point cloud
// background.

/* 
./georegpano \
--region_filename ./packages/calib/frame_p0_0000.pkg \
--region_filename ./packages/calib/frame_p1_0000.pkg \
--region_filename ./packages/calib/frame_p2_0000.pkg \
--region_filename ./packages/calib/frame_p3_0000.pkg \
--region_filename ./packages/calib/frame_p4_0000.pkg \
--region_filename ./packages/calib/frame_p5_0000.pkg \
--region_filename ./packages/calib/frame_p6_0000.pkg \
--region_filename ./packages/calib/frame_p7_0000.pkg \
--region_filename ./packages/calib/frame_p8_0000.pkg \
--region_filename ./packages/calib/frame_p9_0000.pkg \
--region_filename ./packages/calib/dummy.pkg \
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

// ========================================================================
// Last updated on 1/17/11; 5/30/11; 6/4/11; 11/22/11
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
#include "optimum/optimizer.h"
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
   optimizer* optimizer_ptr=new optimizer(photogroup_ptr);
   optimizer_ptr->print_camera_parameters(n_photos);

// Instantiate FeaturesGroup to hold small number of manually
// extracted ladar and corresponding panoramic stills features:

   FeaturesGroup* manual_FeaturesGroup_ptr=new FeaturesGroup(
      ndims,passes_group.get_pass_ptr(n_passes-1),CM_3D_ptr);
//   string manual_features_subdir="./features/manually_selected/";
//   string manual_features_filename=manual_features_subdir+
//      "features_manual_combined.txt";
   string manual_features_subdir="./features/Nov11_selected/";
   string manual_features_filename=manual_features_subdir+
      "features_combined_wisp.txt";
//      "features_renamed_wisp.txt";

   manual_FeaturesGroup_ptr->read_feature_info_from_file(
      manual_features_filename);
   manual_FeaturesGroup_ptr->write_feature_html_file(7);

   exit(-1);

   optimizer_ptr->extract_manual_feature_info(manual_FeaturesGroup_ptr);
   photogroup_ptr->save_initial_camera_f_az_el_roll_params();

   threevector init_camera_posn=photogroup_ptr->get_photograph_ptr(0)->
      get_camera_ptr()->get_world_posn();
   param_range camera_X(
      init_camera_posn.get(0)-5, init_camera_posn.get(0)+5,5);
//      init_camera_posn.get(0)-10, init_camera_posn.get(0)+10,5);
   param_range camera_Y(
      init_camera_posn.get(1)-5, init_camera_posn.get(1)+5,5);
//      init_camera_posn.get(1)-10, init_camera_posn.get(1)+10,5);

   param_range camera_Z(
      init_camera_posn.get(2)-2, init_camera_posn.get(2)+2,5);
//      init_camera_posn.get(2)-3, init_camera_posn.get(2)+3,5);

   double min_delta_thetas_mu=POSITIVEINFINITY;
   double min_delta_thetas_sigma=POSITIVEINFINITY;
   double scalefactor,best_scalefactor;
   rotation R_global,best_R_global;

//   int n_iters=1;
   int n_iters=2;
//   int n_iters=5;
//   int n_iters=8;
//   int n_iters=10;

   cout << "Enter number of search iterations to perform:" << endl;
   cin >> n_iters;
   for (int iter=0; iter<n_iters; iter++)
   {
      cout << "iter = " << iter << " of " << n_iters << endl;

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
//                cout << "camera_posn = " << camera_posn << endl;
            
               photogroup_ptr->restore_initial_camera_f_az_el_roll_params();

/*
               optimizer_ptr->compute_world_and_imagespace_feature_rays(
                  camera_posn);
               scalefactor=optimizer_ptr->
                  compute_scalefactor_between_world_and_imagespace_rays();
               cout << "scalefactor = " << scalefactor << endl;
*/

// For WISP 360, focal length is already known exactly!

               scalefactor=1.0;
//               photogroup_ptr->rescale_focal_lengths(scalefactor);

               optimizer_ptr->compute_world_and_imagespace_feature_rays(
                  camera_posn);
               double delta_thetas_mu,delta_thetas_sigma;
//               R_global=optimizer_ptr->
//                  compute_rotation_between_imagespace_rays_and_world_rays(
//                     delta_thetas_mu,delta_thetas_sigma);
               rotation R_global=optimizer_ptr->
                  compute_RANSAC_rotation_between_imagespace_rays_and_world_rays(
                     iter,camera_posn,delta_thetas_mu,delta_thetas_sigma);


               if (delta_thetas_mu < min_delta_thetas_mu)
               {
                  min_delta_thetas_mu=delta_thetas_mu;
                  min_delta_thetas_sigma=delta_thetas_sigma;
                  camera_X.set_best_value();
                  camera_Y.set_best_value();
                  camera_Z.set_best_value();
                  best_scalefactor=scalefactor;
                  best_R_global=R_global;
                  cout << "min_delta_thetas = " << min_delta_thetas_mu
                       << " deg " << endl;
               }

            } // camera_Z while loop
         } // camera_Y while loop
      } // camera_X while loop

      cout << "******************************************************" << endl;
      cout << "delta_theta = " << min_delta_thetas_mu << " +/- "
           << min_delta_thetas_sigma << endl;
      cout << "******************************************************" << endl;

      if (min_delta_thetas_mu < 0.01) break;

// ========================================================================
// End while loop over camera position parameters
// ========================================================================

//      double frac=0.45;
      double frac=0.55;
      camera_X.shrink_search_interval(camera_X.get_best_value(),frac);
      camera_Y.shrink_search_interval(camera_Y.get_best_value(),frac);
      camera_Z.shrink_search_interval(camera_Z.get_best_value(),frac);

//      outputfunc::enter_continue_char();

   } // loop over iter index

   threevector best_camera_posn(
      camera_X.get_best_value(),camera_Y.get_best_value(),
      camera_Z.get_best_value());
   double best_az,best_el,best_roll;
   best_R_global.az_el_roll_from_rotation(best_az,best_el,best_roll);

   cout << "min_delta_thetas_mu = " << min_delta_thetas_mu << endl;
   cout << "Best camera_X value = " << best_camera_posn.get(0) << endl;
   cout << "Best camera_Y value = " << best_camera_posn.get(1) << endl;
   cout << "Best camera_Z value = " << best_camera_posn.get(2) << endl;
   cout << "Best scalefactor = " << best_scalefactor << endl;
   cout << "Best az_global = " << best_az*180/PI << endl;
   cout << "Best el_global = " << best_el*180/PI << endl;
   cout << "Best roll_global = " << best_roll*180/PI << endl;

   outputfunc::enter_continue_char();

/*

Winter 2011 HAFB WISP panorama camera values:

min_delta_thetas_mu = 0.2886366665
Best camera_X value = 312086.6935
Best camera_Y value = 4703848.428
Best camera_Z value = 28.27242083
Best scalefactor = 1.031139555
Best az_global = -5.350742618
Best el_global = -11.63945172
Best roll_global = 0.5059067174

*/

// Before writing out best fit package files for each of the WISP
// cameras' panels, restore focal and rotation parameters to their
// initial values and then recompute them based upon best_scalefactor
// and best_R_global:

   photogroup_ptr->restore_initial_camera_f_az_el_roll_params();
   photogroup_ptr->rescale_focal_lengths(best_scalefactor);
   photogroup_ptr->globally_reset_camera_world_posn(best_camera_posn);
   photogroup_ptr->globally_rotate(best_R_global);

// Note added on 2/8/09: When we globally rescale and rotate
// imagespace rays onto worldspace rays, photographs are not
// reordered.  So photos_ordered_flag is set equal to false below:
 
   bool photos_ordered_flag=false;
   double frustum_sidelength=18;	// meters
//   double frustum_sidelength=25;	// meters

   string packages_subdir="./packages/";
   photogroup_ptr->export_photo_parameters(
      packages_subdir,photos_ordered_flag,frustum_sidelength);

}
