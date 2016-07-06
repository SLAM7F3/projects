// ========================================================================
// Program FUSE_POINTS takes in a TDP New York City tile, loops over
// each of its XYZP points and resets its corresponding RGB based upon
// its p-value.
// ========================================================================
// Last updated on 4/16/07; 4/23/07; 10/15/07; 12/22/07
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgDB/Registry>
#include <osgDB/WriteFile>

#include "color/colorfuncs.h"
#include "osg/Custom3DManipulator.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/ModeController.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osg3D/tdpfuncs.h"
#include "osg/osgGeometry/TrianglesGroup.h"
#include "osg/osgWindow/ViewerManager.h"
#include "threeDgraphics/xyzpfuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
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
   PassInfo* PassInfo_ptr=passes_group.get_passinfo_ptr(cloudpass_ID);

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

// Instantiate a MyDatabasePager to handle paging of files from disk:

   viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager(
      clouds_group.get_SetupGeomVisitor_ptr(),
      clouds_group.get_ColorGeodeVisitor_ptr());

   osg::Vec3Array* vertices_ptr=cloud_ptr->get_vertices_ptr();
   model::Metadata* metadata_ptr=cloud_ptr->get_metadata_ptr();
   osg::Vec4ubArray* colors_ptr=cloud_ptr->get_colors_ptr();

   const int ten_percent_size=vertices_ptr->size()/10;
   const double wrap_frac=3.0;

// Global height thresholds for thresholded but un-georegistered
// ALIRT-A NYC map:

//   const double zmin=-100.975;	// meters
//   const double zmax=266.477;	// meters

// Global height thresholds for georegistered (and z-thresholded) RTV
// NYC map:

   double zmin=-8.7;	// meters
   double zmax=395;	// meters
   
   double v_hi=1.0;
   double v_lo=0.05;
   
   const unsigned char alpha_byte=
      static_cast<unsigned char>(stringfunc::ascii_integer_to_char(255));

   double h,v,s,r,g,b;
   for (unsigned int i=0; i<cloud_ptr->get_npoints(); i++)
   {
      if (i%ten_percent_size==0) cout << i/ten_percent_size*10 << "% " 
                                      << flush;

      double z=vertices_ptr->at(i).z();
      double p=metadata_ptr->get(i,0);
      xyzpfunc::convert_zp_to_hue_and_intensity(
         wrap_frac,zmax,zmin,z,p,v_hi,v_lo,h,v,s);

// Rotate all hues through a constant angle proportional to
// cyclic_frac_offset.  This allows us to control the basic hue for
// the ground (which we generally like to appear as brownish...)

      h += PassInfo_ptr->get_height_colormap_cyclic_fraction_offset()*360;

      colorfunc::hsv_to_RGB(h,s,v,r,g,b);
      
      int R=r*255;
      int G=g*255;
      int B=b*255;
      colors_ptr->at(i)=osg::Vec4ub(
         static_cast<unsigned char>(static_cast<unsigned int>(R)),
         static_cast<unsigned char>(static_cast<unsigned int>(G)),
         static_cast<unsigned char>(static_cast<unsigned int>(B)),
         alpha_byte);
   }
   cout << endl << endl;

   string tdp_filename=cloudpass_prefix+"_fused.tdp";
   cout << "Writing fused results to " << tdp_filename << endl << endl;
   string UTMzone="18N";	// new york

   delete window_mgr_ptr;
}


