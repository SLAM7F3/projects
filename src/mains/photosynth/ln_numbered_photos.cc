// ==========================================================================
// Program LN_NUMBERED_PHOTOS reads in Noah's reconstructed image
// list.  For each input JPG filename, this program forms a simpler
// alias of the form "photo_XXXX.jpg".  It generates an executable
// script which establishes a softlink between the input JPG filenames
// and the simpler aliases.  The output executable should be run
// within a "numbered_photos/" subdirectory of an "images/" directory.

//  ln_numbered_photos --region_filename ./bundler/MIT2317/packages/peter_inputs.pkg

// ==========================================================================
// Last updated 1/20/10
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
   cout.precision(15);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);

   string image_list_filename=passes_group.get_image_list_filename();
   cout << "image_list_filename = " << image_list_filename << endl;
   filefunc::ReadInfile(image_list_filename);

   string output_filename="generate_numbered_photo_links";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);
   
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);
//      string curr_filename="../"+filefunc::getbasename(substrings[0]);
      string curr_filename="./"+filefunc::getbasename(substrings[0]);
      string numbered_filename="./IMG_"+stringfunc::integer_to_string(i+599,4)
         +".jpg";
//      string numbered_filename="./photo"+stringfunc::integer_to_string(i,4)
//         +".jpg";
      string curr_ln_cmd="ln -s "+curr_filename+"  "+numbered_filename;
      outstream << curr_ln_cmd << endl;
   }
   filefunc::closefile(output_filename,outstream);
   
   string unix_cmd="chmod a+x ./"+output_filename;
   sysfunc::unix_command(unix_cmd);
}

