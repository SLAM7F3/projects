// ==========================================================================
// Program RANDOMHASH imports a subset of compressed binary SIFT
// keyfiles from a specified subdirectory.  It samples some fraction
// of each binary keyfile's SIFT descriptors.  The O(10**8) input SIFT
// descriptors are accumulated within an STL vector of dlib column
// vectors.  We then call dlib's create_random_projection_hash()
// method.  The resulting projection hash object is serialized and
// written out to a binary file.  
// ==========================================================================
// Last updated on 8/19/13; 8/23/13; 8/24/13
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include <dlib/svm_threaded.h>
#include <dlib/gui_widgets.h>
#include <dlib/array.h>
#include <dlib/array2d.h>
#include <dlib/image_keypoint.h>
#include <dlib/image_processing.h>

#include "general/filefuncs.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::string;
using std::vector;

int main (int argc, char * argv[])
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

   int d_dims=128;
   int n_images=compressed_descriptor_filenames.size();
//   int i_skip=4;
//   int i_skip=5;
   int i_skip=101;	// prime

//   long long n_requested_total_features=10000000;  // 10 million
   long long n_requested_total_features=100000000;  // 100 million
//   int n_requested_features_per_sampled_image=3000;

   dlib::matrix<unsigned char,128,1> y;
   vector<dlib::matrix<unsigned char,128,1> > descriptor_columns;
//   dlib::matrix<short,128,1> y;
//   vector<dlib::matrix<short,128,1> > descriptor_columns;
   descriptor_columns.reserve(n_requested_total_features); 

   cout << "n_images = " << n_images << endl;
   cout << "i_skip = " << i_skip << endl;
   cout << "n_requested_total_features = " << n_requested_total_features
        << endl;
//   outputfunc::enter_continue_char();
   sleep(5);
   timefunc::initialize_timeofday_clock();

// Import descriptors from compressed binary key files:

   int image_ID=0;
   long long n_total_features=0;
   int keyfile_counter=0;
   while (n_total_features < n_requested_total_features)
   {
      if (keyfile_counter%200==0) outputfunc::print_elapsed_time();
      image_ID=(image_ID+i_skip)%n_images;

// First uncompress binary descriptor .lzo file:

      string unix_cmd="lzop --uncompress "+compressed_descriptor_filenames[
         image_ID];
      sysfunc::unix_command(unix_cmd);

      string descriptor_filename=compressed_descriptor_filenames[image_ID];
      descriptor_filename=descriptor_filename.substr(
         0,descriptor_filename.size()-4);
//      cout << "descriptor_filename = " << descriptor_filename << endl;

      int fskip=1;
      int n_features_in_curr_keyfile=
         filefunc::size_of_file_in_bytes(descriptor_filename)/d_dims;
//      if (n_features_in_curr_keyfile > n_requested_features_per_sampled_image)
//      {
//         fskip=n_features_in_curr_keyfile/
//            n_requested_features_per_sampled_image;
//      }

      int n_bytes_in_curr_keyfile=n_features_in_curr_keyfile*d_dims;
      unsigned char* data_ptr=filefunc::ReadUnsignedChars(
         descriptor_filename,n_bytes_in_curr_keyfile);

      for (int f=0; f<n_features_in_curr_keyfile; f += fskip)
      {
         int byte_counter=f*d_dims;
         for (int d=0; d<d_dims; d++)
         {
            y(d)=data_ptr[byte_counter++];
//            y(d)=stringfunc::unsigned_char_to_ascii_integer(
//               data_ptr[byte_counter++]);

/*
            if (d==0)
            {
               cout << "f = " << f << endl;
            }
            else if (d < 10)
            {
               cout << stringfunc::unsigned_char_to_ascii_integer(y(d)) 
                    << " " << flush;
            }
            else if (d==11)
            {
               cout << endl;
            }
*/
         } // loop over index d

         descriptor_columns.push_back(y);
         n_total_features++;
         if (n_total_features >= n_requested_total_features) break;
      } // loop over index f labeling features
      delete [] data_ptr;

      keyfile_counter++;
      double avg_n_features_per_image=n_total_features/double(keyfile_counter);

      cout << keyfile_counter
           << " image_ID = " << image_ID
           << " curr_nfeatures=" << n_features_in_curr_keyfile 
           << " n_total_features=" << 1E-6*n_total_features << "M  "
           << " <n_features_per_image>=" << avg_n_features_per_image 
           << endl;
      
// Delete uncompressed binary descriptor file:

      unix_cmd="/bin/rm "+descriptor_filename;
      sysfunc::unix_command(unix_cmd);

   }   // n_total_features < n_requested_total_features while loop
   cout << endl;

   cout << "n_total_features = " << n_total_features << endl;
   outputfunc::print_elapsed_time();

// Form random hash from accumuted "SIFT" descriptors:

//   int bits=14; 	// n_Voronoi_cells = 16384
//   int bits=15;	// n_Voronoi_cells = 32768
//   int bits=16; 	// n_Voronoi_cells = 65536
   int bits=17; 	// n_Voronoi_cells = 131072

   string banner="Generating random projection hash with "+
      stringfunc::number_to_string(bits)+" bits";
   outputfunc::write_banner(banner);
   
   dlib::projection_hash phash = create_random_projection_hash(
      descriptor_columns, bits);
   cout << "phash.num_hash_bins() = " << phash.num_hash_bins() << endl;

// Another thing that is worth knowing is that just about everything
// in dlib is serializable. So for example, you can save the
// learned_pfunct object to disk and recall it later like so:

   string hash_filename=sift_keys_subdir+"phash_"+
      stringfunc::number_to_string(n_total_features)+"_features_"+
      stringfunc::number_to_string(n_images)+"_imgs_"+
      stringfunc::number_to_string(bits)+"_bits.dat";
   ofstream fout(hash_filename.c_str(),ios::binary);
   dlib::serialize(phash,fout);
   fout.close();

   banner="Exported random hash function to "+hash_filename;
   outputfunc::write_banner(banner);

// Now let's open that file back up and load the function object it
// contains:

//   ifstream fin(hash_filename.c_str(),ios::binary);
//   dlib::projection_hash phash2;
//   dlib::deserialize(phash2, fin);

   cout << "At end of program RANDOMHASH" << endl;
   outputfunc::print_elapsed_time();

} 

