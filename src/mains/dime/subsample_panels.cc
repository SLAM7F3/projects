// =======================================================================
// Program SUBSAMPLE_PANELS loops over all 36-degree WISP "panel" PNG
// files.  Using ImageMagick's convert command, it resizes each panel
// image from 4Kx2.2K down to 1Kx0.5K.  

//				subsample_panels

// ========================================================================
// Last updated on 3/26/13; 6/21/13; 7/11/13
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "video/photogroup.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::string;
using std::vector;

// ==========================================================================
int main( int argc, char** argv )
{
   std::set_new_handler(sysfunc::out_of_memory);
   timefunc::initialize_timeofday_clock();

   string bundler_subdir="./bundler/DIME/";
   string MayFieldtest_subdir=bundler_subdir+"May2013_Fieldtest/";
   string FSFdate_subdir=MayFieldtest_subdir+"05202013/";
   cout << "FSFdate_subdir = " << FSFdate_subdir << endl;

   int scene_ID;
   cout << "Enter scene ID:" << endl;
   cin >> scene_ID;
   string scene_ID_str=stringfunc::integer_to_string(scene_ID,2);
   string bundler_IO_subdir=FSFdate_subdir+"Scene"+scene_ID_str+"/";
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;

   string panels_subdir=bundler_IO_subdir+"stable_frames/panels/";
   string subsampled_panels_subdir=panels_subdir+"subsampled_panels/";
   filefunc::dircreate(subsampled_panels_subdir);

   vector<string> panel_filenames=filefunc::image_files_in_subdir(
      panels_subdir);
   for (unsigned int i=0; i<panel_filenames.size(); i++)
   {
      if (i%25==0)
      {
         double progress_frac=double(i)/panel_filenames.size();
         cout << "Fraction of panels subsampled = " 
              << progress_frac << endl;
         outputfunc::print_elapsed_and_remaining_time(progress_frac);
      }

      string basename=filefunc::getbasename(panel_filenames[i]);
      string subsampled_filename=subsampled_panels_subdir+
         "subsampled_"+basename;
      string unix_cmd="convert -resize 25% "+panel_filenames[i]+" "+
         subsampled_filename;
      sysfunc::unix_command(unix_cmd);

   }
   cout << endl;

   string banner="Finished running program SUBSAMPLE_PANELS";
   outputfunc::write_big_banner(banner);
   outputfunc::print_elapsed_time();   
}

