// =======================================================================
// Program SUBSAMPLE_PANOS loops over all images within some
// subdirectory containing either raw (or stabilized) panoramas.  Using
// ImageMagick's convert command, it resizes each WISP panorama from
// 40Kx2.2K pixels down to 3960x218.  An AVI movie is generated which
// contains all subsampled raw WISP panoramas.

//			       ./subsample_panos

// =======================================================================
// Last updated on 8/8/13; 8/12/13; 12/23/13
// =======================================================================

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "math/constant_vectors.h"
#include "general/filefuncs.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

int main( int argc, char** argv ) 
{
   std::set_new_handler(sysfunc::out_of_memory);
   timefunc::initialize_timeofday_clock();

   string pano_type_char;
//   cout << "Enter 's:' to subsample stabilized panos or 'r' to subsample raw panos" << endl;
//   cin >> pano_type_char;

   bool raw_pano_flag=true;
   if (pano_type_char=="s") raw_pano_flag=false;
   
   string date_string="05202013";
   cout << "Enter date string (e.g. 05202013 or 05222013):" << endl;
   cin >> date_string;
   filefunc::add_trailing_dir_slash(date_string);

   string images_subdir,subsampled_subdir;
   if (!raw_pano_flag)
   {
      string bundler_subdir="./bundler/DIME/";
      string MayFieldtest_subdir=bundler_subdir+"May2013_Fieldtest/";
//      string FSFdate_subdir=MayFieldtest_subdir+"05202013/";
      string FSFdate_subdir=MayFieldtest_subdir+date_string;
      cout << "FSFdate_subdir = " << FSFdate_subdir << endl;

      int scene_ID;
      cout << "Enter scene ID:" << endl;
      cin >> scene_ID;
      string scene_ID_str="Scene"+stringfunc::integer_to_string(scene_ID,2);
      string bundler_IO_subdir=FSFdate_subdir+scene_ID_str+"/";
      cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
      string stable_frames_subdir=bundler_IO_subdir+"stable_frames/";
      string subsampled_stable_frames_subdir=
         stable_frames_subdir+"subsampled/";

      images_subdir=stable_frames_subdir;
      subsampled_subdir=subsampled_stable_frames_subdir;
   }
   else
   {
      string DIME_subdir="/data/DIME/";
      string MayFieldtest_subdir=DIME_subdir+"panoramas/May2013_Fieldtest/";
//      string FSFdate_subdir=MayFieldtest_subdir+"05202013/";
      string FSFdate_subdir=MayFieldtest_subdir+date_string;
      cout << "FSFdate_subdir = " << FSFdate_subdir << endl;
      filefunc::dircreate(FSFdate_subdir);

      int scene_ID;
      cout << "Enter scene ID:" << endl;
      cin >> scene_ID;
      string scene_ID_str=stringfunc::integer_to_string(scene_ID,2);
      string panos_subdir=FSFdate_subdir+"Scene"+scene_ID_str+"/";
      cout << "panos_subdir = " << panos_subdir << endl;

// If program SUBSAMPLE_PANOS is being run directly after
// WISP360CALASSISTANT has converted .img files into JPG panorama
// files, then we first need to create a "raw/" subdirectory of
// panos_subdir and move all of the WISP panos into that subdirectory:

      string raw_images_subdir=panos_subdir+"raw/";
      if (!filefunc::direxist(raw_images_subdir))
      {
         filefunc::dircreate(raw_images_subdir);
         string unix_cmd="mv "+panos_subdir+"*.jpg "+raw_images_subdir;
//         cout << "unix_cmd = " << unix_cmd << endl;
         sysfunc::unix_command(unix_cmd);
      }

      string subsampled_raw_images_subdir=raw_images_subdir+"subsampled/";

      images_subdir=raw_images_subdir;
      subsampled_subdir=subsampled_raw_images_subdir;
   }

   cout << "subsampled_subdir = " << subsampled_subdir << endl;
   filefunc::dircreate(subsampled_subdir);

   vector<string> image_filenames=filefunc::image_files_in_subdir(
      images_subdir);
   cout << "image_filenames.size() = " << image_filenames.size() << endl;

   for (unsigned int i=0; i<image_filenames.size(); i++)
   {
      double progress_frac=double(i)/double(image_filenames.size());
      outputfunc::print_elapsed_and_remaining_time(progress_frac);

      string image_filename=image_filenames[i];
      string image_basename=filefunc::getbasename(image_filename);
      string subsampled_filename=subsampled_subdir
         +"subsampled_"+image_basename;

      string unix_cmd="convert -resize 9.9% "+image_filename+" "+
         subsampled_filename;
      sysfunc::unix_command(unix_cmd);

      string banner="Exported "+subsampled_filename;
      outputfunc::write_banner(banner);
   }

// Generate AVI movie from subsampled raw panoramas:

   if (raw_pano_flag)
   {
      string AVI_filename=subsampled_subdir+"raw_20fps.avi";
      string unix_cmd="cd "+subsampled_subdir+";";
      unix_cmd += "mkmpeg4 -v -f 20 -b 24000000 -o "+
         AVI_filename+" *.jpg";
      sysfunc::unix_command(unix_cmd);
      string banner="Exported subsampled AVI movie to "+AVI_filename;
      outputfunc::write_big_banner(banner);
   }
   
   string banner="Finished running program SUBSAMPLE_PANOS";
   outputfunc::write_big_banner(banner);
   outputfunc::print_elapsed_time();
}
