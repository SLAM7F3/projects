// ==========================================================================
// Program DESCRIPTORS_ENTROPY loops over a subset of binarized SIFT
// keyfiles.  It computes their descriptors' entropies as defined by
// Wei Dong in his Princeton Ph.D. thesis "High-dimensional similarity
// search for large datasets".  This program generates and plots a
// descriptor entropy distribution.  We observe that this distribution
// is qualitiatively similar to the one presented by Dong in fig 6.3
// on page 144 of his thesis.
// ==========================================================================
// Last updated on 8/25/13
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <flann/flann.hpp>
#include <flann/io/hdf5.h>

#include "cluster/akm.h"
#include "datastructures/descriptor.h"
#include "general/filefuncs.h"
#include "passes/PassesGroup.h"
#include "math/prob_distribution.h"
#include "video/sift_detector.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

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
   cout.precision(16);

   timefunc::initialize_timeofday_clock();      

   string sift_keys_subdir="/data/sift_keyfiles/";
   string binary_descriptors_subdir=sift_keys_subdir+"binary_descriptors/";

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("lzo");
   vector<string> compressed_descriptor_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,binary_descriptors_subdir);

   int n_images=compressed_descriptor_filenames.size();
   int i_start=0;
   int i_stop=n_images;
   int d_dims=128;

   cout << "n_images = " << n_images << endl;

//   int i_skip=1;
//   int i_skip=5;
//   int i_skip=10;
//   int i_skip=100;
   int i_skip=300;
//   int i_skip=1000;

   const int D=128;

// Import SIFT descriptors for individual "1 through N" images from
// either gzipped or lzop compressed key files:

   descriptor curr_descriptor(D);

   int n_total_features=0;
   vector<double> descriptor_entropies;
   for (int i=i_start; i<i_stop; i += i_skip)
   {
      outputfunc::update_progress_fraction(i,100,i_stop);
      if (i%200==0) outputfunc::print_elapsed_time();

// First uncompress binary descriptor .lzo file:

      string unix_cmd="lzop --uncompress "+compressed_descriptor_filenames[i];
      sysfunc::unix_command(unix_cmd);

      string descriptor_filename=compressed_descriptor_filenames[i];
      descriptor_filename=descriptor_filename.substr(
         0,descriptor_filename.size()-4);
//      cout << "descriptor_filename = " << descriptor_filename << endl;

      int fskip=1;
      int n_features=
         filefunc::size_of_file_in_bytes(descriptor_filename)/d_dims;

      int n_bytes=n_features*d_dims;
      unsigned char* data_ptr=filefunc::ReadUnsignedChars(
         descriptor_filename,n_bytes);

      int byte_counter=0;
      for (int f=0; f<n_features; f += fskip)
      {
         for (int d=0; d<d_dims; d++)
         {
            curr_descriptor.put(
               d,stringfunc::unsigned_char_to_ascii_integer(
                  data_ptr[byte_counter++]));
         }
         descriptor_entropies.push_back(curr_descriptor.entropy());
         n_total_features++;

      } // loop over index f labeling features

      cout << i 
           << " n_features=" << n_features 
           << " n_total_features=" << 1E-6*n_total_features << "M  "
           << endl;

      delete [] data_ptr;
      
// Delete uncompressed binary descriptor file:

      unix_cmd="/bin/rm "+descriptor_filename;
      sysfunc::unix_command(unix_cmd);

   } // loop over index i labeling binary descriptor files
   cout << endl;

   prob_distribution prob(descriptor_entropies,100,0);
   prob.writeprobdists(false);

   cout << "At end of program DESCRIPTORS_ENTROPY" << endl;
   outputfunc::print_elapsed_time();
}

   
