// ========================================================================
// Program ENSEMBLE_CHARS is a variant of CLASSIFY_CHARS.  It takes in
// a multiple finetuned caffe models along with folder containing a
// set of input test images.  ENSEMBLE_CHARS loops over each test
// image and computes the classification label from each caffe model.
// It then averages together the models' results via some reasonable
// method (e.g. straight vote averaging, score averaging, or maximum
// score winner takes all).  ENSEMBLE_CHARS reports the number of
// correctly and incorrectly classified characters as a function of
// the number of trained caffe models within the ensemble average.
// ========================================================================
// Last updated on 2/16/16; 2/19/16; 2/21/16; 2/23/16
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
   if (argc != 5) {
      cerr << "Usage: " << argv[0]
           << " deploy.prototxt "
           << " mean.binaryproto labels.txt input_images_subdir" 
           << endl;
      return 1;
   }

   ::google::InitGoogleLogging(argv[0]);

   string deploy_prototxt_file = argv[1];
   string mean_rgb_image_file = argv[2];
   string label_file = argv[3];
   string input_images_subdir = argv[4];

   vector<string> caffe_model_filenames;
   string caffe_models_subdir="/data/caffe/";
   string digit_models_subdir=caffe_models_subdir+"digits/";
//   caffe_model_filenames.push_back(digit_models_subdir+"test01.caffemodel");
//   caffe_model_filenames.push_back(digit_models_subdir+"test02.caffemodel");
//   caffe_model_filenames.push_back(digit_models_subdir+"test03.caffemodel");
//   caffe_model_filenames.push_back(digit_models_subdir+"test04.caffemodel");
//   caffe_model_filenames.push_back(digit_models_subdir+"test05.caffemodel");
//   caffe_model_filenames.push_back(digit_models_subdir+"test06.caffemodel");
//   caffe_model_filenames.push_back(digit_models_subdir+"test07.caffemodel");
   caffe_model_filenames.push_back(digit_models_subdir+"test08.caffemodel");
   caffe_model_filenames.push_back(digit_models_subdir+"test09.caffemodel");
   caffe_model_filenames.push_back(digit_models_subdir+"test10.caffemodel");
   caffe_model_filenames.push_back(digit_models_subdir+"test11.caffemodel");
   caffe_model_filenames.push_back(digit_models_subdir+"test12.caffemodel");
   caffe_model_filenames.push_back(digit_models_subdir+"test13.caffemodel");
   caffe_model_filenames.push_back(digit_models_subdir+"test14.caffemodel");
   caffe_model_filenames.push_back(digit_models_subdir+"test15.caffemodel");
   caffe_model_filenames.push_back(digit_models_subdir+"test16.caffemodel");
   caffe_model_filenames.push_back(digit_models_subdir+"test17.caffemodel");
   caffe_model_filenames.push_back(digit_models_subdir+"test18.caffemodel");
   caffe_model_filenames.push_back(digit_models_subdir+"test19.caffemodel");

   int n_trained_caffe_models(caffe_model_filenames.size());

//   cout << "deploy.prototxt = " << deploy_prototxt_file << endl;
//   cout << "mean_rgb_image_file = " << mean_rgb_image_file << endl;
//   cout << "label_file = " << label_file << endl;
//   cout << "input_images_subdir = " << input_images_subdir << endl;

// Instantiate ensemble caffe classifiers:

   cout << "-------------------------------------------------------" << endl;
   cout << "Total number of trained caffe models = " 
        << n_trained_caffe_models << endl;

   int n_ensemble_caffe_models;
   cout << "Enter number of caffe models to use within ensemble:" << endl;
   cin >> n_ensemble_caffe_models;
   if(n_ensemble_caffe_models > n_trained_caffe_models)
   {
      exit(-1);
   }

   vector<caffe_classifier*> caffe_classifier_ptrs;
   for(int c = 0; c < n_trained_caffe_models; c++)
   {
      cout << "Loading caffe model c = " << c 
           << " model filename = " << caffe_model_filenames[c] << endl;
      caffe_classifier* classifier_ptr = new caffe_classifier(
         deploy_prototxt_file, caffe_model_filenames[c], mean_rgb_image_file, 
         label_file);
      caffe_classifier_ptrs.push_back(classifier_ptr);
   }

