// ==========================================================================
// Program PREPARE_CLASSIFICATION_INPUTS imports a set of face and
// non-face image chips from a specified subdirectory.  It generates a
// text file containing the image chip files' full pathnames vs their
// class IDs.  PREPARE_CLASSIFICATION_INPUTS then shuffles the pairs
// of pathnames vs class IDs multiple times.  The shuffled text file
// becomes an input to Caffe fine-tuning of VGG-16/Resnet networks.

//                     ./prepare_classification_inputs

// ==========================================================================
// Last updated on 8/2/16; 8/3/16; 8/6/16; 8/10/16
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

   vector<string> training_images_subdirs;
   string faces_subdir = "/data/caffe/faces/";
   string face_chips_subdir = faces_subdir+"image_chips/";
   string training_subdir = face_chips_subdir+"training/";

   vector<string> dated_subdirs;
   dated_subdirs.push_back("Aug2_female_106x106_augmented/");
   dated_subdirs.push_back("Aug2_male_106x106_augmented/");
   dated_subdirs.push_back("Jul31_106x106_adience/");
   dated_subdirs.push_back("Iran_106x106/");
   dated_subdirs.push_back("nonface_106x106/");

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
   cout << "Total number of input images = " << image_filenames.size()
        << endl;

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

   cout << "n_non_augmented = " << n_non_augmented << endl;
} 


