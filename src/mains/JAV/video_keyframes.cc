// ==========================================================================
// Program VIDEO_KEYFRAMES imports color, texture and LBP image
// matching scores for some set of video frame pairs.  It declares the
// very first video frame to be a "keyframe".  The program then
// requires all subsequent keyframes to be nontrivially different in
// terms of their color, texture and LBP contents relative to all
// preceding keyframes.  VIDEO_KEYFRAMES creates soft links to video
// frames declared to be key within a keyframes/ subdirectory of the
// root video directory.  It also generates (but unfortunately cannot
// run) an executable script to form an AVI movie of the video
// keyframes.

//			  ./video_keyframes

// ==========================================================================
// Last updated on 10/27/13; 10/30/13; 12/21/13
// ==========================================================================

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "templates/mytemplates.h"
#include "general/outputfuncs.h"
#include "math/prob_distribution.h"
#include "video/RGB_analyzer.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ios;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
   timefunc::initialize_timeofday_clock();      

   string ImageEngine_subdir="/data/ImageEngine/";
   string BostonBombing_subdir=ImageEngine_subdir+"BostonBombing/";
   string JAV_subdir=BostonBombing_subdir+"clips_1_thru_25/";
   string root_subdir=JAV_subdir;
   string images_subdir=root_subdir+"jpg_frames/";

/*
//   string JAV_subdir="/data/video/JAV/NewsWraps/early_Sep_2013/";
   string JAV_subdir="/data/video/JAV/NewsWraps/w_transcripts/";
//   string JAV_subdir="/data/video/JAV/UIUC_Broadcast_News/";
   string root_subdir=JAV_subdir;
   string images_subdir=root_subdir+"jpg_frames/";
   string keyframes_subdir=images_subdir+"keyframes/";
*/


/*
//   string JAV_subdir="/data/video/JAV/NewsWraps/early_Sep_2013/";
   string JAV_subdir="/data/video/JAV/NewsWraps/w_transcripts/";
//   string JAV_subdir="/data/video/JAV/UIUC_Broadcast_News/";
   string root_subdir=JAV_subdir;
   string images_subdir=root_subdir+"jpg_frames/";
*/


   string keyframes_subdir=images_subdir+"keyframes/";
   filefunc::dircreate(keyframes_subdir);
   filefunc::purge_files_in_subdir(keyframes_subdir);

   vector<string> image_filenames=filefunc::image_files_in_subdir(
      images_subdir);

// Import list of similarly colored image pairs:

//   string color_histograms_subdir=root_subdir+"color_histograms/";
//   string image_colors_comparison_filename=color_histograms_subdir+
//      "image_colors_sameclips.comparison";
   string color_histograms_subdir=root_subdir+"global_color_histograms/";
   string image_colors_comparison_filename=color_histograms_subdir+
      "image_global_colors_sameclips.comparison";
//      "image_global_colors_sameclips.comparison.keyframes";
   
   vector<vector<string> > row_substrings;
   filefunc::ReadInSubstrings(
      image_colors_comparison_filename,row_substrings);

// Fill STL map with pairs of similarly-colored image filename prefixes:

   typedef map<string,double> IMAGEPAIR_COLOR_MATCHES_MAP;
   IMAGEPAIR_COLOR_MATCHES_MAP imagepair_color_matches_map;
// independent string = concatenated image pair filename prefixes
// dependent double = image pair matching score   

   for (int r=0; r<row_substrings.size(); r++)
   {
      double color_match_score=stringfunc::string_to_number(
         row_substrings[r].at(0));
      string image_filename1_prefix=row_substrings[r].at(1);
      string image_filename2_prefix=row_substrings[r].at(2);
      string imagepair_prefixes=image_filename1_prefix+" "+
         image_filename2_prefix;
//      cout << "r = " << r 
//           << " imagepair_prefixes = " << imagepair_prefixes << endl;
      imagepair_color_matches_map[imagepair_prefixes]=color_match_score;
   }
   cout << "imagepair_color_matches_map.size() = "
        << imagepair_color_matches_map.size() << endl;

// Import list of similarly textured image pairs:

   string texture_histograms_subdir=root_subdir+"texture_histograms/";
   string image_textures_comparison_filename=texture_histograms_subdir+
      "image_textures_sameclips.comparison";
