// ========================================================================
// Program CLASSIFY_GENDERS is a variant of the CLASSIFY_CHARS.  It
// takes in a caffe model trained from scratch along with folder
// containing a set of input test images.  CLASSIFY_GENDERS loops over
// each test image and prints out the label for the top class which
// the trained caffe model predicts for the input image.  It exports
// correctly and incorrectly classified image chips to separate
// subfolders.

// ./classify_genders 
// /data/caffe/faces/trained_models/test_96.prototxt                     
// /data/caffe/faces/trained_models/Aug2_184K_T3/train_iter_200000.caffemodel 
// /data/caffe/faces/image_chips/testing/Jul30_and_31_96x96

// ========================================================================
// Last updated on 7/30/16; 8/1/16; 8/2/16; 8/3/16
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
//   bool copy_image_chips_flag = true;
   bool copy_image_chips_flag = false;

   bool truth_known_flag = true;
//   bool truth_known_flag = false;

   string test_prototxt_filename   = argv[1];
   string caffe_model_filename = argv[2];
   string input_images_subdir = argv[3];
   filefunc::add_trailing_dir_slash(input_images_subdir);

   string output_chips_subdir = input_images_subdir+"classified_genders/";
   ofstream output_stream;
   string output_text_filename=output_chips_subdir + "gender.classifications";

   if(!truth_known_flag)
   {
      filefunc::dircreate(output_chips_subdir);
      copy_image_chips_flag = true;
      filefunc::openfile(output_text_filename, output_stream);
      output_stream << "# Index   imagefile_basename   gender   score"
                    << endl << endl;
   }
   
   if (argc != 4) {
      cerr << "Usage: " << argv[0]
           << " test.prototxt network.caffemodel input_images_subdir"
           << endl;
      return 1;
   }

   ::google::InitGoogleLogging(argv[0]);

   string cropped_chips_subdir="./cropped_image_chips/";
   filefunc::dircreate(cropped_chips_subdir);
   string classified_chips_subdir="./classified_chips/";
   filefunc::dircreate(classified_chips_subdir);
   string correct_chips_subdir=classified_chips_subdir+"correct/";
   filefunc::dircreate(correct_chips_subdir);

   string incorrect_chips_subdir=classified_chips_subdir+"incorrect/";
   filefunc::dircreate(incorrect_chips_subdir);

   if(copy_image_chips_flag)
   {
      filefunc::purge_files_in_subdir(correct_chips_subdir);
      filefunc::purge_files_in_subdir(incorrect_chips_subdir);
   }

   caffe_classifier classifier(test_prototxt_filename, caffe_model_filename);

//   double Bmean = 114.45;
//   double Gmean = 114.45;
//   double Rmean = 114.45;

   double Bmean = 104.008;
   double Gmean = 116.669;
   double Rmean = 122.675;

   classifier.set_mean_bgr(Bmean, Gmean, Rmean);

   classifier.add_label("background");
   classifier.add_label("male");
   classifier.add_label("female");

   bool search_all_children_dirs_flag = false;
