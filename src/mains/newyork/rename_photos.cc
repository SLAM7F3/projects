// ==========================================================================
// Program RENAME_PHOTOS is a quick-and-dirty utility we wrote to
// generate a script file which creates soft links between Noah's
// BUNDLER input photos to number photo filenames of the form
// photo_XXXX.jpg.  This program also writes file
// bundler_photo_order.dat.
// ==========================================================================
// Last updated on 4/20/09; 6/22/09
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "video/photograph.h"
#include "video/photogroup.h"
#include "math/rotation.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/threevector.h"

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

// Read in Noah's original set of photos and reconstructed cameras:

   string subdir=
      "/home/cho/programs/c++/svn/projects/src/mains/newyork/bundler/nyc_1000/Manhattan1012Compressed/";
   string image_list_filename=subdir+"Manhattan.1012.txt";

//   string subdir=
//      "/home/cho/programs/c++/svn/projects/src/mains/newyork/bundler/Manhattan/";
//   string compressed_image_list_filename=subdir+"list.compressed.txt";

// Generate STL map of compressed image filenames.  Also generate text file 
// bundler_photo_order.dat containing order and photoID integers: 

   string output_filename="bundler_photo_order.dat";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);
   outstream << "# Pass compositing order:" << endl << endl;
   outstream << "# Order, photo ID,  filename" << endl << endl;

   filefunc::ReadInfile(image_list_filename);
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);

      string curr_filename=filefunc::getbasename(substrings[0]);
      string suffix=stringfunc::suffix(curr_filename);

      string numbered_filename=
         "photo_"+stringfunc::integer_to_string(i,4)+"."+suffix;
//      cout << "i = " << i 
//           << " curr_filename = " << filefunc::getbasename(curr_filename) 
//           << " numbered_filename = " << numbered_filename << endl;      

      string unix_command="ln -s ../"+curr_filename+" "+numbered_filename;
      cout << unix_command << endl;

      outstream << i << "  " << i << "   #   " 
                << numbered_filename << endl;
   } // loop over index i 

   filefunc::closefile(output_filename,outstream);
}

