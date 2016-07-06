// ========================================================================
// Program MICROTHUMBNAILS reads in a set of thumbnail images from
// bundler_IO_subdir/images/thumbnails . It generates
// microthumbnails for display within a SIMILE timeline and places
// them into bundler_IO_subdir/images/thumbnails/microthumbnails/ . 

/*

 microthumbnails --region_filename ./bundler/kermit/packages/peter_inputs.pkg

*/

// ========================================================================
// Last updated on 2/20/12; 2/22/12
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "passes/PassesGroup.h"
#include "video/photograph.h"
#include "general/sysfuncs.h"
#include "video/videofuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);

   string image_list_filename=passes_group.get_image_list_filename();
   cout << " image_list_filename = " << image_list_filename << endl;
   string image_sizes_filename=passes_group.get_image_sizes_filename();
   cout << " image_sizes_filename = " << image_sizes_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string thumbnails_subdir=bundler_IO_subdir+"images/thumbnails/";
   if (!filefunc::direxist(thumbnails_subdir))
   {
      cout << "Did not find thumbnails_subdir="+thumbnails_subdir << endl;
      cout << "First need to run program THUMBNAILS!" << endl;
      exit(-1);
   }

   string microthumbnails_subdir=thumbnails_subdir+"microthumbnails/";
   filefunc::dircreate(microthumbnails_subdir);

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("jpg");
   allowed_suffixes.push_back("JPG");
   bool search_all_children_dirs_flag=true;

   vector<string> jpeg_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,thumbnails_subdir,search_all_children_dirs_flag);

   for (unsigned int i=0; i<jpeg_filenames.size(); i++)
   {

// Only create micro thumbnails from existing thumbnails:

      int thumbnail_substring_posn=stringfunc::first_substring_location(
         jpeg_filenames[i],"thumbnail");
      if (thumbnail_substring_posn < 0) continue;

      string banner="Generating micro version of "+jpeg_filenames[i];
      outputfunc::write_banner(banner);

      photograph* photo_ptr=new photograph(jpeg_filenames[i]);

/*
      int thumbnail_xdim,thumbnail_ydim;
      videofunc::get_thumbnail_dims(
         photo_ptr->get_xdim(),photo_ptr->get_ydim(),
         thumbnail_xdim,thumbnail_ydim);
      videofunc::generate_thumbnail(
         photo_ptr->get_filename(),thumbnail_xdim,thumbnail_ydim);
*/
    
//      double zoom_factor=0.25;
      double zoom_factor=0.33;
//      double zoom_factor=0.5;
      string thumbnail_prefix="micro";
      videofunc::generate_thumbnail(
         photo_ptr->get_filename(),zoom_factor,thumbnail_prefix);
      
      delete photo_ptr;
   } // loop over index i labeling input jpeg files

}
