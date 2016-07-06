// ==========================================================================
// POOL_FEATURES reads in a set of text or non-text images files
// from a specified input subdirectory.  The width or height for each
// input image should precisely equal 32 pixels in size, but both need
// not equal 32.  It also imports a precomputed character dictionary
// along with whitening mean and covariance matrices.  Looping over
// each input window file, POOL_FEATURES extracts 8x8 patches in
// each window with pixel for every possible ~25x~25 pixel offset.  Each
// 8x8 patch is projected onto the dictionary and converted into a
// K-dimensional histogram. The histogram descriptors are averaged
// together within 3x3 regions of the ~32x~32 window.  So the final
// descriptor for each ~32x~32 window is a 9*K dimensional vector.
// The 9*K dimensional window descriptors for all valid ~32x~32
// windows are exported to windows_descriptors.hdf5.

// 			      pool_features

// Note: This version of POOL_FEATURES is deprecated as of Jun
// 2014.  Use mains/programs/imagetext/POOL_FEATURES instead!

// ==========================================================================
// Last updated on 7/12/12; 8/20/12; 8/23/12
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <flann/flann.hpp>
#include <flann/io/hdf5.h>

#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "general/sysfuncs.h"
#include "classification/text_detector.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   int K=1024;

   bool rgb_pixels_flag=false;
   string dictionary_subdir="./training_data/dictionary/";
   text_detector* text_detector_ptr=new text_detector(
      dictionary_subdir,rgb_pixels_flag);
   text_detector_ptr->import_inverse_sqrt_covar_matrix();

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("png");
   allowed_suffixes.push_back("jpg");

   string pool_char="t";
   cout << "Enter 't' to pool text features or 'n' to pool non-text features:"
        << endl;
   cin >> pool_char;

   bool text_image_input_flag=true;
   if (pool_char=="n")
   {
      text_image_input_flag=false;
   }

   string training_images_subdir;
   if (text_image_input_flag)
   {
//      training_images_subdir="./training_data/renorm_chars/";
      training_images_subdir="./training_data/synthetic_Ng_chars_July/";
//      training_images_subdir=
//         "./training_data/char_intensity/extremal_regions/all_symbols_extremal_regions/";
   }
   else
   {
      training_images_subdir="./training_data/nontext_images/";
   }
   bool search_all_children_dirs_flag=false;
   vector<string> image_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,training_images_subdir,
         search_all_children_dirs_flag);
   int n_images=image_filenames.size();
//    cout << "Number of input images = " << n_images << endl;

   flann::Matrix<float>* window_descriptors_ptr=
      new flann::Matrix<float>(new float[n_images*9*K],n_images,9*K);

   int i_start=0;
//   int i_stop=3000;
   int i_stop=n_images;
   string banner;
   if (text_image_input_flag)
   {
      banner="Pooling text features for "+stringfunc::number_to_string(
         i_stop-i_start)+" images";
   }
   else
   {
      banner="Pooling non-text features for "+stringfunc::number_to_string(
         i_stop-i_start)+" images";
   }
   outputfunc::write_banner(banner);

   timefunc::initialize_timeofday_clock();

   int window_features_counter=0;
   for (int i=i_start; i<i_stop; i++)
   {
      if (i%50==0 && i > i_start)
      {
         double elapsed_secs=timefunc::elapsed_timeofday_time();
         double avg_time_per_image=elapsed_secs/(i-i_start);
         double estimated_total_time=avg_time_per_image*(i_stop-i_start);
         double estimated_remaining_time=estimated_total_time-elapsed_secs;

         cout << "i=" << i << " of " << i_stop-i_start 
              << "  t=" << elapsed_secs/60 << " mins" 
              << "  time/img=" << avg_time_per_image << " secs"
              << "  t_remaining=" << estimated_remaining_time/60
              << " mins" 
              << endl;
      }

      unsigned int width,height;
      imagefunc::get_image_width_height(image_filenames[i],width,height);
      text_detector_ptr->import_image_from_file(image_filenames[i]);

      text_detector_ptr->set_window_width(width);
      text_detector_ptr->set_window_height(height);
      text_detector_ptr->clear_window_features_vector();

      bool flag=text_detector_ptr->average_window_features(0,0);

      if (!flag)
      {
         cout << "i = " << i << " window features not calculated" << endl;
      }
      else
      {
         float* window_histogram=text_detector_ptr->
            get_nineK_window_descriptor();
         for (int k=0; k<9*K; k++)
         {
            if (fabs(window_histogram[k]) > POSITIVEINFINITY)
            {
               cout << "i = " << i 
                    << " k = " << k << " window_histogram[k] = "
                    << window_histogram[k] << endl;
               outputfunc::enter_continue_char();
            }
            (*window_descriptors_ptr)[window_features_counter][k]=
               window_histogram[k];
         } // loop over index k labeling dictionary descriptors
         window_features_counter++;
      }
   } // loop over index i labeling input images
   cout << endl;

   delete text_detector_ptr;

   string image_type_label="_text";
   if (!text_image_input_flag)
   {
      image_type_label="_nontext";
   }

   string window_descriptors_hdf5_filename=dictionary_subdir+
      stringfunc::number_to_string(window_features_counter)+
      image_type_label+"_window_descriptors.hdf5";
   cout << "window_filename = "
        << window_descriptors_hdf5_filename << endl;
   flann::save_to_file(
      *window_descriptors_ptr,window_descriptors_hdf5_filename,
      "window_descriptors");
   delete window_descriptors_ptr;

   banner="Wrote 9*K="+stringfunc::number_to_string(9*K)
      +" dimensional descriptors for "
      +stringfunc::number_to_string(window_features_counter)
      +" window images to "+window_descriptors_hdf5_filename;
   outputfunc::write_big_banner(banner);
}

   
