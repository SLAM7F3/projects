// ==========================================================================
// Program GENERATE_MATCHING_FILENAMES scans over montage images
// within a specified input subdirectory.  It extracts clip and frame
// IDs for matching image pairs. Full path filenames for matching
// image pairs are then exported to an output text file.

//		        ./generate_matching_filenames

// ==========================================================================
// Last updated on 12/26/13; 12/31/13; 1/9/14
// ==========================================================================

#include  <iostream>
#include  <map>
#include  <string>
#include  <vector>

#include "general/filefuncs.h"
#include "video/descriptorfuncs.h"
#include "image/imagefuncs.h"
#include "math/ltquadruple.h"
#include "math/mathfuncs.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "math/prob_distribution.h"
#include "datastructures/Quadruple.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(12);

   timefunc::initialize_timeofday_clock();      

// Import basic HOG BoW processing parameters:

   string gist_subdir="../gist/";
   string BoW_params_filename=gist_subdir+"BoW_params.dat";
   filefunc::ReadInfile(BoW_params_filename);

   bool video_frames_input_flag=
      stringfunc::string_to_boolean(filefunc::text_line[8]);
   cout << "video_frames_input_flag = " << video_frames_input_flag << endl;

   string JAV_subdir,input_matches_subdir;
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
//         input_matches_subdir="~/Desktop/Newswrap_keyframe_matches/";
         input_matches_subdir=filefunc::text_line[10];
      }
      else if (video_corpus_ID==2)
      {
         JAV_subdir="/data/video/JAV/NewsWraps/w_transcripts/";
//         input_matches_subdir="~/Desktop/Newswrap_keyframe_matches/";
         input_matches_subdir=filefunc::text_line[10];
      }
      else if (video_corpus_ID==3)
      {
         JAV_subdir=BostonBombing_subdir+"clips_1_thru_25/";
//         input_matches_subdir="~/Desktop/BB_keyframe_matches/";
         input_matches_subdir=filefunc::text_line[11];
      }
      else
      {
         exit(-1);
      }
   }
   else
   {
      JAV_subdir="./bundler/tidmarsh/";
//      input_matches_subdir="~/Desktop/tidmarsh_matches/";
      input_matches_subdir=filefunc::text_line[12];
   } // video_frames_input_flag conditional
   cout << "JAV_subdir = " << JAV_subdir << endl;

   string root_subdir=JAV_subdir;
   string images_subdir;
   if (video_frames_input_flag)
   {
      images_subdir=root_subdir+"jpg_frames/";
   }
   else
   {
      images_subdir=root_subdir+"standard_sized_images/";
   }

   vector<string> annotated_pair_filenames=filefunc::image_files_in_subdir(
      input_matches_subdir);
   int n_matching_pairs=annotated_pair_filenames.size();

   string matching_images_filename=root_subdir+
      stringfunc::number_to_string(n_matching_pairs)+
      "_matching_image_filenames.dat";
//   cout << "matching_images_filename = " << matching_images_filename << endl;
   
   ofstream outstream;
   filefunc::openfile(matching_images_filename,outstream);

   string separator_chars="_.";
   for (int i=0; i<n_matching_pairs; i++)
   {
//      cout << annotated_pair_filenames[i] << endl;
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         filefunc::getbasename(annotated_pair_filenames[i]),separator_chars);

//      for (int s=0; s<substrings.size(); s++)
//      {
//         cout << "s = " << s << " substrings[s] = " << substrings[s]
//              << endl;
//      }
//      outputfunc::enter_continue_char();

      string image1_filename,image2_filename;
      if (video_frames_input_flag)
      {
         if (substrings[0]=="annotated")
         {
            image1_filename=images_subdir+substrings[2]+"_"+
               substrings[3]+"_"+substrings[4]+".jpg";
            image2_filename=images_subdir+substrings[5]+"_"+
               substrings[6]+"_"+substrings[7]+".jpg";

         }
         else if (substrings[0]=="montage")
         {
            image1_filename=images_subdir+substrings[1]+"_"+
               substrings[2]+"_"+substrings[3]+".jpg";
            image2_filename=images_subdir+substrings[4]+"_"+
               substrings[5]+"_"+substrings[6]+".jpg";
         }
         else if (substrings[2]=="montage")
         {
            image1_filename=images_subdir+substrings[3]+"_"+
               substrings[4]+"_"+substrings[5]+".jpg";
            image2_filename=images_subdir+substrings[6]+"_"+
               substrings[7]+"_"+substrings[8]+".jpg";
         }
      }
      else
      {
         if (substrings[0]=="annotated")
         {
            image1_filename=images_subdir+substrings[2]+".jpg";
            image2_filename=images_subdir+substrings[3]+".jpg";
         }
         else
         {
            image1_filename=images_subdir+substrings[3]+".jpg";
            image2_filename=images_subdir+substrings[4]+".jpg";
         }
      }
      outstream << image1_filename << " " << image2_filename << endl;
   } // loop over index i labeing matching image pairs
   filefunc::closefile(matching_images_filename,outstream);

   string banner="Exported "+stringfunc::number_to_string(n_matching_pairs)+
      " matching image pair filenames to "+matching_images_filename;
   outputfunc::write_big_banner(banner);

   cout << "At end of program GENERATE_MATCHING_FILENAMES" << endl;
   outputfunc::print_elapsed_time();
}

