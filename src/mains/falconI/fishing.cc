// ========================================================================
// Program FISHING is a variant of VIEWPOINTS intended for developing
// a new physics-based approach to ground finding.  We start with a
// "fishnet" set within a constant z-plane located above some filtered
// point cloud generated via program DENSITY_L1.  The sites within the
// deformable fishnet are systematically lowered in height, and an
// energy function is calculated after each perturbation.  The energy
// function has gravity, spring and "pressure" potential terms.
// After some number of iterations, the deformed fishnet should be
// reasonably smoothly draped over the squished L1 point cloud.

//			fishing filtered_points.osga


// ========================================================================
// Last updated on 12/5/11; 12/6/11; 12/7/11
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
#include "osg/osgGeometry/CylindersGroup.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osg3D/FishnetsGroup.h"
#include "osg/osg3D/FishnetsKeyHandler.h"
#include "osg/osgSceneGraph/HiresDataVisitor.h"
#include "osg/ModeController.h"
#include "osg/osgModels/MODELSGROUP.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "osg/osg3D/tdpfuncs.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgGeometry/TrianglesGroup.h"
#include "osg/osgWindow/ViewerManager.h"
#include "coincidence_processing/VolumetricCoincidenceProcessor.h"

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

// Constants 

   const double voxel_binsize=0.25;		// meter
   const double Fishnet_VCP_XY_binsize=3;	// meters
   const double Fishnet_VCP_Z_binsize=1;	// meters

// Repeated variable declarations:

   string banner,unix_cmd="";
   bool perturb_voxels_flag;
   double min_prob_threshold=0;
   
// Use an ArgumentParser object to manage the program arguments:
   
   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);

   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
   cout << "cloudpass_ID = " << cloudpass_ID << endl;

   string filtered_osga_filename=passes_group.get_pass_ptr(cloudpass_ID)->
      get_first_filename();
   string filtered_file_dir=filefunc::getdirname(filtered_osga_filename);
   string filtered_file_basename=filefunc::getbasename(filtered_osga_filename);
   string filtered_file_prefix=stringfunc::prefix(filtered_file_basename);
   string filtered_file_tdpname=filtered_file_dir+
      filtered_file_prefix+".tdp";
   cout << "filtered tdp filename = " << filtered_file_tdpname << endl;

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("3D imagery");
   
// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

   bool display_movie_state=false;
//   bool display_movie_state=true;
   bool display_movie_number=false;
//   bool display_movie_number=true;
   bool display_movie_world_time=false;
   bool hide_Mode_HUD_flag=false;
//   bool hide_Mode_HUD_flag=true;
   Operations operations(
      ndims,window_mgr_ptr,passes_group,
      display_movie_state,display_movie_number,display_movie_world_time,
      hide_Mode_HUD_flag);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   ModeController_ptr->setState(ModeController::MANIPULATE_FISHNET);
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   root->addChild(operations.get_OSGgroup_ptr());

// Add a custom manipulator to the event handler list:

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);
   CM_3D_ptr->set_min_camera_height_above_grid(0.1);	// meters
//   CM_3D_ptr->set_min_camera_height_above_grid(100);	// meters
   CM_3D_ptr->set_enable_underneath_looking_flag(true);
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold all decorations:
   
   Decorations decorations(window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

   AlirtGrid* grid_ptr=decorations.add_AlirtGrid(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
//    grid_ptr->set_threeD_grid_flag(true);
   threevector* grid_origin_ptr=grid_ptr->get_world_origin_ptr();
   CM_3D_ptr->set_Grid_ptr(grid_ptr);
   decorations.set_grid_origin_ptr(grid_origin_ptr);

// Instantiate triangle decorations group:

   decorations.add_Triangles(ndims,passes_group.get_pass_ptr(cloudpass_ID));

// Instantiate group to hold pointcloud information:

   PointCloudsGroup clouds_group(
      passes_group.get_pass_ptr(cloudpass_ID),grid_origin_ptr);

   bool index_tree_flag=false;

   vector<PointCloud*>* PointCloud_ptrs_ptr=clouds_group.generate_Clouds(
      passes_group,index_tree_flag,decorations.get_TrianglesGroup_ptr());
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_ptr,CM_3D_ptr));

// Set colormap:

   PointCloud* PointCloud_ptr=PointCloud_ptrs_ptr->at(0);
   ColorMap* height_ColorMap_ptr=PointCloud_ptr->get_z_ColorMap_ptr();
   height_ColorMap_ptr->set_mapnumber(4); 
		      // large hue value sans white

