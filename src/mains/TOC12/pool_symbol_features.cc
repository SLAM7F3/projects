// ==========================================================================
// POOL_SYMBOL_FEATURES reads in a set of text or non-text images
// files from a specified input subdirectory.  The width or height for
// each input image should precisely equal 32 pixels in size, but both
// need not equal 32.  It also imports a precomputed symbol dictionary
// along with whitening mean and covariance matrices.  Looping over
// each input window file, POOL_SYMBOL_FEATURES extracts 8x8 patches
// in each window for every possible ~25x~25 pixel offset.  Each 8x8
// patch is projected onto the dictionary and converted into a
// K-dimensional histogram. The histogram descriptors are averaged
// together within 3x3 regions of the ~32x~32 window.  So the final
// descriptor for each ~32x~32 window is a 9*K dimensional vector.
// The 9*K dimensional window descriptors for all valid ~32x~32
// windows are exported to windows_descriptors.hdf5.

// 			      pool_symbol_features

// ==========================================================================
// Last updated on 10/4/12; 10/20/12; 10/21/12; 6/7/14
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

   vector<string> symbol_names;
//   symbol_names.push_back("orange_biohazard");
//   symbol_names.push_back("yellow_radiation");
//   symbol_names.push_back("blue_radiation");
//   symbol_names.push_back("blue_water");
   symbol_names.push_back("blue_gas");
//   symbol_names.push_back("red_stop");
//   symbol_names.push_back("bw_skull");
//   symbol_names.push_back("bw_eat");
//   symbol_names.push_back("green_start");

/*
  string symbol_name;
  cout << "Enter symbol name:" << endl;
  cout << "  yellow_radiation,orange_biohazard,blue_radiation" << endl;
  cout << "  blue_water,blue_gas,red_stop" << endl;
  cout << "  green_start,bw_skull,bw_eat:" << endl;
  cin >> symbol_name;

  double hours_to_sleep;
  cout << "Enter number of hours to initially sleep:" << endl;
  cin >> hours_to_sleep;
  int secs_to_sleep=hours_to_sleep*3600;

// FAKE FAKE:  Thurs Oct 4 at 11:17 am

cout << "Sleeping for " << hours_to_sleep << " hours" << endl;
sleep(secs_to_sleep);
*/

   bool RGB_pixels_flag=false;
//   bool RGB_pixels_flag=true;
   cout << "RGB_pixels_flag = " << RGB_pixels_flag << endl;

   int D=64*3;	// RGB color
   if (!RGB_pixels_flag)
   {
      D=64;	// greyscale
   }

//   string pool_char="t";
   string pool_char="n";
   cout << "Enter 't' to pool text features or 'n' to pool non-text features:"
        << endl;
   cin >> pool_char;

   bool text_image_input_flag=true;
   if (pool_char=="n")
   {
      text_image_input_flag=false;
   }

   for (unsigned int s=0; s<symbol_names.size(); s++)
   {
      string symbol_name=symbol_names[s];

      string banner="Pooling features for "+symbol_name;
      outputfunc::write_big_banner(banner);

      string final_signs_subdir="./images/final_signs/";
//   string ppt_signs_subdir="./images/ppt_signs/";
      string synthetic_subdir=final_signs_subdir+"synthetic_symbols/";
      string synthetic_symbols_subdir=synthetic_subdir+symbol_name+"/";
      string dictionary_subdir=synthetic_symbols_subdir;

      string not_particular_signs_subdir=
         "./images/non_signs/not_particular_final_signs/";

      text_detector* text_detector_ptr=new text_detector(dictionary_subdir, RGB_pixels_flag);
      text_detector_ptr->import_inverse_sqrt_covar_matrix();

      vector<string> allowed_suffixes;
      allowed_suffixes.push_back("png");
      allowed_suffixes.push_back("jpg");
   
      vector<string> training_images_subdirs;
      if (text_image_input_flag)
      {
         training_images_subdirs.push_back(dictionary_subdir);
      }
      else
      {
         training_images_subdirs.push_back(
            "/home/cho/programs/c++/svn/projects/src/mains/TOC12/images/non_signs/all_nonsigns/non_symbols/");
         
         string not_sign_subdir=not_particular_signs_subdir+
            "not_"+symbol_name+"/";
         training_images_subdirs.push_back(not_sign_subdir);
      }

      bool search_all_children_dirs_flag=false;
      vector<string> image_filenames;

      for (unsigned int t=0; t<training_images_subdirs.size(); t++)
      {
         cout << "t = " << t << " training_images_subdir = " 
              << training_images_subdirs[t] << endl;
         vector<string> curr_image_filenames=
            filefunc::files_in_subdir_matching_specified_suffixes(
               allowed_suffixes,training_images_subdirs[t],
               search_all_children_dirs_flag);

         int n_training_images=curr_image_filenames.size();
         n_training_images=basic_math::min(n_training_images,60000);
//         n_training_images=basic_math::min(n_training_images,80000);
         for (int j=0; j<n_training_images; j++)
         {
            image_filenames.push_back(curr_image_filenames[j]);
         } // loop over index j
      } // loop over index t labeling training images subdirs

      int n_images=image_filenames.size();
      cout << "Number of input images = " << n_images << endl;


      flann::Matrix<float>* window_descriptors_ptr=
         new flann::Matrix<float>(new float[n_images*9*K],n_images,9*K);

      int i_start=0;
      int i_stop=n_images;
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

// Perform TOC12 sign-dependent conversion from RGB to greyscale:

         if (symbol_name=="yellow_radiation" ||
         symbol_name=="orange_biohazard")
         {
            text_detector_ptr->get_texture_rectangle_ptr()->
               convert_color_image_to_greyscale(); 
         }
         else if (symbol_name=="blue_radiation")
         {
            text_detector_ptr->get_texture_rectangle_ptr()->
               convert_color_image_to_single_color_channel(1,true);  
            // red channel
         }
         else
         {
            text_detector_ptr->get_texture_rectangle_ptr()->
               convert_color_image_to_luminosity();
         }

         text_detector_ptr->set_window_width(width);
         text_detector_ptr->set_window_height(height);

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
      flann::save_to_file(
         *window_descriptors_ptr,window_descriptors_hdf5_filename,
         "window_descriptors");
      delete window_descriptors_ptr;

      banner="Wrote 9*K="+stringfunc::number_to_string(9*K)
         +" dimensional descriptors for "
         +stringfunc::number_to_string(window_features_counter)
         +" window images to "+window_descriptors_hdf5_filename;
      outputfunc::write_big_banner(banner);

   } // loop over index s labeling symbol names
   
}

   
