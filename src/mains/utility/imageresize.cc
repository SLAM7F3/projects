// ==========================================================================
// Program IMAGERESIZE reads in a set of JPEG or PNG files within the
// current subdirectory.  It generates a new set of image files whose
// pixel width and height are guaranteed to not exceed 2400.  The
// aspect ratio of each output image equals that of its input
// counterpart.

//				./image_resize

// ==========================================================================
// Last updated on 4/15/12; 6/4/12; 6/6/13
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::string;
using std::vector;

int main (int argc, char * argv[])
{
   std::set_new_handler(sysfunc::out_of_memory);

//   string input_subdir="/data_third_disk/BostonBombing/clip_0002/";
//   string input_subdir="/data_third_disk/BostonBombing/clips1-6/";
   string input_subdir="/data_third_disk/DIME/panoramas/Feb2013_DeerIsland/";
   input_subdir += "wisp8-spin-.5hz-ocean/panos_360/raw/";
//   string output_subdir=input_subdir+"half_size/";
   string output_subdir=input_subdir+"resize/";
   filefunc::dircreate(output_subdir);
   
   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("jpg");
   allowed_suffixes.push_back("JPG");
   allowed_suffixes.push_back("png");

   vector<string> input_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,input_subdir);

//   int new_max_width=2400;
//   int new_max_height=2400;

//   int new_max_width=960;
//   int new_max_height=540;

   int new_max_width=2000;
   int new_max_height=540;

   int iskip=10;
   for (unsigned int i=0; i<input_filenames.size(); i += iskip)
   {
      string input_filename=input_filenames[i];

      string dirname=filefunc::getdirname(input_filename);
      string basename=filefunc::getbasename(input_filename);
      string prefix=stringfunc::prefix(basename);
      string suffix=stringfunc::suffix(basename);
      string output_filename=output_subdir+prefix+"_scaled."+suffix;
      videofunc::downsize_image(
         input_filename,new_max_width,new_max_height,output_filename);
      string banner="Exported "+output_filename;
      outputfunc::write_banner(banner);
   }
   
} 

