// ========================================================================
// Program PANOVSLADAR displays WISP panoramic imagery within 10 aligned
// 3D OBSFRUSTA.  If the OBSFRUSTA have been previously geoaligned via
// program GEOREGPANO, PANOVSLADAR can be used to inspect the
// registration match between the WISP video and a background ladar
// point cloud.

/*

./panovsladar \
--region_filename ./packages/miniHAFB.pkg \
--region_filename ./packages/frame_p8_0000.pkg \
--region_filename ./packages/frame_p9_0000.pkg \
--region_filename ./packages/frame_p0_0000.pkg \
--region_filename ./packages/frame_p1_0000.pkg \
--region_filename ./packages/frame_p2_0000.pkg \
--region_filename ./packages/frame_p3_0000.pkg \
--region_filename ./packages/frame_p4_0000.pkg \
--region_filename ./packages/frame_p5_0000.pkg \
--region_filename ./packages/frame_p6_0000.pkg \
--region_filename ./packages/frame_p7_0000.pkg \
--world_start_UTC 2010,7,10,10,6,3 \
--world_stop_UTC 2010,7,10,10,6,46 \
--world_time_step 0.2 \
--initial_mode Manipulate_Fused_Data_Mode 

*/

// ========================================================================
// Last updated on 2/27/11; 5/30/11; 6/5/11
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include "osg/osgGraphicals/AnimationController.h"
#include "video/camera.h"
#include "osg/CompassHUD.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "osg/osgGrid/GridKeyHandler.h"
#include "osg/osgSceneGraph/HiresDataVisitor.h"
#include "osg/osgGrid/LatLongGridsGroup.h"
#include "osg/ModeController.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osg2D/MovieKeyHandler.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgWindow/ViewerManager.h"

using std::cin;
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
   int videopass_ID=passes_group.get_videopass_ID();
   vector<int> GISlayer_IDs=passes_group.get_GISlayer_IDs();
//   cout << "cloudpass_ID = " << cloudpass_ID << endl;
//   cout << "videopass_ID = " << videopass_ID << endl;

// Instantiate postgis database objects to send data to and retrieve
// data from external Postgres database:

   postgis_databases_group* postgis_databases_group_ptr=
      new postgis_databases_group;
   postgis_database* postgis_db_ptr=postgis_databases_group_ptr->
      generate_postgis_database_from_GISlayer_IDs(passes_group,GISlayer_IDs);

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group);
   int n_photos=photogroup_ptr->get_n_photos();
   cout << "n_photos = " << n_photos << endl;

   photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(0);
   camera* camera_ptr=photo_ptr->get_camera_ptr();

/*
   int n_horiz_pixels=233;
   int n_vertical_pixels=276;
   double FOV_u=(180/5)*PI/180;
   double aspect_ratio=double(n_horiz_pixels)/double(n_vertical_pixels);
   double FOV_v=camerafunc::vert_FOV_from_horiz_FOV_and_aspect_ratio(
      FOV_u,aspect_ratio);
   cout << "FOV_v = " << FOV_v*180/PI << endl;

   double f;
   camerafunc::f_and_aspect_ratio_from_horiz_vert_FOVs(
      FOV_u,FOV_v,f,aspect_ratio);
   cout << "Correct f = " << f << endl;
   cout << "aspect_ratio = " << aspect_ratio << endl;
   cout << "horiz/vert pixels = " << n_horiz_pixels/double(n_vertical_pixels)
        << endl;
*/

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("3D imagery");
//   window_mgr_ptr->getUsage(*arguments.getApplicationUsage());

// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

//   bool display_movie_state=true;
//   bool display_movie_number=true;
//   bool display_movie_world_time=true;
   bool display_movie_state=false;		// viewgraphs
   bool display_movie_number=false;		// viewgraphs
   bool display_movie_world_time=false;       // viewgraphs
   bool hide_Mode_HUD_flag=false;
//   bool hide_Mode_HUD_flag=true;

   Operations operations(
      ndims,window_mgr_ptr,passes_group,display_movie_state,
      display_movie_number,display_movie_world_time,hide_Mode_HUD_flag);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();

// Instantiate AnimationController which acts as master game clock:

   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   AnimationController_ptr->setDelay(0.1);

// Specify start, stop and step times for master game clock:

   operations.set_master_world_start_UTC(
      passes_group.get_world_start_UTC_string());
   operations.set_master_world_stop_UTC(
      passes_group.get_world_stop_UTC_string());
   operations.set_delta_master_world_time_step_per_master_frame(
      passes_group.get_world_time_step());

   AnimationController_ptr->set_world_time_params(
      operations.get_master_world_start_time(),
      operations.get_master_world_stop_time(),
      operations.get_delta_master_world_time_step_per_master_frame());

