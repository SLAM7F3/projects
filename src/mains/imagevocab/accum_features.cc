// ==========================================================================
// Program ACCUM_FEATURES imports HDF5 binary files containing raw
// SIFT descriptors for a set of images.  It converts each SIFT
// descriptor unsigned integer (0-255) to an unsigned char (byte).
// Each byte is exported to binary file "all_raw_sift_descriptors.binary".
// The final size of the cumulative feature binary file should
// precisely equal 128*n_features bytes.
// ==========================================================================
// Last updated on 4/11/12; 4/19/12; 4/20/12
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "cluster/akm.h"
#include "general/filefuncs.h"
#include "passes/PassesGroup.h"
#include "video/sift_detector.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);

   string image_list_filename=passes_group.get_image_list_filename();
//   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   string image_sizes_filename=passes_group.get_image_sizes_filename();
   string sift_keys_subdir=bundler_IO_subdir+"images/keys/";
//   cout << "sift_keys_subdir = " << sift_keys_subdir << endl;
   string raw_hdf5_subdir=sift_keys_subdir+"raw/";
   cout << "raw_hdf5_subdir = " << raw_hdf5_subdir << endl;

   bool FLANN_flag=true;
   sift_detector* sift_detector_ptr=new sift_detector(NULL,FLANN_flag);
   vector<string> compressed_sift_hdf5_filenames=sift_detector_ptr->
      import_compressed_sift_hdf5_filenames(raw_hdf5_subdir);
   int n_images=compressed_sift_hdf5_filenames.size();
   delete sift_detector_ptr;

// Open binary file to hold all raw SIFT feature descriptors as
// strings of 128 unsigned chars (i.e. bytes):

   string output_filename=sift_keys_subdir+"all_raw_sift_descriptors.binary";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);

   int i_start=0;
   int i_stop=n_images;
   int n_total_features=0;
   flann::Matrix<float> SIFT_descriptors;
   for (int i=i_start; i<i_stop; i++)
   {
      string compressed_sift_hdf5_filename=compressed_sift_hdf5_filenames[i];
      string suffix=stringfunc::suffix(compressed_sift_hdf5_filename);
      string unix_cmd;
      if (suffix=="gz")
      {
         unix_cmd="gunzip "+compressed_sift_hdf5_filename;
      }
      else if (suffix=="lzo")
      {
         unix_cmd="lzop --uncompress "+compressed_sift_hdf5_filename;
      }
//      cout << "unix_cmd = " << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);      

      string sift_hdf5_filename=
         stringfunc::prefix(compressed_sift_hdf5_filename);

      cout << "===============================================" << endl;
      cout << i << " of " << n_images << ": Processing " 
               << sift_hdf5_filename << endl;

      delete [] SIFT_descriptors.ptr();
      flann::load_from_file(
         SIFT_descriptors,sift_hdf5_filename.c_str(),"sift_features");

      if (suffix=="lzo")
      {

// Delete uncompressed SIFT HDF5 file:

         unix_cmd="/bin/rm "+sift_hdf5_filename;
      }
      else if (suffix=="gz")
      {
         unix_cmd="gzip "+sift_hdf5_filename;
      }
      sysfunc::unix_command(unix_cmd);

      int n_features=SIFT_descriptors.rows;
      for (int f=0; f<n_features; f++)
      {
         for (int d=0; d<128; d++)
         {
            int curr_descriptor = SIFT_descriptors[f][d];
            unsigned char c=stringfunc::ascii_integer_to_unsigned_char(
               curr_descriptor);
            outstream << c;
         }
         n_total_features++;

      } // loop over index f labeling SIFT features

   } // loop over index i labeling image

   filefunc::closefile(output_filename,outstream);
   
   long long n_bytes=n_total_features*128;
   double n_Gbytes=n_bytes/1.0E9;
   cout << "n_total_features = " << n_total_features << endl;
   cout << "n_total_bytes = " << n_bytes << endl;
   cout << "n_total_Gbytes = " << n_Gbytes << endl;
   cout << "n_images = " << n_images << endl;
   cout << "features/image = " << double(n_total_features)/double(n_images)
        << endl;
   
   string features_metadata_filename=
      sift_keys_subdir+"all_raw_sift_descriptors.metadata";
   ofstream features_metadata_stream;
   filefunc::openfile(features_metadata_filename,features_metadata_stream);
   features_metadata_stream << "n_total_features = " << n_total_features 
                            << endl;
   features_metadata_stream << "n_total_bytes = " << n_bytes
                            << endl;
   features_metadata_stream << "n_total_Gbytes = " << n_Gbytes
                            << endl;
   features_metadata_stream << "n_images = " << n_images << endl;
   features_metadata_stream << "features/n_images = "
                            << double(n_total_features)/double(n_images) 
                            << endl;
   filefunc::closefile(features_metadata_filename,features_metadata_stream);
}

   
