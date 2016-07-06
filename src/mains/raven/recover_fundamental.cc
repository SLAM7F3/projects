// ========================================================================
// Program FUNDRECOVER
// ========================================================================
// Last updated on 4/3/12; 4/4/12; 4/5/12; 4/6/12; 2/28/13
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "video/camerafuncs.h"
#include "general/filefuncs.h"
#include "structmotion/fundamental.h"
#include "geometry/geometry_funcs.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "general/sysfuncs.h"

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
   PassesGroup passes_group(&arguments);

   string image_list_filename=passes_group.get_image_list_filename();
   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;

   string image_sizes_filename=passes_group.get_image_sizes_filename();
   cout << "image_sizes_filename = " << image_sizes_filename << endl;
//   string common_planes_filename=passes_group.get_common_planes_filename();
//   cout << "common_planes_filename = " << common_planes_filename << endl;

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group,image_sizes_filename);
   photogroup_ptr->set_bundler_IO_subdir(bundler_IO_subdir);
   int n_photos=photogroup_ptr->get_n_photos();
   cout << "n_photos = " << n_photos << endl;

   photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(0);
   camera* camera_ptr=photo_ptr->get_camera_ptr();
   
   photograph* photoprime_ptr=photogroup_ptr->get_photograph_ptr(1);
   camera* cameraprime_ptr=photoprime_ptr->get_camera_ptr();

   fundamental* fundamental_ptr=new fundamental();

/*

// Calculate fundamental matrix derived from bundler solutions for
// projection matrices:

   genmatrix* Funprimeprime_ptr=
      camerafunc::calculate_fundamental_matrix(camera_ptr,cameraprime_ptr);
   cout << "Funprimeprime = " << *Funprimeprime_ptr << endl;

   fundamental_ptr->set_F_values(*Funprimeprime_ptr);

   cout << "epipole_XY = " << fundamental_ptr->get_epipole_XY() << endl;
   cout << "epipole_UV = " << fundamental_ptr->get_epipole_UV() << endl;
*/



/*
*F_ptr = 
0.0398717158995	0.671358648019	-0.829766848904	
0.67040042255	0.00715254034214	-3.21051279534	
-0.638327794729	2.15349779291	1	
*/

// Fundamental matrix for kermit0 and kermit2:
// kermit0 = unprimed (UV) image
// kermit2 = primed (XY) image

// XY^trans * F * UV = 0

// XY <--> primed coordinates
// UV <--> unprimed coordinates

// epipole_XY = eprime
// -3.219855529
// 1.143658228
// 1

// epipole_UV = e
// 4.778789599
// 0.9521413176
// 1

   genmatrix F_kermit(3,3);
   F_kermit.put(0,0,0.0398717158995);
   F_kermit.put(0,1,0.671358648019);
   F_kermit.put(0,2,-0.829766848904);

   F_kermit.put(1,0,0.67040042255);
   F_kermit.put(1,1,0.00715254034214);
   F_kermit.put(1,2,-3.21051279534);

   F_kermit.put(2,0,-0.638327794729);
   F_kermit.put(2,1,2.15349779291);
   F_kermit.put(2,2,1);

   cout << "F_kermit = " << F_kermit << endl;
   fundamental_ptr->set_F_values(F_kermit);
   cout << "epipole_XY = " << fundamental_ptr->get_epipole_XY() << endl;
   cout << "epipole_UV = " << fundamental_ptr->get_epipole_UV() << endl;

   genmatrix* P_ptr=fundamental_ptr->compute_trivial_projection_matrix();
   genmatrix* Pprime_ptr=fundamental_ptr->
      compute_nontrivial_projection_matrix();

   camera_ptr->set_projection_matrix(*P_ptr);
   cameraprime_ptr->set_projection_matrix(*Pprime_ptr);

/*
   camera* cameraP_ptr=new camera();
   cameraP_ptr->set_projection_matrix(*P_ptr);
   cameraP_ptr->decompose_projection_matrix();
   cameraP_ptr->print_external_and_internal_params();
*/

/*
# Normalization factor rho = 1
# fu = -1 fv = -1
# u0 = 0 v0 = 0
# theta = 90 degs
# Camera world X = 0
# Camera world Y = 0
# Camera world Z = 0
# Camera azimuth = 90 degs
# Camera elevation = -90 degs
# Camera roll = 180 degs

# Uhat = -1 , -0 , -0
# Vhat = 0 , -1 , 0
# What = 0 , 0 , 1
*/

// Double-check that we recover *F_ptr from P and P' derived from
// *F_ptr:

   genmatrix* calculated_F_ptr=
      camerafunc::calculate_fundamental_matrix(P_ptr,Pprime_ptr);

   cout << "calculated F = " << *calculated_F_ptr << endl;
   cout << "F_kermit = " << F_kermit << endl;
   cout << "Difference = " << F_kermit - *calculated_F_ptr << endl;

//   fundamental_ptr->compute_from_projection_matrices(*P_ptr,*Pprime_ptr);


// Start with two 2D tiepoints which approximately satisfy the
// epipolar constraint curr_XY * F * curr_UV = 0:

   twovector curr_XY(0.3905,0.536333333333);
   twovector curr_UV(0.380375,0.473687);

   threevector X0=fundamental_ptr->triangulate_noisy_tiepoints(
      curr_UV,curr_XY);
   cout << "Intersection point for noisy tiepoints = "
        << X0 << endl;

// Solve for corrected versions of these tiepoints which more
// precisely satisfy the epipolar constraint:

   twovector corrected_XY,corrected_UV;
   fundamental_ptr->correct_tiepoint_coordinates(
      curr_XY,curr_UV,corrected_XY,corrected_UV);

   cout << "original UV = " << curr_UV << endl;
   cout << "corrected UV = " << corrected_UV << endl << endl;

   cout << "original XY = " << curr_XY << endl;
   cout << "corrected XY = " << corrected_XY << endl;

   threevector X=fundamental_ptr->triangulate(corrected_UV,corrected_XY);

   cout << "Back inside main(), Intersection point = " << X << endl;

// Recompute intersection point for corrected tiepoints using
// geometry_func::multi_line_intersection_point() method:

   threevector ray=camera_ptr->pixel_ray_direction(corrected_UV);
   threevector ray_prime=cameraprime_ptr->pixel_ray_direction(corrected_XY);

//   cout << "ray = " << ray << endl;
//   cout << "ray_prime = " << ray_prime << endl;
   
//   cout << "camera position = " << camera_ptr->get_world_posn() << endl;
//   cout << "camera prime position = " 
//        << cameraprime_ptr->get_world_posn() << endl;

   linesegment l(
      camera_ptr->get_world_posn(),camera_ptr->get_world_posn()+ray);
   linesegment lprime(
      cameraprime_ptr->get_world_posn(),
      cameraprime_ptr->get_world_posn()+ray_prime);

   vector<linesegment> lines;
   lines.push_back(l);
   lines.push_back(lprime);
   threevector intersection_point;
   geometry_func::multi_line_intersection_point(lines,intersection_point);
   cout << "intersection_point = " << intersection_point << endl;

   double l_dist=l.point_to_line_distance(intersection_point);
   double lprime_dist=lprime.point_to_line_distance(intersection_point);

   cout << "l_dist = " << l_dist << endl;
   cout << "lprime_dist = " << lprime_dist << endl;

}
