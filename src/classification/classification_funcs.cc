// ==========================================================================
// Stand-alone classification methods
// ==========================================================================
// Last updated on 3/11/12; 4/18/12; 10/5/13; 6/7/14
// ==========================================================================

#include <iostream>
#include "classification/classification_funcs.h"
#include "general/filefuncs.h"
#include "video/G99VideoDisplay.h"
#include "image/imagefuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

namespace classification_func
{

// --------------------------------------------------------------------------
// Method detectAndDraw() is a slightly modified version of OpenCV
// example program facedetect.cpp.  It returns circles around detected
// faces within output STL vector face_circles.

   void detectAndDraw( 
      Mat& img,CascadeClassifier& cascade, 
      CascadeClassifier& nestedCascade,double scale)
   {
      vector<threevector> face_circles;
      detectAndDraw(img,cascade,nestedCascade,scale,face_circles);
   }

   void detectAndDraw( 
      Mat& img,CascadeClassifier& cascade, 
      CascadeClassifier& nestedCascade,double scale,
      vector<threevector>& face_circles)
   {
      int i = 0;
      double t = 0;
      vector<Rect> faces;
      const static Scalar colors[] =  { CV_RGB(0,0,255),
                                        CV_RGB(0,128,255),
                                        CV_RGB(0,255,255),
                                        CV_RGB(0,255,0),
                                        CV_RGB(255,128,0),
                                        CV_RGB(255,255,0),
                                        CV_RGB(255,0,0),
                                        CV_RGB(255,0,255)} ;
      Mat gray, smallImg( cvRound (img.rows/scale), cvRound(img.cols/scale), 
	CV_8UC1 );

      cvtColor( img, gray, CV_BGR2GRAY );
      resize( gray, smallImg, smallImg.size(), 0, 0, INTER_LINEAR );
      equalizeHist( smallImg, smallImg );

      t = (double)cvGetTickCount();
      cascade.detectMultiScale( smallImg, faces,
      1.1, 2, 0
      //|CV_HAAR_FIND_BIGGEST_OBJECT
      //|CV_HAAR_DO_ROUGH_SEARCH
      |CV_HAAR_SCALE_IMAGE
      ,
      Size(30, 30) );
      t = (double)cvGetTickCount() - t;
//       double detection_time=t/((double)cvGetTickFrequency()*1000);
//      cout << "Detection time = " << detection_time << " msecs" << endl;

      int n_faces_detected=0;
      for( vector<Rect>::const_iterator r = faces.begin(); r != faces.end(); 
           r++, i++ )
      {
         Mat smallImgROI;
         vector<Rect> nestedObjects;
         Point center;
         Scalar color = colors[i%8];
         int radius;
         center.x = cvRound((r->x + r->width*0.5)*scale);
         center.y = cvRound((r->y + r->height*0.5)*scale);
         radius = cvRound((r->width + r->height)*0.25*scale);
         circle( img, center, radius, color, 3, 8, 0 );
         n_faces_detected++;

         threevector curr_face_circle(center.x,center.y,radius);
         face_circles.push_back(curr_face_circle);

         if( nestedCascade.empty() )
            continue;
         smallImgROI = smallImg(*r);
         nestedCascade.detectMultiScale( smallImgROI, nestedObjects,
         1.1, 2, 0
         //|CV_HAAR_FIND_BIGGEST_OBJECT
         //|CV_HAAR_DO_ROUGH_SEARCH
         //|CV_HAAR_DO_CANNY_PRUNING
         |CV_HAAR_SCALE_IMAGE
         ,
         Size(30, 30) );
         for( vector<Rect>::const_iterator nr = nestedObjects.begin(); 
              nr != nestedObjects.end(); nr++ )
         {
            center.x = cvRound((r->x + nr->x + nr->width*0.5)*scale);
            center.y = cvRound((r->y + nr->y + nr->height*0.5)*scale);
            radius = cvRound((nr->width + nr->height)*0.25*scale);
            circle( img, center, radius, color, 3, 8, 0 );

         }
      }
//      cout << "Number of detected faces = " << face_circles.size() << endl;
      cv::imshow( "result", img );
   }

// --------------------------------------------------------------------------
// Method detectFaces() is a high-level subroutine which takes in the
// file name of an image and returns an STL vector containing circles
// enclosing detected human faces.

   vector<threevector> detectFaces(
      string image_filename,CascadeClassifier& cascade,
      CascadeClassifier& nestedCascade,double scale)
   {
//      cout << "Processing image file " << image_filename << endl;

      vector<threevector> face_circles;
      Mat image = imread( image_filename.c_str(), 1 );
      if( !image.empty() )
      {
         classification_func::detectAndDraw( 
            image,cascade,nestedCascade,scale,face_circles);
      }

      return face_circles;
   }

// --------------------------------------------------------------------------
// Method detect_faces() takes in the file name for some image and
// returns an STL vector containing circles enclosing detected human
// faces.  It calls the face detection binary by Z. Kalal, J. Matas,
// and K. Mikolajczyk (see
// http://kahlan.eps.surrey.ac.uk/featurespace/web/face.html).
// Empirically derived threshold on "cornerness" as function of face
// radius helps to eliminate false alarms.  As of 4/18/2012, we
// believe this face detector works significantly better than the
// default one which comes with OpenCV.