// Initialize ALIRT grid based upon cloud's bounding box:

//   Grid::Distance_Scale distance_scale=Grid::meter;
//   double delta_s=2;	// meters
   decorations.get_AlirtGridsGroup_ptr()->initialize_grid(
      grid_ptr,clouds_group.get_xyz_bbox());
//   decorations.get_AlirtGridsGroup_ptr()->initialize_grid(
//      grid_ptr,clouds_group.get_xyz_bbox(),distance_scale,delta_s);
   cout << "*grid_origin_ptr = " << *grid_origin_ptr << endl;

// Instantiate a MyDatabasePager to handle paging of files from disk:

   viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager(
      clouds_group.get_SetupGeomVisitor_ptr(),
      clouds_group.get_ColorGeodeVisitor_ptr());
   clouds_group.get_HiresDataVisitor_ptr()->setDatabasePager(
      MyDatabasePager_ptr);

// Instantiate a PointFinder;

   PointFinder pointfinder(&clouds_group);
   CM_3D_ptr->set_PointFinder(&pointfinder);

// Instantiate signpost and feature decoration groups:

   SignPostsGroup* SignPostsGroup_ptr=
      decorations.add_SignPosts(ndims,passes_group.get_pass_ptr(cloudpass_ID));
   SignPostsGroup_ptr->set_ladar_height_data_flag(true);
   SignPostsGroup_ptr->set_CM_3D_ptr(CM_3D_ptr);

   decorations.add_Features(ndims,passes_group.get_pass_ptr(cloudpass_ID));
   decorations.get_FeaturesGroup_ptr()->set_TrianglesGroup_ptr(
      decorations.get_TrianglesGroup_ptr());
   decorations.get_FeaturePickHandler_ptr()->
      set_surface_picking_flag(false);
   decorations.get_FeaturePickHandler_ptr()->
      set_Zplane_picking_flag(false);

// Read in TDP file containing aggregated and filtered Puerto Rico
// data points:

// First compute extremal XYZ values over all squished tdp files:

   threevector XYZ_min(1.0E15,1.0E15,1.0E15);
   threevector XYZ_max(-1.0E15,-1.0E15,-1.0E15);
   tdpfunc::compute_extremal_XYZ_points_in_tdpfile(
      filtered_file_tdpname,XYZ_min,XYZ_max);
   cout << "XYZ_min = " << XYZ_min << " XYZ_max = " << XYZ_max << endl;

   vector<double>* X_ptr=new vector<double>;
   vector<double>* Y_ptr=new vector<double>;
   vector<double>* Z_ptr=new vector<double>;
   vector<double>* P_ptr=new vector<double>;

   int npoints=tdpfunc::npoints_in_tdpfile(filtered_file_tdpname);
   X_ptr->reserve(npoints);
   Y_ptr->reserve(npoints);
   Z_ptr->reserve(npoints);
   P_ptr->reserve(npoints);

   tdpfunc::read_XYZP_points_from_tdpfile(
      filtered_file_tdpname,*X_ptr,*Y_ptr,*Z_ptr,*P_ptr);

// Calculate Z points' probability distribution:

   prob_distribution Z_prob(*Z_ptr,500);
   double z_001=Z_prob.find_x_corresponding_to_pcum(0.001);
//   double z_10=Z_prob.find_x_corresponding_to_pcum(0.10);
   double z_25=Z_prob.find_x_corresponding_to_pcum(0.25);
   double z_50=Z_prob.find_x_corresponding_to_pcum(0.5);
//   double z_66=Z_prob.find_x_corresponding_to_pcum(0.66);
   double z_999=Z_prob.find_x_corresponding_to_pcum(0.999);
   cout << "z_001 = " << z_001 << endl;
   cout << "z_50 = " << z_50 << endl;
   cout << "z_999 = " << z_999 << endl;

// Instantiate Volumetric Coincidence Processors:

   VolumetricCoincidenceProcessor* points_VCP_ptr=
      new VolumetricCoincidenceProcessor;
   points_VCP_ptr->initialize_coord_system(XYZ_min,XYZ_max,voxel_binsize);

