// ========================================================================
// Program VIEW_TEL imports a set of TEL contours generated via
// program ALIGN_CONTOUR_VERTICES.  The TEL contours are represented
// via OSG PolyLines.  TEL tires are represented via OSG Cylinders.
// We hardwire TEL tire locations relative to the TEL's front "star"
// symbol within this program.

// VIEW_TEL displays the 3D TEL model on an XY grid.  It also displays
// the two internet ground photos as 3D frusta.  A user can
// doubleclick on the frusta in order to compare the 3D TEL's model
// projection with the ground photos.

// ========================================================================
// Last updated on 9/19/13; 9/20/13; 9/25/13; 9/26/13
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "osg/osgGrid/AlirtGridsGroup.h"
#include "osg/osgGraphicals/AnimationController.h"
#include "video/camera.h"
#include "math/constant_vectors.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/ModeController.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "geometry/plane.h"
#include "geometry/polyline.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
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

   string image_list_filename=passes_group.get_image_list_filename();
//   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string contours_subdir=bundler_IO_subdir+"contours/";

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group);
   int n_photos=photogroup_ptr->get_n_photos();

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("3D imagery");

// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

   Operations operations(ndims,window_mgr_ptr,passes_group);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   root->addChild(operations.get_OSGgroup_ptr());

