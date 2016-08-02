// ====================================================================
// Program IMFDB_IMAGES extracts gender labels from text files which
// accompany images from the Indian Movie Face Data Base.  It then
// resizes, renames and homogenizes face image chips.  

// 			 ./imgdb_images
// ====================================================================
// Last updated on 8/2/16
// ====================================================================

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::exception;
using std::flush;
using std::ifstream;
using std::ios;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

int main(int argc, char** argv)
{  
   int max_xdim = 96;
   int max_ydim = 96;
   string imfdb_subdir = "/media/DataTransfer/faces/imfdb/";
   string IMFDB_subdir = imfdb_subdir+"/IMFDB_final/";
   string output_faces_subdir=imfdb_subdir+"imfdb_faces/";
   filefunc::dircreate(output_faces_subdir);

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("txt");

   bool search_all_children_dirs_flag=true;
   vector<string> text_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes, IMFDB_subdir, search_all_children_dirs_flag);

   int face_counter = 0;
   for(unsigned int t = 0; t < text_filenames.size(); t++)
   {
      cout << "Processing directory t = " << t 
           << " of " << text_filenames.size() << endl;
      vector<string> substrings = filefunc::ReadInStrings(text_filenames[t]);

      string gender = "unknown";
      for(unsigned int s = 0; s < substrings.size(); s++)
      {
         if(substrings[s] == "FEMALE")
         {
            gender = "female";
            break;
         }
         else if (substrings[s] == "MALE")
         {
            gender = "male";
            break;
         }
      } // loop over index s

      string images_subdir=filefunc::getdirname(text_filenames[t])+"images/";
      vector<string> image_filenames = filefunc::image_files_in_subdir(
         images_subdir);

      for(unsigned int i = 0; i < image_filenames.size(); i++)
      {
         string output_filename = output_faces_subdir+gender+"_"+
            stringfunc::integer_to_string(face_counter++,5)+".jpg";
         videofunc::downsize_image(
            image_filenames[i], max_xdim, max_ydim, output_filename);
      } // loop over index i labeling image filenames
   } // loop over index t labeling text filenames
}