//      "image_textures_sameclips.comparison.keyframes";
   
   row_substrings.clear();
   filefunc::ReadInSubstrings(
      image_textures_comparison_filename,row_substrings);

// Fill STL map with pairs of similarly-textured image filename prefixes:

   typedef map<string,double> IMAGEPAIR_TEXTURE_MATCHES_MAP;
   IMAGEPAIR_TEXTURE_MATCHES_MAP imagepair_texture_matches_map;
// independent string = concatenated image pair filename prefixes
// dependent double = image pair matching score   

   for (int r=0; r<row_substrings.size(); r++)
   {
      double texture_match_score=stringfunc::string_to_number(
         row_substrings[r].at(0));
      string image_filename1_prefix=row_substrings[r].at(1);
      string image_filename2_prefix=row_substrings[r].at(2);
      string imagepair_prefixes=image_filename1_prefix+" "+
         image_filename2_prefix;
      imagepair_texture_matches_map[imagepair_prefixes]=texture_match_score;
   }
   cout << "imagepair_texture_matches_map.size() = "
        << imagepair_texture_matches_map.size() << endl;

// Import list of similarly LBPed image pairs:

   string LBP_histograms_subdir=root_subdir+"LBP_histograms/";
   string image_LBPs_comparison_filename=LBP_histograms_subdir+
      "image_LBPs_sameclips.comparison";
//      "image_LBPs_sameclips.comparison.keyframes";
   
   row_substrings.clear();
   filefunc::ReadInSubstrings(
      image_LBPs_comparison_filename,row_substrings);

// Fill STL map with pairs of similarly-LBPed image filename prefixes:

   typedef map<string,double> IMAGEPAIR_LBP_MATCHES_MAP;
   IMAGEPAIR_LBP_MATCHES_MAP imagepair_LBP_matches_map;
// independent string = concatenated image pair filename prefixes
// dependent double = image pair matching score   

   for (int r=0; r<row_substrings.size(); r++)
   {
      double LBP_match_score=stringfunc::string_to_number(
         row_substrings[r].at(0));
      string image_filename1_prefix=row_substrings[r].at(1);
      string image_filename2_prefix=row_substrings[r].at(2);
      string imagepair_prefixes=image_filename1_prefix+" "+
         image_filename2_prefix;
      imagepair_LBP_matches_map[imagepair_prefixes]=LBP_match_score;
   }
   cout << "imagepair_LBP_matches_map.size() = "
        << imagepair_LBP_matches_map.size() << endl;

// Instantiate STL map to hold keyframe filename prefixes:

   typedef map<int,string> KEYFRAME_ID_PREFIX_MAP;
   KEYFRAME_ID_PREFIX_MAP keyframe_id_prefix_map;

// Take very first video frame to be a keyframe:

   int curr_keyframe_ID=0;
   string curr_image_basename=filefunc::getbasename(
      image_filenames[curr_keyframe_ID]);
   string curr_image_prefix=stringfunc::prefix(curr_image_basename);
   keyframe_id_prefix_map[curr_keyframe_ID]=curr_image_prefix;

