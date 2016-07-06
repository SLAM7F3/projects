// ==========================================================================
// Program COMPUTE_LBP_HISTOGRAMS is a minor rewrite of LBPTest.cpp
// written by Navid Nourani-Vatani on 1/25/2013.  It subdivides each
// input image into a small number of regions of interest.  Within
// each ROI, COMPUTE_LBP_HISTOGRAMS calls Noruani-Vatani's LBP
// descriptor library and generates a histogram.  The ROI histgrams
// may be individually L2-normalized and then concatenated together
// into a single large descriptor which may also be L2-normalized.
// Concatenated LPB histograms are exported to output text files.

//			./compute_LBP_histograms

// ==========================================================================
// Last updated on 10/23/13; 10/25/13; 12/21/13
// ==========================================================================

/*
#include <iostream>
#include <ctime>
#include <opencv2/opencv.hpp>
#include "LBP.hpp"

#include "math/basic_math.h"
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

using namespace lbp;
using std::cout;
using std::endl;

// Load and image and calculate the LBP-HF descriptor for the whole image

vector<double> image_descriptor(string image_filename)
{
//   cout << "inside image_descriptor()" << endl;

   // Read an (RGB) image and convert to monochrome
   cv::Mat img = imread( image_filename.c_str(), 0 );

   // convert to double precision
   img.convertTo( img, CV_64F );

//   cout << "image w/h = " << img.rows << "/" << img.cols << " (" 
//        << img.rows*img.cols << ")" << endl;

   // Create an LBP instance of type HF using 8 support points

   LBP lbp( 8, LBP_MAPPING_NONE );
//   LBP lbp(16, LBP_MAPPING_NONE);
//   LBP lbp(8, LBP_MAPPING_RIU2);
//   LBP lbp(12, LBP_MAPPING_RIU2);
//   LBP lbp(16, LBP_MAPPING_RIU2);

   // Calculate the descriptor
   lbp.calcLBP( img );
   vector<double> descriptor = lbp.calcHist().getHist( false );

//   double hist_sum = 0;
   double hist_sqr_sum = 0;
   for( int i = 0; i < descriptor.size(); i++ ) 
   {
//      hist_sum += descriptor[i];
      hist_sqr_sum += sqr(descriptor[i]);
   }

//   double sqr_norm=0;
   for( int i = 0; i < descriptor.size(); i++ ) 
   {
//      descriptor[i] /= hist_sum;
      descriptor[i] /= sqrt(hist_sqr_sum);
//      sqr_norm += sqr(descriptor[i]);
   }

//   double norm=sqrt(sqr_norm);
//   cout << "norm = " << norm << endl;
   
   return descriptor;
}

// ==========================================================================

// Load an image, calculate LBP riu2 and then calculate the histogram
// on sub-regions of the image

vector<double> subregion_descriptors(
   int n_rows,int n_columns,string image_filename) 
{

// Read an (RGB) image and convert to monochrome

   cv::Mat img = imread( image_filename.c_str(), CV_LOAD_IMAGE_GRAYSCALE );

// Convert to double precision

   img.convertTo( img, CV_64F );

// Create an LBP instance of type rotation invariant uniform 2
// using 8 support points

//   LBP lbp( 8, LBP_MAPPING_NONE );
   LBP lbp( 8, LBP_MAPPING_RIU2 );

// Calculate the descriptor image and get it

   Mat lbpImg = lbp.calcLBP( img ).getLBPImage();

// Create a mask same size as the image:

   Mat mask( lbpImg.rows, lbpImg.cols, CV_8UC1 );

   vector<double> total_histogram;

// Compute histograms for sub-images:

   for( int j = 0; j < n_rows; j++ ) 
   {
      for( int i = 0; i < n_columns; i++ ) 
      {
         // Reset mask. Will actually not allocate the data as it is
         // 		same size as before.

         mask = Mat::zeros(lbpImg.rows, lbpImg.cols, CV_8UC1);

// Get a sub-image (ROI) of the whole image:

         int x = lbpImg.cols / n_columns * i;
         int y = lbpImg.rows / n_rows * j;
         int w = lbpImg.cols/n_columns-n_columns;
         int h = lbpImg.rows/n_rows-n_rows;

         Mat roi( mask, Range(y,y+h), Range(x,x+w) );
         roi = Scalar(255);

// Calculate histogram for the ROI:

         vector<double> curr_hist = lbp.calcHist( mask ).getHist();

         bool L2_normalize_flag=false;
//         bool L2_normalize_flag=true;
         double normalization_factor=1;
         if (L2_normalize_flag)
         {

// Renormalize current ROI histogram so that total_histogram has unit
// L2 norm:

            double sqr_norm=0;
            for (int h=0; h<curr_hist.size(); h++)
            {
               sqr_norm += sqr(curr_hist[h]);
            }
            normalization_factor=1.0/sqrt(n_rows*n_columns*sqr_norm);
         }
         
//         cout << "hist(" << j << "," << i << ") = [";
         for (int h= 0; h < curr_hist.size(); h++ ) 
         {
            curr_hist[h] *= normalization_factor;

//            cout << curr_hist[h] << ", ";
            total_histogram.push_back(curr_hist[h]);
         }
//         cout << "]; " << endl;
//         cout << "curr_hist.size() = " << curr_hist.size() << endl;
         
      }
   }

//   double sqr_norm=0;
//   for (int t=0; t<total_histogram.size(); t++)
//   {
//      sqr_norm += sqr(total_histogram[t]);
//   }
//   cout << "Total histogram norm = " << sqrt(sqr_norm) << endl;
//   cout << "total_histogram.size() = " << total_histogram.size() << endl;
 
   return total_histogram;
}

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

//   string JAV_subdir="/data/video/JAV/NewsWraps/early_Sep_2013/";
//   string JAV_subdir="/data/video/JAV/NewsWraps/w_transcripts/";
//   string JAV_subdir="/data/video/JAV/UIUC_Broadcast_News/";
//   string tidmarsh_subdir=ImageEngine_subdir+"tidmarsh/";

//   string root_subdir=JAV_subdir;
//   string images_subdir=root_subdir+"jpg_frames/";

//   string images_subdir=root_subdir;
//   string images_subdir=tidmarsh_subdir;

   string LBP_histogram_subdir=root_subdir+"LBP_histograms/";
   filefunc::dircreate(LBP_histogram_subdir);

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
            double imagenumber_frac=double(imagenumber-imagenumber_start)/
               double(imagenumber_stop-imagenumber_start);
            outputfunc::print_elapsed_and_remaining_time(imagenumber_frac);

            cout << "Processing image " << imagenumber+1 << " of " 
                 << n_images << endl;
         }
         string image_filename=image_filenames[imagenumber];

// Export LBP histogram to output text file:

         string basename=filefunc::getbasename(image_filename);
         string prefix=stringfunc::prefix(basename);
         string LBP_histogram_filename=LBP_histogram_subdir+
            prefix+".LBP_hist";
         
// Check whether LBP histogram file for current image
// already exists.  If so, move on...

         if (filefunc::fileexist(LBP_histogram_filename))
         {
            cout << "LBP histogram file already exists" << endl;
            continue;
         }

//         vector<double> LBP_histogram=image_descriptor(image_filename);

//         int n_rows=4;
//         int n_columns=4;
         int n_rows=8;
         int n_columns=8;
//         int n_rows=10;
//         int n_columns=10;

         vector<double> LBP_histogram=subregion_descriptors(
            n_rows,n_columns,image_filename);
   
         ofstream LBP_stream;
         filefunc::openfile(LBP_histogram_filename,LBP_stream);

         for (int b=0; b<LBP_histogram.size(); b++)
         {
            LBP_stream << LBP_histogram[b] << " " << flush;
         }
         LBP_stream << endl;
         filefunc::closefile(LBP_histogram_filename,LBP_stream);
         cout << "Exported LBP histogram to "+
            LBP_histogram_filename << endl << endl;

      } // loop over imagenumber index labeling input images

   } // loop over input_image_subdir_index

   cout << "At end of program COMPUTE_LBP_HISTOGRAMS" << endl;

   outputfunc::print_elapsed_time();
}

*/
