// =======================================================================
// Program HOMOGENIZE_VIDEO_FILENAMES imports a set of mp4, real-media
// or Advanced System Format video clips from a user-specified
// subdirectory. It first alphabetically sorts the input video clips'
// filenames.  It next wraps the video filenames in double quotes in
// order to shield subsequent link commands from any white spaces.

// HOMOGENIZE_VIDEO_FILENAMES generates an executable script which
// links the input mp4 files to homogenized output filenames.  It then
// runs and deletes this script.

// =======================================================================
// Last updated on 10/30/13; 4/1/14; 4/2/14
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

   string videos_subdir;
   cout << endl;
   cout << "Enter full path for subdirectory containing video clip(s) to be ripped:"
        << endl;
   cout << " (e.g. /home/cho/programs/c++/svn/projects/src/mains/video/videos/ )" << endl;

   cin >> videos_subdir;
   filefunc::add_trailing_dir_slash(videos_subdir);
   string homogenized_videos_subdir=
      videos_subdir+"homogenized_video_filenames/";
   filefunc::dircreate(homogenized_videos_subdir);

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

   string output_filename="run_homogenize_video_filenames";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);

   int clip_ID_offset=0;
   cout << endl;
   cout << "Enter starting clip's ID:" << endl;
   cout << "  (e.g. 1)" << endl;
   cin >> clip_ID_offset;
   
   for (unsigned int i=0; i<video_filenames.size(); i++)
   {
      string curr_video_filename=video_filenames[i];
      string video_subdir=filefunc::getdirname(curr_video_filename);
      string suffix=stringfunc::suffix(curr_video_filename);
      int clip_ID=i+clip_ID_offset;
      
      string homogenized_video_filename=
         homogenized_videos_subdir+
         "clip_"+stringfunc::integer_to_string(
         clip_ID,4)+"."+suffix;
      string unix_cmd="ln -s \""+curr_video_filename
         +"\" "+homogenized_video_filename;
      outstream << unix_cmd << endl;
   }
   filefunc::closefile(output_filename,outstream);
   filefunc::make_executable(output_filename);

   sysfunc::unix_command(output_filename);
   filefunc::deletefile(output_filename);


   string banner="Homogenized video clip filenames links written to "+
      homogenized_videos_subdir;
   outputfunc::write_big_banner(banner);
}
