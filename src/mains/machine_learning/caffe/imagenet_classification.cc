// ========================================================================
// Program IMAGENET_CLASSIFICATION is a minor variant of the caffe
// example program classification.cpp.  It queries the user to enter
// the basename for some image file.  IMAGENET_CLASSIFICATION then
// prints out the labels for the top N = 5 classes which the caffe
// model predicts for the input image.
// ========================================================================
// Last updated on 11/18/15; 1/8/16; 1/24/16
// ========================================================================

#include <opencv2/highgui/highgui.hpp>
#include "classification/caffe_classifier.h"

using namespace caffe;  // NOLINT(build/namespaces)
using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

/* Pair (label, confidence) representing a prediction. */
typedef std::pair<string, float> Prediction;

int main(int argc, char** argv) 
{
   if (argc != 5) {
      std::cerr << "Usage: " << argv[0]
                << " deploy.prototxt network.caffemodel"
                << " mean.binaryproto labels.txt" << std::endl;
      return 1;
   }

   ::google::InitGoogleLogging(argv[0]);

   string model_file   = argv[1];
   string trained_file = argv[2];
   string mean_rgb_image_file    = argv[3];
   string label_file   = argv[4];
   caffe_classifier classifier(
     model_file, trained_file, mean_rgb_image_file, label_file);

   while(true)
   {
      string image_basename;
      cout << "Enter input image filename" << endl;
      cin >> image_basename;
      string images_subdir = "./images/";
      string image_filename = images_subdir + image_basename;

      cout << "---------- Prediction for " << image_filename 
           << " ----------" << endl;

      cv::Mat img = cv::imread(image_filename, -1);
      CHECK(!img.empty()) << "Unable to decode image " << image_filename;
      vector<Prediction> predictions = classifier.Classify(img);

      // Print the top N predictions: 

      for (size_t i = 0; i < predictions.size(); ++i) 
      {
         Prediction p = predictions[i];
         cout << std::fixed << std::setprecision(4) << p.second << " - \""
              << p.first << "\"" << endl;
      }

   } // infinite while loop 

}
