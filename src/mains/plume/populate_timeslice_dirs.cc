// ==========================================================================
// Program POPULATE_TIMESLICE_DIRS generates a set of folders with
// names of the form time_slices/tNNNNNN.  It then fills these folders
// with soft-links which convert a set of t>0 plume images organized
// according to camera ID to a new set organized according to time
// slices.  It also generates a set of soft-links for t>0 mask files
// so that they too are organized according to time slice ID.  We
// wrote this utility in order to convert the format of Will
// Lawrence's set of Nov 2012 Day2H video frames into that needed by
// LOAD_PHOTO_METADATA.
// ==========================================================================
// Last updated on 1/8/13; 1/10/13
// ==========================================================================

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
using std::ios;
using std::ofstream;
using std::string;
using std::vector;


// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   string tgreater0_subdir=
      "/data_third_disk/plume/Nov_2012/video/t_greater_0/";
   string image_timeslices_subdir=
      "/data/ImageEngine/plume/Nov2012/Day2/H/images/time_slices/";
   string mask_timeslices_subdir=
      "/data/ImageEngine/plume/Nov2012/Day2/H/bksub/time_slices/";
   filefunc::dircreate(image_timeslices_subdir);
   filefunc::dircreate(mask_timeslices_subdir);

   int start_camera_ID=1;
   int stop_camera_ID=10;
   for (int camera_ID=start_camera_ID; camera_ID <= stop_camera_ID;
        camera_ID++)
   {
      string camera_subdir=tgreater0_subdir+"Vid_"+
         stringfunc::integer_to_string(camera_ID,2)+"/";
      cout << "camera_subdir = " << camera_subdir << endl;
      string mask_subdir=camera_subdir+"bksub/";
      cout << "mask_subdir = " << mask_subdir << endl;

      vector<string> camera_image_files=filefunc::image_files_in_subdir(
         camera_subdir);
      cout << "camera_image_files.size() = " << camera_image_files.size()
           << endl;

      vector<string> allowed_suffixes;
      allowed_suffixes.push_back("bin");
      allowed_suffixes.push_back("bz2");
      vector<string> camera_mask_files=
         filefunc::files_in_subdir_matching_specified_suffixes(
            allowed_suffixes,mask_subdir);
      cout << "camera_mask_files.size() = " << camera_mask_files.size()
           << endl;

      for (int i=0; i<camera_image_files.size(); i++)
      {
         string curr_image_filename=filefunc::getbasename(
            camera_image_files[i]);
         string curr_mask_filename=filefunc::getbasename(
            camera_mask_files[i]);
         cout << curr_image_filename << " "
              << curr_mask_filename << endl;
         
         vector<string> substrings=
            stringfunc::decompose_string_into_substrings(
               curr_image_filename,"_");
         string timeslice_ID_str=substrings[3];
         int timeslice_ID=stringfunc::string_to_number(timeslice_ID_str);
//         timeslice_ID_str=stringfunc::integer_to_string(timeslice_ID,3);
//         cout << timeslice_ID << " " << timeslice_ID_str << endl;

         string image_slice_subdir=
            image_timeslices_subdir+"t"+timeslice_ID_str+"/";
         filefunc::dircreate(image_slice_subdir);
         string unix_cmd="ln -s "+camera_image_files[i]+" "+
            image_slice_subdir;
         sysfunc::unix_command(unix_cmd);

         string mask_slice_subdir=
            mask_timeslices_subdir+"t"+timeslice_ID_str+"/";
         filefunc::dircreate(mask_slice_subdir);
         unix_cmd="ln -s "+camera_mask_files[i]+" "+mask_slice_subdir;
         sysfunc::unix_command(unix_cmd);

//         outputfunc::enter_continue_char();
      } // loop over index i labeling camera image files
   } // loop over camera_ID index
}
