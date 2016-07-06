// ==========================================================================
// Program LABELED_IMAGE_MONTAGER imports names of images output by
// program GENERATE_MATCH_SCORES which were manually identified as
// matching and nonmatching.  For each pair, a montage image is
// generated and moved into subdirectories of labeled_images_subdir.
// The montage's filename along with an integer ID are saved text
// files within the matching and nonmatching image subdirectories.

//			   ./labeled_image_montager

// ==========================================================================
// Last updated on 10/15/13
// ==========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ios;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
 
   string ImageEngine_subdir="/data/ImageEngine/";
   string tidmarsh_subdir=ImageEngine_subdir+"tidmarsh/";
   string JAV_subdir="/data/video/JAV/NewsWraps/early_Sep_2013/";

   string root_subdir=JAV_subdir;
//   string root_subdir=tidmarsh_subdir;

   string images_subdir=JAV_subdir+"jpg_frames/";
//   string images_subdir=tidmarsh_subdir;
   
   string labeled_images_subdir=root_subdir+"labeled_images/";
   filefunc::dircreate(labeled_images_subdir);
   string labeled_matching_images_subdir=
      labeled_images_subdir+"matching_images/";
   string labeled_nonmatching_images_subdir=
      labeled_images_subdir+"nonmatching_images/";
   filefunc::dircreate(labeled_matching_images_subdir);
   filefunc::dircreate(labeled_nonmatching_images_subdir);

   string matching_images_filename="matching_files.txt";
   filefunc::ReadInfile(matching_images_filename);

   string matching_pairs_filename=labeled_matching_images_subdir+
      "matching_images.txt";
   ofstream outstream;
   filefunc::openfile(matching_pairs_filename,outstream);

   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      cout << i << " " << flush;
      vector<string> image_filenames=
         stringfunc::decompose_string_into_substrings(filefunc::text_line[i]);

      int image_pair_ID=stringfunc::string_to_number(image_filenames[0]);
      string image1_filename=images_subdir+image_filenames[1]+".jpg";
      string image2_filename=images_subdir+image_filenames[2]+".jpg";
      string unix_cmd="montageview "+image1_filename+" "+image2_filename+" n";
      sysfunc::unix_command(unix_cmd);

      string substring="montage_";
      vector<string> montage_filenames=
         filefunc::files_in_subdir_matching_substring("./",substring);
      string montage_jpg_filename=montage_filenames.back();

      unix_cmd="mv "+montage_jpg_filename+" "+labeled_matching_images_subdir;
      sysfunc::unix_command(unix_cmd);

      montage_jpg_filename=labeled_matching_images_subdir+
         filefunc::getbasename(montage_jpg_filename);
      outstream << image_pair_ID << "  " 
                << montage_jpg_filename << endl;
   }
   filefunc::closefile(matching_pairs_filename,outstream);
   cout << endl;
   string banner="Exported "+matching_pairs_filename;
   outputfunc::write_banner(banner);


   string nonmatching_images_filename="nonmatching_files.txt";
   filefunc::ReadInfile(nonmatching_images_filename);

   string nonmatching_pairs_filename=labeled_nonmatching_images_subdir+
      "nonmatching_images.txt";
   filefunc::openfile(nonmatching_pairs_filename,outstream);

   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      cout << i << " " << flush;
      vector<string> image_filenames=
         stringfunc::decompose_string_into_substrings(filefunc::text_line[i]);

      int image_pair_ID=stringfunc::string_to_number(image_filenames[0]);
      string image1_filename=images_subdir+image_filenames[1]+".jpg";
      string image2_filename=images_subdir+image_filenames[2]+".jpg";
      string unix_cmd="montageview "+image1_filename+" "+image2_filename+" n";
      sysfunc::unix_command(unix_cmd);

      string substring="montage_";
      vector<string> montage_filenames=
         filefunc::files_in_subdir_matching_substring("./",substring);
      string montage_jpg_filename=montage_filenames.back();

      unix_cmd="mv "+montage_jpg_filename+" "+
         labeled_nonmatching_images_subdir;
      sysfunc::unix_command(unix_cmd);

      montage_jpg_filename=labeled_nonmatching_images_subdir+
         filefunc::getbasename(montage_jpg_filename);
      outstream << image_pair_ID << "  " 
                << montage_jpg_filename << endl;
   }
   filefunc::closefile(nonmatching_pairs_filename,outstream);
   cout << endl;
   
   banner="Exported "+nonmatching_pairs_filename;
   outputfunc::write_banner(banner);


}

