// ==========================================================================
// Program PHOTO_SIZES reads in a set of photo image filenames.  It
// calls ImageMagick's ping command to extract image width and height
// measured in pixels.  Image size results are saved into output text
// file bundler_IO_subdir/image_sizes.dat.

//  photo_sizes --region_filename ./packages/bundler/kermit/photo_sizes.pkg

// ==========================================================================
// Last updated 11/22/09; 12/7/09; 3/29/12; 12/30/13
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
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

   photogroup* bundler_photogroup_ptr=new photogroup();

   int n_photos_to_reconstruct=-1;
   bundler_photogroup_ptr->generate_bundler_photographs(
      bundler_IO_subdir,image_list_filename,n_photos_to_reconstruct);

   string image_sizes_filename=bundler_IO_subdir+"image_sizes.dat";
   bundler_photogroup_ptr->export_image_sizes(image_sizes_filename);

   string banner="Photo sizes in pixels written to output file "+
      image_sizes_filename;
   outputfunc::write_big_banner(banner);
}

