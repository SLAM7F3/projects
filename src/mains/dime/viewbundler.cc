// ========================================================================
// This variant of VIEWBUNDLER displays a "wagon wheel" of OBSFRUSTA
// corresponding to WISP panels within a 3D point cloud.  It also has
// an intentionally large AlirtGrid which yields a 3D "horizon"
// separating sky from sea when projected into the 2D WISP panels.
// ========================================================================
// Last updated on 3/26/13; 4/1/13; 4/3/13
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

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   string image_list_filename=passes_group.get_image_list_filename();
//   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
//   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string image_sizes_filename=passes_group.get_image_sizes_filename();
//   cout << "image_sizes_filename = " << image_sizes_filename << endl;
   string xyz_points_filename=passes_group.get_xyz_points_filename();
//   cout << "xyz_points_filename = " << xyz_points_filename << endl;

   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
//   int texturepass_ID=passes_group.get_curr_texturepass_ID();

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group);
   photogroup_ptr->set_bundler_IO_subdir(bundler_IO_subdir);
   int n_photos=photogroup_ptr->get_n_photos();
   cout << "n_photos = " << n_photos << endl;

   threevector camera_world_posn;
   for (int n=0; n<n_photos; n++)
   {
      photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(n);
      camera* camera_ptr=photo_ptr->get_camera_ptr();
      camera_world_posn=camera_ptr->get_world_posn();

// Translate camera up by 7 meters:

//      camera_world_posn += threevector(0,0,7);
//      camera_ptr->set_world_posn(camera_world_posn);

//      cout << "n = " << n
//           << " camera = " << *camera_ptr 
//           << " camera posn = " << camera_ptr->get_world_posn()
//           << endl;
   }

// Read in reconstructed XYZ points plus their IDs along with visible
// camera IDs into videofunc map *xyz_map_ptr:

//   videofunc::CAMERAID_XYZ_MAP* cameraid_xyz_map_ptr=NULL;
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
   CM_3D_ptr->set_max_camera_height_above_grid_factor(100);
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold all decorations:

   Decorations decorations(
      window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

// Instantiate Grid:

   AlirtGrid* grid_ptr=decorations.add_AlirtGrid(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
//   grid_ptr->set_curr_color(colorfunc::brightpurple);
   grid_ptr->set_curr_color(osg::Vec4(0.75,0.75,0.75,1));
   threevector* grid_origin_ptr=grid_ptr->get_world_origin_ptr();
   CM_3D_ptr->set_Grid_ptr(grid_ptr);
   decorations.set_grid_origin_ptr(grid_origin_ptr);

// Instantiate group to hold pointcloud information:

   PointCloudsGroup clouds_group(
      passes_group.get_pass_ptr(cloudpass_ID),grid_origin_ptr);
   clouds_group.generate_Clouds(passes_group);
   clouds_group.set_Terrain_Manipulator_ptr(CM_3D_ptr);
   clouds_group.set_auto_resize_points_flag(false);
   decorations.set_PointCloudsGroup_ptr(&clouds_group);

   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_ptr,CM_3D_ptr));

// Initialize ALIRT grid based upon cloud's bounding box:

//   Grid::Distance_Scale distance_scale=Grid::meter;
   Grid::Distance_Scale distance_scale=Grid::kilometer;
   double delta_s=-1;
   double magnification_factor=12;   

   osg::BoundingBox bbox=clouds_group.get_xyz_bbox();
   decorations.get_AlirtGridsGroup_ptr()->initialize_grid(
      grid_ptr,bbox,distance_scale,delta_s,magnification_factor);
   grid_ptr->set_delta_xy(2500,2500);
   cout << "*grid_origin_ptr = " << *grid_origin_ptr << endl;
//   grid_ptr->set_axis_char_label_size(
//      3*grid_ptr->get_axis_char_label_size());
//   grid_ptr->set_tick_char_label_size(
//      3*grid_ptr->get_tick_char_label_size());
   grid_ptr->set_axes_labels("Relative Easting (km)","Relative Northing(km)");

   grid_ptr->update_grid();

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

