// =======================================================================
// Program EXTRACT_SURF_FEATURES imports a set of image files.
// It extracts SURF features via calls to DLIB.  Serialized binary
// files containing SURF features for each image are exported to a
// SURF_features subdirectory.

//	       		./extract_surf_features                     

// =======================================================================
// Last updated on 11/8/13; 11/9/13
// =======================================================================

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "video/image_matcher.h"
#include "datastructures/map_unionfind.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

using std::cout;
using std::endl;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

int main( int argc, char** argv ) 
{
   std::set_new_handler(sysfunc::out_of_memory);

   string ImageEngine_subdir="/data/ImageEngine/";

//   string root_subdir=ImageEngine_subdir+"NewsWrap/";
//   string root_subdir=ImageEngine_subdir+
//      "BostonBombing/Nightline_YouTube2/transcripted/";
//   string root_subdir=
//      "/data/ImageEngine/BostonBombing/clips_1_thru_133/clip34/";
//   string root_subdir=ImageEngine_subdir+"BostonBombing/clip3/";
//   string root_subdir=
//      "/home/cho/programs/c++/svn/projects/src/mains/korea/NK/ground_videos/NorthKorea/";
//   string images_subdir=root_subdir;
//   string images_subdir="/data/ImageEngine/BostonBombing/clips_1_thru_133/";

   string images_subdir=ImageEngine_subdir+"/tidmarsh/";
   string root_subdir=images_subdir;

/*
   string JAV_subdir="/data/video/JAV/NewsWraps/early_Sep_2013/";
//   string JAV_subdir="/data/video/JAV/NewsWraps/w_transcripts/";
//   string JAV_subdir="/data/video/JAV/UIUC_Broadcast_News/";
   string root_subdir=JAV_subdir;
   string images_subdir=root_subdir+"jpg_frames/";
//   string images_subdir=root_subdir+"mini_jpg_frames/";
*/

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(images_subdir);

// We explicitly confirmed on 1/30/13 that the FLANN library yields
// noticeably better feature matching results than the older ANN
// library:

   bool FLANN_flag=true;
   image_matcher SIFT(photogroup_ptr,FLANN_flag);
   
   string features_subdir=root_subdir+"SURF_features/";
   filefunc::dircreate(features_subdir);

   timefunc::initialize_timeofday_clock();

// --------------------------------------------------------------------------
// Extract SURF features from each input image:

   string SURF_keys_subdir=root_subdir+"SURF_keys/";
   cout << "SURF_keys_subdir = " << SURF_keys_subdir << endl;
   SIFT.extract_SURF_features(SURF_keys_subdir);

   cout << "At end of program EXTRACT_SURF_FEATURES" << endl;
   outputfunc::print_elapsed_time();
}
