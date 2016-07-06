// ==========================================================================
// Program GENERATE_BOW_SCORES is a variant of GENERATE_MATCH_SCORES
// specialized to work exclusively with HOG BOW features.  It first
// imports BoW histograms calculated via program
// COMPUTE_BOW_HISTOGRAMS within an STL map.  GENERATE_BOW_SCORES next
// imports relatively small numbers of pairs of manually identified
// matching & non-matching image filenames generated via programs
// GENERATE_[NON]MATCHING_FILENAMES.  Repeated image pair
// filenames are ignored.

// Video clip and frame IDs are stored as independent variables within
// STL map BoW_descriptor_scores_map.  Image overlaps for each HOG bin
// are calculated and stored as dependent score vectors in
// BoW_descriptor_scores_map.

// Iterating over all entries in BoW_descriptor_scores_map,
// GENERATE_BOW_SCORES exports matching_images.dat and
// nonmatching_images.dat text files which list n_HOG_bins feature
// scores for matching and nonmatching image categories. These text
// files become input for program SVM_GLOBAL_DESCRIPS.  It also
// outputs matching_files.txt and nonmatching_files.txt which contain
// the names of paired files whose scores are contained in
// matching_images.dat and nonmatching_images.dat.

//		          ./generate_BoW_scores

// ==========================================================================
// Last updated on 12/27/13; 12/28/13; 12/31/13
// ==========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <dlib/matrix.h>

#include "general/filefuncs.h"
#include "video/descriptorfuncs.h"
#include "math/ltduple.h"
#include "image/imagefuncs.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "math/prob_distribution.h"
#include "datastructures/Quadruple.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ios;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(12);

   timefunc::initialize_timeofday_clock();      

//   bool video_frames_input_flag=true;
   bool video_frames_input_flag=false;
   bool keyframes_flag=video_frames_input_flag;


   string gist_subdir="../gist/";
   string BoW_params_filename=gist_subdir+"BoW_params.dat";
   filefunc::ReadInfile(BoW_params_filename);
   int n_HOG_bins=stringfunc::string_to_number(filefunc::text_line[0]);
   cout << "n_HOG_bins = " << n_HOG_bins << endl;

   typedef std::map<long long,genvector*> BOW_DESCRIPTOR_SCORES_MAP;
   BOW_DESCRIPTOR_SCORES_MAP BoW_descriptor_scores_map;
   BOW_DESCRIPTOR_SCORES_MAP::iterator iter;

// independent long long = quadruple ID
// dependent genvector holds BoW comparison scores   

   int max_clip_ID=stringfunc::string_to_number(filefunc::text_line[2]);
   int max_frame_ID=stringfunc::string_to_number(filefunc::text_line[3]);
//   cout << "max_clip_ID = " << max_clip_ID << endl;
//   cout << "max_frame_ID = " << max_frame_ID << endl;


   string JAV_subdir;
   if (video_frames_input_flag)
   {
      string ImageEngine_subdir="/data/ImageEngine/";
      string BostonBombing_subdir=ImageEngine_subdir+"BostonBombing/";

      cout << "1: early September 2013 NewsWraps" << endl;
      cout << "2: October 2013 NewsWraps with transcripts" << endl;
      cout << "3: Boston Bombing YouTube clips 1 - 25" << endl;

      int video_corpus_ID;
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
      JAV_subdir="./bundler/tidmarsh/";
   } // video_frames_input_flag conditional
   cout << "JAV_subdir = " << JAV_subdir << endl;

   string root_subdir=JAV_subdir;
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
   string BoW_subdir=root_subdir+"BoWs/";


   bool sqrt_diff_over_sum_flag=true;
//   bool sqrt_diff_over_sum_flag=false;

// Store video keyframes' clip and frame IDs within STL map keyframes_map:

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

// If input filename has form clip_0000_frame-00001.jpg, record clip
// and frame IDs with keyframes_map:

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

// Import BoW histograms generated by program COMPUTE_BoW_HISTOGRAMS:

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("BoW_hist");
   vector<string> histogram_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,BoW_subdir);
//   cout << "histogram_filenames.size() = " << histogram_filenames.size()
//        << endl;

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

   int n_bins=-1;
   for (unsigned int i=0; i<histogram_filenames.size(); i++)
   {
      outputfunc::update_progress_fraction(i,100,histogram_filenames.size());
      string basename=filefunc::getbasename(histogram_filenames[i]);
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
         histogram_filenames[i]);
      
      n_bins=curr_bin_values.size();
      double* curr_histogram_ptr=new double[n_bins];
      BoW_histograms_map[curr_duple]=curr_histogram_ptr;
      
      for (int b=0; b<n_bins; b++)
      {
         (curr_histogram_ptr)[b]=curr_bin_values[b];
      }
   } // loop over index i labeling video keyframe HOG BoW histograms
   cout << endl;

   cout << "n_BoW_histograms = " << BoW_histograms_map.size() << endl;
   cout << "n_HOG_bins = " << n_bins << endl;

   string matching_images_filename=root_subdir+"matching_image_filenames.dat";
   string nonmatching_images_filename=root_subdir+
      "nonmatching_image_filenames.dat";
   vector<string> input_image_filenames;
   input_image_filenames.push_back(matching_images_filename);
   input_image_filenames.push_back(nonmatching_images_filename);

