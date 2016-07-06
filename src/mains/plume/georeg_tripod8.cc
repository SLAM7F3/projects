// ========================================================================
// Program GEOREG_TRIPOD8 is a specialized utility that we wrote in
// order to calibrate some of the camera parameters for video camera
// sitting on tripod #8 for the Nov 2012 plume experiment in South
// Carolina.  Bundler failed to reasonably reconstruct the camera
// parameters for video camera #8.  So we have to resort to
// semi-automatically calibrating this particular camera.

// GEOREG_TRIPOD8 first imports a set of 2D/3D tiepoints pairs
// corresponding to the tops of some wooden posts which are visible
// within video camera #8.  It also loads in reasonable initial
// estimates for all parameters for camera #8.  This program then
// performs a brute-force search over focal length, azimuth angle,
// trans_X and trans_Y parameters for the quadruple which minimizes
// the discrepancy between projected 3D woodpost points and 2D
// counterparts.

// Using this program, we have found a reasonable (though certainly
// not perfect) set of camera parameters for tripod video #8.
// ========================================================================
// Last updated on 1/6/12
// ========================================================================

#include <iostream>
#include <string>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>

#include "osg/osgGraphicals/AnimationController.h"
#include "osg/Custom2DManipulator.h"
#include "osg/osgFeatures/FeaturesGroup.h"
#include "osg/ModeController.h"
#include "osg/ModeKeyHandler.h"
#include "osg/osgfuncs.h"
#include "osg/osgOperations/Operations.h"
#include "numerical/param_range.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgWindow/ViewerManager.h"

#include "general/outputfuncs.h"

using std::cout;
using std::endl;
using std::string;

// ==========================================================================
int main( int argc, char** argv )
{
// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments_2D(&argc,argv);
   osg::ArgumentParser arguments_3D(&argc,argv);
   osg::ArgumentParser arguments(&argc,argv);

   PassesGroup passes_group(&arguments);
   int videopass_ID=passes_group.get_videopass_ID();
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();

   string image_list_filename=passes_group.get_image_list_filename();
//   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
//   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string image_sizes_filename=passes_group.get_image_sizes_filename();
//   cout << "image_sizes_filename = " << image_sizes_filename << endl;

// Construct the viewers and instantiate ViewerManagers:

   WindowManager* window_mgr_2D_ptr=new ViewerManager();
   WindowManager* window_mgr_3D_ptr=new ViewerManager();

// Instantiate Operations object to handle mode, animation and image
// number control:

   bool display_movie_state=true;
   bool display_movie_number=true;
   Operations operations(2,window_mgr_2D_ptr,display_movie_state,
   display_movie_number);

   ModeController* ModeController_2D_ptr=operations.get_ModeController_ptr();
   ModeController* ModeController_3D_ptr=new ModeController();

// Add custom manipulators to the event handler list:

   osgGA::Custom2DManipulator* CM_2D_ptr = new 
      osgGA::Custom2DManipulator(ModeController_2D_ptr,window_mgr_2D_ptr);
   window_mgr_2D_ptr->set_CameraManipulator(CM_2D_ptr);

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_3D_ptr,window_mgr_3D_ptr);
   window_mgr_3D_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate feature decorations groups:

   FeaturesGroup* FeaturesGroup_2D_ptr=new FeaturesGroup(
      2,passes_group.get_pass_ptr(videopass_ID),CM_2D_ptr);
   string features_2D_filename="./notes/woodpost_tripod8_features_2D.txt";
   FeaturesGroup_2D_ptr->read_feature_info_from_file(features_2D_filename);

   FeaturesGroup* FeaturesGroup_3D_ptr=new FeaturesGroup(
      3,passes_group.get_pass_ptr(cloudpass_ID),CM_3D_ptr);
   string features_3D_filename="./notes/woodpost_features_3D.txt";
   FeaturesGroup_3D_ptr->read_feature_info_from_file(features_3D_filename);

   int n_features=FeaturesGroup_2D_ptr->get_n_Graphicals();