//   cout << "mdim = " << points_VCP_ptr->get_mdim() 
//        << " ndim = " << points_VCP_ptr->get_ndim()
//        << " pdim = " << points_VCP_ptr->get_pdim() << endl;

   VolumetricCoincidenceProcessor* Fishnet_VCP_ptr=
      new VolumetricCoincidenceProcessor;
   Fishnet_VCP_ptr->initialize_coord_system(
      XYZ_min,XYZ_max,Fishnet_VCP_XY_binsize,Fishnet_VCP_Z_binsize);

   for (unsigned int i=0; i<X_ptr->size(); i++)
   {
      points_VCP_ptr->accumulate_points_and_probs(
         X_ptr->at(i),Y_ptr->at(i),Z_ptr->at(i),P_ptr->at(i));
      Fishnet_VCP_ptr->accumulate_points_and_probs(
         X_ptr->at(i),Y_ptr->at(i),Z_ptr->at(i),P_ptr->at(i));

//      cout << "i = " << i
//           << " X = " << X_ptr->at(i)
//           << " Y = " << Y_ptr->at(i)
//           << " Z = " << Z_ptr->at(i)
//           << " P = " << P_ptr->at(i) << endl;
   }
   cout << "Number of nonempty VCP voxels = " << points_VCP_ptr->size() 
        << endl;
   cout << "Number of nonempty Fishnet VCP voxels = " 
        << Fishnet_VCP_ptr->size() << endl;

   delete X_ptr;
   delete Y_ptr;
   delete Z_ptr;
   delete P_ptr;

// Write out voxelized counts extracted from *Fishnet_VCP_ptr:

   vector<double>* X_fishnet_binned_ptr=new vector<double>;
   vector<double>* Y_fishnet_binned_ptr=new vector<double>;
   vector<double>* Z_fishnet_binned_ptr=new vector<double>;
   vector<double>* P_fishnet_binned_ptr=new vector<double>;

   X_fishnet_binned_ptr->reserve(Fishnet_VCP_ptr->size());
   Y_fishnet_binned_ptr->reserve(Fishnet_VCP_ptr->size());
   Z_fishnet_binned_ptr->reserve(Fishnet_VCP_ptr->size());
   P_fishnet_binned_ptr->reserve(Fishnet_VCP_ptr->size());
   
   min_prob_threshold=0;
//   perturb_voxels_flag=true;
    perturb_voxels_flag=false;

    Fishnet_VCP_ptr->retrieve_XYZP_points(
       X_fishnet_binned_ptr,Y_fishnet_binned_ptr,Z_fishnet_binned_ptr,
       P_fishnet_binned_ptr,min_prob_threshold,perturb_voxels_flag);

/*
    string fishnet_binned_tdp_filename="fishnet_VCP.tdp";
    tdpfunc::write_xyzp_data(
       fishnet_binned_tdp_filename,X_fishnet_binned_ptr,Y_fishnet_binned_ptr,
       Z_fishnet_binned_ptr,P_fishnet_binned_ptr);

   unix_cmd="lodtree "+fishnet_binned_tdp_filename;
   sysfunc::unix_command(unix_cmd);
*/

    vector<threevector>* XYZ_fishnet_binned_ptr=
       new vector<threevector>;
    for (unsigned int i=0; i<Z_fishnet_binned_ptr->size(); i++)
    {
       XYZ_fishnet_binned_ptr->push_back(threevector(
          X_fishnet_binned_ptr->at(i),
          Y_fishnet_binned_ptr->at(i),
          Z_fishnet_binned_ptr->at(i)));
    }

   delete X_fishnet_binned_ptr;
   delete Y_fishnet_binned_ptr;
   delete Z_fishnet_binned_ptr;
   delete P_fishnet_binned_ptr;

/*
// Query user to specify if fishnet should fall down or up:

   banner="Enter 'd' ['u'] for fishnet to fall down [up] onto tree tops [ground surface]";
   outputfunc::write_big_banner(banner);
   string input_char;
   cin >> input_char;

   bool falling_downward_flag=false;
   if (input_char != "u")
   {
      falling_downward_flag=true;
   }

*/

   bool falling_downward_flag=true;
   cout << "falling_downward_flag = " << falling_downward_flag << endl;

// Instantiate FishnetsGroup:

   FishnetsGroup* FishnetsGroup_ptr=new FishnetsGroup(
      passes_group.get_pass_ptr(cloudpass_ID),grid_origin_ptr);

   if (falling_downward_flag)
   {
      FishnetsGroup_ptr->set_Zstart(z_999+1);
      FishnetsGroup_ptr->set_Zstop(z_001);
   }
   else
   {
      FishnetsGroup_ptr->set_Zstart(z_001-2);
      FishnetsGroup_ptr->set_Zstop(z_25);
   }

