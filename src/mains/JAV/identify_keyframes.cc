// ==========================================================================
// Program IDENTIFY_KEYFRAMES is a minor variant of program
// VIDEO_KEYFRAMES.  It is intended to work with a set of videos that
// have already been entered into the IMAGERY database.
// IDENTIFY_KEYFRAMES exports SQL scripts which update the keyframes
// column within the videos table of the IMAGERY database.  All video
// frames are assigned to keyframe representatives.


// ==========================================================================
// Last updated on 10/31/13; 11/1/13; 12/21/13
// ==========================================================================

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "templates/mytemplates.h"
#include "general/outputfuncs.h"
#include "passes/PassesGroup.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "math/prob_distribution.h"
#include "video/RGB_analyzer.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"
#include "video/videosdatabasefuncs.h"

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

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);
//   int videopass_ID=passes_group.get_videopass_ID();
//   cout << "videopass_ID = " << videopass_ID << endl;
   string image_list_filename=passes_group.get_image_list_filename();
   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;

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


   vector<int> GISlayer_IDs=passes_group.get_GISlayer_IDs();
//   cout << "GISlayer_IDs.size() = " << GISlayer_IDs.size() << endl;

// Instantiate postgis database objects to send data to and retrieve
// data from external Postgres database:

   postgis_databases_group* postgis_databases_group_ptr=
      new postgis_databases_group;
   postgis_database* postgis_db_ptr=postgis_databases_group_ptr->
      generate_postgis_database_from_GISlayer_IDs(
         passes_group,GISlayer_IDs);