   vector<threevector> detect_faces(string image_filename)
   {
      cout << "inside classificationfunc::detect_faces()" << endl;
//      cout << "Processing image file " << image_filename << endl;

        bool draw_face_circles_flag=false;
//      bool draw_face_circles_flag=true;
      vector<threevector> face_circles;

      string metadata_filename="/tmp/metadata.fmax";
      filefunc::deletefile(metadata_filename);
      
      string unix_cmd=
         "/usr/local/face_detection/compute_descriptors_64bit.ln -fface ";
      unix_cmd += "/usr/local/face_detection/frontal.dat -i ";
      unix_cmd += image_filename+" -o2 "+metadata_filename;
      if (draw_face_circles_flag) unix_cmd += " -DC";

      int status=sysfunc::unix_command(unix_cmd);
      if (status==127 || status==-1)
      {
         cout << "Trouble: status = " << status << endl;
      }

      if (!filefunc::fileexist(metadata_filename))
      {
         return face_circles;
      }

      filefunc::ReadInfile(metadata_filename);
      int line_number=0;
      int n_candidate_faces=stringfunc::string_to_number(
         filefunc::text_line[line_number++]);
      line_number++;
      line_number++;

      unsigned int width,height;
      imagefunc::get_image_width_height(image_filename,width,height);
      double max_u=double(width)/double(height);

      texture_rectangle* texture_rectangle_ptr=NULL;
      if (draw_face_circles_flag)
      {
         texture_rectangle_ptr=new texture_rectangle(image_filename,NULL);
      }
      
// Power-law params as of Weds, Apr 18 early afternoon

// r=0.01   --> threshold=9.68
// r=0.025  --> threshold=7.5
// r=0.05  --> threshold=4.9
// r=0.1  --> threshold=2.09  
// r=0.1667 --> threshold=3.7

// More strict Power-law params as of Weds, Apr 18 evening

// alpha=13.4695; beta=-11.5525

// r=0.01  ==> THRESH=12
// r=0.02  ==> THRESH=10.69
// r=0.03  ==> THRESH=9.52
// r=0.04  ==> THRESH=8.48
// r=0.05  ==> THRESH=7.56
// r=0.06  ==> THRESH=6.73
// r=0.07  ==> THRESH=6

// alpha=26.0108; beta=-20.9536

// r=0.07 ==> THRESH=6
// r=0.08 ==> THRESH=4.87
// r=0.09 ==> THRESH=3.95
// r=0.1  ==> THRESH=3.2

      bool face_detected_flag=false;
      for (int f=0; f<n_candidate_faces; f++)
      {
         vector<double> column_values=stringfunc::string_to_numbers(
            filefunc::text_line[line_number++]);
         int px=column_values[2];
         int py=column_values[3];
         double cornerness=column_values[4];
         double scale=column_values[5];
         double radius=double(scale)/double(height);

         double cornerness_threshold;
         if (radius > 0.1)	 // linear
         {
            double rmin=0.1;
            double rmax=0.1667;
//            double tmin=2.09;
//            double tmax=3.7;
            double tmin=3.2;
            double tmax=4.5;
            cornerness_threshold=tmin+(tmax-tmin)*(radius-rmin)/(
               rmax-rmin);
         }
         else if (radius > 0.07 && radius <= 0.1)
         {
            double alpha=26.0108;
            double beta=-20.9536;
            cornerness_threshold=alpha*exp(beta*radius);
         }
         else			// power-law for radius < 0.07
         {
//            const double alpha=11.4796;
//            const double beta=-17.0267;
            double alpha=13.4695;
            double beta=-11.5525;
            cornerness_threshold=alpha*exp(beta*radius);
         }

/*
         if (f < 5)
         {
            cout << "f = " << f
                 << " radius = " << radius
                 << " cornerness = " << cornerness 
                 << " threshold = " << cornerness_threshold
                 << endl;
         }
*/

         double rmin=0.01;
         double rmax=0.5;
         if (radius < rmin || radius > rmax) continue;

         if (cornerness < cornerness_threshold) continue;

         double u=double(px)/double(height);
         double v=1-double(py)/double(height);

         threevector curr_face_circle(u,v,radius);
         face_circles.push_back(curr_face_circle);

         face_detected_flag=true;
         cout << "Face detected in image " 
              << filefunc::getbasename(image_filename) << endl;
         cout << "  U = " << u 
              << " V = " << v 
              << " radius = " << radius 
              << " cornerness = " << cornerness
              << " cornerness_threshold = " << cornerness_threshold
              << endl;

         if (!draw_face_circles_flag) continue;

         int n_circles=4;
         double theta_start=0;
         double theta_stop=n_circles*2*PI;
         double dtheta=(0.5)*PI/180;
//         double dtheta=(1+1.0/double(n_circles))*PI/180;
         int n_thetabins=(theta_stop-theta_start)/dtheta+1;
         for (int t=0; t<n_circles*n_thetabins; t++)
         {
            double theta=theta_start+t*dtheta;
            double curr_u=u+radius*cos(theta);
            double curr_v=v+radius*sin(theta);
            if (curr_u < 0 || curr_u > max_u) continue;
            if (curr_v < 0 || curr_v > 1) continue;

	    int R=255;
	    int G=0;
	    int B=255;
            texture_rectangle_ptr->set_RGB_values(curr_u,curr_v,R,G,B);

//            cout << "t = " << t
//                 << " curr_u = " << curr_u
//                 << " curr_v = " << curr_v << endl;
            
         } // loop over index t labeling theta bins

      } // loop over index f labeling candidate faces

      if (draw_face_circles_flag && face_detected_flag)
      {
         string output_filename="/tmp/faces.jpg";
         texture_rectangle_ptr->write_curr_frame(output_filename);
         string banner="Exported "+output_filename;
         outputfunc::write_banner(banner);
         outputfunc::enter_continue_char();
      }
      
      delete texture_rectangle_ptr;

      return face_circles;
   }
   
} // classification_func namespace

