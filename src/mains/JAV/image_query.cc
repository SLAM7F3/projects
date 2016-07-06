// ====================================================================
// Program IMAGE_QUERY asks the user to choose a video corpus.  It
// then requests the clip and frame IDs for some video frame
// (which need not necessarily correspond to a keyframe).  IMAGE_QUERY
// first pops open a window displaying the query image.  It next
// computes a histogram for the query image over the HOG BoW
// dictionary.  It then matches the query image's histogram against
// histograms for every keyframe within the specified video corpus.
// IMAGE_QUERY generates a montage from all keyframes whose matching
// scores with the input query image exceed some reasonable threshold
// value.  A second window is popped open which displays the matching
// results in montage format.

//			       ./image_query

// ====================================================================
// Last updated on 1/4/14; 1/8/14; 1/9/14
// ====================================================================

#include <fstream>
#include <iostream>
#include <map>
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
#include "math/ltduple.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "video/RGB_analyzer.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::exception;
using std::flush;
using std::ifstream;
using std::ios;
using std::map;
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
   cout << "n_HOG_bins = " << n_HOG_bins << endl;
//   string kernel_type=filefunc::text_line[1];
//   cout << "kernel_type = " << kernel_type << endl;

   bool video_frames_input_flag=
      stringfunc::string_to_boolean(filefunc::text_line[8]);
   cout << "video_frames_input_flag = " << video_frames_input_flag << endl;
   bool keyframes_flag=video_frames_input_flag;

   bool diff_over_sum_flag=false;
   bool sqrt_dotproduct_flag=false;
   bool sqrt_diff_over_sum_flag=false;
   string BoW_bin_scoring_type=filefunc::text_line[9];
   if (BoW_bin_scoring_type=="diff_over_sum")
   {
      diff_over_sum_flag=true;
   }
   else if (BoW_bin_scoring_type=="sqrt_dotproduct")
   {
      sqrt_dotproduct_flag=true;
   }
   else if (BoW_bin_scoring_type=="sqrt_diff_over_sum")
   {
      sqrt_diff_over_sum_flag=true;
   }
   cout << "diff_over_sum_flag = " << diff_over_sum_flag << endl;
   cout << "sqrt_dotproduct_flag = " << sqrt_dotproduct_flag << endl;
   cout << "sqrt_diff_over_sum_flag = " << sqrt_diff_over_sum_flag << endl;

   int standard_width,standard_height;
   if (video_frames_input_flag)
   {
      standard_width=stringfunc::string_to_number(filefunc::text_line[4]);
      standard_height=stringfunc::string_to_number(filefunc::text_line[5]);
   }
   else
   {
      standard_width=stringfunc::string_to_number(filefunc::text_line[6]);
      standard_height=stringfunc::string_to_number(filefunc::text_line[7]);
   }
//   cout << "Standard width = " << standard_width
//        << " standard height = " << standard_height << endl;

//   const int K=2048;
//   int n_color_bins=33;
   const int K=2048+33;

/*
   if (n_HOG_bins != K)
   {
      cout << "Error! : K = " << K << endl;
      cout << "n_HOG_bins = " << n_HOG_bins << endl;
      exit(-1);
   }
*/

//   int max_clip_ID=stringfunc::string_to_number(filefunc::text_line[2]);
//   int max_frame_ID=stringfunc::string_to_number(filefunc::text_line[3]);
//   cout << "max_clip_ID = " << max_clip_ID << endl;
//   cout << "max_frame_ID = " << max_frame_ID << endl;


// Import HOG dictionary generated via program BUILD_HOG_DICTIONARY:

   string HOG_dictionary_subdir="./HOG_dictionary/";
   string HOG_dictionary_filename=HOG_dictionary_subdir+"HOG_dictionary.dat";
   cout << "Importing HOG dictionary from "+HOG_dictionary_filename << endl;

   dlib::projection_hash h;
   ifstream fin(HOG_dictionary_filename.c_str(), ios::binary);
   dlib::deserialize(h, fin);

// Import probabilistic decision function generated by an SVM 
// on matching and nonmatching images by program SVM_GLOBAL_DESCRIPS:

   typedef dlib::matrix<double, K, 1> sample_type;

// LINEAR KERNEL
//      typedef dlib::linear_kernel<sample_type> kernel_type;

