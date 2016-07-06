// ========================================================================
// Program MERGEPOINTS is a specialized utility program which takes in
// two TDP files as command line arguments.  The first is assumed to
// contain median filled NYC tile information.  The second is
// assumed to contain original NYC RTV tile information which has
// nontrivial wall content.  This program keeps those points from the
// latter whose z values differ by some reasonable amount from the
// height map generated from the former.  The combined median filled
// and wall content information is written to an output TDP file.

// 	mergepoints x0y0_fused_filled.tdp --newpass x0y0_fused.tdp

// ========================================================================
// Last updated on 11/11/07
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/ModeController.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osg3D/tdpfuncs.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "image/TwoDarray.h"
#include "osg/osgWindow/ViewerManager.h"

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

// Read in TDP file containing median-filled height map and store its
// contents within *ztwoDarray_ptr:

   double delta_x=0.5;	// meter
   double delta_y=0.5;	// meter
   string filled_tdp_filename=
      passes_group.get_pass_ptr(0)->get_first_filename();
//   cout << "filled_tdp_filename = " << filled_tdp_filename << endl;
   twoDarray* ztwoDarray_ptr=tdpfunc::generate_ztwoDarray_from_tdpfile(
      filled_tdp_filename,delta_x,delta_y);

// Read in TDP file containing original NYC point cloud with
// non-trivial wall content.  Store its values in STL vectors X,Y,Z:

   string orig_tdp_filename=
      passes_group.get_pass_ptr(1)->get_first_filename();
//   cout << "orig_tdp_filename = " << orig_tdp_filename << endl;

   vector<double> Xorig,Yorig,Zorig;
   tdpfunc::read_XYZ_points_from_tdpfile(orig_tdp_filename,Xorig,Yorig,Zorig);

   vector<bool> include_orig_point;
   include_orig_point.reserve(Xorig.size());

   for (int n=0; n<Xorig.size(); n++)
   {
      double orig_x=Xorig[n];
      double orig_y=Yorig[n];
      double orig_z=Zorig[n];

      bool include_orig_point_flag=true;
      unsigned int px,py;
      if (ztwoDarray_ptr->point_to_pixel(orig_x,orig_y,px,py))
      {
         double filled_z=ztwoDarray_ptr->get(px,py);

         const double max_z_separation=4;	// meters
         if (fabs(filled_z-orig_z) < max_z_separation)
         {
            include_orig_point_flag=false;
         }
      }
      include_orig_point.push_back(include_orig_point_flag);
   } // loop over index n labeling XYZ points within original NYC
     // point cloud

// Combine wall points from original NYC point cloud with median
// filled values and write to new output TDP file:

   string prefix=stringfunc::prefix(orig_tdp_filename);
   string output_tdp_filename=prefix+"_walls.tdp";

   const string UTMzone="18N";		// NYC
   tdpfunc::write_xyz_data(
      UTMzone,output_tdp_filename,ztwoDarray_ptr,Xorig,Yorig,Zorig,
      include_orig_point);
}

