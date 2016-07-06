// ========================================================================
// Program VIEW3DMOVIE is a testing grounds for viewing an animated
// sequence of 3D point clouds.  Each cloud frame is read in as a
// separate .osga file.  VIEW3DMOVIE implements Ross Anderson's
// suggestion to use an OSG Switch in order to rapidly turn on the
// current frame's point cloud while suppressing all other frames'
// clouds.

// cd to some subdir containing a set of .osga files.  From within
// this subdir, chant

// 		      path/view3Dmovie *.osga

// ========================================================================
// Last updated on 12/31/11; 1/3/12; 2/1/12; 2/4/12
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
#include "osg/osg2D/ColorbarHUD.h"
#include "osg/osgOrganization/Decorations.h"
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
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgWindow/ViewerManager.h"

#include "osg/osg2D/ArrowHUD.h"

#include "time/timefuncs.h"

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

   timefunc::initialize_timeofday_clock();
   
// Use an ArgumentParser object to manage the program arguments:
   
   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);

   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
   cout << "cloudpass_ID = " << cloudpass_ID << endl;

   string image_list_filename=passes_group.get_image_list_filename();
   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string image_sizes_filename=passes_group.get_image_sizes_filename();
   cout << "image_sizes_filename = " << image_sizes_filename << endl;

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group,image_sizes_filename);
   int n_photos=photogroup_ptr->get_n_photos();
   cout << "n_photos = " << n_photos << endl;

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("3D imagery");
   
// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

   bool display_movie_state=false;
//   bool display_movie_state=true;
//   bool display_movie_number=false;
   bool display_movie_number=true;
//   bool display_movie_world_time=false;
   bool display_movie_world_time=true;
   bool hide_Mode_HUD_flag=false;
//   bool hide_Mode_HUD_flag=true;
   Operations operations(
      ndims,window_mgr_ptr,passes_group,
      display_movie_state,display_movie_number,display_movie_world_time,
      hide_Mode_HUD_flag);
   root->addChild(operations.get_OSGgroup_ptr());

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();

// Instantiate AnimationController which acts as master clock:

   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   AnimationController_ptr->setDelay(0.25);
   AnimationController_ptr->setExtendedDelay(
      5*AnimationController_ptr->getDelay());

// Instantiate clock pointer to keep track of real time:

   Clock* clock_ptr=AnimationController_ptr->get_clock_ptr();
   clock_ptr->current_local_time_and_UTC();

// Specify start, stop and step times for master clock:

   operations.set_master_world_start_UTC(
      passes_group.get_world_start_UTC_string());
   operations.set_master_world_stop_UTC(
      passes_group.get_world_stop_UTC_string());
   operations.set_delta_master_world_time_step_per_master_frame(
      passes_group.get_world_time_step());
   operations.reset_AnimationController_world_time_params();

// Specify ImageNumber HUDs:

   ImageNumberHUD* ImageNumberHUD_ptr=operations.get_ImageNumberHUD_ptr();
   ImageNumberHUD_ptr->set_text_color(colorfunc::white);
   ImageNumberHUD_ptr->reset_text_size(0.8 , 0);
   ImageNumberHUD_ptr->reset_text_size(0.8 , 1);
   ImageNumberHUD_ptr->reset_text_size(0.8 , 2);
   ImageNumberHUD_ptr->set_display_UTC_flag(false);

/*
// Read in interpolated volumes generated by program MORPH:

   string volume_filename="./plume_results/5C/interp_vols.dat";
//   string volume_filename="interpolated_volumes.dat";
   filefunc::ReadInfile(volume_filename);

   vector<double> interpolated_volumes;
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      interpolated_volumes.push_back(column_values[1]);
   }

   cout << "interp vols.size() = " << interpolated_volumes.size()
        << " AC.nframes = " << AnimationController_ptr->get_nframes()
        << endl;

   for (int i=0; i<AnimationController_ptr->get_nframes(); i++)
   {
      string curr_label="Reconstructed volume: "+stringfunc::number_to_string(
         interpolated_volumes[i],0)+" cubic meters";
      ImageNumberHUD_ptr->push_back_label(curr_label);
   }
*/

