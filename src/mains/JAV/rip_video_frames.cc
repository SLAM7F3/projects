// =======================================================================
// Program RIP_VIDEO_FRAMES imports a set of homogenized mp4,rm or asf
// video clips generated via program LINK_VIDEO_FRAMES.  For each
// clip, it executes an ffmpeg command which rips frames to JPG images
// at 1 Hz.
// =======================================================================
// Last updated on 10/2/13; 10/21/13; 10/22/13
// =======================================================================

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cout;
using std::endl;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

int main( int argc, char** argv ) 
{
   std::set_new_handler(sysfunc::out_of_memory);

//   string videos_subdir="/data/video/JAV/NewsWraps/early_Sep_2013/";
   string videos_subdir="/data/video/JAV/NewsWraps/w_transcripts/mp4/";
//   string videos_subdir="/data/video/JAV/UIUC_Broadcast_News/videos/";
   string substring="clip_";
   vector<string> video_filenames=
      filefunc::files_in_subdir_matching_substring(videos_subdir,substring);
   filefunc::dircreate(videos_subdir+"jpg_frames/");

   string output_filename=videos_subdir+"rip_frames";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);

   for (int i=0; i<video_filenames.size(); i++)
   {
      string curr_filename=filefunc::getbasename(video_filenames[i]);
      string prefix=stringfunc::prefix(curr_filename);
      string clip_ID_str=prefix.substr(5,prefix.size()-5);
      int clip_ID=stringfunc::string_to_number(clip_ID_str);

      string frame_filename="./jpg_frames/clip_"
         +stringfunc::integer_to_string(clip_ID,4)+"_frame-%05d.jpg";
      string unix_cmd="ffmpeg -i "+curr_filename+
         " -r 1 -f image2 "+frame_filename;
//      string unix_cmd="ffmpeg -i "+curr_filename+
//         " -r 1 -f image2 ./jpg_frames/frame-%05d.jpg";
      outstream << unix_cmd << endl;
   }
   filefunc::closefile(output_filename,outstream);
   filefunc::make_executable(output_filename);

   string banner="Exported "+output_filename;
   outputfunc::write_big_banner(banner);
}
