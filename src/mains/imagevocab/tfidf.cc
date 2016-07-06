// ==========================================================================
// Program TFIDF reads in word counts for text files calculated by
// program GENERATE_IMAGE_WORDS.  It counts the number of words for
// each image.  TFIDF exports renormalized term frequencies to output
// text files within a term_freqs subdirectory of sift_keys_subdir.
// It also computes inverse document frequencies within an output
// binary HDF5 file.  
// ==========================================================================
// Last updated on 4/29/12; 4/30/12
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

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);
   
   string image_list_filename=passes_group.get_image_list_filename();
//   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   string sift_keys_subdir=bundler_IO_subdir+"images/keys/";
   cout << "sift_keys_subdir = " << sift_keys_subdir << endl;
   string image_words_subdir=sift_keys_subdir+"image_words/";
   cout << "image_words_subdir = " << image_words_subdir << endl;
   string term_freqs_subdir=sift_keys_subdir+"term_freqs/";
   cout << "term_freqs_subdir = " << term_freqs_subdir << endl;
   filefunc::dircreate(term_freqs_subdir);

   string substring="image_word";
   vector<string> image_word_filenames=
      filefunc::file_basenames_in_subdir_matching_substring(
         image_words_subdir,substring);
   int n_images=image_word_filenames.size();

// First import word clusters for entire imagery database only to
// determine number of clusters K:

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
   delete [] cluster_centers.ptr();

// Import image words for individual "1 through N" images:

   bool FLANN_flag=true;
   akm* akm_ptr=new akm(FLANN_flag);

   timefunc::initialize_timeofday_clock();
   int i_start=0;
   cout << "Enter i_start:" << endl;
   cin >> i_start;
   int i_stop=n_images;

   akm_ptr->clear_document_word_frequencies(K);
   for (int index=i_start; index<i_stop; index++)
   {
      cout << "Processing image " << index << " of " << n_images << endl;

      string image_words_filename=image_word_filenames[index];
      akm_ptr->compute_term_frequencies(
         index,image_words_filename,term_freqs_subdir);
   } // loop over index i labeling individual images

   string banner="Wrote term frequencies to "+term_freqs_subdir;
   outputfunc::write_big_banner(banner);

   akm_ptr->compute_inverse_document_frequencies(
      n_images,sift_keys_subdir);

   banner="Wrote inverse document frequencies to "+sift_keys_subdir;
   outputfunc::write_big_banner(banner);
}
