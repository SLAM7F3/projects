// ==========================================================================
// Program COMPUTE_TEXTURE_HISTOGRAMS loops over all images within
// some set of subdirectories.  For each input image, it computes
// gradient magnitude and phases for separate R, G and B color
// channels.  The results are separated into 45, 90, 135 and 180
// orientation angle bins as well as logarithmic fractional magnitude
// bins.  Binning according to coarse image plane gridding is
// performed as well.  Concatenated texture histograms are exported to
// output text files.

//			./compute_texture_histograms

// ==========================================================================
// Last updated on 10/22/13; 10/25/13; 12/21/13
// ==========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "video/camerafuncs.h"
#include "optimum/emdL1.h"
#include "general/filefuncs.h"
#include "video/descriptorfuncs.h"
#include "image/imagefuncs.h"
#include "video/photograph.h"
#include "math/prob_distribution.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ios;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
   timefunc::initialize_timeofday_clock();      

   vector<string> input_image_subdirs;

   string ImageEngine_subdir="/data/ImageEngine/";
   string BostonBombing_subdir=ImageEngine_subdir+"BostonBombing/";
   string JAV_subdir=BostonBombing_subdir+"clips_1_thru_25/";
   string root_subdir=JAV_subdir;
   string images_subdir=root_subdir+"jpg_frames/";

/*
//   string JAV_subdir="/data/video/JAV/NewsWraps/early_Sep_2013/";
   string JAV_subdir="/data/video/JAV/NewsWraps/w_transcripts/";
//   string JAV_subdir="/data/video/JAV/UIUC_Broadcast_News/";
//   string tidmarsh_subdir=ImageEngine_subdir+"tidmarsh/";
   string root_subdir=JAV_subdir;
   string images_subdir=root_subdir+"jpg_frames/";
*/

   string texture_histogram_subdir=root_subdir+"texture_histograms/";
   filefunc::dircreate(texture_histogram_subdir);

   input_image_subdirs.push_back(images_subdir);

   int n_horiz_subimage_bins=6;
   int n_vert_subimage_bins=6;
   int n_frac_mag_bins=7;
   int theta_bin,n_theta_bins=4;

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
            double imagenumber_frac=double(imagenumber-imagenumber_start)/
               double(imagenumber_stop-imagenumber_start);
            outputfunc::print_elapsed_and_remaining_time(imagenumber_frac);

            cout << "Processing image " << imagenumber+1 << " of " 
                 << n_images << endl;
         }
         string image_filename=image_filenames[imagenumber];

// Export texture histogram to output text file:

         string basename=filefunc::getbasename(image_filename);
         string prefix=stringfunc::prefix(basename);
         string texture_histogram_filename=texture_histogram_subdir+
            prefix+".texture_hist";
         
// Check whether texture histogram file for current image
// already exists.  If so, move on...

         if (filefunc::fileexist(texture_histogram_filename))
         {
            cout << "Texture histogram file already exists" << endl;
            continue;
         }

         vector<double> texture_histogram=
            descriptorfunc::compute_RGB_texture_histogram(
               image_filename,n_horiz_subimage_bins,n_vert_subimage_bins,
               n_frac_mag_bins,n_theta_bins);
   
         ofstream texture_stream;
         filefunc::openfile(texture_histogram_filename,texture_stream);

         for (int b=0; b<texture_histogram.size(); b++)
         {
            texture_stream << texture_histogram[b] << " " << flush;
         }
         texture_stream << endl;
         filefunc::closefile(texture_histogram_filename,texture_stream);
         cout << "Exported texture histogram to "+
            texture_histogram_filename << endl << endl;

      } // loop over imagenumber index labeling input images

   } // loop over input_image_subdir_index

   cout << "At end of program COMPUTE_TEXTURE_HISTOGRAMS" << endl;
   outputfunc::print_elapsed_time();
}
