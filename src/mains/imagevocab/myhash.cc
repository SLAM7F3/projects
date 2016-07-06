// ==========================================================================
// Program MYHASH imports a subset of compressed binary SIFT
// keyfiles from a specified subdirectory.  It samples some fraction
// of each binary keyfile's SIFT descriptors.  The O(10**8) input SIFT
// descriptors are accumulated within an STL vector of dlib column
// vectors.  We then call dlib's create_random_projection_hash()
// method.  The resulting projection hash object is serialized and
// written out to a binary file.  

/*

n_total_features = 100016490
n_bits = 16
n_voronoi_cells = 65536
<features/voronoi cell = 1526.130523681641
1/n_voronoi_cells = 1.52587890625e-05
Cell fractional occupancy = 1.525878906250004e-05 +/- 1.071921682237772e-05
Elapsed time = 2687.3 secs =   44.79 minutes =   0.746 hours 

n_total_features = 32023363
n_bits = 12
n_voronoi_cells = 4096
<features/voronoi cell = 7818.203857421875
1/n_voronoi_cells = 0.000244140625
Cell fractional occupancy = 0.0002441406249999995 +/- 0.0001114064661756599
Elapsed time = 769.1 secs =   12.82 minutes =   0.214 hours 
*/

// ==========================================================================
// Last updated on 8/23/13; 8/24/13; 8/29/13
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "math/binaryfuncs.h"
#include "general/filefuncs.h"
#include "math/prob_distribution.h"
#include "numrec/nrfuncs.h"
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
   string voronoi_subdir=sift_keys_subdir+"voronoi/";
   filefunc::dircreate(voronoi_subdir);
   string binary_descriptors_subdir=sift_keys_subdir+"binary_descriptors/";

// Import mean SIFT descriptor vector into a floating point array:

   string txt_mean_filename=sift_keys_subdir+"mean.txt";
   genvector* mean_descriptor_ptr=mathfunc::
      import_genvector_from_dense_text_format(txt_mean_filename);
   int d_dims=mean_descriptor_ptr->get_mdim();

   cout << "d_dims = " << d_dims << endl;
   float* mean_SIFT_descriptor_ptr=new float[d_dims];
   for (int d=0; d<d_dims; d++)
   {
      mean_SIFT_descriptor_ptr[d]=mean_descriptor_ptr->get(d);
   }
   delete mean_descriptor_ptr;

// Import inverse square root of SIFT descriptors covariance matrix
// into a FLANN matrix array:

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
//         cout << inverse_sqrt_covar[i][j] << " " << flush;
      }
//      cout << endl;
   }
   delete inverse_sqrt_covar_ptr;

// Following Davis King's suggestion, we generate an n_bits x d_dims
// matrix where each row is filled with N(0,1) random variables:

//   int n_bits=8;	// 256 Voronoi cells
//   int n_bits=12;	// 4096 Voronoi cells
   int n_bits=16;	// 65536 Voronoi cells
//   int n_bits=17;
//   int n_rows=32;
   unsigned long n_voronoi_cells=pow(2,n_bits);
   cout << "n_voronoi_cells = " << n_voronoi_cells << endl;
   
   vector<int> voronoi_cell_occupancy;
   for (int v=0; v<n_voronoi_cells; v++)
   {
      voronoi_cell_occupancy.push_back(0);
   }

   flann::Matrix<float> hash_matrix(
      new float[n_bits*d_dims],n_bits,d_dims);

   for (int b=0; b<n_bits; b++)
   {
      for (int d=0; d<d_dims; d++)
      {
         hash_matrix[b][d]=nrfunc::gasdev();
      } // loop over index d labeling columns in hash_matrix
   } // loop over index b labeling rows in hash_matrix

// Import descriptors from compressed binary key files:

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("lzo");
   vector<string> compressed_descriptor_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,binary_descriptors_subdir);

   int n_images=compressed_descriptor_filenames.size();
//   int i_skip=101;	// prime which doesn't divide n_images=110074
   int i_skip=997;	// prime which doesn't divide n_images=110074
//   long long n_requested_total_features=1000000;  // 1 million
//   long long n_requested_total_features=3200000;  // 3.2 million
//   long long n_requested_total_features=10000000;  // 10 million
//   long long n_requested_total_features=32000000;  // 32 million
   long long n_requested_total_features=100000000;  // 100 million

   cout << "n_images = " << n_images << endl;
   cout << "i_skip = " << i_skip << endl;
   cout << "n_requested_total_features = " << n_requested_total_features
        << endl;
//   outputfunc::enter_continue_char();
   sleep(3);
   timefunc::initialize_timeofday_clock();

// Import descriptors from compressed binary key files:

   int image_ID=0;
   long long n_total_features=0;
   int keyfile_counter=0;
   ofstream outstream;

   float* raw_SIFT_descriptor_ptr=new float[d_dims];
   float* whitened_SIFT_descriptor_ptr=new float[d_dims];

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

      int n_features_in_curr_keyfile=
         filefunc::size_of_file_in_bytes(descriptor_filename)/d_dims;
      int n_bytes_in_curr_keyfile=n_features_in_curr_keyfile*d_dims;
      unsigned char* data_ptr=data_ptr=filefunc::ReadUnsignedChars(
         descriptor_filename,n_bytes_in_curr_keyfile);

      float* floats_ptr=new float[n_bytes_in_curr_keyfile];
      for (int b=0; b<n_bytes_in_curr_keyfile; b++)
      {
         floats_ptr[b]=stringfunc::unsigned_char_to_ascii_integer(data_ptr[b]);
      }
      delete [] data_ptr;

