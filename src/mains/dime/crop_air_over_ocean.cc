// ========================================================================
// Program CROP_AIR_OVER_OCEAN reads in "bundler-style" image_list and
// image_sizes files for set of stabilized, UV-corrected WISP panos.
// It crops out the upper right-half region of each pano which
// corresponds to air over ocean.

//			./crop_air_over_ocean

// ========================================================================
// Last updated on 3/29/13
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
   string mains_subdir=
      "/home/cho/programs/c++/svn/projects/src/mains/";
   string bundler_IO_subdir=mains_subdir+
      "photosynth/bundler/DIME/Feb2013_DeerIsland/";
   string sky_subdir=bundler_IO_subdir+"images/sky_pics/";
   filefunc::dircreate(sky_subdir);
   
   string image_list_filename=bundler_IO_subdir+"image_list.dat";
   string image_sizes_filename=bundler_IO_subdir+"image_sizes.dat";
   
   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->generate_bundler_photographs(
      bundler_IO_subdir,image_list_filename,image_sizes_filename);

   int n_photos(photogroup_ptr->get_n_photos());
   cout << "n_photos = " << n_photos << endl;

   int n_start=0;
   cout << "Enter starting photo number:" << endl;
   cin >> n_start;

   int n_stop=n_photos-1;
   cout << "Enter stopping photo_number:" << endl;
   cin >> n_stop;

   for (int n=n_start; n<=n_stop; n++)
   {
      photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(n);
      string filename=photo_ptr->get_filename();

      int xdim=photo_ptr->get_xdim();
      int ydim=photo_ptr->get_ydim();

      cout << endl;
      cout << "n = " << n << " Chopping filename = " << filename 
           << " xdim = " << xdim
           << " ydim = " << ydim 
           << endl;

      int width=0.5*xdim;
      int height=1000;
      int xoffset=0.5*xdim;
      int yoffset=175;

      string basename=filefunc::getbasename(filename);
      string output_filename=sky_subdir+"cropped_"+basename;

      imagefunc::extract_subimage(
         filename,output_filename,width,height,xoffset,yoffset);

   } // loop over index n labeling photos


}

