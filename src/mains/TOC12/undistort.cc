// ==========================================================================
// Program UNDISTORT is a minor variant of Noah's RadialUndistort
// program.  It uses calibration parameters for the PointGrey video
// cameras which will be mounted on the airship and ground tank
// robots.  It outputs radially undistorted versions of input video
// frames.
// ==========================================================================
// Last updated on 10/25/12; 10/27/12; 10/29/12
// ==========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "video/camerafuncs.h"
#include "general/filefuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"

using std::cin;
using std::cout;
using std::endl;
using std::ios;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   string images_subdir="/data_second_disk/TOC12/images/PointGrey_calib/";
   vector<string> image_filenames=filefunc::image_files_in_subdir(
      images_subdir);

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();
   texture_rectangle* undistorted_texture_rectangle_ptr=
      new texture_rectangle();

   int Npu=1280;
   int Npv=960;

   double fu_pixels=833.97667;
   double fv_pixels=830.80215;
//    double aspect_ratio=double(Npu)/double(Npv);

//   double FOV_u,FOV_v;
//   camerafunc::horiz_vert_FOVs_from_f_and_aspect_ratio(
//      f,aspect_ratio,FOV_u,FOV_v);
//   cout << "FOV_u = " << FOV_u*180/PI
//        << " FOV_v = " << FOV_v*180/PI << endl;

   double cu_pixels=637.814;
   double cv_pixels=463.022;
   double k2=-0.33952;
   double k4=0.10150;

   for (unsigned int i=0; i<image_filenames.size(); i++)
   {
      string image_filename=image_filenames[i];
      cout << "i = " << i << " image = " << image_filename << endl;

      texture_rectangle_ptr->reset_texture_content(image_filename);
      undistorted_texture_rectangle_ptr->reset_texture_content(image_filename);

      camerafunc::radially_undistort_image(
         Npu,Npv,fu_pixels,fv_pixels,cu_pixels,cv_pixels,k2,k4,
         texture_rectangle_ptr,undistorted_texture_rectangle_ptr);

      string dirname=filefunc::getdirname(image_filename);
      string basename=filefunc::getbasename(image_filename);
      string undistorted_filename=dirname+"undistorted_"+basename;
      undistorted_texture_rectangle_ptr->write_curr_frame(
         undistorted_filename);

   } // loop over index i labeling image filenames

   delete texture_rectangle_ptr;
   delete undistorted_texture_rectangle_ptr;
}
