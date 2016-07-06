// ========================================================================
// Program RAYS is a testing lab for reading in 2D features and
// converting them to 3D rays for individual calibrated photos.

//	rays --region_filename ./packages/dscf8271.pkg \
//	--region_filename ./packages/lobby7_3D.pkg \
//	--initial_mode Manipulate_Fused_Data_Mode


// ========================================================================
// Last updated on 1/28/09; 1/29/09; 1/30/09; 2/5/09
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
#include "video/camera.h"
#include "math/constant_vectors.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/ModeController.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgWindow/ViewerManager.h"

using std::cin;
using std::cout;
using std::endl;
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

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group);
   int n_photos=photogroup_ptr->get_n_photos();

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("3D imagery");

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

// Instantiate group to hold all decorations:
   
   Decorations decorations(window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

// Instantiate AlirtGrid decorations group:

//   threevector camera_XYZ(327532.1, 4691760.7, 21.7);
   threevector camera_XYZ(327532.1, 4691760.7, 2.6);	
		// Z value extracted from 2005 Alirt Boston point cloud

   double min_X=camera_XYZ.get(0)-100;
   double max_X=camera_XYZ.get(0)+100;
   double min_Y=camera_XYZ.get(1)-100;
   double max_Y=camera_XYZ.get(1)+100;
   double min_Z=camera_XYZ.get(2)-10;

//   double min_X=0;
//   double max_X=150;
//   double min_Y=0;
//   double max_Y=150;
//   double min_Z=0;

   cout << "min_X = " << min_X << " max_X = " << max_X << endl;
   cout << "min_Y = " << min_Y << " max_Y = " << max_Y << endl;
   AlirtGrid* grid_ptr=decorations.add_AlirtGrid(
      ndims,passes_group.get_pass_ptr(0),
      min_X,max_X,min_Y,max_Y,min_Z);

   threevector* grid_origin_ptr=grid_ptr->get_world_origin_ptr();
   CM_3D_ptr->set_Grid_ptr(grid_ptr);
   decorations.set_grid_origin_ptr(grid_origin_ptr);

   grid_ptr->set_axes_labels("X (Meters)","Y (Meters)");
//   grid_ptr->set_delta_xy(10,10);
   grid_ptr->set_delta_xy(20,20);
   grid_ptr->set_axis_char_label_size(5.0);
   grid_ptr->set_tick_char_label_size(5.0);
   grid_ptr->update_grid();

// Instantiate OBSFRUSTAGROUP decoration group:

   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr=decorations.add_OBSFRUSTA(
      passes_group.get_pass_ptr(0),AnimationController_ptr);

// Instantiate an individual OBSFRUSTUM for every input photo.  Each
// contains a separate movie object.

  OBSFRUSTAGROUP_ptr->generate_still_imagery_frusta_for_photogroup(
      photogroup_ptr);

// Set mask_nonselected_OSGsubPATs_flag=false in order to
// simultatneously display OSGsubPATs 0 and 1 respectively containing
// static panorama and dynamic video images:

   decorations.get_OBSFRUSTUMPickHandler_ptr()->
      set_mask_nonselected_OSGsubPATs_flag(false);

// Instantiate FeaturesGroup decoration group to hold photo features:

   FeaturesGroup* FeaturesGroup_ptr=decorations.add_Features(
      ndims,passes_group.get_pass_ptr(0));
   string features_subdir="./features/Lobby7/manually_selected/";
   FeaturesGroup_ptr->read_in_photo_features(photogroup_ptr,features_subdir);
   FeaturesGroup_ptr->convert_2D_coords_to_3D_rays(photogroup_ptr);

   bool output_only_multicoord_features_flag=false;
   bool output_3D_rays_flag=true;
   FeaturesGroup_ptr->write_feature_html_file(
      photogroup_ptr,output_only_multicoord_features_flag,
      output_3D_rays_flag);

// Instantiate 2nd FeaturesGroup to hold ladar features:

   FeaturesGroup* FeaturesGroup_ladar_ptr=new FeaturesGroup(
      ndims,passes_group.get_pass_ptr(1),CM_3D_ptr,grid_origin_ptr);
   string features_filename=features_subdir+"features_XXX.txt";
   FeaturesGroup_ladar_ptr->read_feature_info_from_file(features_filename);
   FeaturesGroup_ladar_ptr->write_feature_html_file();
   
   vector<threevector> XYZ_ray_bundle;
   for (int f=0; f<FeaturesGroup_ladar_ptr->get_n_Graphicals(); f++)
   {
      Feature* feature_ptr=FeaturesGroup_ladar_ptr->get_Feature_ptr(f);
      threevector XYZ;
      feature_ptr->get_UVW_coords(
         FeaturesGroup_ladar_ptr->get_curr_t(),
         FeaturesGroup_ladar_ptr->get_passnumber(),XYZ);
      threevector rel_XYZ=XYZ-camera_XYZ;
      XYZ_ray_bundle.push_back(rel_XYZ.unitvector());
      cout << "f = " << f 
           << " rel_XYZ.mag = " << rel_XYZ.magnitude()
           << " rel_XYZ.hat = " << XYZ_ray_bundle.back() << endl;
   }

   int ray_passnumber=1;
   vector<threevector> UVW_ray_bundle;
   for (int f=0; f<FeaturesGroup_ptr->get_n_Graphicals(); f++)
   {
      Feature* feature_ptr=FeaturesGroup_ptr->get_Feature_ptr(f);
      threevector UVW;
      feature_ptr->get_UVW_coords(
         FeaturesGroup_ptr->get_curr_t(),ray_passnumber,UVW);
      UVW_ray_bundle.push_back(UVW);
      cout << "f = " << f 
           << " UVW_ray = " << UVW_ray_bundle.back() << endl;
   }
   
   rotation R_UVW_to_XYZ;
   R_UVW_to_XYZ.rotation_between_ray_bundles(
      XYZ_ray_bundle,UVW_ray_bundle);
   cout << "R_UVW_to_XYZ = " << R_UVW_to_XYZ << endl;

// R_UVW_to_XYZ = 
// 	0.367471        -0.929124       -0.0411664
//	0.914218        0.368996        -0.167475
//	0.170795        0.023907        0.985017

// delta_az = 68.1023005205 delta_el = 9.83404758968 delta_roll = 1.3903355349

   double delta_az,delta_el,delta_roll;
   R_UVW_to_XYZ.az_el_roll_from_rotation(
      delta_az,delta_el,delta_roll);
   cout.precision(12);
   cout << "delta_az = " << delta_az*180/PI 
        << " delta_el = " << delta_el*180/PI
        << " delta_roll = " << delta_roll*180/PI << endl;

   for (int f=0; f<UVW_ray_bundle.size(); f++)
   {
      threevector UVW=UVW_ray_bundle[f];
      threevector transformed_UVW=R_UVW_to_XYZ*UVW;
      cout << "f = " << f 
           << " transformed UVW_ray = " << transformed_UVW
           << " XYZ ray = " << XYZ_ray_bundle[f] << endl;
   }

   SignPostsGroup* SignPostsGroup_ptr=decorations.add_SignPosts(
      ndims,passes_group.get_pass_ptr(0));
   SignPostsGroup_ptr->set_common_geometrical_size(0.03);

   twovector UV0(0.5 , 0.5);
   SignPost* SignPost0_ptr=OBSFRUSTAGROUP_ptr->
      generate_SignPost_at_imageplane_location(UV0,0,SignPostsGroup_ptr);

   SignPost0_ptr->set_label("Building one");
//   double extra_frac_cyl_height=0.5;
//   SignPost_ptr->set_label("Building one",extra_frac_cyl_height);
//   SignPost0_ptr->set_max_text_width("Build");

// Optimize the scene graph, remove redundent nodes and states, and
// then attach it to the viewer:

   osgUtil::Optimizer optimizer;
   optimizer.optimize(root);
   root->addChild(decorations.get_OSGgroup_ptr());

   window_mgr_ptr->setSceneData(root);

// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_ptr->realize();

   while( !window_mgr_ptr->done() )
   {
      window_mgr_ptr->process();
   }

   delete window_mgr_ptr;
}