//   cout << "postgis_db_ptr = " << postgis_db_ptr << endl;

   int campaign_ID,mission_ID;
   cout << "Enter campaign_ID:" << endl;
   cin >> campaign_ID;

   cout << "Enter mission_ID:" << endl;
   cin >> mission_ID;

   string keyframes_subdir=images_subdir+"keyframes/";
   filefunc::dircreate(keyframes_subdir);
   cout << "Purging previous keyframe files:" << endl;
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

   for (unsigned int r=0; r<row_substrings.size(); r++)
   {
      double color_match_score=stringfunc::string_to_number(
         row_substrings[r].at(0));
      string image_filename1_prefix=row_substrings[r].at(1);
      string image_filename2_prefix=row_substrings[r].at(2);
      string imagepair_prefixes=image_filename1_prefix+" "+
         image_filename2_prefix;
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

   for (unsigned int r=0; r<row_substrings.size(); r++)
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

// Fill STL map with pairs of similarly-LBPd image filename prefixes:

   typedef map<string,double> IMAGEPAIR_LBP_MATCHES_MAP;
   IMAGEPAIR_LBP_MATCHES_MAP imagepair_LBP_matches_map;
// independent string = concatenated image pair filename prefixes
// dependent double = image pair matching score   

   for (unsigned int r=0; r<row_substrings.size(); r++)
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

// independent int = keyframe_ID
// dependent string = keyframe image basename prefix

// Instantiate STL map to hold relationships between non-keyframes
// with their closest keyframes:

   typedef map<int,int> NONKEYFRAME_KEYFRAME_MAP;
   NONKEYFRAME_KEYFRAME_MAP nonkeyframe_keyframe_map;

// independent int = nonkeyframe ID
// dependent int = closest keyframe ID

// Take very first video frame to be a keyframe:

   int curr_keyframe_ID=0;
   string curr_image_basename=filefunc::getbasename(
      image_filenames[curr_keyframe_ID]);
   string curr_image_prefix=stringfunc::prefix(curr_image_basename);
   keyframe_id_prefix_map[curr_keyframe_ID]=curr_image_prefix;

// Sequentially consider video frames.  Declare current frame to be a
// new keyframe if its color, texture and LBP contents differ
// significantly from all previous keyframes:

   cout << "image_filenames.size() = " << image_filenames.size() << endl;

   int matching_keyframe_ID=-1;
   string prev_image_prefix=curr_image_prefix;
   for (unsigned int i=1; i<image_filenames.size(); i++)
   {
      outputfunc::update_progress_fraction(i,100,image_filenames.size());
      string curr_image_basename=filefunc::getbasename(image_filenames[i]);
      string curr_image_prefix=stringfunc::prefix(curr_image_basename);

      bool nonkeyframe_flag=false;
      double max_combined_match_score=-1;
      for (KEYFRAME_ID_PREFIX_MAP::iterator iter=
              keyframe_id_prefix_map.begin(); 
           iter != keyframe_id_prefix_map.end(); iter++)
      {
         string prev_image_prefix=iter->second;
         string imagepair_prefixes=prev_image_prefix+" "+curr_image_prefix;

         IMAGEPAIR_COLOR_MATCHES_MAP::iterator color_iter=
            imagepair_color_matches_map.find(imagepair_prefixes);
         IMAGEPAIR_TEXTURE_MATCHES_MAP::iterator texture_iter=
            imagepair_texture_matches_map.find(imagepair_prefixes);
         IMAGEPAIR_LBP_MATCHES_MAP::iterator LBP_iter=
            imagepair_LBP_matches_map.find(imagepair_prefixes);

         double color_match_score,texture_match_score,LBP_match_score;
         color_match_score=texture_match_score=LBP_match_score=0;
         if ( color_iter != imagepair_color_matches_map.end())
         {
            color_match_score=color_iter->second;
            nonkeyframe_flag=true;
//            break;
         }

         if ( texture_iter != imagepair_texture_matches_map.end()) 
         {
            texture_match_score=texture_iter->second;
            nonkeyframe_flag=true;
//            break;
         }
         
         if ( LBP_iter != imagepair_LBP_matches_map.end()) 
         {
            LBP_match_score=LBP_iter->second;
            nonkeyframe_flag=true;
//            break;
         }

// Match non-keyframes with their closest keyframe:

         if (nonkeyframe_flag)	
         {
            double combined_match_score=color_match_score+texture_match_score+
               LBP_match_score;
            if (combined_match_score > max_combined_match_score)
            {
               max_combined_match_score=combined_match_score;
               matching_keyframe_ID=iter->first;
            }
         } // nonkeyframe_flag conditional

      } // iterator loop over keyframe_id_prefx_map 
      
      if (nonkeyframe_flag)
      {
         nonkeyframe_keyframe_map[i]=matching_keyframe_ID;
      }
      else
      {
         curr_keyframe_ID=i;
         keyframe_id_prefix_map[curr_keyframe_ID]=curr_image_prefix;
         prev_image_prefix=curr_image_prefix;
      }
   } // loop over index i labeling current image filenames
   cout << endl;

// Generate and export SQL script which identifies keyframes within
// videos table of IMAGERY database:
   
   string output_filename=keyframes_subdir+"update_keyframes.sql";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);
   
   for (KEYFRAME_ID_PREFIX_MAP::iterator iter=keyframe_id_prefix_map.begin(); 
        iter != keyframe_id_prefix_map.end(); iter++)
   {
//      int keyframe_ID=iter->first;
      string keyframe_prefix=iter->second;
//      cout << keyframe_ID << "  " << keyframe_prefix << endl;

// As of 10/30/13, we assume video images have names of the form
// clip_0020_frame-00002.jpg :

      string separator_chars="_-";
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         keyframe_prefix,separator_chars);
      int keyframe_clip_ID=stringfunc::string_to_number(substrings[1]);
      int keyframe_frame_ID=stringfunc::string_to_number(substrings[3]);
//      cout << "keyframe clip_ID = " << keyframe_clip_ID 
//           << " keyframe frame_ID = " << keyframe_frame_ID << endl;

      int image_ID=videosdatabasefunc::get_image_ID(
         postgis_db_ptr,campaign_ID,mission_ID,
         keyframe_clip_ID,keyframe_frame_ID);
//      cout << "image_ID = " << image_ID << endl;

      int offset=0;
      int keyframe_image_ID=image_ID+offset;
//      cout << "keyframe_image_ID = " << keyframe_image_ID << endl;

//      videosdatabasefunc::update_keyframe_ID(
//         postgis_db_ptr,campaign_ID,mission_ID,image_ID,keyframe_image_ID);
      string SQL_cmd=videosdatabasefunc::generate_update_keyframe_SQL_command(
         campaign_ID,mission_ID,image_ID,keyframe_image_ID);
      outstream << SQL_cmd << endl;

// Create soft links for images identified as keyframes within
// keyframes_subdir:

      string curr_keyframe=images_subdir+keyframe_prefix+".jpg";
//      string unix_cmd="cp "+curr_keyframe+" "+keyframes_subdir;
      string unix_cmd="ln -s "+curr_keyframe+" "+keyframes_subdir;