// HISTOGRAM INTERSECTION KERNEL (for BoW)
   typedef dlib::histogram_intersection_kernel<sample_type> kernel_type;

   sample_type sample;

   typedef dlib::probabilistic_decision_function<kernel_type> 
      probabilistic_funct_type;  
   typedef dlib::normalized_function<probabilistic_funct_type> pfunct_type;
   pfunct_type pfunct;

   string BoW_matches_subdir="../gist/BoW_matches/";
   string learned_funcs_subdir=BoW_matches_subdir+"learned_functions/";
   string learned_pfunct_filename=learned_funcs_subdir+"pfunct.dat";

   ifstream fin6(learned_pfunct_filename.c_str(),ios::binary);
   dlib::deserialize(pfunct, fin6);

   string JAV_subdir;
   string ImageEngine_subdir="/data/ImageEngine/";
   string BostonBombing_subdir=ImageEngine_subdir+"BostonBombing/";

   int video_corpus_ID=-1;
   double min_color_overlap_score;
   if (video_frames_input_flag)
   {
      min_color_overlap_score=0.35;
      cout << endl;
      cout << "1: early September 2013 NewsWraps" << endl;
      cout << "2: October 2013 NewsWraps with transcripts" << endl;
      cout << "3: Boston Bombing YouTube clips 1 - 25" << endl;

      cout << "Enter video corpus ID:" << endl;
      cin >> video_corpus_ID;
   
      if (video_corpus_ID==1)
      {
         JAV_subdir="/data/video/JAV/NewsWraps/early_Sep_2013/";
      }
      else if (video_corpus_ID==2)
      {
         JAV_subdir="/data/video/JAV/NewsWraps/w_transcripts/";
      }
      else if (video_corpus_ID==3)
      {
         JAV_subdir=BostonBombing_subdir+"clips_1_thru_25/";
      }
      else
      {
         exit(-1);
      }
   }
   else
   {
      min_color_overlap_score=0.5;
      JAV_subdir="./bundler/tidmarsh/";
   }
//   cout << "JAV_subdir = " << JAV_subdir << endl;


   string root_subdir=JAV_subdir;
   string images_subdir,keyframes_subdir;
   if (video_frames_input_flag)
   {
      images_subdir=root_subdir+"jpg_frames/";
      keyframes_subdir=images_subdir+"keyframes/";
   }
   else
   {
      images_subdir=root_subdir+"images/";
   }
   cout << "images_subdir = " << images_subdir << endl;
   
// Store keyframes' clip and frame IDs within STL map keyframes_map:

   typedef map<DUPLE,string> KEYFRAMES_MAP;
   KEYFRAMES_MAP keyframes_map;

// independent DUPLE holds video image clip and frame ID
// dependent string holds video image's filename

   if (keyframes_flag)
   {
      vector<string> keyframe_filenames=filefunc::image_files_in_subdir(
         keyframes_subdir);

      for (unsigned int i=0; i<keyframe_filenames.size(); i++)
      {
         vector<string> substrings=
            stringfunc::decompose_string_into_substrings(
               filefunc::getbasename(keyframe_filenames[i])," _-");

// If input filename has form clip_0000_frame-00001.jpg, record clip's
// ID within STL vector:

         int clip_ID=-1;
         int frame_ID=-1;
         if (substrings.size()==4)
         {
            if (substrings[0]=="clip")
            {
               clip_ID=stringfunc::string_to_number(substrings[1]);
               frame_ID=stringfunc::string_to_number(substrings[3]);
               keyframes_map[DUPLE(clip_ID,frame_ID)]=keyframe_filenames[i];
            }
         }
      } // loop over index i labeling keyframe filenames
   } // keyframes_flag conditional
   cout << "keyframes_map.size() = " << keyframes_map.size() << endl;

   string BoW_histograms_subdir=root_subdir+"BoWs/";
   string color_histograms_subdir=root_subdir+"global_color_histograms/";

// Import BoW histograms generated by program COMPUTE_BoW_HISTOGRAMS:

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("BoW_hist");
   vector<string> BoW_histogram_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,BoW_histograms_subdir);

   string banner="Importing BoW histograms from text files:";
   outputfunc::write_banner(banner);

