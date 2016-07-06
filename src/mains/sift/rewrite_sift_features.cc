// ==========================================================================
// Program REWRITE_SIFT_FEATURES parses all individual SIFT key files
// within a specified input subdirectory.  Each SIFT key file is reexported
// in HDF5 binary files.
// ==========================================================================
// Last updated on 3/28/12
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
   string sift_keys_subdir=bundler_IO_subdir+"images/keys/";
//   cout << "sift_keys_subdir = " << sift_keys_subdir << endl;

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   int n_photos_to_reconstruct=-1;
   photogroup_ptr->generate_bundler_photographs(
      bundler_IO_subdir,image_list_filename,image_sizes_filename,
      n_photos_to_reconstruct);

   bool FLANN_flag=true;
   sift_detector* sift_detector_ptr=new sift_detector(NULL,FLANN_flag);

   vector<string> sift_keys_filenames;
   if (!filefunc::direxist(sift_keys_subdir))
   {
      cout << "Prior to generating SIFT keyfiles:" << endl;
      outputfunc::enter_continue_char();
      sift_keys_filenames=
         sift_detector_ptr->generate_SIFT_keyfiles(sift_keys_subdir);
   }
   else
   {
      sift_keys_filenames=sift_detector_ptr->
         import_sift_keys_filenames(sift_keys_subdir);
   }

   vector<sift_detector::feature_pair> currimage_feature_info;         
   vector<genvector*>* D_ptrs_ptr=new vector<genvector*>;

   int i_start=0;
   for (int i=i_start; i<sift_keys_filenames.size(); i++)
   {
      photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(i);
      cout << "i = " << i 
           << " of " << sift_keys_filenames.size() 
           << " : Converting " 
           << filefunc::getbasename(sift_keys_filenames[i])
           << " to binary HDF5 format" 
           << endl;

      sift_detector_ptr->parse_Lowe_features(
         photograph_ptr,sift_keys_filenames[i],currimage_feature_info);
      
      for (int f=0; f<currimage_feature_info.size(); f++)
      {
         D_ptrs_ptr->push_back(currimage_feature_info[f].second);
      }

      string output_file_prefix="raw_";
      sift_detector_ptr->get_akm_ptr()->export_SIFT_features_in_HDF5_format(
         sift_keys_filenames[i],output_file_prefix,D_ptrs_ptr);

// Destroy all previously instantiated genvectors:

      for (int j=0; j<currimage_feature_info.size(); j++)
      {
         delete currimage_feature_info[j].first;
         delete currimage_feature_info[j].second;
      }
      D_ptrs_ptr->clear();
   } // loop over index i labeling image
   
   delete sift_detector_ptr;
}

   
