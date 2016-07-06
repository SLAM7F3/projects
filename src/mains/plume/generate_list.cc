// ==========================================================================
// Program GENERATELIST is a little utility which we wrote in order to
// hard code an initial guess for a video camera's focal length (measured in
// pixels) into a list.txt file read by Noah's Bundler codes.  

// Bundler needs some estimate for the focal parameter.  And for video
// frames, focal information is contained within input frames' exif
// metadata tags.  So Noah told us in Dec 2010 that we need to
// recreate the list.txt file with focal parameter information
// included.  We then need to set the IMAGE_LIST environment variable
// to point towards list.txt.  In bash, chant	 

// 			export IMAGE_LIST=list.txt

// Then run Noah's BundlerVocab script on LLGrid.

//				generate_list

// ==========================================================================
// Last updated on 8/13/12; 8/17/12; 12/3/12
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "video/camerafuncs.h"
#include "general/filefuncs.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ios;
using std::ofstream;
using std::string;
using std::vector;


// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   string list_filename="list.txt";
   ofstream outstream;
   filefunc::openfile(list_filename,outstream);

//   string subdir="./video_images/renumbered/";
//   string subdir="~/medford/";
//   string subdir=
//      "/data_third_disk/plume/Nov_2012/video/Rover_Day_2_VideoImages/2500-4500/";
   string subdir=
      "/data_third_disk/plume/Nov_2012/video/Rover_Day_2_VideoImages/2000-9000/";

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("jpg");
   vector<string> jpg_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,subdir);

   int dn=1;
   double FOV_u=80.966*PI/180;
   double FOV_v=51.294*PI/180;
   int n_horiz_pixels=1920;
   int n_vert_pixels=1080;

   double aspect_ratio=double(n_horiz_pixels)/double(n_vert_pixels);
   cout << "n_horiz/n_vert = " << aspect_ratio << endl;

   double f;
   camerafunc::f_and_aspect_ratio_from_horiz_vert_FOVs(
      FOV_u,FOV_v,f,aspect_ratio);
   cout << "f = " << f << " calculated aspect ratio = " << aspect_ratio
        << endl;

   double f_n_horiz_pixels=fabs(f)*n_horiz_pixels;
   double f_n_vert_pixels=fabs(f)*n_vert_pixels;
   cout << "f*n_horiz_pixels = " << f_n_horiz_pixels << endl;
   cout << "f*n_vert_pixels = " << f_n_vert_pixels << endl;

   outputfunc::enter_continue_char();

// Critical note:  f_noah in bundler = f_n_vert_pixels !

   for (int n=0; n<jpg_filenames.size(); n += dn)
   {
      string curr_filename=filefunc::getbasename(jpg_filenames[n]);
      curr_filename="images/"+curr_filename;
      outstream << curr_filename 
                << " 0 "
                << f_n_vert_pixels
                << endl;
   }


   filefunc::closefile(list_filename,outstream);


   string banner="Wrote images and estimated value for f*Npy to "+
      list_filename;
   outputfunc::write_banner(banner);
}