// Add a custom manipulator to the event handler list:

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);
   CM_3D_ptr->set_min_camera_height_above_grid(0.1);	// meters
   CM_3D_ptr->set_enable_underneath_looking_flag(true);	
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold all decorations:
   
   Decorations decorations(window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

// Instantiate Grid:

   AlirtGrid* grid_ptr=decorations.add_AlirtGrid(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));

   threevector* grid_origin_ptr=grid_ptr->get_world_origin_ptr();
   CM_3D_ptr->set_Grid_ptr(grid_ptr);
   decorations.set_grid_origin_ptr(grid_origin_ptr);

   grid_ptr->initialize_ALIRT_grid(-25,25,-25,25,0);	// Expt 2B
   grid_ptr->set_delta_xy(5,5);
   grid_ptr->set_axis_char_label_size(5);
   grid_ptr->set_tick_char_label_size(2);

//   grid_ptr->initialize_ALIRT_grid(-50,50,-50,50,0); // Expt 5C
//   grid_ptr->set_delta_xy(10,10);
//   grid_ptr->set_axis_char_label_size(10);
//   grid_ptr->set_tick_char_label_size(4);
   grid_ptr->set_axes_labels(
      "Relative East (meters)","Relative North (meters)");
   grid_ptr->update_grid();
      
//   cout << "*grid_origin_ptr = " << *grid_origin_ptr << endl;

// Instantiate group to hold pointcloud information:

   PointCloudsGroup clouds_group(
      passes_group.get_pass_ptr(cloudpass_ID),
      AnimationController_ptr,grid_origin_ptr);
   clouds_group.set_time_dependence_flag(true);

   clouds_group.generate_separate_clouds_from_input_files(
      passes_group.get_pass_ptr(cloudpass_ID));
   clouds_group.add_Data_Nodes_to_Switch();

   clouds_group.set_prob_color_map(6);	// grey scale
   clouds_group.get_ColorGeodeVisitor_ptr()->
      set_probabilities_magnification(2);

   int n_clouds=clouds_group.get_n_Graphicals();
   cout << "n_clouds = " << n_clouds << endl;
   AnimationController_ptr->set_nframes(n_clouds);

   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_ptr,CM_3D_ptr));

// Instantiate a MyDatabasePager to handle paging of files from disk:

   viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager(
      clouds_group.get_SetupGeomVisitor_ptr(),
      clouds_group.get_ColorGeodeVisitor_ptr());
   clouds_group.get_HiresDataVisitor_ptr()->setDatabasePager(
      MyDatabasePager_ptr);

// Reset extremal z ColorMap values and thresholds based upon hyper
// bounding box values for ALL input point clouds.  Also set height
// color map for all point clouds to reverse large hue value sans white:

   double zmin=clouds_group.get_hyper_bbox().zMin();
   double zmax=clouds_group.get_hyper_bbox().zMax();

// FAKE FAKE:  Weds Feb 1, 2012 at 2:41 pm
// Hardwire zmin & zmax values which are reasonable for Expt 2B

   zmin=0;	// meters
   zmax=2;	// meters

   for (int p=0; p<clouds_group.get_n_Graphicals(); p++)
   {
      PointCloud* cloud_ptr=clouds_group.get_Cloud_ptr(p);
      ColorMap* height_ColorMap_ptr=cloud_ptr->get_z_ColorMap_ptr();
      ColorMap* prob_ColorMap_ptr=cloud_ptr->get_p_ColorMap_ptr();
      
      height_ColorMap_ptr->set_min_value(2,zmin);
      height_ColorMap_ptr->set_min_threshold(2,zmin);
      height_ColorMap_ptr->set_max_value(2,zmax);
      height_ColorMap_ptr->set_max_threshold(2,zmax);
      height_ColorMap_ptr->set_mapnumber(14); 

//      prob_ColorMap_ptr->set_mapnumber(6);	// grey
//      prob_ColorMap_ptr->set_mapnumber(7);	// pure_hue
//      prob_ColorMap_ptr->set_mapnumber(14); // reverse large hue value wo white
      
   }