// Set up STL vector to hold voronoi cell occupation numbers as
// function of voronoi cell ID for individual images:

      vector<double> curr_voronoi_cell_occupancy;
      for (int i=0; i<n_voronoi_cells; i++)
      {
         curr_voronoi_cell_occupancy.push_back(0);
      }

// Whiten features imported from current keyfile after subtracting off
// mean descriptor values.  

      for (int f=0; f<n_features_in_curr_keyfile; f++)
      {
         for (int d=0; d<d_dims; d++)
         {
            int b=f*d_dims+d;
            raw_SIFT_descriptor_ptr[d]=floats_ptr[b];
//            cout << raw_SIFT_descriptor_ptr[d] << " " << flush;
         } // loop over index d 
//         cout << endl;
         
         for (int i=0; i<d_dims; i++)
         {
            whitened_SIFT_descriptor_ptr[i]=0;
            for (int d=0; d<d_dims; d++)
            {
               whitened_SIFT_descriptor_ptr[i] += 
                  inverse_sqrt_covar[i][d] * 
                  (raw_SIFT_descriptor_ptr[d]-mean_SIFT_descriptor_ptr[d]);
            }
//            cout << whitened_SIFT_descriptor_ptr[i] << " " << flush;
         }
//         cout << endl;

// Multiply whitened SIFT decriptor by hash_matrix.  Map result to
// n_bits binary string:

         string binary_str;
         for (int b=0; b<n_bits; b++)
         {
            float curr_value=0;
            for (int d=0; d<d_dims; d++)
            {
               curr_value += hash_matrix[b][d]*whitened_SIFT_descriptor_ptr[d];
            }

            if (curr_value > 0)
            {
               binary_str += "1";
            }
            else
            {
               binary_str += "0";
            }
         } // loop over index b labeling hash_matrix rows
         unsigned long voronoi_cell_ID=
            binaryfunc::binary_string_to_eight_byte_integer(binary_str);
         
//         cout << binary_str << " = " 
//              << voronoi_cell_ID 
//              << endl;

         voronoi_cell_occupancy[voronoi_cell_ID]=
            voronoi_cell_occupancy[voronoi_cell_ID]+1;
         curr_voronoi_cell_occupancy[voronoi_cell_ID]=
            curr_voronoi_cell_occupancy[voronoi_cell_ID]+1;
         
      } // loop over index f labeling features in curr keyfile
      
      delete [] floats_ptr;
      n_total_features += n_features_in_curr_keyfile;
      keyfile_counter++;

// Compute fraction of SIFT words touched by current image:

      int n_touched_cells=0;
      for (int v=0; v<n_voronoi_cells; v++)
      {
         if (curr_voronoi_cell_occupancy[v] > 0) n_touched_cells++;
      }
      double touched_cell_frac=
         double(n_touched_cells)/double(n_voronoi_cells);

      cout << keyfile_counter
           << " image_ID = " << image_ID
           << " curr_nfeatures=" << n_features_in_curr_keyfile 
           << " n_touched_cells = " << n_touched_cells
           << " touched_cell_frac = " << touched_cell_frac
           << " n_total_features=" << 1E-6*n_total_features << "M  "
           << endl;
      
// Export current image's voronoi cell occupancy histogram to output
// text file:

      string descriptor_basename=filefunc::getbasename(descriptor_filename);
      string input_filename_prefix=stringfunc::prefix(descriptor_basename);
      string voronoi_filename=voronoi_subdir+input_filename_prefix+".voronoi";
      filefunc::openfile(voronoi_filename,outstream);
      for (int v=0; v<n_voronoi_cells; v++)
      {
         outstream << curr_voronoi_cell_occupancy[v] << " ";
         if (v%25==0 && v != 0) outstream << endl;
      }
      outstream << endl;
      filefunc::closefile(voronoi_filename,outstream);

// Delete uncompressed binary descriptor file:

      unix_cmd="/bin/rm "+descriptor_filename;
      sysfunc::unix_command(unix_cmd);

   }   // n_total_features < n_requested_total_features while loop
   cout << endl;

   outputfunc::print_elapsed_time();

   vector<double> frac_voronoi_cell_occupancy;
   for (int v=0; v<n_voronoi_cells; v++)
   {
      frac_voronoi_cell_occupancy.push_back(
         double(voronoi_cell_occupancy[v])/n_total_features);
      cout << "Voronoi cell " << v 
           << ": occupancy = " << voronoi_cell_occupancy[v] 
           << " , frac = " << frac_voronoi_cell_occupancy.back()
           << endl;
   }

   cout << endl;
   cout << "n_total_features = " << n_total_features << endl;
   cout << "n_bits = " << n_bits << endl;
   cout << "n_voronoi_cells = " << n_voronoi_cells << endl;
   cout << "<features/voronoi cell = " 
        << double(n_total_features)/n_voronoi_cells << endl;

   cout << "1/n_voronoi_cells = " << 1.0/double(n_voronoi_cells) << endl;
   cout << "Cell fractional occupancy = " 
        << mathfunc::mean(frac_voronoi_cell_occupancy) << " +/- " 
        << mathfunc::std_dev(frac_voronoi_cell_occupancy) << endl;

   delete [] raw_SIFT_descriptor_ptr;
   delete [] whitened_SIFT_descriptor_ptr;
   delete [] inverse_sqrt_covar.ptr();
   delete [] hash_matrix.ptr();

   cout << "At end of program MYHASH" << endl;
   outputfunc::print_elapsed_time();

} 

