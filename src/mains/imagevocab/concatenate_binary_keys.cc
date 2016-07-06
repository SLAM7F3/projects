// ==========================================================================
// Program CONCATENATE_BINARY_KEYS imports the inverse square root of
// the SIFT descriptor covariance matrix calculated by program
// DESCRIPTORS_COVAR.  It next imports binarized SIFT descriptors
// output by program BINARIZE_SIFT_DESCRIPTORS.  Looping over some
// specified number of all the binarized descriptors, this program
// whitens them via the inverse root covariance matrix and exports
// the results to binary files containing floats.  We wrote this
// CONCATENATE_BINARY_KEYS in order to generate binary input for Wei
// Dong's out-of-core kmeans program.

//			./concatenate_binary_keys

/*

n_images = 110074
i_skip = 997
keyfile_counter = 2271
n_considered_descriptors = 39 M
n_total_features = 32 M
rejected descriptors fraction = 0.1752235573166835
<n_features per keyfile> = 14489.02245706737
Elapsed time = 1633.7 secs =   27.23 minutes =   0.454 hours 

n_images = 110074
i_skip = 997
keyfile_counter = 661
n_considered_descriptors = 12 M
n_total_features = 10 M
rejected descriptors fraction = 0.1700097309775926
<n_features per keyfile> = 15261.23449319213
Elapsed time = 493.4 secs =   8.22 minutes =   0.137 hours 



n_images = 110074
i_skip = 997
keyfile_counter = 5794
n_total_features = 100 M
<n_features per keyfile> = 17262.07973765965
Elapsed time = 2022.6 secs =   33.71 minutes =   0.562 hours 

n_images = 110074
i_skip = 997	
keyfile_counter = 1820
n_total_features = 32 M
<n_features per keyfile> = 17595.2543956044
Elapsed time = 752.1 secs =   12.53 minutes =   0.209 hours 

*/

// ==========================================================================
// Last updated on 8/31/13; 9/1/13; 1/24/14
// ==========================================================================

#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>
#include <flann/flann.hpp>

#include "datastructures/descriptor.h"
#include "general/filefuncs.h"
#include "math/genmatrix.h"
#include "math/genvector.h"
#include "math/mathfuncs.h"
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
   string output_sift_keys_subdir="/data_second_disk/sift_keyfiles/";
   string binary_descriptors_subdir=sift_keys_subdir+"binary_descriptors/";
   string BostonBombing_binary_descriptors_subdir=
      binary_descriptors_subdir+"BostonBombing/";

   int d_dims=128;

// Import inverse square root of SIFT features covariance matrix:

   string txt_inverse_sqrt_covar_filename=
      sift_keys_subdir+"inverse_sqrt_covar_matrix.txt";
   genmatrix* inverse_sqrt_covar_ptr=mathfunc::
      import_from_dense_text_format(txt_inverse_sqrt_covar_filename);
//   cout << "inverse sqrt covar = " << *inverse_sqrt_covar_ptr << endl;


   flann::Matrix<float> inverse_sqrt_covar(
      new float[d_dims*d_dims],d_dims,d_dims);

   for (int i=0; i<d_dims; i++)
   {
      for (int j=0; j<d_dims; j++)
      {
         inverse_sqrt_covar[i][j]=inverse_sqrt_covar_ptr->get(i,j);
      }
   }
   delete inverse_sqrt_covar_ptr;

// Import descriptors from compressed binary key files:

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("lzo");
   vector<string> compressed_descriptor_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,BostonBombing_binary_descriptors_subdir);
//         allowed_suffixes,binary_descriptors_subdir);

   int n_images=compressed_descriptor_filenames.size();
//   int i_skip=101;	// prime
   int i_skip=997;	// prime which doesn't divide n_images=44254
			//    Boston Bombing YouTube frames
//   int i_skip=997;	// prime which doesn't divide n_images=110074
//   int i_skip=7919;	// prime

//   long long n_requested_total_features=1000000;   // 1 million
   long long n_requested_total_features=10000000;  // 10 million
//   long long n_requested_total_features=32000000;    // 32 million
//   long long n_requested_total_features=100000000;  // 100 million
//   int n_requested_features_per_sampled_image=3000;

   cout << "n_images = " << n_images << endl;
   cout << "i_skip = " << i_skip << endl;
   cout << "n_requested_total_features = " << n_requested_total_features
        << endl;
//   outputfunc::enter_continue_char();
   sleep(3);
   timefunc::initialize_timeofday_clock();

   string concatenated_keys_filename=
      output_sift_keys_subdir+"concatenated_keys_"+
      stringfunc::number_to_string(n_requested_total_features/1000000)+
      "M_descriptors.bin";
   cout << "concatenated_keys_filename = " << concatenated_keys_filename
        << endl;
   
   ofstream outstream;
   filefunc::open_binaryfile(concatenated_keys_filename,outstream);

