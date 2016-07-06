// ==========================================================================
// Program NEAR_DUPLICATES imports comparison scores for pairs of
// images based upon their color histogram, gist descriptor and
// texture histogram dotproducts.  It identifies image pairs which
// have color, GIST and texture scores nearly equal to unity.  A
// montage of such nearly identical images is generated and displayed.
// The montage file is stored within a near_duplicates subdir of
// JAV_subdir.

//			   ./near_duplicates

// ==========================================================================
// Last updated on 10/4/13; 10/6/13; 10/8/13
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

   typedef std::map<quadruple,threevector,ltquadruple> 
      GLOBAL_DESCRIPTOR_SCORES_MAP;
   GLOBAL_DESCRIPTOR_SCORES_MAP global_descriptor_scores_map;
   GLOBAL_DESCRIPTOR_SCORES_MAP::iterator iter;

   string ImageEngine_subdir="/data/ImageEngine/";
   string tidmarsh_subdir=ImageEngine_subdir+"tidmarsh/";
   string images_subdir=tidmarsh_subdir;
   bool video_clip_data_flag=false;

//   string JAV_subdir=
//      "/data/video/JAV/NewsWraps/early_Sep_2013/";
//   string images_subdir=JAV_subdir+"jpg_frames/";
//   bool video_clip_data_flag=true;

