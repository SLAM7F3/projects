// ========================================================================
// Program GSTREETS is a variant of "Google Streets" meant for indoor
// panorama viewing.
// ========================================================================
// Last updated on 1/31/10; 2/3/10; 2/4/10; 12/4/10
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgText/Text>

#include "osg/osgGrid/AlirtGridsGroup.h"
#include "osg/osgGraphicals/AnimationController.h"
#include "video/camera.h"
#include "osg/CompassHUD.h"
#include "math/constant_vectors.h"
#include "geometry/contour.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "osg/osgGrid/LatLongGridsGroup.h"
#include "osg/ModeController.h"
#include "osg/ModeKeyHandler.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osg2D/MovieKeyHandler.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "osg/osgPanoramas/PanoramasGroup.h"
#include "osg/osgPanoramas/PanoramasKeyHandler.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "geometry/polyline.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgWindow/ViewerManager.h"

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

// Construct the viewers and instantiate 2 ViewerManagers:

   WindowManager* window_mgr_ptr=new ViewerManager();
   WindowManager* window_mgr_global_ptr=new ViewerManager();
   
   string window_title="Robot view";
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
   bool generate_AVI_movie_flag=true;
   Operations operations(
      ndims,window_mgr_ptr,passes_group,
      display_movie_state,display_movie_number,display_movie_world_time,
      generate_AVI_movie_flag);

//   Operations operations(ndims,window_mgr_ptr,passes_group);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   root->addChild(operations.get_OSGgroup_ptr());

   ModeController* ModeController_global_ptr=new ModeController();
   window_mgr_global_ptr->get_EventHandlers_ptr()->push_back( 
      new ModeKeyHandler(ModeController_global_ptr) );
   root_global->addChild(osgfunc::create_Mode_HUD(
      ndims,ModeController_global_ptr));

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

// Instantiate compass:

   CompassHUD* CompassHUD_ptr=new CompassHUD(colorfunc::green);
   CompassHUD_ptr->set_nadir_oriented_compass_flag(false);
   root->addChild(CompassHUD_ptr->getProjection());
   CM_3D_ptr->set_CompassHUD_ptr(CompassHUD_ptr);

// Instantiate group to hold all decorations:
   
   Decorations decorations(window_mgr_ptr,ModeController_ptr,CM_3D_ptr);
   Decorations decorations_global(
      window_mgr_global_ptr,ModeController_global_ptr,CM_3D_global_ptr);

// Instantiate AlirtGrid decorations group:

   AlirtGrid* local_grid_ptr=decorations.add_AlirtGrid(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   threevector* local_grid_origin_ptr=local_grid_ptr->get_world_origin_ptr();
   
   decorations.set_grid_origin_ptr(local_grid_origin_ptr);
   decorations_global.set_grid_origin_ptr(local_grid_origin_ptr);

// Instantiate group to hold movie:

   MoviesGroup movies_group(
      ndims,passes_group.get_pass_ptr(texturepass_ID),
      AnimationController_ptr);
   movies_group.set_OSGgroup_nodemask(1);

   MovieKeyHandler* MoviesKeyHandler_ptr=
      new MovieKeyHandler(ModeController_global_ptr,&movies_group);
   window_mgr_global_ptr->get_EventHandlers_ptr()->push_back(
      MoviesKeyHandler_ptr);

// Instantiate group to hold pointcloud information:

   PointCloudsGroup clouds_group(
      passes_group.get_pass_ptr(cloudpass_ID));
   PointCloud* PointCloud_ptr=clouds_group.generate_new_Cloud(
      passes_group.get_pass_ptr(cloudpass_ID));
   window_mgr_global_ptr->get_EventHandlers_ptr()->push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_ptr,
                               CM_3D_global_ptr));

// Suppress display of raw G76 ladar points:

   clouds_group.set_OSGgroup_nodemask(0);

   LatLongGridsGroup latlonggrids_group(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),CM_3D_global_ptr);
   EarthRegionsGroup earth_regions_group(
      passes_group.get_pass_ptr(cloudpass_ID),      
      &clouds_group,&latlonggrids_group,&movies_group);

   earth_regions_group.set_PassesGroup_ptr(&passes_group);
   earth_regions_group.set_AnimationController_ptr(AnimationController_ptr);

   bool place_onto_bluemarble_flag=false;
   bool generate_pointcloud_LatLongGrid_flag=false;
   bool display_SurfaceTexture_LatLonGrid_flag=true;
