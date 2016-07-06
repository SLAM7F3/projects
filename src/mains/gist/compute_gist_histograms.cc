// ==========================================================================
// Program COMPUTE_GIST_HISTOGRAMS imports images from a series of
// input subdirectories. Each input image is first rescaled and
// cropped down to 256x256 pixels in size.  The LEAR C program is then
// called to compute a 3*512 dimensional descriptor for the subsampled
// image. All GIST descriptor vectors are written to output text
// files.

//			  ./compute_gist_histograms

// ==========================================================================
// Last updated on 10/21/13; 10/22/13; 10/25/13
// ==========================================================================

#include  <iostream>
#include  <string>
#include  <vector>

#include "general/filefuncs.h"
#include "video/descriptorfuncs.h"
#include "image/imagefuncs.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(12);

   timefunc::initialize_timeofday_clock();

   vector<string> input_image_subdirs;

/*
   string outdoor_categories_subdir="./all_images/training_images/";
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
*/

   string ImageEngine_subdir="/data/ImageEngine/";

   string BostonBombing_subdir=ImageEngine_subdir+
      "BostonBombing/clips_1_thru_133/";
   string Nightline_subdir=ImageEngine_subdir+
      "BostonBombing/Nightline_YouTube2/";
   string NewsWrap_subdir=ImageEngine_subdir+"NewsWrap/";
   string NorthKorea_subdir=
      "/home/cho/programs/c++/svn/projects/src/mains/korea/NK/ground_videos/NorthKorea/";

//   string JAV_subdir="/data/video/JAV/NewsWraps/early_Sep_2013/";
   string JAV_subdir="/data/video/JAV/NewsWraps/w_transcripts/";
//   string JAV_subdir="/data/video/JAV/UIUC_Broadcast_News/";
//   string tidmarsh_subdir=ImageEngine_subdir+"tidmarsh/";

   string root_subdir=JAV_subdir;
   string images_subdir=root_subdir+"jpg_frames/";
//   string root_subdir=NewsWrap_subdir;
//   string images_subdir=root_subdir;

   string gist_subdir=root_subdir+"gist_files/";
   filefunc::dircreate(gist_subdir);

//   input_image_subdirs.push_back(BostonBombing_subdir);
//   input_image_subdirs.push_back(NewsWrap_subdir);
//   input_image_subdirs.push_back(Nightline_subdir);
//   input_image_subdirs.push_back(NorthKorea_subdir);
//   input_image_subdirs.push_back(tidmarsh_subdir);
   input_image_subdirs.push_back(images_subdir);

   for (int input_image_subdir_index=0; input_image_subdir_index < 
           input_image_subdirs.size(); input_image_subdir_index++)
   {
      string curr_image_subdir=input_image_subdirs[input_image_subdir_index];
      
      vector<string> image_filenames=filefunc::image_files_in_subdir(
         curr_image_subdir);
      int n_images=image_filenames.size();
      cout << "n_images = " << n_images << endl;

      int imagenumber_start=0;
      int imagenumber_stop=n_images-1;
      for (int imagenumber=imagenumber_start; imagenumber<=imagenumber_stop; 
           imagenumber++)
      {
         if (imagenumber%10==0) 
         {
            outputfunc::print_elapsed_time();
            cout << "Processing image " << imagenumber+1 << " of " 
                 << n_images << endl;
         }

         string input_filename=image_filenames[imagenumber];
         string image_basename=filefunc::getbasename(input_filename);

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
         
         string input_filename_prefix=stringfunc::prefix(image_basename);

// First check whether gist file for current image already exists.  If
// so, move on...

         string gist_filename=gist_subdir+input_filename_prefix+".gist";
         if (filefunc::fileexist(gist_filename)) 
         {
            cout << "Gist file already exists" << endl;
         }
         else
         {
            descriptorfunc::compute_gist_descriptor(
               input_filename,gist_filename);
         }

      } // loop over imagenumber index labeling input images

   } // loop over input_image_subdir_index
}

