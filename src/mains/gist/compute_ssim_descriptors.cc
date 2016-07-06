// ==========================================================================
// Program COMPUTE_SSIM_DESCRIPTORS

//			./compute_ssim_descriptors

// ==========================================================================
// Last updated on 10/6/13
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
   string BostonBombing_subdir=ImageEngine_subdir+
      "BostonBombing/clips_1_thru_133/";
   string Nightline_subdir=ImageEngine_subdir+
      "BostonBombing/Nightline_YouTube2/";
   string NewsWrap_subdir=ImageEngine_subdir+"NewsWrap/";
   string NorthKorea_subdir=
      "/home/cho/programs/c++/svn/projects/src/mains/korea/NK/ground_videos/NorthKorea/";
   string tidmarsh_subdir=ImageEngine_subdir+"tidmarsh/";
   string JAV_subdir=
      "/data/video/JAV/NewsWraps/early_Sep_2013/";
   string images_subdir=JAV_subdir+"jpg_frames/";
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

      string ssim_descriptor_subdir=JAV_subdir+"ssim_descriptors/";

      filefunc::dircreate(ssim_descriptor_subdir);

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
         string image_filename=image_filenames[imagenumber];

// Export ssim descriptor to output text file:

         string basename=filefunc::getbasename(image_filename);
         string prefix=stringfunc::prefix(basename);
         string ssim_descriptor_filename=ssim_descriptor_subdir+
            prefix+".ssim_hist";
         
// Check whether ssim descriptor file for current imag already exists.
// If so, move on...

         if (filefunc::fileexist(ssim_descriptor_filename))
         {
            cout << "SSIM descriptor file already exists" << endl;
            continue;
         }

// FAKE FAKE:  Sun Oct 6, 2013 at 6:27 pm

//         image_filename="peace_purple.jpg";
//         image_filename="mini_peace_trees.jpg";
         image_filename="mini_peace_people.jpg";


         int n_radial_bins=4;
         int n_theta_bins=20;
         vector<double> ssim_descriptor=
            descriptorfunc::compute_SSIM_descriptor(
               image_filename,n_radial_bins,n_theta_bins);
   
         ofstream ssim_stream;
         filefunc::openfile(ssim_descriptor_filename,ssim_stream);

         for (int b=0; b<ssim_descriptor.size(); b++)
         {
            ssim_stream << ssim_descriptor[b] << " " << flush;
         }
         ssim_stream << endl;
         filefunc::closefile(ssim_descriptor_filename,ssim_stream);
         cout << "Exported ssim descriptor to "+
            ssim_descriptor_filename << endl << endl;

      } // loop over imagenumber index labeling input images

   } // loop over input_image_subdir_index

   cout << "At end of program COMPUTE_SSIM_DESCRIPTORS" << endl;
   outputfunc::print_elapsed_time();

}
