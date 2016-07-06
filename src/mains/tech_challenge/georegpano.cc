// ========================================================================
// Program GEOREGPANO reads in package files for 5 D7 panoramic video
// panels containing reasonable initial estimates for the
// cameras' calibration parameters.  For example, here are the
// contents of an initial package file for panel0:

/*

/data/tech_challenge/field_tests/HAFB_mast_July2/frames/cropped_images/panels/cropped_image_p0-00003.png
--Uaxis_focal_length -1.31
--Vaxis_focal_length -1.31
--U0 0.42210
--V0 0.5
--relative_az  72
--relative_el   0
--relative_roll  0
--camera_x_posn 312088.1
--camera_y_posn 4703847.6
--camera_z_posn 27.4
--frustum_sidelength 60

*/

// It also reads in manually established tiepoints selected between a
// ladar point cloud and the D7 panels.  After iterating over possible
// D7 camera position threevectors, GEOREGPANO computes an angular
// scale factor by which the constellation of manually extracted image
// space feature rays needs to be multiplied by in order to match the
// angular extent spanned by the corresponding world space rays
// derived from the ladar point cloud.  GEOREGPANO also computes the
// global rotation which maps the rescaled image space ray
// constellation onto the ladar rays in absolute world space
// coordinates.  After a georegistered set of translated, rotated and
// rescaled package files are written to disk, they can be directly
// imported into program HALFWHEEL in order to view the D7 panorama as
// a set of 3D OBSFRUSTA against the ladar point cloud background.

/* 

./georegpano \
--region_filename ./packages/panel0.pkg \
--region_filename ./packages/panel1.pkg \
--region_filename ./packages/panel2.pkg \
--region_filename ./packages/panel3.pkg \
--region_filename ./packages/panel4.pkg \
--region_filename ./packages/dummy.pkg \
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

// ========================================================================
// Last updated on 1/17/11
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include "video/camera.h"
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

   string order_filename="./packages/panels_order.dat";
   photogroup_ptr->set_photo_order(order_filename);
   
   int n_photos(photogroup_ptr->get_n_photos());
   optimizer* optimizer_ptr=new optimizer(photogroup_ptr);
   optimizer_ptr->print_camera_parameters(n_photos);

// Instantiate FeaturesGroup to hold small number of manually
// extracted ladar and corresponding panoramic stills features:

   FeaturesGroup* manual_FeaturesGroup_ptr=new FeaturesGroup(
      ndims,passes_group.get_pass_ptr(n_passes-1),CM_3D_ptr);
   string manual_features_subdir="./features/D7_HAFB/manually_selected/";
   string manual_features_filename=manual_features_subdir+
      "features_manual_combined.txt";

   manual_FeaturesGroup_ptr->read_feature_info_from_file(
      manual_features_filename);
   manual_FeaturesGroup_ptr->write_feature_html_file(7);

   optimizer_ptr->extract_manual_feature_info(manual_FeaturesGroup_ptr);
   photogroup_ptr->save_initial_camera_f_az_el_roll_params();

   threevector init_camera_posn=photogroup_ptr->get_photograph_ptr(0)->
      get_camera_ptr()->get_world_posn();
   param_range camera_X(
      init_camera_posn.get(0)-3, init_camera_posn.get(0)+3,5);
   param_range camera_Y(
      init_camera_posn.get(1)-3, init_camera_posn.get(1)+3,5);
   param_range camera_Z(
      init_camera_posn.get(2)-3, init_camera_posn.get(2)+3,5);

   double min_delta_thetas_mu=POSITIVEINFINITY;
   double scalefactor,best_scalefactor;
   rotation R_global,best_R_global;

   int n_iters=10;
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

               optimizer_ptr->compute_world_and_imagespace_feature_rays(
                  camera_posn);
               scalefactor=optimizer_ptr->
                  compute_scalefactor_between_world_and_imagespace_rays();
//               cout << "scalefactor = " << scalefactor << endl;

               photogroup_ptr->rescale_focal_lengths(scalefactor);
            
               optimizer_ptr->compute_world_and_imagespace_feature_rays(
                  camera_posn);

               double delta_thetas_mu,delta_thetas_sigma;
               R_global=optimizer_ptr->
                  compute_rotation_between_imagespace_rays_and_world_rays(
                     delta_thetas_mu,delta_thetas_sigma);

               if (delta_thetas_mu < min_delta_thetas_mu)
               {
                  min_delta_thetas_mu=delta_thetas_mu;
                  camera_X.set_best_value();
                  camera_Y.set_best_value();
                  camera_Z.set_best_value();
                  best_scalefactor=scalefactor;
                  best_R_global=R_global;
               }

            } // camera_Z while loop
         } // camera_Y while loop
      } // camera_X while loop
   
      cout << "min_delta_thetas_mu = " << min_delta_thetas_mu << endl;

// ========================================================================
// End while loop over camera position parameters
// ========================================================================

      double frac=0.45;
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

/*

HAFB D7 panorama camera values:

min_delta_thetas_mu = 0.2886366665
Best camera_X value = 312086.6935
Best camera_Y value = 4703848.428
Best camera_Z value = 28.27242083
Best scalefactor = 1.031139555
Best az_global = -5.350742618
Best el_global = -11.63945172
Best roll_global = 0.5059067174

*/

// Before writing out best fit package files for each of the D7
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
   double frustum_sidelength=25;	// meters
   string packages_subdir="./packages/";
   photogroup_ptr->export_photo_parameters(packages_subdir,photos_ordered_flag,
      frustum_sidelength);

//   delete optimizer_ptr;
//   delete window_mgr_ptr;
}
