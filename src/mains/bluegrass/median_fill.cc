// ========================================================================
// Program MEDIAN_FILL takes in some TDP tile on the command line. 


//		    median_fill x1y2_filled.tdp

// ========================================================================
// Last updated on 11/23/07
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include "osg/osgOrganization/Decorations.h"
#include "ladar/groundfuncs.h"
#include "image/imagefuncs.h"
#include "ladar/ladarfuncs.h"
#include "osg/ModeController.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "geometry/parallelogram.h"
#include "passes/PassesGroup.h"
#include "image/recursivefuncs.h"
#include "osg/osg3D/tdpfuncs.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "image/TwoDarray.h"
#include "osg/osgWindow/ViewerManager.h"
#include "threeDgraphics/xyzpfuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
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

// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

   Operations operations(ndims,window_mgr_ptr);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
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

// Read in TDP file containing height map:

   double delta_x=0.5;	// meter
   double delta_y=0.5;	// meter
   string tdp_filename=
      passes_group.get_pass_ptr(cloudpass_ID)->get_first_filename();
   cout << "tdp_filename = " << tdp_filename << endl;

   twoDarray* ztwoDarray_ptr=tdpfunc::generate_ztwoDarray_from_tdpfile(
      tdp_filename,delta_x,delta_y);
//   double zground_min=imagefunc::min_intensity_above_floor(
//      0.5*xyzpfunc::null_value,ztwoDarray_ptr);
//   cout << "zground_min = " << zground_min << endl;

//   const string UTMzone="18N";		// NYC
//   const string UTMzone="38T";	// Baghdad
   const string UTMzone="14";		// Lubbock

//   tdp_filename=cloudpass_prefix+"_init_ztwoDarray.tdp";
//   tdpfunc::write_zp_twoDarrays(
//      tdp_filename,UTMzone,ztwoDarray_ptr,ztwoDarray_ptr);

   int niters=2;
//   int niters=4;
   int nsize=3;
   ladarfunc::median_fill_image(niters,nsize,ztwoDarray_ptr);

   tdp_filename=cloudpass_prefix+"_filled.tdp";
   tdpfunc::write_zp_twoDarrays(
      tdp_filename,UTMzone,ztwoDarray_ptr,ztwoDarray_ptr);
}