// Instantiate OBSFRUSTA decoration group:

   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr=decorations.add_OBSFRUSTA(
      passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
   OBSFRUSTAGROUP_ptr->pushback_Messenger_ptr(
      &OBSFRUSTA_photo_network_messenger);
//   OBSFRUSTAGROUP_ptr->set_enable_OBSFRUSTA_blinking_flag(true);
   OBSFRUSTAGROUP_ptr->set_enable_OBSFRUSTA_blinking_flag(false);
//   OBSFRUSTAGROUP_ptr->set_erase_other_OBSFRUSTA_flag(true);
   OBSFRUSTAGROUP_ptr->set_erase_other_OBSFRUSTA_flag(false);

// Instantiate an individual OBSFRUSTUM for every photograph:

   double frustum_sidelength=20;
//   double frustum_sidelength=60;
//   double frustum_sidelength=250;
//   double frustum_sidelength=25000;
   double movie_downrange_distance=-1;
   bool multicolor_frusta_flag=false;
   bool thumbnails_flag=true;
   OBSFRUSTAGROUP_ptr->generate_still_imagery_frusta_for_photogroup(
      photogroup_ptr,frustum_sidelength,movie_downrange_distance,
      multicolor_frusta_flag,thumbnails_flag);

   decorations.get_OBSFRUSTUMPickHandler_ptr()->
      set_mask_nonselected_OSGsubPATs_flag(true);
   decorations.get_OBSFRUSTAKeyHandler_ptr()->set_SignPostsGroup_ptr(
      SignPostsGroup_ptr);

// Instantiate Polyhedra, Polygons & PolyLines decoration groups:

   PolyhedraGroup* PolyhedraGroup_ptr=decorations.add_Polyhedra(
      passes_group.get_pass_ptr(cloudpass_ID));
   PolyhedraGroup_ptr->set_OFF_subdir(
      "/home/cho/programs/c++/svn/projects/src/mains/modeling/OFF/");

   osgGeometry::PolygonsGroup* PolygonsGroup_ptr=decorations.add_Polygons(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
   OBSFRUSTAGROUP_ptr->set_PolygonsGroup_ptr(PolygonsGroup_ptr);

   PolyLinesGroup* PolyLinesGroup_ptr=decorations.add_PolyLines(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   PolygonsGroup_ptr->set_PolyLinesGroup_ptr(PolyLinesGroup_ptr);

   PolyLinesGroup_ptr->set_width(3);
   PolyLinesGroup_ptr->set_multicolor_flags(true);
   PolyLinesGroup_ptr->set_ID_labels_flag(true);
   PolyLinesGroup_ptr->set_CM_3D_ptr(CM_3D_ptr);
   PolyLinesGroup_ptr->set_altitude_dependent_labels_flag(false);

   PolyLinePickHandler* PolyLinePickHandler_ptr=
      decorations.get_PolyLinePickHandler_ptr();
   PolyLinePickHandler_ptr->set_min_doubleclick_time_spread(0.05);
   PolyLinePickHandler_ptr->set_max_doubleclick_time_spread(0.25);
   PolyLinePickHandler_ptr->set_z_offset(5);

//   PolyLinePickHandler_ptr->set_form_polyhedron_skeleton_flag(true);
//   PolyLinePickHandler_ptr->set_form_polygon_flag(true);

// Import ERSA tracks and display as 3D polylines:   

   string ERSA_subdir="./ERSA/";
   string tracks_filename=ERSA_subdir+"tracks.dat";
   filefunc::ReadInfile(tracks_filename);

   vector<int> specified_OBSFRUSTA_IDs;
   specified_OBSFRUSTA_IDs.push_back(0);
   specified_OBSFRUSTA_IDs.push_back(1);
   specified_OBSFRUSTA_IDs.push_back(2);
   specified_OBSFRUSTA_IDs.push_back(3);
   specified_OBSFRUSTA_IDs.push_back(4);
   specified_OBSFRUSTA_IDs.push_back(5);
   specified_OBSFRUSTA_IDs.push_back(6);
   specified_OBSFRUSTA_IDs.push_back(7);
   specified_OBSFRUSTA_IDs.push_back(8);
   specified_OBSFRUSTA_IDs.push_back(9);

   int track_ID=-1;
   vector<threevector> V;
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      if (column_values[0] != track_ID && track_ID >= 0 && V.size() > 5)
      {
         int color_index=track_ID%12;

         osg::Vec4 track_color=colorfunc::get_OSG_color(
            colorfunc::get_color(color_index));

/*
         vector<osg::Vec4> track_colors;
         double hue_start=300;
         double hue_stop=0;
         for (int j=0; j<V.size(); j++)
         {
            double curr_frac=double(j)/(V.size()-1);
            double hue=hue_start+(hue_stop-hue_start)*curr_frac;
            double s=1;
            double v=1;
            double r,g,b;
            colorfunc::hsv_to_RGB(hue,s,v,r,g,b);
//            cout << "curr_frac = " << curr_frac
//                 << " r = " << r << " g = " << g << " b = " << b << endl;
            track_colors.push_back(osg::Vec4(r,g,b,1));
         }
*/

         bool force_display_flag=false;
         bool single_polyline_per_geode_flag=true;
         int n_text_messages=1;
         int PolyLine_ID=track_ID;

         PolyLine* PolyLine_ptr=PolyLinesGroup_ptr->generate_new_PolyLine(
            V.front(),V,track_color,
            force_display_flag,single_polyline_per_geode_flag,
            n_text_messages,PolyLine_ID);

//         PolyLine* PolyLine_ptr=PolyLinesGroup_ptr->generate_new_PolyLine(
//            V.front(),V,track_colors,
//            force_display_flag,single_polyline_per_geode_flag,
//            n_text_messages,PolyLine_ID);


         V.clear();
      }

      track_ID=column_values[0];
//      double curr_time=column_values[1];
      double easting=column_values[2];
      double northing=column_values[3];
      double altitude=column_values[4];
      threevector curr_aerial_posn(easting,northing,altitude);

      V.push_back(curr_aerial_posn);

   } // loop over index i labeling text_line
   PolyLinesGroup_ptr->reset_labels();
   PolyLinesGroup_ptr->change_size(1,1,1,5);

// Instantiate Points decorations group:

   osgGeometry::PointsGroup* PointsGroup_ptr=decorations.add_Points(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
   OBSFRUSTAGROUP_ptr->set_PointsGroup_ptr(PointsGroup_ptr);

// Instantiate 2D features decorations group:

//   FeaturesGroup* UV_FeaturesGroup_ptr=
//      decorations.add_Features(2,passes_group.get_pass_ptr(cloudpass_ID));
//   UV_FeaturesGroup_ptr->set_OBSFRUSTAGROUP_ptr(OBSFRUSTAGROUP_ptr);

// Instantiate 3D features decorations group:

   FeaturesGroup* FeaturesGroup_ptr=
      decorations.add_Features(ndims,passes_group.get_pass_ptr(cloudpass_ID));

// Instantiate cylinders decoration group to mark light houses in
// Boston Harbor:

   CylindersGroup* CylindersGroup_ptr=
      decorations.add_Cylinders(passes_group.get_pass_ptr(cloudpass_ID),
      AnimationController_ptr);
   osg::Quat q(0,0,0,1);

   double cyl_height=31;	// meters
   CylindersGroup_ptr->set_rh(5,0.5*cyl_height);

// Graves light house:

   double graves_eff_z=-3.9;
   threevector Graves_cyl_posn(346082,4692006,graves_eff_z);
   threevector cyl_center=Graves_cyl_posn+0.5*cyl_height*z_hat;
   colorfunc::Color Graves_color=colorfunc::green;
   Cylinder* Graves_Cylinder_ptr=CylindersGroup_ptr->generate_new_Cylinder(
      cyl_center,q,Graves_color);
//   Graves_Cylinder_ptr->set_permanent_color(
//      colorfunc::get_OSG_color(colorfunc::green,1));

// Boston light house:

   double boston_eff_z=-2.5;
   threevector Boston_cyl_posn(344271,4687916,boston_eff_z);
   cyl_center=Boston_cyl_posn+0.5*cyl_height*z_hat;
   colorfunc::Color Boston_color=colorfunc::blue;
   Cylinder* Boston_Cylinder_ptr=CylindersGroup_ptr->generate_new_Cylinder(
      cyl_center,q,Boston_color);
//   Boston_Cylinder_ptr->set_permanent_color(
//      colorfunc::get_OSG_color(colorfunc::blue,1));

// Attach all data and decorations to scenegraph:

   root->addChild(clouds_group.get_OSGgroup_ptr());
   root->addChild(decorations.get_OSGgroup_ptr());

// Attach scene graph to the viewer:

   window_mgr_ptr->setSceneData(root);

// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_ptr->realize();

   int n_PolyLines=PolyLinesGroup_ptr->get_n_Graphicals();
   cout << "n_PolyLines = " << n_PolyLines << endl;
   PolyLinesGroup_ptr->set_OSGgroup_nodemask(0);	// hide ERSA tracks
//   clouds_group.set_OSGgroup_nodemask(0);		// hide point cloud


   while( !window_mgr_ptr->done() )
   {
      window_mgr_ptr->process();
   }
   delete window_mgr_ptr;

}
