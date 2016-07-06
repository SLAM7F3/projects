// ========================================================================
// Program FINDGROUND takes in some TDP tile on the command line.  It
// first identifies likely ground pixels based upon the lowest part of
// z-map's height distribution.  It then performs an initial "oozing"
// operation until every pixel within the input TDP tile has been
// classified as ground or non-ground.  Small ground & non-ground
// islands are also recursively emptied.  A binary ground mask (which
// is reminscent of "Mario bashing" results in the past) is
// subsequently generated and used to generate a lattice of ground
// locations and average height values.

// FINDGROUND next constructs a threshold field based upon the sampled
// ground values.  This threshold field establishes a first estimate
// for the bald-earth ground surface.  The threshold field is then
// subtracted from the original height map to yield a approximately
// flattened height map.  We next discard pixels within the flattened
// height map lying too far from z=0 to generate an improved ground
// mask.  A second round of "oozing" is then performed with the
// improved ground mask providing seed locations.  

// The final ground classification information is written to an output
// TDP file in a fused height/binary classification format which can
// be viewed using program VIEWPOINTS.

//		    findground baghdad48_hi_hi.tdp

//		    findground x1y2_filled.tdp

//   findground /media/usbdisk/new_york/rtv/median_filled/3_iters/tdp_files/x1y1_fused_filled.tdp

// ========================================================================
// Last updated on 11/1/07; 11/10/07; 11/11/07; 2/5/09
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include "osg/osgGraphicals/AnimationController.h"
#include "osg/Custom3DManipulator.h"
#include "osg/osgOrganization/Decorations.h"
#include "ladar/groundfuncs.h"
#include "image/imagefuncs.h"
#include "ladar/ladarfuncs.h"
#include "osg/ModeController.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "geometry/parallelogram.h"
#include "passes/PassesGroup.h"
#include "image/raster_parser.h"
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

// Read in TDP file containing height map:

   double delta_x=0.5;	// meter
   double delta_y=0.5;	// meter
   string tdp_filename=
      passes_group.get_pass_ptr(cloudpass_ID)->get_first_filename();
   cout << "tdp_filename = " << tdp_filename << endl;

   twoDarray* ztwoDarray_ptr=tdpfunc::generate_ztwoDarray_from_tdpfile(
      tdp_filename,delta_x,delta_y);

   double zground_min=imagefunc::min_intensity_above_floor(
      0.5*xyzpfunc::null_value,ztwoDarray_ptr);
//   cout << "zground_min = " << zground_min << endl;

   const string UTMzone="18N";		// NYC
//   const string UTMzone="38T";	// Baghdad

//   tdp_filename=cloudpass_prefix+"_init_ztwoDarray.tdp";
//   tdpfunc::write_zp_twoDarrays(
//      tdp_filename,UTMzone,ztwoDarray_ptr,ztwoDarray_ptr);

/*
// Parallelogram bounding box for ESB car video:

   vector<threevector> V;
   V.push_back(threevector(585650.1250 , 4511183.000 , 13.05000210  ));
   V.push_back(threevector(585801.3750 , 4511470.500 , 21.74999809 ));
   V.push_back(threevector(585036.3125 , 4511881.000 , 9.900000572  ));
   V.push_back(threevector(584774.5625 , 4511386.000 , 8.459998131  ));
   parallelogram data_bbox(V);
*/

//   int niters=4;
//   int nsize=3;
//   ladarfunc::median_fill_image(niters,nsize,ztwoDarray_ptr);

// Instantiate ground mask twoDarray:

   twoDarray* groundmask_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);   
   groundmask_twoDarray_ptr->initialize_values(xyzpfunc::null_value);

// Read in any manually identified ground points:

   FeaturesGroup* FeaturesGroup_ptr=
      decorations.add_Features(ndims,passes_group.get_pass_ptr(cloudpass_ID));
   FeaturesGroup_ptr->read_feature_info_from_file();

