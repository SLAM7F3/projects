// ==========================================================================
// Program 8BIT_TO_24BIT asks the user to specify the full path to
// some subdirectory which sits atop a "tree" of directories filled
// with 8-bit binary plume masks.  It searches the entire tree for all
// jpg files which are assumed to correspond to 8-bit binary masks.
// 8BIT_to_24BIT overwrites the 8-bit binary masks with their 24-bit
// analogs.  Note that the blue channel is slightly randomized in
// order to force the output JPEG masks to have 24 rather than 8 bits.
// ==========================================================================
// Last updated on 9/18/13
// ==========================================================================

#include  <iostream>
#include  <string>
#include  <vector>

#include "general/filefuncs.h"
#include "general/stringfuncs.h"
#include "video/texture_rectangle.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(12);

   string eight_bit_masks_subdir="./time_slices/";
   cout << "Enter full path for 'time_slices' subdirectory holding tree of 8-bit mask jpg files:" << endl;
   cin >> eight_bit_masks_subdir;
//   string RGB_subdir=subdir+"RGB_masks/";
//   filefunc::dircreate(RGB_subdir);
   
   bool search_all_children_dirs_flags=true;
   vector<string> input_filenames=filefunc::image_files_in_subdir(
      eight_bit_masks_subdir,search_all_children_dirs_flags);

   for (int i=0; i<input_filenames.size(); i++)
   {
      string mask_filename=filefunc::getbasename(input_filenames[i]);
      cout << "i = " << i << " mask_filename = " 
           << input_filenames[i] << endl;

      texture_rectangle* texture_rectangle_ptr=new texture_rectangle(
         input_filenames[i],NULL);
      texture_rectangle* RGB_texture_rectangle_ptr=texture_rectangle_ptr->
         generate_RGB_from_grey_texture_rectangle();

      string RGB_mask_filename=input_filenames[i];
//      string RGB_mask_filename=RGB_subdir+mask_filename;
      RGB_texture_rectangle_ptr->write_curr_frame(RGB_mask_filename);

      delete RGB_texture_rectangle_ptr;
      delete texture_rectangle_ptr;

   } // loop over index i labeling input binary compressed files

}

