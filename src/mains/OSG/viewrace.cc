// ========================================================================
// Program VIEWRACE animates the motion of an aircraft executing a
// racetrack orbit centered upon a ground point located in Lowell
// chunk 45-51 of ALIRT data collected in July 2003.  The Cessna model
// flies around in a circular orbit, and its line of sight towards the
// center of the orbit on the ground is updated as well.

// In order to see a working example of this program, first chant

// 			viewrace lowell.xyzp

// 			viewrace lowell.osga

// 			viewrace --region_filename ./packages/lowell.pkg

// from within src/mains/OSG.  Then enter "los.txt" in response to the
// query for a file containing line segment information.  Finally,
// press "R" to enter Run Movie Mode followed by "a" to animate the
// Cessna plane and its line-of-sight segment.

// ========================================================================
// Last updated on 9/21/07; 10/11/07; 10/15/07
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include "osg/osgGrid/AlirtGridsGroup.h"
#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgGrid/GridKeyHandler.h"
#include "osg/ModeController.h"
#include "osg/osgModels/ModelsGroup.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgWindow/ViewerManager.h"

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
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("3D imagery");
//   window_mgr_ptr->getUsage(*arguments.getApplicationUsage());

// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

   bool display_movie_state=true;
   bool display_movie_number=true;
   Operations operations(ndims,window_mgr_ptr,display_movie_state,
                         display_movie_number);

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

   AlirtGrid* grid_ptr=decorations.add_AlirtGrid(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   threevector* grid_origin_ptr=grid_ptr->get_world_origin_ptr();
   CM_3D_ptr->set_Grid_ptr(grid_ptr);
   decorations.set_grid_origin_ptr(grid_origin_ptr);

// Instantiate group to hold pointcloud information:

   PointCloudsGroup clouds_group(
      passes_group.get_pass_ptr(cloudpass_ID),grid_origin_ptr);
   clouds_group.generate_Clouds(passes_group);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_ptr));

// Instantiate a PointFinder;

   PointFinder pointfinder(&clouds_group);
   CM_3D_ptr->set_PointFinder(&pointfinder);

// Initialize ALIRT grid based upon cloud's bounding box:

   decorations.get_AlirtGridsGroup_ptr()->initialize_grid(
      grid_ptr,clouds_group.get_xyz_bbox());

// Instantiate a MyDatabasePager to handle paging of files from disk:

//   viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager(
//      clouds_group.get_SetupGeomVisitor_ptr(),
//      clouds_group.get_ColorGeodeVisitor_ptr());

// Instantiate CentersGroup and center pick handler to handle mid
// point selection:

   CentersGroup centers_group(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   CenterPickHandler* CenterPickHandler_ptr=new CenterPickHandler(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),
      CM_3D_ptr,&centers_group,ModeController_ptr,window_mgr_ptr,
      grid_origin_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      CenterPickHandler_ptr);

// Instantiate model decorations group:

   decorations.add_Models(
      passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
   ModelsGroup* ModelsGroup_ptr=decorations.get_ModelsGroup_ptr();
   string model_filename="./my_cessna.osg";
   Model* curr_model_ptr=ModelsGroup_ptr->generate_new_Model(model_filename);

// Generate racetrack orbit for airplane model:

   int n_total_frames=180;
   AnimationController_ptr->set_nframes(n_total_frames);

   threevector center_origin(300,275,0);
   double radius=275;
   double height_above_center=311;
   double orbit_period=1.0*n_total_frames;
   ModelsGroup_ptr->generate_racetrack_orbit(
      curr_model_ptr,center_origin,radius,height_above_center,
      orbit_period);

   const double cessna_scale_factor=1;
   ModelsGroup_ptr->set_constant_scale(curr_model_ptr,cessna_scale_factor);

// Instantiate line-of-sight linesegments decorations group:

   decorations.add_LineSegments(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),
      AnimationController_ptr);
   string lines_filename="los.txt";
   decorations.get_LineSegmentsGroup_ptr()->
      reconstruct_lines_from_file_info(lines_filename);
   decorations.get_LineSegmentsGroup_ptr()->set_width(4);

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


// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_ptr->realize();

   while( !window_mgr_ptr->done() )
   {
      window_mgr_ptr->process();
   }

   delete window_mgr_ptr;
}

