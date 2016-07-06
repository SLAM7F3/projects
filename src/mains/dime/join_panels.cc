// ========================================================================
// Program JOIN_PANELS concatenates together subsampled "panel" images
// into a single, larger image.  We wrote this program in order to
// join together FSF panels so that the Athena ship could always be
// seen somewhere within the concatenation over an entire pass.
// ========================================================================
// Last updated on 6/24/13
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"

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

   string stable_frames_subdir=bundler_IO_subdir+"stable_frames/";
   string panels_subdir=stable_frames_subdir+"panels/";
   string subsampled_panels_subdir=panels_subdir+"subsampled_panels/";

   cout << "subsampled_panels_subdir = " 
        << subsampled_panels_subdir << endl;
   string joined_panels_subdir=subsampled_panels_subdir+"joined_panels/";
   filefunc::dircreate(joined_panels_subdir);

   int n_start=1;
   cout << "Enter starting pano number:" << endl;
   cin >> n_start;

   int n_stop;
   cout << "Enter stopping pano number:" << endl;
   cin >> n_stop;

   int p_start=2;
   int p_stop=5;
   
   ofstream outstream;
   string output_filename="merge_panels.script";
   filefunc::openfile(output_filename,outstream);

   for (int n=n_start; n<=n_stop; n++)
   {
      string unix_cmd="convert ";
      string p_all="p";
      for (int p=p_start; p<=p_stop; p++)
      {
         unix_cmd += "subsampled_stable_p"+stringfunc::number_to_string(p);
         unix_cmd += "_uvcorrected_wisp_res0_";
         unix_cmd += stringfunc::integer_to_string(n,5)+".png ";
         p_all += stringfunc::number_to_string(p);
      } // loop over index p labeling panels to be joined

      string joined_panel_filename="./joined_panels/"+p_all+"_"+
         stringfunc::integer_to_string(n,5)+".png";
      unix_cmd += "+append '"+joined_panel_filename+"'";

      outstream << unix_cmd << endl;
   } // loop over index n labeling pano frames

   filefunc::closefile(output_filename,outstream);

   string unix_cmd="chmod a+x "+output_filename;
   sysfunc::unix_command(unix_cmd);

   string banner="Exported panel joining commands to "+output_filename;
   outputfunc::write_big_banner(banner);
   
}

