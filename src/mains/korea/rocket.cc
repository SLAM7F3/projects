// ========================================================================
// Program ROCKET is a testing lab for instantiating and manipulating
// surfaces of revolution.  It displays a Google Earth aerial
// image as a background surface texture.  Building models based upon
// Google Earth information are also imported and displayed
// within the 3D map.  Calibrated ground photos can be inserted and
// displayed as OBSFRUSTA.
// ========================================================================
// Last updated on 8/4/13; 8/5/13; 8/6/13
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "osg/osgGraphicals/AnimationController.h"
#include "models/Building.h"
#include "models/BuildingsGroup.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "osg/osgGrid/LatLongGridsGroup.h"
#include "osg/ModeController.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "geometry/plane.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgGeometry/TrianglesGroup.h"
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
   Pass* pass_ptr=passes_group.get_pass_ptr(0);
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
   cout << "cloudpass_ID = " << cloudpass_ID << endl;
   int texturepass_ID=passes_group.get_curr_texturepass_ID();
   cout << "texturepass_ID = " << texturepass_ID << endl;

   string image_list_filename=passes_group.get_image_list_filename();
//   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
//   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string image_sizes_filename=passes_group.get_image_sizes_filename();
//   cout << "image_sizes_filename = " << image_sizes_filename << endl;

// Import ground photographs:

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
   }

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("3D imagery");
   
// Create OSG root node:

   osg::Group* root = new osg::Group;

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

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   root->addChild(operations.get_OSGgroup_ptr());

