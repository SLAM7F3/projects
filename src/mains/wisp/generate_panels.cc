// ========================================================================
// Program GENERATE_PANELS reads in "bundler-style" image_list and
// image_sizes files for set of 360 deg panorama mosaics.  It chops up
// each input panorama into 10 individual panels which should be
// oriented 36 degrees apart in azimuth.  The individual panel images
// are written to a panels subdirectory of the images directory
// holding all of the input mosaics.  Each panel is labeled by the
// name of its parent panorama and a "pn" descriptor where n ranges
// from 0 to 9.

//				generate_panels

// ========================================================================
// Last updated on 1/19/11; 2/8/11; 5/29/11; 5/30/11
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "video/photogroup.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::string;
using std::vector;

// ==========================================================================
int main( int argc, char** argv )
{

   string bundler_IO_subdir=
      "/home/cho/programs/c++/svn/projects/src/mains/photosynth/bundler/wispnew/";
//      "/home/cho/programs/c++/svn/projects/src/mains/photosynth/bundler/wisp/";
   string image_list_filename=bundler_IO_subdir+"image_list.dat";
   string image_sizes_filename=bundler_IO_subdir+"image_sizes.dat";
   
   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->generate_bundler_photographs(
      bundler_IO_subdir,image_list_filename,image_sizes_filename);

   int n_photos(photogroup_ptr->get_n_photos());
   cout << "n_photos = " << n_photos << endl;

   int n_start=0;
   int n_panels=10;
//   n_photos=1;
   for (int n=n_start; n<n_photos; n++)
   {
      photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(n);
      string filename=photo_ptr->get_filename();

      int xdim=photo_ptr->get_xdim();
      int ydim=photo_ptr->get_ydim();
      
      cout << "n = " << n << " filename = " << filename 
//           << " xdim = " << xdim
//           << " ydim = " << ydim 
           << endl;

      string subdir=filefunc::getdirname(filename);
      string basename=filefunc::getbasename(filename);
      string filename_copy=subdir+"copy_"+basename;
      string panels_subdir=subdir+"panels/";
      filefunc::dircreate(panels_subdir);

//      cout << "filename_copy = " << filename_copy << endl;
//      cout << "panels_subdir = " << panels_subdir << endl;

      for (int p=0; p<n_panels; p++)
      {
         string unix_cmd="cp "+filename+" "+filename_copy;
         sysfunc::unix_command(unix_cmd);
         
         int width=double(xdim)/double(n_panels);
         int height=ydim;
         int xoffset=double(p*xdim)/double(n_panels);
         int yoffset=0;
         imagefunc::crop_image(filename_copy,width,height,xoffset,yoffset);
         
         string basename_prefix=stringfunc::prefix(basename);
         vector<string> substrings=
            stringfunc::decompose_string_into_substrings(
               basename_prefix,"_");
         string basename_suffix=stringfunc::suffix(basename);

         unix_cmd="mv "+filename_copy+" "+panels_subdir+
            substrings[0]+
            "_p"+stringfunc::number_to_string(p)+
            +"_"+substrings[1]+
            "."+basename_suffix;
//         cout << "unix_cmd = " << unix_cmd << endl;
         sysfunc::unix_command(unix_cmd);
      } // loop over index p labeling panels
   } // loop over index n labeling photos


}

