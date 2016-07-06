// ====================================================================
// Program RESET_IMG_NUMBERS is a specialized utility we wrote in order
// to rename "test_images" whose numbering started at 0 as
// "face_images_20" images whose number starts at 5480.
// ====================================================================
// Last updated on 5/7/16
// ====================================================================

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;


int main(int argc, char** argv)
{  
   int delta_img_number = 5480;

// First increase every image number within testimages_xml_filename by
// delta_img_number.  Export new version of XML file:

   string testimages_xml_filename = "./testimages.xml";
   string output_filename = "./renumbered_gimages.xml";
   ofstream outstream;

   filefunc::openfile(output_filename, outstream);

   filefunc::ReadInfile(testimages_xml_filename);
   for(unsigned int i = 0; i < filefunc::text_line.size(); i++)
   {
      string substr="file='";
      string curr_line = filefunc::text_line[i];
      if(stringfunc::first_substring_location(
            curr_line, substr) < 0) 
      {
         outstream << curr_line << endl;
         continue;
      }

      string separator_chars="_.";
      vector<string> substrings = stringfunc::decompose_string_into_substrings(
         curr_line,separator_chars);
      int curr_img_number = stringfunc::string_to_number(substrings[1]);
      int new_img_number = curr_img_number + delta_img_number;
      string new_line = substrings[0] + "_"+stringfunc::integer_to_string(
         new_img_number, 5)+".jpg'>";
      outstream << new_line << endl;
   }
   filefunc::closefile(output_filename, outstream);
   
// Second, rename every image within input_imagesdir by adding
// delta_img_number to its numerical index:

   string input_imagesdir = "./";
   vector<string> image_filenames=filefunc::image_files_in_subdir(
      input_imagesdir);
   cout << "Imported " << image_filenames.size() << " images" << endl;

   output_filename="rename_image_filenames";
   filefunc::openfile(output_filename, outstream);
   for(unsigned int i = 0; i < image_filenames.size(); i++)
   {
      string image_filename=image_filenames[i];
      string suffix=stringfunc::suffix(image_filename);
      string separator_chars="_.";
      vector<string> substrings = stringfunc::decompose_string_into_substrings(
         image_filename,separator_chars);
      int curr_img_number = stringfunc::string_to_number(substrings[1]);
      int new_img_number = curr_img_number + delta_img_number;
      
      string new_image_filename="image_"+stringfunc::integer_to_string(
         new_img_number, 5)+"."+suffix;
      cout << image_filename << " " << new_image_filename << endl;
      string unix_cmd="mv "+image_filename+" "+new_image_filename;
      outstream << unix_cmd << endl;
   }
   filefunc::closefile(output_filename, outstream);
   filefunc::make_executable(output_filename);
}

