// ========================================================================
// Program VIEWPANOS is a variant of "Google Streets" meant for indoor
// panorama viewing.
// ========================================================================
// Last updated on 2/23/11; 2/24/11; 3/8/11
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

   Pass* texturepass_ptr=passes_group.get_pass_ptr(texturepass_ID);
   string texture_filename=texturepass_ptr->get_first_filename();
   cout << "texture_filename = " << texture_filename << endl;

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group);
//   int n_photos=photogroup_ptr->get_n_photos();

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
   bool hide_Mode_HUD_flag=true;
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

// Instantiate compass:

//   CompassHUD* CompassHUD_ptr=new CompassHUD(colorfunc::green);
//   CompassHUD_ptr->set_nadir_oriented_compass_flag(false);
//   root->addChild(CompassHUD_ptr->getProjection());
//   CM_3D_ptr->set_CompassHUD_ptr(CompassHUD_ptr);

// Instantiate group to hold all decorations:
   
   Decorations decorations(window_mgr_ptr,ModeController_ptr,CM_3D_ptr);
   Decorations decorations_global(
      window_mgr_global_ptr,ModeController_global_ptr,CM_3D_global_ptr);

// Instantiate AlirtGrid decorations group:

   double min_X=0;
   double max_X=40;	// meters  (Aud #2)
//   double max_X=50;	// meters  (Feb 9 Auditorium)

   double min_Y=0;
   double max_Y=45;	// meters  (Aud #2)
//   double max_Y=40;	// meters  (Feb 9 Auditorium)
   double min_Z=0;

   cout << "min_X = " << min_X << " max_X = " << max_X << endl;
   cout << "min_Y = " << min_Y << " max_Y = " << max_Y << endl;
   cout << "min_Z = " << min_Z << endl;
   AlirtGrid* grid_ptr=decorations.add_AlirtGrid(
      ndims,pass_ptr,min_X,max_X,min_Y,max_Y,min_Z);

   threevector* grid_origin_ptr=grid_ptr->get_world_origin_ptr();
   cout << "*grid_origin_ptr = " << *grid_origin_ptr << endl;
   double z_grid=grid_origin_ptr->get(2);
//   cout << "z_grid = " << z_grid << endl;

   CM_3D_ptr->set_Grid_ptr(grid_ptr);
   CM_3D_global_ptr->set_Grid_ptr(grid_ptr);
   decorations.set_grid_origin_ptr(grid_origin_ptr);
   decorations_global.set_grid_origin_ptr(grid_origin_ptr);

   grid_ptr->set_axes_labels("Relative X (meters)","Relative Y (meters)");
   grid_ptr->set_delta_xy(5,5);		// Auditorium
   grid_ptr->set_axis_char_label_size(3);	// Auditorium
   grid_ptr->set_tick_char_label_size(1);	// Auditorium
//   grid_ptr->set_axis_char_label_size(15);	// G47 area
//   grid_ptr->set_tick_char_label_size(4);	// G47 area
   grid_ptr->set_curr_color(colorfunc::get_OSG_color(colorfunc::blgr));
   grid_ptr->update_grid();

