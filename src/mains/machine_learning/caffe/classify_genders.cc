// ========================================================================
// Program CLASSIFY_GENDERS is a variant of the CLASSIFY_CHARS.  It
// takes in a caffe model trained from scratch along with folder
// containing a set of input test images.  CLASSIFY_GENDERS loops over
// each test image and prints out the label for the top class which
// the trained caffe model predicts for the input image.  It exports
// correctly and incorrectly classified image chips to separate
// subfolders.
// ========================================================================
// Last updated on 2/12/16; 2/23/16; 7/30/16; 8/1/16
// ========================================================================

#include <opencv2/highgui/highgui.hpp>
#include "classification/caffe_classifier.h"
#include "general/filefuncs.h"
#include "math/genmatrix.h"
#include "image/imagefuncs.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"

using namespace caffe;  // NOLINT(build/namespaces)
using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

// Pair (label, confidence) representing a prediction:

typedef pair<string, float> Prediction;

int main(int argc, char** argv) 
{
   if (argc != 4) {
      cerr << "Usage: " << argv[0]
           << " test.prototxt network.caffemodel input_images_subdir"
           << endl;
      return 1;
   }

   ::google::InitGoogleLogging(argv[0]);

   string test_prototxt_filename   = argv[1];
   string caffe_model_filename = argv[2];
   string input_images_subdir = argv[3];

   cout << "test.prototxt = " << test_prototxt_filename << endl;
   cout << "trained caffe model = " << caffe_model_filename << endl;
   cout << "input_images_subdir = " << input_images_subdir << endl;

   string caffe_data_subdir = "/data/caffe/";
   string faces_subdir = caffe_data_subdir + "faces/";
   string image_chips_subdir = faces_subdir + "image_chips/";
   string validation_chips_subdir = image_chips_subdir + "validation/";
   string testing_chips_subdir = image_chips_subdir + "testing/";
   
   string dated_subdir = image_chips_subdir + "Jul29_106x106_augmented/";

   string cropped_chips_subdir="./cropped_image_chips/";
   filefunc::dircreate(cropped_chips_subdir);
   string classified_chips_subdir="./classified_chips/";
   filefunc::dircreate(classified_chips_subdir);
   string correct_chips_subdir=classified_chips_subdir+"correct/";
   filefunc::dircreate(correct_chips_subdir);
   filefunc::purge_files_in_subdir(correct_chips_subdir);
   string incorrect_chips_subdir=classified_chips_subdir+"incorrect/";
   filefunc::dircreate(incorrect_chips_subdir);
   filefunc::purge_files_in_subdir(incorrect_chips_subdir);

   caffe_classifier classifier(test_prototxt_filename, caffe_model_filename);

   double Bmean = 114.45;
   double Gmean = 114.45;
   double Rmean = 114.45;

//   double Bmean = 104.008;
//   double Gmean = 116.669;
//   double Rmean = 122.675;

//   double Rmean = 104.008;
//   double Gmean = 116.669;
//   double Bmean = 122.675;
   classifier.set_mean_bgr(Bmean, Gmean, Rmean);

   classifier.add_label("background");
   classifier.add_label("male");
   classifier.add_label("female");

   bool search_all_children_dirs_flag = true;
   vector<string> image_filenames=filefunc::image_files_in_subdir(
      input_images_subdir, search_all_children_dirs_flag);

   int n_images = image_filenames.size();
   cout << "n_images = " << n_images << endl;
   vector<int> shuffled_image_indices = mathfunc::random_sequence(n_images);

   int istart=0;
   int istop = n_images;
   int n_correct = 0, n_incorrect = 0;

   int n_classes = classifier.get_n_labels();
   genmatrix confusion_matrix(n_classes,n_classes);
   confusion_matrix.clear_matrix_values();

   timefunc::initialize_timeofday_clock();

   for(int i = istart; i < istop; i++)
   {
      outputfunc::update_progress_and_remaining_time(
         i, 200, (istop-istart));

      int image_ID = shuffled_image_indices[i];
      string orig_image_filename = image_filenames[image_ID];
      string image_filename = orig_image_filename;
      string image_basename = filefunc::getbasename(image_filename);
//      cout << "i = " << i << " image_basename = " << image_basename << endl;
      
      vector<string> substrings = 
         stringfunc::decompose_string_into_substrings(image_basename,"_");

      int true_label = -1;
      string true_gender_label = substrings[0];
      if(true_gender_label == "background")
      {
         true_label = 0;
      }
      else if(true_gender_label == "male")
      {
         true_label = 1;
      }
      else if(true_gender_label == "female")
      {
         true_label = 2;
      }

      unsigned int input_img_width, input_img_height;
      imagefunc::get_image_width_height(
         orig_image_filename, input_img_width, input_img_height);

      if(input_img_width != 96 || input_img_height != 96)
      {
         cout << "Error" << endl;
         cout << "input_img_width = " << input_img_width << endl;
         cout << "input_img_height = " << input_img_height << endl;
      }

/*
      if(input_img_width > 96 || input_img_height > 96)
      {
         string id_str = stringfunc::integer_to_string(i, 3);
         image_filename=cropped_chips_subdir+"cropped_image_"+id_str+".jpg";
         int xoffset = (input_img_width - 96)/2;
         int yoffset = (input_img_height - 96)/2;
         imagefunc::crop_image(
            orig_image_filename,image_filename, 96, 96, xoffset, yoffset);
      }
      else
      {
//         cout << "input_image_width = " << input_img_width
//              << " input_image_height = " << input_img_height << endl;
      }
*/

      texture_rectangle curr_image(image_filename, NULL);
      classifier.rgb_img_to_bgr_fvec(curr_image);
      classifier.generate_dense_map();

      int classification_label = classifier.get_classification_result();
      double classification_score = classifier.get_classification_score();
//      cout << "Label:  True = " << true_label 
//           << " Classified = " << classification_label 
//           << " score = " << classification_score 
//           << endl;

      string classified_chip_basename=true_gender_label+"_"+
         stringfunc::number_to_string(classification_score)+".jpg";
      string classified_chip_imagename;
      string unix_cmd;
      if(classification_label == true_label)
      {
         n_correct++;
         classified_chip_imagename = correct_chips_subdir+
            classified_chip_basename;
      }
      else
      {
         n_incorrect++;
         classified_chip_imagename = incorrect_chips_subdir+
            classified_chip_basename;
      }

      unix_cmd = "cp "+orig_image_filename+" "+classified_chip_imagename;
//      sysfunc::unix_command(unix_cmd);

      confusion_matrix.put(
         true_label, classification_label,
         confusion_matrix.get(true_label, classification_label) + 1);

      if(i%200 == 0)
      {
         double frac_correct = double(n_correct)/(n_correct+n_incorrect);
         cout << "n_correct = " << n_correct 
              << " n_incorrect = " << n_incorrect 
              << " frac_correct = " << frac_correct
              << endl;
      }

   } // loop over index i labeling input image tiles

   double frac_correct = double(n_correct)/(n_correct+n_incorrect);
   cout << "n_correct = " << n_correct 
        << " n_incorrect = " << n_incorrect 
        << " frac_correct = " << frac_correct
        << endl;

   cout << "Confusion matrix:" << endl;
   cout << confusion_matrix << endl << endl;

   string banner="Exported correctly classified chips to "+
      correct_chips_subdir;
   outputfunc::write_banner(banner);
   banner="Exported incorrectly classified chips to "+
      incorrect_chips_subdir;
   outputfunc::write_banner(banner);
}
   