// Add a custom manipulator to the event handler list:

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

//    HiresDataVisitor* HiresDataVisitor_ptr=new HiresDataVisitor();

// FAKE FAKE:  Mon Feb 28, 2011 at 10:34 am
// Comment out compass for 3D blob tracking movie generation purposes.

/*
// Instantiate compass:

   CompassHUD* CompassHUD_ptr=new CompassHUD(colorfunc::white);
   root->addChild(CompassHUD_ptr->getProjection());
   CM_3D_ptr->set_CompassHUD_ptr(CompassHUD_ptr);
*/

// Instantiate group to hold all decorations:
   
   Decorations decorations(
      window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

// Instantiate groups to hold multiple point clouds,
// latitude-longitude grids and associated earth regions:

   PointCloudsGroup clouds_group(
      passes_group.get_pass_ptr(cloudpass_ID));
   clouds_group.set_Terrain_Manipulator_ptr(CM_3D_ptr);
//   clouds_group.set_auto_resize_points_flag(false);
   clouds_group.set_auto_resize_points_flag(true);
   decorations.set_PointCloudsGroup_ptr(&clouds_group);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_ptr,CM_3D_ptr));

   LatLongGridsGroup latlonggrids_group(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),CM_3D_ptr);
   EarthRegionsGroup earth_regions_group(
      passes_group.get_pass_ptr(cloudpass_ID),      
      &clouds_group,&latlonggrids_group);
   earth_regions_group.generate_regions(passes_group);

   LatLongGrid* LatLongGrid_ptr=latlonggrids_group.get_Grid_ptr(0);
   threevector* grid_origin_ptr=LatLongGrid_ptr->get_world_origin_ptr();
//   cout << "*grid_origin_ptr = " << *grid_origin_ptr << endl;
   CM_3D_ptr->set_Grid_ptr(LatLongGrid_ptr);

   decorations.set_grid_origin_ptr(grid_origin_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new GridKeyHandler(ModeController_ptr,LatLongGrid_ptr));

   double xmin=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().xMin();
   double xmax=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().xMax();
   double ymin=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().yMin();
   double ymax=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().yMax();
   if (postgis_db_ptr != NULL)
      postgis_db_ptr->pushback_gis_bbox(xmin,xmax,ymin,ymax);

// Instantiate a PointFinder;

   PointFinder pointfinder;
   CM_3D_ptr->set_PointFinder(&pointfinder);

// Instantiate feature decoration groups:

   decorations.add_Features(ndims,passes_group.get_pass_ptr(cloudpass_ID));
   decorations.get_FeaturePickHandler_ptr()->
      set_surface_picking_flag(false);
   decorations.get_FeaturePickHandler_ptr()->
      set_Zplane_picking_flag(false);
   
// Instantiate a MyDatabasePager to handle paging of files from disk:

   viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager(
      clouds_group.get_SetupGeomVisitor_ptr(),
      clouds_group.get_ColorGeodeVisitor_ptr());
   clouds_group.get_HiresDataVisitor_ptr()->setDatabasePager(
      MyDatabasePager_ptr);

// Instantiate OBSFRUSTAGROUP decoration group:

   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr=decorations.add_OBSFRUSTA(
      passes_group.get_pass_ptr(0),AnimationController_ptr);

// Set OBSFRUSTAGROUP's ground plane equal to Z-plane with z=11.2
// meters (= typical altitude in HAFB minimap for roads nearby HAFB
// flight facilty):

   threevector HAFB_flightfacility_posn(0,0,11.2);
   plane groundplane(z_hat,HAFB_flightfacility_posn);
   OBSFRUSTAGROUP_ptr->set_groundplane_pi(groundplane.get_pi());

// Instantiate an individual OBSFRUSTUM for every input video.  Each
// contains a separate movie object.

   threevector camera_posn=camera_ptr->get_world_posn();

//   double frustum_sidelength=-1;
   double frustum_sidelength=18;
   double movie_downrange_distance=-1;

// Instantiate group to hold movie:

   MoviesGroup movies_group(
      ndims,passes_group.get_pass_ptr(videopass_ID),
      AnimationController_ptr);
   cout << "In main, &movies_group = " << &movies_group << endl;

   string movie_filename=
      passes_group.get_videopass_ptr()->get_first_filename();
   cout << "movie_filename = " << movie_filename << endl;
   string image_subdir=filefunc::getdirname(movie_filename);
   if (image_subdir.size()==0) image_subdir="./";
   cout << "image_subdir = " << image_subdir << endl;
   AnimationController_ptr->store_ordered_image_filenames(image_subdir);

   threevector rotation_origin(0,0,0);
   threevector global_camera_translation(0,0,0);
   double global_daz=0*PI/180;
   double global_del=0*PI/180;
   double global_droll=0*PI/180;
   double local_spin_daz=0*PI/180;
   bool multicolor_frusta_flag=false;
   bool thumbnails_flag=false;

   OBSFRUSTAGROUP_ptr->generate_still_imagery_frusta_for_photogroup(
      photogroup_ptr,photogroup_ptr->get_n_photos(),
      camera_ptr->get_world_posn(),global_camera_translation,
      global_daz,global_del,global_droll,rotation_origin,local_spin_daz,
      frustum_sidelength,movie_downrange_distance,multicolor_frusta_flag,
      thumbnails_flag);

