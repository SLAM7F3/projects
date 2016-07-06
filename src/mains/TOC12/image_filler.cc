// ==========================================================================
// Program IMAGE_FILLER writes a stream of images to a hardwired
// output images subdir.  We wrote this program to simulate PointGrey
// imagery being dumped to some subdirectory from which our TANK_SIGNS
// and PG_RECOG programs should take their imagery input.
// ==========================================================================
// Last updated on 11/4/12; 11/5/12
// ==========================================================================

#include  <iostream>
#include  <string>
#include  <vector>

#include "color/colorfuncs.h"
#include "general/filefuncs.h"
#include "image/pngfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(12);
   string input_images_subdir="./images/final_signs/videos/raw_21_frames/";
//   string output_images_subdir="./images/incoming_PointGrey_images/";
   string output_images_subdir="~/Desktop/incoming_PointGrey_images/";

   string unix_cmd="/bin/rm -r -f "+output_images_subdir;
   sysfunc::unix_command(unix_cmd);
   filefunc::dircreate(output_images_subdir);

   vector<string> image_files=filefunc::image_files_in_subdir(
      input_images_subdir);
   cout << "image_files.size() = " << image_files.size() << endl;

   int i=0;
   while (i < image_files.size())
   {
      unix_cmd="cp "+image_files[i]+" "+output_images_subdir;
      cout << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);
      sleep(1);
//      sleep(4);
      i++;
      if (i >= image_files.size()) i==0;
   } // infinite while loop
   

}

