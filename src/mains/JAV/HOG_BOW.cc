// ====================================================================
// Program HOG_BOW
// ====================================================================
// Last updated on 12/12/13; 12/13/13
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
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "time/timefuncs.h"

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

//   string JAV_subdir="/data/video/JAV/NewsWraps/early_Sep_2013/";
   string JAV_subdir="/data/video/JAV/NewsWraps/w_transcripts/";
   string root_subdir=JAV_subdir;
   string images_subdir=root_subdir+"jpg_frames/";
   string keyframes_subdir=images_subdir+"keyframes/";


   string HOG_dictionary_filename="HOG_dictionary.dat";
//   bool build_dictionary_flag = true;
   bool build_dictionary_flag = false;
   if (!build_dictionary_flag)
   {
      vector<string> image_filenames=filefunc::image_files_in_subdir(
         images_subdir);

      string BoW_histograms_subdir=root_subdir+"BoWs/";
      filefunc::dircreate(BoW_histograms_subdir);

// This section projects each input image's HOG content onto the
// dictionary basis and exports a histogram of coefficients:

      dlib::projection_hash h;
      ifstream fin(HOG_dictionary_filename.c_str(), ios::binary);
      dlib::deserialize(h, fin);
      
      for (int i=0; i<image_filenames.size(); i++)
      {
         
         double counter_frac=double(i)/image_filenames.size();
         outputfunc::print_elapsed_and_remaining_time(counter_frac);

// Export BoW histogram to output text file:

         string basename=filefunc::getbasename(image_filenames[i]);
         string prefix=stringfunc::prefix(basename);
         string BoW_histogram_filename=BoW_histograms_subdir+
            prefix+".BoW_hist";
         
// Check whether BoW histogram file for current image
// already exists.  If so, move on...

         if (filefunc::fileexist(BoW_histogram_filename))
         {
            cout << "BoW histogram file already exists" << endl;
            continue;
         }

         dlib::array2d<unsigned char> img, temp;
         dlib::load_image(img, image_filenames[i].c_str());

         dlib::matrix<double,0,1> hist(h.num_hash_bins());
         hist = 0;

         const int min_image_size=50*50;
         while (img.size() > min_image_size)
         {
            get_feats(img,h,hist);
            dlib::pyramid_down<4> pyr;
            pyr(img,temp); 
            temp.swap(img);
         }

         ofstream BoW_stream;
         filefunc::openfile(BoW_histogram_filename,BoW_stream);
         BoW_stream << trans(hist) << endl;
         filefunc::closefile(BoW_histogram_filename,BoW_stream);
         
      } // loop over index i labeling input images
      string banner="Exported BoW histograms to "+BoW_histograms_subdir;
      outputfunc::write_banner(banner);
   }
   else
   {

   vector<string> image_filenames=filefunc::image_files_in_subdir(
      keyframes_subdir);

      
// This section creates the dictionary based on the files given on the
// command line

      dlib::array<dlib::array2d<unsigned char> > images;
      for (int i=0; i<image_filenames.size(); i++)
      {
         dlib::array2d<unsigned char> img;
         dlib::load_image(img, image_filenames[i].c_str());
         images.push_back(img);

      }

      feat_type feat;
      dlib::random_subset_selector<feat_type::descriptor_type> samps = 
         dlib::randomly_sample_image_features(
            images, dlib::pyramid_down<4>(),feat, 200000);

      dlib::projection_hash h = dlib::create_random_projection_hash(samps, 10);

      ofstream fout(HOG_dictionary_filename.c_str(), ios::binary);
      dlib::serialize(h, fout);

      string banner="Exported HOG dictionary to "+HOG_dictionary_filename;
      outputfunc::write_banner(banner);
      
   } // build_dictionary_flag conditional


   string banner="At end of program HOG_BOW";
   outputfunc::write_big_banner(banner);
   outputfunc::print_elapsed_time();
}