// Instantiate color bar HUD:

   double hue_start=300;	// purple
   double hue_stop=0;		// red
   double scalar_value_start=zmin;
   double scalar_value_stop=zmax;
   string title="Elevation above ground (meters)";
   ColorbarHUD* ColorbarHUD_ptr=new ColorbarHUD(
      hue_start,hue_stop,scalar_value_start,scalar_value_stop,title);
   ColorbarHUD_ptr->set_truncate_value_flag(false);

   clouds_group.set_ColorbarHUD_ptr(ColorbarHUD_ptr);
//   root->addChild(ColorbarHUD_ptr->getProjection());

// Globally set maximum z threshold which applies to all imported
// point clouds:

   double max_z_threshold=
      passes_group.get_pass_ptr(cloudpass_ID)->get_PassInfo_ptr()->
      get_max_threshold(2);
   clouds_group.set_max_threshold(max_z_threshold);

// Instantiate a PointFinder;

   PointFinder pointfinder(&clouds_group);
   CM_3D_ptr->set_PointFinder(&pointfinder);

// Instantiate signpost decoration groups:

   SignPostsGroup* SignPostsGroup_ptr=
      decorations.add_SignPosts(ndims,passes_group.get_pass_ptr(cloudpass_ID));
//   SignPostsGroup_ptr->set_ladar_height_data_flag(true);
//   SignPostsGroup_ptr->set_CM_3D_ptr(CM_3D_ptr);
   SignPostsGroup_ptr->set_altitude_dependent_size_flag(false);

// Instantiate Polyhedra decoration groups:

   PolyhedraGroup* PolyhedraGroup_ptr=decorations.add_Polyhedra(
      passes_group.get_pass_ptr(cloudpass_ID));
   PolyhedraGroup_ptr->set_OFF_subdir(
      "/home/cho/programs/c++/svn/projects/src/mains/plume/OFF/");

