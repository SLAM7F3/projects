// ==========================================================================
// Program NORMALIZE_CHARS reads in a set of image files which contain
// single text characters from a directory tree.  It reassigns uniform
// names to the input files of the form "12345.jpg".  NORMALIZE_CHARS
// also resizes each output jpg file so that its new height precisely
// equals 32 pixels.

//			normalize_chars

// ==========================================================================
// Last updated on 5/31/12; 6/2/12
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
//   string subdir="./icdar03/";
   string subdir="./training_data/EnglishChars/";
   bool search_all_children_dirs_flag=true;

   vector<string> image_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,subdir,search_all_children_dirs_flag);
   vector<string> renorm_filenames;
   
   string output_subdir="./renorm_chars/";
   filefunc::dircreate(output_subdir);

   int i_offset=12469;
   
   for (unsigned int i=0; i<image_filenames.size(); i++)
   {
//      string suffix=stringfunc::suffix(image_filenames[i]);
      string new_imagefilename=stringfunc::integer_to_string(i+i_offset,5);
      new_imagefilename=output_subdir+new_imagefilename+".jpg";
//      new_imagefilename=output_subdir+new_imagefilename+"."+suffix;
      renorm_filenames.push_back(new_imagefilename);
    
      cout << "i = " << i 
           << " orig filename = " << image_filenames[i]
           << " new filename = " << new_imagefilename 
           << endl;

      string unix_cmd="cp "+image_filenames[i]+" "+new_imagefilename;
//      cout << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);

      unsigned int orig_xdim,orig_ydim;
      imagefunc::get_image_width_height(
         new_imagefilename,orig_xdim,orig_ydim);

      int new_ydim=32;
      int new_xdim=basic_math::round(
         double(orig_xdim)/double(orig_ydim)*new_ydim);

      videofunc::resize_image(
         new_imagefilename,orig_xdim,orig_ydim,
         new_xdim,new_ydim,new_imagefilename);

//      outputfunc::enter_continue_char();

   } // loop over index i labeling all char image files
  
   


}