//   bool display_SurfaceTexture_LatLonGrid_flag=false;
   earth_regions_group.generate_regions(
      passes_group,place_onto_bluemarble_flag,
      generate_pointcloud_LatLongGrid_flag,
      display_SurfaceTexture_LatLonGrid_flag);
   root_global->addChild( earth_regions_group.get_OSGgroup_ptr() );

   LatLongGrid* LatLongGrid_ptr=latlonggrids_group.get_Grid_ptr(0); 
		// Surface texture latlong grid and NOT point cloud grid
   LatLongGrid_ptr->set_curr_color(
      colorfunc::get_OSG_color(colorfunc::brightpurple));
   LatLongGrid_ptr->update_grid_text_color();
   LatLongGrid_ptr->set_depth_buffering_off_flag(true);

   threevector* latlon_grid_origin_ptr=
      LatLongGrid_ptr->get_world_origin_ptr();
   cout << "*latlon_grid_origin_ptr = " << *latlon_grid_origin_ptr << endl;
   CM_3D_ptr->set_Grid_ptr(LatLongGrid_ptr);
   CM_3D_global_ptr->set_Grid_ptr(LatLongGrid_ptr);

// Initialize ALIRT grid based upon cloud's bounding box:

   decorations.get_AlirtGridsGroup_ptr()->initialize_grid(
      local_grid_ptr,clouds_group.get_xyz_bbox());
   local_grid_ptr->set_axes_labels(
      "Relative X (meters)","Relative Y (meters)");
   local_grid_ptr->set_delta_xy(50,50);
   local_grid_ptr->set_axis_char_label_size(20);
   local_grid_ptr->set_tick_char_label_size(10);
   local_grid_ptr->update_grid();

// Turn off local grid display since we already have a global lat-lon
// grid appearing in the 3D overhead view window:

   local_grid_ptr->toggle_mask();

// Instantiate a MyDatabasePager to handle paging of files from disk:

   viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager(
      clouds_group.get_SetupGeomVisitor_ptr(),
      clouds_group.get_ColorGeodeVisitor_ptr());
   clouds_group.get_HiresDataVisitor_ptr()->setDatabasePager(
      MyDatabasePager_ptr);

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

   double z_local_grid=local_grid_origin_ptr->get(2);
//   threevector global_camera_translation(0,0,z_grid+1);
//   threevector global_camera_translation(10,5,z_grid+1);
   threevector global_camera_translation(125,93.5,z_local_grid-1);
//   threevector global_camera_translation(125,93.5,z_grid+2);
//   threevector global_camera_translation(120,93.5,z_grid+2);
//   double global_daz=0*PI/180;

//   const double avg_canonical_edge_orientation = 56.33 * PI/180; 
//   const double avg_canonical_edge_orientation = 60 * PI/180; 
   const double avg_canonical_edge_orientation = 63 * PI/180; 

   double global_daz=PI/2-avg_canonical_edge_orientation;
   double global_del=0;
   double global_droll=0;
   threevector rotation_origin=*local_grid_origin_ptr;
   OBSFRUSTAGROUP_ptr->generate_still_imagery_frusta_for_photogroup(
      photogroup_ptr,global_camera_translation,
      global_daz,global_del,global_droll,rotation_origin);

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

   SignPostsGroup* SignPostsGroup_ptr=decorations_global.add_SignPosts(
      ndims,pass_ptr);
   SignPostsGroup_ptr->set_common_geometrical_size(0.0025);
   decorations.get_OBSFRUSTAKeyHandler_ptr()->
      set_SignPostsGroup_ptr(SignPostsGroup_ptr);

// Instantiate imageplane SignPosts group and pick handler:

   SignPostsGroup* imageplane_SignPostsGroup_ptr=new SignPostsGroup(
      ndims,pass_ptr,local_grid_origin_ptr);
   imageplane_SignPostsGroup_ptr->set_AnimationController_ptr(
      AnimationController_ptr);
   SignPostsGroup_ptr->set_imageplane_SignPostsGroup_ptr(
      imageplane_SignPostsGroup_ptr);
   imageplane_SignPostsGroup_ptr->set_common_geometrical_size(0.001);
//   imageplane_SignPostsGroup_ptr->set_common_geometrical_size(0.002);


