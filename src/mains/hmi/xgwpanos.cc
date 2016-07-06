// ========================================================================
// Program XGWPANOS is a variant of XSENSPANOS which can input from
// the orange XSENS box, a GPS hockey puck sensor and/or Wiimote.  We
// wrote this program specifically to be used for outdoor "magic
// window" sensing around Killian Courtyard and LL's volleyball court.
// ========================================================================
// Last updated on 8/19/11; 8/24/11; 8/28/11
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgText/Text>

#include "osg/osgGraphicals/AnimationController.h"
#include "geometry/bounding_box.h"
#include "video/camera.h"
#include "osg/CompassHUD.h"
#include "math/constant_vectors.h"
#include "geometry/contour.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "astro_geo/GPS_datastream.h"
#include "osg/osgGrid/GridKeyHandler.h"
#include "osg/osgGrid/LatLongGridsGroup.h"
#include "osg/ModeController.h"
#include "osg/ModeKeyHandler.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "osg/osgPanoramas/PanoramasGroup.h"
#include "osg/osgPanoramas/PanoramasKeyHandler.h"
#include "osg/osgPanoramas/PanoramaPickHandler.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "geometry/polyline.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "time/timefuncs.h"
#include "osg/osgWindow/ViewerManager.h"
#include "ins/wiimote.h"
#include "ins/xsens_ins.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::string;
using std::vector;

// ==========================================================================
int main( int argc, char** argv )
{

   timefunc::initialize_timeofday_clock();

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   Pass* pass_ptr=passes_group.get_pass_ptr(0);
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
   cout << "cloudpass_ID = " << cloudpass_ID << endl;
   int texturepass_ID=passes_group.get_curr_texturepass_ID();
   cout << "texturepass_ID = " << texturepass_ID << endl;

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group);
   int n_photos=photogroup_ptr->get_n_photos();
   bounding_box photo_bbox=photogroup_ptr->get_bbox();

// Construct the viewers and instantiate 2 ViewerManagers:

   WindowManager* window_mgr_ptr=new ViewerManager();
   WindowManager* window_mgr_global_ptr=new ViewerManager();
   
   string window_title="Ground view";
   string window_global_title="Overhead view";
   window_mgr_ptr->initialize_dual_windows(
      window_title,window_global_title,window_mgr_global_ptr);

// Create OSG root node:

   osg::Group* root = new osg::Group;
   osg::Group* root_global = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

   bool display_movie_state=false;
   bool display_movie_number=false;
   bool display_movie_world_time=false;
   bool hide_Mode_HUD_flag=false;
   Operations operations(
      ndims,window_mgr_ptr,passes_group,
      display_movie_state,display_movie_number,display_movie_world_time,
      hide_Mode_HUD_flag);

//   Operations operations(ndims,window_mgr_ptr,passes_group);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   root->addChild(operations.get_OSGgroup_ptr());

   ModeController* ModeController_global_ptr=new ModeController();
   window_mgr_global_ptr->get_EventHandlers_ptr()->push_back( 
      new ModeKeyHandler(ModeController_global_ptr) );

   if (!hide_Mode_HUD_flag)
   {
      root_global->addChild(osgfunc::create_Mode_HUD(
         ndims,ModeController_global_ptr));
   }
   
// Add one panorama custom manipulator and another global 3D custom
// manipulator to the event handler list:

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);
   CM_3D_ptr->set_allow_only_az_rotation_flag(true);
   CM_3D_ptr->set_disallow_zoom_flag(true);

   osgGA::Terrain_Manipulator* CM_3D_global_ptr=
      new osgGA::Terrain_Manipulator(
         ModeController_global_ptr,window_mgr_global_ptr);
   window_mgr_global_ptr->set_CameraManipulator(CM_3D_global_ptr);

