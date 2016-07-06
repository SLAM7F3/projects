// ========================================================================
// Program VIEWTRIPODS is a specialized version of
// photosynth/VIEWBUNDLER which we use to view reconstructed (and
// preferably georegistered) tripod camera frusta, tripod camera
// SignPost labels and South Carolina wood post locations.
// ========================================================================
// Last updated on 1/3/13; 1/6/13; 1/16/13
// ========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <osgDB/FileUtils>

#include "osg/osgGraphicals/AnimationController.h"
#include "geometry/bounding_box.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CentersKeyHandler.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgSceneGraph/HiresDataVisitor.h"
#include "messenger/Messenger.h"
#include "osg/ModeController.h"
#include "osg/osgModels/MODELSGROUP.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGeometry/PointsGroup.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgGIS/postgis_database.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "time/timefuncs.h"
#include "video/videofuncs.h"
#include "osg/osgWindow/ViewerManager.h"

#include "general/outputfuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::map;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Constants

   const double feet_per_meter=3.2808;
   double alpha=0.5;

// Repeated variable declarations

   double dz;
   vector<threevector> XYZ;

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);

   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
   int texturepass_ID=passes_group.get_curr_texturepass_ID();
   int videopass_ID=passes_group.get_videopass_ID();

   cout << "cloudpass_ID = " << cloudpass_ID
        << " texturepass_ID = " << texturepass_ID 
        << " videopass_ID = " << videopass_ID 
        << endl;

   string image_list_filename=passes_group.get_image_list_filename();
   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;

   string image_sizes_filename=passes_group.get_image_sizes_filename();
   cout << "image_sizes_filename = " << image_sizes_filename << endl;
   string xyz_points_filename=passes_group.get_xyz_points_filename();
   cout << "xyz_points_filename = " << xyz_points_filename << endl;
//   string common_planes_filename=passes_group.get_common_planes_filename();
//   cout << "common_planes_filename = " << common_planes_filename << endl;

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group,image_sizes_filename);
   photogroup_ptr->set_bundler_IO_subdir(bundler_IO_subdir);
   int n_photos=photogroup_ptr->get_n_photos();
   cout << "n_photos = " << n_photos << endl;

   for (int n=0; n<n_photos; n++)
   {
      photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(n);
      camera* camera_ptr=photo_ptr->get_camera_ptr();
//      cout << "n = " << n
//           << " camera = " << *camera_ptr 
//           << " camera posn = " << camera_ptr->get_world_posn()
//           << endl;

      cout << "n = " << n << " photo_filename = " 
           << photo_ptr->get_filename() << endl;
   }

// Read in reconstructed XYZ points plus their IDs along with visible
// camera IDs into videofunc map *xyz_map_ptr:

   videofunc::CAMERAID_XYZ_MAP* cameraid_xyz_map_ptr=NULL;
   if (xyz_points_filename.size() > 0)
   {
      videofunc::import_reconstructed_XYZ_points(xyz_points_filename);

//  Generate STL map containing STL vectors of reconstructed XYZ
//  points as a function of visible camera ID:

//      cameraid_xyz_map_ptr=videofunc::sort_XYZ_points_by_camera_ID();
   }

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("3D imagery");

// Instantiate separate messengers for each Decorations group which
// needs to receive mail:

   int pass_ID=passes_group.get_n_passes()-1;
   string broker_URL=passes_group.get_pass_ptr(pass_ID)->
      get_PassInfo_ptr()->get_ActiveMQ_hostname();
   cout << "ActiveMQ broker_URL = " << broker_URL << endl;

   string message_sender_ID="VIEWBUNDLER";

// Instantiate urban network messengers for communication with Michael
// Yee's social network tool:

   string photo_network_message_queue_channel_name="photo_network";
   Messenger OBSFRUSTA_photo_network_messenger( 
      broker_URL,photo_network_message_queue_channel_name,message_sender_ID);

   string GPS_message_queue_channel_name="viewer_update";
   Messenger GPS_messenger( 
      broker_URL, GPS_message_queue_channel_name,message_sender_ID);

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
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   AnimationController_ptr->setDelay(2);

   root->addChild(operations.get_OSGgroup_ptr());

// Add a custom manipulator to the event handler list:

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold all decorations:

   Decorations decorations(
      window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

// Instantiate Grid:

   AlirtGrid* grid_ptr=decorations.add_AlirtGrid(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   threevector* grid_origin_ptr=grid_ptr->get_world_origin_ptr();

   CM_3D_ptr->set_Grid_ptr(grid_ptr);
   decorations.set_grid_origin_ptr(grid_origin_ptr);
   
// Instantiate group to hold pointcloud information:

   PointCloudsGroup clouds_group(
      passes_group.get_pass_ptr(cloudpass_ID),grid_origin_ptr);
   vector<PointCloud*>* cloudptrs_ptr=
      clouds_group.generate_Clouds(passes_group);

   clouds_group.set_Terrain_Manipulator_ptr(CM_3D_ptr);
   clouds_group.set_auto_resize_points_flag(false);
   decorations.set_PointCloudsGroup_ptr(&clouds_group);

   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_ptr,CM_3D_ptr));

