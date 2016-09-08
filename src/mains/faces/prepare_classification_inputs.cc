// ==========================================================================
// Program PREPARE_CLASSIFICATION_INPUTS imports a set of face and
// non-face image chips from a specified subdirectory.  It generates a
// text file containing the image chip files' full pathnames vs their
// class IDs.  PREPARE_CLASSIFICATION_INPUTS then shuffles the pairs
// of pathnames vs class IDs multiple times.  The shuffled text file
// becomes an input to Caffe fine-tuning of VGG-16/Resnet networks.

// This program can also be used to compute the mean RGB values for
// all training image chips.

//                     ./prepare_classification_inputs

// ==========================================================================
// Last updated on 8/15/16; 8/28/16; 9/7/16; 9/8/16
// ==========================================================================

#include <iostream>
#include <Magick++.h>
#include <string>
#include <vector>
#include "color/colorfuncs.h"
#include "math/constants.h"
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
   timefunc::initialize_timeofday_clock();
   sysfunc::clearscreen();

    bool compute_mean_RGB_values_flag = true;
//   bool compute_mean_RGB_values_flag = false;

// On 9/8/16, we used this program to compute the following mean RGB
// values for O(360K) 96x96 training face image chips which have as
// little black border as possible:

// Mean B value = 89.6968
// Mean G value = 99.0488
// Mean R value = 114.735

// On 8/28/16, we used this program to compute the following mean RGB
// values for O(350K) 96x96 training face image chips:

//   Mean B value = 38.5125
//   Mean G value = 41.9476
//   Mean R value = 49.0264

   vector<string> training_images_subdirs;
   string faces_subdir = "/data/caffe/faces/";
   string face_chips_subdir = faces_subdir+"image_chips/";
   string training_subdir = face_chips_subdir+"training/Sep7/";

   vector<string> dated_subdirs;
   dated_subdirs.push_back("20k_female_106x106/");
   dated_subdirs.push_back("20k_male_106x106/");
   dated_subdirs.push_back("adience_female_106x106/");
   dated_subdirs.push_back("adience_male_106x106/");
   dated_subdirs.push_back("iran_female_106x106/");
   dated_subdirs.push_back("nonface_106x106/");

//   dated_subdirs.push_back("Aug15_female_106x106_augmented/");
//   dated_subdirs.push_back("Aug15_male_106x106_augmented/");
//   dated_subdirs.push_back("Aug2_female_106x106_augmented/");
//   dated_subdirs.push_back("Aug2_male_106x106_augmented/");
//   dated_subdirs.push_back("Jul31_106x106_adience/");
//   dated_subdirs.push_back("Iran_106x106/");
//   dated_subdirs.push_back("nonface_106x106/");