//   string root_subdir=JAV_subdir;
   string root_subdir=tidmarsh_subdir;

   string color_histograms_subdir=root_subdir+"color_histograms/";
   string gist_subdir=root_subdir+"gist_files/";
   string texture_histograms_subdir=root_subdir+"texture_histograms/";
   string near_duplicates_subdir=root_subdir+"near_duplicates/";
   filefunc::dircreate(near_duplicates_subdir);

   string color_comparison_filename=color_histograms_subdir+
      "image_colors.comparison";
   string gist_comparison_filename=gist_subdir+
      "image_gists.comparison";
   string texture_comparison_filename=texture_histograms_subdir+
      "image_textures.comparison";

   string output_filename;
   vector<vector<string> > substrings;

   cout << endl;
   cout << "Importing global image descriptors:" << endl;
   cout << endl;

   int n_descriptors=3;
   for (int descriptor=0; descriptor<n_descriptors; descriptor++)
   {
      cout << "descriptor = " << descriptor << endl;
      substrings.clear();
      if (descriptor==0)
      {
         output_filename=color_histograms_subdir+"image_colors.dat";
         substrings=filefunc::ReadInSubstrings(color_comparison_filename);
      }
      else if (descriptor==1)
      {
         output_filename=gist_subdir+"image_gists.dat";
         substrings=filefunc::ReadInSubstrings(gist_comparison_filename);
      }
      else if (descriptor==2)
      {
         output_filename=texture_histograms_subdir+"image_textures.dat";
         substrings=filefunc::ReadInSubstrings(texture_comparison_filename);
      }
      
      string separator_chars=" _-";
      for (unsigned int i=0; i<substrings.size(); i++)
      {
//      cout << "i = " << i << " " << flush;
         double score=stringfunc::string_to_number(
            substrings[i].at(0));

         int first_clip_ID=0;
         int second_clip_ID=0;
         int first_frame_ID,second_frame_ID;
         if (video_clip_data_flag)
         {
            vector<string> subsubstrings=
               stringfunc::decompose_string_into_substrings(
                  substrings[i].at(1),separator_chars);
            first_clip_ID=stringfunc::string_to_number(subsubstrings[1]);
            first_frame_ID=stringfunc::string_to_number(subsubstrings[3]);
         }
         else
         {
            string curr_substr=substrings[i].at(1);
            first_frame_ID=stringfunc::string_to_number(
               curr_substr.substr(3,curr_substr.size()-3));
         }

         if (video_clip_data_flag)
         {
            vector<string> subsubstrings=
               stringfunc::decompose_string_into_substrings(
                  substrings[i].at(2),separator_chars);
            second_clip_ID=stringfunc::string_to_number(subsubstrings[1]);
            second_frame_ID=stringfunc::string_to_number(subsubstrings[3]);
         }
         else
         {
            string curr_substr=substrings[i].at(2);
            second_frame_ID=stringfunc::string_to_number(
               curr_substr.substr(3,curr_substr.size()-3));
         }

/*
         vector<string> subsubstrings=
            stringfunc::decompose_string_into_substrings(
               substrings[i].at(1),separator_chars);
         int first_clip_ID=stringfunc::string_to_number(subsubstrings[1]);
         int first_frame_ID=stringfunc::string_to_number(subsubstrings[3]);

         subsubstrings=
            stringfunc::decompose_string_into_substrings(
               substrings[i].at(2),separator_chars);
         int second_clip_ID=stringfunc::string_to_number(subsubstrings[1]);
         int second_frame_ID=stringfunc::string_to_number(subsubstrings[3]);
*/
      
         quadruple curr_q(
            first_clip_ID,first_frame_ID,second_clip_ID,second_frame_ID);
//         cout << "curr_q = " << curr_q << endl;

         iter=global_descriptor_scores_map.find(curr_q);
         if (iter==global_descriptor_scores_map.end())
         {
            threevector descriptor_score(-1,-1,-1);
            descriptor_score.put(descriptor,score);
//            cout << "Descriptor_score = " << descriptor_score << endl;
            global_descriptor_scores_map[curr_q]=descriptor_score;
         }
         else
         {
            iter->second.put(descriptor,score);
         }
      }
   }

   int counter=0;
   int n_descriptor_scores=global_descriptor_scores_map.size();
   for (iter=global_descriptor_scores_map.begin(); iter != 
           global_descriptor_scores_map.end(); iter++)
   {
      outputfunc::update_progress_fraction(
         counter++,n_descriptor_scores/100,n_descriptor_scores);
      double counter_frac=double(counter)/n_descriptor_scores;

      double color_score=iter->second.get(0);
      double gist_score=iter->second.get(1);
      double texture_score=iter->second.get(2);

      if (color_score < 0 || gist_score < 0 || texture_score < 0) continue;

      if (color_score < 0.95) continue;
//      if (color_score < 0.99) continue;
      if (gist_score < 0.99) continue;
      if (texture_score < 0.99) continue;

      int first_clip_ID=iter->first.first;
      int first_frame_ID=iter->first.second;
      int second_clip_ID=iter->first.third;
      int second_frame_ID=iter->first.fourth;

      string first_image_filename,second_image_filename;
      if (video_clip_data_flag)
      {
         first_image_filename=images_subdir+
            "clip_"+stringfunc::integer_to_string(first_clip_ID,4)+"_frame-"+
            stringfunc::integer_to_string(first_frame_ID,5)+".jpg";
         second_image_filename=images_subdir+
            "clip_"+stringfunc::integer_to_string(second_clip_ID,4)+"_frame-"+
            stringfunc::integer_to_string(second_frame_ID,5)+".jpg";
      }
      else
      {
         first_image_filename=images_subdir+
            "pic"+stringfunc::integer_to_string(first_frame_ID,5)+".jpg";
         second_image_filename=images_subdir+
            "pic"+stringfunc::integer_to_string(second_frame_ID,5)+".jpg";
      }

      string unix_cmd="montageview "+first_image_filename+" "+
         second_image_filename;
      sysfunc::unix_command(unix_cmd);

      cout << "----------------------------------------------------" << endl;
      cout << "counter = " << counter << " counter_frac = " << counter_frac 
           << endl;
      cout << "first_image_filename = " 
           << filefunc::getbasename(first_image_filename) << endl;
      cout << "second_image_filename = " 
           << filefunc::getbasename(second_image_filename) << endl;

      cout << iter->first << "  "
           << color_score << " "
           << gist_score << " "
           << texture_score << endl;

//      usleep(300*1000);

      string substring="montage_";
      vector<string> montage_filenames=
         filefunc::files_in_subdir_matching_substring(
            "./",substring);
      for (unsigned int m=0; m<montage_filenames.size(); m++)
      {
         string unix_cmd="mv "+montage_filenames[m]+" "+
            near_duplicates_subdir;
         sysfunc::unix_command(unix_cmd);
      }

      vector<int> process_IDs=sysfunc::my_get_pid("display");
      for (unsigned int p=0; p<process_IDs.size(); p++)
      {
//         cout << "p = " << p << " process_ID = " << process_IDs[p] << endl;
         string unix_cmd="kill -9 "+
            stringfunc::number_to_string(process_IDs[p]);
         sysfunc::unix_command(unix_cmd);
      }
   } // loop over global_descriptor_scores_map iterator
   
   cout << "At end of program NEAR_DUPLICATES" << endl;
   outputfunc::print_elapsed_time();
}