// Instantiate group to hold pointcloud information:

   PointCloudsGroup clouds_group(
      passes_group.get_pass_ptr(cloudpass_ID),grid_origin_ptr);
   if (cloudpass_ID >= 0)
   {
//      PointCloud* PointCloud_ptr=
         clouds_group.generate_new_Cloud(
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

// Auditorium data set #2 floorplan:

   double Umin=-14.024;
   double Umax=29.176;
   double Vmin=-42.988;
   double Vmax=7.512;

/*
// Feb 9 auditorium floorplan;

   double Umin=-11.2998;
   double Umax=44.112;
   double Vmin=11.5885;
   double Vmax=52.6337;
*/

//   threevector global_camera_translation=Zero_vector;
   threevector global_camera_translation(-Umin,-Vmin,z_grid+1);	// aud set #2
//   threevector global_camera_translation(13.5,-10,z_grid+1);	// Feb 9 aud
//   threevector global_camera_translation(0,0,z_grid+1);
//   threevector global_camera_translation(40,10,z_grid+1);	// auditorium
//   threevector global_camera_translation(30,-20,z_grid+2);	// G47 area
//   threevector global_camera_translation(10,5,z_grid+1);
//   threevector global_camera_translation(120,93.5,z_grid+2);
//   threevector global_camera_translation(125,93.5,z_grid+2);
//   threevector global_camera_translation(125,93.5,z_grid+2);

   threevector bottom_left(Umin,Vmin,-0.2);
   threevector bottom_right(Umax,Vmin,-0.2);
   threevector top_right(Umax,Vmax,-0.2);
   threevector top_left(Umin,Vmax,-0.2);

   top_right += *grid_origin_ptr + 
      threevector(twovector(global_camera_translation));
   top_left += *grid_origin_ptr + 
      threevector(twovector(global_camera_translation));
   bottom_left += *grid_origin_ptr + 
      threevector(twovector(global_camera_translation));
   bottom_right += *grid_origin_ptr + 
      threevector(twovector(global_camera_translation));

//   cout << "top_right = " << top_right << endl;
//   cout << "top_left = " << top_left << endl;
//   cout << "bottom_right = " << bottom_right << endl;
//   cout << "bottom_left = " << bottom_left << endl;

//   double global_daz=0*PI/180;
//   cout << "Enter global_daz in degs:" << endl;
//   cin >> global_daz;
//   global_daz *= PI/180;

//   const double avg_canonical_edge_orientation = 0 * PI/180; 
//   const double avg_canonical_edge_orientation = 63 * PI/180; // G47 area

   double global_daz=0;
   double global_del=0;
   double global_droll=0;
   threevector rotation_origin=*grid_origin_ptr;
//   double local_spin_daz=0*PI/180;
//   double local_spin_daz=6.5*36*PI/180;
   double local_spin_daz=6.6*36*PI/180;	// Aud data #2 fudge factor
//   double local_spin_daz=7*36*PI/180;
//   double local_spin_daz=130*PI/180;	// G47 area

   colorfunc::Color OBSFRUSTUM_color=colorfunc::blue;
//   bool thumbnails_flag=false;
   bool thumbnails_flag=true;
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

   SignPostsGroup* SignPostsGroup_ptr=decorations_global.add_SignPosts(
      ndims,pass_ptr);
   SignPostsGroup_ptr->set_common_geometrical_size(0.0025);
   decorations.get_OBSFRUSTAKeyHandler_ptr()->
      set_SignPostsGroup_ptr(SignPostsGroup_ptr);

// Instantiate imageplane SignPosts group and pick handler:

   SignPostsGroup* imageplane_SignPostsGroup_ptr=new SignPostsGroup(
      ndims,pass_ptr,grid_origin_ptr);
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

   colorfunc::Color permanent_pano_color=colorfunc::blue;
   colorfunc::Color selected_pano_color=colorfunc::red;
   PanoramasGroup_ptr->set_colors(permanent_pano_color,selected_pano_color);

   int n_OBSFRUSTA_per_panorama=10;
   PanoramasGroup_ptr->generate_panoramas(n_OBSFRUSTA_per_panorama);

/*
   PanoramasGroup_ptr->add_panorama_network_link(0,1);
   PanoramasGroup_ptr->add_panorama_network_link(1,2);
   PanoramasGroup_ptr->add_panorama_network_link(2,3);
   PanoramasGroup_ptr->add_panorama_network_link(3,4);
   PanoramasGroup_ptr->add_panorama_network_link(4,5);
   PanoramasGroup_ptr->add_panorama_network_link(5,6);
   PanoramasGroup_ptr->add_panorama_network_link(6,7);
   PanoramasGroup_ptr->add_panorama_network_link(7,8);
   PanoramasGroup_ptr->add_panorama_network_link(8,9);
   PanoramasGroup_ptr->add_panorama_network_link(9,10);

   PanoramasGroup_ptr->add_panorama_network_link(10,11);
   PanoramasGroup_ptr->add_panorama_network_link(11,12);
   PanoramasGroup_ptr->add_panorama_network_link(12,13);
   PanoramasGroup_ptr->add_panorama_network_link(13,14);
   PanoramasGroup_ptr->add_panorama_network_link(14,15);
   PanoramasGroup_ptr->add_panorama_network_link(15,16);
   PanoramasGroup_ptr->add_panorama_network_link(16,17);
   PanoramasGroup_ptr->add_panorama_network_link(17,18);
   PanoramasGroup_ptr->add_panorama_network_link(18,19);
   PanoramasGroup_ptr->add_panorama_network_link(19,20);

   PanoramasGroup_ptr->add_panorama_network_link(20,21);
   PanoramasGroup_ptr->add_panorama_network_link(21,22);
   PanoramasGroup_ptr->add_panorama_network_link(22,23);
   PanoramasGroup_ptr->add_panorama_network_link(23,24);
   PanoramasGroup_ptr->add_panorama_network_link(24,25);
   PanoramasGroup_ptr->add_panorama_network_link(25,26);
   PanoramasGroup_ptr->add_panorama_network_link(26,27);
   PanoramasGroup_ptr->add_panorama_network_link(27,28);
   PanoramasGroup_ptr->add_panorama_network_link(28,29);
   PanoramasGroup_ptr->add_panorama_network_link(29,30);
*/

/*

// G47 area

   PanoramasGroup_ptr->add_panorama_network_link(0,6);
   PanoramasGroup_ptr->add_panorama_network_link(1,2);
   PanoramasGroup_ptr->add_panorama_network_link(1,6);
   PanoramasGroup_ptr->add_panorama_network_link(1,7);
   PanoramasGroup_ptr->add_panorama_network_link(2,3);
   PanoramasGroup_ptr->add_panorama_network_link(3,4);
   PanoramasGroup_ptr->add_panorama_network_link(3,6);
   PanoramasGroup_ptr->add_panorama_network_link(4,5);
   PanoramasGroup_ptr->add_panorama_network_link(7,8);
   PanoramasGroup_ptr->add_panorama_network_link(7,9);
   PanoramasGroup_ptr->add_panorama_network_link(9,10);
   PanoramasGroup_ptr->add_panorama_network_link(10,11);
   PanoramasGroup_ptr->add_panorama_network_link(11,13);
   PanoramasGroup_ptr->add_panorama_network_link(12,13);
   PanoramasGroup_ptr->add_panorama_network_link(13,18);
   PanoramasGroup_ptr->add_panorama_network_link(14,15);
   PanoramasGroup_ptr->add_panorama_network_link(14,19);
   PanoramasGroup_ptr->add_panorama_network_link(15,16);
   PanoramasGroup_ptr->add_panorama_network_link(16,17);
   PanoramasGroup_ptr->add_panorama_network_link(17,18);
   PanoramasGroup_ptr->add_panorama_network_link(17,18);
   PanoramasGroup_ptr->add_panorama_network_link(19,20);
   PanoramasGroup_ptr->add_panorama_network_link(20,21);
   PanoramasGroup_ptr->add_panorama_network_link(21,22);
   PanoramasGroup_ptr->add_panorama_network_link(22,23);
   PanoramasGroup_ptr->add_panorama_network_link(22,39);
   PanoramasGroup_ptr->add_panorama_network_link(23,24);
   PanoramasGroup_ptr->add_panorama_network_link(24,25);
   PanoramasGroup_ptr->add_panorama_network_link(25,26);
   PanoramasGroup_ptr->add_panorama_network_link(25,27);
   PanoramasGroup_ptr->add_panorama_network_link(27,28);
   PanoramasGroup_ptr->add_panorama_network_link(28,29);
   PanoramasGroup_ptr->add_panorama_network_link(29,30);
   PanoramasGroup_ptr->add_panorama_network_link(30,38);
   PanoramasGroup_ptr->add_panorama_network_link(30,31);
   PanoramasGroup_ptr->add_panorama_network_link(31,32);
   PanoramasGroup_ptr->add_panorama_network_link(32,33);
   PanoramasGroup_ptr->add_panorama_network_link(33,34);
   PanoramasGroup_ptr->add_panorama_network_link(34,35);
   PanoramasGroup_ptr->add_panorama_network_link(35,36);
   PanoramasGroup_ptr->add_panorama_network_link(36,37);
   PanoramasGroup_ptr->add_panorama_network_link(37,38);
   PanoramasGroup_ptr->add_panorama_network_link(39,40);
   PanoramasGroup_ptr->add_panorama_network_link(40,41);
*/
   
//   PanoramasGroup_ptr->translate(threevector(30,-20));

   PanoramasKeyHandler* PanoramasKeyHandler_ptr=
      new PanoramasKeyHandler(PanoramasGroup_ptr,ModeController_ptr);
   PanoramasKeyHandler_ptr->set_SignPostsGroup_ptr(SignPostsGroup_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      PanoramasKeyHandler_ptr);

   PanoramaPickHandler* PanoramaPickHandler_ptr=new PanoramaPickHandler(
         pass_ptr,CM_3D_global_ptr,PanoramasGroup_ptr,
         ModeController_ptr,window_mgr_global_ptr,grid_origin_ptr);
   window_mgr_global_ptr->get_EventHandlers_ptr()->push_back(
         PanoramaPickHandler_ptr);

/*
// Instantiate Polyhedra, Polygons & PolyLines decoration groups:

   PolyhedraGroup* PolyhedraGroup_ptr=decorations_global.add_Polyhedra(
      passes_group.get_pass_ptr(texturepass_ID));

   PolyLinesGroup* PolyLinesGroup_ptr=decorations_global.add_PolyLines(
      ndims,passes_group.get_pass_ptr(texturepass_ID));
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
      colorfunc::get_OSG_color(colorfunc::blue));
*/

// Instantiate group to hold 2D ladar floorplan:

   MoviesGroup movies_group(
      ndims,passes_group.get_pass_ptr(texturepass_ID),AnimationController_ptr);
   texture_rectangle* texture_rectangle_ptr=
      movies_group.generate_new_texture_rectangle(texture_filename);
   Movie* movie_ptr=movies_group.generate_new_Movie(texture_rectangle_ptr);

   movie_ptr->reset_geom_vertices(
      top_right,top_left,bottom_left,bottom_right);

// Generate modified Delaunay triangle network which allows viewer to
// fly among panorama centers:

   PanoramasGroup_ptr->Delaunay_triangulate_pano_centers(
      bottom_left,top_right,movie_ptr);
   PanoramasGroup_ptr->add_panorama_network_link(4,5);
   PanoramasGroup_ptr->add_panorama_network_link(5,6);

// Attach scene graphs to viewers:

   root->addChild(PanoramasGroup_ptr->get_OSGgroup_ptr());
   root->addChild(decorations.get_OSGgroup_ptr());
//   root->addChild(imageplane_SignPostsGroup_ptr->get_OSGgroup_ptr());

   root_global->addChild(PanoramasGroup_ptr->get_OSGgroup_ptr());
   root_global->addChild(decorations.get_OSGgroup_ptr());
   root_global->addChild(decorations_global.get_OSGgroup_ptr());

   root_global->addChild(movies_group.get_OSGgroup_ptr());

   window_mgr_ptr->setSceneData(root);
   window_mgr_global_ptr->setSceneData(root_global);
//   window_mgr_global_ptr->setSceneData(TotalTransform_ptr);

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

   CM_3D_ptr->set_worldspace_center(grid_ptr->get_world_middle());
   CM_3D_ptr->set_eye_to_center_distance(
      2*basic_math::max(grid_ptr->get_xsize(),grid_ptr->get_ysize()));
   CM_3D_ptr->update_M_and_Minv();

   CM_3D_global_ptr->set_worldspace_center(
      grid_ptr->get_world_middle());
   CM_3D_global_ptr->set_eye_to_center_distance(
      2*basic_math::max(grid_ptr->get_xsize(),grid_ptr->get_ysize()));
   CM_3D_global_ptr->update_M_and_Minv();

   window_mgr_ptr->process();

// Start virtual camera at OBSFRUSTUM #0:

   int initial_pano_ID=0;

//   OBSFRUSTAGROUP_ptr->set_selected_Graphical_ID(1);
   OBSFRUSTAGROUP_ptr->set_selected_Graphical_ID(
      initial_pano_ID*n_OBSFRUSTA_per_panorama);
//   OBSFRUSTAGROUP_ptr->set_selected_Graphical_ID(8*n_OBSFRUSTA_per_panorama);

// Hardwire horizontal and vertical fields-of-view for "1st person"
// window which are appropriate for Auditorium data set #2 collected
// by Lady Bug in Feb 2011:

   double hfov=75.195;
   double vfov=57.337;
   window_mgr_ptr->set_viewer_horiz_vert_fovs(hfov,vfov);

   PanoramasGroup_ptr->recolor_start_stop_panoramas(
      initial_pano_ID,initial_pano_ID);
//   PanoramasGroup_ptr->set_selected_Panorama_ID(8);
   PanoramasGroup_ptr->load_hires_panels(initial_pano_ID);

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

