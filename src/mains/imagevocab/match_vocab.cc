// ==========================================================================
// Program MATCH_VOCAB first imports 128D descriptors for voronoi
// cell centroids.  It also reads in the vocabulary output by program
// GENERATE_VOCAB.  MATCH_VOCAB then queries the user to enter
// whitened SIFT feature descriptors within an HDF5 binary file for
// some "N+1st" image.  Dotproducts between the input image and the
// "N" database images are calculated and sorted.  The archive images
// with the highest dotproducts with the N+1st image are returned by
// this program.
// ==========================================================================
// Last updated on 4/10/12; 4/11/12; 4/30/12
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <flann/flann.hpp>
#include <flann/io/hdf5.h>

#include "cluster/akm.h"
#include "general/filefuncs.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
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
//   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   string sift_keys_subdir=bundler_IO_subdir+"images/keys/";
   string tfidfs_subdir=sift_keys_subdir+"tfidfs/";
   cout << "tfidfs_subdir = " << tfidfs_subdir << endl;
   string nplusone_subdir=sift_keys_subdir+"nplusone/";
   cout << "nplusone_subdir = " << nplusone_subdir << endl;

   string substring="tfidfs_";
   vector<string> tfidf_filenames=
      filefunc::file_basenames_in_subdir_matching_substring(
         tfidfs_subdir,substring);
   int n_images=tfidf_filenames.size();
   cout << "n_images = tfidf_filenames.size() = " 
        << tfidf_filenames.size() << endl;

// First import word clusters for entire imagery database:

   string cluster_centers_filename=sift_keys_subdir+
      "whitened_large_cluster_centers.hdf5";
   cout << "cluster_centers_filename = " << cluster_centers_filename
        << endl;

   flann::Matrix<float> cluster_centers;
   flann::load_from_file(
      cluster_centers, cluster_centers_filename.c_str(),"cluster_centers");

   int K=cluster_centers.rows;
   cout << "K = n_cluster_centers = " << K << endl;
   int D=cluster_centers.cols;
   cout << "D = " << D << endl;

   bool FLANN_flag=true;
   akm* akm_ptr=new akm(FLANN_flag);
   akm_ptr->set_cluster_centers_matrix_ptr(&cluster_centers);
   akm_ptr->clear_document_word_frequencies(K);
   akm_ptr->import_inverse_doc_freq_matrix(sift_keys_subdir);

// Next import SIFT descriptors for "N+1st" image:

   string input_SIFT_filename;
   cout << "Enter name of HDF5 file containing whitened SIFT descriptors for N+1st image:" << endl;
   cin >> input_SIFT_filename;

//   input_SIFT_filename=
//      "/data_second_disk/bundler/MIT_100/images/keys/nplusone/whitened_CIMG0367.hdf5";
//       "/data/ImageEngine/MIT32K_and_aerial/keys/whitened_CIMG0367.hdf5";   

   flann::Matrix<float> SIFT_descriptors;
   flann::load_from_file(
      SIFT_descriptors,input_SIFT_filename.c_str(),"sift_features");

   int N=SIFT_descriptors.rows;
   akm_ptr->reset_params(N,D,K);
   akm_ptr->set_SIFT_descriptors_matrix_ptr(&SIFT_descriptors);

   int curr_image_ID=n_images;
   akm_ptr->compute_term_frequencies(curr_image_ID);
   akm_ptr->compute_tfidfs_for_nplusone_image(curr_image_ID,nplusone_subdir);

   int i_start=0;
   int i_stop=n_images;
   string image_basename;
   double max_dotproduct=-1;
   string closest_matching_image_basename;
   vector<int> image_indices;
   vector<double> dotproducts;
   vector<string> image_basenames;
   for (int index=i_start; index<i_stop; index++)
   {
      image_indices.push_back(index);
      string archive_tfidfs_filename=tfidf_filenames[index];
      double curr_dotproduct=akm_ptr->
         compare_Nplusone_image_with_archive_image(
            archive_tfidfs_filename,image_basename);
      dotproducts.push_back(curr_dotproduct);
      image_basenames.push_back(image_basename);
      if (curr_dotproduct > max_dotproduct)
      {
         max_dotproduct=curr_dotproduct;
         closest_matching_image_basename=image_basename;
      }
      cout << index << "/" << n_images 
           << " dotprod=" << curr_dotproduct
           << " max_dot=" << max_dotproduct 
           << " best match=" << closest_matching_image_basename 
           << endl;
   }   
   templatefunc::Quicksort_descending(
      dotproducts,image_indices,image_basenames);

// Compute dotproducts between images "1 thru N" and "N+1st" image:

   int n_matching_images=basic_math::min(10,int(image_indices.size()));
   for (int i=0; i<n_matching_images; i++)
   {
      int curr_index=image_indices[i];
      cout << "i = " << i
           << " dotproduct = " << dotproducts[i]
           << " image = " << image_basenames[i] 
           << endl;
   }

   
}
