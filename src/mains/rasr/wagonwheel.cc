// ========================================================================
// Program WAGONWHEEL is a variant of program NEW_FOV .  It's a
// testing lab for viewing 360 panoramas as 3D quasi-cylinders.

/*

/home/cho/programs/c++/svn/projects/src/mains/rasr/wagonwheel \
--region_filename pano1_0.pkg --region_filename pano1_1.pkg \
--region_filename pano1_2.pkg --region_filename pano1_3.pkg \
--region_filename pano1_4.pkg --region_filename pano1_5.pkg \
--region_filename pano1_6.pkg --region_filename pano1_7.pkg \
--region_filename pano1_8.pkg --region_filename pano1_9.pkg \
--initial_mode Manipulate_Fused_Data_Mode

*/

// ========================================================================
// Last updated on 11/20/09; 12/3/09; 4/23/10
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgText/Text>

#include "osg/osgGrid/AlirtGridsGroup.h"
#include "osg/osgGraphicals/AnimationController.h"
#include "video/camera.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "math/constant_vectors.h"
#include "osg/osgOrganization/Decorations.h"
#include "geometry/homography.h"
#include "osg/osgSceneGraph/HiresDataVisitor.h"
#include "osg/ModeController.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "geometry/polyline.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
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

   HiresDataVisitor* HiresDataVisitor_ptr=new HiresDataVisitor();

// Instantiate group to hold all decorations:
   
   Decorations decorations(
      window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

// Instantiate AlirtGrid decorations group:

   double min_X=0;
   double max_X=150;
   double min_Y=0;
   double max_Y=150;
   double min_Z=0;

   cout << "min_X = " << min_X << " max_X = " << max_X << endl;
   cout << "min_Y = " << min_Y << " max_Y = " << max_Y << endl;
   AlirtGrid* grid_ptr=decorations.add_AlirtGrid(
      ndims,passes_group.get_pass_ptr(0),
      min_X,max_X,min_Y,max_Y,min_Z);

   threevector* grid_origin_ptr=grid_ptr->get_world_origin_ptr();
   CM_3D_ptr->set_Grid_ptr(grid_ptr);
   decorations.set_grid_origin_ptr(grid_origin_ptr);

   grid_ptr->set_axes_labels("X (Meters)","Y (Meters)");
   grid_ptr->set_delta_xy(10,10);
   grid_ptr->set_axis_char_label_size(5.0);
   grid_ptr->set_tick_char_label_size(5.0);

   grid_ptr->set_HiresDataVisitor_ptr(HiresDataVisitor_ptr);
   grid_ptr->set_root_ptr(root);

// Instantiate a PointFinder;

   PointFinder pointfinder;
   CM_3D_ptr->set_PointFinder(&pointfinder);

// Instantiate OBSFRUSTAGROUP decoration group:

   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr=decorations.add_OBSFRUSTA(
      passes_group.get_pass_ptr(0),AnimationController_ptr);

// Instantiate an individual OBSFRUSTUM for every input video.  Each
// contains a separate movie object.

//   threevector camera_posn=*grid_origin_ptr;
   threevector camera_posn=*grid_origin_ptr+threevector(100,100,50);

   double frustum_sidelength=-1;
   double movie_downrange_distance=-1;
//   double frustum_sidelength=50;
//   double movie_downrange_distance=25;
//   double frustum_sidelength=800;
//   double movie_downrange_distance=600;

//   bool video_frustum_included_flag=false;
   bool video_frustum_included_flag=true;
   if (!video_frustum_included_flag)
   {
      OBSFRUSTAGROUP_ptr->generate_still_imagery_frusta_for_photogroup(
         photogroup_ptr,photogroup_ptr->get_n_photos(),camera_posn,
         frustum_sidelength,movie_downrange_distance);

      for (unsigned int n=0; n<OBSFRUSTAGROUP_ptr->get_n_Graphicals(); n++)
      {
         Movie* Movie_ptr=OBSFRUSTAGROUP_ptr->get_OBSFRUSTUM_ptr(n)->
            get_Movie_ptr();

         threevector n_hat(1,1,0);
         n_hat=n_hat.unitvector();
         threevector origin=*grid_origin_ptr+threevector(200,0,0);
         plane imageplane(n_hat,origin);

         Movie_ptr->warp_photo_onto_imageplane(imageplane);
      
      } // loop over index n labeling OBSFRUSTA
   }
   else
   {
      OBSFRUSTAGROUP_ptr->generate_still_imagery_frusta_for_photogroup(
         photogroup_ptr,photogroup_ptr->get_n_photos()-1,camera_posn,
         frustum_sidelength,movie_downrange_distance);

      for (unsigned int n=0; n<OBSFRUSTAGROUP_ptr->get_n_Graphicals(); n++)
      {

//         Movie* Movie_ptr=OBSFRUSTAGROUP_ptr->get_OBSFRUSTUM_ptr(n)->
//            get_Movie_ptr();
//         Movie_ptr->get_texture_rectangle_ptr()->
//            convert_color_image_to_greyscale();

// Write out new set of photograph package files for Lobby7 example
// where we transform all photos by approximate global rotation needed
// to bring them into alignment with 2007 JAUDIT MIT point cloud:

//      photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(n);
//      string filename_descriptor="_init_ext";
//      photograph_ptr->export_camera_parameters(
//         Movie_ptr->get_camera_ptr(),filename_descriptor);

      } // loop over index n labeling OBSFRUSTA
      
/*
      double Zplane_altitude=0;	// meters
      bool multicolor_frusta_flag=false;
      bool initially_mask_all_frusta_flag=false;
      OBSFRUSTAGROUP_ptr->
         generate_still_imagery_frusta_from_projection_matrices(
            passes_group,camera_posn,Zplane_altitude,
            multicolor_frusta_flag,initially_mask_all_frusta_flag);
*/

   } // !video_frustum_included_flag conditional
   
// Set mask_nonselected_OSGsubPATs_flag=false in order to
// simultatneously display OSGsubPATs 0 and 1 respectively containing
// static panorama and dynamic video images:

   decorations.get_OBSFRUSTUMPickHandler_ptr()->
      set_mask_nonselected_OSGsubPATs_flag(false);

/*
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

   twovector UV1(0.75 , 0.75);
   SignPost* SignPost1_ptr=OBSFRUSTAGROUP_ptr->
      generate_SignPost_at_imageplane_location(UV1,1,SignPostsGroup_ptr);
   SignPost1_ptr->set_label("Building two is a big structure");

//   double extra_frac_cyl_height=0.5;
//   SignPost_ptr->set_label("Building one",extra_frac_cyl_height);
//   SignPost1_ptr->set_max_text_width("Build");
*/

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

