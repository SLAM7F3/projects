// ==========================================================================
// Program RECTIFY implements the rectification algorithm presented by
// A. Fusiello, E. Trucco and A. Verri in "A compact algorithm for
// rectification of stereo pairs".  For each image pair labeled
// by indices i and j, RECTIFY computes homographies Hi and Hj
// which warp the input images.  Corresponding pixels in the warped
// images have the same height values.  The warped image pairs are
// exported to jpg files.

/*

./rectify --region_filename ./bundler/kermit/packages/peter_inputs.pkg \
--region_filename ./bundler/kermit/packages/photo_0000.pkg \
--region_filename ./bundler/kermit/packages/photo_0002.pkg 

*/

// ==========================================================================
// Last updated on 1/27/13; 2/3/13
// ==========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "video/camerafuncs.h"
#include "general/filefuncs.h"
#include "geometry/homography.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "math/rotation.h"          
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "math/threevector.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
   cout.precision(15);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);
   
   string image_list_filename=passes_group.get_image_list_filename();
   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;

   string image_sizes_filename=passes_group.get_image_sizes_filename();
   cout << "image_sizes_filename = " << image_sizes_filename << endl;
   string bundle_filename=passes_group.get_bundle_filename();
   cout << "bundle_filename = " << bundle_filename << endl;

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group,image_sizes_filename);
   int n_photos=photogroup_ptr->get_n_photos();
   cout << "n_photos = " << n_photos << endl;

   photogroup_ptr->set_bundler_IO_subdir(bundler_IO_subdir);

   vector<camera*> camera_ptrs;
   for (int n=0; n<n_photos; n++)
   {
      photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(n);
      camera* camera_ptr=photo_ptr->get_camera_ptr();
      camera_ptrs.push_back(camera_ptr);
//      cout << "n = " << n
//           << " camera = " << *camera_ptr 
//           << " camera posn = " << camera_ptr->get_world_posn()
//           << endl;
   }


   homography Hi,Hj;

   string blank_filename="blank.jpg";
   for (int i=0; i<n_photos; i++)
   {
      photograph* photo_i_ptr=photogroup_ptr->get_photograph_ptr(i);
      string image_i_filename=photo_i_ptr->get_filename();
      texture_rectangle* texture_rectangle_i_ptr=new texture_rectangle(
         image_i_filename,NULL);
      int width_i=texture_rectangle_i_ptr->getWidth();
      int height_i=texture_rectangle_i_ptr->getHeight();
      double umax_i=double(width_i)/double(height_i);
      
      camera* camera_i_ptr=camera_ptrs[i];
      genmatrix* Ki_ptr=camera_i_ptr->get_K_ptr();
      genmatrix* Ki_inv_ptr=camera_i_ptr->get_Kinv_ptr();
      rotation* Ri_ptr=camera_i_ptr->get_Rcamera_ptr();
      threevector what_i=camera_i_ptr->get_What();
      threevector C_i=camera_i_ptr->get_world_posn();

      for (int j=i+1; j<n_photos; j++)
      {
         photograph* photo_j_ptr=photogroup_ptr->get_photograph_ptr(j);
         string image_j_filename=photo_j_ptr->get_filename();
         texture_rectangle* texture_rectangle_j_ptr=new texture_rectangle(
            image_j_filename,NULL);

         int width_j=texture_rectangle_j_ptr->getWidth();
         int height_j=texture_rectangle_j_ptr->getHeight();
         double umax_j=double(width_j)/double(height_j);

         camera* camera_j_ptr=camera_ptrs[j];
         genmatrix* Kj_ptr=camera_j_ptr->get_K_ptr();
         genmatrix* Kj_inv_ptr=camera_j_ptr->get_Kinv_ptr();
         rotation* Rj_ptr=camera_j_ptr->get_Rcamera_ptr();         
         threevector C_j=camera_j_ptr->get_world_posn();

         threevector uhat_rect=(C_j-C_i).unitvector();
         threevector vhat_rect=what_i.cross(uhat_rect);
         vhat_rect=vhat_rect.unitvector();
         threevector what_rect=uhat_rect.cross(vhat_rect);

         genmatrix K_rect(3,3);
         K_rect=0.5*(*Ki_ptr + *Kj_ptr);

         rotation R_rect;
         R_rect.put_row(0,uhat_rect);
         R_rect.put_row(1,vhat_rect);
         R_rect.put_row(2,what_rect);
         
         genmatrix Hi_mat(3,3),Hj_mat(3,3);
         Hi_mat=K_rect * R_rect * Ri_ptr->transpose() * (*Ki_inv_ptr);
         Hj_mat=K_rect * R_rect * Rj_ptr->transpose() * (*Kj_inv_ptr);

         cout << "Hi = " << Hi_mat << endl;
         cout << "Hj = " << Hj_mat << endl;

         Hi.set_H(Hi_mat);
         Hj.set_H(Hj_mat);

// Search for pixel bbox in warped image plane corresponding to bbox in
// image_i:

         int pu_i_min=width_i;
         int pu_i_max=0;
         int pv_i_min=height_i;
         int pv_i_max=0;

         double x,y;         
         for (int pu=-3*width_i; pu<3*width_i; pu += 5)
         {
            double u=double(pu)/(height_i-1);
            for (int pv=-3*height_i; pv<3*height_i; pv += 5)
            {
               double v=1-double(pv)/(height_i-1);
               Hi.project_image_plane_to_world_plane(u,v,x,y);
               if (x < 0 || x > umax_j || y < 0 || y > 1) continue;

               pu_i_min=basic_math::min(pu_i_min,pu);
               pu_i_max=basic_math::max(pu_i_max,pu);
               pv_i_min=basic_math::min(pv_i_min,pv);
               pv_i_max=basic_math::max(pv_i_max,pv);
            }
         }
         cout << "pu_i_min = " << pu_i_min 
              << " pu_i_max = " << pu_i_max << endl;
         cout << "pv_i_min = " << pv_i_min 
              << " pv_i_max = " << pv_i_max << endl;

         int warped_width_i=pu_i_max-pu_i_min;
         int warped_height_i=pv_i_max-pv_i_min;

         cout << "warped_width_i = " << warped_width_i << endl;
         cout << "warped_height_i = " << warped_height_i << endl;

// Search for pixel bbox in warped image plane corresponding to bbox
// in image_j:

         int pu_j_min=width_j;
         int pu_j_max=0;
         int pv_j_min=height_j;
         int pv_j_max=0;
         
         for (int pu=-3*width_j; pu<3*width_j; pu += 5)
         {
            double u=double(pu)/(height_j-1);
            for (int pv=-3*height_j; pv<3*height_j; pv += 5)
            {
               double v=1-double(pv)/(height_j-1);
               Hj.project_image_plane_to_world_plane(u,v,x,y);
               if (x < 0 || x > umax_j || y < 0 || y > 1) continue;

               pu_j_min=basic_math::min(pu_j_min,pu);
               pu_j_max=basic_math::max(pu_j_max,pu);
               pv_j_min=basic_math::min(pv_j_min,pv);
               pv_j_max=basic_math::max(pv_j_max,pv);
            }
         }
         cout << "pu_j_min = " << pu_j_min 
              << " pu_j_max = " << pu_j_max << endl;
         cout << "pv_j_min = " << pv_j_min 
              << " pv_j_max = " << pv_j_max << endl;

         int warped_width_j=pu_j_max-pu_j_min;
         int warped_height_j=pv_j_max-pv_j_min;

         cout << "warped_width_j = " << warped_width_j << endl;
         cout << "warped_height_j = " << warped_height_j << endl;

// Instantiate warped texture rectangles for images i and j which have
// the same pixel height but generally different pixel widths:

         int warped_width=basic_math::max(warped_width_i,warped_width_j);
         
         int pv_min=basic_math::min(pv_i_min,pv_j_min);
         int pv_max=basic_math::max(pv_i_max,pv_j_max);
         int warped_height=pv_max-pv_min;

// Export rectified versions of images i and j;

         string warped_ij_filename="rectified_"
            +stringfunc::number_to_string(i)+"_"
            +stringfunc::number_to_string(j)+".jpg";
         camerafunc::rectify_image(
            texture_rectangle_i_ptr,warped_width,warped_height,
            pu_i_min,pu_i_max,pv_min,pv_max,
            Hi,warped_ij_filename);

         string warped_ji_filename="rectified_"
            +stringfunc::number_to_string(j)+"_"
            +stringfunc::number_to_string(i)+".jpg";
         camerafunc::rectify_image(
            texture_rectangle_j_ptr,warped_width,warped_height,
            pu_j_min,pu_j_max,pv_min,pv_max,
            Hj,warped_ji_filename);

         delete texture_rectangle_j_ptr;
      } // loop over index j labeling photos

      delete texture_rectangle_i_ptr;
      
   } // loop over index i labeling photos

}