// Add a custom manipulator to the event handler list:

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);
   CM_3D_ptr->set_min_camera_height_above_grid(0.1);	// meters
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold all decorations:
   
   Decorations decorations(window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

// Instantiate groups to hold multiple surface textures,
// latitude-longitude grids and associated earth regions:

   LatLongGridsGroup latlonggrids_group(
      ndims,passes_group.get_pass_ptr(texturepass_ID),CM_3D_ptr);
   root->addChild(latlonggrids_group.get_OSGgroup_ptr());

// Instantiate Earth Region:

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

   LatLongGrid* LatLongGrid_ptr=latlonggrids_group.get_Grid_ptr(0);
   threevector* grid_origin_ptr=LatLongGrid_ptr->get_world_origin_ptr();
   cout << "*grid_origin_ptr = " << *grid_origin_ptr << endl;

   CM_3D_ptr->set_Grid_ptr(LatLongGrid_ptr);
   decorations.set_grid_origin_ptr(grid_origin_ptr);

// Experiment with surfaces of revolution:
   
   vector<twovector> rz_vertices;
//   rz_vertices.push_back(twovector(2,0));
//   rz_vertices.push_back(twovector(2,1));
//   rz_vertices.push_back(twovector(1,2));
//   rz_vertices.push_back(twovector(1,3));
//   rz_vertices.push_back(twovector(0,4));

   rz_vertices.push_back(twovector(1.21316903141,0));
   rz_vertices.push_back(twovector(1.21316903141,14.8158353468));
   rz_vertices.push_back(twovector(1.12993101728,15.46066867));
   rz_vertices.push_back(twovector(0.928576049479,16.0016517109));
   rz_vertices.push_back(twovector(0.930967366112,16.844722949));
   rz_vertices.push_back(twovector(0.922664663436,18.1414677011));
   rz_vertices.push_back(twovector(0.9059453193,18.7280304601));
   rz_vertices.push_back(twovector(0.802358469977,19.4249366473));
   rz_vertices.push_back(twovector(0.509900085256,20.3666240163));
   rz_vertices.push_back(twovector(0.400590774925,22.078035925));
   rz_vertices.push_back(twovector(0.288146513934,22.5680276077));
   rz_vertices.push_back(twovector(0.242257886938,22.7954596953));
   rz_vertices.push_back(twovector(0,23.0844482106));

   double rocket_height=23.0844482106;
   threevector s_hat(-0.1998644826 , -0.9798235497, 0); // sym axis
//   threevector rocket_tip(738202.3637, 4322535.217, 22.1797492896);
   threevector rocket_tip(738234.455998,4322562.24282,23.6495873436);
   threevector rocket_base=rocket_tip-rocket_height*s_hat;

   polyhedron SoR;
   SoR.generate_surface_of_revolution(rz_vertices);

/*
   threevector SoR_tip=SoR.get_tip();
   threevector SoR_base=SoR.get_base();
   double rocket_height=(SoR_tip-SoR_base).magnitude();


   double z_ground=19;
   double rocket_easting=  738243;
   double rocket_northing=4322624;
   double rocket_altitude_above_ground=5;
   double rocket_altitude=z_ground+5;

//   threevector rocket_tip(
//      rocket_easting,rocket_northing,rocket_altitude);

   threevector relative_tip(20,11,9);   
   threevector relative_base(0,9,5);
   threevector base_minus_tip=relative_base-relative_tip;
   threevector rocket_base=rocket_tip+base_minus_tip;
   threevector trans=rocket_base;

   double true_rocket_height=(rocket_tip-rocket_base).magnitude();   // meters
//   threevector scale(1,1,true_rocket_height/rocket_height);

//   threevector r_hat=(rocket_tip-rocket_base).unitvector();
*/

   threevector trans=rocket_base;
   threevector scale(1,1,1);
   double theta=acos(s_hat.get(2));
   double phi=atan2(s_hat.get(1),s_hat.get(0));

// Generate Polygon which displays rocket's symmetry axis as it
// travels along parade road:

   threevector sym_plane_pt(738239.2 , 4322585.5 , 19);
   sym_plane_pt -= *grid_origin_ptr;

   threevector nhat(-0.9798235497 , 0.1998644826 , 0);
   plane rocket_sym_plane(nhat,sym_plane_pt);

   threevector a_hat=rocket_sym_plane.get_ahat();
   threevector b_hat=rocket_sym_plane.get_bhat();
//   cout << "a_hat = " << rocket_sym_plane.get_ahat() << endl;
//   cout << "b_hat = " << rocket_sym_plane.get_bhat() << endl;
   
   vector<threevector> V;
   V.push_back(sym_plane_pt+100*a_hat+100*b_hat);
   V.push_back(sym_plane_pt+100*a_hat-100*b_hat);
   V.push_back(sym_plane_pt-100*a_hat-100*b_hat);
   V.push_back(sym_plane_pt-100*a_hat+100*b_hat);
   V.push_back(V.front());

   polyline sym_plane_polyline(V);
   polygon sym_plane_poly(sym_plane_polyline);
   cout << "sym_plane_poly = " << sym_plane_poly << endl;

   osgGeometry::PolygonsGroup* PolygonsGroup_ptr=decorations.add_Polygons(
      ndims,passes_group.get_pass_ptr(texturepass_ID),AnimationController_ptr);

   osgGeometry::Polygon* Polygon_ptr=
      PolygonsGroup_ptr->generate_new_Polygon(
         *grid_origin_ptr,sym_plane_poly);
   Polygon_ptr->set_permanent_color(colorfunc::green,0.5);
   PolygonsGroup_ptr->reset_colors();

//   PolygonsGroup_ptr->set_OSGgroup_nodemask(0);

// Instantiate Polyhedra decoration group:

   PolyhedraGroup* PolyhedraGroup_ptr=decorations.add_Polyhedra(
      passes_group.get_pass_ptr(texturepass_ID));
   Polyhedron* SoRPolyhedron_ptr=PolyhedraGroup_ptr->
      generate_new_Polyhedron(&SoR);
   SoRPolyhedron_ptr->scale_rotate_and_then_translate(
      0,0,theta,phi,scale,trans);

   osg::Vec4 edge_color=colorfunc::get_OSG_color(colorfunc::red);
   osg::Vec4 volume_color=colorfunc::get_OSG_color(colorfunc::pink);

   SoRPolyhedron_ptr->set_color(edge_color,volume_color);
   SoRPolyhedron_ptr->reset_volume_alpha(0.25);

// Instantiate BuildingsGroup object:

   BuildingsGroup* BuildingsGroup_ptr=new BuildingsGroup();
   string OFF_subdir="./OFF/";

   BuildingsGroup_ptr->import_from_OFF_files(OFF_subdir);
   PolyhedraGroup_ptr->generate_Building_Polyhedra(BuildingsGroup_ptr);

// Instantiate PolyLines decorations group to hold street lines:

   PolyLinesGroup* PolyLinesGroup_ptr=
      decorations.add_PolyLines(ndims,passes_group.get_pass_ptr(
         texturepass_ID));
   PolyLinesGroup_ptr->set_width(3);
   string street_polylines_filename="./camera_calib/street_polylines.dat";
   PolyLinesGroup_ptr->reconstruct_polylines_from_file_info(
      street_polylines_filename);

// Instantiate OBSFRUSTA decoration group:

   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr=decorations.add_OBSFRUSTA(
      passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
   OBSFRUSTAGROUP_ptr->set_enable_OBSFRUSTA_blinking_flag(false);
   OBSFRUSTAGROUP_ptr->set_erase_other_OBSFRUSTA_flag(true);

// Instantiate an individual OBSFRUSTUM for every photograph:

   bool multicolor_frusta_flag=false;
   bool thumbnails_flag=true;
   OBSFRUSTAGROUP_ptr->generate_still_imagery_frusta_for_photogroup(
      photogroup_ptr,multicolor_frusta_flag,thumbnails_flag);

   decorations.get_OBSFRUSTUMPickHandler_ptr()->
      set_mask_nonselected_OSGsubPATs_flag(true);

// Add all decorations to root node:

   root->addChild(decorations.get_OSGgroup_ptr());

// Attach scene graph to viewer:

   window_mgr_ptr->setSceneData(root);

// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_ptr->realize();

   while( !window_mgr_ptr->done() )
   {
      window_mgr_ptr->process();
   }

   delete BuildingsGroup_ptr;
   delete window_mgr_ptr;
}

