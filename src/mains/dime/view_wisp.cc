// ========================================================================
// Program VIEW_WISP is a variant of program NEW_FOV .  It's a testing
// lab for instantiating, manipulating and displaying OBSFRUSTA.  It
// also stands alone from any point cloud input.

/*

/home/cho/programs/c++/svn/projects/src/mains/dime/view_wisp \
--region_filename /home/cho/programs/c++/svn/projects/src/mains/photosynth/bundler/DIME/packages/panels/wisp_p0_res0_000.pkg \
--region_filename /home/cho/programs/c++/svn/projects/src/mains/photosynth/bundler/DIME/packages/panels/wisp_p1_res0_000.pkg \
--region_filename /home/cho/programs/c++/svn/projects/src/mains/photosynth/bundler/DIME/packages/panels/wisp_p2_res0_000.pkg \
--region_filename /home/cho/programs/c++/svn/projects/src/mains/photosynth/bundler/DIME/packages/panels/wisp_p3_res0_000.pkg \
--region_filename /home/cho/programs/c++/svn/projects/src/mains/photosynth/bundler/DIME/packages/panels/wisp_p4_res0_000.pkg \
--region_filename /home/cho/programs/c++/svn/projects/src/mains/photosynth/bundler/DIME/packages/panels/wisp_p5_res0_000.pkg \
--region_filename /home/cho/programs/c++/svn/projects/src/mains/photosynth/bundler/DIME/packages/panels/wisp_p6_res0_000.pkg \
--region_filename /home/cho/programs/c++/svn/projects/src/mains/photosynth/bundler/DIME/packages/panels/wisp_p7_res0_000.pkg \
--region_filename /home/cho/programs/c++/svn/projects/src/mains/photosynth/bundler/DIME/packages/panels/wisp_p8_res0_000.pkg \
--region_filename /home/cho/programs/c++/svn/projects/src/mains/photosynth/bundler/DIME/packages/panels/wisp_p9_res0_000.pkg \
--initial_mode Manipulate_Fused_Data_Mode

*/


// ========================================================================
// Last updated on 2/25/13
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
#include "osg/osgClipping/Clipping.h"
#include "osg/osgClipping/ClippingKeyHandler.h"
#include "color/colorfuncs.h"
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
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "osg/osgGeometry/PolygonsGroup.h"
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
   string image_list_filename=passes_group.get_image_list_filename();
//   cout << " image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
//   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;

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

//   bool display_movie_state=false;
   bool display_movie_state=true;
//   bool display_movie_number=false;
   bool display_movie_number=true;
   bool display_movie_world_time=false;
//   bool display_movie_world_time=true;
   bool hide_Mode_HUD_flag=false;
//   bool hide_Mode_HUD_flag=true;
   Operations operations(
      ndims,window_mgr_ptr,passes_group,
      display_movie_state,display_movie_number,display_movie_world_time,
      hide_Mode_HUD_flag);
   root->addChild(operations.get_OSGgroup_ptr());

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();

// Instantiate AnimationController which acts as master clock:

   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   AnimationController_ptr->setDelay(0.00);
//   AnimationController_ptr->setExtendedDelay(
//      5*AnimationController_ptr->getDelay());

// Instantiate clock pointer to keep track of real time:

   Clock* clock_ptr=AnimationController_ptr->get_clock_ptr();
   clock_ptr->current_local_time_and_UTC();

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
   double max_X=2000;
   double min_Y=0;
   double max_Y=2000;
   double min_Z=0;

   cout << "min_X = " << min_X << " max_X = " << max_X << endl;
   cout << "min_Y = " << min_Y << " max_Y = " << max_Y << endl;
   AlirtGrid* grid_ptr=decorations.add_AlirtGrid(
      ndims,passes_group.get_pass_ptr(0),
      min_X,max_X,min_Y,max_Y,min_Z);

   threevector* grid_origin_ptr=grid_ptr->get_world_origin_ptr();
   CM_3D_ptr->set_Grid_ptr(grid_ptr);
   decorations.set_grid_origin_ptr(grid_origin_ptr);

   grid_ptr->set_axes_labels("Relative X (Meters)","Relative Y (Meters)");
   grid_ptr->set_delta_xy(200,200);
   grid_ptr->set_axis_char_label_size(50.0);
   grid_ptr->set_tick_char_label_size(50.0);
   grid_ptr->set_curr_color(colorfunc::purple);

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
//   threevector camera_posn=*grid_origin_ptr+threevector(100,100,0);
   threevector camera_posn=*grid_origin_ptr+threevector(1000,1000,0);

   double frustum_sidelength=-1;
   double movie_downrange_distance=-1;
   bool multicolor_frusta_flag=false;
   OBSFRUSTAGROUP_ptr->generate_still_imagery_frusta_for_photogroup(
      photogroup_ptr,photogroup_ptr->get_n_photos()-1,camera_posn,
      frustum_sidelength,movie_downrange_distance,multicolor_frusta_flag);

