// ==========================================================================
// Program KEYS_TO_HDF5 parses all individual SIFT key files
// within a specified input subdirectory.  Each SIFT key file is reexported
// as a compressed HDF5 binary file.
// ==========================================================================
// Last updated on 4/17/12; 4/19/12; 4/30/12; 5/31/13
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "cluster/akm.h"
#include "datastructures/descriptor.h"
#include "general/filefuncs.h"
#include "passes/PassesGroup.h"
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

/*
// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);

   string image_list_filename=passes_group.get_image_list_filename();
//   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   string image_sizes_filename=passes_group.get_image_sizes_filename();
   string sift_keys_subdir=bundler_IO_subdir+"images/keys/";
*/

   string sift_keys_subdir="/data2/sift_keys/";
//   cout << "sift_keys_subdir = " << sift_keys_subdir << endl;
   string raw_hdf5_subdir=sift_keys_subdir+"raw/";
   cout << "raw_hdf5_subdir = " << raw_hdf5_subdir << endl;
   filefunc::dircreate(raw_hdf5_subdir);

   if (!filefunc::direxist(sift_keys_subdir))
   {
      cout << "Error:  Did not find sift_keys_subdir = "
           << sift_keys_subdir << endl;
      exit(-1);
   }

   bool FLANN_flag=true;
   sift_detector* sift_detector_ptr=new sift_detector(NULL,FLANN_flag);

   cout << "Importing SIFT keys:" << endl;
   vector<string> sift_keys_filenames=sift_detector_ptr->
      import_sift_keys_filenames(sift_keys_subdir);

   vector<descriptor*>* D_ptrs_ptr=new vector<descriptor*>;

   int i_start=0;
   cout << "Enter starting image number:" << endl;
   cout << "(Default value = 0)" << endl;
   cin >> i_start;
   
   for (unsigned int i=i_start; i<sift_keys_filenames.size(); i++)
   {
      cout << "i = " << i 
           << " of " << sift_keys_filenames.size() 
           << " : Converting " 
           << filefunc::getbasename(sift_keys_filenames[i])
           << " to binary HDF5 format" 
           << endl;

      bool keys_parsed_flag=sift_detector_ptr->parse_Lowe_descriptors(
         sift_keys_filenames[i],D_ptrs_ptr);
      if (!keys_parsed_flag) continue;

      if (D_ptrs_ptr->size()==0) 
      {
         cout << "*** Input SIFT key file had zero descriptors ***" << endl;
         continue;
      }

      string output_file_prefix="raw_";
      string HDF5_filename=
         sift_detector_ptr->get_akm_ptr()->export_SIFT_features_in_HDF5_format(
            sift_keys_filenames[i],raw_hdf5_subdir,output_file_prefix,
            D_ptrs_ptr);

// As of 4/19/12, we use LZOP rather than GZIP to compress raw HDF5
// binary files.  LZOP does not compress as well as GZIP.  But it runs
// appreciably faster:

//      string unix_cmd="gzip "+HDF5_filename;
      string unix_cmd="lzop -U "+HDF5_filename;
      sysfunc::unix_command(unix_cmd);

      cout << " Wrote " << D_ptrs_ptr->size() << " keys to compressed " 
           << HDF5_filename << endl;

      for (unsigned int j=0; j<D_ptrs_ptr->size(); j++)
      {
         delete D_ptrs_ptr->at(j);
      }
      D_ptrs_ptr->clear();

   } // loop over index i labeling image
   
   delete sift_detector_ptr;
}

   
