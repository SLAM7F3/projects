// ==========================================================================
// Program EXTRACT_SIFT_FEATURES searches for
// bundler_IO_subdir/images/keys.  If this subdirectory does not
// exist, it runs Lowe's binary on all input images and writes SIFT
// key files to the keys subdirectory.  EXTRACT_SIFT_FEATURES then
// parses all individual SIFT key files and re-exports them as HDF5
// binary files to a new bundler_IO_subdir/images/keys/raw
// subdirectory.  It also exports the cumulative set of SIFT feature
// descriptors as unsigned chars within an output binary file.
// ==========================================================================
// Last updated on 2/28/13; 5/16/13; 12/23/13
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "cluster/akm.h"
#include "datastructures/descriptor.h"
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
   cout << "sift_keys_subdir = " << sift_keys_subdir << endl;
   cout << "filefunc::direxist(sift_keys_subdir) = "
        << filefunc::direxist(sift_keys_subdir) << endl;

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   int n_photos_to_reconstruct=-1;
   photogroup_ptr->generate_bundler_photographs(
      bundler_IO_subdir,image_list_filename,image_sizes_filename,
      n_photos_to_reconstruct);

   bool FLANN_flag=true;
   sift_detector* sift_detector_ptr=new sift_detector(
      photogroup_ptr,FLANN_flag);

   vector<string> sift_keys_filenames;
   if (!filefunc::direxist(sift_keys_subdir))
   {
      cout << "Prior to generating SIFT keyfiles:" << endl;
//      outputfunc::enter_continue_char();
      sift_keys_filenames=
         sift_detector_ptr->generate_SIFT_keyfiles(sift_keys_subdir);
   }
   else
   {
      sift_keys_filenames=sift_detector_ptr->
         import_sift_keys_filenames(sift_keys_subdir);
   }

   string raw_hdf5_subdir=sift_keys_subdir+"raw/";
   cout << "raw_hdf5_subdir = " << raw_hdf5_subdir << endl;
   filefunc::dircreate(raw_hdf5_subdir);


// Open binary file to hold all SIFT feature descriptors as strings of
// 128 unsigned chars (i.e. bytes) if it doesn't already exist:

   bool AllSiftDescriptorsExistsFlag=false;	// default
//   bool AllSiftDescriptorsExistsFlag=true;	// MIT2317

   string output_filename=sift_keys_subdir+"all_raw_sift_descriptors.binary";
   ofstream outstream;
   if (!AllSiftDescriptorsExistsFlag)
   {
      filefunc::openfile(output_filename,outstream);
   }

   vector<sift_detector::feature_pair> currimage_feature_info;         
   vector<descriptor*>* D_ptrs_ptr=new vector<descriptor*>;

   int i_start=0;
   int i_stop=3;
//   int i_stop=sift_keys_filenames.size();
   int n_total_features=0;
   for (int i=i_start; i<i_stop; i++)
   {
      photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(i);
      cout << "i = " << i 
           << " of " << sift_keys_filenames.size() 
           << " : Converting " 
           << filefunc::getbasename(sift_keys_filenames[i])
           << " to binary HDF5 format" 
           << endl;

      string curr_key_filename=sift_keys_filenames[i];

/*
      string suffix=stringfunc::suffix(curr_key_filename);
      if (suffix=="gz")
      {
         string unix_cmd="gunzip "+curr_key_filename;
         sysfunc::unix_command(unix_cmd);
         curr_key_filename=curr_key_filename.substr(
            0,curr_key_filename.size()-3);
      }
*/

// On 4/30/12, we rediscovered the painful way that Lowe's binary can
// fail to produce an output keys file.  If so, we should NOT attempt
// to generate any binary HDF5 keys file!

      bool Lowe_SIFT_flag=true;
      bool nonempty_keysfile_flag=sift_detector_ptr->parse_Lowe_features(
         Lowe_SIFT_flag,photograph_ptr->get_xdim(),
         photograph_ptr->get_ydim(),curr_key_filename,currimage_feature_info);
      if (!nonempty_keysfile_flag) continue;


      bool export_features_flag=true;
//       bool export_features_flag=false;
      if (export_features_flag)
      {
         string features_subdir="./features/";
         filefunc::dircreate(features_subdir);
         sift_detector_ptr->export_feature_tracks(
            i,features_subdir,&currimage_feature_info);
      }
      
      for (int f=0; f<currimage_feature_info.size(); f++)
      {
         descriptor* curr_D_ptr=currimage_feature_info[f].second;

         if (!AllSiftDescriptorsExistsFlag)
         {
            for (int d=0; d<curr_D_ptr->get_mdim(); d++)
            {
               int curr_descriptor=curr_D_ptr->get(d);
               if (curr_descriptor > 255)
               {
                  cout << "f = " << f << " d = " << d << " descrip = "
                       << curr_descriptor << endl;
               }
               unsigned char c=stringfunc::ascii_integer_to_unsigned_char(
                  curr_descriptor);
               outstream << c;
            }
         } // !AllSiftDescriptorsExistsFlag conditional

         n_total_features++;

         D_ptrs_ptr->push_back(curr_D_ptr);
      } // loop over index f labeling SIFT features

      string output_file_prefix="raw_";
      string HDF5_filename=
         sift_detector_ptr->get_akm_ptr()->export_SIFT_features_in_HDF5_format(
            curr_key_filename,raw_hdf5_subdir,output_file_prefix,D_ptrs_ptr);

// As of 4/19/12, we use LZOP rather than GZIP to compress raw HDF5
// binary files.  LZOP does not compress as well as GZIP.  But it runs
// appreciably faster:

      string unix_cmd="lzop -U "+HDF5_filename;
      sysfunc::unix_command(unix_cmd);

      cout << " Wrote " << D_ptrs_ptr->size() << " keys to compressed " 
           << HDF5_filename << endl;

/*
      if (suffix=="gz")
      {
         string unix_cmd="gzip "+curr_key_filename;
         sysfunc::unix_command(unix_cmd);
      }
*/

// Destroy all previously instantiated descriptors:

      for (int j=0; j<currimage_feature_info.size(); j++)
      {
         delete currimage_feature_info[j].first;
         delete currimage_feature_info[j].second;
      }
      D_ptrs_ptr->clear();
   } // loop over index i labeling image

   if (!AllSiftDescriptorsExistsFlag)
   {
      filefunc::closefile(output_filename,outstream);
   }
   
   int n_images=photogroup_ptr->get_n_photos();
   cout << "n_total_features = " << n_total_features << endl;
   cout << "n_images = " << n_images << endl;
   cout << "features/image = " << double(n_total_features)/double(n_images)
        << endl;
   delete sift_detector_ptr;
}

   
