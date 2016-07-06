// ==========================================================================
// Program DECOMPOSE_VIDEO uses ffmpeg to split input video clips into
// individual output frames.

//		       		decompose_video

// Need to turn this into a clickable icon which Delsey can run much
// more easily and not at the command line...

// ==========================================================================
// Last updated on 6/29/10; 6/30/10; 7/18/10
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <map>
#include <string>
#include <vector>
#include "astro_geo/Clock.h"
#include "general/filefuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"


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
   
   sysfunc::clearscreen();
//   string video_clips_subdir=
//      "~/programs/c++/svn/projects/src/mains/tech_challenge/video_clips/";
   string video_clips_subdir="./";
   string frames_subdir=video_clips_subdir+"video_frames/";

   string command1="mkdir "+frames_subdir;
   cout << "command1 = " << command1 << endl;
   sysfunc::unix_command(command1);

   string video_filename;
   cout << "Enter video_filename:" << endl;
   cin >> video_filename;
   video_filename=video_clips_subdir+video_filename;

//   string video_filename=video_clips_subdir+"RC_airplane.MOV";
//   string video_filename="../RC_airplane.MOV";
//   string video_filename="../rover_panorama.flv";

   double fps=1;

   string command2="ffmpeg -i "+video_filename+" -copyts ";
   command2 += "-r "+stringfunc::number_to_string(fps)+" ";
   command2 += frames_subdir+"frame-%05d.jpg";
//   command2 += frames_subdir+"frame-%05d.png";
   cout << "command2 = " << command2 << endl;
   sysfunc::unix_command(command2);

   string banner="Video frames split apart into "+frames_subdir;
   outputfunc::write_big_banner(banner);
}