// Instantiate an XSENS_INS object to detect physical motions:

   xsens_ins XSENS;
   while (!XSENS.initialize_xsens())
   {
      cout << "Trying to initialize XSENS" << endl;
   }
   XSENS.initialize_xsens_2();
   XSENS.set_CM_3D_ptr(CM_3D_ptr);

// Search for GPS device attached to /dev/ttyUSBN where N=0,1,2,...
// On 7/16/2011, we pushed the search for /dev/ttyUSB0 to the end of
// the possible /dev list.  This change allows the orange XSENS box to
// be plugged in first to a laptop USB port and be assigned
// /dev/ttyUSB0.  A subsequent GPS hockeypuck device will then be
// assigned /dev/ttyUSB1 or higher.  Output from both sensors can then
// be recorded to text files.

   vector<string> candidate_serial_ports;
   candidate_serial_ports.push_back("/dev/ttyUSB1");
   candidate_serial_ports.push_back("/dev/ttyUSB2");
   candidate_serial_ports.push_back("/dev/ttyUSB3");
   candidate_serial_ports.push_back("/dev/ttyUSB4");
   candidate_serial_ports.push_back("/dev/ttyUSB5");
   candidate_serial_ports.push_back("/dev/ttyUSB0");

   bool serial_port_device_found_flag=false;   
   string serialPort;
   for (int i=0; i<candidate_serial_ports.size(); i++)
   {
      string curr_serialPort=candidate_serial_ports[i];
      if (filefunc::chardevexist(curr_serialPort))
      {
         serialPort=curr_serialPort;
         serial_port_device_found_flag=true;
         break;
      }
   }
    
   cout << endl;
   if (!serial_port_device_found_flag)
   {
      cout << "No GPS device detected attached to /dev/ttyUSBN!" << endl;
      exit(-1);
   }
   else
   {
      cout << "GPS device detected attached to "+serialPort << endl;
   }

   GPS_datastream GPS_datastream(serialPort);
   double prev_time=0; 

// Wait until satellite connection for GPS hockeypuck is established
// before continuing:

   string banner="Trying to establish satellite contact:";
   outputfunc::write_big_banner(banner);

   cout << "GPS satellite link should hopefully be soon established.  If no GPS lock"  << endl;
   cout << "is achieved in roughly 10 secs, it's reasonable to kill this program" <<endl;
   cout << "by entering control-c and restart it again.  You may need to restart" << endl;
   cout << "the program multiple times before a solid link with GPS satellites" << endl;
   cout << "is established and useful GPS data starts recording to disk..." << endl << endl << endl;

   bool date_read_flag=false;
   bool time_read_flag=false;
   bool curr_date_read_flag,curr_time_read_flag;
   while (!date_read_flag || !time_read_flag)
   { 
      if (GPS_datastream.read_curr_data(
         curr_date_read_flag,curr_time_read_flag))
      {
         if (curr_date_read_flag) date_read_flag=true;
         if (curr_time_read_flag) time_read_flag=true;
      }
   }

   banner="Satellite contact established!";
   outputfunc::write_big_banner(banner);

// Instantiate compass:

   CompassHUD* CompassHUD_ptr=new CompassHUD(colorfunc::green);
   CompassHUD_ptr->set_nadir_oriented_compass_flag(false);
   root->addChild(CompassHUD_ptr->getProjection());
   CM_3D_ptr->set_CompassHUD_ptr(CompassHUD_ptr);

// Instantiate group to hold all decorations:
   
   Decorations decorations(window_mgr_ptr,ModeController_ptr,CM_3D_ptr);
   Decorations decorations_global(
      window_mgr_global_ptr,ModeController_global_ptr,CM_3D_global_ptr);

