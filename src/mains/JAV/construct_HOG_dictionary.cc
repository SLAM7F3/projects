// ====================================================================
// Program CONSTRUCT_HOG_DICTIONARY builds upon Davis King's
// quick-n-dirty code to generate a BoW dictionary from HOG features
// extracted from an input set of images.  It first loops over a set
// of specified subdirectories and imports video keyframes.
// CONSTRUCT_HOG_DICTIONARY resizes each input video keyframe to a
// standard pixel width and pixel height.  It then extracts HOG
// descriptors over a lattice.  The total HOG descriptor space is
// subsequently partitioned with randomly oriented planes.  "Voronoi
// cells" in this partitioned space form HOG
// words. CONSTRUCT_HOG_DICTIONARY exports a serialized version of the
// random projection hash to a binary output file in
// BoW_histograms_subdir.
// ====================================================================
// Last updated on 12/30/13; 1/3/14; 1/9/14; 6/21/14
// ====================================================================

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <dlib/svm_threaded.h>
#include <dlib/gui_widgets.h>
#include <dlib/array.h>
#include <dlib/array2d.h>
#include <dlib/image_keypoint.h>
#include <dlib/image_processing.h>
#include <dlib/cmd_line_parser.h>
#include <dlib/data_io.h>

#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "time/timefuncs.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::exception;
using std::flush;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::string;
using std::vector;

typedef dlib::hog_image<
4,4,1,8,dlib::hog_signed_gradient,dlib::hog_full_interpolation> feat_type;

void get_feats (
   const dlib::array2d<unsigned char>& img,
   const dlib::projection_hash& h,
   dlib::matrix<double,0,1>& hist
   )
{
   feat_type feat;
   feat.load(img);
   for (long r = 0; r < feat.nr(); ++r)
   {
      for (long c = 0; c < feat.nc(); ++c)
      {
         hist(h(feat(r,c)))++;
      }
   }
}

// ====================================================================
int main(int argc, char** argv)
{  
   timefunc::initialize_timeofday_clock();

// Import basic HOG BoW processing parameters:

   string gist_subdir="../gist/";
   string BoW_params_filename=gist_subdir+"BoW_params.dat";
   filefunc::ReadInfile(BoW_params_filename);
   int n_HOG_bins=stringfunc::string_to_number(filefunc::text_line[0]);
   int n_hash_planes=basic_math::round(
      log(double(n_HOG_bins))/log(2.0));
   string kernel_type=filefunc::text_line[1];

   bool video_frames_input_flag=
      stringfunc::string_to_boolean(filefunc::text_line[8]);
   cout << "video_frames_input_flag = " << video_frames_input_flag << endl;

   int standard_width,standard_height;
   vector<string> JAV_subdirs;
   if (video_frames_input_flag)
   {
      standard_width=stringfunc::string_to_number(filefunc::text_line[4]);
      standard_height=stringfunc::string_to_number(filefunc::text_line[5]);
      JAV_subdirs.push_back("/data/video/JAV/NewsWraps/early_Sep_2013/");
      JAV_subdirs.push_back("/data/video/JAV/NewsWraps/w_transcripts/");
      JAV_subdirs.push_back(
         "/data/ImageEngine/BostonBombing/clips_1_thru_25/");
   }
   else
   {
      standard_width=stringfunc::string_to_number(filefunc::text_line[6]);
      standard_height=stringfunc::string_to_number(filefunc::text_line[7]);
      JAV_subdirs.push_back("./bundler/tidmarsh/");
   }

   cout << "n_HOG_bins = " << n_HOG_bins << endl;
   cout << "n_hash_planes = " << n_hash_planes << endl;
   cout << "kernel_type = " << kernel_type << endl;
   cout << "Standard width = " << standard_width
        << " standard height = " << standard_height << endl;
   
   dlib::array<dlib::array2d<unsigned char> > images;
   for (unsigned int J=0; J<JAV_subdirs.size(); J++)
   {
      string root_subdir=JAV_subdirs[J];
      string images_subdir,keyframes_subdir;
      if (video_frames_input_flag)
      {
         images_subdir=root_subdir+"jpg_frames/";
         keyframes_subdir=images_subdir+"keyframes/";
      }
      else
      {
         images_subdir=root_subdir+"standard_sized_images/";
      }

      string banner;
      vector<string> image_filenames;
      if (video_frames_input_flag)
      {
         banner="Processing keyframes from "+keyframes_subdir;
         image_filenames=filefunc::image_files_in_subdir(keyframes_subdir);
      }
      else
      {
         banner="Processing images from "+images_subdir;
         image_filenames=filefunc::image_files_in_subdir(images_subdir);
      }
      outputfunc::write_banner(banner);

      int n_images=image_filenames.size();
      for (int i=0; i<n_images; i++)
      {
         outputfunc::update_progress_fraction(i,100,n_images);

// On 12/19/13, Davis King recommended that all input images used for
// dictionary training should conform to the standard pixel size that
// we adopt within COMPUTE_BOW_HISTOGRAMS:

         string std_sized_image_filename="/tmp/std_sized_image.jpg";
         videofunc::force_size_image(
            image_filenames[i],standard_width,standard_height,
            std_sized_image_filename);

         dlib::array2d<unsigned char> img, temp;
         dlib::load_image(img, std_sized_image_filename.c_str());
         images.push_back(img);
      }
   } // loop over index J labeling JAV subdirs

   cout << endl;
   cout << "Number of input images used to build HOG dictionary =" 
        << images.size() << endl;

   feat_type feat;
   dlib::random_subset_selector<feat_type::descriptor_type> samps = 
      dlib::randomly_sample_image_features(
         images, dlib::pyramid_down<4>(),feat, 200000);
   dlib::projection_hash h = dlib::create_random_projection_hash(
      samps, n_hash_planes);

   string HOG_dictionary_filename;
   if (video_frames_input_flag)
   {
      HOG_dictionary_filename="./HOG_dictionary_"+
         stringfunc::number_to_string(images.size())+"_video_keyframes_"+
         stringfunc::number_to_string(n_HOG_bins)+"_words.dat";
   }
   else
   {
      HOG_dictionary_filename="./HOG_dictionary_"+
         stringfunc::number_to_string(images.size())+"_images_"+
         stringfunc::number_to_string(n_HOG_bins)+"_words.dat";
   }

   ofstream fout(HOG_dictionary_filename.c_str(), ios::binary);
   dlib::serialize(h, fout);

   string banner="Exported HOG dictionary to "+HOG_dictionary_filename;
   outputfunc::write_banner(banner);

   banner="At end of program BUILD_HOG_DICTIONARY";
   outputfunc::write_big_banner(banner);
   outputfunc::print_elapsed_time();
}

