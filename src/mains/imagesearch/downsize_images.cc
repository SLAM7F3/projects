// ========================================================================
// Program DOWNSIZE_IMAGES scans through images within
// bundler_IO_subdir/images.  It downsamples any which are larger than
// max_xdim,max_ydim in pixel size.  Oversized original images are
// moved into a separate subdirectory, and their place within the images
// subdir is taken by their downsized counterparts.  Original or
// downsampled image size results are saved into output text file
// bundler_IO_subdir/image_sizes.dat.

// This expensive operation should only be performed once!

//  downsize_images --region_filename ./bundler/kermit/packages/peter_inputs.pkg

// ========================================================================
// Last updated on 3/23/12; 5/7/12
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "general/sysfuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

//   const int max_xdim=1200;
//   const int max_ydim=1200;
   const int max_xdim=2400;
   const int max_ydim=2400;

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);

   string image_list_filename=passes_group.get_image_list_filename();
   cout << " image_list_filename = " << image_list_filename << endl;
//   string image_sizes_filename=passes_group.get_image_sizes_filename();
//   cout << " image_sizes_filename = " << image_sizes_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;

   photogroup* bundler_photogroup_ptr=new photogroup();
   bundler_photogroup_ptr->set_bundler_IO_subdir(bundler_IO_subdir);

   int n_photos_to_reconstruct=-1;
   bool parse_exif_metadata_flag=false;
   bool check_for_corrupted_images_flag=true;
   bundler_photogroup_ptr->generate_bundler_photographs(
      bundler_IO_subdir,image_list_filename,n_photos_to_reconstruct,
      parse_exif_metadata_flag,check_for_corrupted_images_flag);

   bundler_photogroup_ptr->downsize_photos(max_xdim,max_ydim);

   string image_sizes_filename=bundler_IO_subdir+"image_sizes.dat";
   bundler_photogroup_ptr->export_image_sizes(image_sizes_filename);

   delete bundler_photogroup_ptr;

   string banner="Photo sizes in pixels written to output file "+
      image_sizes_filename;
   outputfunc::write_big_banner(banner);

}
