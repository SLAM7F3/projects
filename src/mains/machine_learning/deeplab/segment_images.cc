// ========================================================================
// Program SEGMENT_IMAGES imports a caffe model finetuned via deeplab,
// its test.prototxt file, a text file containing class labels and
// another text file containing mean BGR values.  It instantiates a
// caffe_classifier from these inputs.  SEGMENT_IMAGES then loops over
// all image tiles within a specified subdirectory and performs
// semantic segmentation on each one.  Image tile segmentation labels
// and scores are exported as to files whose names end with
// "_segmented.jpg" and "_score.jpg".

// Important notes: 

//  1.  Make sure name for fc8 layer within
//  /data/deeplab/faces_deploy/test.prototxt agrees with fc8 layer name as it
//  appears within /config/vgg128_large_fov/train.prototxt on GPU Titan
//  machine!

//  2.  Make sure width and height dimensions at top of
//  /data/deeplab/faces_deploy/test.prototxt agree with those for tiles to be
//  segmented by this program.  

//  3.  Make sure object_names.classes within /data/deeplab/faces_deploy/ has
//  the correct number of object labels.  Similarly make sure final num_outputs
//  appearing in bottom layer = fc7 --> top layer = fc8_faces/... equals the
//  correct number of classes.

/*

From within /src/mains/machine_learning/deeplab/scripts_faces/, chant

../segment_images \
/data/deeplab/faces_deploy/test.prototxt_1500x1500 \
/data/deeplab/faces_deploy/train_iter_40000_05102016_T1.caffemodel \
/data/deeplab/faces_deploy/fcn.pixel_mean \
/data/deeplab/faces_deploy/object_names.classes \
../images/faces/testing_images_05/fullsized

*/

#include "classification/caffe_classifier.h"
#include "general/filefuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"

using namespace caffe;  // NOLINT(build/namespaces)
using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

// ========================================================================
// Last updated on 5/20/16; 5/24/16; 6/16/16; 7/31/16
// ========================================================================

int main(int argc, char** argv) 
{
   string banner="Starting SEGMENT_IMAGES program";
   outputfunc::write_big_banner(banner);

   timefunc::initialize_timeofday_clock();
   cout << endl << endl << endl;
   cout << "Make certain name/date of fc8 layer in /data/deeplab/XXXX_deploy/test.prototxt__1500x1500 has been updated !!! " << endl;
//    outputfunc::enter_continue_char();

   if (argc != 6) {
      std::cerr << "Usage: " << argv[0]
                << " deploy.prototxt network.caffemodel"
                << " labels_filename mean_rgb_filename input_tiles_basedir" 
		<< endl;
      return 1;
   }

   ::google::InitGoogleLogging(argv[0]);

   string deploy_prototxt_filename = argv[1];
   string trained_caffe_model_filename = argv[2];
   string mean_rgb_filename    = argv[3];
   string labels_filename    = argv[4];
   string input_images_basedir = argv[5];
   bool imagenet_classification_flag = false;

   bool doublesized_imagery_flag = false;
   bool halfsized_imagery_flag = false;
   
   vector<string> substrings = stringfunc::decompose_string_into_substrings(
      input_images_basedir,"/");

   if(substrings.back() == "doublesized")
   {
      doublesized_imagery_flag = true;
   }
   else if(substrings.back() == "halfsized")
   {
      halfsized_imagery_flag = true;
   }

   caffe_classifier classifier(
     deploy_prototxt_filename, trained_caffe_model_filename, 
     mean_rgb_filename, labels_filename, imagenet_classification_flag);
   classifier.set_segmentation_flag(true);

   string input_images_subdir = input_images_basedir;
   string input_tiles_subdir=input_images_subdir+"/tiles/";
   string output_subdir = input_tiles_subdir + "segmentation_results/";
   filefunc::dircreate(output_subdir);
   cout << "input_tiles_subdir = " << input_tiles_subdir << endl;

   vector<string> image_tile_filenames=filefunc::image_files_in_subdir(
     input_tiles_subdir);
   int n_image_tiles = image_tile_filenames.size();
   cout << "n_image_tiles = " << n_image_tiles << endl;

   int istart=0;
   int istop = n_image_tiles;

//   bool find_text_middle_line_flag = true;
//   bool find_text_middle_line_flag = false;
// FAKE FAKE:  Mon Apr 18 at 7:35 am
//   if(find_text_middle_line_flag)
//   {
//      istop = 2;
//   }

   for(int i = istart; i < istop; i++)
   {
      double progress_frac = double(i - istart)/double(istop-istart);
      if((i-istart)% 10 == 0)
      {
         outputfunc::print_elapsed_and_remaining_time(progress_frac);
      }
      string image_filename = image_tile_filenames[i];
      texture_rectangle curr_image(image_filename, NULL);

      classifier.rgb_img_to_bgr_fvec(curr_image);
      classifier.generate_dense_map();
      classifier.display_segmentation_labels(curr_image,output_subdir);
      classifier.display_segmentation_scores(curr_image,output_subdir);
/*
      if(find_text_middle_line_flag)
      {
         int lower_label = 2;
         int upper_label = 1;
         classifier.display_vertical_borders(curr_image,output_subdir, 
                                             lower_label, upper_label);
      }
*/

      classifier.cleanup_memory();

   } // loop over index i labeling input image tiles
}
