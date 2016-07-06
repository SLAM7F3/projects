// ==========================================================================
// Program PREPARE_FINETUNING_INPUTS imports a set of text and
// non-text image chips from a specified subdirectory.  It generates a
// text file containing the image chip files' full pathnames vs their
// class IDs.  Upper and lower case letters are assigned the same
// class IDs.  PREPARE_FINETUNING_INPUTS then shuffles the pairs of
// pathnames vs class IDs multiple times.  The shuffled text file
// becomes an input to Caffe fine-tuning of VGG-16/Alexnet networks.

//                     ./prepare_finetuning_inputs

// ==========================================================================
// Last updated on 1/30/16; 1/31/16; 2/4/16; 2/29/16
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
   const int ascii_0=48;
   const int ascii_9=57;
   const int ascii_a=97;
   const int ascii_z=122;
   const int ascii_A=65;
   const int ascii_Z=90;

   std::set_new_handler(sysfunc::out_of_memory);
   timefunc::initialize_timeofday_clock();
   sysfunc::clearscreen();

   string syntext_subdir = "/data/peter_stuff/imagery/syntext/";
//   string dated_subdir = "mnist_just_digits_Feb29/";
   string dated_subdir = "digits_Feb16_r6_137K/";
   //   string dated_subdir = "digits_Feb16_r5_137K/";
   //   string dated_subdir = "digits_Feb16_r4_137K/";
   //   string dated_subdir = "digits_Feb16_r3_137K/";
   //   string dated_subdir = "digits_Feb16_r2_137K/";
   //    string dated_subdir = "digits_Feb16_r1_137K/";
   //   string dated_subdir = "digits_Feb14_q_137K/";
//   string dated_subdir = "digits_Feb14_p_137K/";
//   string dated_subdir = "handwriting_digits_Feb12_1st/";
   //   string dated_subdir = "digits_Feb4_o_137K_fifth/";
   //   string dated_subdir = "digits_Feb4_o_137K_fourth/";
   //   string dated_subdir = "digits_Feb4_o_137K_third/";
   //   string dated_subdir = "digits_Feb4_o_137K_second/";
   //   string dated_subdir = "digits_Feb4_o_137K_first/";
   //   string dated_subdir = "simple_digits_Feb3_n_150K/";
   //   string dated_subdir = "simple_digits_Feb3_m_second/";
   //   string dated_subdir = "simple_digits_Feb3_m/";
   //   string dated_subdir = "simple_digits_Feb2_l_second/";
   //   string dated_subdir = "simple_digits_Feb2_l/";
   //   string dated_subdir = "simple_digits_Feb1_k/";
   //   string dated_subdir = "simple_digits_Jan31_j/";
   //   string dated_subdir = "simple_digits_Jan30_i/";
   //   string dated_subdir = "simple_digits_Jan30_h/";
   //   string dated_subdir = "simple_digits_Jan29_g/";
   //   string dated_subdir = "simple_digits_Jan29_f/";
   //   string dated_subdir = "simple_digits_Jan29_e/";
   //   string dated_subdir = "simple_digits_Jan28_d/";
   //   string dated_subdir = "simple_digits_Jan28_c/";
   //   string dated_subdir = "simple_digits_Jan28_b/";
   //   string dated_subdir = "simple_digits_Jan28_a/";
   //   string dated_subdir = "numbers_Jan26_a/";
   //   string dated_subdir = "svhn_w_background_Jan26/";
   //   string dated_subdir = "svhn_digits_Jan26/";
   //   string dated_subdir = "numbers_Jan18/";
   //   string dated_subdir = "letters_Jan18/";
   //   string dated_subdir = "numbers_Jan25/";
   //   string dated_subdir = "numbers_Jan16/";
   //   string dated_subdir = "letters_Jan16/";

   string training_images_subdir = syntext_subdir + dated_subdir;
   bool just_digits = true;
   bool just_letters = false;
   bool letters_and_digits = false;
   
   cout << "just_digits = " << just_digits << endl;
   cout << "just_letters = " << just_letters << endl;
   cout << "letters_and_digits = " << letters_and_digits << endl;
   cout << "Specified training_images_subdir = " << training_images_subdir
        << endl;
   cout << "Make sure this subdirectory contains the latest training imagery!"
        << endl;
   outputfunc::enter_continue_char();