/*
   SignPostPickHandler* imageplane_SignPostPickHandler_ptr=
      new SignPostPickHandler(
         ndims,pass_ptr,CM_3D_ptr,
         imageplane_SignPostsGroup_ptr,ModeController_ptr,
         window_mgr_ptr,grid_world_origin_ptr);
   imageplane_SignPostPickHandler_ptr->
      set_allow_doubleclick_in_manipulate_fused_data_mode(true);
   imageplane_SignPostPickHandler_ptr->set_DataNode_ptr(
      movie_ptr->getGeode());
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      imageplane_SignPostPickHandler_ptr);
*/

//   SignPostsGroup_ptr->set_imageplane_SignPostPickHandler_ptr(
//      imageplane_SignPostPickHandler_ptr);

//   decorations.get_OSGgroup_ptr()->addChild(imageplane_SignPostsGroup_ptr->
//                                            get_OSGgroup_ptr());

// Instantiate PanoramasGroup to hold multiple 360 degree panoramas:

   PanoramasGroup* PanoramasGroup_ptr=new PanoramasGroup(
      pass_ptr,OBSFRUSTAGROUP_ptr,decorations.get_OBSFRUSTUMPickHandler_ptr(),
      ArmySymbolsGroup_ptr,CM_3D_ptr,CM_3D_global_ptr);

   int n_OBSFRUSTA_per_panorama=10;
   PanoramasGroup_ptr->generate_panoramas(n_OBSFRUSTA_per_panorama);

   PanoramasGroup_ptr->add_panorama_network_link(0,1);
   PanoramasGroup_ptr->add_panorama_network_link(1,2);
   PanoramasGroup_ptr->add_panorama_network_link(2,3);
   PanoramasGroup_ptr->add_panorama_network_link(3,4);
   PanoramasGroup_ptr->add_panorama_network_link(4,5);
   PanoramasGroup_ptr->add_panorama_network_link(4,6);
   PanoramasGroup_ptr->add_panorama_network_link(6,7);
   PanoramasGroup_ptr->add_panorama_network_link(7,8);
   PanoramasGroup_ptr->add_panorama_network_link(8,9);

//   PanoramasGroup_ptr->translate(threevector(50,50));

   PanoramasKeyHandler* PanoramasKeyHandler_ptr=
      new PanoramasKeyHandler(PanoramasGroup_ptr,ModeController_ptr);
   PanoramasKeyHandler_ptr->set_SignPostsGroup_ptr(SignPostsGroup_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      PanoramasKeyHandler_ptr);