// Instantiate groups to hold multiple surface textures,
// latitude-longitude grids and associated earth regions:

   LatLongGridsGroup latlonggrids_group(
      ndims,passes_group.get_pass_ptr(texturepass_ID),CM_3D_ptr);
   root->addChild(latlonggrids_group.get_OSGgroup_ptr());
   root_global->addChild(latlonggrids_group.get_OSGgroup_ptr());

   EarthRegionsGroup earth_regions_group(
      passes_group.get_pass_ptr(texturepass_ID),&latlonggrids_group);
   bool place_onto_bluemarble_flag=false;
   bool generate_pointcloud_LatLongGrid_flag=false;
   bool display_SurfaceTexture_LatLongGrid_flag=true;
   earth_regions_group.generate_regions(
      passes_group,place_onto_bluemarble_flag,
      generate_pointcloud_LatLongGrid_flag,
      display_SurfaceTexture_LatLongGrid_flag);
   
   root->addChild(earth_regions_group.get_OSGgroup_ptr());
   root_global->addChild(earth_regions_group.get_OSGgroup_ptr());

// Display latlonggrid underneath a surface texture only for algorithm
// development purposes:

   LatLongGrid* latlonggrid_ptr=latlonggrids_group.get_Grid_ptr(0);
   threevector* grid_origin_ptr=latlonggrid_ptr->get_world_origin_ptr();

   CM_3D_ptr->set_Grid_ptr(latlonggrid_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new GridKeyHandler(ModeController_ptr,latlonggrid_ptr));

   CM_3D_ptr->set_Grid_ptr(latlonggrid_ptr);
   CM_3D_global_ptr->set_Grid_ptr(latlonggrid_ptr);
   decorations.set_grid_origin_ptr(grid_origin_ptr);
   decorations_global.set_grid_origin_ptr(grid_origin_ptr);

// Instantiate group to hold pointcloud information:

   PointCloudsGroup clouds_group(
      passes_group.get_pass_ptr(cloudpass_ID),grid_origin_ptr);
   if (cloudpass_ID >= 0)
   {
      PointCloud* PointCloud_ptr=clouds_group.generate_new_Cloud(
         passes_group.get_pass_ptr(cloudpass_ID));
      clouds_group.set_pt_size(3);
      window_mgr_global_ptr->get_EventHandlers_ptr()->push_back(
         new PointCloudKeyHandler(&clouds_group,ModeController_ptr,CM_3D_ptr));
      clouds_group.set_OSGgroup_nodemask(1);
      root_global->addChild(clouds_group.get_OSGgroup_ptr());

// Instantiate a MyDatabasePager to handle paging of files from disk:

      viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager(
         clouds_group.get_SetupGeomVisitor_ptr(),
         clouds_group.get_ColorGeodeVisitor_ptr());
      clouds_group.get_HiresDataVisitor_ptr()->setDatabasePager(
         MyDatabasePager_ptr);
   }
   
// Instantiate a PointFinder;

   PointFinder pointfinder;
   CM_3D_ptr->set_PointFinder(&pointfinder);
   CM_3D_global_ptr->set_PointFinder(&pointfinder);

// Instantiate OBSFRUSTAGROUP decoration group:

   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr=decorations.add_OBSFRUSTA(
      pass_ptr,AnimationController_ptr);
   OBSFRUSTAGROUP_ptr->set_FOV_excess_fill_factor(0.98);

// Instantiate an individual OBSFRUSTUM for every input video.  Each
// contains a separate movie object.

   OBSFRUSTAGROUP_ptr->get_MoviesGroup_ptr()->set_hide_backfaces_flag(true);

//   double global_daz=0*PI/180;
//   cout << "Enter global_daz in degs:" << endl;
//   cin >> global_daz;
//   global_daz *= PI/180;

   const double avg_canonical_edge_orientation = 0 * PI/180; 
   double global_daz=0;
   double global_del=0;
   double global_droll=0;
   threevector rotation_origin=Zero_vector;
//   threevector rotation_origin=*grid_origin_ptr;
   double local_spin_daz=0*PI/180;	
//   double local_spin_daz=6.6*36*PI/180;	// Aud data #2 fudge factor

//   string banner="Enter azimuthal spin angle in degrees:";
//   outputfunc::write_big_banner(banner);
//   cin >> local_spin_daz;
//   local_spin_daz *= PI/180;

   colorfunc::Color OBSFRUSTUM_color=colorfunc::blue;
