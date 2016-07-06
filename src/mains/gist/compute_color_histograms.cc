// ==========================================================================
// Program COMPUTE_COLOR_HISTOGRAMS loops over all images within an
// images directory.  It forceably resizes each input image to a
// standard width and height measured in pixels.
// COMPUTE_COLOR_HISTGRAMS next exports their color histograms to text
// files within a color_histograms subdirectory of the images
// directory. A calculated histogram can either be for an entire image
// or multiple regions of interest within an image.

// 			 ./compute_color_histograms

// ==========================================================================
// Last updated on 1/1/14; 1/3/14; 1/9/14
// ==========================================================================

#include  <iostream>
#include  <string>
#include  <vector>

#include "general/filefuncs.h"
#include "video/descriptorfuncs.h"
#include "general/outputfuncs.h"
#include "video/RGB_analyzer.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   timefunc::initialize_timeofday_clock();      

// Import basic HOG BoW processing parameters:

   string gist_subdir="../gist/";
   string BoW_params_filename=gist_subdir+"BoW_params.dat";
   filefunc::ReadInfile(BoW_params_filename);

   bool video_frames_input_flag=
      stringfunc::string_to_boolean(filefunc::text_line[8]);
   cout << "video_frames_input_flag = " << video_frames_input_flag << endl;

   int standard_width,standard_height;
   if (video_frames_input_flag)
   {
      standard_width=stringfunc::string_to_number(filefunc::text_line[4]);
      standard_height=stringfunc::string_to_number(filefunc::text_line[5]);
   }
   else
   {
      standard_width=stringfunc::string_to_number(filefunc::text_line[6]);
      standard_height=stringfunc::string_to_number(filefunc::text_line[7]);
   }

   cout.precision(12);
   RGB_analyzer* RGB_analyzer_ptr=new RGB_analyzer();
   string liberalized_color="";
   RGB_analyzer_ptr->import_quantized_RGB_lookup_table(liberalized_color);

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();

   int video_corpus_ID;
   string JAV_subdir;
   if (video_frames_input_flag)
   {
      string ImageEngine_subdir="/data/ImageEngine/";
      string BostonBombing_subdir=ImageEngine_subdir+"BostonBombing/";

      cout << "1: early September 2013 NewsWraps" << endl;
      cout << "2: October 2013 NewsWraps with transcripts" << endl;
      cout << "3: Boston Bombing YouTube clips 1 - 25" << endl;

      cout << "Enter video corpus ID:" << endl;
      cin >> video_corpus_ID;
   
      if (video_corpus_ID==1)
      {
         JAV_subdir="/data/video/JAV/NewsWraps/early_Sep_2013/";
      }
      else if (video_corpus_ID==2)
      {
         JAV_subdir="/data/video/JAV/NewsWraps/w_transcripts/";
      }
      else if (video_corpus_ID==3)
      {
         JAV_subdir=BostonBombing_subdir+"clips_1_thru_25/";
      }
      else
      {
         exit(-1);
      }
   }
   else
   {
      JAV_subdir="./bundler/tidmarsh/";
   } // video_frames_input_flag conditional
   cout << "JAV_subdir = " << JAV_subdir << endl;

   string root_subdir=JAV_subdir;
   string images_subdir;
   if (video_frames_input_flag)
   {
      images_subdir=root_subdir+"jpg_frames/";
   }
   else
   {
      images_subdir=root_subdir+"standard_sized_images/";
   }

   vector<string> image_filenames=filefunc::image_files_in_subdir(
      images_subdir);

   string global_char;
   bool global_color_histogram_flag=true;
   cout << "Enter 'g' to compare global rather than local color histograms"
        << endl;
   cin >> global_char;
   if (global_char != "g")
   {
      global_color_histogram_flag=false;
   }

   string output_subdir=root_subdir+"color_histograms/";
   if (global_color_histogram_flag)
   {
      output_subdir=root_subdir+"global_color_histograms/";
   }
   filefunc::dircreate(output_subdir);

   int n_images=image_filenames.size();
   for (int i=0; i<n_images; i++)
   {
      if (i%10==0)
      {
         double progress_frac=double(i)/image_filenames.size();
         cout << "Progress fraction = " << progress_frac << endl;
         outputfunc::print_elapsed_and_remaining_time(progress_frac);
      }

      string image_filename=image_filenames[i];
      string basename=filefunc::getbasename(image_filename);
      string prefix=stringfunc::prefix(basename);
//      cout << "i = " << i 
//           << " image_filename = " << image_filename << endl;
      if (prefix.substr(0,10)=="thumbnail_")
      {
         prefix=prefix.substr(10,prefix.size()-10);
      }
      string color_histogram_filename=output_subdir+prefix+".color_hist";
//      cout << "color_histogram_filename = "
//           << color_histogram_filename << endl;

// Check whether color histogram for current image already exists.  If
// so, move on...

      if (filefunc::fileexist(color_histogram_filename)) 
      {
         cout << "Color histogram file already exists" << endl;
         continue;
      }

// As of 12/18/13, we force all input images to conform to a standard
// pixel size:

      string std_sized_image_filename="/tmp/std_sized_image.jpg";
      if (video_corpus_ID > 0)
      {
         std_sized_image_filename += 
            stringfunc::number_to_string(video_corpus_ID);
      }
      std_sized_image_filename += ".jpg";
//      cout << "std_sized_image_filename = " << std_sized_image_filename 
//           << endl;

      videofunc::force_size_image(
         image_filenames[i],standard_width,standard_height,
         std_sized_image_filename);
      
//      bool conventional_dotproduct_flag=true;
      bool conventional_dotproduct_flag=false;

      vector<double> total_color_histogram;
      if (global_color_histogram_flag)
      {
         total_color_histogram=
            descriptorfunc::compute_color_histogram(
               std_sized_image_filename,color_histogram_filename,
               texture_rectangle_ptr,RGB_analyzer_ptr);
      }
      else
      {
         int n_rows=3;
         int n_columns=3;
         for (int r=0; r<n_rows; r++)
         {
            for (int c=0; c<n_columns; c++)
            {
               vector<double> curr_hist=
                  descriptorfunc::compute_sector_color_histogram(
                     n_rows,n_columns,r,c,std_sized_image_filename,
                     texture_rectangle_ptr,RGB_analyzer_ptr);

               double sqr_norm=1;
               if (conventional_dotproduct_flag)
               {

// L2 normalize current sector histogram:

                  sqr_norm=0;
                  for (int h=0; h<curr_hist.size(); h++)
                  {
                     sqr_norm += sqr(curr_hist[h]);
                  }
               }
               
// L2 normalize total color histogram:

               double normalization_factor=
                  1.0/sqrt(n_rows*n_columns*sqr_norm);
               for (int h=0; h<curr_hist.size(); h++) 
               {
                  curr_hist[h] *= normalization_factor;
                  total_color_histogram.push_back(curr_hist[h]);
               }
                  
            } // loop over index c labeling columns
         } // loop over index r labeling rows
      } // global_color_histogram_flag conditional
          
      ofstream color_stream;
      filefunc::openfile(color_histogram_filename,color_stream);

      for (int b=0; b<total_color_histogram.size(); b++)
      {
         color_stream << total_color_histogram[b] << " " << flush;
      }
      color_stream << endl;
      filefunc::closefile(color_histogram_filename,color_stream);

/*
      if (global_color_histogram_flag)
      {
         cout << "Exported global color histogram to " 
              << color_histogram_filename << endl << endl;
      }
      else
      {
         cout << "Exported multi-sectored color histogram to " 
              << color_histogram_filename << endl << endl;
      }
*/

   } // loop over index i labeling image filenames

   delete RGB_analyzer_ptr;
   delete texture_rectangle_ptr;

   cout << "At end of program COMPUTE_COLOR_HISTOGRAMS" << endl;
   outputfunc::print_elapsed_time();
}