// Save labels from label_file:

   filefunc::ReadInfile(label_file);
   vector<string> class_labels;
   for(unsigned int i = 0; i < filefunc::text_line.size(); i++)
   {
      class_labels.push_back(filefunc::text_line[i]);
   }

   string extended_chips_subdir="./extended_chips/";
   filefunc::dircreate(extended_chips_subdir);

   int n_classes = caffe_classifier_ptrs[0]->get_n_labels();
   genmatrix confusion_matrix(n_classes,n_classes);
   confusion_matrix.clear_matrix_values();

   bool search_all_children_dirs_flag = true;
   vector<string> image_filenames=filefunc::image_files_in_subdir(
      input_images_subdir, search_all_children_dirs_flag);

   int n_images = image_filenames.size();
   vector<int> shuffled_image_indices = mathfunc::random_sequence(n_images);

   int istart=0;
   int istop = n_images;
   int n_correct = 0, n_incorrect = 0;
   string unix_cmd;
   timefunc::initialize_timeofday_clock();
   const int i_print = 50;

// -------------------------------------------------------------------
// Main loop over input test images starts here:

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

// For each image, perform multiple iterations where we randomly
// extract n_ensemble_caffe_models from all the loaded
// n_trained_caffe_models to use for inference:

      int n_iters = basic_math::min(3, n_trained_caffe_models);
      for(int iter = 0; iter < n_iters; iter++)
      {
         vector<int> caffe_models_ID = mathfunc::random_sequence(
            n_trained_caffe_models, n_ensemble_caffe_models);

// Initialize class_IDs and class_scores STL vectors for each
// iteration:

         vector<int> class_IDs;
         vector<double> class_scores;
         for(int i = 0; i < n_classes; i++)
         {
            class_IDs.push_back(i);
            class_scores.push_back(0);
         }

         vector<int> ensemble_classified_labels;
         for(int c = 0; c < n_ensemble_caffe_models; c++)
         {
            int model_ID = caffe_models_ID[c];
         
            vector<Prediction> curr_predictions = 
               caffe_classifier_ptrs[model_ID]->Classify(img);

            int curr_classified_label = -1;
            Prediction p = curr_predictions[0];
            if(stringfunc::is_number(p.first))
            {
               curr_classified_label = stringfunc::string_to_integer(p.first);
            }

            ensemble_classified_labels.push_back(curr_classified_label);
         
//         cout << " c=" << c 
//              << " label=" << curr_classified_label;

//            class_scores[curr_classified_label+1] = 
//               basic_math::max(
//                  class_scores[curr_classified_label+1], double(p.second));
//			(winner takes all)


//         class_scores[curr_classified_label+1] = 
//            class_scores[curr_classified_label+1] * p.second; 
// 			(geometric mean)
         
          class_scores[curr_classified_label+1] = 
         class_scores[curr_classified_label+1] + p.second;
// 			(arithmetic mean)

//         class_scores[curr_classified_label+1] = 
//            class_scores[curr_classified_label+1] + 1;
//			(simple voting)

         } // loop over index c labeling trained caffe models

         templatefunc::Quicksort_descending(class_scores, class_IDs);
      
         int best_class_ID = class_IDs[0];
         int best_classified_label = best_class_ID - 1;
//      cout << "    best_label = " << best_classified_label << endl;

         string orig_image_basename=filefunc::getbasename(orig_image_filename);
         string image_suffix = stringfunc::suffix(orig_image_filename);

         if(best_classified_label == true_label)
         {
            n_correct++;
         }
         else
         {
            n_incorrect++;
         }

         confusion_matrix.put(
            true_label + 1, best_classified_label + 1,
            confusion_matrix.get(true_label + 1, best_classified_label + 1) 
            + 1);

         if(i%i_print == 0 || i > istop - 5)
         {
            cout << "Processing image " << (i-istart) 
                 << " of " << (istop-istart) << endl;
            cout << "Original image filename = " << orig_image_filename 
                 << endl;

            double frac_correct = double(n_correct)/(n_correct+n_incorrect);

            cout << "Number of ensemble caffe models = " 
                 << n_ensemble_caffe_models << endl;
            for(int m = 0; m < n_ensemble_caffe_models; m++)
            {
               cout << "m = " << m << " caffe_model_ID = " 
                    << caffe_models_ID[m] << endl;
            }
            
            cout << "n_correct = " << n_correct 
                 << " n_incorrect = " << n_incorrect 
                 << " frac_correct = " << frac_correct
                 << endl;
            cout << "Confusion matrix:" << endl;
            cout << confusion_matrix << endl;
         }
         outputfunc::update_progress_and_remaining_time(
            i, i_print, (istop-istart));
  
      } // loop over iter index labeling random sequence of caffe models
	// in current ensemble
    
   } // loop over index i labeling input image tiles
// -------------------------------------------------------------------

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

   string banner="Number of trained models within ensemble = "+
      stringfunc::number_to_string(n_ensemble_caffe_models);
   outputfunc::write_banner(banner);
}
   
