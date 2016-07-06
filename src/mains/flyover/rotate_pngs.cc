// ==========================================================================
// Program ROTATE_PNGS
// ==========================================================================
// Last updated on 8/28/15
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

   string pngs_subdir = "/data/peter_stuff/imagery/Aug_vehicle_system/20150825_202459-T1/20150825_202459-T1/m131316-a05-20150825_202724-t1/05-1095A-08462/0k/pngs/housenumbers/";

   vector<string> input_image_filenames = filefunc::image_files_in_subdir(
     pngs_subdir);

   for (unsigned int i = 0; i < input_image_filenames.size(); i++){
     string png_basename = filefunc::getbasename(input_image_filenames.at(i));
     string prefix=stringfunc::prefix(png_basename);
     string rot_png_filename=prefix+"_rot.png";
//     cout << "png_basename = " << png_basename << endl;
     string unix_cmd = "convert -rotate 270 "+png_basename+" "+rot_png_filename;
     cout << unix_cmd << endl;
   }

}
