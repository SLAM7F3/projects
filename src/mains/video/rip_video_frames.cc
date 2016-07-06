// =======================================================================
// Program RIP_VIDEO_FRAMES imports a set of homogenized mp4,rm or asf
// video clips generated via program HOMOGENIZE_VIDEO_FRAMES from some
// user-specified subdirectory.  For each clip, it executes an ffmpeg
// command which rips frames to JPG images at a user-specified frame rate.
// RIP_VIDEO_FRAMES then exports executable scripts for each input
// video clip which invoke program VPLAYER.

// =======================================================================
// Last updated on 10/21/13; 10/22/13; 4/1/14; 4/2/14
// =======================================================================

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "astro_geo/Clock.h"
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

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

   string videos_subdir;
   cout << endl;
   cout << "Enter subdirectory containing video clip(s) with homogenized filenames to be ripped:"
        << endl;
   cout << "  (e.g. /home/cho/programs/c++/svn/projects/src/mains/video/videos/homogenized_video_filenames/ )" << endl;
   cin >> videos_subdir;

   double frames_per_second;
   cout << endl;
   cout << "Enter rate at which frames should be ripped from clip(s):"
        << endl;
   cout << "  (e.g. 10 for ten frames per second)" << endl;
   cin >> frames_per_second;

   string substring="clip_";
   vector<string> video_filenames=
      filefunc::files_in_subdir_matching_substring(videos_subdir,substring);

   string output_filename=videos_subdir+"rip_frames";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);

   Clock clock;
   clock.current_local_time_and_UTC();
   string start_time = stringfunc::number_to_string(clock.get_year())+","
      +stringfunc::number_to_string(clock.get_month())+","
      +stringfunc::number_to_string(clock.get_day())+","
      +stringfunc::number_to_string(clock.get_UTC_hour())+","
      +stringfunc::number_to_string(clock.get_minute())+","
      +stringfunc::number_to_string(clock.get_seconds());
   cout << "start_time = " << start_time << endl;

   for (unsigned int i=0; i<video_filenames.size(); i++)
   {

/*
      cout << "i = " << i 
           << " video_filename = " << video_filenames[i] << endl;
      string unix_cmd = "mediainfo "+video_filenames[i];
      unix_cmd += " | awk '/Duration/ {split($2,a,\":\");";
      unix_cmd += "print a[1]*3600+a[2]*60+a[3]}'";
      cout << "unix_cmd = " << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);
      outputfunc::enter_continue_char();
*/

      string curr_filename=filefunc::getbasename(video_filenames[i]);
      string prefix=stringfunc::prefix(curr_filename);
      string clip_ID_str=prefix.substr(5,prefix.size()-5);
      int clip_ID=stringfunc::string_to_number(clip_ID_str);

      string clip_prefix="clip_"+stringfunc::integer_to_string(clip_ID,4);
      string clip_subdir=videos_subdir+clip_prefix+"/";
      filefunc::dircreate(clip_subdir);

      string frame_filename=clip_subdir+clip_prefix+"_frame-%05d.jpg";
      string unix_cmd="ffmpeg -i "+curr_filename+
         " -r "+stringfunc::number_to_string(frames_per_second)+
         " -f image2 "+frame_filename;
      outstream << unix_cmd << endl;

      string vplayer_script_filename="run_vplayer_"+clip_prefix;
      ofstream scriptstream;
      filefunc::openfile(vplayer_script_filename,scriptstream);
      scriptstream << "./vplayer \\" << endl;
      scriptstream << "--newpass "+clip_subdir+
         clip_prefix+"_frame-00001.jpg   \\" << endl;
      scriptstream << "--world_start_UTC 2013,9,27,16,8,54  \\" << endl;
      scriptstream << "--world_stop_UTC 2013,9,27,16,59,0   \\" << endl;
      scriptstream << "--world_time_step "+
         stringfunc::number_to_string(1.0/frames_per_second)+" \\" << endl;
      scriptstream << "--initial_mode Run_Movie_Mode " << endl;
      filefunc::closefile(vplayer_script_filename,scriptstream);
      filefunc::make_executable(vplayer_script_filename);
   }
   filefunc::closefile(output_filename,outstream);
   filefunc::make_executable(output_filename);

   string banner="Run executable script exported to "+output_filename;
   outputfunc::write_big_banner(banner);


   cout << "Then run any of the auto-generated vplayer scripts" << endl;
   cout << "within the present working directory" << endl;
   cout << "  (e.g. chant ./run_vplayer_clip_0001  )" << endl;
}
