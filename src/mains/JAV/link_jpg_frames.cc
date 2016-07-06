// =======================================================================
// Program LINK_JPG_FRAMES is a specialized utility we wrote in order
// to rename video frames extracted from Boston Bombing video clips so
// that they conform to our JAV naming conventions.  
// =======================================================================
// Last updated on 12/21/13
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

   string ImageEngine_subdir="/data/ImageEngine/";
   string BostonBombing_subdir=ImageEngine_subdir+"BostonBombing/";
   string JAV_subdir=BostonBombing_subdir+"clips_1_thru_20/";
   string root_subdir=JAV_subdir;
   string images_subdir=root_subdir+"jpg_frames/";
   string keyframes_subdir=images_subdir+"keyframes/";
   string frames_source_subdir=BostonBombing_subdir+"clips_1_thru_133/";

   vector<string> source_image_filenames=filefunc::image_files_in_subdir(
      frames_source_subdir);

// On 10/29/13, we discovered the painful way that video_filenames is
// generally NOT sorted alphabetically (or by date).  So we explicitly
// sort the strings within STL vector video_filenames:

   string output_filename=images_subdir+"link_video_frames";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);

   
   int clip_ID_offset=20;
   cout << "Enter starting destination clip's ID:" << endl;
   cin >> clip_ID_offset;
   
   for (int i=0; i<source_image_filenames.size(); i++)
   {
      string curr_frame_filename=source_image_filenames[i];
      string video_subdir=filefunc::getdirname(curr_frame_filename);
      string prefix=filefunc::getprefix(curr_frame_filename);
      string suffix=stringfunc::suffix(curr_frame_filename);

      string separator_chars="_-.";
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         prefix,separator_chars);

/*
      cout << "curr_frame_filename = " << curr_frame_filename << endl;
      for (int s=0; s<substrings.size(); s++)
      {
         cout << "s = " << s << " substrings[s] = " << substrings[s] << endl;
      }
      outputfunc::enter_continue_char();
*/

      int clip_ID=stringfunc::string_to_number(substrings[1])+clip_ID_offset;
      int frame_ID=stringfunc::string_to_number(substrings[3]);
      
      string destination_frame_filename="clip_"+stringfunc::integer_to_string(
         clip_ID,4)+"_frame-"+stringfunc::integer_to_string(frame_ID,5)+
         "."+suffix;
      string unix_cmd="ln -s \""+curr_frame_filename
         +"\" "+destination_frame_filename;
      outstream << unix_cmd << endl;

   }
   filefunc::closefile(output_filename,outstream);
   filefunc::make_executable(output_filename);
   string banner="Exported "+output_filename;
   outputfunc::write_big_banner(banner);

}