//    Aug2 female non-augmented = 18940
//    Aug2 male non-augmented = 18877
//    Aug2 female + male non-augmented = 37817
//    Adience non augmented = 16586
//    Iran non augmented = 357
//    Total non augmented faces = 54,760
//    Total augmented faces = 260,788
//    Non-face chips = 99204 

   for(unsigned int d = 0; d < dated_subdirs.size(); d++)
   {
      string curr_training_images_subdir = training_subdir+dated_subdirs[d];
      training_images_subdirs.push_back(curr_training_images_subdir);
   }

   cout << "Specified training_images_subdirs:" << endl;
   for(unsigned t = 0; t < training_images_subdirs.size(); t++)
   {
      cout << training_images_subdirs[t] << endl;
   }
   cout << "Make sure these subdirectories contain the latest training imagery!"
        << endl;
   outputfunc::enter_continue_char();

   string unix_cmd = "/bin/rm "+faces_subdir+"object_names.classes";
   sysfunc::unix_command(unix_cmd);

   bool male_female = true;
   if(male_female)
   {
      unix_cmd="ln -s "+faces_subdir+"male_female.classes "
         +faces_subdir+"object_names.classes";
   }
   sysfunc::unix_command(unix_cmd);

   vector<string> image_filenames;
   for(unsigned t = 0; t < training_images_subdirs.size(); t++)
   {
      vector<string> curr_image_filenames=filefunc::image_files_in_subdir(
         training_images_subdirs[t]);
      for(unsigned int i = 0; i < curr_image_filenames.size(); i++)
      {
         image_filenames.push_back(curr_image_filenames[i]);
      }
   }

   if(compute_mean_RGB_values_flag)
   {
      string banner="Computing mean RGB values";
      outputfunc::write_banner(banner);

      double img_mu_R, img_mu_G, img_mu_B;
      double total_mu_R = 0, total_mu_G = 0, total_mu_B = 0;
      texture_rectangle *tr_ptr = new texture_rectangle(
         image_filenames.front(), NULL);

      int n_images = image_filenames.size();
      double renorm_factor = 1.0 / n_images;
      for(int i = 0; i < n_images; i++)
      {
         double progress_frac=double(i)/n_images;
         if(i > 0 && i%5000 == 0)
         {
            outputfunc::print_elapsed_and_remaining_time(progress_frac);
            cout << "i = " << i << " n_images = " << n_images << endl;
            double factor = double(n_images) / i;
            cout << "Mean B value = " << total_mu_B * factor << endl;
            cout << "Mean G value = " << total_mu_G * factor << endl;
            cout << "Mean R value = " << total_mu_R * factor << endl;
         }
         tr_ptr->fast_import_photo_from_file(image_filenames[i]);
         tr_ptr->get_pixel_RGB_means(img_mu_R, img_mu_G, img_mu_B);
         total_mu_R += renorm_factor * img_mu_R;
         total_mu_G += renorm_factor * img_mu_G;
         total_mu_B += renorm_factor * img_mu_B;
      }
      delete tr_ptr;
      cout << "=================================================== " << endl;
      cout << "Mean B value = " << total_mu_B << endl;
      cout << "Mean G value = " << total_mu_G << endl;
      cout << "Mean R value = " << total_mu_R << endl;
      cout << "=================================================== " << endl;
      exit(-1);
   } // compute_mean_RGB_values_flag conditional

   string output_filename=training_subdir+"all_images_vs_classes.txt";
   ofstream output_stream;
   filefunc::openfile(output_filename, output_stream);

   int n_non_augmented = 0;
   unsigned int istart=0;
   unsigned int istop = image_filenames.size();
   for(unsigned int i = istart; i < istop; i++)
   {
      string basename = filefunc::getbasename(image_filenames[i]);
      vector<string> substrings = stringfunc::decompose_string_into_substrings(
        basename,"_");
      
      int curr_label = 0; // other
      if(substrings[0] == "male")
      {
         curr_label = 1; // male
      }
      else if(substrings[0] == "female")
      {
         curr_label = 2; // female
      }
      else if (substrings[0] == "unknown")
      {
         continue;
      }

      if(substrings[3] == "00") n_non_augmented++;

//      string output_subdir="/image_chips/training/"+dated_subdir;
      string output_subdir = filefunc::getdirname(image_filenames[i]);
      string new_subdir = output_subdir.substr(17, output_subdir.size() - 17);
      string output_image_filename=new_subdir+basename;
//      string output_image_filename=output_subdir+basename;
      output_stream << output_image_filename << "  " << curr_label << endl;

   } // loop over index i 

   filefunc::closefile(output_filename, output_stream);

   string banner="Image paths vs class labels exported to " + output_filename;
   outputfunc::write_banner(banner);

// Multiply shuffle pairs of image paths and class labels:

   string input_filename = output_filename;
   string tmp_shuffled_filename;
//   int n_shuffles = 50;
   int n_shuffles = 100;

   string shuffled_filename=faces_subdir;
   if(male_female)
   {
      shuffled_filename += "shuffled_male_female_images_vs_classes.txt";
   }

   for(int iter = 0; iter < n_shuffles; iter++)
   {
      tmp_shuffled_filename = "./shuffled_"
         +stringfunc::number_to_string(iter)+".txt";
      unix_cmd = "shuf "+input_filename+" > " + tmp_shuffled_filename;
      cout << "iter = " << iter << " unix_cmd = " << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);

      if(iter > 0)
      {
         unix_cmd = "/bin/rm "+input_filename;
         sysfunc::unix_command(unix_cmd);
      }
      
      input_filename=tmp_shuffled_filename;
      output_filename=tmp_shuffled_filename;
   }

   unix_cmd = "mv "+output_filename+" "+shuffled_filename;
   sysfunc::unix_command(unix_cmd);

// Reset soft link from text file holding shuffled image vs class
// names to dataset.txt within $faces_subdir:

   unix_cmd = "/bin/rm "+faces_subdir+"dataset.txt";
   sysfunc::unix_command(unix_cmd);
   unix_cmd = "ln -s "+shuffled_filename+" "+faces_subdir+"dataset.txt";
   sysfunc::unix_command(unix_cmd);

   banner="Shuffled image paths vs class labels exported to "+
      shuffled_filename;
   outputfunc::write_banner(banner);

   int n_augmented = image_filenames.size() - n_non_augmented;
   cout << "Total number of input (augmented + non-augmented) images = " 
        << image_filenames.size() << endl;
   cout << "Number of non-augmented input images = " 
        << n_non_augmented << endl;
   cout << "Number of augmented input images = " 
        << n_augmented << endl;

} 


