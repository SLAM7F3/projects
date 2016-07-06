// ==========================================================================
// Program NORMALIZE_WORDS reads in a set of JPG files from
// words_subdir which contain multi-character words from a directory
// tree.  It reassigns uniform names to the input files of the form
// "12345.jpg".  NORMALIZE_WORDS also resizes each input jpg file so
// that its new height precisely equals 32 pixels.
// ==========================================================================
// Last updated on 6/1/12; 6/2/12; 6/5/12
// ==========================================================================

#include  <iostream>
#include  <string>
#include  <vector>

#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(12);

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("jpg");
   allowed_suffixes.push_back("png");
//   string words_subdir="./training_data/icdar11/digital_words/";
   string words_subdir="./training_data/icdar11/test_digital_words/";
//   string words_subdir="./training_data/test-wordrec/";
//   string words_subdir="./training_data/train-wordrec/";
//   string words_subdir="./training_data/text_database/parsed_words/";
   bool search_all_children_dirs_flag=false;
//   bool search_all_children_dirs_flag=true;

   vector<string> image_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,words_subdir,search_all_children_dirs_flag);
   vector<string> renorm_filenames;
   
   string output_subdir="./renorm_words/";
   filefunc::dircreate(output_subdir);

   int i_offset;
   cout << "Enter i_offset:" << endl;
   cin >> i_offset;
   for (unsigned int i=0; i<image_filenames.size(); i++)
   {
      string new_jpegfilename=stringfunc::integer_to_string(i+i_offset,5);
      new_jpegfilename=output_subdir+new_jpegfilename+".jpg";
      renorm_filenames.push_back(new_jpegfilename);
    
      cout << "i = " << i 
           << " image_filename = " << image_filenames[i]
           << " new filename = " << new_jpegfilename 
           << endl;

      string unix_cmd="cp "+image_filenames[i]+" "+new_jpegfilename;
//      cout << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);

      unsigned int orig_xdim,orig_ydim;
      imagefunc::get_image_width_height(
         new_jpegfilename,orig_xdim,orig_ydim);

      int new_ydim=32;
      int new_xdim=basic_math::round(
         double(orig_xdim)/double(orig_ydim)*new_ydim);

      videofunc::resize_image(
         new_jpegfilename,orig_xdim,orig_ydim,
         new_xdim,new_ydim,new_jpegfilename);

//      outputfunc::enter_continue_char();

   } // loop over index i labeling all char image files
  
   


}