// Store BoW histograms as functions of video keyframe clip and frame
// IDs within STL map BoW_histograms_map:

   typedef map<DUPLE,double*> BOW_HISTOGRAMS_MAP;
   BOW_HISTOGRAMS_MAP BoW_histograms_map;
   BOW_HISTOGRAMS_MAP::iterator BoW_histogram_iter;

// independent DUPLE holds video image clip and frame ID.  If we're
// working with digital stills rather than video clips, clip_ID=0
// while frame_ID=image_ID.

// dependent double* holds BoW histogram

   for (unsigned int i=0; i<BoW_histogram_filenames.size(); i++)
   {
      outputfunc::update_progress_fraction(
         i,100,BoW_histogram_filenames.size());
      string basename=filefunc::getbasename(BoW_histogram_filenames[i]);
//      cout << "basename = " << basename << endl;
      
      int clip_ID=-1;
      int frame_ID=-1;
      DUPLE curr_duple;
      if (keyframes_flag && keyframes_map.size() > 0)
      {
         vector<string> substrings=
            stringfunc::decompose_string_into_substrings(
               stringfunc::prefix(basename)," _-");

// Ignore input histogram if its filename has form
// clip_0000_frame-00001 and a corresponding clip_ID/frame_ID duple
// entry is NOT found within keyframes_map:

         if (substrings.size()==4)
         {
            if (substrings[0]=="clip")
            {
               clip_ID=stringfunc::string_to_number(substrings[1]);
               frame_ID=stringfunc::string_to_number(substrings[3]);
               curr_duple=DUPLE(clip_ID,frame_ID);
               KEYFRAMES_MAP::iterator keyframe_iter=
                  keyframes_map.find(curr_duple);
               if (keyframe_iter==keyframes_map.end())
               {
                  continue;
               }
            } // substrings[0] conditional
         } // substrings.size()==4 conditional
      }
      else
      {
         clip_ID=0;
         frame_ID=stringfunc::string_to_number(basename.substr(3,5));
         curr_duple=DUPLE(clip_ID,frame_ID);
      } // keyframes_flag conditional

      vector<double> curr_bin_values=filefunc::ReadInNumbers(
         BoW_histogram_filenames[i]);
      
      double* curr_BoW_histogram_ptr=new double[n_HOG_bins];
      BoW_histograms_map[curr_duple]=curr_BoW_histogram_ptr;
      
      for (int b=0; b<n_HOG_bins; b++)
      {
         if (sqrt_dotproduct_flag || sqrt_diff_over_sum_flag)
         {
            (curr_BoW_histogram_ptr)[b]=sqrt(curr_bin_values[b]);
         }
         else
         {
            (curr_BoW_histogram_ptr)[b]=curr_bin_values[b];
         }
      }
   } // loop over index i labeling video keyframe HOG BoW histograms
   cout << endl;

   int n_BoW_histograms=BoW_histograms_map.size();
   cout << "n_BoW_histograms = " << n_BoW_histograms << endl;
   cout << "n_HOG_bins = " << n_HOG_bins << endl;

// Import global color histograms generated by program
// COMPUTE_COLOR_HISTOGRAMS:

   allowed_suffixes.clear();
   allowed_suffixes.push_back("color_hist");
   vector<string> color_histogram_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,color_histograms_subdir);
   cout << "color_histogram_filenames.size() = " 
        << color_histogram_filenames.size()
        << endl;

   banner="Importing color histograms from text files:";
   outputfunc::write_banner(banner);

// Store color histograms as functions of video keyframe clip and frame
// IDs within STL map color_histograms_map:

   typedef map<DUPLE,double*> COLOR_HISTOGRAMS_MAP;
   COLOR_HISTOGRAMS_MAP color_histograms_map;
   COLOR_HISTOGRAMS_MAP::iterator color_histogram_iter;

// independent DUPLE holds video image clip and frame ID.  If we're
// working with digital stills rather than video clips, clip_ID=0
// while frame_ID=image_ID.

