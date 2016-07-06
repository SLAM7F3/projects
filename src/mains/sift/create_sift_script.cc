// ========================================================================
// Program CREATE_SIFT_SCRIPT is a little utility program which we
// wrote in order to generate an executable script for matching the
// transcripted "center" Nightline Boston Bombing video frames with a
// candidate "left" YouTube video.
// ========================================================================
// Last updated on 5/30/13; 6/1/13; 6/3/13
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main( int argc, char** argv )
{
/*
   int YouTube_start_frame,YouTube_stop_frame;
   cout << "Enter starting YouTube clip frame number:" << endl;
   cin >> YouTube_start_frame;
   cout << "Enter stopping YouTube clip frame number:" << endl;
   cin >> YouTube_stop_frame;

   string bundler_IO_subdir="./bundler/BostonBombing/NightLine_YT2/";
//   string bundler_IO_subdir="./bundler/BostonBombing/ICE_proposal/";
   string images_subdir=bundler_IO_subdir+"images/";
   vector<string> frame_filenames;
   for (int f=YouTube_start_frame; f<= YouTube_stop_frame; f++)
   {
      string curr_frame_filename=images_subdir+
         "YouTube_clip_0002_frame-"+
         stringfunc::integer_to_string(f,5)+".jpg";
//      cout << curr_frame_filename << endl;
      frame_filenames.push_back(curr_frame_filename);
   }

   int transcripted_start_frame,transcripted_stop_frame;
   cout << "Enter starting transcripted clip frame number:" << endl;
   cin >> transcripted_start_frame;
   cout << "Enter stopping transcripted clip frame number:" << endl;
   cin >> transcripted_stop_frame;

   for (int f=transcripted_start_frame; f<= transcripted_stop_frame; f++)
   {
      string curr_frame_filename=images_subdir+
         "transcripted_clip_0002_frame-"+
         stringfunc::integer_to_string(f,5)+".jpg";
//      cout << curr_frame_filename << endl;
      frame_filenames.push_back(curr_frame_filename);
   }
*/

   int Obama_start_frame,Obama_stop_frame;
   cout << "Enter starting Obama clip frame number:" << endl;
   cin >> Obama_start_frame;
   cout << "Enter stopping Obama clip frame number:" << endl;
   cin >> Obama_stop_frame;

   string bundler_IO_subdir="./bundler/BostonBombing/ICE_proposal/";
   string images_subdir=bundler_IO_subdir+"images/";
   vector<string> frame_filenames;
   for (int f=Obama_start_frame; f<= Obama_stop_frame; f++)
   {
      string curr_frame_filename=images_subdir+
         "Z_Obamaclip_frame-"+
         stringfunc::integer_to_string(f,5)+".jpg";
//      cout << curr_frame_filename << endl;
      frame_filenames.push_back(curr_frame_filename);
   }

   string scriptfilename="run_Obama_sift";
   ofstream scriptstream;
   filefunc::openfile(scriptfilename,scriptstream);

   scriptstream << "./asiftvid \\" << endl;
   for (int f=0; f<frame_filenames.size(); f++)
   {
      scriptstream << "--newpass "+frame_filenames[f]+" \\" << endl;
   }
   scriptstream << "--image_list_filename "
      +bundler_IO_subdir+"image_list.dat" << endl;

   filefunc::closefile(scriptfilename,scriptstream);
   string unix_command="chmod a+x "+scriptfilename;
   sysfunc::unix_command(unix_command);

   string banner="Exported "+scriptfilename;
   outputfunc::write_big_banner(banner);
}
