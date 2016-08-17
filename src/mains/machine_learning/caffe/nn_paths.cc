// ========================================================================
// Program NN_PATHS 

// ./nn_paths 
// /data/caffe/faces/trained_models/test_96.prototxt                     
// /data/caffe/faces/trained_models/Aug2_184K_T3/train_iter_200000.caffemodel 
// /data/caffe/faces/image_chips/testing/Jul30_and_31_96x96

// ========================================================================
// Last updated on 8/16/16
// ========================================================================

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

int main(int argc, char** argv) 
{
   timefunc::initialize_timeofday_clock();

   string test_prototxt_filename   = argv[1];
   string caffe_model_filename = argv[2];
   caffe_classifier classifier(test_prototxt_filename, caffe_model_filename);

   string input_images_subdir = argv[3];
   filefunc::add_trailing_dir_slash(input_images_subdir);

   double Bmean = 104.008;
   double Gmean = 116.669;
   double Rmean = 122.675;
   classifier.set_mean_bgr(Bmean, Gmean, Rmean);

   classifier.add_label("non");
   classifier.add_label("male");
   classifier.add_label("female");
   classifier.add_label("unsure");
   int unsure_label = classifier.get_n_labels() - 1;

   int n_classes = classifier.get_n_labels();

   bool search_all_children_dirs_flag = false;
//   bool search_all_children_dirs_flag = true;
   vector<string> image_filenames=filefunc::image_files_in_subdir(
      input_images_subdir, search_all_children_dirs_flag);

   int n_images = image_filenames.size();
   cout << "n_images = " << n_images << endl;
   vector<int> shuffled_image_indices = mathfunc::random_sequence(n_images);


   int istart=0;
   int istop = n_images;
   for(int i = istart; i < istop; i++)
   {
      outputfunc::update_progress_and_remaining_time(
         i, 500, (istop-istart));
      int image_ID = shuffled_image_indices[i];
      
      string orig_image_filename = image_filenames[image_ID];
      string image_filename = orig_image_filename;
      string image_basename = filefunc::getbasename(image_filename);
      cout << "i = " << i 
//           << " image_basename = " << image_basename 
           << " image_filename = " << image_filename
           << endl;
      
      vector<string> substrings = 
         stringfunc::decompose_string_into_substrings(image_basename,"_");

      unsigned int input_img_width, input_img_height;
      imagefunc::get_image_width_height(
         orig_image_filename, input_img_width, input_img_height);

      if(input_img_width != 96 || input_img_height != 96)
      {
         cout << "Error" << endl;
         cout << "input_img_width = " << input_img_width << endl;
         cout << "input_img_height = " << input_img_height << endl;
      }

      texture_rectangle curr_image(image_filename, NULL);
      classifier.rgb_img_to_bgr_fvec(curr_image);
      classifier.generate_dense_map();

      int classification_label = classifier.get_classification_result();
      double classification_score=classifier.get_classification_score();

      int true_label = -1;
      string true_gender_label = substrings[0];
      if(true_gender_label == "non")
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

      string classification_gender_label;
      if(classification_label == 0)
      {
         classification_gender_label = "non";
      }
      else if(classification_label == 1)
      {
         classification_gender_label = "male";
      }
      else if(classification_label == 2)
      {
         classification_gender_label = "female";
      }

      cout << "true_gender_label = " << true_gender_label << endl;
      cout << "classification_gender_label = " << classification_gender_label
           << " classification score = " << classification_score
           << endl;

      string blob_name = "fc5";
//      string blob_name = "fc6";
//      string blob_name = "fc7_faces";
      classifier.retrieve_layer_activations(blob_name);

      outputfunc::enter_continue_char();

   } // loop over index i labeling input image tiles

   cout << "test.prototxt = " << test_prototxt_filename << endl;
   cout << "trained caffe model = " << caffe_model_filename << endl;
   cout << "input_images_subdir = " << input_images_subdir << endl;
         
}
   
