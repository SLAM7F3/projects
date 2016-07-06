// ==========================================================================
// Program NPLUSONE queries the user to enter a full path to some
// "N+1st" image.  It then calls Lowe's binary and generates a SIFT
// key file in text and HDF5 binary formats.  The SIFT descriptors are
// subsequently whitened via multiplication with the inverse square
// root SIFT feature covariance matrix.  The whitened SIFT descriptors
// for the N+1st image are exported to an output HDF5 binary file in a
// sift_keys_subdir/nplusone/
// ==========================================================================
// Last updated on 4/10/12; 4/30/12; 5/31/13; 6/15/13; 6/7/14
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "cluster/akm.h"
#include "datastructures/descriptor.h"
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
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
   PassesGroup passes_group(&arguments);

   string image_list_filename=passes_group.get_image_list_filename();
//   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   string image_sizes_filename=passes_group.get_image_sizes_filename();
   string sift_keys_subdir=bundler_IO_subdir+"images/keys/";
   cout << "sift_keys_subdir = " << sift_keys_subdir << endl;
   string nplusone_subdir=sift_keys_subdir+"nplusone/";
   cout << "nplusone_subdir = " << nplusone_subdir << endl;
   filefunc::dircreate(nplusone_subdir);

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;

   bool FLANN_flag=true;
   sift_detector* sift_detector_ptr=new sift_detector(
      photogroup_ptr,FLANN_flag);

   string image_filename;
   cout << "Enter N+1st image filename:" << endl;
   cin >> image_filename;

//   string image_filename=
//      "/home/cho/programs/c++/svn/projects/src/mains/photosynth/bundler/fundamental/images/tower_pushcart.jpg";

   unsigned int image_xdim,image_ydim;
   imagefunc::get_image_width_height(image_filename,image_xdim,image_ydim);

// Generate SIFT key file for input image in text and HDF5 binary
// formats:
   
   string sift_keys_filename=sift_detector_ptr->generate_Lowe_keyfile(
      sift_keys_subdir,image_filename);

   bool Lowe_SIFT_flag=true;
   vector<sift_detector::feature_pair> currimage_feature_info;         
   sift_detector_ptr->parse_Lowe_features(
      Lowe_SIFT_flag,image_xdim,image_ydim,sift_keys_filename,
      currimage_feature_info);
      
   vector<descriptor*>* D_ptrs_ptr=new vector<descriptor*>;
   for (unsigned int f=0; f<currimage_feature_info.size(); f++)
   {
      descriptor* curr_D_ptr=currimage_feature_info[f].second;
      D_ptrs_ptr->push_back(curr_D_ptr);
   } // loop over index f labeling SIFT features

   string output_file_prefix="raw_";
   string hdf5_filename=
      sift_detector_ptr->get_akm_ptr()->export_SIFT_features_in_HDF5_format(
         sift_keys_filename,nplusone_subdir,output_file_prefix,
         D_ptrs_ptr);
   
// Destroy all previously instantiated descriptors:

   for (unsigned int j=0; j<currimage_feature_info.size(); j++)
   {
      delete currimage_feature_info[j].first;
      delete currimage_feature_info[j].second;
   }
   delete D_ptrs_ptr;
   delete sift_detector_ptr;

// Import square root of SIFT feature covariance matrix from text file:

   string inverse_covar_sqrt_filename=
      sift_keys_subdir+"inverse_sqrt_covar_matrix.dat";
   filefunc::ReadInfile(inverse_covar_sqrt_filename);

   const int D=128;   
   genmatrix* inverse_covar_sqrt_matrix_ptr=new genmatrix(D,D);
   inverse_covar_sqrt_matrix_ptr->clear_values();

   int line_counter=0;
   for (int i=0; i<D; i++)
   {
      for (int j=0; j<D; j++)
      {
         double curr_value=stringfunc::string_to_number(
            filefunc::text_line[line_counter]);
         inverse_covar_sqrt_matrix_ptr->put(i,j,curr_value);
         line_counter++;         
      }
   }
//   cout << "*inverse_covar_sqrt_matrix_ptr = " 
//        << *inverse_covar_sqrt_matrix_ptr << endl;
//   cout << "inverse_covar_sqrt_matrix_ptr->rank() = "
//        << inverse_covar_sqrt_matrix_ptr->rank() << endl;

// Re-import SIFT descriptors for N+1st image and whiten them
// by multiplying with *inverse_covar_sqrt_ptr:

   cout << endl;
   cout << "Whitening " << hdf5_filename << endl;


   flann::Matrix<float> SIFT_descriptors;
   flann::load_from_file(
      SIFT_descriptors,hdf5_filename.c_str(),"sift_features");

   genvector curr_A(D);
   int N=SIFT_descriptors.rows;
   for (int n=0; n<N; n++)
   {
      for (int d=0; d<D; d++)
      {
         curr_A.put(d,SIFT_descriptors[n][d]);
      } // loop over index d
      curr_A=*inverse_covar_sqrt_matrix_ptr * curr_A;
      for (int d=0; d<D; d++)
      {
         SIFT_descriptors[n][d]=curr_A.get(d);
      } // loop over index d
   } // loop over index n 

   output_file_prefix="whitened_";
   string basename=filefunc::getbasename(sift_keys_filename);
   string whitened_filename=nplusone_subdir+output_file_prefix+
      stringfunc::prefix(basename)+".hdf5";
   flann::save_to_file(SIFT_descriptors,whitened_filename,"sift_features");

   string banner="Whitened SIFT features for N+1st image exported to "+
      whitened_filename;
   outputfunc::write_big_banner(banner);
}

   
