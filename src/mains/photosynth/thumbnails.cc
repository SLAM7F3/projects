// ========================================================================
// Program THUMBNAILS generates subsampled versions of input photos
// within bundler_IO_subdir/images/thumbnails/.  This expensive
// operation should only be performed once!

//  thumbnails --region_filename ./packages/bundler/kermit/thumbnails.pkg

// ========================================================================
// Last updated on 1/29/10; 2/2/10; 5/19/10; 11/26/15
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);
   timefunc::initialize_timeofday_clock();

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);

   string image_list_filename=passes_group.get_image_list_filename();
   cout << " image_list_filename = " << image_list_filename << endl;
   string image_sizes_filename=passes_group.get_image_sizes_filename();
   cout << " image_sizes_filename = " << image_sizes_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;

   photogroup* bundler_photogroup_ptr=new photogroup();

   int n_photos_to_reconstruct=-1;
   bundler_photogroup_ptr->generate_bundler_photographs(
      bundler_IO_subdir,image_list_filename,image_sizes_filename,
      n_photos_to_reconstruct);

   bundler_photogroup_ptr->generate_thumbnails();
   cout << endl;

   outputfunc::print_elapsed_time();

   delete bundler_photogroup_ptr;
}
