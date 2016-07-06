// ==========================================================================
// Program REMOVE_DANGLING_GIST_FILES scans through all gist files
// within the testing images subdirs.  For each gist file, it searches
// for a corresponding test image.  If no such test image exists, this
// program deletes the dangling gist file.  

// We wrote this program to clean out gist files for test images after
// they're repurposed as training images.


//		       ./remove_dangling_gist_files

// ==========================================================================
// Last updated on 4/14/13
// ==========================================================================

#include  <iostream>
#include  <map>
#include  <string>
#include  <vector>

#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::map;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(12);
   timefunc::initialize_timeofday_clock();

   string mains_gist_subdir=
      "/home/cho/programs/c++/svn/projects/src/mains/gist/";
   string all_images_subdir=mains_gist_subdir+"all_images/";
   string testing_images_subdir=all_images_subdir+"testing_images/";

   vector<string> input_image_subdirs;
   input_image_subdirs.push_back(testing_images_subdir+"coast/");
   input_image_subdirs.push_back(testing_images_subdir+"forest/");
   input_image_subdirs.push_back(testing_images_subdir+"highway/");
   input_image_subdirs.push_back(testing_images_subdir+"insidecity/");
   input_image_subdirs.push_back(testing_images_subdir+"mountain/");
   input_image_subdirs.push_back(testing_images_subdir+"opencountry/");
   input_image_subdirs.push_back(testing_images_subdir+"street/");
   input_image_subdirs.push_back(testing_images_subdir+"tallbldg/");

   for (unsigned int input_image_subdir_index=0; input_image_subdir_index < 
           input_image_subdirs.size(); input_image_subdir_index++)
   {
      outputfunc::print_elapsed_time();
      string curr_image_subdir=input_image_subdirs[input_image_subdir_index];
      string gist_subdir=curr_image_subdir+"gist_files/";

// Save image basename prefixes into STL map:

      typedef map<string,int> IMAGE_BASENAME_PREFIX_MAP;
      IMAGE_BASENAME_PREFIX_MAP image_basename_prefix_map;
      IMAGE_BASENAME_PREFIX_MAP::iterator iter;

      vector<string> image_filenames=filefunc::image_files_in_subdir(
         curr_image_subdir);
      for (unsigned int i=0; i<image_filenames.size(); i++)
      {
         string curr_image_basename=filefunc::getbasename(image_filenames[i]);
         string curr_image_basename_prefix=
            stringfunc::prefix(curr_image_basename);
         image_basename_prefix_map[curr_image_basename_prefix]=1;
      } // loop over index i labeling image filenames

      vector<string> gist_filenames=
         filefunc::files_in_subdir(gist_subdir);
      int n_gist_files=gist_filenames.size();

      vector<string> gist_files_to_delete;
      for (int g=0; g<n_gist_files; g++)
      {
//         outputfunc::print_elapsed_time();
         string input_filename=gist_filenames[g];
         string gist_basename=filefunc::getbasename(input_filename);
         string input_basename_prefix=stringfunc::prefix(gist_basename);

// Check whether image file prefix corresponding to current gist file
// exists within image_basename_prefix_map. If not, delete current
// gist file:

         iter=image_basename_prefix_map.find(input_basename_prefix);
         if (iter==image_basename_prefix_map.end())
         {
            gist_files_to_delete.push_back(gist_filenames[g]);
         }
      } // loop over index g labeling gist files

      cout << "gist_files_to_delete.size() = "
           << gist_files_to_delete.size() << endl;

      for (unsigned int i=0; i<gist_files_to_delete.size(); i++)
      {
         string unix_cmd="/bin/rm "+gist_files_to_delete[i];
//         cout << unix_cmd << endl;
         sysfunc::unix_command(unix_cmd);
      }
      


   } // loop over input_image_subdir_index
}

