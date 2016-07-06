// ==========================================================================
// Program REORDER_BBOX_IMAGES creates a copy of all consolidated
// image masks with ground truth bboxes.  It assigns new indices to
// the members of the copy based upon their pixel widths.  We wrote
// this utility in order to simply viewing of semantic segmentation
// inference results using our 2D VIEWIMAGES viewer.
// ==========================================================================
// Last updated on 5/18/16; 5/20/16; 5/22/16; 6/25/16
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "math/basic_math.h"
#include "geometry/bounding_box.h"
#include "color/colorfuncs.h"
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
int main (int argc, char * argv[])
{
   std::set_new_handler(sysfunc::out_of_memory);

   string banner="Starting REORDER_BBOX_IMAGES program";
   outputfunc::write_big_banner(banner);

   if(argc != 2)
   {
      cout << "Must pass input_images_subdir (e.g. '/home/pcho/programs/c++/svn/projects/src/mains/machine_learning/deeplab/images/faces/testing_images_05/') as command-line argument" << endl;
      exit(-1);
   }
   string input_images_subdir(argv[1]);
   filefunc::add_trailing_dir_slash(input_images_subdir);
   cout << "input_images_subdir = " << input_images_subdir << endl;

//   string consolidated_subdir = input_images_subdir+"consolidated/";
   string consolidated_subdir = input_images_subdir+"ccs/consolidated_images/";
   string bboxes_subdir=consolidated_subdir+"bboxes/";
   string reordered_bboxes_subdir = bboxes_subdir+"reordered/";
   filefunc::dircreate(reordered_bboxes_subdir);

   vector<string> bboxes_image_filenames=
      filefunc::image_files_in_subdir(bboxes_subdir);
   unsigned int n_bboxes_images = bboxes_image_filenames.size();

   vector<double> image_widths;
   for(unsigned int i = 0; i < n_bboxes_images; i++)
   {
      unsigned int width, height;
      imagefunc::get_image_width_height(
         bboxes_image_filenames[i],width, height);
      image_widths.push_back(width);

//      cout << i << " width = " << width << " image = "
//           << bboxes_image_filenames[i] << endl;
   } // loop over index i 

   templatefunc::Quicksort(image_widths, bboxes_image_filenames);

   for(unsigned int i = 0; i < n_bboxes_images; i++)
   {
      string basename=filefunc::getbasename(bboxes_image_filenames[i]);
      string image_ID_str = basename.substr(7,5);
      string reordered_bboxes_image_filename=
         reordered_bboxes_subdir+"segmented_image_"+
         stringfunc::integer_to_string(i,5)+"_"+image_ID_str+".png";
      string unix_cmd="cp "+bboxes_image_filenames[i]+" "
         +reordered_bboxes_image_filename;
      sysfunc::unix_command(unix_cmd);
   }

   banner="Exported image masks with ground truth bboxes ordered by their pixel widths to ";
   banner += reordered_bboxes_subdir;
   outputfunc::write_banner(banner);
}