// Loop over training pairs of coarsely matching and non-matching
// image filenames starts here:

   int n_repeat_filename_pairs=0;
   int f_start=0;
   for (unsigned int f=f_start; f<input_image_filenames.size(); f++)
   {
      if (f==0)
      {
         cout << "Importing names of matching image files:" << endl;
      }
      else
      {
         cout << "Importing names of non-matching image files:" << endl;
      }

      BoW_descriptor_scores_map.clear();

// Import pairs of matching and non-matching image filenames.  Store
// their clip & frame IDs as independent variables within STL map
// BoW_descriptor_scores_map.  Then compute their image overlaps for
// each HOG bin.  Store resulting overlap score vectors as dependent
// variables in BoW_descriptor_scores_map:

      string image_pair_filenames=input_image_filenames[f];
      cout << "image_pair_filenames = " << image_pair_filenames << endl;
      filefunc::ReadInfile(image_pair_filenames);

      string separator_chars=" _-";
      for (unsigned int i=0; i<filefunc::text_line.size(); i++)
      {
         string curr_line=filefunc::text_line[i];
         vector<string> substrings=
            stringfunc::decompose_string_into_substrings(curr_line);
         string basename1=filefunc::getbasename(substrings[0]);
         string basename2=filefunc::getbasename(substrings[1]);
//         cout << "basename1 = " << basename1 
//              << " basename2 = " << basename2 << endl;

         int first_clip_ID=0;
         int second_clip_ID=0;
         int first_frame_ID,second_frame_ID;
         double* first_histogram_ptr=NULL;
         double* second_histogram_ptr=NULL;
         if (video_frames_input_flag)
         {
            vector<string> subsubstrings=
               stringfunc::decompose_string_into_substrings(
                  basename1,separator_chars);
            first_clip_ID=stringfunc::string_to_number(subsubstrings[1]);
            first_frame_ID=stringfunc::string_to_number(subsubstrings[3]);
         }
         else
         {
            string curr_substr=basename1;
            first_frame_ID=stringfunc::string_to_number(
               curr_substr.substr(3,5));
         }
//         cout << "first_clip_ID = " << first_clip_ID
//              << " first_frame_ID = " << first_frame_ID << endl;

         BoW_histogram_iter=BoW_histograms_map.find(
            DUPLE(first_clip_ID,first_frame_ID));
         if (BoW_histogram_iter==BoW_histograms_map.end()) continue;
         first_histogram_ptr=BoW_histogram_iter->second;

         if (video_frames_input_flag)
         {
            vector<string> subsubstrings=
               stringfunc::decompose_string_into_substrings(
                  basename2,separator_chars);
            second_clip_ID=stringfunc::string_to_number(subsubstrings[1]);
            second_frame_ID=stringfunc::string_to_number(subsubstrings[3]);
         }
         else
         {
            string curr_substr=basename2;
            second_frame_ID=stringfunc::string_to_number(
               curr_substr.substr(3,5));
         }
//         cout << "second_clip_ID = " << second_clip_ID
//              << " second_frame_ID = " << second_frame_ID << endl;

         BoW_histogram_iter=BoW_histograms_map.find(
            DUPLE(second_clip_ID,second_frame_ID));
         if (BoW_histogram_iter==BoW_histograms_map.end()) continue;
         second_histogram_ptr=BoW_histogram_iter->second;

         long long quadruple_ID=
            mathfunc::unique_integer(
               first_clip_ID,first_frame_ID,second_clip_ID,second_frame_ID,
               max_clip_ID,max_frame_ID,max_clip_ID);
//         cout << "quadruple_ID = " << quadruple_ID << endl;

         iter=BoW_descriptor_scores_map.find(quadruple_ID);
         if (iter==BoW_descriptor_scores_map.end())
         {
            genvector* descriptor_score_ptr=new genvector(n_HOG_bins);
            BoW_descriptor_scores_map[quadruple_ID]=descriptor_score_ptr;

// Calculate comparison score between first and second BoW histograms:

            for (int b=0; b<n_HOG_bins; b++)
            {
               float overlap_score=0;
               if (sqrt_diff_over_sum_flag)
               {
                  float sqrt_1st_hist=sqrt(first_histogram_ptr[b]);
                  float sqrt_2nd_hist=sqrt(second_histogram_ptr[b]);
                  
                  float numer=sqrt_1st_hist-sqrt_2nd_hist;
                  float denom=sqrt_1st_hist+sqrt_2nd_hist;
                  if (denom > 0)
                  {
                     overlap_score=1-fabs(numer)/denom;
                  }
                  else
                  {
                     overlap_score=1;
                  }
               } // overlap score computation conditional
               descriptor_score_ptr->put(b,overlap_score);
            } // loop over index b labeling HOG bins
         }
         else
         {
            n_repeat_filename_pairs++;
         }
      } // loop over index i labeling matching image filenames

      cout << "n_repeat_filename_pairs = " << n_repeat_filename_pairs << endl;
      cout << "BoW_descriptor_scores_map.size() = "
           << BoW_descriptor_scores_map.size() << endl;

// Export training pairs of matching/nonmatching images' overlap
// scores for all n_HOG_bins along with images' basenames to output
// text files:

      string output_filename=root_subdir+"matching_images.dat";
      string files_filename=root_subdir+"matching_files.txt";
      if (f==1)
      {
         output_filename=root_subdir+"nonmatching_images.dat";
         files_filename=root_subdir+"nonmatching_files.txt";
      }

      ofstream outstream,filesstream;
      filefunc::openfile(output_filename,outstream);
      filefunc::openfile(files_filename,filesstream);
      outstream << "# Overlap scores for n_HOG_bins = " << n_HOG_bins 
                << " image1_filename image2_filename" << endl << endl;
      filesstream << "# image1_filename image2_filename" << endl << endl;

      int image_pair_counter=0;
      for (iter=BoW_descriptor_scores_map.begin(); 
           iter != BoW_descriptor_scores_map.end(); iter++)
      {
         long long quadruple_ID=iter->first;
         long long clip1_ID,frame1_ID,clip2_ID,frame2_ID;
         mathfunc::decompose_unique_integer(
            quadruple_ID,max_clip_ID,max_frame_ID,max_clip_ID,
            clip1_ID,frame1_ID,clip2_ID,frame2_ID);
         int first_clip_ID=clip1_ID;
         int first_frame_ID=frame1_ID;
         int second_clip_ID=clip2_ID;
         int second_frame_ID=frame2_ID;
         genvector* descriptor_score_ptr=iter->second;
//         cout << "*descriptor_score_ptr = " << *descriptor_score_ptr << endl;

// Do not export purely zero-valued descriptors:

         double descriptors_sum=0;
         for (int b=0; b<n_HOG_bins; b++)
         {
            descriptors_sum += descriptor_score_ptr->get(b);
         }
         if (nearly_equal(0,descriptors_sum)) continue;

         for (int b=0; b<n_HOG_bins; b++)
         {
            outstream << descriptor_score_ptr->get(b) << " " << flush;
         }

         if (video_frames_input_flag)
         {
            outstream << "clip_"+stringfunc::integer_to_string(first_clip_ID,4)
               +"_frame-"+stringfunc::integer_to_string(first_frame_ID,5) 
                      << " ";
            outstream << "clip_"+stringfunc::integer_to_string(
               second_clip_ID,4)
               +"_frame-"+stringfunc::integer_to_string(second_frame_ID,5) 
                      << endl;

            filesstream << image_pair_counter 
                        << "  clip_"+stringfunc::integer_to_string(
                           first_clip_ID,4)
               +"_frame-"+stringfunc::integer_to_string(first_frame_ID,5) 
                      << " ";
            filesstream << "  clip_"+stringfunc::integer_to_string(
                           second_clip_ID,4)
               +"_frame-"+stringfunc::integer_to_string(second_frame_ID,5) 
                      << endl;
         }
         else
         {
            outstream << "pic"+stringfunc::integer_to_string(first_frame_ID,5)
                      << " ";
            outstream << "pic"+stringfunc::integer_to_string(
               second_frame_ID,5) << endl;
            filesstream << image_pair_counter 
                        << "  pic"+stringfunc::integer_to_string(
                           first_frame_ID,5) << " ";
            filesstream << "  pic"+stringfunc::integer_to_string(
               second_frame_ID,5) << endl;
         }
         image_pair_counter++;
         
      } // loop over iter
   
      filefunc::closefile(output_filename,outstream);
      filefunc::closefile(files_filename,outstream);

      string banner="Exported "+output_filename;
      outputfunc::write_big_banner(banner);

      banner="Exported "+files_filename;
      outputfunc::write_big_banner(banner);

      vector<string> subdir_strings=
         stringfunc::decompose_string_into_substrings(
            root_subdir,"/");
      string second_output_filename=root_subdir+"matching_images_"+
         subdir_strings.back()+"_"+
         stringfunc::number_to_string(image_pair_counter)+".dat";
      if (f==1)
      {
         second_output_filename=root_subdir+"nonmatching_images_"+
            subdir_strings.back()+"_"+
            stringfunc::number_to_string(image_pair_counter)+".dat";
      }
      string unix_cmd="ln -s "+filefunc::getbasename(output_filename)+" "
         +second_output_filename;
//      cout << "unix_cmd = " << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);

   } // loop over index f labeling input image filenames

   cout << endl;
   cout << "At end of program GENERATE_BOW_SCORES" << endl;
   outputfunc::print_elapsed_time();
}