// dependent double* holds color histogram

   int n_color_bins=-1;
   for (unsigned int i=0; i<color_histogram_filenames.size(); i++)
   {
      outputfunc::update_progress_fraction(
         i,100,color_histogram_filenames.size());
      string basename=filefunc::getbasename(color_histogram_filenames[i]);
//      cout << "basename = " << basename << endl;
//      outputfunc::enter_continue_char();

      int clip_ID=-1;
      int frame_ID=-1;
      DUPLE curr_duple;
      if (keyframes_flag && keyframes_map.size() > 0)
      {
         vector<string> substrings=
            stringfunc::decompose_string_into_substrings(
               stringfunc::prefix(basename)," _-");

// Ignore input histogram if its filename has form
// clip_0000_frame-00001 and a corresponding clip_ID/frame_ID duple
// entry is NOT found within keyframes_map:

         if (substrings.size()==4)
         {
            if (substrings[0]=="clip")
            {
               clip_ID=stringfunc::string_to_number(substrings[1]);
               frame_ID=stringfunc::string_to_number(substrings[3]);
               curr_duple=DUPLE(clip_ID,frame_ID);
               KEYFRAMES_MAP::iterator keyframe_iter=
                  keyframes_map.find(curr_duple);
               if (keyframe_iter==keyframes_map.end())
               {
                  continue;
               }
            } // substrings[0] conditional
         } // substrings.size()==4 conditional
      }
      else
      {
         clip_ID=0;
         frame_ID=stringfunc::string_to_number(basename.substr(3,5));
         curr_duple=DUPLE(clip_ID,frame_ID);
      } // keyframes_flag conditional

      vector<double> curr_bin_values=filefunc::ReadInNumbers(
         color_histogram_filenames[i]);
      
      n_color_bins=curr_bin_values.size();
      double* curr_color_histogram_ptr=new double[n_color_bins];
      color_histograms_map[curr_duple]=curr_color_histogram_ptr;
      
      for (int b=0; b<n_color_bins; b++)
      {
         (curr_color_histogram_ptr)[b]=curr_bin_values[b];
      }
   } // loop over index i labeling video keyframe HOG color histograms
   cout << endl;

   cout << "n_color_histograms = " << color_histograms_map.size() << endl;
   cout << "n_color_bins = " << n_color_bins << endl;

   string query_results_subdir="./query_montage_results/";
   filefunc::dircreate(query_results_subdir);