//      cout << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);
   }
   filefunc::closefile(output_filename,outstream);

   string banner="Video keyframes links generated within "+keyframes_subdir;
   outputfunc::write_big_banner(banner);

   banner="Exported SQL script for updating keyframe IDs to "+
      output_filename;
   outputfunc::write_banner(banner);

// Generate and export SQL script which assigns nonkeyframes to
// keyframes within videos table of IMAGERY database:

   output_filename=keyframes_subdir+"update_nonkeyframes.sql";
   filefunc::openfile(output_filename,outstream);
   
   for (NONKEYFRAME_KEYFRAME_MAP::iterator iter=
           nonkeyframe_keyframe_map.begin(); 
        iter != nonkeyframe_keyframe_map.end(); iter++)
   {
      int nonkeyframe_index=iter->first;
      int keyframe_index=iter->second;

      string nonkeyframe_filename=image_filenames[nonkeyframe_index];
      string nonkeyframe_basename=filefunc::getbasename(nonkeyframe_filename);
      string nonkeyframe_prefix=stringfunc::prefix(nonkeyframe_basename);

// As of 10/30/13, we assume video images have names of the form
// clip_0020_frame-00002.jpg :

      string separator_chars="_-";
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         nonkeyframe_prefix,separator_chars);
      int nonkeyframe_clip_ID=stringfunc::string_to_number(substrings[1]);
      int nonkeyframe_frame_ID=stringfunc::string_to_number(substrings[3]);
      int nonkeyframe_image_ID=videosdatabasefunc::get_image_ID(
         postgis_db_ptr,campaign_ID,mission_ID,
         nonkeyframe_clip_ID,nonkeyframe_frame_ID);

      string keyframe_filename=image_filenames[keyframe_index];      
      string keyframe_basename=filefunc::getbasename(keyframe_filename);
      string keyframe_prefix=stringfunc::prefix(keyframe_basename);

      substrings=stringfunc::decompose_string_into_substrings(
         keyframe_prefix,separator_chars);
      int keyframe_clip_ID=stringfunc::string_to_number(substrings[1]);
      int keyframe_frame_ID=stringfunc::string_to_number(substrings[3]);
      int keyframe_image_ID=videosdatabasefunc::get_image_ID(
         postgis_db_ptr,campaign_ID,mission_ID,
         keyframe_clip_ID,keyframe_frame_ID);

      string SQL_cmd=videosdatabasefunc::generate_update_keyframe_SQL_command(
         campaign_ID,mission_ID,nonkeyframe_image_ID,keyframe_image_ID);
      outstream << SQL_cmd << endl;
   }
   filefunc::closefile(output_filename,outstream);
   banner="Exported SQL script for updating keyframe IDs for nonkeyframes to "+
      output_filename;
   outputfunc::write_banner(banner);
   
// Compute and export keyframe statistics to keyframe.stats text file:

   int n_images=image_filenames.size();
   int n_keyframes=keyframe_id_prefix_map.size();
   cout << "N_frames = " << n_images << endl;
   cout << "N_keyframes = " << n_keyframes << endl;
   cout << "n_keyframes/n_frames = " << double(n_keyframes)/n_images << endl;

   output_filename=keyframes_subdir+"keyframes.stats";
   filefunc::openfile(output_filename,outstream);
   outstream << "N_frames = " << n_images << endl;
   outstream << "N_keyframes = " << n_keyframes << endl;
   double keyframe_ratio=double(n_keyframes)/n_images;
   outstream << "n_keyframes/n_frames = " << keyframe_ratio << endl;
   outstream << "Keyframe compression = " 
             << basic_math::round(100*(1-keyframe_ratio)) << "%" << endl;
   filefunc::closefile(output_filename,outstream);

// Export scripts for generating AVI movies from original and keyframe
// images:

   string orig_AVI_filename="origframes.avi";
   string unix_cmd="mkmpeg4 -v -f 15 -b 24000000 -o "+orig_AVI_filename+
      " *.jpg";
   output_filename=images_subdir+"generate_origframes_avi";
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

/*
   vector<int> image_IDs,keyframe_IDs;
   videosdatabasefunc::get_keyframe_IDs(
      postgis_db_ptr,campaign_ID,mission_ID,image_IDs,keyframe_IDs);
   for (int i=0; i<image_IDs.size(); i++)
   {
      cout << "image_ID = " << image_IDs[i]
           << " keyframe_ID = " << keyframe_IDs[i] 
           << endl;
   }
*/
 
   cout << "At end of program IDENTIFY_KEYFRAMES" << endl;
   outputfunc::print_elapsed_time();
}