// Set mask_nonselected_OSGsubPATs_flag=false in order to
// simultaneously display OSGsubPATs 0 and 1 respectively containing
// static panorama and dynamic video images:

   decorations.get_OBSFRUSTUMPickHandler_ptr()->
      set_mask_nonselected_OSGsubPATs_flag(false);
   
// Retrieve correlated Movie IDs, slice numbers and thumbnail
// filenames from photos table of plume database: 

   string panel_images_subdir=bundler_IO_subdir+"images/panels/";
   vector<string> panel_image_filenames=
      filefunc::image_files_in_subdir(panel_images_subdir);
   int n_panels=10;
   int n_frames=panel_image_filenames.size()/n_panels;
   AnimationController_ptr->set_nframes(n_frames);
   cout << "n_frames = " << n_frames << endl;
   cout << "panel_image_filenames.size() = " << panel_image_filenames.size()
        << endl;

   MoviesGroup* MoviesGroup_ptr=OBSFRUSTAGROUP_ptr->get_MoviesGroup_ptr();
   
   int starting_frame_number=0;
   int stopping_frame_number=n_frames-1;

// Insert correlated movie IDs, frame numbers and photo filenames into
// MoviesGroup STL map:

   int image_index=0;
   vector<int> Movie_IDs,relative_frame_numbers;
   for (int OBSFRUSTUM_ID=0; OBSFRUSTUM_ID <= n_panels-1; OBSFRUSTUM_ID++)
   {
      for (int frame_number=starting_frame_number; frame_number <=
              stopping_frame_number; frame_number++)
      {
//         Movie_IDs.push_back(OBSFRUSTUM_ID);
//         relative_frame_numbers.push_back(frame_number-starting_frame_number);
         int rel_frame_number=frame_number-starting_frame_number;
         MoviesGroup_ptr->insert_photo_filename_into_map(
            OBSFRUSTUM_ID,rel_frame_number,panel_image_filenames[image_index]);
         image_index++;
      } // loop over frame_number
   } // loop over OBSFRUSTUM_ID

/*
   for (int i=0; i<panel_image_filenames.size(); i++)
   {
      cout << "i = " << i
           << " Movie ID = " << Movie_IDs[i]
           << " relative frame number = " << relative_frame_numbers[i]
           << " panel_image = " << panel_image_filenames[i] 
           << endl;
      MoviesGroup_ptr->insert_photo_filename_into_map(
         Movie_IDs[i],relative_frame_numbers[i],panel_image_filenames[i]);
   }
*/

/*
   for (int i=0; i<OBSFRUSTAGROUP_ptr->get_n_Graphicals(); i++)
   {
      double U=0.5;
      double V=0.5;
      double magnitude=50;
      OBSFRUSTAGROUP_ptr->draw_ray_through_imageplane(
         i,twovector(U,V),magnitude,colorfunc::get_color(i));
   } // loop over index i labeling OBSFRUSTA
*/

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

// Instantiate Polyhedra, Polygons & PolyLines decoration groups:

   PolyhedraGroup* PolyhedraGroup_ptr=decorations.add_Polyhedra(
      passes_group.get_pass_ptr(0));

   osgGeometry::PolygonsGroup* PolygonsGroup_ptr=decorations.add_Polygons(
      ndims,passes_group.get_pass_ptr(0),NULL);

   osgGeometry::PolygonsGroup* Clipped_PolygonsGroup_ptr=
      decorations.add_Polygons(
         ndims,passes_group.get_pass_ptr(0),NULL);

   PolyLinesGroup* PolyLinesGroup_ptr=decorations.add_PolyLines(
      ndims,passes_group.get_pass_ptr(0));
   PolyLinesGroup_ptr->set_width(2);

   PolyLinesGroup* Clipped_PolyLinesGroup_ptr=decorations.add_PolyLines(
      ndims,passes_group.get_pass_ptr(0));
   Clipped_PolyLinesGroup_ptr->set_width(20);

   Clipping* Clipping_ptr=new Clipping(OBSFRUSTAGROUP_ptr);
   Clipping_ptr->set_PolygonsGroup_ptr(PolygonsGroup_ptr);
   Clipping_ptr->set_Clipped_PolygonsGroup_ptr(Clipped_PolygonsGroup_ptr);

   Clipping_ptr->set_PolyhedraGroup_ptr(PolyhedraGroup_ptr);

   Clipping_ptr->set_PolyLinesGroup_ptr(PolyLinesGroup_ptr);
   Clipping_ptr->set_Clipped_PolyLinesGroup_ptr(Clipped_PolyLinesGroup_ptr);

   ClippingKeyHandler* ClippingKeyHandler_ptr=new ClippingKeyHandler(
      Clipping_ptr,ModeController_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(ClippingKeyHandler_ptr);

 
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