// ********************************************************************
// Main loop over query and pairs of BoW+color histograms starts here.

   while (true)
   {
      sysfunc::kill_process("display");

// Assemble query image(s):
      
//   bool loop_over_all_keyframes_flag=true;
      bool loop_over_all_keyframes_flag=false;

      vector<string> query_filenames;
      if (loop_over_all_keyframes_flag)
      {
         query_filenames=filefunc::image_files_in_subdir(images_subdir);
      }
      else
      {
         string query_image_filename;
         if (video_frames_input_flag)
         {
            int clip_ID,frame_ID;
            cout << "Enter clip ID:" << endl;
            cin >> clip_ID;
            cout << "Enter frame ID:" << endl;
            cin >> frame_ID;

            if (video_corpus_ID==3)	// Boston Bombing
            {
               string query_images_subdir=BostonBombing_subdir+
                  "clips_1_thru_133/";
               query_image_filename=query_images_subdir+
                  "clip_"+stringfunc::integer_to_string(clip_ID,4)+
                  "_frame-"+stringfunc::integer_to_string(frame_ID,5)+
                  "-scaled.jpg";
            }
            else
            {
               query_image_filename=images_subdir+
                  "clip_"+stringfunc::integer_to_string(clip_ID,4)+
                  "_frame-"+stringfunc::integer_to_string(frame_ID,5)+
                  ".jpg";
            }
         }
         else
         {
//         cout << "Enter query image filename:" << endl;
//         cin >> query_image_filename;
            string query_image_basename;
            cout << "Enter query image basename:" << endl;
            cin >> query_image_basename;
            query_image_filename=images_subdir+query_image_basename;
         }
         query_filenames.push_back(query_image_filename);
      
         cout << "query filename = " << query_image_filename << endl;
         string unix_cmd="smallview "+query_image_filename;
         sysfunc::unix_command(unix_cmd);
      }

      double* query_BoW_histogram_ptr=new double[n_HOG_bins];
      for (unsigned int q=0; q<query_filenames.size(); q++)
      {
         double query_progress_frac=double(q)/query_filenames.size();
         outputfunc::update_progress_fraction(q,20,query_filenames.size());

// Important note added on 12/15/13 at 5 pm: Query image size must
// equal standard image size !!

         string resized_query_filename="/tmp/std_sized_image.jpg";
         videofunc::force_size_image(
            query_filenames[q],standard_width,
            standard_height,resized_query_filename);

// Compute query image's HOG BoW histogram:

         dlib::array2d<unsigned char> img, temp;
         dlib::load_image(img, resized_query_filename.c_str());

         dlib::matrix<double,0,1> hist(h.num_hash_bins());
         hist = 0;

         const unsigned int min_image_size=50*50;
         while (img.size() > min_image_size)
         {
            get_feats(img,h,hist);
            dlib::pyramid_down<4> pyr;
            pyr(img,temp); 
            temp.swap(img);
         }

         for (int b=0; b<n_HOG_bins; b++)
         {

            if (sqrt_dotproduct_flag || sqrt_diff_over_sum_flag)
            {
               (query_BoW_histogram_ptr)[b]=sqrt(hist(b));
            }
            else
            {
               (query_BoW_histogram_ptr)[b]=hist(b);
            }
         }

// Compute query image's global color histogram:

         RGB_analyzer* RGB_analyzer_ptr=new RGB_analyzer();
         string liberalized_color="";
         RGB_analyzer_ptr->import_quantized_RGB_lookup_table(
            liberalized_color);
         texture_rectangle* texture_rectangle_ptr=new texture_rectangle();

         texture_rectangle_ptr->reset_texture_content(resized_query_filename);
         vector<double> query_color_histogram=
            RGB_analyzer_ptr->compute_color_histogram(
               texture_rectangle_ptr,liberalized_color);

         delete RGB_analyzer_ptr;
         delete texture_rectangle_ptr;

// Compute overlap score between query image's and video keyframes'
// HOG BoW and color histograms:

         vector<double> match_probs;
         vector<string> archive_image_filenames;

         for (BoW_histogram_iter=BoW_histograms_map.begin(); 
              BoW_histogram_iter != BoW_histograms_map.end();
              BoW_histogram_iter++)
         {
            DUPLE curr_duple=BoW_histogram_iter->first;

            if (keyframes_flag && keyframes_map.size() > 0)
            {
               KEYFRAMES_MAP::iterator keyframe_iter=
                  keyframes_map.find(curr_duple);
               if (keyframe_iter==keyframes_map.end()) continue;
            }

            int curr_clip_ID=curr_duple.first;
            int curr_frame_ID=curr_duple.second;

            double* curr_BoW_histogram_ptr=BoW_histogram_iter->second;

            color_histogram_iter=color_histograms_map.find(
               DUPLE(curr_clip_ID,curr_frame_ID));
            if (color_histogram_iter==color_histograms_map.end()) continue;
         
            double* curr_color_histogram_ptr=color_histogram_iter->second;

// First compute comparison score between current and next color
// histograms.  Reject any image pair whose color contents are too
// different:

            float numer,denom;
            float color_overlap_score=0;
            for (int b=0; b<n_color_bins; b++)
            {
               denom=curr_color_histogram_ptr[b]+query_color_histogram[b];
               if (denom > 0)
               {
                  numer=
                     curr_color_histogram_ptr[b]-query_color_histogram[b];
                  color_overlap_score += fabs(numer)/denom;
               }
            } // loop over index b labeling color bins
            color_overlap_score /= n_color_bins;

// Transform overlap score so that unity [zero] value indicates
// similarity [dissimilarity]:

            color_overlap_score=1-color_overlap_score;

            if (color_overlap_score < min_color_overlap_score) continue;

// Calculate comparison score between current and next BoW histograms:

            for (int b=0; b<n_HOG_bins; b++)
            {
               float HOG_overlap_score=0;

// Recall curr and query histogram values have already been
// square-rooted if sqrt_diff_over_sum_flag==true or
// sqrt_dotproduct_flag==true:

//               if (sqrt_diff_over_sum_flag)
               {
                  denom=curr_BoW_histogram_ptr[b]+query_BoW_histogram_ptr[b];
                  if (denom > 0)
                  {
                     numer=curr_BoW_histogram_ptr[b]-
                        query_BoW_histogram_ptr[b];
                     HOG_overlap_score=1-fabs(numer)/denom;
                  }
                  else
                  {
                     HOG_overlap_score=1;
                  }
               } // overlap score computation conditional

// As of 1/1/14, we experiment with dividing HOG_overlap_score by
// n_HOG_bins to facilitate later combination of HOG and color
// histograms which have different numbers of bins:

               sample(b)=HOG_overlap_score/n_HOG_bins;

            } // loop over index b labeling HOG bins

// Calculate comparison score between current and next color histograms:

            for (int b=0; b<n_color_bins; b++)
            {
               float color_overlap_score=0;
               float curr_hist=curr_color_histogram_ptr[b];
               float query_hist=query_color_histogram[b];

               denom=curr_hist+query_hist;
               if (denom > 0)
               {
                  numer=curr_hist-query_hist;
                  color_overlap_score=1-fabs(numer)/denom;
               }
               else
               {
                  color_overlap_score=1;
               }

// As of 1/1/14, we experiment with dividing color_overlap_score by
// n_color_bins to facilitate later combination of HOG and color
// histograms which have different numbers of bins:

               sample(n_HOG_bins+b)=color_overlap_score/n_color_bins;

            } // loop over index b labeling color bins

            double curr_match_prob=pfunct(sample);
            match_probs.push_back(curr_match_prob);
//         cout << "curr_match_prob = " << curr_match_prob << endl;
//       outputfunc::enter_continue_char();

            string curr_image_filename;
            if (video_frames_input_flag)
            {
               curr_image_filename=images_subdir+
                  "clip_"+stringfunc::integer_to_string(curr_clip_ID,4)
                  +"_frame-"+
                  stringfunc::integer_to_string(curr_frame_ID,5)+".jpg";
            }
            else
            {
               curr_image_filename=images_subdir+
                  "pic"+stringfunc::integer_to_string(curr_frame_ID,5)+".jpg";
            }
            archive_image_filenames.push_back(curr_image_filename);

//         cout << filefunc::getbasename(archive_image_filenames.back()) << "  "
//              << match_probs.back() << endl;

         } // BoW histogram iterator loop

         if (match_probs.size()==0) continue;

         cout << "match_probs.size() = " << match_probs.size()
              << " archive_image_filenames.size() = "
              << archive_image_filenames.size() << endl;

         templatefunc::Quicksort_descending(
            match_probs,archive_image_filenames);

         const double min_match_prob=0.5;
//      const double min_match_prob=0.6;
//      const double min_match_prob=0.66;
//      const double min_match_prob=0.75;
//      const double min_match_prob=0.8;

// Always display at least one result to image query:

         int i_stop=7;
         if (match_probs.front() < min_match_prob)
         {
            i_stop=0;
         }

         
         string montage_cmd="montageview ";
         for (int i=0; i<=i_stop; i++)
         {
            if (match_probs[i] > min_match_prob || i_stop==0)
            {
               string text_color="purple";
               string caption="P="+stringfunc::number_to_string(
                  match_probs[i],2);
               string eastwest_location="west";
               string northsouth_location="south";
               string annotated_image_filename=query_results_subdir+
                  filefunc::getbasename(archive_image_filenames[i]);
               imagefunc::add_text_to_image(
                  text_color,caption,
                  eastwest_location,northsouth_location,
                  archive_image_filenames[i],
                  annotated_image_filename);

               cout << i << "  "
                    << match_probs[i] << "  "
                    << filefunc::getbasename(archive_image_filenames[i]) 
                    << endl;
               montage_cmd += annotated_image_filename+" ";
//                  montage_cmd += archive_image_filenames[i]+" ";
            }
         } // loop over top matches between archived and query images

//         cout << "montage_cmd = " << montage_cmd << endl;
         sysfunc::unix_command(montage_cmd);

         filefunc::dircreate(query_results_subdir);
         string mv_cmd="mv montage_*.jpg "+query_results_subdir;

//         cout << "mv_cmd = " << mv_cmd << endl;
         sysfunc::unix_command(mv_cmd);

         if (loop_over_all_keyframes_flag)
         {
            outputfunc::enter_continue_char();
         }
      
      } // loop over index q labeling query filenames

      delete query_BoW_histogram_ptr;

      outputfunc::enter_continue_char();
   } // infinite while (true) loop
   
   banner="At end of program IMAGE_QUERY";
   outputfunc::write_banner(banner);
}

