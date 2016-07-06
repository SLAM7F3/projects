// ========================================================================
// Program VIEWSAT is a specialized version of VIEWPOINTS intended for
// satellite point cloud viewing.

// 		 viewsat --region_filename ./packages/spase.pkg

// ========================================================================
// Last updated on 12/23/07; 12/24/07; 12/30/07; 1/14/08
// ========================================================================

#include <iostream>
#include <string>
#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CentersKeyHandler.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "color/colorfuncs.h"
#include "osg/Custom3DManipulator.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/ModeController.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "passes/TextDialogBox.h"
#include "osg/osgWindow/ViewerManager.h"

using std::cout;
using std::endl;
using std::ifstream;
using std::string;

// ==========================================================================
int main( int argc, char** argv )
{

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
   cout << "cloudpass_ID = " << cloudpass_ID << endl;
   int videopass_ID=passes_group.get_videopass_ID();
   cout << "videopass_ID = " << videopass_ID << endl;

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("3D imagery");
//   window_mgr_ptr->getUsage(*arguments.getApplicationUsage());

// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

   Operations operations(ndims,window_mgr_ptr);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   root->addChild(operations.get_OSGgroup_ptr());

// Add a custom manipulator to the event handler list:

   bool disable_rotations_flag=false;
   bool emulate_GoogleEarth_rotations=false;
   osgGA::Custom3DManipulator* CM_3D_ptr=new osgGA::Custom3DManipulator(
      ModeController_ptr,window_mgr_ptr,disable_rotations_flag,
      emulate_GoogleEarth_rotations);
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold all decorations:

   Decorations decorations(window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

// Instantiate AlirtGrid decorations group:

   AlirtGrid* grid_ptr=decorations.add_AlirtGrid(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));

   threevector* grid_origin_ptr=grid_ptr->get_world_origin_ptr();
   CM_3D_ptr->set_grid_origin_ptr(grid_origin_ptr);
   decorations.set_grid_origin_ptr(grid_origin_ptr);

// Instantiate group to hold pointcloud information:

   PointCloudsGroup clouds_group(
      passes_group.get_pass_ptr(cloudpass_ID),grid_origin_ptr);
   PointCloud* cloud_ptr=clouds_group.generate_new_Cloud();
   bool set_origin_to_zeroth_xyz_flag=false;
   PointCloudKeyHandler* PointCloudKeyHandler_ptr=
      new PointCloudKeyHandler(&clouds_group,ModeController_ptr,
                               set_origin_to_zeroth_xyz_flag);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      PointCloudKeyHandler_ptr);
//   root->addChild(clouds_group.get_OSGgroup_ptr());

// Instantiate a transformer in order to convert between screen and
// world space coordinate systems:

   PointFinder pointfinder(&clouds_group);
   CM_3D_ptr->set_PointFinder(&pointfinder);

// Instantiate a MyDatabasePager to handle paging of files from disk:

   viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager(
      clouds_group.get_SetupGeomVisitor_ptr(),
      clouds_group.get_ColorGeodeVisitor_ptr());

// Initialize ALIRT grid based upon cloud's bounding box:

//   const double x_magnification_factor=2.5;
//   const double y_magnification_factor=1.5;
   const double x_magnification_factor=2;	// OK for AK model
   const double y_magnification_factor=1.3;	// OK for AK model
   double min_X=x_magnification_factor*cloud_ptr->get_min_value(0);
   double max_X=x_magnification_factor*cloud_ptr->get_max_value(0);
   double min_Y=y_magnification_factor*cloud_ptr->get_min_value(1);
   double max_Y=y_magnification_factor*cloud_ptr->get_max_value(1);
   double min_Z=cloud_ptr->get_min_value(2);

   decorations.get_AlirtGridsGroup_ptr()->initialize_grid(
      grid_ptr,min_X,max_X,min_Y,max_Y,min_Z);

//   bool SPASE_flag=true;
   bool SPASE_flag=false;
   grid_ptr->initialize_satellite_grid(SPASE_flag);

/*
//   cout << "min_X = " << min_X << " max_X = " << max_X << endl;
//   cout << "min_Y = " << min_Y << " max_Y = " << max_Y << endl;

   grid_ptr->set_axes_labels("Relative Model X (meters)",
                             "Relative Model Y (meters)");
   grid_ptr->set_delta_xy(1,1);
   grid_ptr->set_axis_char_label_size(0.6);	// OK for AK model
   grid_ptr->set_tick_char_label_size(0.6);	// OK for AK model
//   grid_ptr->set_axis_char_label_size(1.0);
//   grid_ptr->set_tick_char_label_size(1.0);
   grid_ptr->update_grid();
*/

