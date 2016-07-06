/* This is a standalone program. Pass an image name as a first parameter
   of the program.  Switch between standard and probabilistic Hough transform
   by changing "#if 1" to "#if 0" and back */

#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <math.h>
#include <iostream>

using namespace cv;
using std::cin;
using std::cout;
using std::endl;

int main(int argc, char** argv)
{
   Mat src, dst, color_dst;
   if( argc != 2 || !(src=imread(argv[1], 0)).data)
      return -1;

   double lo_threshold=50;
//   double lo_threshold=100;
//   double hi_threshold=200;
   double hi_threshold=250;
   Canny( src, dst, lo_threshold, hi_threshold, 3 );
   cvtColor( dst, color_dst, CV_GRAY2BGR );

#if 0
   vector<Vec2f> lines;
   int hough_threshold=100;
   cout << "Enter hough threshold:" << endl;
   cin >> hough_threshold;

   HoughLines( dst, lines, 1, CV_PI/180, hough_threshold );

   for( size_t i = 0; i < lines.size(); i++ )
   {
      float rho = lines[i][0];
      float theta = lines[i][1];
      double a = cos(theta), b = sin(theta);
      double x0 = a*rho, y0 = b*rho;
      Point pt1(cvRound(x0 + 1000*(-b)),
      cvRound(y0 + 1000*(a)));
      Point pt2(cvRound(x0 - 1000*(-b)),
      cvRound(y0 - 1000*(a)));
      line( color_dst, pt1, pt2, Scalar(0,0,255), 3, 8 );
   }
#else
   vector<Vec4i> lines;
   cout << "Before calling HoughLinesP()" << endl;

   int hough_threshold=80;
   cout << "Enter hough threshold:" << endl;
   cin >> hough_threshold;

   double minLineLength=50;
   double maxLineGap=10;
   
   HoughLinesP( 
      dst, lines, 1, CV_PI/180, hough_threshold, minLineLength, maxLineGap);
   for( size_t i = 0; i < lines.size(); i++ )
   {
      line( color_dst, Point(lines[i][0], lines[i][1]),
      Point(lines[i][2], lines[i][3]), Scalar(0,0,255), 3, 8 );
   }
#endif
   namedWindow( "Source", 1 );
   imshow( "Source", src );

   namedWindow( "Detected Lines", 1 );
   imshow( "Detected Lines", color_dst );

   waitKey(0);
   return 0;
}