// Instantiate OBSFRUSTA decoration group:

   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr=decorations.add_OBSFRUSTA(
      passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
   OBSFRUSTAGROUP_ptr->set_enable_OBSFRUSTA_blinking_flag(false);
   OBSFRUSTAGROUP_ptr->set_erase_other_OBSFRUSTA_flag(true);

// Instantiate an individual OBSFRUSTUM for every photograph:

   bool multicolor_frusta_flag=false;
   bool thumbnails_flag=true;
//   OBSFRUSTAGROUP_ptr->generate_still_imagery_frusta_for_photogroup(
//      photogroup_ptr,multicolor_frusta_flag,thumbnails_flag);

   double frustum_sidelength=7.5;
   double movie_downrange_distance=-1;
   OBSFRUSTAGROUP_ptr->generate_still_imagery_frusta_for_photogroup(
      photogroup_ptr,frustum_sidelength,movie_downrange_distance,
      multicolor_frusta_flag,thumbnails_flag);

   decorations.get_OBSFRUSTUMPickHandler_ptr()->
      set_mask_nonselected_OSGsubPATs_flag(true);
   decorations.get_OBSFRUSTAKeyHandler_ptr()->set_SignPostsGroup_ptr(
      SignPostsGroup_ptr);

// ========================================================================
// Mark 3D locations of November 2011 grid sensors using colored
// Geometricals (hemispheres, boxes, cylinders, cones)
// ========================================================================

// Constants

   const double feet_per_meter=3.2808;
   double alpha=0.99;

// Repeated variable declarations

   double dz;
   vector<threevector> XYZ;

   double thetax=0;
   double thetay=0;
   double thetaz=0*PI/180;

//   cout << "Enter theta_z in degs:" << endl;
//   cin >> thetaz;

   thetaz=0;  // Angle needed to align sensors grid with recon pt cloud
//   thetaz=39;  // Angle needed to align sensors grid with recon pt cloud

   thetaz *= PI/180;
   rotation R(thetax,thetay,thetaz);

   double delta_x,delta_y;

//   cout << "Enter delta x in meters:" << endl;
//   cin >> delta_x;
//   cout << "Enter delta y in meters:" << endl;
//   cin >> delta_y;

// Average position for all 10 reconstructed camera tripods determined
// for Day 5 on 12/16/2011:

   threevector avg_camera_posn(0.080055 , 0.377872 , 0.043724 ) ;

   threevector sensor_grid_center=avg_camera_posn+threevector(delta_x,delta_y);
   cout << "sensor grid center = " << sensor_grid_center << endl;

// Instantiate SphereSegments decoration group:

   SphereSegmentsGroup* SphereSegmentsGroup_ptr=
      decorations.add_SphereSegments(
         passes_group.get_pass_ptr(cloudpass_ID));
   
   dz=0;

   XYZ.clear();
   XYZ.push_back(threevector(-6,-6,dz));
   XYZ.push_back(threevector(0,-6,dz));
   XYZ.push_back(threevector(6,-6,dz));
   XYZ.push_back(threevector(-6,0,dz));
   XYZ.push_back(threevector(6,0,dz));
   XYZ.push_back(threevector(-6,6,dz));
   XYZ.push_back(threevector(0,6,dz));
   XYZ.push_back(threevector(6,6,dz));
   
   colorfunc::Color hemisphere_color=colorfunc::yegr;
   double hemisphere_radius=0.5;
   for (int i=0; i<XYZ.size(); i++)
   {
      threevector rel_hemisphere_posn(R*XYZ[i]);
      threevector hemisphere_posn=sensor_grid_center+rel_hemisphere_posn;
      SphereSegment* SphereSegment_ptr=SphereSegmentsGroup_ptr->
         generate_new_hemisphere(hemisphere_radius,hemisphere_posn);
      SphereSegment_ptr->set_permanent_color(colorfunc::get_OSG_color(
         hemisphere_color,alpha));
   }

// Instantiate boxes decoration group:

   BoxesGroup* BoxesGroup_ptr=decorations.add_Boxes(
      passes_group.get_pass_ptr(cloudpass_ID));
   BoxesGroup_ptr->set_wlh(0.5,0.5,0.5);

   dz=0 /feet_per_meter;	// meters

   XYZ.clear();
   XYZ.push_back(threevector(-16,-16,dz));
   XYZ.push_back(threevector(16,-16,dz));

   XYZ.push_back(threevector(-10,0,dz));
   XYZ.push_back(threevector(0,10,dz));
   XYZ.push_back(threevector(0,-10,dz));
   XYZ.push_back(threevector(10,0,dz));

   XYZ.push_back(threevector(16,16,dz));
   XYZ.push_back(threevector(-16,16,dz));

   XYZ.push_back(threevector(3,0,dz));
   XYZ.push_back(threevector(-3,0,dz));
   XYZ.push_back(threevector(0,3,dz));
   XYZ.push_back(threevector(0,-3,dz));

   colorfunc::Color box_color=colorfunc::red;
   for (int i=0; i<XYZ.size(); i++)
   {
      threevector rel_box_posn(R*XYZ[i]);
      threevector box_center=sensor_grid_center+rel_box_posn;
      Box* Box_ptr=BoxesGroup_ptr->generate_new_Box(box_center);
      Box_ptr->set_permanent_color(colorfunc::get_OSG_color(
         box_color,alpha));
   }

// Instantiate cylinders decoration group:

   CylindersGroup* CylindersGroup_ptr=
      decorations.add_Cylinders(passes_group.get_pass_ptr(cloudpass_ID),
                                AnimationController_ptr);
   osg::Quat q(0,0,0,1);

   double cyl_height=10 /feet_per_meter;	// meters
   CylindersGroup_ptr->set_rh(0.5,0.5*cyl_height);

   XYZ.clear();
   XYZ.push_back(threevector(-12,-12,0));
   XYZ.push_back(threevector(0,-12,0));
   XYZ.push_back(threevector(12,-12,0));
   XYZ.push_back(threevector(-12,0,0));
   XYZ.push_back(threevector(12,0,0));
   XYZ.push_back(threevector(12,12,0));
   XYZ.push_back(threevector(0,12,0));
   XYZ.push_back(threevector(-12,12,0));

   colorfunc::Color cyl_color=colorfunc::blgr;
   for (int i=0; i<XYZ.size(); i++)
   {
      threevector rel_cyl_posn(R*XYZ[i]);
      threevector cyl_center=sensor_grid_center+rel_cyl_posn+0.5*cyl_height*
         z_hat;
      Cylinder* Cylinder_ptr=CylindersGroup_ptr->generate_new_Cylinder(
         cyl_center,q,cyl_color);
      Cylinder_ptr->set_permanent_color(
         colorfunc::get_OSG_color(cyl_color,alpha));
   }

// Instantiate cones decoration group:

   ConesGroup* ConesGroup_ptr=
      decorations.add_Cones(passes_group.get_pass_ptr(cloudpass_ID));

   double cone_height=10 /feet_per_meter;	// meters
   double cone_radius=0.5;
   ConesGroup_ptr->set_rh(cone_radius,cone_height);

   XYZ.clear();
   XYZ.push_back(threevector(0,0,0));		// Marks sensor grid center

   XYZ.push_back(threevector(-20,-20,0));
   XYZ.push_back(threevector(-7,-20,0));
   XYZ.push_back(threevector(7,-20,0));
   XYZ.push_back(threevector(20,-20,0));

   XYZ.push_back(threevector(-12,12,0));
   XYZ.push_back(threevector(0,12,0));
   XYZ.push_back(threevector(12,12,0));

   XYZ.push_back(threevector(-20,7,0));
   XYZ.push_back(threevector(20,7,0));

   XYZ.push_back(threevector(-12,0,0));
   XYZ.push_back(threevector(12,0,0));

   XYZ.push_back(threevector(-20,-7,0));
   XYZ.push_back(threevector(20,-7,0));

   XYZ.push_back(threevector(-12,-12,0));
   XYZ.push_back(threevector(0,-12,0));
   XYZ.push_back(threevector(12,-12,0));

   XYZ.push_back(threevector(20,20,0));
   XYZ.push_back(threevector(-7,20,0));
   XYZ.push_back(threevector(7,20,0));
   XYZ.push_back(threevector(-20,20,0));

   colorfunc::Color cone_color=colorfunc::yellow;
   for (int i=0; i<XYZ.size(); i++)
   {
      if (i > 0)
      {
         cone_color=colorfunc::blue;	// 5' particle counters
      }

      threevector rel_cone_posn(R*XYZ[i]);
      threevector cone_base=rel_cone_posn;
      threevector cone_tip=cone_base+threevector(0,0,cone_height);

      Cone* Cone_ptr=ConesGroup_ptr->generate_new_Cone();
      Cone_ptr->set_theta(PI);

      threevector trans=cone_tip;
      trans += sensor_grid_center - *grid_origin_ptr;

      Cone_ptr->scale_rotate_and_then_translate(
         ConesGroup_ptr->get_curr_t(),ConesGroup_ptr->get_passnumber(),trans);

      Cone_ptr->set_color(
         colorfunc::get_OSG_color(cone_color,alpha));
   } 

// ========================================================================

// Attach all 3D graphicals to CentersGroup SpinTransform which can
// perform global rotation about some axis coming out of user selected
// center location:

   root->addChild(decorations.get_OSGgroup_ptr());
   root->addChild(clouds_group.get_OSGgroup_ptr());
   root->addChild(clouds_group.get_Switch_ptr());

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

