// ==========================================================================
// Program DESCRIPTORS_COVAR parses all compressed binary SIFT
// descriptor files within a specified input subdirectory.  It
// sequentially forms first and second moments from SIFT descriptors
// as they're individually imported.  Once all SIFT descriptors have
// been processed, their 128x128 covariance matrix is calculated.  The
// mean, covariance, square root and inverse square root covariance
// matrices are written to text and binary files within the sift keys
//  subdirectory.
// ==========================================================================
// Last updated on 12/3/12; 8/18/13; 8/19/13
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
   int i_skip=100;

   const int D=128;
   genvector* recur_mean_ptr=new genvector(D);
   recur_mean_ptr->clear_values();
   genmatrix* recur_second_moment_ptr=new genmatrix(D,D);
   recur_second_moment_ptr->clear_values();

// Import SIFT descriptors for individual "1 through N" images from
// either gzipped or lzop compressed key files:

   genvector curr_A(D);
   genmatrix* outerproduct_ptr=new genmatrix(D,D);

   int n_total_features=0;
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
            curr_A.put(
               d,stringfunc::unsigned_char_to_ascii_integer(
                  data_ptr[byte_counter++]));
         }

         mathfunc::recursive_mean(D,n_total_features,&curr_A,recur_mean_ptr);
         mathfunc::recursive_second_moment(
            D,n_total_features,&curr_A,outerproduct_ptr,
            recur_second_moment_ptr);
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

   string txt_mean_filename=sift_keys_subdir+"mean.txt";
   string bin_mean_filename=sift_keys_subdir+"mean.bin";
   recur_mean_ptr->export_to_dense_text_format(txt_mean_filename);
   recur_mean_ptr->export_to_dense_binary_format(bin_mean_filename);

// Compute covariance matrix from recursively calculated first and
// second moments for all SIFT descriptors:

   genmatrix recur_mean_outerprod(D,D);
   recur_mean_outerprod=recur_mean_ptr->outerproduct(*recur_mean_ptr);
//   cout << "recur_mean_outerprod = " << recur_mean_outerprod << endl;
   string txt_mean_outerprod_filename=sift_keys_subdir+"mean_outerprod.txt";
   string bin_mean_outerprod_filename=sift_keys_subdir+"mean_outerprod.bin";
   recur_mean_outerprod.export_to_dense_text_format(
      txt_mean_outerprod_filename);
   recur_mean_outerprod.export_to_dense_binary_format(
      bin_mean_outerprod_filename);

   genmatrix* recur_covar_ptr=new genmatrix(D,D);
   *recur_covar_ptr = *recur_second_moment_ptr - recur_mean_outerprod;
//   cout << "*recur_covar_ptr = " << *recur_covar_ptr << endl;
   string txt_covar_filename=sift_keys_subdir+"covar_matrix.txt";
   string bin_covar_filename=sift_keys_subdir+"covar_matrix.bin";
   recur_covar_ptr->export_to_dense_text_format(txt_covar_filename);
   recur_covar_ptr->export_to_dense_binary_format(bin_covar_filename);

   genmatrix* sqrt_covar_ptr=new genmatrix(D,D);
   recur_covar_ptr->square_root(*sqrt_covar_ptr);
   delete recur_covar_ptr;
   string txt_sqrt_covar_filename=sift_keys_subdir+"sqrt_covar_matrix.txt";
   string bin_sqrt_covar_filename=sift_keys_subdir+"sqrt_covar_matrix.bin";
   sqrt_covar_ptr->export_to_dense_text_format(txt_sqrt_covar_filename);
   sqrt_covar_ptr->export_to_dense_binary_format(bin_sqrt_covar_filename);

   genmatrix* inverse_sqrt_covar_ptr=new genmatrix(D,D);
   sqrt_covar_ptr->inverse(*inverse_sqrt_covar_ptr);

// Export inverse square root of SIFT feature covariance matrix to
// text file so that it does not need to be computed more than once:

   string txt_inverse_sqrt_covar_filename=
      sift_keys_subdir+"inverse_sqrt_covar_matrix.txt";
   string bin_inverse_sqrt_covar_filename=
      sift_keys_subdir+"inverse_sqrt_covar_matrix.bin";
   inverse_sqrt_covar_ptr->export_to_dense_text_format(
      txt_inverse_sqrt_covar_filename);
   inverse_sqrt_covar_ptr->export_to_dense_binary_format(
      bin_inverse_sqrt_covar_filename);

   string banner=
      "Wrote inverse square root of SIFT feature covariance matrix to "+
      txt_inverse_sqrt_covar_filename;
   outputfunc::write_big_banner(banner);

   cout << "At end of program DESCRIPTORS_COVAR" << endl;
   outputfunc::print_elapsed_time();
}

   
