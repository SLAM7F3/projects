// ========================================================================
// Program STDSIZE_IMAGES scans through images within
// bundler_IO_subdir/images.  It rescales and crops each image so that 
// the new version's pixel dimensions precisely equal the specified
// standard width and height.  The standardized images are exported to
// bundler_IO_subdir/standard_sized_images.

//  stdsize_images --region_filename ./bundler/kermit/packages/peter_inputs.pkg

// ========================================================================
// Last updated on 12/29/13
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

   int standard_xdim=640;
   int standard_ydim=480;

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);

   string image_list_filename=passes_group.get_image_list_filename();
   cout << " image_list_filename = " << image_list_filename << endl;
//   string image_sizes_filename=passes_group.get_image_sizes_filename();
//   cout << " image_sizes_filename = " << image_sizes_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string standard_sized_images_subdir=bundler_IO_subdir+
      "standard_sized_images/";
   filefunc::dircreate(standard_sized_images_subdir);

   photogroup* bundler_photogroup_ptr=new photogroup();
   bundler_photogroup_ptr->set_bundler_IO_subdir(bundler_IO_subdir);

   int n_photos_to_reconstruct=-1;
   bool parse_exif_metadata_flag=false;
   bool check_for_corrupted_images_flag=true;
   bundler_photogroup_ptr->generate_bundler_photographs(
      bundler_IO_subdir,image_list_filename,n_photos_to_reconstruct,
      parse_exif_metadata_flag,check_for_corrupted_images_flag);

   bundler_photogroup_ptr->standard_size_photos(
      standard_xdim,standard_ydim,standard_sized_images_subdir);

   delete bundler_photogroup_ptr;
}
