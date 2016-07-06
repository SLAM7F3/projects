// ==========================================================================
// Program COMPUTE_SEGMENT_HISTOGRAMS loops over all images within
// some set of subdirectories.  For each input image, it calls the LSD
// algorithms/codes by von Gioi (Nov 2011) to extract a set of line
// segments.  COMPUTE_SEGMENT_HISTOGRAMS then computes a 2D histogram
// for the extracted segments where the bins' independent variables
// segment angle and magnitude.  The calculated histograms are
// exported to output text files.
// ==========================================================================
// Last updated on 9/4/13; 9/5/13; 9/6/13; 11/21/13
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

/*
   string image_filename;
   vector<string> input_params;
   if (!filefunc::parameter_input(argc,argv,input_params))
   {
      cout << "Enter image filename:" << endl;
      cin >> image_filename;
   }
   else
   {
      image_filename=input_params.back();
   }
//   cout << "image_filename = " << image_filename << endl;
*/

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
//   input_image_subdirs.push_back(BostonBombing_subdir);
//   input_image_subdirs.push_back(NewsWrap_subdir);
//   input_image_subdirs.push_back(Nightline_subdir);
//   input_image_subdirs.push_back(NorthKorea_subdir);
   input_image_subdirs.push_back(tidmarsh_subdir);

   int n_frac_mag_bins=4;
   int n_theta_bins=4;

   for (unsigned int input_image_subdir_index=0; input_image_subdir_index < 
           input_image_subdirs.size(); input_image_subdir_index++)
   {
      string curr_image_subdir=input_image_subdirs[input_image_subdir_index];
      
      vector<string> image_filenames=filefunc::image_files_in_subdir(
         curr_image_subdir);
      int n_images=image_filenames.size();
      cout << "n_images = " << n_images << endl;

      string segments_histogram_subdir=
         curr_image_subdir+"segment_histograms/";   
      filefunc::dircreate(segments_histogram_subdir);

      int imagenumber_start=0;
//      int imagenumber_stop=10;
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

// Export line segments histogram to output text file:

         string basename=filefunc::getbasename(image_filename);
         string prefix=stringfunc::prefix(basename);
         string segments_histogram_filename=segments_histogram_subdir+
            prefix+".segments_hist";
         
// Check whether line segments histogram file for current image
// already exists.  If so, move on...

         if (filefunc::fileexist(segments_histogram_filename))
         {
            cout << "Line segments histogram file already exists" << endl;
            continue;
         }

         unsigned int width,height;
         imagefunc::get_image_width_height(image_filename,width,height);
//          double aspect_ratio=double(width)/double(height);

         int n_horiz_subimage_bins=8;
         int n_vert_subimage_bins=8;
//         if (aspect_ratio > 1)
//         {
//            n_horiz_subimage_bins=8;
//            n_vert_subimage_bins=n_horiz_subimage_bins/aspect_ratio;
//         }
//         else
//         {
//            n_vert_subimage_bins=8;
//            n_horiz_subimage_bins=n_vert_subimage_bins*aspect_ratio;
//         }

         vector<double> linesegments_histogram=descriptorfunc::
            compute_image_segments_histogram(
              n_horiz_subimage_bins,n_vert_subimage_bins,
              n_frac_mag_bins,n_theta_bins,image_filename);

         ofstream segments_stream;
         filefunc::openfile(segments_histogram_filename,segments_stream);

         for (unsigned int b=0; b<linesegments_histogram.size(); b++)
         {
            segments_stream << linesegments_histogram[b] << " " << flush;
         }
         segments_stream << endl;
         filefunc::closefile(segments_histogram_filename,segments_stream);
         cout << "Exported line segments histogram to "+
            segments_histogram_filename << endl << endl;
         
// Superpose detected line segments on input image:

//   bool display_line_segments_flag=true;
         bool display_line_segments_flag=false;
         if (display_line_segments_flag)
         {
            texture_rectangle* texture_rectangle_ptr=new texture_rectangle();
            if (!texture_rectangle_ptr->reset_texture_content(image_filename))
            {
               exit(-1);
            }
            vector<linesegment> linesegments=videofunc::detect_line_segments(
               texture_rectangle_ptr);
   
            string linesegments_filename="./linesegments.jpg";
            int segment_color_index=-1;	// random segment coloring
            videofunc::draw_line_segments(
               linesegments,texture_rectangle_ptr,segment_color_index);
            texture_rectangle_ptr->write_curr_frame(linesegments_filename);

            cout << "Exported detected line segments to "+linesegments_filename
                 << endl;

            delete texture_rectangle_ptr;
         } // display_line_segments_flag conditional

      } // loop over imagenumber index labeling input images

   } // loop over input_image_subdir_index

   cout << "At end of program COMPUTE_SEGMENT_HISTOGRAMS" << endl;
   outputfunc::print_elapsed_time();

}
