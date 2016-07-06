// ==========================================================================
// Program COMPUTE_SIFT_KEYFILES imports a set of images from a
// specified set of subdirectories of /data/ImageEngine/.  It calls
// either Lowe's binary or libsiftfast in order to generate a text
// SIFT keyfile for every input image.  A random integer is added to
// each output keyfile name in order to minimize the probability any
// two image files yield the same key file name.

// 			    ./compute_sift_keyfiles

// ==========================================================================
// Last updated on 8/16/13; 8/19/13
// ==========================================================================

#include  <algorithm>
#include  <iostream>
#include  <map>
#include  <string>
#include  <vector>

#include <dlib/svm_threaded.h>
#include <dlib/gui_widgets.h>
#include <dlib/array.h>
#include <dlib/array2d.h>
#include <dlib/image_keypoint.h>
#include <dlib/image_processing.h>

#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "video/image_matcher.h"
#include "datastructures/map_unionfind.h"
#include "video/photogroup.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   string ImageEngine_subdir="/data/ImageEngine/";

// Set random seed based upon current computer clock time:

   Clock clock;
   clock.set_time_based_on_local_computer_clock();
   double epoch=clock.secs_elapsed_since_reference_date();
   string seed_str=stringfunc::number_to_string(epoch);
   dlib::rand* rnd_ptr=new dlib::rand();
//   rnd_ptr->set_seed(seed_str);

   vector<string> image_subdirs;
//    image_subdirs.push_back(ImageEngine_subdir+"baseball/");
//   image_subdirs.push_back(
//      ImageEngine_subdir+"BostonBombing/clips_1_thru_133/");
//   image_subdirs.push_back(ImageEngine_subdir+"garden/");
//   image_subdirs.push_back(ImageEngine_subdir+"GrandCanyon/images/");
//      image_subdirs.push_back(ImageEngine_subdir+"Human_Faces/");
//   image_subdirs.push_back(ImageEngine_subdir+"kermit/");
//   image_subdirs.push_back(ImageEngine_subdir+"MIT32K_and_aerial/");
//   image_subdirs.push_back(ImageEngine_subdir+"south_India/");
   image_subdirs.push_back(ImageEngine_subdir+"tidmarsh/");

   vector<string> all_image_filenames;
   for (unsigned int s=0; s<image_subdirs.size(); s++)
   {
      bool search_all_children_dirs_flag=false;
      vector<string> curr_image_filenames=filefunc::image_files_in_subdir(
         image_subdirs[s],search_all_children_dirs_flag);
      for (unsigned int j=0; j<curr_image_filenames.size(); j++)
      {
         all_image_filenames.push_back(curr_image_filenames[j]);
      }
      cout << "s = " << s
           << " subdir = " << image_subdirs[s]
           << " all_image_filenames.size() = " << all_image_filenames.size()
           << endl;
   }

   photogroup* photogroup_ptr=new photogroup;

/*
   for (int i=0; i<all_image_filenames.size(); i++)
   {
      cout << i << " " << filefunc::getbasename(all_image_filenames[i])
           << endl;
   }
*/
   bool FLANN_flag=true;
   image_matcher SIFT(photogroup_ptr,FLANN_flag);

   bool delete_pgm_file_flag=true;

   string sift_keys_subdir="/data2/sift_keys/";
   filefunc::dircreate(sift_keys_subdir);
   
   vector<sift_detector::feature_pair> currimage_feature_info;

   int i_start=0;
   for (unsigned int i=i_start; i<all_image_filenames.size(); i++)
   {
      string image_filename=all_image_filenames[i];
//      cout << "image_filename = " << image_filename << endl;

      int random_int=1000000*rnd_ptr->get_random_double();
      string random_str=stringfunc::number_to_string(random_int);

      string basename=filefunc::getbasename(image_filename);
      string prefix=stringfunc::prefix(basename);
      string sift_keys_filename=sift_keys_subdir+prefix+"_"+random_str+".key";
      SIFT.generate_Lowe_keyfile(
         delete_pgm_file_flag,sift_keys_filename,image_filename);
   }
   
   delete photogroup_ptr;
   delete rnd_ptr;
}