// Import descriptors from compressed binary key files:

   int image_ID=0;
   long long n_total_features=0;
   int keyfile_counter=0;
   int n_rejected_descriptors=0;
   int n_considered_descriptors=0;

   float* raw_SIFT_descriptor_ptr=new float[d_dims];
   float* whitened_SIFT_descriptor_ptr=new float[d_dims];
   descriptor* raw_descriptor_ptr=new descriptor(d_dims);

   while (n_total_features < n_requested_total_features &&
          keyfile_counter < n_images)
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

      int n_features_in_curr_keyfile=
         filefunc::size_of_file_in_bytes(descriptor_filename)/d_dims;

      int n_bytes_in_curr_keyfile=n_features_in_curr_keyfile*d_dims;
      unsigned char* data_ptr=filefunc::ReadUnsignedChars(
         descriptor_filename,n_bytes_in_curr_keyfile);

      float* floats_ptr=new float[n_bytes_in_curr_keyfile];
      for (int b=0; b<n_bytes_in_curr_keyfile; b++)
      {
         floats_ptr[b]=stringfunc::unsigned_char_to_ascii_integer(data_ptr[b]);
      }
      delete [] data_ptr;

// Whiten features imported from current keyfile.  Then export the
// whitened features as floats to the output binary file:

      for (int f=0; f<n_features_in_curr_keyfile; f++)
      {
         for (int d=0; d<d_dims; d++)
         {
            int b=f*d_dims+d;
            raw_SIFT_descriptor_ptr[d]=floats_ptr[b];
            raw_descriptor_ptr->put(d,floats_ptr[b]);
         } // loop over index d 

// As of 8/31/13, we experiment with ignoring any descriptor with a low 
// entropy as defined by Wei Dong in his Princeton Ph.D. thesis:
         
         n_considered_descriptors++;
         const double min_descriptor_entropy=4.4;
         if (raw_descriptor_ptr->entropy() < min_descriptor_entropy)
         {
            n_rejected_descriptors++;
            continue;
         }
         
         for (int i=0; i<d_dims; i++)
         {
            for (int d=0; d<d_dims; d++)
            {
               whitened_SIFT_descriptor_ptr[i]=
                  inverse_sqrt_covar[i][d] * raw_SIFT_descriptor_ptr[d];
            }
         }
         
         outstream.write(
            (char *) whitened_SIFT_descriptor_ptr,d_dims*sizeof(float));
         n_total_features++;

      } // loop over index f labeling features in curr keyfile
      
      delete [] floats_ptr;

      keyfile_counter++;

      double rejected_descriptor_frac=double(n_rejected_descriptors)/
         n_considered_descriptors;
      cout << keyfile_counter << "/" << n_images 
           << " image_ID = " << image_ID
           << " curr_nfeatures=" << n_features_in_curr_keyfile 
           << " n_total_features=" << stringfunc::number_to_string(
              1E-6*n_total_features,1) << "M  "
           << " rejected_descriptor_frac = " << rejected_descriptor_frac
           << endl;
      
// Delete uncompressed binary descriptor file:

      unix_cmd="/bin/rm "+descriptor_filename;
      sysfunc::unix_command(unix_cmd);

   }   // n_total_features < n_requested_total_features while loop
   cout << endl;

   filefunc::closefile(concatenated_keys_filename,outstream);

   delete raw_descriptor_ptr;
   delete [] raw_SIFT_descriptor_ptr;
   delete [] whitened_SIFT_descriptor_ptr;
   delete [] inverse_sqrt_covar.ptr();

   double rejected_descriptors_frac=double(n_rejected_descriptors)/
      n_considered_descriptors;
   double avg_n_features_per_keyfile=double(n_total_features)/keyfile_counter;
   cout << "n_images = " << n_images << endl;
   cout << "i_skip = " << i_skip << endl;
   cout << "keyfile_counter = " << keyfile_counter << endl;
   cout << "n_considered_descriptors = " 
        << stringfunc::number_to_string(n_considered_descriptors/1000000)+" M"
        << endl;
   cout << "n_total_features = " 
        << stringfunc::number_to_string(n_total_features/1000000)+" M"
        << endl;
   cout << "rejected descriptors fraction = " << rejected_descriptors_frac
        << endl;
   cout << "<n_features per keyfile> = " << avg_n_features_per_keyfile
        << endl;
   outputfunc::print_elapsed_time();

   cout << "At end of program CONCATENATE_BINARY_KEYS" << endl;
   outputfunc::print_elapsed_time();
} 

