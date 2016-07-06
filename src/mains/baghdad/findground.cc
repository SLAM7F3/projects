// ========================================================================
// Program FINDGROUND takes in some TDP tile on the command line.  It
// also queries the user to enter a corresponding 3D feature file
// which marks locations on the ground.  FINDGROUND then performs
// multiple ground "oozing" iterations until every pixel within the
// input TDP tile has been classified as ground or non-ground.  Small
// non-ground islands are also recursively emptied.  The final ground
// mask is written to an output TDP file in a fused height/binary
// classification format which can be viewed using program VIEWPOINTS
// with the large_hue_value_sans_white colormap.

// Chant from within ./tdp_files subdir which contains
// features_3D_baghdad*.txt feature files, 

//		    findground baghdad48_hi_hi.tdp

// ========================================================================
// Last updated on 9/21/07; 10/11/07; 10/15/07; 2/5/09
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CentersKeyHandler.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "osg/Custom3DManipulator.h"
#include "osg/osgOrganization/Decorations.h"
#include "ladar/groundfuncs.h"
#include "osg/osgSceneGraph/HiresDataVisitor.h"
#include "osg/ModeController.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "image/raster_parser.h"
#include "osg/osg3D/tdpfuncs.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "passes/TextDialogBox.h"
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
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   root->addChild(operations.get_OSGgroup_ptr());

// Add a custom manipulator to the event handler list:

//   osgGA::Custom3DManipulator* CM_3D_ptr=new osgGA::Custom3DManipulator(
//      ModeController_ptr);
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

// Instantiate triangle decorations group:

   decorations.add_Triangles(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));

// Instantiate group to hold pointcloud information:

   PointCloudsGroup clouds_group(
      passes_group.get_pass_ptr(cloudpass_ID),grid_origin_ptr);
   bool index_tree_flag=false;
   vector<PointCloud*>* cloudptrs_ptr=clouds_group.generate_Clouds(
      passes_group,index_tree_flag,decorations.get_TrianglesGroup_ptr());

   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_ptr));
//   root->addChild(clouds_group.get_OSGgroup_ptr());

// Instantiate a PointFinder;

   PointFinder pointfinder(&clouds_group);
   CM_3D_ptr->set_PointFinder(&pointfinder);

// Initialize ALIRT grid based upon cloud's bounding box:

   decorations.get_AlirtGridsGroup_ptr()->initialize_grid(
      grid_ptr,clouds_group.get_xyz_bbox());

// Instantiate a MyDatabasePager to handle paging of files from disk:

   viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager(
      clouds_group.get_SetupGeomVisitor_ptr(),
      clouds_group.get_ColorGeodeVisitor_ptr());
   clouds_group.get_HiresDataVisitor_ptr()->setDatabasePager(
      MyDatabasePager_ptr);

// Instantiate signpost and feature decoration groups:

   decorations.add_SignPosts(ndims,passes_group.get_pass_ptr(cloudpass_ID));
   decorations.add_Features(ndims,passes_group.get_pass_ptr(cloudpass_ID));
   FeaturesGroup* FeaturesGroup_ptr=decorations.get_FeaturesGroup_ptr();

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

// Recall ladarimage member of PointCloud class is generally NOT
// instantiated.  We need to explicitly do so here:

   PointCloud* cloud_ptr=cloudptrs_ptr->at(0);
   double delta_x=1.0;	// meter
   double delta_y=1.0;	// meter
   cloud_ptr->generate_ladarimage(delta_x,delta_y);
   twoDarray* ztwoDarray_ptr=cloud_ptr->get_ladarimage_ptr()->
      get_z2Darray_ptr();

//   cout << "*ztwoDarray_ptr = " << *ztwoDarray_ptr << endl;

   FeaturesGroup_ptr->read_feature_info_from_file();

   vector<threevector> groundpoint_XYZ;
   threevector curr_XYZ;
   for (unsigned int n=0; n<FeaturesGroup_ptr->get_n_Graphicals(); n++)
   {
      Feature* feature_ptr=FeaturesGroup_ptr->get_Feature_ptr(n);
      feature_ptr->get_UVW_coords(
         FeaturesGroup_ptr->get_curr_t(),
         FeaturesGroup_ptr->get_passnumber(),curr_XYZ);
      groundpoint_XYZ.push_back(curr_XYZ);
      cout << "n = " << n << " XYZ = " << curr_XYZ << endl;
   } // loop over index n labeling manually picked ground points

   int n_recursion_iters=10;
   const double max_gradient_magnitude_lo=0.25;
   
   twoDarray* zhilo_twoDarray_ptr=groundfunc::find_low_local_pixels(
      groundpoint_XYZ,ztwoDarray_ptr,max_gradient_magnitude_lo,
      n_recursion_iters);

   twoDarray* fused_twoDarray_ptr=xyzpfunc::fuse_z_and_binary_p_images(
      ztwoDarray_ptr,zhilo_twoDarray_ptr);

//   const string UTMzone="38T";	// Baghdad
   const string UTMzone="18N";	// NYC
   string tdp_filename=cloudpass_prefix+"_fused_mask.tdp";
   tdpfunc::write_zp_twoDarrays(
      tdp_filename,UTMzone,ztwoDarray_ptr,fused_twoDarray_ptr);

   tdp_filename=cloudpass_prefix+"_ground_mask.tdp";
   tdpfunc::write_zp_twoDarrays(
      tdp_filename,UTMzone,ztwoDarray_ptr,zhilo_twoDarray_ptr);


   exit(-1);

   raster_parser RasterParser;

   bool output_floats_flag=false;

   string geotiff_filename=cloudpass_prefix+"_height.tif";
   int output_UTM_zonenumber=38;
   bool output_northern_hemisphere_flag=true;
   RasterParser.write_raster_data(
      output_floats_flag,geotiff_filename,
      output_UTM_zonenumber,output_northern_hemisphere_flag,
      ztwoDarray_ptr);

   geotiff_filename=cloudpass_prefix+"_fused.tif";
   RasterParser.write_raster_data(
      output_floats_flag,geotiff_filename,
      output_UTM_zonenumber,output_northern_hemisphere_flag,
      fused_twoDarray_ptr);

   geotiff_filename=cloudpass_prefix+"_ground_mask.tif";
   RasterParser.write_raster_data(
      output_floats_flag,geotiff_filename,
      output_UTM_zonenumber,output_northern_hemisphere_flag,
      zhilo_twoDarray_ptr);
}

