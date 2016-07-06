// ==========================================================================
// Program COVAR_DESCRIPTORS parses all compressed SIFT HDF5 binary
// files within a specified input subdirectory.  It sequentially forms
// first and second moments from SIFT descriptors as they're
// individually imported.  Once all SIFT descriptors have been
// processed, their 128x128 covariance matrix is calculated.  The
// square root of the inverse covariance matrix is written to a text
// file within the sift keys subdirectory.
// ==========================================================================
// Last updated on 4/17/12; 4/19/12; 12/3/12; 4/6/14
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <flann/flann.hpp>
#include <flann/io/hdf5.h>

#include "cluster/akm.h"
#include "general/filefuncs.h"
#include "passes/PassesGroup.h"
#include "video/sift_detector.h"
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
   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   string image_sizes_filename=passes_group.get_image_sizes_filename();
   string sift_keys_subdir=bundler_IO_subdir+"images/keys/";
   cout << "sift_keys_subdir = " << sift_keys_subdir << endl;
   string raw_hdf5_subdir=sift_keys_subdir+"raw/";
   cout << "raw_hdf5_subdir = " << raw_hdf5_subdir << endl;

   bool FLANN_flag=true;
   sift_detector* sift_detector_ptr=new sift_detector(NULL,FLANN_flag);
   vector<string> compressed_sift_hdf5_filenames=sift_detector_ptr->
      import_compressed_sift_hdf5_filenames(raw_hdf5_subdir);
   int n_images=compressed_sift_hdf5_filenames.size();
   delete sift_detector_ptr;

   const int D=128;
   genvector* recur_mean_ptr=new genvector(D);
   recur_mean_ptr->clear_values();
   genmatrix* recur_second_moment_ptr=new genmatrix(D,D);
   recur_second_moment_ptr->clear_values();

// Import SIFT descriptors for individual "1 through N" images from
// either gzipped or lzop compressed key files:

   genvector curr_A(D);
   flann::Matrix<float> SIFT_descriptors;
   int counter=0;

   int i_start=0;
   cout << "Enter starting image number:" << endl;
   cin >> i_start;

   genmatrix* outerproduct_ptr=new genmatrix(D,D);

   int i_stop=n_images;
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
      cout << "i = " << i << " Importing " << sift_hdf5_filename << endl;

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

      int N=SIFT_descriptors.rows;
      for (int n=0; n<N; n++)
      {
         for (int d=0; d<D; d++)
         {
            curr_A.put(d,SIFT_descriptors[n][d]);
         } // loop over index d

//         mathfunc::recursive_mean(counter,&curr_A,recur_mean_ptr);
         mathfunc::recursive_mean(D,counter,&curr_A,recur_mean_ptr);
//         cout << "recur_mean(0) = " << recur_mean_ptr->get(0)
//              << " recur_mean(1) = " << recur_mean_ptr->get(1)
//              << " recur_mean(2) = " << recur_mean_ptr->get(2) << endl;

//         mathfunc::recursive_second_moment(
//            counter,&curr_A,recur_second_moment_ptr);
         mathfunc::recursive_second_moment(
            D,counter,&curr_A,outerproduct_ptr,recur_second_moment_ptr);

//         cout << "second_mom(0,0) = " << recur_second_moment_ptr->get(0,0)
//              << endl;
//         cout << "second_mom(0,1) = " << recur_second_moment_ptr->get(0,1)
//              << endl;
//         cout << "second_mom(1,0) = " << recur_second_moment_ptr->get(1,0)
//              << endl;
//         cout << "second_mom(1,1) = " << recur_second_moment_ptr->get(1,1)
//              << endl;
//         outputfunc::enter_continue_char();

         counter++;
      } // loop over index n labeling SIFT features in current image
   } // loop over index i labeling individual images

// Compute covariance matrix from recursively calculated first and
// second moments for all SIFT descriptors:

   genmatrix recur_mean_outerprod(D,D);
//   recur_mean_outerprod=recur_mean_ptr->outerproduct(*recur_mean_ptr);
   recur_mean_ptr->outerproduct(*recur_mean_ptr,recur_mean_outerprod);

//   cout << "recur_mean_outerprod = " << recur_mean_outerprod << endl;

   genmatrix* recur_covar_ptr=new genmatrix(D,D);
   *recur_covar_ptr = *recur_second_moment_ptr - recur_mean_outerprod;
//   cout << "*recur_covar_ptr = " << *recur_covar_ptr << endl;

   genmatrix* covar_sqrt_ptr=new genmatrix(D,D);
   recur_covar_ptr->square_root(*covar_sqrt_ptr);
   delete recur_covar_ptr;

   genmatrix* inverse_covar_sqrt_ptr=new genmatrix(D,D);
   covar_sqrt_ptr->inverse(*inverse_covar_sqrt_ptr);

// Export inverse square root of SIFT feature covariance matrix to
// text file so that it does not need to be computed more than once:

   string covar_sqrt_filename=
      sift_keys_subdir+"sqrt_covar_matrix.dat";
   string inverse_covar_sqrt_filename=
      sift_keys_subdir+"inverse_sqrt_covar_matrix.dat";
   ofstream outstream,inverse_outstream;
   filefunc::openfile(covar_sqrt_filename,outstream);
   filefunc::openfile(inverse_covar_sqrt_filename,inverse_outstream);

   for (int i=0; i<D; i++)
   {
      for (int j=0; j<D; j++)
      {
         outstream << covar_sqrt_ptr->get(i,j) << endl;
         inverse_outstream << inverse_covar_sqrt_ptr->get(i,j) << endl;
      }
   }
   filefunc::closefile(covar_sqrt_filename,outstream);   
   filefunc::closefile(inverse_covar_sqrt_filename,inverse_outstream);   

   string banner=
      "Wrote inverse square root of SIFT feature covariance matrix to "+
      inverse_covar_sqrt_filename;
   outputfunc::write_big_banner(banner);
 

}

   
