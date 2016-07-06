// =======================================================================
// Program LINK_VIDEO_FILENAMES imports a set of mp4, real-media or
// Advanced System Format video clips from a specified subdirectory.
// It first alphabetically sorts the input video clips' filenames.  It
// next wraps the video filenames in double quotes in order to shield
// subsequent link commands from any white spaces.
// LINK_VIDEO_FILENAMES generates an executable script which links the
// input mp4 files to homogenized output filenames.

// If text transcripts for the videos' sound tracks exist within the
// same subdirectories as the raw media files, LINK_VIDEO_FILENAMES
// generates an executable script which links the text files to
// homogenized output filenames.

// =======================================================================
// Last updated on 10/22/13; 10/29/13; 10/30/13
// =======================================================================

#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

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

//   string videos_subdir="/data/video/JAV/NewsWraps/early_Sep_2013/";
   string videos_subdir=
      "/data/video/JAV/NewsWraps/w_transcripts/raw_mp4s_and_transcripts/";
//   string videos_subdir="/data/video/JAV/UIUC_Broadcast_News/";

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("mp4");
   allowed_suffixes.push_back("rm");
   allowed_suffixes.push_back("asf");
   bool search_all_children_dirs_flag=true;

   vector<string> video_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,videos_subdir,
         search_all_children_dirs_flag);

// On 10/29/13, we discovered the painful way that video_filenames is
// generally NOT sorted alphabetically (or by date).  So we explicitly
// sort the strings within STL vector video_filenames:

   std::sort(video_filenames.begin(),video_filenames.end());

   string output_filename=videos_subdir+"link_YouTube_files";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);

   string transcript_filename=videos_subdir+"link_transcript_files";
   ofstream transcript_stream;
   filefunc::openfile(transcript_filename,transcript_stream);

   
   int clip_ID_offset=20;
   cout << "Enter starting clip's ID:" << endl;
   cin >> clip_ID_offset;
   
   for (int i=0; i<video_filenames.size(); i++)
   {
      string curr_video_filename=video_filenames[i];
      string video_subdir=filefunc::getdirname(curr_video_filename);
      string suffix=stringfunc::suffix(curr_video_filename);
      int clip_ID=i+clip_ID_offset;
      
      string homogenized_video_filename="clip_"+stringfunc::integer_to_string(
         clip_ID,4)+"."+suffix;
      string unix_cmd="ln -s \""+curr_video_filename
         +"\" "+homogenized_video_filename;
      outstream << unix_cmd << endl;

      string curr_transcript_filename=video_subdir+"transcript.txt";
      if (filefunc::fileexist(curr_transcript_filename))
      {
         string homogenized_transcript_filename=
            "clip_"+stringfunc::integer_to_string(clip_ID,4)+".transcript";
         unix_cmd="ln -s \""+curr_transcript_filename
            +"\" "+homogenized_transcript_filename;
         transcript_stream << unix_cmd << endl;
      }
   }
   filefunc::closefile(output_filename,outstream);
   filefunc::make_executable(output_filename);
   string banner="Exported "+output_filename;
   outputfunc::write_big_banner(banner);

   filefunc::closefile(transcript_filename,transcript_stream);
   filefunc::make_executable(transcript_filename);
   banner="Exported "+transcript_filename;
   outputfunc::write_big_banner(banner);
}