// Instantiate Polyhedra, Polygons & PolyLines decoration groups:

   PolyhedraGroup* PolyhedraGroup_ptr=decorations_global.add_Polyhedra(
      passes_group.get_pass_ptr(cloudpass_ID));

   PolyLinesGroup* PolyLinesGroup_ptr=decorations_global.add_PolyLines(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   PolyLinesGroup_ptr->set_width(4);

// Read in pre-calculated contours:

   string subdir="./south_lab/";
   string components_subdir=subdir+"components/";

   int c_start=0;
   int n_contours=198;
   for (int c=c_start; c<n_contours; c++)
   {
      cout << c << " " << flush;
      string contour_filename=components_subdir+
         "contour_"+stringfunc::integer_to_string(c,3)+".contour";
      filefunc::ReadInfile(contour_filename);

      polygon bottom_poly;
      unsigned int linenumber=0;
      bottom_poly.read_from_text_lines(linenumber,filefunc::text_line);

      vector<threevector> vertices;
      for (int n=0; n<bottom_poly.get_nvertices(); n++)
      {
         threevector curr_vertex(bottom_poly.get_vertex(n));
         curr_vertex.put(2,1);
         vertices.push_back(curr_vertex);
      }
      vertices.push_back(vertices.at(0));
      PolyLine* PolyLine_ptr=PolyLinesGroup_ptr->generate_new_PolyLine(
         vertices);
      PolyLine_ptr->set_color(
         colorfunc::get_OSG_color(colorfunc::yellow));
   } // loop over index c labeling contours
   PolyLinesGroup_ptr->set_uniform_color(
      colorfunc::get_OSG_color(colorfunc::cyan));

// Attach scene graphs to viewers:

   root->addChild(PanoramasGroup_ptr->get_OSGgroup_ptr());
   root->addChild(decorations.get_OSGgroup_ptr());
   root->addChild(imageplane_SignPostsGroup_ptr->get_OSGgroup_ptr());

// Globally rotate, scale and translate G76 ladar data plus panoramas
// so that they geoalign with aerial EO image:

   double interior_to_exterior_az=167.0158*PI/180;

// In order to geoalign *CM_3D_ptr's compass with true north, we
// must subtract off interior_to_exterior_az from CM_3D_ptr's current
// az value:

   CompassHUD_ptr->set_north_az_offset(-interior_to_exterior_az);

   rotation R(0,0,interior_to_exterior_az);
   rotation_origin=(-54.5281232306 , -42.8101004314 , 0);

   double scale=1.01857;
   threevector trans(313745.398657431 , 4703318.64428043 , 0);

// As of 2/3/10, we do not understand why trans doesn't precisely
// equal the difference between the exterior and interior feature
// COMs.  We empirically found that we need to add an additional fudge
// translation in order to georegister the interior map with the LL
// aerial photo:

   double fudge_x=-119;	// meters
   double fudge_y=-73;
   double fudge_z=-20;
   trans += threevector(fudge_x,fudge_y,fudge_z);

   threevector lowered_grid_origin=*latlon_grid_origin_ptr+2*fudge_z*z_hat;
   CM_3D_global_ptr->set_grid_origin_ptr(&lowered_grid_origin);

   osg::MatrixTransform* TotalTransform_ptr=
      osgfunc::generate_rot_scale_and_trans(rotation_origin,R,scale,trans);

   TotalTransform_ptr->addChild(decorations_global.get_OSGgroup_ptr());
   TotalTransform_ptr->addChild(PanoramasGroup_ptr->get_OSGgroup_ptr());
//   TotalTransform_ptr->addChild(
//      imageplane_SignPostsGroup_ptr->get_OSGgroup_ptr());
   TotalTransform_ptr->addChild(decorations.get_OSGgroup_ptr());
   TotalTransform_ptr->addChild(clouds_group.get_OSGgroup_ptr());
   root_global->addChild(TotalTransform_ptr);

   root_global->addChild(latlonggrids_group.get_OSGgroup_ptr());
   root_global->addChild( movies_group.get_OSGgroup_ptr() );

   window_mgr_ptr->setSceneData(root);
   window_mgr_global_ptr->setSceneData(root_global);

//   osg::Group* both = new osg::Group;
//   both->addChild(root_global);
//   both->addChild(TotalTransform_ptr);
   //  window_mgr_global_ptr->setSceneData(both);

// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_ptr->realize();
   window_mgr_global_ptr->realize();

// Set initial camera lateral posn to grid's midpoint and scale its
// altitude according to grid's maximal linear dimension:

   CM_3D_ptr->set_worldspace_center(LatLongGrid_ptr->get_world_middle());
   CM_3D_ptr->set_eye_to_center_distance(
      2*basic_math::max(
         LatLongGrid_ptr->get_xsize(),LatLongGrid_ptr->get_ysize()));
   CM_3D_ptr->update_M_and_Minv();

   CM_3D_global_ptr->set_worldspace_center(
      LatLongGrid_ptr->get_world_middle()+2*fudge_z*z_hat);
   CM_3D_global_ptr->set_eye_to_center_distance(
      3*basic_math::max(
         LatLongGrid_ptr->get_xsize(),LatLongGrid_ptr->get_ysize()));
   CM_3D_global_ptr->update_M_and_Minv();

   window_mgr_ptr->process();

// Start virtual camera at OBSFRUSTUM #0:

//   OBSFRUSTAGROUP_ptr->set_selected_Graphical_ID(0);

   PanoramasGroup_ptr->set_selected_Panorama_ID(2);
//   PanoramasGroup_ptr->set_selected_Panorama_ID(8);
   OBSFRUSTAGROUP_ptr->set_selected_Graphical_ID(2*n_OBSFRUSTA_per_panorama);

   int n_anim_steps=0;
   OBSFRUSTAGROUP_ptr->flyto_camera_location(
      OBSFRUSTAGROUP_ptr->get_selected_Graphical_ID(),n_anim_steps);

   while( !window_mgr_ptr->done() && !window_mgr_global_ptr->done() )
   {
      window_mgr_ptr->process();
      window_mgr_global_ptr->process();
   }

   delete window_mgr_ptr;
   delete window_mgr_global_ptr;
}

