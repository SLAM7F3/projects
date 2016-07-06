// ==========================================================================
// Program RECOLOR_DEEPLAB_MASKS
// ==========================================================================
// Last updated on 9/22/15; 9/25/15; 9/28/15; 9/29/15
// ==========================================================================

#include <iostream>
#include <math.h>
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

// ----------------------------------------------------------------
void print_class_pixel_info(vector<long long int>& class_pixel_freqs)
{
   long long int class_pixel_freq_sum = 0;
   for(unsigned int i = 0; i < class_pixel_freqs.size(); i++)
   {
     class_pixel_freq_sum += class_pixel_freqs[i];
   }
   for(unsigned int i = 0; i < class_pixel_freqs.size(); i++)
   {
      double n_pixels = class_pixel_freqs[i];
      double class_pixel_freq_frac = n_pixels/class_pixel_freq_sum;
      cout << "Class i = " << i 
           << " pixel frequency fraction = " << class_pixel_freq_frac
           << std::scientific << " n_pixels = " << n_pixels
//           << " n_pixels = " << class_pixel_freqs[i]
           << endl;
   }
}

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
   cout.precision(5);

   timefunc::initialize_timeofday_clock();

   /*
   //   string maskdir = "/home/pcho/programs/c++/svn/projects/src/mains/photosynth/bundler/faces/images/deeplab_inputs/eightbit_masks/";
   string bundler_dir = "/home/pcho/programs/c++/svn/projects/src/mains/photosynth/bundler/";
   cout << "bundler_dir = " << bundler_dir << endl;
   string maskdir = bundler_dir+"faces/images/deeplab_inputs/eightbit_masks/";
   cout << "maskdir = " << maskdir << endl;
   */
   string maskdir = "/home/pcho/programs/c++/svn/projects/src/mains/machine_learning/deeplab/eightbit_masks/";


   int n_max_classes = 5;
   vector<long long int> class_pixel_freqs;
   for(int i = 0; i < n_max_classes; i++)
   {
      class_pixel_freqs.push_back(0);
   }

   int width, height;
   texture_rectangle* texture_rectangle_ptr=NULL;

   vector<string> mask_filenames = filefunc::image_files_in_subdir(maskdir);
   cout << "n_masks = " << mask_filenames.size() << endl;

   outputfunc::enter_continue_char();

   for(unsigned int m = 0; m < mask_filenames.size(); m++)
   {
     outputfunc::update_progress_fraction(m, 500, mask_filenames.size());

     if(m%1000 == 0 && m > 0)
     {
        cout << endl;
        print_class_pixel_info(class_pixel_freqs);
        outputfunc::print_elapsed_time();
     }

     if(texture_rectangle_ptr == NULL)
     {
        texture_rectangle_ptr=new texture_rectangle(mask_filenames[m], NULL);
        width = texture_rectangle_ptr->getWidth();
        height = texture_rectangle_ptr->getHeight();
	cout << "height = " << height << " width = " << width
	     << endl;
	cout << "n_channels = " << texture_rectangle_ptr->getNchannels()
	     << endl;
     }
     else
     {
        texture_rectangle_ptr->fast_import_photo_from_file(mask_filenames[m]);
     }

      for(int py = 0; py < height; py++)
      {
         for(int px = 0; px < width; px++)
         {
 	    int curr_class = texture_rectangle_ptr->
               fast_get_pixel_intensity_value(px, py);
            class_pixel_freqs[curr_class] = class_pixel_freqs[curr_class] + 1;
            if (curr_class >= 2)
            {
               cout << "m = " << m << " mask_filename = " 
                    << mask_filenames[m] << endl;
               cout << "px = " << px << " py = " << py 
                    <<  " curr_class = " << curr_class << endl;
               outputfunc::enter_continue_char();
            }
         }
      }
      
/*
      texture_rectangle* colored_texture_rectangle_ptr=new texture_rectangle(
         width, height, 1, 3, NULL);

      string blank_filename="blank.jpg";
      colored_texture_rectangle_ptr->generate_blank_image_file(
         width, height, blank_filename, 0.5);
      colored_texture_rectangle_ptr->
         convert_single_twoDarray_to_three_channels(
            texture_rectangle_ptr->get_ptwoDarray_ptr(), true);
      colored_texture_rectangle_ptr->convert_grey_values_to_hues(180,0);

      string basename=filefunc::getbasename(mask_filenames[m]);
      string prefix = stringfunc::prefix(basename);
      string output_filename=maskdir+basename+"_mask.jpg";
      colored_texture_rectangle_ptr->write_curr_frame(output_filename);
      string banner="Exported "+output_filename;
      outputfunc::write_banner(banner);

      delete colored_texture_rectangle_ptr;

*/

   } // loop over index m labeling mask filename

   delete texture_rectangle_ptr;
   cout << endl;

   cout << "Final pixel counts and frequency fractions" << endl << endl;
   print_class_pixel_info(class_pixel_freqs);
 
}
