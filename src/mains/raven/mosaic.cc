// ==========================================================================
// Program MOSAIC reads in a set of input ground digital photos.
// It then uses SIFT feature matching, ANN candidate pair searching
// and RANSAC outlier determination to identify feature tiepoint
// pairs.  TIEPOINTS exports feature information to output feature
// text files which can be read in by programs VIDEO and PANORAMA.
// Only features which appear in at least 2 photos are written out.

/*

/home/cho/programs/c++/svn/projects/src/mains/raven/mosaic \
--newpass ./images/IMG_2950.JPG \
--newpass ./images/IMG_3000.JPG 

*/

// ==========================================================================
// Last updated on 11/4/11; 11/5/11; 12/3/12
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "structmotion/fundamental.h"
#include "geometry/homography.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "video/sift_detector.h"
#include "video/texture_rectangle.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ios;
using std::ofstream;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group);

   sift_detector SIFT(photogroup_ptr);

   string sift_keys_subdir="./";
   SIFT.extract_SIFT_features(sift_keys_subdir);
//   SIFT.print_features(5);

// SIFT feature matching becomes LESS stringent as sqrd_max_ratio
// increases.

// SIFT tiepoint inlier identification becomes LESS stringent as
// max_scalar_product increases.

   const int n_min_quadrant_features=1;		
//   const int n_min_quadrant_features=2;
//   const double sqrd_max_ratio=sqr(0.6);	
   const double sqrd_max_ratio=sqr(0.675);
//   const double sqrd_max_ratio=sqr(0.7);	
   const double worst_frac_to_reject=0.15;
   const double max_scalar_product=0.01;
//   const double max_scalar_product=0.03;
//   const double max_scalar_product=0.05;

   SIFT.identify_candidate_feature_matches_via_fundamental_matrix(
      n_min_quadrant_features,sqrd_max_ratio,worst_frac_to_reject,
      max_scalar_product);

//   SIFT.generate_features_map();

   fundamental* fundamental_ptr=SIFT.get_fundamental_ptr();
   genmatrix* F_ptr=fundamental_ptr->get_F_ptr();

   cout << "*fundamental_ptr = " << *fundamental_ptr << endl;
   cout << "fundamental rank = " << F_ptr->rank() << endl;

   int n_iters=3;
//   cout << "Enter n_iters:" << endl;
//   cin >> n_iters;

   for (int iter=0; iter<n_iters; iter++)
   {
      threevector e0_hat=fundamental_ptr->get_null_vector();
      fundamental_ptr->solve_for_fundamental(e0_hat);
   } // loop over iter index

   threevector e_XY=fundamental_ptr->get_epipole_XY();
   cout << "Epipole e_XY = " << e_XY << endl;
   threevector e_UV=fundamental_ptr->get_epipole_UV();
   cout << "Epipole e_UV = " << e_UV << endl;

   cout << "F * e_UV = " << *(fundamental_ptr->get_F_ptr()) * e_UV << endl;
   cout << "e_XY * F = " << e_XY * (*(fundamental_ptr->get_F_ptr()))
        << endl;

// Recall (X,Y)^T * F * (U,V) = 0 where (X,Y) comes from the 1st input
// image while (U,V) comes from the 2nd input image.  So epipole e_XY
// corresponds to 1st input image while e_UV e corresponds to the 2nd
// input image:

// Warp first XY image:

   photograph* photo_XY_ptr=photogroup_ptr->get_photograph_ptr(0);
   double Umax_XY=photo_XY_ptr->get_maxU();
   homography* H_XY_ptr=fundamental_ptr->map_epipole_to_infinity(Umax_XY,e_XY);
   cout << "Homography_XY = " << *H_XY_ptr << endl;

   string photo_XY_filename=photo_XY_ptr->get_filename();
   cout << "photo_XY_filename = " << photo_XY_filename << endl;
   texture_rectangle* texture_rectangle_XY_ptr=
      new texture_rectangle(photo_XY_filename,NULL);
   cout << "texture_rectangle_XY = " << *texture_rectangle_XY_ptr << endl;

   int width=texture_rectangle_XY_ptr->getWidth();
   int height=texture_rectangle_XY_ptr->getHeight();
   int n_images=1;
   int n_channels=texture_rectangle_XY_ptr->getNchannels();
   texture_rectangle* texture_rectangle_XY_warped_ptr=
      new texture_rectangle(width,height,n_images,n_channels,NULL);

   texture_rectangle_XY_warped_ptr->reset_UV_coords(0,Umax_XY,0,1);
   texture_rectangle_XY_warped_ptr->generate_blank_image_file(
      width,height,"blank.jpg",0);
   
   cout << "texture_rectangle_XY_warped = " 
        << *texture_rectangle_XY_warped_ptr << endl;

   int R,G,B;
   double u,v,x,y;
   for (int pu=0; pu<width; pu++)
   {
      for (int pv=0; pv<height; pv++)
      {
         texture_rectangle_XY_warped_ptr->get_uv_coords(pu,pv,u,v);
//         H_XY_ptr->project_world_plane_to_image_plane(u,v,x,y);
         H_XY_ptr->project_image_plane_to_world_plane(u,v,x,y);
         R=G=B=0;
         texture_rectangle_XY_ptr->get_RGB_values(x,y,R,G,B);
/*
         if (!nearly_equal(R,0) || !nearly_equal(G,0) || !nearly_equal(B,0))
         {
            cout << "pu = " << pu << " pv = " << pv << endl;
            cout << "u = " << u << " v = " << v
                 << " x = " << x << " y = " << y << endl;
            cout << "R = " << R << " G = " << G << " B = " << B << endl;
         }
*/

         texture_rectangle_XY_warped_ptr->set_pixel_RGB_values(pu,pv,R,G,B);
      } // loop over py index
   } // loop over px index

   string warped_XY_filename="warped_XY.jpg";
   texture_rectangle_XY_warped_ptr->write_curr_frame(warped_XY_filename);

// Now warp second UV image:

   photograph* photo_UV_ptr=photogroup_ptr->get_photograph_ptr(1);
   double Umax_UV=photo_UV_ptr->get_maxU();

   homography* H_UV_ptr=fundamental_ptr->compute_matching_UV_homography(
      H_XY_ptr);
   cout << "Homography_UV = " << *H_UV_ptr << endl;

   string photo_UV_filename=photo_UV_ptr->get_filename();
   cout << "photo_UV_filename = " << photo_UV_filename << endl;
   texture_rectangle* texture_rectangle_UV_ptr=
      new texture_rectangle(photo_UV_filename,NULL);
   cout << "texture_rectangle_UV = " << *texture_rectangle_UV_ptr << endl;

   width=texture_rectangle_UV_ptr->getWidth();
   height=texture_rectangle_UV_ptr->getHeight();
   n_images=1;
   n_channels=texture_rectangle_UV_ptr->getNchannels();
   texture_rectangle* texture_rectangle_UV_warped_ptr=
      new texture_rectangle(width,height,n_images,n_channels,NULL);

   texture_rectangle_UV_warped_ptr->reset_UV_coords(0,Umax_UV,0,1);
   texture_rectangle_UV_warped_ptr->generate_blank_image_file(
      width,height,"blank.jpg",0);
   
   cout << "texture_rectangle_UV_warped = " 
        << *texture_rectangle_UV_warped_ptr << endl;

   for (int pu=0; pu<width; pu++)
   {
      for (int pv=0; pv<height; pv++)
      {
         texture_rectangle_UV_warped_ptr->get_uv_coords(pu,pv,u,v);
//         H_UV_ptr->project_world_plane_to_image_plane(u,v,x,y);
         H_UV_ptr->project_image_plane_to_world_plane(u,v,x,y);
         R=G=B=0;
         texture_rectangle_UV_ptr->get_RGB_values(x,y,R,G,B);
/*
         if (!nearly_equal(R,0) || !nearly_equal(G,0) || !nearly_equal(B,0))
         {
            cout << "pu = " << pu << " pv = " << pv << endl;
            cout << "u = " << u << " v = " << v
                 << " x = " << x << " y = " << y << endl;
            cout << "R = " << R << " G = " << G << " B = " << B << endl;
         }
*/

         texture_rectangle_UV_warped_ptr->set_pixel_RGB_values(pu,pv,R,G,B);
      } // loop over py index
   } // loop over px index

   string warped_UV_filename="warped_UV.jpg";
   texture_rectangle_UV_warped_ptr->write_curr_frame(warped_UV_filename);
}

   