// Load 3D and 2D tiepoint feature coordinates into threevectors XYZ
// and UV:

   double curr_t=0;
   threevector curr_XYZ,curr_UVW;
   vector<threevector> XYZ;
   vector<twovector> UV;
   for (int f=0; f<n_features; f++)
   {
      Feature* curr_2D_feature_ptr=FeaturesGroup_2D_ptr->get_Feature_ptr(f);
      Feature* curr_3D_feature_ptr=FeaturesGroup_3D_ptr->get_Feature_ptr(f);
      curr_2D_feature_ptr->get_UVW_coords(curr_t,videopass_ID,curr_UVW);
      curr_3D_feature_ptr->get_UVW_coords(curr_t,cloudpass_ID,curr_XYZ);
      XYZ.push_back(curr_XYZ);
      UV.push_back(twovector(curr_UVW));
//      cout << "f = " << f
//           << " XYZ = " << XYZ.back()
//           << " UV = " << UV.back() << endl;
   } // loop over index f labeling Video tripod #8 features

 
// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group,image_sizes_filename);
   photogroup_ptr->set_bundler_IO_subdir(bundler_IO_subdir);
   int n_photos=photogroup_ptr->get_n_photos();
   cout << "n_photos = " << n_photos << endl;

   photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(0);
   cout << " photo_filename = " << photo_ptr->get_filename() << endl;

   camera* camera_ptr=photo_ptr->get_camera_ptr();
   double init_f=camera_ptr->get_fu();
   double init_az=camera_ptr->get_rel_az();
   double init_el=camera_ptr->get_rel_el();
   double init_roll=camera_ptr->get_rel_roll();
   threevector init_camera_posn=camera_ptr->get_world_posn();

   param_range camera_f(init_f-0.01,init_f+0.01,7);
   param_range camera_az(init_az-1*PI/180,init_az+1*PI/180,7);
   param_range camera_X(
      init_camera_posn.get(0)-1, init_camera_posn.get(0)+1,7);
   param_range camera_Y(
      init_camera_posn.get(1)-1, init_camera_posn.get(1)+1,7);

   int n_iters=10;
   double min_residual=POSITIVEINFINITY;
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
            threevector camera_posn(
               camera_X.get_value(),camera_Y.get_value(),
               init_camera_posn.get(2));
            
            while (camera_az.prepare_next_value())
            {
               while (camera_f.prepare_next_value())
               {
                  camera_ptr->set_world_posn(camera_posn);
                  camera_ptr->set_Rcamera(
                     camera_az.get_value(),init_el,init_roll);
                  camera_ptr->construct_projection_matrix();
            
                  double curr_residual=0;
                  double u_proj,v_proj;
                  for (int f=0; f<n_features; f++)
                  {
                     camera_ptr->project_XYZ_to_UV_coordinates(
                        XYZ[f].get(0),XYZ[f].get(1),XYZ[f].get(2),
                        u_proj,v_proj);
                     curr_residual += 
                        sqr(u_proj-UV[f].get(0))+sqr(v_proj-UV[f].get(1));
                  } // loop over index f labeling tiepoint features
            
                  if (curr_residual < min_residual)
                  {
                     min_residual=curr_residual;
                     camera_f.set_best_value();
                     camera_az.set_best_value();
                     camera_X.set_best_value();
                     camera_Y.set_best_value();

                     cout << "curr_f = " << camera_f.get_value()
                          << " camera_az = " << camera_az.get_value()*180/PI 
                          << endl;
                     cout << "camera_posn = " << camera_posn << endl;
                     cout << "curr_residual = " << curr_residual << endl;
                  }

               } // camera_f while loop
            } // camera_az while loop
         } // camera_Y while loop
      } // camera_X while loop

// ========================================================================
// End while loop over camera position parameters
// ========================================================================

      double frac=0.66;
      camera_f.shrink_search_interval(camera_f.get_best_value(),frac);
      camera_az.shrink_search_interval(camera_az.get_best_value(),frac);
      camera_X.shrink_search_interval(camera_X.get_best_value(),frac);
      camera_Y.shrink_search_interval(camera_Y.get_best_value(),frac);
   } // loop over iter index


}