//   bool search_all_children_dirs_flag = true;
   vector<string> image_filenames=filefunc::image_files_in_subdir(
      input_images_subdir, search_all_children_dirs_flag);

   int n_images = image_filenames.size();
   cout << "n_images = " << n_images << endl;
   vector<int> shuffled_image_indices = mathfunc::random_sequence(n_images);

   int istart=0;
   int istop = n_images;
   int n_classes = classifier.get_n_labels();
   vector<double> correct_scores, incorrect_scores;

   genmatrix confusion_matrix(n_classes,n_classes);
   confusion_matrix.clear_matrix_values();

   timefunc::initialize_timeofday_clock();

   for(int i = istart; i < istop; i++)
   {
      outputfunc::update_progress_and_remaining_time(
         i, 200, (istop-istart));

      int image_ID = shuffled_image_indices[i];
      if(!truth_known_flag) image_ID = i;
      
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

//      cout << "image_filename = " << image_filename << endl;
      texture_rectangle curr_image(image_filename, NULL);
      classifier.rgb_img_to_bgr_fvec(curr_image);
      classifier.generate_dense_map();

      int classification_label = classifier.get_classification_result();
      double classification_score = classifier.get_classification_score();
//      cout << "Label:  True = " << true_label 
//           << " Classified = " << classification_label 
//           << " score = " << classification_score 
//           << endl;

      string classified_chip_imagename;
      string unix_cmd;
      string classified_chip_basename=true_gender_label+"_"+
         stringfunc::number_to_string(classification_score)+
         stringfunc::integer_to_string(i,5)+"_"+
         ".jpg";

      if(!truth_known_flag)
      {
         string gender="female";
         if(classification_label == 1)
         {
            gender = "male";
         }
//         classified_chip_basename=gender+"_"+image_basename;
         classified_chip_basename=stringfunc::prefix(image_basename)+"_"+
            gender+"."+stringfunc::suffix(image_basename);
         classified_chip_imagename = output_chips_subdir+
            classified_chip_basename;

         output_stream << i << "   "
                       << image_basename << "   "
                       << gender << "   "
                       << classification_score 
                       << endl;
      }
      else
      {
         if(classification_label == true_label)
         {
            correct_scores.push_back(classification_score);
            classified_chip_imagename = correct_chips_subdir+
               classified_chip_basename;
         }
         else
         {
            incorrect_scores.push_back(classification_score);
            classified_chip_imagename = incorrect_chips_subdir+
               classified_chip_basename;
         }
      
         confusion_matrix.put(
            true_label, classification_label,
            confusion_matrix.get(true_label, classification_label) + 1);

         if(i%200 == 0)
         {
            int n_correct = correct_scores.size();
            int n_incorrect = incorrect_scores.size();
            double frac_correct = double(n_correct)/(n_correct+n_incorrect);
            cout << "n_correct = " << n_correct 
                 << " n_incorrect = " << n_incorrect 
                 << " frac_correct = " << frac_correct
                 << endl;
         }

      } // truth_known_flag conditional

      if(copy_image_chips_flag)
      {
         unix_cmd = "cp "+orig_image_filename+" "+classified_chip_imagename;
         sysfunc::unix_command(unix_cmd);
      }

   } // loop over index i labeling input image tiles

   cout << "test.prototxt = " << test_prototxt_filename << endl;
   cout << "trained caffe model = " << caffe_model_filename << endl;
   cout << "input_images_subdir = " << input_images_subdir << endl;

   if(truth_known_flag)
   {
      prob_distribution prob_correct(correct_scores, 100, 0);
      prob_distribution prob_incorrect(incorrect_scores, 100, 0);

      prob_correct.set_title(
         "Classification scores for correctly classified genders");
      prob_incorrect.set_title(
         "Classification scores for incorrectly classified genders");
      prob_correct.set_xtic(0.2);
      prob_correct.set_xsubtic(0.1);
      prob_correct.set_xlabel("Classification score");
      prob_incorrect.set_xtic(0.2);
      prob_incorrect.set_xsubtic(0.1);
      prob_incorrect.set_xlabel("Classification score");      

      prob_correct.set_densityfilenamestr("correct_dens.meta");
      prob_incorrect.set_densityfilenamestr("incorrect_dens.meta");
      prob_correct.set_cumulativefilenamestr("correct_cum.meta");
      prob_incorrect.set_cumulativefilenamestr("incorrect_cum.meta");
      prob_correct.writeprobdists(false);
      prob_incorrect.writeprobdists(false);

      int n_correct = correct_scores.size();
      int n_incorrect = incorrect_scores.size();
      double frac_correct = double(n_correct)/(n_correct+n_incorrect);
      cout << "n_images = " << n_images << endl;
      cout << "n_correct + n_incorrect = " << n_correct + n_incorrect \
           << endl;
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
   else
   {
      filefunc::closefile(output_text_filename, output_stream);
      string banner="Exported gender classifications to "+
         output_text_filename;
      outputfunc::write_banner(banner);

      banner="Exported classified chips to " + output_chips_subdir;
      outputfunc::write_banner(banner);
   }
}
   
