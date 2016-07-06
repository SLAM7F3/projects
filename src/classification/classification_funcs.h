// ==========================================================================
// Header file for stand-alone classification methods
// ==========================================================================
// Last updated on 3/11/12; 4/18/12
// ==========================================================================

#ifndef CLASSIFICATION_H
#define CLASSIFICATION_H

#include <vector>
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "math/threevector.h"

using namespace cv;

namespace classification_func
{

   void detectAndDraw( 
      Mat& img,CascadeClassifier& cascade,CascadeClassifier& nestedCascade,
      double scale);
   void detectAndDraw( 
      Mat& img,CascadeClassifier& cascade,CascadeClassifier& nestedCascade,
      double scale,std::vector<threevector>& face_circles);
   std::vector<threevector> detectFaces(
      std::string image_filename,CascadeClassifier& cascade,
      CascadeClassifier& nestedCascade,double scale);

   std::vector<threevector> detect_faces(std::string image_filename);
   

// ==========================================================================
// Inlined methods:
// ==========================================================================
   
} // classification_func namespace

#endif  // classification_funcs.h
