/*
 * LBPTest.cpp
 * Example routines for using the LBP class.
 *
 *  Created on: Jan 25, 2013
 *      Author: Navid Nourani-Vatani
 *      Email: Navid.Nourani-Vatani@sydney.edu.au

 */

#include <iostream>
#include <ctime>
#include <opencv2/opencv.hpp>

#include "LBP.hpp"

using namespace lbp;
using std::cout;
using std::endl;


// Load and image and calculate the LBP-HF descriptor for the whole image

vector<double> LBP_HF_descriptor(string image_filename)
{
   cout << endl << "Example 1..." << endl;

   // Read an (RGB) image and convert to monochrome
   cv::Mat img = imread( image_filename.c_str(), 0 );

   // convert to double precision
   img.convertTo( img, CV_64F );
   cout << "image w/h = " << img.rows << "/" << img.cols << " (" 
        << img.rows*img.cols << ")" << endl;

   // Create an LBP instance of type HF using 8 support points
   LBP lbp( 8, LBP_MAPPING_NONE );
   // Calculate the descriptor
   lbp.calcLBP( img );
   // Calculate Fourier tranformed histogram
   vector<double> hist = lbp.calcHist().getHist( false );

   cout << "hist.size() = " << hist.size() << endl;

   double hist_sum = 0;
   for( int i = 0; i < hist.size(); i++ ) 
   {
      hist_sum += hist[i];
   }

// Print out renormalized histogram values:

   cout << "LBP hist = [";
   for( int i = 0; i < hist.size(); i++ ) 
   {
      hist[i] /= hist_sum;
      cout << hist[i] << " " << flush;
   }
   cout << "]; " << endl;

   return hist;
}

/**
 * Load an image, calculate LBP riu2 and then calculate the histogram on sub-regions of the image
 */
void example_2( void ) {
   // Read an (RGB) image and convert to monochrome
   cv::Mat img = imread( "./peace_BW.jpg", CV_LOAD_IMAGE_GRAYSCALE );
//   cv::Mat img = imread( "./peace_purple.jpg", CV_LOAD_IMAGE_GRAYSCALE );
//   cv::Mat img = imread( "../test_image_1.bmp", CV_LOAD_IMAGE_GRAYSCALE );
   // convert to double precision
   img.convertTo( img, CV_64F );
    

   // Create an LBP instance of type rotation invariant uniform 2
   // using 8 support points

   LBP lbp( 8, LBP_MAPPING_RIU2 );
   // Calculate the descriptor image and get it
   Mat lbpImg = lbp.calcLBP( img ).getLBPImage();

   // Create a mask same size as the image
   Mat mask( lbpImg.rows, lbpImg.cols, CV_8UC1 );

   // Get the histogram for sub-images
   for( int j = 0; j < 2; j++ ) {
      for( int i = 0; i < 2; i++ ) {
         // Reset mask. Will actually not allocate the data as it is
         // 		same size as before.
         mask = Mat::zeros(lbpImg.rows, lbpImg.cols, CV_8UC1);
         // Get a sub-image (ROI) the size of 1/4 of the whole image
         int x = lbpImg.cols / 2 * i;
         int y = lbpImg.rows / 2 * j;
         int w = lbpImg.cols/2-2;
         int h = lbpImg.rows/2-2;

         Mat roi( mask, Range(y,y+h), Range(x,x+w) );
         roi = Scalar(255);

         // Calculate histogram for the ROI
         vector<double> hist = lbp.calcHist( mask ).getHist();

         // Print out the histogram values
         cout << "hist(" << j << "," << i << ") = [";
         for( int i = 0; i < hist.size(); i++ ) {
            cout << hist[i] << ", ";
         }
         cout << "]; " << endl;
      }
   }

}

/** 
 * Calculate a mapping, save it and load it. This is especially useful for 
 *	larger mappings (24 pts) which can takes many seconds to calculate.
 */

void example_3( void ) 
{
   clock_t startTime, endTime;
	
   LBP lbp( 16, LBP_MAPPING_U2 );
   cout << lbp.toString() << endl;
   startTime = clock();
   lbp.saveMapping( "mapping.txt" );
   endTime = clock();
   cout << "save took " 
        << double( endTime - startTime ) / double( CLOCKS_PER_SEC ) << "s"
	<< endl;
	
   LBP lbp2;
   startTime = clock();
   lbp2.loadMapping("mapping.txt");
   endTime = clock();
   cout << lbp2.toString() << endl;
   cout << "load took " 
        << double( endTime - startTime ) / double( CLOCKS_PER_SEC ) << "s"
        << endl;
}

// Load an image, calculate LBP riu2 and then calculate the histogram
// on sub-regions of the image

void example_4( string image_filename ) {
   // Read an (RGB) image and convert to monochrome
   cv::Mat img = imread( image_filename.c_str(), CV_LOAD_IMAGE_GRAYSCALE );
   // convert to double precision
   img.convertTo( img, CV_64F );

   // Create an LBP instance of type rotation invariant uniform 2
   // using 8 support points

//   LBP lbp( 8, LBP_MAPPING_NONE );
   LBP lbp( 8, LBP_MAPPING_RIU2 );
   // Calculate the descriptor image and get it
   Mat lbpImg = lbp.calcLBP( img ).getLBPImage();

   // Create a mask same size as the image
   Mat mask( lbpImg.rows, lbpImg.cols, CV_8UC1 );

   // Get the histogram for sub-images

   for( int j = 0; j < 4; j++ ) 
   {
      for( int i = 0; i < 4; i++ ) 
      {
         // Reset mask. Will actually not allocate the data as it is
         // 		same size as before.

         mask = Mat::zeros(lbpImg.rows, lbpImg.cols, CV_8UC1);
         // Get a sub-image (ROI) the size 1/16th of the whole image
         int x = lbpImg.cols / 4 * i;
         int y = lbpImg.rows / 4 * j;
         int w = lbpImg.cols/4-4;
         int h = lbpImg.rows/4-4;

         Mat roi( mask, Range(y,y+h), Range(x,x+w) );
         roi = Scalar(255);

         // Calculate histogram for the ROI
         vector<double> hist = lbp.calcHist( mask ).getHist();

         // Print out the histogram values
         cout << "hist(" << j << "," << i << ") = [";
         for( int i = 0; i < hist.size(); i++ ) {
            cout << hist[i] << ", ";
         }
         cout << "]; " << endl;
      }
   }

}



int main( int argc, char ** argv ) 
{
   clock_t startTime, endTime;

/*
   startTime = clock();


//   vector<double> descriptor=LBP_HF_descriptor(image_filename);
   
   endTime = clock();
   
cout << "Example took " 
        << double( endTime - startTime ) / double( CLOCKS_PER_SEC ) << "s"
        << endl;
*/

//   example_2();

//   example_3();


   string image_filename="peace_purple.jpg";   
   example_4(image_filename);    

   return 0;
}