// Form STL vector of manually identified ground points:

   vector<threevector> groundseed;
   for (int n=0; n<FeaturesGroup_ptr->get_n_Graphicals(); n++)
   {
      Feature* feature_ptr=FeaturesGroup_ptr->get_Feature_ptr(n);
      threevector curr_XYZ;
      feature_ptr->get_UVW_coords(
         FeaturesGroup_ptr->get_curr_t(),
         FeaturesGroup_ptr->get_passnumber(),curr_XYZ);
      groundseed.push_back(curr_XYZ);
   } // loop over index n labeling manually picked ground points
   
// If no ground seed points were manually inputed, identify likely
// ground locations within *ztwoDarray_ptr based simply upon the
// lowest part of its height distribution:

//   if (groundseed.size()==0)
   {
      groundfunc::identify_ground_seed_pixels(
         ztwoDarray_ptr,groundmask_twoDarray_ptr);
   }
   
//   tdp_filename=cloudpass_prefix+"_init_groundseeds.tdp";
//   tdpfunc::write_zp_twoDarrays(
//      tdp_filename,UTMzone,ztwoDarray_ptr,groundmask_twoDarray_ptr);

//   int n_iters=1;
//   int n_iters=2;
//   int n_iters=3;
   int n_iters=4;

   vector<double> threshold_intensity;
   vector<twovector> centers_posn;

   vector<int> n_recursion_iters;
   vector<double> max_gradient_magnitude_lo;
   vector<double> cutoff_height;

// FAKE FAKE: November 11 at 3 pm: For debugging seemingly absured
// z_ground generation results, we set n_recursion_iters to small
// values ...

   n_recursion_iters.push_back(5);
   n_recursion_iters.push_back(5);
   n_recursion_iters.push_back(5);
   n_recursion_iters.push_back(5);

//   n_recursion_iters.push_back(20);
//   n_recursion_iters.push_back(20);
//   n_recursion_iters.push_back(20);
//   n_recursion_iters.push_back(20);

   max_gradient_magnitude_lo.push_back(0.35);
   max_gradient_magnitude_lo.push_back(0.35);
   max_gradient_magnitude_lo.push_back(0.30);
   max_gradient_magnitude_lo.push_back(0.30);

// FAKE FAKE: November 11 at 12:30 pm: Experiment with tightening
// proximity requirement for seeds to threshold ground surface estimate:

