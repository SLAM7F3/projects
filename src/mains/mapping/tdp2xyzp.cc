// ==========================================================================
// Program TDP2XYZP reads in a point cloud from some input TDP file.
// It writes out a corresponding XYZP file containing probability of
// detection information.

// 			tdp2xyzp  foo.tdp

// ==========================================================================
// Last updated on 10/24/07; 1/8/11
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include "osg/Custom3DManipulator.h"
#include "astro_geo/geopoint.h"
#include "osg/ModeController.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "general/sysfuncs.h"
#include "osg/osg3D/tdpfuncs.h"
#include "osg/osgWindow/ViewerManager.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   using std::cin;
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

   string cloudpass_prefix=
      passes_group.get_pass_ptr(cloudpass_ID)->get_passname_prefix();
   cout << "cloudpass_prefix = " << cloudpass_prefix << endl;

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
   root->addChild(operations.get_OSGgroup_ptr());

// Add a custom manipulator to the event handler list:

   osgGA::Custom3DManipulator* CM_3D_ptr=new osgGA::Custom3DManipulator(
      ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold pointcloud information:

   threevector* grid_origin_ptr=NULL;
   PointCloudsGroup clouds_group(
      passes_group.get_pass_ptr(cloudpass_ID),grid_origin_ptr);
   bool index_tree_flag=false;
   vector<PointCloud*>* cloudptrs_ptr=clouds_group.generate_Clouds(
      passes_group,index_tree_flag,NULL);

   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_ptr));
//   root->addChild(clouds_group.get_OSGgroup_ptr());

// Instantiate a MyDatabasePager to handle paging of files from disk:

   viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager(
      clouds_group.get_SetupGeomVisitor_ptr(),
      clouds_group.get_ColorGeodeVisitor_ptr());

// Recall ladarimage member of PointCloud class is generally NOT
// instantiated.  We need to explicitly do so here:

   PointCloud* cloud_ptr=cloudptrs_ptr->at(0);

// Doublecheck UTM zone info:

//   double representative_longitude=-71.313;	// Lowell
//   double representative_latitude=42.631;	// Lowell
//   geopoint representative_point(representative_longitude,
//                                 representative_latitude);
//   cout << "easting = " << representative_point.get_UTM_easting() << endl;
//   cout << "northing = " << representative_point.get_UTM_northing() << endl;
//   cout << "UTM zonenumber = " << representative_point.get_UTM_zonenumber() 
//        << endl;

// Write point cloud to output XYZP or XYZRGBA file:
   
   string output_filename=cloudpass_prefix+"_output";
   cout << "Output filename = " << output_filename << endl;

   string output_xyzp_filename=output_filename+".xyzp";
   string subdir="./";
   cloud_ptr->write_XYZP_file(output_xyzp_filename,subdir);
//   cloud_ptr->write_XYZRGBA_file(output_filename,subdir);

//   cloud_ptr->write_TDP_file(output_filename);

}