//   bool thumbnails_flag=false;
   bool thumbnails_flag=true;
   threevector global_camera_translation=Zero_vector;
   OBSFRUSTAGROUP_ptr->generate_still_imagery_frusta_for_photogroup(
      photogroup_ptr,global_camera_translation,
      global_daz,global_del,global_droll,rotation_origin,
      local_spin_daz,OBSFRUSTUM_color,thumbnails_flag);

// Set mask_nonselected_OSGsubPATs_flag=false in order to
// simultaneously display OSGsubPATs 0 and 1 respectively containing
// static panorama and dynamic video images:

//   decorations.get_OBSFRUSTUMPickHandler_ptr()->
//      set_mask_nonselected_OSGsubPATs_flag(false);

// Instantiate Army Symbols decoration group to hold forward
// translation arrows:

   ArmySymbolsGroup* ArmySymbolsGroup_ptr=
      decorations.add_ArmySymbols(pass_ptr);

// Instantiate SignPostsGroup:

   SignPostsGroup* GPS_SignPostsGroup_ptr=decorations_global.add_SignPosts(
      ndims,pass_ptr);
//   GPS_SignPostsGroup_ptr->set_common_geometrical_size(0.0025);
//   decorations.get_OBSFRUSTAKeyHandler_ptr()->
//      set_SignPostsGroup_ptr(GPS_SignPostsGroup_ptr);
   XSENS.set_GPS_SignPostsGroup_ptr(GPS_SignPostsGroup_ptr);

   double size=0.1;
   double height_multiplier=1;
   threevector GPS_UVW(327762 , 4691737 , 0);
   SignPost* GPS_SignPost_ptr=GPS_SignPostsGroup_ptr->generate_new_SignPost(
      size,height_multiplier,GPS_UVW);
   GPS_SignPost_ptr->set_label("curr GPS");
   GPS_SignPost_ptr->set_permanent_color(
      colorfunc::get_OSG_color(colorfunc::red));

// Instantiate PanoramasGroup to hold multiple 360 degree panoramas:

   PanoramasGroup* PanoramasGroup_ptr=new PanoramasGroup(
      pass_ptr,OBSFRUSTAGROUP_ptr,decorations.get_OBSFRUSTUMPickHandler_ptr(),
      ArmySymbolsGroup_ptr,CM_3D_ptr,CM_3D_global_ptr,grid_origin_ptr);
   PanoramasGroup_ptr->set_GPS_SignPostsGroup_ptr(GPS_SignPostsGroup_ptr);

   colorfunc::Color permanent_pano_color=colorfunc::blue;
   colorfunc::Color selected_pano_color=colorfunc::red;
   PanoramasGroup_ptr->set_colors(permanent_pano_color,selected_pano_color);

// Initialize Wiimote if user does NOT use GPS to automatically select
// panoramas based upon his instantaneous position:

   wiimote WM;
   

   banner="Enter 'g' to enable auto pano selection via GPS:";
   outputfunc::write_big_banner(banner);
   string GPS_input_str;
   cin >> GPS_input_str;
   if (GPS_input_str=="g") 
   {
      PanoramasGroup_ptr->set_GPS_response_flag(true);
      cout << "GPS response flag set to true:" << endl;
      outputfunc::enter_continue_char();
   }
   else
   {
      WM.initialize_wiimote();
      WM.set_CM_3D_ptr(CM_3D_ptr);
   }

   int n_OBSFRUSTA_per_panorama=10;
   double label_delta_z=8;	// meters
   double label_text_size=5;
   PanoramasGroup_ptr->generate_panoramas(
      n_OBSFRUSTA_per_panorama,label_delta_z,label_text_size);

   PanoramasKeyHandler* PanoramasKeyHandler_ptr=
      new PanoramasKeyHandler(PanoramasGroup_ptr,ModeController_ptr);
