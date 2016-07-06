// ==========================================================================
// Program RENAME_THUMBNAILS
// ==========================================================================
// Last updated on 8/7/15
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "math/genmatrix.h"
#include "image/imagefuncs.h"
#include "image/myimage.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/rotation.h"
#include "video/texture_rectangle.h"
#include "video/videofuncs.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::ios;
   using std::ofstream;
   using std::ostream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);


// ==========================================================================

   string thumbnails_subdir = "./thumbnails/";
   vector<string> input_image_filenames = filefunc::image_files_in_subdir(
     thumbnails_subdir);

   for (unsigned int i = 0; i < input_image_filenames.size(); i++){
     string thumbnail_basename = filefunc::getbasename(input_image_filenames.at(i));
     string prefix=stringfunc::prefix(thumbnail_basename);

     
     int posn = stringfunc::first_substring_location(prefix,"_thumbnail");
//     cout << "prefix = " << prefix << " posn = " << posn << endl;
     string substr = prefix.substr(0, posn);
//     cout << "  substr = " << substr << endl;
     string new_icon_filename = substr+".jpg";

     string unix_cmd = "mv "+thumbnail_basename+" "+new_icon_filename;
     cout << unix_cmd << endl;
   }

}
