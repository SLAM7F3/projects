// ========================================================================
// Program TRANSFORM_POINTS takes in a TDP New York City tile, loops
// over each of its XYZ points and transforms them to corrected X'Y'Z'
// coordinates which represent genuine UTM locations.

// 		    transform_points lowell.x0.y0.tdp

// ========================================================================
// Last updated on 4/13/07; 4/16/07; 4/23/07; 10/15/07
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgDB/Registry>
#include <osgDB/WriteFile>

#include "osg/Custom3DManipulator.h"
#include "viewer/CustomDatabasePager.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/ModeController.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGeometry/TrianglesGroup.h"
#include "osg/osgWindow/ViewerManager.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cout;
   using std::endl;
   using std::ifstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);
   
// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("3D imagery");

// Create OSG root node and black backdrop:

   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

   Operations operations(ndims,window_mgr_ptr);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   root->addChild(operations.get_OSGgroup_ptr());

// Add a custom manipulator to the event handler list:

   osgGA::Custom3DManipulator* CM_3D_ptr=new osgGA::Custom3DManipulator(
      ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold all decorations:
   
   Decorations decorations(window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

// Instantiate AlirtGrid decorations group:

   AlirtGrid* grid_ptr=decorations.add_AlirtGrid(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   threevector* grid_origin_ptr=grid_ptr->get_world_origin_ptr();

// Instantiate triangle decorations group:

   decorations.add_Triangles(ndims,passes_group.get_pass_ptr(cloudpass_ID));

// Instantiate group to hold pointcloud information:

   PointCloudsGroup clouds_group(
      passes_group.get_pass_ptr(cloudpass_ID),grid_origin_ptr);
   bool index_tree_flag=false;
   vector<PointCloud*>* clouds_ptrs=clouds_group.generate_Clouds(
      passes_group,index_tree_flag,decorations.get_TrianglesGroup_ptr());
   PointCloud* cloud_ptr=clouds_ptrs->at(0);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_ptr));
   root->addChild(clouds_group.get_OSGgroup_ptr());

// Initialize ALIRT grid based upon cloud's bounding box:

   decorations.get_AlirtGridsGroup_ptr()->initialize_grid(
      grid_ptr,clouds_group.get_xyz_bbox());

// Instantiate a MyDatabasePager to handle paging of files from disk:

   viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager(
      clouds_group.get_SetupGeomVisitor_ptr(),
      clouds_group.get_ColorGeodeVisitor_ptr());

   cloud_ptr->transform_vertices();

//   string triangles_subdir="./features_and_triangles";
//   string triangles_subdir=".";

// On 4/16/07, we empirically determined that the lowest region within
// the Former World Trade Center pit has a relative z = -96 meters, In
// order not to lose any of the FWTC data, we need to set
// ceiling_min_z value when thresholding newyork.x0.y1_a.tdp,

//   double ceiling_min_z=-101;	// meters
//   cloud_ptr->remove_snowflakes(triangles_subdir,ceiling_min_z);

   delete window_mgr_ptr;
}

