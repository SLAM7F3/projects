// ========================================================================
// Program CLASSIFY_CHARS is a variant of the caffe example program
// classification.cpp.  It takes in a finetuned caffe model along with
// folder containing a set of input test images.  CLASSIFY_CHARS loops
// over each test image and prints out the labels for the top N = 5
// classes which the finetuned caffe model predicts for the input
// image.  It exports correctly and incorrectly classified image chips
// to separate subfolders.  
// ========================================================================
// Last updated on 2/4/16; 2/11/16; 2/12/16; 2/23/16
// ========================================================================

#include <opencv2/highgui/highgui.hpp>
#include "classification/caffe_classifier.h"
#include "general/filefuncs.h"
#include "math/genmatrix.h"
#include "image/imagefuncs.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
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
   if (argc != 6) {
      cerr << "Usage: " << argv[0]
           << " deploy.prototxt network.caffemodel"
           << " mean.binaryproto labels.txt input_images_subdir" 
           << endl;
      return 1;
   }

   ::google::InitGoogleLogging(argv[0]);

   string deploy_prototxt_filename   = argv[1];
   string caffe_model_filename = argv[2];
   string mean_rgb_image_file    = argv[3];
   string label_file   = argv[4];
   string input_images_subdir = argv[5];

   cout << "deploy.prototxt = " << deploy_prototxt_filename << endl;
   cout << "trained caffe model = " << caffe_model_filename << endl;
   cout << "mean_rgb_image_file = " << mean_rgb_image_file << endl;
   cout << "label_file = " << label_file << endl;
   cout << "input_images_subdir = " << input_images_subdir << endl;

// Save labels from label_file:

   filefunc::ReadInfile(label_file);
   vector<string> class_labels;
   for(unsigned int i = 0; i < filefunc::text_line.size(); i++)
   {
      class_labels.push_back(filefunc::text_line[i]);
   }

   string extended_chips_subdir="./extended_chips/";
   filefunc::dircreate(extended_chips_subdir);
   string classified_chips_subdir="./classified_chips/";
   filefunc::dircreate(classified_chips_subdir);

   string correct_chips_subdir=classified_chips_subdir+"correct/";
   filefunc::dircreate(correct_chips_subdir);
   filefunc::purge_files_in_subdir(correct_chips_subdir);

   string incorrect_chips_subdir=classified_chips_subdir+"incorrect/";
   filefunc::dircreate(incorrect_chips_subdir);
   filefunc::purge_files_in_subdir(incorrect_chips_subdir);

   string hard_negative_chips_subdir=classified_chips_subdir
      +"hard_negatives/";
   filefunc::dircreate(hard_negative_chips_subdir);
   filefunc::purge_files_in_subdir(hard_negative_chips_subdir);

   caffe_classifier classifier(
      deploy_prototxt_filename, caffe_model_filename, mean_rgb_image_file, 
      label_file);

   bool search_all_children_dirs_flag = true;
   vector<string> image_filenames=filefunc::image_files_in_subdir(
      input_images_subdir, search_all_children_dirs_flag);

   int n_images = image_filenames.size();
   vector<int> shuffled_image_indices = mathfunc::random_sequence(n_images);

   int istart=0;
   int istop = n_images;
   int n_correct = 0, n_incorrect = 0;
   string unix_cmd;

   int n_classes = classifier.get_n_labels();
   genmatrix confusion_matrix(n_classes,n_classes);
   confusion_matrix.clear_matrix_values();