// Add a custom manipulator to the event handler list:

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold all decorations:
   
   Decorations decorations(window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

// Instantiate AlirtGrid decorations group:

   double min_X=0;
   double max_X=150;
   double min_Y=0;
   double max_Y=150;
   double min_Z=0;

   cout << "min_X = " << min_X << " max_X = " << max_X << endl;
   cout << "min_Y = " << min_Y << " max_Y = " << max_Y << endl;
   AlirtGrid* grid_ptr=decorations.add_AlirtGrid(
      ndims,passes_group.get_pass_ptr(0),
      min_X,max_X,min_Y,max_Y,min_Z);

   threevector* grid_origin_ptr=grid_ptr->get_world_origin_ptr();
   CM_3D_ptr->set_Grid_ptr(grid_ptr);
   decorations.set_grid_origin_ptr(grid_origin_ptr);

   grid_ptr->set_axes_labels("X (Meters)","Y (Meters)");
   grid_ptr->set_delta_xy(10,10);
   grid_ptr->set_axis_char_label_size(5.0);
   grid_ptr->set_tick_char_label_size(5.0);

   grid_ptr->set_root_ptr(root);

// Instantiate OBSFRUSTAGROUP decoration group:

   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr=decorations.add_OBSFRUSTA(
      passes_group.get_pass_ptr(0),AnimationController_ptr);

// Instantiate an individual OBSFRUSTUM for every photograph:

   bool multicolor_frusta_flag=false;
   bool thumbnails_flag=true;
   OBSFRUSTAGROUP_ptr->generate_still_imagery_frusta_for_photogroup(
      photogroup_ptr,multicolor_frusta_flag,thumbnails_flag);

   decorations.get_OBSFRUSTUMPickHandler_ptr()->
      set_mask_nonselected_OSGsubPATs_flag(true);

// Instantiate Polyhedra & PolyLines decoration groups:

   PolyhedraGroup* PolyhedraGroup_ptr=decorations.add_Polyhedra(
      passes_group.get_pass_ptr(0));

   PolyLinesGroup* PolyLinesGroup_ptr=decorations.add_PolyLines(
      ndims,passes_group.get_pass_ptr(0));
   PolyLinesGroup_ptr->set_width(2);
   PolyLinesGroup_ptr->set_Pointsize_scalefactor(0.02);
//   PolyLinesGroup_ptr->set_Pointsize_scalefactor(0.1);
//   PolyLinesGroup_ptr->set_Pointsize_scalefactor(0.25);

//   string TEL_contours_filename=contours_subdir+"TEL_rays.dat";
//   string TEL_contours_filename=contours_subdir+"TEL_contours.dat";
   string TEL_contours_filename=contours_subdir+
      "aligned_correctedNAZ_TEL_contours.dat";
   PolyLinesGroup_ptr->reconstruct_polylines_from_file_info(
      TEL_contours_filename);

// Generate Polygon which displays rocket's reflection symmetry plane
// as it travels along parade road:

   threevector sym_plane_pt(60.1208, 49.2312,1.6156);
   sym_plane_pt -= *grid_origin_ptr;
   threevector n_hat(-0.9798235497 , 0.1998644826 , 0);
   double phi_n=atan2(n_hat.get(1),n_hat.get(0));
   cout << "phi_n = " << phi_n*180/PI << endl;

   double delta_phi_n=0;
//   cout << "Enter delta_phi_n in degs:" << endl;
//   cin >> delta_phi_n;
   phi_n += delta_phi_n*PI/180;
   n_hat=threevector(cos(phi_n),sin(phi_n));

   plane rocket_sym_plane(n_hat,sym_plane_pt);

   threevector a_hat=rocket_sym_plane.get_ahat();
   threevector b_hat=rocket_sym_plane.get_bhat();
   cout << "a_hat = " << rocket_sym_plane.get_ahat() << endl;
   cout << "b_hat = " << rocket_sym_plane.get_bhat() << endl;
   
   vector<threevector> V;
   V.push_back(sym_plane_pt+30*a_hat+30*b_hat);
   V.push_back(sym_plane_pt+30*a_hat-30*b_hat);
   V.push_back(sym_plane_pt-30*a_hat-30*b_hat);
   V.push_back(sym_plane_pt-30*a_hat+30*b_hat);
   V.push_back(V.front());

   polyline sym_plane_polyline(V);
   polygon sym_plane_poly(sym_plane_polyline);
   cout << "sym_plane_poly = " << sym_plane_poly << endl;

   osgGeometry::PolygonsGroup* PolygonsGroup_ptr=decorations.add_Polygons(
      ndims,passes_group.get_pass_ptr(0),AnimationController_ptr);

   osgGeometry::Polygon* Polygon_ptr=
      PolygonsGroup_ptr->generate_new_Polygon(
         *grid_origin_ptr,sym_plane_poly);
   Polygon_ptr->set_permanent_color(colorfunc::green,0.5);
   PolygonsGroup_ptr->reset_colors();

   PolygonsGroup_ptr->set_OSGgroup_nodemask(0);

// Instantiate cylinders decoration group.  Use OSG cylinders to model
// TEL tires:

   CylindersGroup* CylindersGroup_ptr=
      decorations.add_Cylinders(
         passes_group.get_pass_ptr(0),AnimationController_ptr);

   double tire_radius=1.917;  // m
   double tire_thickness=0.75; // m
   decorations.get_CylindersGroup_ptr()->set_rh(tire_radius,tire_thickness);

   rotation R;
   a_hat=z_hat.cross(n_hat);
//   cout << "n_hat = " << n_hat << endl;
//   cout << "a_hat = " << a_hat << endl;
   
   fourvector q(0,0,0,1);
   osg::Quat tire_quat(q.get(0),q.get(1),q.get(2),q.get(3));
   colorfunc::Color tire_color=colorfunc::cyan;

   vector<threevector> trans;
   threevector star_center(60.1208, 49.2312,1.6156);

// Tire pair 1:

   threevector curr_trans = star_center + 3.5*n_hat - 8.6*a_hat - 3*z_hat;
   trans.push_back(curr_trans);
   curr_trans = star_center - (3.5+tire_thickness)*n_hat - 8.6*a_hat - 3*z_hat;
   trans.push_back(curr_trans);

// Tire pair 2:

   curr_trans = star_center + 3.5*n_hat - (8.6+2.5*tire_radius)*a_hat
      - 3*z_hat;
   trans.push_back(curr_trans);
   curr_trans = star_center - (3.5+tire_thickness)*n_hat - 
      (8.6+2.5*tire_radius)*a_hat - 3*z_hat;
   trans.push_back(curr_trans);

// Tire pair 3:

   curr_trans = star_center + 3.5*n_hat - (8.6+5.2*tire_radius)*a_hat
      - 3*z_hat;
   trans.push_back(curr_trans);
   curr_trans = star_center - (3.5+tire_thickness)*n_hat - 
      (8.6+5.2*tire_radius)*a_hat - 3*z_hat;
   trans.push_back(curr_trans);

// Tire pair 4:

   curr_trans = star_center + 3.5*n_hat - (8.6+7.4*tire_radius)*a_hat
      - 3*z_hat;
   trans.push_back(curr_trans);
   curr_trans = star_center - (3.5+tire_thickness)*n_hat - 
      (8.6+7.4*tire_radius)*a_hat - 3*z_hat;
   trans.push_back(curr_trans);

// Tire pair 5:

   curr_trans = star_center + 3.5*n_hat - (8.6+9.8*tire_radius)*a_hat
      - 3*z_hat;
   trans.push_back(curr_trans);
   curr_trans = star_center - (3.5+tire_thickness)*n_hat - 
      (8.6+9.8*tire_radius)*a_hat - 3*z_hat;
   trans.push_back(curr_trans);

// Tire pair 6:

   curr_trans = star_center + 3.5*n_hat - (8.6+12.1*tire_radius)*a_hat
      - 3*z_hat;
   trans.push_back(curr_trans);
   curr_trans = star_center - (3.5+tire_thickness)*n_hat - 
      (8.6+12.1*tire_radius)*a_hat - 3*z_hat;
   trans.push_back(curr_trans);

// Tire pair 7:

   curr_trans = star_center + 3.5*n_hat - (8.6+14.4*tire_radius)*a_hat
      - 3*z_hat;
   trans.push_back(curr_trans);
   curr_trans = star_center - (3.5+tire_thickness)*n_hat - 
      (8.6+14.4*tire_radius)*a_hat - 3*z_hat;
   trans.push_back(curr_trans);

// Tire pair 8:

   curr_trans = star_center + 3.5*n_hat - (8.6+16.7*tire_radius)*a_hat
      - 3*z_hat;
   trans.push_back(curr_trans);
   curr_trans = star_center - (3.5+tire_thickness)*n_hat - 
      (8.6+16.7*tire_radius)*a_hat - 3*z_hat;
   trans.push_back(curr_trans);

   threevector tire_center(0,0,0);
   threevector rotation_origin(0,0,0);
   threevector new_zhat=n_hat;
   threevector new_xhat=a_hat;
   threevector new_yhat=z_hat;

//   bool render_tires_flag=false;
   bool render_tires_flag=true;
   if (render_tires_flag)
   {
      int n_tires=trans.size();
      for (int t=0; t<n_tires; t++)
      {
         Cylinder* curr_tire_ptr=CylindersGroup_ptr->generate_new_Cylinder(
            tire_center,tire_quat,tire_color);
         curr_tire_ptr->rotate_about_specified_origin_then_translate(
            0,0,rotation_origin,new_xhat,new_yhat,trans[t]);
      } // loop over index t labeling tires
   } // render_tires_flag
   
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

   delete window_mgr_ptr;
}

