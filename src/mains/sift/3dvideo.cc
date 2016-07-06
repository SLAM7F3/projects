// ========================================================================
// Program 3DVIDEO is a testing grounds for playing an OBSFRUSTUM
// movie against a static 3D panorama mosaic.
// ========================================================================
// Last updated on 1/18/09; 1/28/09; 1/31/09; 2/15/09; 2/19/09
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
   int n_photos=photogroup_ptr->get_n_photos();

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
//   bool hide_Mode_HUD_flag=true;
   bool hide_Mode_HUD_flag=false;
   Operations operations(
      ndims,window_mgr_ptr,passes_group,
      display_movie_state,display_movie_number,hide_Mode_HUD_flag);
//   Operations operations(ndims,window_mgr_ptr,passes_group);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   root->addChild(operations.get_OSGgroup_ptr());

// Add a custom manipulator to the event handler list:

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

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
   grid_ptr->update_grid();

// Instantiate a PointFinder;

   PointFinder pointfinder;
   CM_3D_ptr->set_PointFinder(&pointfinder);

// Instantiate CentersGroup and center pick handler to handle mid
// point selection:

   CentersGroup centers_group(ndims,passes_group.get_pass_ptr(0));
   CenterPickHandler* CenterPickHandler_ptr=new CenterPickHandler(
      ndims,passes_group.get_pass_ptr(0),
      CM_3D_ptr,&centers_group,ModeController_ptr,window_mgr_ptr,
      grid_origin_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      CenterPickHandler_ptr);

// Instantiate OBSFRUSTAGROUP decoration group:

   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr=decorations.add_OBSFRUSTA(
      passes_group.get_pass_ptr(0),AnimationController_ptr);

// As of 2/12/09, we hard-code geolocation of camera for Jan 6, 2009
// Lobby7 video sequence.  Should eventually read this parameter in
// via package file...

//   threevector camera_XYZ(327532.1, 4691760.7, 5.0);	
   threevector camera_posn=*grid_origin_ptr+threevector(80,85,50);
   cout << "camera_posn = " << camera_posn << endl;

// Instantiate an individual OBSFRUSTUM for every still panoramic
// image:

   double frustum_sidelength=55;
   double movie_downrange_distance=-1;
   OBSFRUSTAGROUP_ptr->generate_still_imagery_frusta_for_photogroup(
      photogroup_ptr,camera_posn,frustum_sidelength,movie_downrange_distance);

// Convert background panorama to greyscale:

   for (int n=0; n<OBSFRUSTAGROUP_ptr->get_n_Graphicals(); n++)
   {
      Movie* Movie_ptr=OBSFRUSTAGROUP_ptr->get_OBSFRUSTUM_ptr(n)->
         get_Movie_ptr();
      Movie_ptr->get_texture_rectangle_ptr()->
         convert_color_image_to_greyscale();
   } // loop over index n labeling OBSFRUSTA

// Instantiate dynamic OBSFRUSTA for video(s):

   double Zplane_altitude=0;	// meters
   bool multicolor_frusta_flag=false;
   bool initially_mask_all_frusta_flag=false;

   OBSFRUSTAGROUP_ptr->get_MoviesGroup_ptr()->
      set_static_camera_posn_offset(camera_posn);

   OBSFRUSTAGROUP_ptr->generate_still_imagery_frusta_from_projection_matrices(
      passes_group,camera_posn,Zplane_altitude,
      multicolor_frusta_flag,initially_mask_all_frusta_flag);

// Set mask_nonselected_OSGsubPATs_flag=false in order to
// simultaneously display OSGsubPATs 0 and 1 respectively containing
// static panorama and dynamic video images:

   decorations.get_OBSFRUSTUMPickHandler_ptr()->
      set_mask_nonselected_OSGsubPATs_flag(false);

// Optimize the scene graph, remove redundent nodes and states, and
// then attach it to the viewer:

   osgUtil::Optimizer optimizer;
   optimizer.optimize(root);
   root->addChild(decorations.get_OSGgroup_ptr());

   window_mgr_ptr->setSceneData(root);

// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_ptr->realize();

// Adjust viewer's field-of-view if some positive virtual_horiz_FOV
// parameter is passed an input argument:

   double virtual_horiz_FOV=passes_group.get_virtual_horiz_FOV();
   if (virtual_horiz_FOV > 0)
   {
      double FOV_h=window_mgr_ptr->get_lens_horizontal_FOV();
      cout << "virtual_horiz_FOV = " << virtual_horiz_FOV << endl;
      double angular_scale_factor=virtual_horiz_FOV/FOV_h;  
//   cout << "angular_scale_factor = " << angular_scale_factor << endl;
      window_mgr_ptr->rescale_viewer_FOV(angular_scale_factor);
   }
   
   while( !window_mgr_ptr->done() )
   {
      window_mgr_ptr->process();
   }

   delete window_mgr_ptr;
}