// Instantiate Fishnet:

   Fishnet* Fishnet_ptr=FishnetsGroup_ptr->generate_new_Fishnet(
      falling_downward_flag);
   Fishnet_ptr->set_linewidth(3);
   Fishnet_ptr->set_VCP_ptr(Fishnet_VCP_ptr);

   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new FishnetsKeyHandler(FishnetsGroup_ptr,ModeController_ptr));

//   double min_x=grid_ptr->get_min_grid_x();
//   double max_x=grid_ptr->get_max_grid_x();
//   double min_y=grid_ptr->get_min_grid_y();
//   double max_y=grid_ptr->get_max_grid_y();

   double min_x=XYZ_min.get(0);
   double max_x=XYZ_max.get(0);
   double min_y=XYZ_min.get(1);
   double max_y=XYZ_max.get(1);
   double min_z=XYZ_min.get(2);
   double max_z=XYZ_max.get(2);

   Fishnet_ptr->set_min_z_points(min_z);
   Fishnet_ptr->set_max_z_points(max_z);

//   double mid_x=0.5*(min_x+max_x);
//   double mid_y=0.5*(min_y+max_y);
   
//   double fishnet_Xstep=5;	// meters
//   double fishnet_Ystep=5;	// meters
//   double fishnet_Xstep=20;	// meters
//   double fishnet_Ystep=20;	// meters
   double fishnet_Xstep=40;	// meters
   double fishnet_Ystep=40;	// meters

//   double fishnet_Xstep=7.5;	// meters
//   double fishnet_Ystep=7.5;	// meters
//   double fishnet_Xstep=10;
//   double fishnet_Ystep=10;
//   double fishnet_Xstep=50;
//   double fishnet_Ystep=50;
   Fishnet_ptr->init_coord_system(
      fishnet_Xstep,fishnet_Ystep,min_x,max_x,min_y,max_y,
      FishnetsGroup_ptr->get_Zstart());

//      fishnet_Xstep,fishnet_Ystep,
//      mid_x-2*fishnet_Xstep,mid_x+2*fishnet_Xstep,
//      mid_y-2*fishnet_Ystep,mid_y+2*fishnet_Ystep,
//       FishnetsGroup_ptr->get_Zstart());

   Fishnet_ptr->regenerate_PolyLines();

// Calculate "cumulative probability distribution" within each chimney
// as a function of Z just once!

   Fishnet_VCP_ptr->integrate_probs_within_column_integrals();

// Calculate fishnet's pressure mask just once:

   Fishnet_ptr->generate_pressure_mask();

   double ground_surface_thickness=0;
   banner="Enter ground surface thickness in meters:";
   outputfunc::write_big_banner(banner);
   cin >> ground_surface_thickness;
   Fishnet_ptr->set_ground_surface_thickness(ground_surface_thickness);

/*
// Instantiate cylinders decoration group:

   CylindersGroup* CylindersGroup_ptr=
      decorations.add_Cylinders(passes_group.get_pass_ptr(cloudpass_ID),
                                AnimationController_ptr);
   CylindersGroup_ptr->set_rh(0.5,0.5);
   root->addChild(
      decorations.get_CylindersGroup_ptr()->
      createCylinderLight(threevector(20,10,10)));

   osg::Quat q(0,0,0,1);
   colorfunc::Color cyl_color=colorfunc::white;
   for (int i=0; i<XYZ_fishnet_binned_ptr->size(); i++)
   {
      Cylinder* Cylinder_ptr=CylindersGroup_ptr->generate_new_Cylinder(
         XYZ_fishnet_binned_ptr->at(i),q,cyl_color);
   }
*/

// Instantiate CentersGroup and center pick handler to handle mid
// point selection:

   CentersGroup centers_group(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),
      AnimationController_ptr,grid_origin_ptr);
   CenterPickHandler* CenterPickHandler_ptr=new CenterPickHandler(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),
      CM_3D_ptr,&centers_group,ModeController_ptr,window_mgr_ptr,
      grid_origin_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(CenterPickHandler_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new CentersKeyHandler(ndims,CenterPickHandler_ptr,ModeController_ptr));

// Attach all 3D graphicals to CentersGroup SpinTransform which can
// perform global rotation about some axis coming out of user selected
// center location:

   centers_group.get_SpinTransform_ptr()->addChild(
      decorations.get_OSGgroup_ptr());
   centers_group.get_SpinTransform_ptr()->addChild(
      clouds_group.get_OSGgroup_ptr());
   centers_group.get_SpinTransform_ptr()->addChild(
      FishnetsGroup_ptr->get_OSGgroup_ptr());
   root->addChild(centers_group.get_SpinTransform_ptr());

// Attach scene graph to viewer:

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

