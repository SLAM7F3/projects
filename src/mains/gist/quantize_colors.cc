// ==========================================================================
// Program QUANTIZE_COLORS imports a set of images and computes their
// color histograms.  Each image's histogram is exported to a text
// file with a .colorhist suffix.

//				./quantize_colors

// ==========================================================================
// Last updated on 4/7/13; 4/8/13
// ==========================================================================

#include  <iostream>
#include  <string>
#include  <vector>

#include "general/filefuncs.h"
#include "video/descriptorfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/RGB_analyzer.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"


using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(12);
   timefunc::initialize_timeofday_clock();

   RGB_analyzer* RGB_analyzer_ptr=new RGB_analyzer();
   RGB_analyzer_ptr->import_quantized_RGB_lookup_table();
   string lookup_map_name=RGB_analyzer_ptr->get_lookup_filename();
   cout << "lookup_map_name = " << lookup_map_name << endl;

   string outdoor_categories_subdir="./all_images/combined_image_links/";

   vector<string> input_image_subdirs;
   input_image_subdirs.push_back(outdoor_categories_subdir+"coast/");
   input_image_subdirs.push_back(outdoor_categories_subdir+"forest/");
   input_image_subdirs.push_back(outdoor_categories_subdir+"highway/");
   input_image_subdirs.push_back(outdoor_categories_subdir+"insidecity/");
   input_image_subdirs.push_back(outdoor_categories_subdir+"mountain/");
   input_image_subdirs.push_back(outdoor_categories_subdir+"opencountry/");
   input_image_subdirs.push_back(outdoor_categories_subdir+"street/");
   input_image_subdirs.push_back(outdoor_categories_subdir+"tallbldg/");
   input_image_subdirs.push_back(
      outdoor_categories_subdir+"negative_examples/");

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();

   for (int input_image_subdir_index=0; input_image_subdir_index < 
           input_image_subdirs.size(); input_image_subdir_index++)
   {
      outputfunc::print_elapsed_time();
      string curr_image_subdir=input_image_subdirs[input_image_subdir_index];
      
      vector<string> image_filenames=filefunc::image_files_in_subdir(
         curr_image_subdir);
      int n_images=image_filenames.size();
      cout << "n_images = " << n_images << endl;

      string colorhist_subdir=curr_image_subdir+"color_histograms/";
      filefunc::dircreate(colorhist_subdir);

      int imagenumber_start=0;
      int imagenumber_stop=n_images-1;

      for (int imagenumber=imagenumber_start; imagenumber<=imagenumber_stop; 
           imagenumber++)
      {
         outputfunc::print_elapsed_time();
         string image_filename=image_filenames[imagenumber];
         string image_basename=filefunc::getbasename(image_filename);
         
// Ignore any image whose basename contains white spaces or parentheses:

         string separator_chars=" ()\t\n";
         vector<string> substrings=
            stringfunc::decompose_string_into_substrings(
               image_basename,separator_chars);
         if (substrings.size() > 1) 
         {
//            cout << "Rejected basename = " << image_basename << endl;
//            outputfunc::enter_continue_char();
            continue;
         }

         cout << "Processing image " << imagenumber+1 << " of " 
              << n_images << endl;
         cout << "  Image basename = " << image_basename << endl;

         string image_filename_prefix=stringfunc::prefix(image_basename);

// First check whether color histogram for current image already
// exists.  If  so, move on...

         string color_histogram_filename=colorhist_subdir+
            image_filename_prefix+".colorhist";
         if (filefunc::fileexist(color_histogram_filename)) 
         {
            cout << "  Color histogram file already exists" << endl;
            continue;
         }

         descriptorfunc::compute_color_histogram(
            image_filename,color_histogram_filename,
            texture_rectangle_ptr,RGB_analyzer_ptr);

      } // loop over index i labeling image filenames
   } // loop over input_image_subdir_index

   delete RGB_analyzer_ptr;
   delete texture_rectangle_ptr;
}

