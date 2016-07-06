// ========================================================================
// Program THUMBNAILS queries the user to enter the full path for the
// subdirectory which holds all t>0 "time-slice" photos.  It then
// reads JPEG photos from that root directory along with all its
// children subdirectories.  THUMBNAILS generates subsampled versions
// of each JPG file and exports them to files of the form
// blah/blah/thumbnails/thumbnail_blahblah.JPG

//    /home/cho/programs/c++/svn/projects/src/mains/plume/thumbnails

// ========================================================================
// Last updated on 2/4/12; 5/17/12; 9/27/12
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
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

   string frames_subdir="./";
   cout << endl;
   cout << "Enter full path for subdirectory which holds all t>0 'time_slices' photos:" << endl << endl;
   cout << " (e.g. /data/ImageEngine/plume/Nov2011/Day2/B/images/time_slices/ ) "
        << endl << endl;
   cin >> frames_subdir;

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("jpg");
   allowed_suffixes.push_back("JPG");
   bool search_all_children_dirs_flag=true;

   vector<string> jpeg_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,frames_subdir,search_all_children_dirs_flag);

   for (int i=0; i<jpeg_filenames.size(); i++)
   {

// Ignore any existing thumbnails!

      int thumbnail_substring_posn=stringfunc::first_substring_location(
         jpeg_filenames[i],"thumbnail");
      if (thumbnail_substring_posn > 0) continue;

      string banner="Generating thumbnail for "+jpeg_filenames[i];
      outputfunc::write_banner(banner);

      photograph* photo_ptr=new photograph(jpeg_filenames[i]);

      unsigned int thumbnail_xdim,thumbnail_ydim;
      videofunc::get_thumbnail_dims(
         photo_ptr->get_xdim(),photo_ptr->get_ydim(),
         thumbnail_xdim,thumbnail_ydim);

      string thumbnail_filename=videofunc::generate_thumbnail(
         photo_ptr->get_filename(),photo_ptr->get_xdim(),photo_ptr->get_ydim(),
         thumbnail_xdim,thumbnail_ydim);
//      cout << "thumbnail_filename = " << thumbnail_filename << endl;

//      double zoom_factor=0.25;
//      videofunc::generate_thumbnail(
//         photo_ptr->get_filename(),zoom_factor);
      
      delete photo_ptr;
   } // loop over index i labeling input jpeg files

}