//   vector<double> first_scores, second_scores;
//   vector<double> score_ratios, score_deltas;
//   string scores_filename="scores.dat";
//   ofstream outstream;
//   filefunc::openfile(scores_filename, outstream);
//   outstream << "# Dominant label score    Runner-up label score   Correct classification flag" << endl << endl;

   timefunc::initialize_timeofday_clock();
   const int i_print = 50;
   for(int i = istart; i < istop; i++)
   {
      int image_ID = shuffled_image_indices[i];
      string orig_image_filename = image_filenames[image_ID];

      string image_filename = orig_image_filename;
      string image_basename = filefunc::getbasename(image_filename);
      
      vector<string> substrings = 
         stringfunc::decompose_string_into_substrings(
            image_basename,"_");

      int true_label = -1;
      string char_substr = substrings[0];

// As of 2/4/16, mnist test image chips have basenames of the form
// mnist_D_XXXX.jpg where D = 0, 1, 2, ..., 9:

      if(substrings[0] == "mnist")
      {
         char_substr = substrings[1];
      }

      if(stringfunc::is_number(char_substr))
      {
         true_label = stringfunc::string_to_number(char_substr);
      }

      unsigned int input_img_width, input_img_height;
      imagefunc::get_image_width_height(
         orig_image_filename, input_img_width, input_img_height);

// Ignore any input image chip which is too small:
         
      if(input_img_width < 16) continue;

      if(input_img_width > 224 || input_img_height > 224)
      {
         string id_str = stringfunc::integer_to_string(i, 3);
         image_filename="cropped_image_"+id_str+".jpg";
         imagefunc::crop_image(
            orig_image_filename,image_filename, 224, 224, 16, 16);
      }
      else
      {
         image_filename=extended_chips_subdir
            +stringfunc::prefix(image_basename)+"_256."
            +stringfunc::suffix(image_basename);
         unix_cmd="convert -size 256x256 xc:black ";
         unix_cmd += orig_image_filename;
         unix_cmd += " -gravity center -composite ";
         unix_cmd += image_filename;
         sysfunc::unix_command(unix_cmd);
      }
      
      cv::Mat img = cv::imread(image_filename, -1);
      CHECK(!img.empty()) << "Unable to decode image " << image_filename;
      vector<Prediction> predictions = classifier.Classify(img);

      int classified_label = -1;

      Prediction p = predictions[0];
      Prediction q = predictions[1];
      double first_score = -1;
      double second_score = -1;
      if(stringfunc::is_number(p.first))
      {

// Only accept classifier's result if its score exceeds some
// threshold value.  Otherwise, we accept the default
// label -1 which corresponds to "non-character":

         first_score = p.second;
         classified_label = stringfunc::string_to_integer(p.first);
         if(stringfunc::is_number(q.first))
         {
            second_score = q.second;
         }
      }

      string orig_image_basename=filefunc::getbasename(orig_image_filename);
      string image_suffix = stringfunc::suffix(orig_image_filename);

      if(classified_label == true_label)
      {
         n_correct++;
         unix_cmd = "cp "+orig_image_filename+" "+correct_chips_subdir;

//         if(first_score > 0)
//         {
//            first_scores.push_back(first_score);
//            second_scores.push_back(second_score);
//            score_ratios.push_back(second_score / first_score);
//            score_deltas.push_back(first_score - second_score);
//            outstream << first_score << "   " << second_score 
//                      << "   1" << endl;
//         }
      }
      else
      {
         n_incorrect++;
         string incorrectly_classified_filename = incorrect_chips_subdir+
            orig_image_basename+"__"+stringfunc::number_to_string(
               classified_label)
            +"."+image_suffix;
         
         unix_cmd = "cp "+orig_image_filename+" "+
            incorrectly_classified_filename+";";
//         unix_cmd += "mv "+image_filename+" "+
//            hard_negative_chips_subdir;
      }
      sysfunc::unix_command(unix_cmd);

      confusion_matrix.put(
         true_label + 1, classified_label + 1,
         confusion_matrix.get(true_label + 1, classified_label + 1) + 1);

      if(i%i_print == 0 || i > istop - 5)
      {
         cout << "Processing image " << (i-istart) 
              << " of " << (istop-istart) << endl;
         cout << "Original image filename = " << orig_image_filename << endl;

         cout << "image width = " << input_img_width
              << " image height = " << input_img_height << endl;
         cout << "---------- Prediction for " << image_filename 
              << " ----------" << endl;

         // Print the top N predictions: 

         for (size_t q = 0; q < predictions.size(); q++) 
         {
            Prediction p = predictions[q];
            cout << std::fixed << std::setprecision(4) << p.second << " - \""
                 << p.first << "\"" << endl;
         }
         double frac_correct = double(n_correct)/(n_correct+n_incorrect);

         cout << "n_correct = " << n_correct 
              << " n_incorrect = " << n_incorrect 
              << " frac_correct = " << frac_correct
              << endl;
         cout << "Confusion matrix:" << endl;
         cout << confusion_matrix << endl << endl;
      }
      outputfunc::update_progress_and_remaining_time(
         i, i_print, (istop-istart));
      
   } // loop over index i labeling input image tiles

// Loop over rows within confusion matrix corresponding to true
// labels.  Compute sums of incorrectly classified labels (columns in
// confusion matrix)

   for(unsigned r = 0; r < confusion_matrix.get_mdim(); r++)
   {
      int n_correct_classifications = 0;
      int n_incorrect_classifications = 0;
      for(unsigned c = 0; c < confusion_matrix.get_ndim(); c++)
      {
         if (c == r)
         {
            n_correct_classifications += confusion_matrix.get(r,c);
         }
         else
         {
            n_incorrect_classifications += confusion_matrix.get(r,c);
         }
      }
      double frac_incorrect = double(n_incorrect_classifications)/
         (n_correct_classifications + n_incorrect_classifications);
      cout << "r = " << r 
           << " class label = " << class_labels[r] 
           << "  n_incorrect_classifications = " 
           << n_incorrect_classifications
           << " frac_incorrect = " << frac_incorrect
           << endl;
   }

   string banner="Exported incorrectly classified chips to "+
      incorrect_chips_subdir;
   outputfunc::write_banner(banner);

//   filefunc::closefile(scores_filename, outstream);
}
   
