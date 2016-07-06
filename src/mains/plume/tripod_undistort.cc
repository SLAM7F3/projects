// ==========================================================================
// Program TRIPOD_UNDISTORT uses radial distortion parameters
// reconstructed for fixed tripod cameras before a smoke event to
// remove radial distortion in tripod photos collected during a smoke event.

//	         	       tripod_undistort


// ==========================================================================
// Last updated on 11/25/11
// ==========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "bundler/bundlerfuncs.h"
#include "general/filefuncs.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "geometry/plane.h"
#include "math/rotation.h"          
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "osg/osg3D/tdpfuncs.h"
#include "math/threevector.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
   cout.precision(15);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);

   string bundler_IO_subdir="./bundler/plume/Nov_2011/BC_rovers_ABC_tripods/";
   string image_list_filename=bundler_IO_subdir+"image_list.dat";
   string image_sizes_filename=bundler_IO_subdir+"image_sizes.dat";
   string bundle_filename=bundler_IO_subdir+"bundle_orig.out";

// ------------------------------------------------------------------------
// Instantiate reconstructed photos:

   int n_tripods=10;

   cout << "Instantiating photogroup:" << endl;
   photogroup* photogroup_ptr=new photogroup();
   int n_photos_to_reconstruct=n_tripods;
   photogroup_ptr->reconstruct_bundler_cameras(
      bundler_IO_subdir,image_list_filename,image_sizes_filename,
      bundle_filename,n_photos_to_reconstruct);

   vector<camera*> camera_ptrs;
   for (int n=0; n<n_tripods; n++)
   {
      photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(n);
      camera* camera_ptr=photo_ptr->get_camera_ptr();
//      cout << "n = " << n
//           << " camera = " << *camera_ptr << endl;
      camera_ptrs.push_back(camera_ptr);
   }

   int tripod_start_ID=18;
   int tripod_stop_ID=18;
//   int tripod_stop_ID=26;
   for (int tripod_ID=tripod_start_ID; tripod_ID <= tripod_stop_ID; 
        tripod_ID++)
   {
      string smoke_subdir="./smoke_shots/Nov_2011/Day5/5B/";
      cout << "smoke subdir = " << smoke_subdir << endl;

      vector<string> suffixes;
      suffixes.push_back("jpg");
//      suffixes.push_back("JPG");
      vector<string> distorted_jpeg_filenames=
         filefunc::files_in_subdir_matching_specified_suffixes(
            suffixes,smoke_subdir);

      for (int j=0; j<distorted_jpeg_filenames.size(); j++)
      {
         string smoke_photo_filename=distorted_jpeg_filenames[j];
         string undistorted_images_subdir="./undistorted_frames/";
         string undistorted_filename=bundlerfunc::radial_undistort(
            camera_ptrs[tripod_ID-17],smoke_photo_filename,
            undistorted_images_subdir);
         string new_undistorted_filename=undistorted_images_subdir+
            "smoke_"+stringfunc::number_to_string(tripod_ID)+".jpg";
         cout << "orig undistorted filename = " << undistorted_filename
              << " new filename = " << new_undistorted_filename << endl;
         string unix_cmd="mv "+undistorted_filename+" "+
            new_undistorted_filename;
         sysfunc::unix_command(unix_cmd);
      } // loop over index j labeling distorted jpeg files

   } // loop over trip_ID
   
}



