// ==========================================================================
// Program CHOP_WORDS reads in a set of JPG files output by
// NORMALIZE_WORDS containing multi-character words.  The vertical
// height of each image file should precisely equal 32 pixels in size.
// It excises 32x32 "window" images and shifts 16 pixels over for each
// new window.  The exported images have filenames of the form
// text_123456.jpg.
// ==========================================================================
// Last updated on 6/2/12; 6/3/12; 6/5/12; 6/17/14
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

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("jpg");
   string words_subdir="./training_data/renorm_words/";
   bool search_all_children_dirs_flag=false;

   vector<string> image_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,words_subdir,search_all_children_dirs_flag);
   cout << "image_filenames.size() = " << image_filenames.size() << endl;

   vector<string> chopped_filenames;
   string chopped_subdir="./chopped_words/";
   filefunc::dircreate(chopped_subdir);

   unsigned int width,height;
   int counter=0;
//   int counter=48970;

   int i_start=0;
//   int i_start=8813;
   for (unsigned int i=i_start; i<image_filenames.size(); i++)
//   for (int i=0; i<10; i++)
   {
      if (i%100==0) cout << i << " " << flush;
      imagefunc::get_image_width_height(
         image_filenames[i],width,height);
      int n_chopped_words=width/32;
      
      for (int n=0; n<2*n_chopped_words; n++)
      {
         int xoffset=16*n;

// We must ensure that all chopped words are precisely 32x32 in size.
// So check difference between current xoffset and width:

         if (width-xoffset < 20) continue;
         if (width-xoffset < 32) xoffset=width-32;

         string output_filename=chopped_subdir+
            "text_"+stringfunc::integer_to_string(counter,6)+".jpg";
         counter++;

         imagefunc::crop_image(
            image_filenames[i],output_filename,32,32,xoffset,0);
      } // loop over index n labeling chopped words
      
//      outputfunc::enter_continue_char();

   } // loop over index i labeling all renormalized word image files
   cout << endl;

}

