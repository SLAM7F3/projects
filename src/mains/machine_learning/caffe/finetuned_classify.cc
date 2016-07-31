// ========================================================================
// Program FINETUNED_CLASSIFY imports a fine-tuned caffe model 
// its test.prototxt file, a text file containing class
// labels and another text file containing mean BGR values.  It
// instantiates a caffe_classifier from these inputs.
// FINETUNED_CLASSIFY then loops over input image files within a
// specified subdirectory and performs classification on each one.
// ========================================================================
// Last updated on 1/20/16; 1/24/16
// ========================================================================

#include "classification/caffe_classifier.h"
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "video/texture_rectangle.h"

using namespace caffe;  // NOLINT(build/namespaces)
using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

int main(int argc, char** argv) 
{
   if (argc != 5) {
      std::cerr << "Usage: " << argv[0]
                << " deploy.prototxt network.caffemodel"
                << " labels_filename mean_rgb_filename" << std::endl;
      return 1;
   }

   ::google::InitGoogleLogging(argv[0]);

   string deploy_prototxt_filename = argv[1];
   string trained_caffe_model_filename = argv[2];
   string mean_rgb_filename    = argv[3];
   string labels_filename    = argv[4];
   bool imagenet_classification_flag = false;

   caffe_classifier classifier(
     deploy_prototxt_filename, trained_caffe_model_filename, 
     mean_rgb_filename, labels_filename, imagenet_classification_flag);

   string input_images_subdir="./images/digits/";

   vector<string> image_filenames=filefunc::image_files_in_subdir(
     input_images_subdir);
   int n_images = image_filenames.size();

   int istart=0;
   int istop = n_images;
   int n_correct = 0, n_incorrect = 0;
   for(int i = istart; i < istop; i++)
   {
      string orig_image_filename = image_filenames[i];
      unsigned int input_img_width, input_img_height;
      imagefunc::get_image_width_height(
         orig_image_filename, input_img_width, input_img_height);
      
// FAKE FAKE:  Weds Jan 20 at 3 pm

// This next ugly call to ImageMagick should soon be replaced with
// cropping an oversized 256x256 image in memory!

      cout << "Original image filename = " << orig_image_filename << endl;
      string image_filename = orig_image_filename;
      string image_basename = filefunc::getbasename(image_filename);
      
      vector<string> substrings = stringfunc::decompose_string_into_substrings(
        image_basename,"_");

      int true_label = -1;
      if(stringfunc::is_number(substrings[0]))
      {
         true_label = stringfunc::string_to_number(substrings[0]);
      }
      
      if(input_img_width != 224 || input_img_height != 224)
      {
         image_filename="cropped_image.jpg";
         imagefunc::crop_image(
            orig_image_filename,image_filename,
            224, 224, 16, 16);
      }
      
      texture_rectangle curr_image(image_filename, NULL);
      classifier.rgb_img_to_bgr_fvec(curr_image);
      classifier.generate_dense_map();
      classifier.cleanup_memory();

      int classified_label = classifier.get_classification_result();
      cout << "Label:  True = " << true_label 
           << " Classified = " << classified_label << endl;
      if(classified_label == true_label)
      {
         n_correct++;
      }
      else
      {
         n_incorrect++;
      }
   } // loop over index i labeling input image tiles

   cout << "n_correct = " << n_correct 
        << " n_incorrect = " << n_incorrect 
        << endl;
}