// Sequentially consider video frames.  Declare current frame to be a
// new keyframe if its color, texture and LBP contents differ
// significantly from all previous keyframes:

   string prev_image_prefix=curr_image_prefix;
   for (int i=1; i<image_filenames.size(); i++)
   {
      string curr_image_basename=filefunc::getbasename(image_filenames[i]);
      string curr_image_prefix=stringfunc::prefix(curr_image_basename);

      cout << "i = " << i << " prefix = " << curr_image_prefix << endl;

      bool continue_flag=false;
      for (KEYFRAME_ID_PREFIX_MAP::iterator iter=
              keyframe_id_prefix_map.begin(); 
           iter != keyframe_id_prefix_map.end(); iter++)
      {
         string prev_image_prefix=iter->second;
         string imagepair_prefixes=prev_image_prefix+" "+curr_image_prefix;
//         cout << "imagepair_prefixes = " << imagepair_prefixes << endl;

         IMAGEPAIR_COLOR_MATCHES_MAP::iterator color_iter=
            imagepair_color_matches_map.find(imagepair_prefixes);
         IMAGEPAIR_TEXTURE_MATCHES_MAP::iterator texture_iter=
            imagepair_texture_matches_map.find(imagepair_prefixes);
         IMAGEPAIR_LBP_MATCHES_MAP::iterator LBP_iter=
            imagepair_LBP_matches_map.find(imagepair_prefixes);

         if ( color_iter != imagepair_color_matches_map.end())
         {
            double colormatch_score=color_iter->second;
            cout << "     colormatch_score = " << colormatch_score << endl;
            continue_flag=true;
            break;
         }

         if ( texture_iter != imagepair_texture_matches_map.end()) 
         {
            double texturematch_score=texture_iter->second;
            cout << "     texturematch_score = " << texturematch_score << endl;
            continue_flag=true;
            break;
         }
         
         if ( LBP_iter != imagepair_LBP_matches_map.end()) 
         {
            double LBPmatch_score=LBP_iter->second;
            cout << "     LBPmatch_score = " << LBPmatch_score << endl;
            continue_flag=true;
            break;
         }
      } // iterator loop over keyframe_id_prefx_map 
      
      if (continue_flag) continue;

      curr_keyframe_ID=i;
      keyframe_id_prefix_map[curr_keyframe_ID]=curr_image_prefix;

      prev_image_prefix=curr_image_prefix;
      
   } // loop over index i labeling current image filenames
   cout << "keyframe_id_prefix_map.size() = "
        << keyframe_id_prefix_map.size() << endl;

   for (KEYFRAME_ID_PREFIX_MAP::iterator iter=keyframe_id_prefix_map.begin(); 
        iter != keyframe_id_prefix_map.end(); iter++)
   {
      int keyframe_ID=iter->first;
      string keyframe_prefix=iter->second;
//      cout << keyframe_ID << "  " << keyframe_prefix << endl;
      string curr_keyframe=images_subdir+keyframe_prefix+".jpg";
//      string unix_cmd="cp "+curr_keyframe+" "+keyframes_subdir;
      string unix_cmd="ln -s "+curr_keyframe+" "+keyframes_subdir;
//      cout << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);
   }

   int n_images=image_filenames.size();
   int n_keyframes=keyframe_id_prefix_map.size();
   cout << "N_frames = " << n_images << endl;
   cout << "N_keyframes = " << n_keyframes << endl;
   cout << "n_keyframes/n_frames = " << double(n_keyframes)/n_images << endl;
   string output_filename=keyframes_subdir+"keyframes.stats";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);
   outstream << "N_frames = " << n_images << endl;
   outstream << "N_keyframes = " << n_keyframes << endl;
   double keyframe_ratio=double(n_keyframes)/n_images;
   outstream << "n_keyframes/n_frames = " << keyframe_ratio << endl;
   outstream << "Keyframe compression = " 
             << basic_math::round(100*(1-keyframe_ratio)) << "%" << endl;
   filefunc::closefile(output_filename,outstream);

   string banner="Video keyframes links generated within "+keyframes_subdir;
   outputfunc::write_big_banner(banner);

   string orig_AVI_filename="origframes.avi";
   string unix_cmd="mkmpeg4 -v -f 15 -b 24000000 -o "+orig_AVI_filename+
      " *.jpg";
   output_filename=images_subdir+"generate_origframes_avi";
   outstream;
   filefunc::openfile(output_filename,outstream);
   outstream << unix_cmd << endl;
   filefunc::closefile(output_filename,outstream);
   filefunc::make_executable(output_filename);
   banner="Exported executable script "+output_filename;
   outputfunc::write_banner(banner);
   
   string keyframe_AVI_filename="keyframes.avi";
   unix_cmd="mkmpeg4 -v -f 5 -b 24000000 -o "+keyframe_AVI_filename+" *.jpg";
   output_filename=keyframes_subdir+"generate_keyframes_avi";
   filefunc::openfile(output_filename,outstream);
   outstream << unix_cmd << endl;
   filefunc::closefile(output_filename,outstream);
   filefunc::make_executable(output_filename);
   banner="Exported executable script "+output_filename;
   outputfunc::write_banner(banner);   

   cout << "At end of program IDENTIFY_KEYFRAMES" << endl;
   outputfunc::print_elapsed_time();
}