//   PanoramasKeyHandler_ptr->set_SignPostsGroup_ptr(SignPostsGroup_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      PanoramasKeyHandler_ptr);

   PanoramaPickHandler* PanoramaPickHandler_ptr=new PanoramaPickHandler(
         pass_ptr,CM_3D_global_ptr,PanoramasGroup_ptr,
         ModeController_ptr,window_mgr_global_ptr,grid_origin_ptr);
   window_mgr_global_ptr->get_EventHandlers_ptr()->push_back(
         PanoramaPickHandler_ptr);

// Generate modified Delaunay triangle network which allows viewer to
// fly among panorama centers:

   PanoramasGroup_ptr->Delaunay_triangulate_pano_centers();

// Attach scene graphs to viewers:

   root->addChild(PanoramasGroup_ptr->get_OSGgroup_ptr());
   root->addChild(decorations.get_OSGgroup_ptr());
//   root->addChild(imageplane_SignPostsGroup_ptr->get_OSGgroup_ptr());

   root_global->addChild(PanoramasGroup_ptr->get_OSGgroup_ptr());
   root_global->addChild(
      PanoramasGroup_ptr->get_ArrowsGroup_ptr()->get_OSGgroup_ptr());
   root_global->addChild(decorations.get_OSGgroup_ptr());
   root_global->addChild(decorations_global.get_OSGgroup_ptr());

   window_mgr_ptr->setSceneData(root);
   window_mgr_global_ptr->setSceneData(root_global);
//   window_mgr_global_ptr->setSceneData(TotalTransform_ptr);

// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_ptr->realize();
   window_mgr_global_ptr->realize();

// Set initial camera lateral posn to grid's midpoint and scale its
// altitude according to grid's maximal linear dimension:

   CM_3D_ptr->set_worldspace_center(latlonggrid_ptr->get_world_middle());
   CM_3D_ptr->set_eye_to_center_distance(
      2*basic_math::max(latlonggrid_ptr->get_xsize(),
      latlonggrid_ptr->get_ysize()));
   CM_3D_ptr->update_M_and_Minv();

   CM_3D_global_ptr->set_worldspace_center(
      latlonggrid_ptr->get_world_middle());
   CM_3D_global_ptr->set_eye_to_center_distance(
      2*basic_math::max(latlonggrid_ptr->get_xsize(),
      latlonggrid_ptr->get_ysize()));
   CM_3D_global_ptr->update_M_and_Minv();

   window_mgr_ptr->process();

// Start virtual camera at OBSFRUSTUM #0:

   int initial_pano_ID=0;
   OBSFRUSTAGROUP_ptr->set_selected_Graphical_ID(
      initial_pano_ID*n_OBSFRUSTA_per_panorama);
//   OBSFRUSTAGROUP_ptr->set_selected_Graphical_ID(8*n_OBSFRUSTA_per_panorama);

// Hardwire horizontal and vertical fields-of-view for "1st person"
// window which are appropriate for Auditorium data set #2 collected
// by Lady Bug in Feb 2011:

   double hfov=75.195;
   double vfov=57.337;
//   window_mgr_ptr->set_viewer_horiz_vert_fovs(hfov,vfov);

   PanoramasGroup_ptr->recolor_start_stop_panoramas(
      initial_pano_ID,initial_pano_ID);
   PanoramasGroup_ptr->load_hires_panels(initial_pano_ID);

   int n_anim_steps=0;
   OBSFRUSTAGROUP_ptr->flyto_camera_location(
      OBSFRUSTAGROUP_ptr->get_selected_Graphical_ID(),n_anim_steps);

