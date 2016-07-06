// ==========================================================================
// Program COMPRESS_GIST imports GIST descriptor files from a set of
// input subdirectories.  Within each input subdirectory, a binary
// hdf5 file is generated which contains all its scene GIST
// descriptors.

// 			./compress_gist

// ==========================================================================
// Last updated on 3/31/13; 4/6/13; 4/14/13
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <flann/flann.hpp>
#include <flann/io/hdf5.h>

#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(12);

   string outdoor_categories_subdir="./all_images/training_images/";

   vector<string> input_image_subdirs;
   input_image_subdirs.push_back(outdoor_categories_subdir+"coast/");
   input_image_subdirs.push_back(outdoor_categories_subdir+"forest/");
   input_image_subdirs.push_back(outdoor_categories_subdir+"highway/");
   input_image_subdirs.push_back(outdoor_categories_subdir+"insidecity/");
   input_image_subdirs.push_back(outdoor_categories_subdir+"mountain/");
   input_image_subdirs.push_back(outdoor_categories_subdir+"opencountry/");
   input_image_subdirs.push_back(outdoor_categories_subdir+"street/");
   input_image_subdirs.push_back(outdoor_categories_subdir+"tallbldg/");

   for (int input_image_subdir_index=0; input_image_subdir_index < 
           input_image_subdirs.size(); input_image_subdir_index++)
   {
      string curr_image_subdir=input_image_subdirs[input_image_subdir_index];
      string gist_subdir=curr_image_subdir+"gist_files/";

      vector<string> allowed_suffixes;
      allowed_suffixes.push_back("gist");

      vector<string> gist_filenames=
         filefunc::files_in_subdir_matching_specified_suffixes(
            allowed_suffixes,gist_subdir);
      int n_images=gist_filenames.size();
      cout << "n_images = " << n_images << endl;

      int K=3*512;
      flann::Matrix<float>* gist_descriptors_ptr=
         new flann::Matrix<float>(new float[n_images*K],n_images,K);

      for (int i=0; i<n_images; i++)
      {
         string gist_descriptor_filename=gist_filenames[i];
         string basename=filefunc::getbasename(gist_descriptor_filename);
         string prefix=stringfunc::prefix(basename);

         bool flag=filefunc::ReadInfile(gist_descriptor_filename);
         if (!flag) continue;
         int n_input_lines=filefunc::text_line.size();
         if (n_input_lines != 1) continue;

         vector<double> column_values=stringfunc::string_to_numbers(
            filefunc::text_line[0]);
         for (int k=0; k<column_values.size(); k++)
         {
            (*gist_descriptors_ptr)[i][k]=column_values[k];
         }
      } // loop over index i labeling gist filenames

      string gist_descriptors_hdf5_filename=gist_subdir+
         "scene_gist_descriptors.hdf5";
      filefunc::deletefile(gist_descriptors_hdf5_filename);

      flann::save_to_file(
         *gist_descriptors_ptr,gist_descriptors_hdf5_filename,
         "gist_descriptors");

      string banner="Wrote "+stringfunc::number_to_string(K)
         +" dimensional GIST descriptors for "
         +stringfunc::number_to_string(n_images)
         +" scene images to "+gist_descriptors_hdf5_filename;
      outputfunc::write_big_banner(banner);

      delete gist_descriptors_ptr;
   } // loop over input_image_subdir_index

}