// Instantiate feature and planes decoration groups:

   decorations.add_Features(ndims,passes_group.get_pass_ptr(cloudpass_ID));
   decorations.add_Planes(passes_group.get_pass_ptr(cloudpass_ID));

// Instantiate PolyLines decoration group:

   PolyLinesGroup* PolyLinesGroup_ptr=decorations.add_PolyLines(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   PolyLinesGroup_ptr->set_width(8);

// Instantiate signposts, features, army symbol and sphere segments
// decoration groups:

   SignPostsGroup* SignPostsGroup_ptr=decorations.add_SignPosts(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));

   double size=0.25;
   double height_multiplier=4;
   if (!SPASE_flag)
   {
      size *= 3;
      height_multiplier *= 3;
   }
   
   SignPost* avg_sundir_SignPost_ptr=
      SignPostsGroup_ptr->generate_new_SignPost(
         size,height_multiplier,Zero_vector);

   threevector SignPost_base(1,1,-1);
   threevector SignPost_dir(0.9255, -0.1502, -0.3477);
   SignPost_dir=SignPost_dir.unitvector();
   
   avg_sundir_SignPost_ptr->reset_attitude_posn(
      0,0,SignPost_base,SignPost_dir);
   avg_sundir_SignPost_ptr->reset_scale(0,0,0.01);
   
   avg_sundir_SignPost_ptr->set_label("Average sun illumination direction");
   avg_sundir_SignPost_ptr->set_max_text_width("Average sun");
   avg_sundir_SignPost_ptr->set_color(
      colorfunc::get_OSG_color(colorfunc::yellow));

// Instantiate cylinders decoration group:

   decorations.add_Cylinders(passes_group.get_pass_ptr(cloudpass_ID),
                             AnimationController_ptr);
   CylindersGroup* CylindersGroup_ptr=decorations.get_CylindersGroup_ptr();
   CylinderPickHandler* CylinderPickHandler_ptr=
      decorations.get_CylinderPickHandler_ptr();

   CylindersGroup_ptr->set_rh(0.033 , 0.0033);
   CylindersGroup_ptr->set_delta_move_z(0.025);
   CylinderPickHandler_ptr->set_text_size(0.0001);
   CylinderPickHandler_ptr->set_text_displacement(0.0001);
   CylinderPickHandler_ptr->set_permanent_color(colorfunc::pink);
//   CylinderPickHandler_ptr->set_permanent_color(colorfunc::green);
   root->addChild(decorations.get_CylindersGroup_ptr()->
                  createCylinderLight(threevector(20,10,10)));

// Instantiate CentersGroup and center pick handler to handle mid
// point selection:

   CentersGroup centers_group(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),
      AnimationController_ptr,grid_origin_ptr);
   CenterPickHandler* CenterPickHandler_ptr=new CenterPickHandler(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),
      CM_3D_ptr,&centers_group,ModeController_ptr,window_mgr_ptr,
      grid_origin_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      CenterPickHandler_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new CentersKeyHandler(ndims,CenterPickHandler_ptr,ModeController_ptr));
   root->addChild(centers_group.get_OSGgroup_ptr());

// Attach all 3D graphicals to CentersGroup SpinTransform which can
// perform global rotation about some axis coming out of user selected
// center location:

   centers_group.get_SpinTransform_ptr()->addChild(
      decorations.get_OSGgroup_ptr());
   centers_group.get_SpinTransform_ptr()->addChild(
      clouds_group.get_OSGgroup_ptr());
   root->addChild(centers_group.get_SpinTransform_ptr());

// Attach scene graph to the viewer:
   
   window_mgr_ptr->setSceneData(root);

// Open text dialog box to display feature information:

//   decorations.get_FeaturesGroup_ptr()->get_TextDialogBox_ptr()->
//	open("Feature Information");
//   decorations.get_FeaturesGroup_ptr()->update_feature_text();

// Create the windows and run the threads.  Viewer's realize method
// calls the CUstomManipulator's home() method:

   window_mgr_ptr->realize();

// Set initial camera view to grid's midpoint:

   CM_3D_ptr->set_worldspace_center(grid_ptr->get_world_middle());
   CM_3D_ptr->set_eye_to_center_distance(
      2*basic_math::max(grid_ptr->get_xsize(),grid_ptr->get_ysize()));
   CM_3D_ptr->update_M_and_Minv();

//   grid_ptr->set_world_origin_and_middle(0,0);

   while( !window_mgr_ptr->done() )
   {
      window_mgr_ptr->process();
   }

//   decorations.get_FeaturesGroup_ptr()->get_TextDialogBox_ptr()->close();
}

