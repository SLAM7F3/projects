// ========================================================================
// Program REDUCE_FEATURES imports global "gist-like" feature
// descriptors exported by EXTRACT_FEATURES.

//			./reduce_features

// ========================================================================
// Last updated on 2/10/16; 2/11/16
// ========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <dlib/matrix.h>

#include "math/basic_math.h"
#include "math/compressfuncs.h"
#include "general/filefuncs.h"
#include "math/genmatrix.h"
#include "math/genvector.h"
#include "general/outputfuncs.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

// ------------------------------------------------------------------

int main() 
{
   string software_subdir = "/home/pcho/software/";
   string caffe_subdir = software_subdir+"caffe_custom/";
   string _temp_subdir = caffe_subdir+"examples/_temp/";
   string features_subdir = "features_roadsigns/";
  
   filefunc::add_trailing_dir_slash(features_subdir);
   features_subdir = _temp_subdir+features_subdir;
   cout << "Full features_subdir = " << features_subdir << endl;

   timefunc::initialize_timeofday_clock();

// Import features from text file:

   string banner="Importing features:";
   outputfunc::write_banner(banner);
   string features_filename = features_subdir + "global_features.dat";
   filefunc::ReadInfile(features_filename);
   filefunc::text_line[0];

   vector<double> column_values = stringfunc::string_to_numbers(
      filefunc::text_line[0]);
   int n_feature_vectors = column_values[0];
   int n_feature_dims = column_values[1];
   int k = 128;
   cout << "Enter k = reduced dimension for randomly projected subspace:"
        << endl;
   cin >> k;

   cout << "n_feature_vectors = " << n_feature_vectors
        << " n_feature_dims = " << n_feature_dims << endl;
   cout << "k = " << k << endl;

   genmatrix* X_ptr = new genmatrix(n_feature_dims, n_feature_vectors);
   for(int f = 0; f < n_feature_vectors; f++)
   {
      outputfunc::update_progress_fraction(f, 1000, n_feature_vectors);
      
      vector<double> curr_column_values = stringfunc::string_to_numbers(
         filefunc::text_line[f+1]);

      for(int i = 0; i < n_feature_dims; i++)
      {
         double curr_value = curr_column_values[i];
         X_ptr->put(i, f, curr_value);
      }
   } // loop over index f labeling feature vectors
   cout << endl;

//   cout << "Data matrix X = " << *X_ptr << endl;

   genmatrix R(k, n_feature_dims);
   compressfunc::generate_random_projection_matrix(R);
   outputfunc::print_elapsed_time();

//   genmatrix RRtrans(R * R.transpose());
//   cout << "R*Rtrans = " << RRtrans << endl;

   genmatrix *Xreduced_ptr = new genmatrix(k, n_feature_vectors);
   compressfunc::randomly_project_data_matrix(*X_ptr, R, *Xreduced_ptr);

   vector<double> orig_over_reduced_ratios;
   for(int n = 0; n < n_feature_vectors; n++)
   {
      outputfunc::update_progress_fraction(n, 500, n_feature_vectors);

      for(int p = n; p < n_feature_vectors; p++)
      {
         double orig_dotproduct=X_ptr->columns_dotproduct(n, p);
         double reduced_dotproduct=Xreduced_ptr->columns_dotproduct(n, p);
         orig_over_reduced_ratios.push_back(
            orig_dotproduct / reduced_dotproduct);
      } // loop over index p labeling samples
   } // loop over index n labeling samples

   prob_distribution prob_ratios(orig_over_reduced_ratios, 100);
   prob_ratios.set_title("Orig / reduced dot product ratios");

   string subtitle="K = "+stringfunc::number_to_string(k)+
      " for randomly projected subspace";
   prob_ratios.set_subtitle(subtitle);
   prob_ratios.set_densityfilenamestr("ratios_dens.meta");
   prob_ratios.set_cumulativefilenamestr("ratios_cum.meta");
   prob_ratios.set_xmin(prob_ratios.mean() - 3 * prob_ratios.std_dev() );
   prob_ratios.set_xmax(prob_ratios.mean() + 3 * prob_ratios.std_dev() );
   prob_ratios.writeprobdists(false, true);

   delete X_ptr;
   delete Xreduced_ptr;

   outputfunc::print_elapsed_time();
}
