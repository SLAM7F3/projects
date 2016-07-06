// ==========================================================================
// Program FLIR_SELECT is a special utility which reads irregularly
// named files from Orange disk #3 corresponding to image frames from
// the Dec 2009 flight facility FLIR pass.  It generates a list.txt
// file containing reasonable input estimates for the focal parameter
// f*Npx.  FLIR_SELECT also generates an executable script which
// copies the selected image files into an ./images subdirectory for
// uploading to TXX.
// ==========================================================================
// Last updated on 1/24/11; 1/25/11
// ==========================================================================

#include <iomanip>
#include <map>
#include <string>
#include <vector>
#include <curl/curl.h>

#include "osg/osgGraphicals/AnimationController.h"
#include "video/camera.h"
#include "video/camerafuncs.h"
#include "general/filefuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

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
   string subdir="/media/10EB-267A/aerial_video/FLIR_flightfacility/cropped_images/";
   vector<string> image_filenames=filefunc::files_in_subdir(
      subdir);
   cout << "image_filenames.size() = " << image_filenames.size() << endl;


   int n_horiz_pixels=703;
   int n_vert_pixels=358;
   double aspect_ratio=double(n_horiz_pixels)/double(n_vert_pixels);
   double horiz_FOV=29.795*PI/180;

   double vert_FOV=camerafunc::vert_FOV_from_horiz_FOV_and_aspect_ratio(
      horiz_FOV,aspect_ratio);

   double f;
   camerafunc::f_and_aspect_ratio_from_horiz_vert_FOVs(
      horiz_FOV,vert_FOV,f,aspect_ratio);
   double f_in_pixels=fabs(f)*n_horiz_pixels;

   cout << "horiz_FOV = " << horiz_FOV*180/PI << endl;
   cout << "vert_FOV = " << vert_FOV*180/PI << endl;
   cout << "f = " << f << endl;
   cout << "aspect ratio = " << aspect_ratio << endl << endl;

   cout << "n_horiz_pixels = " << n_horiz_pixels << endl;
   cout << "n_vert_pixels = " << n_vert_pixels << endl;
   cout << "|f|*n_horiz_pixels = " << f_in_pixels << endl;

   string output_filename="FLIR_list.txt";
   string copy_filename="copy_images";
   ofstream outstream,copystream;
   filefunc::openfile(output_filename,outstream);
   filefunc::openfile(copy_filename,copystream);

   int i_skip=10;
//   int i_skip=20;
   for (unsigned int i=0; i<image_filenames.size(); i += i_skip)
   {
      string basename=filefunc::getbasename(image_filenames[i]);
      outstream << "./images/"+basename
                << " 0 "
                << f_in_pixels
                << endl;
      string copy_cmd="cp "+basename+" ./images/";
      copystream << copy_cmd << endl;
   }
   filefunc::closefile(output_filename,outstream);
   filefunc::closefile(copy_filename,copystream);

   string unix_cmd="chmod a+x "+copy_filename;
   sysfunc::unix_command(unix_cmd);

}
