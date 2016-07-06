// ==========================================================================
// Program DISPLAY_AUDIO_TRACK is a playground for adding an audio
// track's transcript to a video timeline "sine-wave" display within
// Michael Yee's graph viewer.

//			  ./display_audio_track

// ==========================================================================
// Last updated on 10/28/13; 10/29/13
// ==========================================================================

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "templates/mytemplates.h"
#include "general/outputfuncs.h"
#include "math/prob_distribution.h"
#include "video/RGB_analyzer.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
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
   timefunc::initialize_timeofday_clock();      

//   string JAV_subdir="/data/video/JAV/NewsWraps/early_Sep_2013/";
   string JAV_subdir="/data/video/JAV/NewsWraps/w_transcripts/";
//   string JAV_subdir="/data/video/JAV/UIUC_Broadcast_News/";
   string root_subdir=JAV_subdir;
   string images_subdir=root_subdir+"jpg_frames/";

   string transcript_filename="transcript_20.txt";
   vector<string> transcript_strings=
      filefunc::ReadInStrings(transcript_filename);
   cout << "transcript_strings.size() = " << transcript_strings.size()
        << endl;

   for (unsigned int w=0; w<transcript_strings.size(); w++)
   {
      cout << transcript_strings[w] << " " << flush;
      if (w%10==0) cout << endl;
   }
   cout << endl;

   cout << "At end of program DISPLAY_AUDIO_TRACK" << endl;
   outputfunc::print_elapsed_time();
}