// On 8/19/11, we realized that OSG window updating is sufficiently
// time consuming that it noticeably increases reaction time to XSENS
// rotational input.  So we try to increase the number of calls to
// XSENS measurement readings relative to OSG window updating in order
// to make XSENSPANOS more responsive to changes in angular
// orientation.

   bool flag1=true;
   bool flag2=false;
   int counter=0;
   while( !window_mgr_ptr->done() && !window_mgr_global_ptr->done() )
   {
      if (flag1) window_mgr_ptr->process();
      flag1=!flag1;

      XSENS.update_ins_metadata();

      XSENS.update_ins_metadata_2();

      XSENS.update_median_az_el_roll();
//      XSENS.update_avg_az_el_roll();
      XSENS.alpha_filter_az_el_roll();

//      XSENS.update_median_lat_lon_alt();
//      XSENS.alpha_filter_lat_lon_alt();

//      cout << "lon = " << XSENS.get_alpha_filtered_lon()
//           << " lat = " << XSENS.get_alpha_filtered_lat()
//           << " alt = " << XSENS.get_alpha_filtered_alt() << endl;
//      cout << "filtered az = " << XSENS.get_alpha_filtered_az()*180/PI
//           << " filtered el = " << XSENS.get_alpha_filtered_el()*180/PI
//           << " filtered roll = " << XSENS.get_alpha_filtered_roll()*180/PI 
//           << endl;

      if (flag2) window_mgr_global_ptr->process();
      flag2=!flag2;

      XSENS.update_ins_metadata();
      XSENS.update_ins_metadata_2();

      XSENS.update_median_az_el_roll();
//      XSENS.update_avg_az_el_roll();
      XSENS.alpha_filter_az_el_roll();

//      cout << "filtered az = " << XSENS.get_alpha_filtered_az()*180/PI
//           << " filtered el = " << XSENS.get_alpha_filtered_el()*180/PI
//           << " filtered roll = " << XSENS.get_alpha_filtered_roll()*180/PI 
//           << endl;

      XSENS.update_median_lat_lon_alt();
      XSENS.alpha_filter_lat_lon_alt();

/*
// On 8/28/11, we empirically found that executing GPS hockeypuck
// update calls is expensive and intolerably slows down XSENS
// azimuthal orientation responsiveness.  So we need to significantly
// limit the frequency of GPS hockey puck calls:

      int counter_multiple=20;
      if (!PanoramasGroup_ptr->get_GPS_response_flag())
      {
         counter_multiple=10;
      }
    
      if (counter%counter_multiple == 0)
      {
         GPS_datastream.read_curr_data(); 

//         const double alpha=0.01;
//         const double alpha=0.1;
//         const double alpha=0.3;
         const double alpha=1.0;
         GPS_datastream.alpha_filter_lat_lon_alt(alpha);
//         cout << "alpha filtered GPS UTM posn = "
//              << GPS_datastream.get_alpha_filtered_UTM_posn().get(0) << " , "
//              << GPS_datastream.get_alpha_filtered_UTM_posn().get(1) << " , "
//              << GPS_datastream.get_alpha_filtered_UTM_posn().get(2) << endl;

//         threevector curr_GPS_posn=GPS_datastream.
//            get_alpha_filtered_UTM_posn();
         threevector curr_GPS_posn=GPS_datastream.get_raw_UTM_posn();

         cout << "raw GPS posn = " << curr_GPS_posn << endl;
         curr_GPS_posn.put(2,0);

         GPS_SignPost_ptr->set_UVW_coords(
            GPS_SignPostsGroup_ptr->get_curr_t(),
            GPS_SignPostsGroup_ptr->get_passnumber(),curr_GPS_posn);
         
// If GPS is NOT being used to automatically select panoramas based
// upon user's instantaneous position, query Wii to see if user has
// pushed any button to indicate movement to next, forward panorama
// wheel:

         if (!PanoramasGroup_ptr->get_GPS_response_flag())
         {
            WM.update_state();
            int button_value=WM.get_curr_button_value();
//            if (button_value > 0) 
//            cout << "button value = " << button_value << endl;
         }

      }
*/

      counter++;      
   }

   delete window_mgr_ptr;
   delete window_mgr_global_ptr;
}