// Set ALIRT grid so that it has constant size independent of cloud's
// bounding box:

   double z_grid=0;	// meters
   decorations.get_AlirtGridsGroup_ptr()->initialize_grid(
      grid_ptr,-30,30,-30,30,z_grid);
//   cout << "*grid_origin_ptr = " << *grid_origin_ptr << endl;

// Instantiate a PointFinder;

   PointFinder pointfinder(&clouds_group);
   if (!passes_group.get_pick_points_on_Zplane_flag())
   {
      CM_3D_ptr->set_PointFinder(&pointfinder);
   }
   
// Instantiate a MyDatabasePager to handle paging of files from disk:

   viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager(
      clouds_group.get_SetupGeomVisitor_ptr(),
      clouds_group.get_ColorGeodeVisitor_ptr());
   clouds_group.get_HiresDataVisitor_ptr()->setDatabasePager(
      MyDatabasePager_ptr);

// Instantiate signpost decoration group:

   SignPostsGroup* SignPostsGroup_ptr=decorations.add_SignPosts(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   SignPostsGroup_ptr->set_permanent_colorfunc_color(colorfunc::red);
   SignPostsGroup_ptr->set_colors(
      colorfunc::white,colorfunc::blue);
//      colorfunc::red,colorfunc::blue);	// GE background
   SignPostsGroup_ptr->update_colors();

// Instantiate OBSFRUSTA decoration group:

   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr=decorations.add_OBSFRUSTA(
      passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
   OBSFRUSTAGROUP_ptr->pushback_Messenger_ptr(
      &OBSFRUSTA_photo_network_messenger);
//   OBSFRUSTAGROUP_ptr->set_enable_OBSFRUSTA_blinking_flag(true);
   OBSFRUSTAGROUP_ptr->set_enable_OBSFRUSTA_blinking_flag(false);
   OBSFRUSTAGROUP_ptr->set_erase_other_OBSFRUSTA_flag(true);
//   OBSFRUSTAGROUP_ptr->set_erase_other_OBSFRUSTA_flag(false);
//   OBSFRUSTAGROUP_ptr->set_cameraid_xyz_map_ptr(cameraid_xyz_map_ptr);

// Instantiate an individual OBSFRUSTUM for every photograph:

   bool multicolor_frusta_flag=false;
   bool thumbnails_flag=true;
   colorfunc::Color OBSFRUSTUM_color=colorfunc::blue;
   
   threevector global_camera_translation(0,0,0);	// bundler cloud
//   threevector global_camera_translation(0,0,1);	// reconstructed cloud
   OBSFRUSTAGROUP_ptr->generate_still_imagery_frusta_for_photogroup(
      photogroup_ptr,global_camera_translation,0,0,0,Zero_vector,0,
      OBSFRUSTUM_color,thumbnails_flag);

   // multicolor_frusta_flag,thumbnails_flag);

   decorations.get_OBSFRUSTUMPickHandler_ptr()->
      set_mask_nonselected_OSGsubPATs_flag(true);
   decorations.get_OBSFRUSTAKeyHandler_ptr()->set_SignPostsGroup_ptr(
      SignPostsGroup_ptr);

/*
// Draw rays through middle wooden post 2D features:

// middle red post

   vector<linesegment> red_rays;
   red_rays.push_back(OBSFRUSTAGROUP_ptr->draw_ray_through_imageplane(
      0,twovector(0.4147781432,0.4396919906),100,colorfunc::red));	// V1
   red_rays.push_back(OBSFRUSTAGROUP_ptr->draw_ray_through_imageplane(
      1, twovector(0.2079612613, 0.5053676367),100,colorfunc::red));	// V2
   red_rays.push_back(OBSFRUSTAGROUP_ptr->draw_ray_through_imageplane(
      4,twovector(1.649536371,0.578313231),100,colorfunc::red));	// V5
   red_rays.push_back(OBSFRUSTAGROUP_ptr->draw_ray_through_imageplane(
      7,twovector(1.155983686,0.4830408096),100,colorfunc::red));	// V9
//   red_rays.push_back(OBSFRUSTAGROUP_ptr->draw_ray_through_imageplane(
//      8,twovector(0.7495135069,0.4759723544),100,colorfunc::red));	// V10
 
   threevector red_post_posn=geometry_func::multi_line_intersection_point(
      red_rays);

// middle green post

   vector<linesegment> green_rays;
   green_rays.push_back(OBSFRUSTAGROUP_ptr->draw_ray_through_imageplane(
      0,twovector(1.245699406,0.4607249498),100,colorfunc::green));	// V1
   green_rays.push_back(OBSFRUSTAGROUP_ptr->draw_ray_through_imageplane(
      1,twovector(1.066468716,0.5305445194),100,colorfunc::green));	// V2
   green_rays.push_back(OBSFRUSTAGROUP_ptr->draw_ray_through_imageplane(
      2,twovector(0.6939950585,0.4948799014),100,colorfunc::green));	// V3
   green_rays.push_back(OBSFRUSTAGROUP_ptr->draw_ray_through_imageplane(
      3,twovector(0.3255355656,0.4302573800),100,colorfunc::green));	// V4

   threevector green_post_posn=geometry_func::multi_line_intersection_point(
      green_rays);

// middle blue post

   vector<linesegment> blue_rays;
   blue_rays.push_back(OBSFRUSTAGROUP_ptr->draw_ray_through_imageplane(
      4,twovector(1.583119273,0.5397678018),100,colorfunc::blue));	// V5
   blue_rays.push_back(OBSFRUSTAGROUP_ptr->draw_ray_through_imageplane(
      5,twovector(1.311945677,0.4434888363),100,colorfunc::blue));	// V6
   blue_rays.push_back(OBSFRUSTAGROUP_ptr->draw_ray_through_imageplane(
      6,twovector(1.087366819,0.476188123),100,colorfunc::blue));	// V7
//   blue_rays.push_back(OBSFRUSTAGROUP_ptr->draw_ray_through_imageplane(
//      8,twovector(0.3313242197,0.4841722846),100,colorfunc::blue));	// V9

   threevector blue_post_posn=geometry_func::multi_line_intersection_point(
      blue_rays);

// middle orange post

   vector<linesegment> orange_rays;
   orange_rays.push_back(OBSFRUSTAGROUP_ptr->draw_ray_through_imageplane(
      2,twovector(1.658132792,0.4543587565),100,colorfunc::orange));	// V3
   orange_rays.push_back(OBSFRUSTAGROUP_ptr->draw_ray_through_imageplane(
      3,twovector(1.218947887,0.4805459976),100,colorfunc::orange));	// V4
   orange_rays.push_back(OBSFRUSTAGROUP_ptr->draw_ray_through_imageplane(
      4,twovector(0.7943426371,0.5215801001),100,colorfunc::orange));	// V5
   orange_rays.push_back(OBSFRUSTAGROUP_ptr->draw_ray_through_imageplane(
      5,twovector(0.5377812386,0.441584706),100,colorfunc::orange));	// V6
   orange_rays.push_back(OBSFRUSTAGROUP_ptr->draw_ray_through_imageplane(
      6,twovector(0.2809332013,0.4739176035),100,colorfunc::orange));	// V7

   threevector orange_post_posn=geometry_func::multi_line_intersection_point(
      orange_rays);
*/


   threevector blue_post_posn(25.39397778,23.99390857,1.13251027);	// NE
   threevector red_post_posn(24.70671364,-21.60018909,1.177241794);	// SE
   threevector green_post_posn(-23.54938968,-24.71375663,1.081088517);	// SW
   threevector orange_post_posn(-23.8489854,30.19430996,0.8385607904);	// NW

   vector<threevector> derived_post_posns;
   derived_post_posns.push_back(blue_post_posn);
   derived_post_posns.push_back(red_post_posn);
   derived_post_posns.push_back(green_post_posn);
   derived_post_posns.push_back(orange_post_posn);

   for (int d=0; d<derived_post_posns.size(); d++)
   {
      cout << "d = " << d << " derived post posn = " << derived_post_posns[d]
           << endl;
   }
   

   threevector sensor_grid_center(0,0,0);
   double thetax=0;
   double thetay=0;
   double thetaz=0*PI/180;
   rotation R(thetax,thetay,thetaz);

// Instantiate SphereSegments decoration group:

   SphereSegmentsGroup* SphereSegmentsGroup_ptr=
      decorations.add_SphereSegments(passes_group.get_pass_ptr(cloudpass_ID));
   
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

/*
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
*/

// Depict 4 sets of wooden posts via Cylinders:

   CylindersGroup_ptr->set_rh(0.1,0.5*cyl_height);

   XYZ.clear();
   XYZ.push_back(threevector(19.8848,   22.3228,   1.524));
   XYZ.push_back(threevector(22.1238,   22.392,   1.5494));
   XYZ.push_back(threevector(22.2444,   20.0636,   1.5113));
   XYZ.push_back(threevector(23.135,   -20.2825,   1.4097));
   XYZ.push_back(threevector(23.0408,   -22.2898,   1.5748));
   XYZ.push_back(threevector(20.6738,  -22.3807,   1.524));
   XYZ.push_back(threevector(-20.0271,   -22.5936,   1.4859));
   XYZ.push_back(threevector(-22.1606,   -22.6965,   1.5748));
   XYZ.push_back(threevector(-22.4681,   -20.4006,   1.5494));
   XYZ.push_back(threevector(-21.7022,   24.2473,   1.4605));
   XYZ.push_back(threevector(-21.6222,   26.5879,   1.5113));
   XYZ.push_back(threevector(-19.2814,   26.3858,   1.4478));

   colorfunc::Color cyl_color=colorfunc::brown;
   cyl_height=1.5;	// meter
   for (int i=0; i<XYZ.size(); i++)
   {
      threevector curr_XY(XYZ[i].get(0),XYZ[i].get(1));
      threevector rel_cyl_posn(R*curr_XY);
      
      threevector cyl_center=sensor_grid_center+rel_cyl_posn;
      Cylinder* Cylinder_ptr=CylindersGroup_ptr->generate_new_Cylinder(
         cyl_center,q,cyl_color);

      int c=i/3;
      cyl_color=colorfunc::get_color(c);

      if (cyl_color==colorfunc::blue) cyl_color=colorfunc::yellow;
      
      Cylinder_ptr->set_permanent_color(
         colorfunc::get_OSG_color(cyl_color,alpha));
   }

/*
// Mark multi-ray intersection points denoting derived post positions
// with colored Cylinders:

   CylindersGroup_ptr->set_rh(0.5,2*cyl_height);
   for (int i=0; i<4; i++)
   {
      cout << "i = " << i << " derived_post_posn = " << derived_post_posns[i]
           << endl;
      
      threevector curr_XY(
         derived_post_posns[i].get(0),derived_post_posns[i].get(1));
      threevector rel_cyl_posn(R*curr_XY);

      threevector cyl_center=sensor_grid_center+rel_cyl_posn;
      Cylinder* Cylinder_ptr=CylindersGroup_ptr->generate_new_Cylinder(
         cyl_center,q,cyl_color);

      int c=i+4;
      cyl_color=colorfunc::get_color(c);
      Cylinder_ptr->set_permanent_color(
         colorfunc::get_OSG_color(cyl_color,alpha));
   }
*/

// Instantiate cones decoration group:

   ConesGroup* ConesGroup_ptr=
      decorations.add_Cones(passes_group.get_pass_ptr(cloudpass_ID));

   double cone_height=5 /feet_per_meter;	// meters
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
   alpha=1;
   for (int i=0; i<XYZ.size(); i++)
   {
      if (i > 0)
      {
         cone_color=colorfunc::blue;	// 5' particle counters
         alpha=0.9;
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

// Instantiate group to hold movie:

   cout << "Instantiating movies_group" << endl;
   
   Pass* texture_pass_ptr=passes_group.get_pass_ptr(texturepass_ID);
   PassInfo* texture_PassInfo_ptr=texture_pass_ptr->get_PassInfo_ptr();

   MoviesGroup movies_group(ndims,texture_pass_ptr,AnimationController_ptr);
   string movie_filename=texture_pass_ptr->get_first_filename();

   texture_rectangle* texture_rectangle_ptr=
      movies_group.generate_new_texture_rectangle(movie_filename);
   Movie* movie_ptr=movies_group.generate_new_Movie(texture_rectangle_ptr);
   movies_group.set_OSGgroup_nodemask(1);

   vector<threevector> movie_corner_vertices=texture_PassInfo_ptr-> 
      get_video_corner_vertices();
   movie_ptr->reset_geom_vertices(
      movie_corner_vertices[1],movie_corner_vertices[0],
      movie_corner_vertices[3],movie_corner_vertices[2]);

// Attach all data and decorations to scenegraph:

   root->addChild(clouds_group.get_OSGgroup_ptr());
   root->addChild(decorations.get_OSGgroup_ptr());
   root->addChild(movies_group.get_OSGgroup_ptr());

// FAKE FAKE:  Thurs Jan 3, 2013 at 11:19 am

// Turn off symbols and point cloud so that we can see wooden posts better:

   SphereSegmentsGroup_ptr->set_OSGgroup_nodemask(0);
   BoxesGroup_ptr->set_OSGgroup_nodemask(0);
   ConesGroup_ptr->set_OSGgroup_nodemask(0);

//   CylindersGroup_ptr->set_OSGgroup_nodemask(0);
//   clouds_group.set_OSGgroup_nodemask(0);
//   movies_group.set_OSGgroup_nodemask(0);

// Attach scene graph to the viewer:

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
