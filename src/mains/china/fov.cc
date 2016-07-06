// ========================================================================
// Program FOV is a testing lab for instantiating, manipulating and
// displaying ObsFrusta.  It also stands alone from any point cloud
// input.

// 			fov Shanghai_05.vid

//		 fov Shanghai_05.vid --newpass Shanghai_04.vid

//   fov --region_filename shanghai_06.pkg --region_filename shanghai_05.pkg --region_filename shanghai_04.pkg

//   fov --region_filename shanghai_04.pkg --region_filename shanghai_05.pkg > fov.out 

/*

    fov --region_filename ricoh29.pkg --region_filename ricoh30.pkg \
   --region_filename ricoh31.pkg --region_filename ricoh32.pkg \
   --region_filename ricoh33.pkg --region_filename ricoh34.pkg \
   --region_filename ricoh35.pkg --region_filename ricoh36.pkg \
   --region_filename ricoh37.pkg --region_filename ricoh38.pkg 

*/

// ========================================================================
// Last updated on 6/17/07; 8/20/07; 9/21/07; 9/26/07; 10/11/07
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgText/Text>

#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgGrid/AlirtGridsGroup.h"
#include "osg/osgGraphicals/AnimationController.h"
#include "video/camera.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "osg/osgSceneGraph/ColorMap.h"
#include "math/constant_vectors.h"
#include "osg/osgOrganization/Decorations.h"
#include "general/filefuncs.h"
#include "math/mathfuncs.h"
#include "osg/ModeController.h"
#include "osg/ModeKeyHandler.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osgfuncs.h"
#include "passes/PassesGroup.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "geometry/polyline.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
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
   vector<int> videopass_ID;
   for (unsigned int n=0; n<passes_group.get_n_passes(); n++)
   {
      if (passes_group.get_pass_ptr(n)->get_passtype()==Pass::video)
      {
         videopass_ID.push_back(n);
         cout << "n = " << n << " videopass_ID = " << videopass_ID.back()
              << endl;
      }
   }

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("3D imagery");
//   window_mgr_ptr->getUsage(*arguments.getApplicationUsage());

// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate a mode controller and key event handler:

   ModeController* ModeController_ptr=new ModeController();
   window_mgr_ptr->get_EventHandlers_ptr()->push_back( 
      new ModeKeyHandler(ModeController_ptr) );
   root->addChild(osgfunc::create_Mode_HUD(ndims,ModeController_ptr));

// Instantiate animation controller & key handler:

   int n_animation_frames=1;
   AnimationController* AnimationController_ptr=new AnimationController(
      n_animation_frames);
   root->addChild(AnimationController_ptr->get_OSGgroup_ptr());

// Add a custom manipulator to the event handler list:

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold all decorations:
   
   Decorations decorations(
      window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

// Instantiate AlirtGrid decorations group:

   double min_X=0;
   double max_X=100;
   double min_Y=0;
   double max_Y=100;
   double min_Z=0;

   cout << "min_X = " << min_X << " max_X = " << max_X << endl;
   cout << "min_Y = " << min_Y << " max_Y = " << max_Y << endl;
   AlirtGrid* grid_ptr=decorations.add_AlirtGrid(
      ndims,passes_group.get_pass_ptr(videopass_ID.front()),
      min_X,max_X,min_Y,max_Y,min_Z);

   threevector* grid_origin_ptr=grid_ptr->get_world_origin_ptr();
   CM_3D_ptr->set_Grid_ptr(grid_ptr);
   decorations.set_grid_origin_ptr(grid_origin_ptr);

   cout << "*grid_origin_ptr = " << *grid_origin_ptr << endl;

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

   CentersGroup centers_group(
      ndims,passes_group.get_pass_ptr(videopass_ID.front()));
   CenterPickHandler* CenterPickHandler_ptr=new CenterPickHandler(
      ndims,passes_group.get_pass_ptr(videopass_ID.front()),
      CM_3D_ptr,&centers_group,ModeController_ptr,window_mgr_ptr,
      grid_origin_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      CenterPickHandler_ptr);

// Instantiate ObsFrusta decoration group:

   decorations.add_ObsFrusta(
      passes_group.get_pass_ptr(videopass_ID.front()),
      AnimationController_ptr,CM_3D_ptr);

// Instantiate an individual ObsFrustum for every input video.
// Each contains a separate movie object.

   vector<string> video_filename;
   vector<ObsFrustum*> ObsFrusta_ptrs;

   for (unsigned int n=0; n<passes_group.get_n_passes(); n++)
   {
      video_filename.push_back(passes_group.get_pass_ptr(n)->
                               get_first_filename());
      cout << "n = " << n << " video_filename = " << video_filename.back()
           << endl;

      ObsFrusta_ptrs.push_back(
         decorations.get_ObsFrustaGroup_ptr()->generate_movie_ObsFrustum(
            video_filename.back()));

      ObsFrustum* ObsFrustum_ptr=ObsFrusta_ptrs.back();
      ObsFrustum_ptr->set_color(colorfunc::get_OSG_color(
         colorfunc::get_color(n)));

// Manipulate camera corresponding to ObsFrustum's movie:

      camera* camera_ptr=ObsFrustum_ptr->get_Movie_ptr()->get_camera_ptr();

//      double f=-1.5;
      double f=passes_group.get_pass_ptr(n)->get_PassInfo_ptr()->
         get_focal_length();
//      cout << "Enter focal length parameter f:" << endl;
//      cin >> f;
//      cout << "focal length = " << f << endl;

      camera_ptr->set_internal_params(
         f,f,camera_ptr->get_u0(),camera_ptr->get_v0());

      threevector camera_posn=*grid_origin_ptr+threevector(0,0,50);
      camera_ptr->set_world_posn(camera_posn);
      cout << "camera posn = " << camera_posn << endl;

      camera_ptr->construct_projection_matrix();

      double downrange_distance=50;
      double movie_downrange_distance=downrange_distance;

      double curr_t=0;
      int pass_number=0;
      double z_offset=0;
      ObsFrustum_ptr->build_frustum_with_movie(
         curr_t,pass_number,z_offset,
         movie_downrange_distance,downrange_distance);

      double az=passes_group.get_pass_ptr(n)->get_PassInfo_ptr()->
         get_relative_az();
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

      az *= PI/180;
      el *= PI/180;
      roll *= PI/180;

      threevector abs_posn=*grid_origin_ptr+threevector(60,60,50);
      ObsFrustum_ptr->absolute_position_and_orientation(
         curr_t,pass_number,abs_posn,az,el,roll);

   } // loop over index n labeling video passes

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

