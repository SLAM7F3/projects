// ==========================================================================
// Program PARSE_WORDS
// ==========================================================================
// Last updated on 6/2/12
// ==========================================================================

#include  <iostream>
#include  <string>
#include  <vector>

#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(12);
   
   string input_subdir="./training_data/text_database/";
   string input_filename=input_subdir+"text_locs.dat";
   filefunc::ReadInfile(input_filename);

   string output_subdir="./parsed_words/";
   filefunc::dircreate(output_subdir);
   
   int word_counter=0;
   int image_number=-1;
   string image_filename;
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      cout << filefunc::text_line[i] << endl;
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);
      if (substrings[0]=="ITEM:")
      {
         image_number++;
         cout << "Processing image "+stringfunc::number_to_string(image_number)
              << endl;
//         outputfunc::enter_continue_char();
      }
      else if (substrings[0]=="IMAGE:")
      {
         image_filename=input_subdir+"text_img"+
            stringfunc::integer_to_string(image_number,4)+".png";
         cout << "image_filename = " << image_filename << endl;
      }
      else if (substrings[0]=="RECT:")
      {
         int px=stringfunc::string_to_number(substrings[2]);
         int py=stringfunc::string_to_number(substrings[4]);
         int w=stringfunc::string_to_number(substrings[6]);
         int h=stringfunc::string_to_number(substrings[8]);

// Ignore any word which is too small in size:

         if (w >= 25 && h >= 25)
         {
            cout << "px = " << px << " py = " << py
                 << " w = " << w << " h = " << h << endl;
            string output_filename=output_subdir+"word_"+
               stringfunc::integer_to_string(word_counter++,5)+".png";
            imagefunc::crop_image(
               image_filename,output_filename,w,h,px,py);
         }
      }
      

   }

}