// First, reset soft link from syntext_subdir/object_names.classes
// depending upon whether just_digits, just_letters or
// letters_and_digits flag == true:

   string unix_cmd = "/bin/rm "+syntext_subdir+"object_names.classes";
   sysfunc::unix_command(unix_cmd);

   if(just_digits)
   {
      unix_cmd="ln -s "+syntext_subdir+"digit_names.classes "+
         syntext_subdir+"object_names.classes";
   }
   else if (just_letters)
   {
      unix_cmd="ln -s "+syntext_subdir+"letter_names.classes "+
         syntext_subdir+"object_names.classes";

   }
   else if (letters_and_digits)
   {
      unix_cmd="ln -s "+syntext_subdir+"digit_and_letter_names.classes "+
         syntext_subdir+"object_names.classes";
   }
   sysfunc::unix_command(unix_cmd);


   vector<string> image_filenames=filefunc::image_files_in_subdir(
     training_images_subdir);
   
   string output_filename=syntext_subdir+"all_images_vs_classes.txt";
   ofstream output_stream;
   filefunc::openfile(output_filename, output_stream);

   unsigned int istart=0;
   unsigned int istop = image_filenames.size();
   
   string output_subdir="/"+dated_subdir;
   for(unsigned int i = istart; i < istop; i++)
   {
      string basename = filefunc::getbasename(image_filenames[i]);
      vector<string> substrings = stringfunc::decompose_string_into_substrings(
        basename,"_");
      
      char curr_char=stringfunc::string_to_char(substrings[0]);

// Recall for MNIST digits, label occurs in 1st rather than 0th substring:
//      char curr_char=stringfunc::string_to_char(substrings[1]);

      int curr_ascii_val = stringfunc::char_to_ascii_integer(curr_char);

      int curr_label = -1;
      if(substrings[0] == "non" || substrings[0] == "ambiguous")
      {
         curr_label = 0;	// Non-text label
      }
      else if(curr_ascii_val >= ascii_0 && curr_ascii_val <= ascii_9)
      {
         curr_label = curr_ascii_val - ascii_0 + 1; // Digit
      }
      else if(curr_ascii_val >= ascii_A && curr_ascii_val <= ascii_Z)
      {
         curr_label = curr_ascii_val - ascii_A + 1;         
         if(letters_and_digits) curr_label += 10;
      }
      else if(curr_ascii_val >= ascii_a && curr_ascii_val <= ascii_z)
      {
         curr_label = curr_ascii_val - ascii_a + 1;         
         if(letters_and_digits) curr_label += 10;
      }
      else
      {
         cout << "curr_char = " << curr_char << endl;
         cout << "Error:  Ascii val = " << curr_ascii_val 
              << " does not correspond to a digit or letter!" << endl;
         exit(-1);
      }
      
// Text character --> curr_label

// "non-text"  	--> 0
// "0"  	--> 1
// "1"  	--> 2
// "9"  	--> 10
// "A", "a"     --> 11
// "B", "b"     --> 12
// "Z", "z"     --> 36

      string output_image_filename=output_subdir+basename;
      output_stream << output_image_filename << "  " << curr_label << endl;

      //      cout << "i = " << i << " basename = " << basename
      //     << " substrings[0] = " << substrings[0] 
      //     << " substrings[1] = " << substrings[1] 
      //     << " curr_label = " << curr_label
      //     << endl;

   } // loop over index i 

   filefunc::closefile(output_filename, output_stream);

   string banner="Image paths vs class labels exported to "+
     output_filename;
   outputfunc::write_banner(banner);

// Multiply shuffle pairs of image paths and class labels:

   string input_filename = output_filename;
   string tmp_shuffled_filename;
   int n_shuffles = 10;

   string shuffled_filename=syntext_subdir;
   if(just_digits)
   {
      shuffled_filename += "shuffled_all_digit_images_vs_classes.txt";
   }
   else if(just_letters)
   {
      shuffled_filename += "shuffled_all_letter_images_vs_classes.txt";
   }
   else if(letters_and_digits)
   {
      shuffled_filename += "shuffled_all_letter_and_digit_images_vs_classes.txt";
   }

/*
   for(int iter = 0; iter < n_shuffles; iter++)
   {
      filefunc::ReadInfile(input_filename);

      vector<int> shuffled_image_indices = mathfunc::random_sequence(
         image_filenames.size());   

      tmp_shuffled_filename = "./shuffled_"
         +stringfunc::number_to_string(iter)+".txt";

      filefunc::openfile(tmp_shuffled_filename, output_stream);
      for(unsigned int i = istart; i < istop; i++)
      {
         int curr_index = shuffled_image_indices[i];
         output_stream << filefunc::text_line[curr_index] << endl;
      }
      filefunc::closefile(tmp_shuffled_filename, output_stream);

      input_filename=tmp_shuffled_filename;
      output_filename=tmp_shuffled_filename;
      cout << "iter = " << iter 
           << " output_filename = " << output_filename << endl;
   }
*/

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
// names to dataset.txt within $syntext_subdir:

   unix_cmd = "/bin/rm "+syntext_subdir+"dataset.txt";
   sysfunc::unix_command(unix_cmd);
   unix_cmd = "ln -s "+shuffled_filename+" "+syntext_subdir+"dataset.txt";
   sysfunc::unix_command(unix_cmd);

   banner="Shuffled image paths vs class labels exported to "+
      shuffled_filename;
   outputfunc::write_banner(banner);
} 