//   cutoff_height.push_back(4);
//   cutoff_height.push_back(4);
//   cutoff_height.push_back(4);

   cutoff_height.push_back(2);
   cutoff_height.push_back(2);
   cutoff_height.push_back(2);

   twoDarray* zthreshold_twoDarray_ptr=NULL;

   for (int iter=0; iter<n_iters; iter++)
   {
      string iter_string=stringfunc::number_to_string(iter);
      string big_banner="Iteration "+iter_string;
      outputfunc::write_big_banner(big_banner);

// Add manually identified ground points to *groundmask_twoDarray_ptr:

      for (int n=0; n<groundseed.size(); n++)
      {
         unsigned int px,py;
         if (groundmask_twoDarray_ptr->point_to_pixel(
            groundseed[n].get(0),groundseed[n].get(1),px,py))
         {
            groundmask_twoDarray_ptr->put(px,py,0);
         }
      } // loop over index n labeling manually picked ground points

// Apply oozing and recursive emptying procedures to propagate ground
// pixel classification throughout most of *ztwoDarray_ptr:

      groundfunc::find_low_local_pixels(
         ztwoDarray_ptr,groundmask_twoDarray_ptr,
         max_gradient_magnitude_lo[iter],n_recursion_iters[iter]);

      tdp_filename=cloudpass_prefix+"_ooze_"+iter_string+".tdp";
      tdpfunc::write_zp_twoDarrays(
         tdp_filename,UTMzone,ztwoDarray_ptr,groundmask_twoDarray_ptr);

      if (iter==n_iters-1) continue;

// Change unit-values within binary *groundmask_twoDarray_ptr to
// xyzpfunc::null_value in new *zground_twoDarray_ptr:

      twoDarray* zground_twoDarray_ptr=groundfunc::generate_zground_twoDarray(
         iter,ztwoDarray_ptr,groundmask_twoDarray_ptr);
//      tdp_filename=cloudpass_prefix+"_zground_"+iter_string+".tdp";
//      tdpfunc::write_zp_twoDarrays(
//         tdp_filename,UTMzone,zground_twoDarray_ptr,zground_twoDarray_ptr);

// Sample initial ground mask values and generate lattice of ground
// location and height values:

      int mbins=35;
      int nbins=35;
//      int mbins=45;
//      int nbins=45;
      double correlation_distance=60;   // meters
//   double correlation_distance=50;   // meters
//      double correlation_distance=40;   // meters
      double local_threshold_frac=0.5;
      double minimal_fill_frac=0.025;

      groundfunc::generate_threshold_centers( 
         mbins,nbins,correlation_distance,
         local_threshold_frac,zground_min,minimal_fill_frac,
         zground_twoDarray_ptr,threshold_intensity,centers_posn);
      
      delete zground_twoDarray_ptr;

// Construct threshold field based upon sampled ground values.  This
// threshold field provides an estimate for the bald-earth ground
// surface:

      zthreshold_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
      groundfunc::generate_threshold_field( 
         correlation_distance,NULL,threshold_intensity,centers_posn,
         ztwoDarray_ptr,zthreshold_twoDarray_ptr);

      if (iter==n_iters-2)
      {
         tdp_filename=cloudpass_prefix+"_threshold_field_"+iter_string+".tdp";
         tdpfunc::write_zp_twoDarrays(
            tdp_filename,UTMzone,zthreshold_twoDarray_ptr,
            zthreshold_twoDarray_ptr);
      }
      
// For alg development, we can read in previously calculated initial
// threshold field:

//   twoDarray* zthreshold_twoDarray_ptr=
//      tdpfunc::generate_ztwoDarray_from_tdpfile(
//         tdp_filename,delta_x,delta_y);

// Subtract threshold field from original height map and compute
// flattened ground surface:

      twoDarray* zflattened_twoDarray_ptr=
         groundfunc::generate_thresholded_flattened_height_map(
            ztwoDarray_ptr,zthreshold_twoDarray_ptr,cutoff_height[iter]);

      if (iter < n_iters-2) 
      {
         cout << "iter = " << iter << " n_iters-2 = " << n_iters-2
              << endl;
         cout << "Deleting zthreshold_twoDarray_ptr in main()" << endl;
         delete zthreshold_twoDarray_ptr;
      }
      
//      tdp_filename=cloudpass_prefix+"_flattened"+iter_string+".tdp";
//      tdpfunc::write_zp_twoDarrays(
//         tdp_filename,UTMzone,zflattened_twoDarray_ptr,
//         zflattened_twoDarray_ptr);
   
// Reset unit-valued pixels in *groundmask_twoDarray_ptr to
// xyzpfunc::null_value and refine ground mask estimate:

      groundfunc::refine_ground_mask_seeds(
         cutoff_height[iter],zflattened_twoDarray_ptr,
         groundmask_twoDarray_ptr);
      delete zflattened_twoDarray_ptr;
   
//   tdp_filename=cloudpass_prefix+"_groundmask_"+iter_string+".tdp";
//   bool insert_fake_coloring_points_flag=false;
//   tdpfunc::write_zp_twoDarrays(
//      tdp_filename,UTMzone,ztwoDarray_ptr,groundmask_twoDarray_ptr,
//      insert_fake_coloring_points_flag);
      
   } // loop over iter index


//   double max_delta_height=7;	// meters
   double max_delta_height=10;	// meters

   int n_filter_size=2;
   groundfunc::eliminate_ground_outliers(
      n_filter_size,ztwoDarray_ptr,groundmask_twoDarray_ptr,max_delta_height);

   tdp_filename=cloudpass_prefix+"_groundmask.tdp";
   tdpfunc::write_zp_twoDarrays(
      tdp_filename,UTMzone,ztwoDarray_ptr,groundmask_twoDarray_ptr);

   cout << "Fusing z and binary p images" << endl;

   twoDarray* fused_twoDarray_ptr=xyzpfunc::fuse_z_and_binary_p_images(
      ztwoDarray_ptr,groundmask_twoDarray_ptr);
   delete groundmask_twoDarray_ptr;

   tdp_filename=cloudpass_prefix+"_detected_ground.tdp";
   tdpfunc::write_zp_twoDarrays(
      tdp_filename,UTMzone,ztwoDarray_ptr,fused_twoDarray_ptr);

}

