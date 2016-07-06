// ========================================================================
// Program SoR imports camera calibration parameters for some internet
// photo of a Korean rocket.  It further imports manually-selected
// point along the rocket's hull.  Assuming the rocket's symmetry axis
// lies within a constant z-plane, we model the rocket as a surface of
// revolution.  SoR exports a pairs of radius,z coordinates for the
// rocket's hull.  The r,z pairs are hardwired within 3D visualization
// program ROCKET.
// ========================================================================
// Last updated on 8/6/13
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

   photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(0);
   camera* camera_ptr=photo_ptr->get_camera_ptr();
   threevector camera_posn=camera_ptr->get_world_posn();
   threevector grid_origin(737871.323253 , 4322310.6757 , 20.0961298341);

// Rocket's symmetry axis = shat.  Rocket appears to lie horizontal
// wrt ground Z-plane:
   
   threevector shat(-0.1998644826 , -0.9798235497, 0);

   threevector nhat(-0.9798235497 , 0.1998644826 , 0);
   threevector sym_plane_pt(738239.2 , 4322585.5 , 19);
   plane rocket_sym_plane(nhat,sym_plane_pt);

   rocket_sym_plane.set_ahat(shat);
   rocket_sym_plane.set_bhat(z_hat);

// Import manually-selected points along rocket's hull from
// ground_photo1.jpg:

//   string input_filename="features_rocket_2D.txt";
   string input_filename="features_2D_TEL_wheels.txt";
   filefunc::ReadInfile(input_filename);
   
   threevector curr_intersection;
   vector<threevector> intersection_point;
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      double U=column_values[3];
      double V=column_values[4];
      threevector ray_hat=camera_ptr->pixel_ray_direction(U,V);
      if (rocket_sym_plane.ray_intersection(
         camera_posn,ray_hat,curr_intersection))
      {
//         cout << "i = " << i 
//              << " intersection = " << curr_intersection << endl;

         intersection_point.push_back(curr_intersection);

         double time=0;
         int polyline_ID=2+i;
         int passnumber=0;
         double red=1;
         double green=0;
         double blue=0;
         double alpha=1;
         
         cout << time << " "
              << polyline_ID << " "
              << passnumber << " "
              << camera_posn.get(0) << " "
              << camera_posn.get(1) << " "
              << camera_posn.get(2) << " "
              << red << " "
              << green << " "
              << blue << " "
              << alpha << endl;
         cout << time << " "
              << polyline_ID << " "
              << passnumber << " "
              << curr_intersection.get(0) << " "
              << curr_intersection.get(1) << " "
              << curr_intersection.get(2) << " "
              << red << " "
              << green << " "
              << blue << " "
              << alpha << endl << endl;

      }
   } // loop over index i labeling manually selected rocket hull points

   for (int i=0; i<intersection_point.size()-1; i++)
   {
      double curr_dist=(intersection_point[i+1]-intersection_point[i]).
         magnitude();
      cout << "i = " << i << " curr_dist = " << curr_dist << endl;
   }
   

   exit(-1);

   vector<twovector> hull_planar_coords=
      rocket_sym_plane.planar_coords(intersection_point);
   vector<twovector> rz_vertices;
//   for (int i=0; i<hull_planar_coords.size(); i++)
   for (int i=hull_planar_coords.size()-1; i>=0; i--)
   {
//      cout << "i = " << i << " hull planar coords = "
//           << hull_planar_coords[i] << endl;
      double curr_R=hull_planar_coords[i].get(1)-
         hull_planar_coords.front().get(1);
      double curr_Z=hull_planar_coords[i].get(0)-
         hull_planar_coords.back().get(0);
//      cout << "i = " << i 
//           << " R = " << curr_R << " Z = " << curr_Z << endl;

      cout << "rz_vertices.push_back(twovector("
           << curr_R << "," << curr_Z << "));" << endl;
   }
   


/*



// Experiment with surfaces of revolution:
   
   vector<twovector> rz_vertices;
   rz_vertices.push_back(twovector(2,0));
   rz_vertices.push_back(twovector(2,1));
   rz_vertices.push_back(twovector(1,2));
   rz_vertices.push_back(twovector(1,3));
   rz_vertices.push_back(twovector(0,4));

   polyhedron SoR;
   SoR.generate_surface_of_revolution(rz_vertices);
   threevector SoR_tip=SoR.get_tip();
   threevector SoR_base=SoR.get_base();
   double rocket_height=(SoR_tip-SoR_base).magnitude();

   double z_ground=19;
   double rocket_easting=  738243;
   double rocket_northing=4322624;
   double rocket_altitude_above_ground=5;
   double rocket_altitude=z_ground+5;

   threevector rocket_tip(
      rocket_easting,rocket_northing,rocket_altitude);

   threevector relative_tip(20,11,9);   
   threevector relative_base(0,9,5);
   threevector base_minus_tip=relative_base-relative_tip;
   threevector rocket_base=rocket_tip+base_minus_tip;
   threevector trans=rocket_base;

   double true_rocket_height=(rocket_tip-rocket_base).magnitude();   // meters
   threevector scale(1,1,true_rocket_height/rocket_height);
   threevector r_hat=(rocket_tip-rocket_base).unitvector();
   double theta=acos(r_hat.get(2));
   double phi=atan2(r_hat.get(1),r_hat.get(0));

// Generate Polygon which displays rocket's symmetry axis as it
// travels along parade road:

//   threevector sym_plane_pt(738239.8 , 4322607.7 , 19);
   threevector sym_plane_pt(738239.2 , 4322585.5 , 19);
   sym_plane_pt -= grid_origin;
   threevector reference_origin=grid_origin;
   
   threevector nhat(-0.9798235497 , 0.1998644826 , 0);
   plane rocket_sym_plane(nhat,reference_origin);

   threevector a_hat=rocket_sym_plane.get_ahat();
   threevector b_hat=rocket_sym_plane.get_bhat();
   cout << "a_hat = " << rocket_sym_plane.get_ahat() << endl;
   cout << "b_hat = " << rocket_sym_plane.get_bhat() << endl;
   
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
         reference_origin,sym_plane_poly);
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
*/

}