// Scan through subdirectory containing video frames.  Set minimum and
// maximum photo numbers based upon the files' name:

   int min_photo_number=-1;
   int max_photo_number=-1;
   videofunc::find_min_max_photo_numbers(
      image_subdir,min_photo_number,max_photo_number);
   cout << "min_photo_number = " << min_photo_number
        << " max_photo_number = " << max_photo_number << endl;

   int Nimages=max_photo_number-min_photo_number+1;
   AnimationController_ptr->set_nframes(Nimages);
   for (unsigned int n=0; n<OBSFRUSTAGROUP_ptr->get_n_Graphicals(); n++)
   {
      OBSFRUSTUM* OBSFRUSTUM_ptr=OBSFRUSTAGROUP_ptr->get_OBSFRUSTUM_ptr(n);
      Movie* movie_ptr=OBSFRUSTUM_ptr->get_Movie_ptr();
      movie_ptr->get_texture_rectangle_ptr()->set_first_frame_to_display(
         min_photo_number);
      movie_ptr->get_texture_rectangle_ptr()->
         set_last_frame_to_display(max_photo_number);

      int panel_number=(8+n)%10;	// WISP panels

      movie_ptr->get_texture_rectangle_ptr()->set_panel_number(panel_number);

// Read in tracks generated by Luke Skelly for 5 separate OBSFRUSTA:

//      movie_ptr->generate_Luke_blob_tracks(OBSFRUSTUM_ptr->get_ID());
   }

   MovieKeyHandler* MoviesKeyHandler_ptr=
      new MovieKeyHandler(ModeController_ptr,&movies_group);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(MoviesKeyHandler_ptr);
  
// Set mask_nonselected_OSGsubPATs_flag=false in order to
// simultaneously display OSGsubPATs 0 and 1 respectively containing
// static panorama and dynamic video images:

   decorations.get_OBSFRUSTUMPickHandler_ptr()->
      set_mask_nonselected_OSGsubPATs_flag(false);

/*
   SignPostsGroup* SignPostsGroup_ptr=decorations.add_SignPosts(
      ndims,passes_group.get_pass_ptr(0));
   SignPostsGroup_ptr->set_common_geometrical_size(0.03);

   twovector UV0(0.5 , 0.5);
   SignPost* SignPost0_ptr=OBSFRUSTAGROUP_ptr->
      generate_SignPost_at_imageplane_location(UV0,0,SignPostsGroup_ptr);

   SignPost0_ptr->set_label("Building one");
//   double extra_frac_cyl_height=0.5;
//   SignPost_ptr->set_label("Building one",extra_frac_cyl_height);
//   SignPost0_ptr->set_max_text_width("Build");

   twovector UV1(0.75 , 0.75);
   SignPost* SignPost1_ptr=OBSFRUSTAGROUP_ptr->
      generate_SignPost_at_imageplane_location(UV1,1,SignPostsGroup_ptr);
   SignPost1_ptr->set_label("Building two is a big structure");

//   double extra_frac_cyl_height=0.5;
//   SignPost_ptr->set_label("Building one",extra_frac_cyl_height);
//   SignPost1_ptr->set_max_text_width("Build");
*/

// Optimize the scene graph, remove redundent nodes and states, and
// then attach it to the viewer:

//   osgUtil::Optimizer optimizer;
//   optimizer.optimize(root);

   root->addChild(operations.get_OSGgroup_ptr());
   root->addChild(decorations.get_OSGgroup_ptr());
   root->addChild( movies_group.get_OSGgroup_ptr() );
   root->addChild(clouds_group.get_OSGgroup_ptr());
   root->addChild(latlonggrids_group.get_OSGgroup_ptr());

   window_mgr_ptr->setSceneData(root);

// The HAFB ALIRT minimap has very dark intensity values.  So for
// intensity display purposes, we artificially brighten all p-values
// by the following intensity magnification factor:

   double intensity_magnification=5.0;
   clouds_group.get_ColorGeodeVisitor_ptr()->
      set_probabilities_magnification(intensity_magnification);

// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_ptr->realize();

   while( !window_mgr_ptr->done() )
   {
      window_mgr_ptr->process();
   }

   delete window_mgr_ptr;
}

