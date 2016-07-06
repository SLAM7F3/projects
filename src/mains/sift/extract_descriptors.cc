// ==========================================================================
// Program EXTRACT_DESCRIPTORS runs the Oxford
// extract_features_64bit.ln binary on all input images for some
// user-selected detector type.  It writes their ascii key files to
// the keys subdirectory.
// ==========================================================================
// Last updated on 4/9/12; 4/11/12; 2/10/13
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "cluster/akm.h"
#include "general/filefuncs.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "video/sift_detector.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);

   string image_list_filename=passes_group.get_image_list_filename();
//   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   string image_sizes_filename=passes_group.get_image_sizes_filename();

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   int n_photos_to_reconstruct=-1;
   photogroup_ptr->generate_bundler_photographs(
      bundler_IO_subdir,image_list_filename,image_sizes_filename,
      n_photos_to_reconstruct);

   bool FLANN_flag=true;
   sift_detector* sift_detector_ptr=new sift_detector(
      photogroup_ptr,FLANN_flag);

   string detector_name="hesaff";
//   string detector_name="sedgelap";
   cout << "Enter detector name (e.g. 'hesaff', 'sedgelap', 'harhes'):" 
        << endl;
   cin >> detector_name;

   string detector_descriptor_keys_subdir=bundler_IO_subdir+detector_name+
      "_keys/";
//   cout << "detector_descriptor_keys_subdir = " 
//        << detector_descriptor_keys_subdir << endl;
   
   vector<string> detector_descriptor_keys_filenames=
      sift_detector_ptr->generate_detector_descriptor_keyfiles(
         detector_name,detector_descriptor_keys_subdir);

   delete sift_detector_ptr;
}

   
