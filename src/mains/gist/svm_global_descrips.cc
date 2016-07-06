// ==========================================================================
// SVM_GLOBAL_DESCRIPS first imports matching and non-matching labels
// for pairs of images generated via program
// GENERATE_BOW_COLOR_SCORES.  It then uses Davis King's DLIB library
// to load the n_HOG_bins+n_color_bins feature vectors along
// with their positive and negative training labels.  After ordering
// of the input samples is randomized, cross-correlation is performed
// in order to estimate a reasonable value for a slack variable C.
// DLIB then trains binary decision and probabilistic decision
// functions. Serialized versions of the binary and probabilistic
// decision functions are exported to output binary files in
// learned_funcs_subdir.

// 		            ./svm_global_descrips

// ==========================================================================
// Last updated on 1/4/14; 1/5/14; 1/9/14
// ==========================================================================

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <flann/flann.hpp>
#include <flann/io/hdf5.h>
#include "dlib/svm.h"

#include "astro_geo/Clock.h"
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "numerical/param_range.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ios;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   string BoW_matches_subdir="./BoW_matches/";
   string matching_images_filename=BoW_matches_subdir+"matching_images.dat";
   string nonmatching_images_filename=
      BoW_matches_subdir+"nonmatching_images.dat";

// Import basic HOG BoW processing parameters:

   string gist_subdir="../gist/";
   string BoW_params_filename=gist_subdir+"BoW_params.dat";
   filefunc::ReadInfile(BoW_params_filename);
   int n_HOG_bins=stringfunc::string_to_number(filefunc::text_line[0]);
   cout << "n_HOG_bins = " << n_HOG_bins << endl;

   bool video_frames_input_flag=
      stringfunc::string_to_boolean(filefunc::text_line[8]);
   cout << "video_frames_input_flag = " << video_frames_input_flag << endl;

// The svm functions use column vectors to contain a lot of the
// data on which they operate. So the first thing we do here is
// declare a convenient typedef.

// This typedef declares a matrix with K rows and 1 column.  It will
// be the object that contains each of our K dimensional samples. 

//   const int K=3;
//   const int K=4;
//   const int K=5;
//   const int K=6;
//   const int K=7;
//   const int K=11;
//   const int K=1024;
//   const int K=2048;
//   const int K_color=33;

   const int K=2048+33;

   typedef dlib::matrix<double, 0, 1> sample_type;
   std::vector<sample_type> samples;
   std::vector<double> labels;

   cout << "Max number of features = " << K << endl;
   int K_process=K;
//   cout << "Enter number of features to process:" << endl;
//   cin >> K_process;

   typedef map<int,bool> IGNORED_FEATURES_MAP;
// independent int = ID of feature to ignore
// dependent bool = dummy var
   
   IGNORED_FEATURES_MAP ignored_features_map;
   IGNORED_FEATURES_MAP::iterator ignored_features_iter;

   for (int f=0; f<K-K_process; f++)
   {
      int feature_ID;
      cout << "Enter ID of feature to ignore:" << endl;
      cin >> feature_ID;
      ignored_features_map[feature_ID]=true;
   }
   sample_type samp(K);

   vector<vector<double> > matching_row_numbers=filefunc::ReadInRowNumbers(
      matching_images_filename);
   vector<vector<double> > nonmatching_row_numbers=filefunc::ReadInRowNumbers(
      nonmatching_images_filename);
   int Nmatches=matching_row_numbers.size();
   int Nnonmatches=nonmatching_row_numbers.size();
   cout << "Nmatches = " << Nmatches
        << " Nnonmatches = " << Nnonmatches << endl;

   samples.reserve(Nmatches+Nnonmatches);
   labels.reserve(Nmatches+Nnonmatches);

// Load matching image global descriptors into samples and labels objects:

   for (int r=0; r<Nmatches; r++)
   {
      for (int c=0; c<K; c++)
      {
         ignored_features_iter=ignored_features_map.find(c);
         if (ignored_features_iter==ignored_features_map.end())
         {
            samp(c)=matching_row_numbers[r].at(c);
         }
      }
      samples.push_back(samp);
      labels.push_back(1);
   } // loop over index r labeling rows within input file
   cout << "Matching samples.size() = " << samples.size() << endl;

// Load nonmatching image global descriptors into samples and labels objects:

   for (int r=0; r<Nnonmatches; r++)
   {
      for (int c=0; c<K; c++)
      {
         ignored_features_iter=ignored_features_map.find(c);
         if (ignored_features_iter==ignored_features_map.end())
         {
            samp(c)=nonmatching_row_numbers[r].at(c);
         }
      }
      samples.push_back(samp);
      labels.push_back(-1);
   } // loop over index r labeling rows within input file
   cout << "Matching+nonmatching samples.size() = " << samples.size() << endl;

// Define N as geometric mean of Nmatches and Nnonmatches:

   int N=sqrt(Nmatches*Nnonmatches);

   double positive_to_negative_data_ratio=
      double(Nmatches)/double(Nnonmatches);
   cout << "Positive-to-negative data ratio = " 
        << positive_to_negative_data_ratio << endl;

// This is a typedef for the type of kernel we are going to use in
// this example.  In this case I have selected the linear kernel that
// can operate on our K-dim sample_type objects

//   bool linear_kernel_flag=true;
   bool linear_kernel_flag=false;
   
// LINEAR KERNEL
//   typedef dlib::linear_kernel<sample_type> kernel_type;

// HISTOGRAM INTERSECTION KERNEL (for BoW)
   typedef dlib::histogram_intersection_kernel<sample_type> kernel_type;

// GAUSSIAN KERNEL
//   typedef dlib::radial_basis_kernel<sample_type> kernel_type;


// On 10/11/12, we asked Davis for how to eventually work with much
// larger numbers of training samples than O(10**5).  He said that we
// basically must move towards working with sparse vectors.  Doing so
// will free up RAM and thereby increase the number of training
// samples we can input.  See svm_sparse_ex.cpp for the syntax for
// Davis' sparse vectors.  Also, the previous typedef will become

// 	typedef sparse_linear_kernel<sample_type> kernel_type

// [Davis also stressed that dotproducting input samples with
// dictionaries must be followed by some nonlinear threshold.  For
// Coates-Ng classification, the scalar nonlinear function is
// z=max{0,|Dx|-alpha} where alpha=0.5 . If no such nonlinear
// thresholding takes place, then linear classification with 9*N
// dictionary elements is mathematically equivalent to working with
// the initial image patch and not bothering with any dictionary
// dotproducting.]

// Here we normalize all the samples by subtracting their mean and
// dividing by their standard deviation. This is generally a good idea
// since it often heads off numerical stability problems and also
// prevents one large feature from smothering others.  Doing this
// doesn't matter much in this example so I'm just doing this here so
// you can see an easy way to accomplish this with the library.

// On 10/24/13, Davis taught us that we can use a
// vector_normalizer_pca in place of a vector_normalizer.  If so, we
// to call normalizer.train(samples,1) below rather than
// normalizer.train(samples).  When we're training a linear classifier
// and working with relatively small numbers of feature dimensions,
// using a vector_normalizer_pca does not appear to yield noticeable
// improvement compared to vector_normalizer:

   typedef dlib::vector_normalizer<sample_type> normalizer_type;
//   typedef dlib::vector_normalizer_pca<sample_type> normalizer_type;
   normalizer_type normalizer;

// let the normalizer learn the mean and standard deviation of the samples

   normalizer.train(samples);
//   normalizer.train(samples,1);	// pca normalizer

// Now normalize each sample so that it has zero mean and unit
// standard deviation:

   for (unsigned long i = 0; i < samples.size(); ++i)
   {
      samples[i] = normalizer(samples[i]); 
   }
   
// Now that we have some data we want to train on it.  However, there
// are two parameters to the training.  These are the nu and gamma
// parameters.  Our choice for these parameters will influence how
// good the resulting decision function is.  To test how good a
// particular choice of these parameters is we can use the
// cross_validate_trainer() function to perform n-fold cross
// validation on our training data.  However, there is a problem with
// the way we have sampled our distribution above.  The problem is
// that there is a definite ordering to the samples.  That is, the
// first half of the samples look like they are from a different
// distribution than the second half.  This would screw up the cross
// validation process but we can fix it by randomizing the order of
// the samples with the following function call.

   int n_randomize=1;
   cout << endl;
   cout << "Enter number of times to randomize samples:" << endl;
   cin >> n_randomize;
   
   for (int r=0; r<n_randomize; r++)
   {
      dlib::randomize_samples(samples, labels);
   }

// Here we make an instance of the svm_c_linear_trainer object that uses our
// kernel type:

// LINEAR KERNEL

//   dlib::svm_c_linear_trainer<kernel_type> trainer;

// HISTOGRAM INTERSECTION KERNEL (for BoW)

   dlib::svm_c_trainer<kernel_type> trainer;

/*
// GAUSSIAN KERNEL ( = Radial basis kernel):

   dlib::svm_c_trainer<kernel_type> trainer;

// Gamma should be comparable to spacing between feature vectors.  But
// recall that we've normalized all features to have zero norm and
// unit variance:

// Gamma = 1/sigma**2 of gaussian balls

//  double gamma=0.01;
  double gamma=0.1;
//  double gamma=1;
  cout << "Enter gamma:" << endl;
  cin >> gamma;
  trainer.set_kernel(kernel_type(gamma));

// dlib::svm_c_ekm_trainer<kernel_type> trainer; 

// for lots of data,approx, fast method
*/

   string input_char;
   cout << "Enter 'c' to perform cross validation:" << endl;
   cin >> input_char;
   
   timefunc::initialize_timeofday_clock();  

   if (input_char=="c")
   {

// CROSS VALIDATION STARTS HERE:

      double min_C,max_C;
      if (video_frames_input_flag)
      {
         min_C=6E-7;	// JAV video clips
         max_C=8E-1;	// JAV video clips
      }
      else
      {
         min_C=6E-7;	// Tidmarsh
         max_C=8E-1;	// Tidmarsh
      }
      
      param_range log_C_param(log(min_C),log(max_C),5);

      double max_product_correct=-1;
      double best_product_pos_correct,best_product_neg_correct;
      double max_sum_correct=-1;
      double best_sum_pos_correct,best_sum_neg_correct;
      double best_product_C=0,best_sum_C=0;
      
// Vary over C and not C/N when working with radial basis functions:

// On 10/15/13, Davis taught us that increasing C should generally
// yield better classification results for *training* samples.  But as
// C is increased, overfitting increasingly occurs.  So cross
// validation performance generally increases with C up to some
// maximum and then decreases as C is further increased.

// Now we loop over some different C values to see how good
// they are.  Note that this is a very simple way to try out a few
// possible parameter choices.  You should look at the
// model_selection_ex.cpp program for examples of more sophisticated
// strategies for determining good parameter choices.

      int n_iterations=4;
      for (int iteration=0; iteration<n_iterations; iteration++)
      {
         cout << "************************************************" << endl;
         cout << "C parameter iteration = " << iteration << endl << endl;
         cout << "************************************************" << endl;

// ========================================================================
// Begin while loop over log_C_param
// ========================================================================

         while (log_C_param.prepare_next_value())
         {
            cout << "Processing bin " << log_C_param.get_counter()
                 << " of " << log_C_param.get_nbins() << endl;
            outputfunc::print_elapsed_time();
            double logC=log_C_param.get_value();
            double C=exp(logC);
//            cout << "C = " << C << endl;

// Rule of thumb: If # class2/# class1 = alpha, then c_class1 = alpha
// c_class2 
            
            double C_positive=C;
            double C_negative=C*positive_to_negative_data_ratio;
            trainer.set_c_class1(C_positive);
            trainer.set_c_class2(C_negative);
//         cout << "C_positive/N = " << C_positive/double(N)
//              << " C_negative/N = " << C_negative/double(N) << endl;
            cout << "C_positive = " << C_positive
                 << " C_negative = " << C_negative << endl;

// Print out the cross validation accuracy for 3-fold cross validation
// using the current C coeffient. cross_validate_trainer() returns a
// row vector.  The first element of the vector is the fraction of +1
// training examples correctly classified and the second number is the
// fraction of -1 training examples correctly classified.

            dlib::matrix<double,1,2> pos_neg_results=
               cross_validate_trainer(trainer,samples,labels,3);
            double pos_correct=pos_neg_results(0,0);
            double neg_correct=pos_neg_results(0,1);
         
            double product_correct=pos_correct*neg_correct;
            if (product_correct >=0.99999*max_product_correct)
            {
               max_product_correct=product_correct;
               best_product_pos_correct=pos_correct;
               best_product_neg_correct=neg_correct;
               best_product_C=C;
               log_C_param.set_best_value();
            }
         
            double sum_correct=pos_correct+neg_correct;
            if (sum_correct >= 0.99999*max_sum_correct)
            {
               max_sum_correct=sum_correct;
               best_sum_pos_correct=pos_correct;
               best_sum_neg_correct=neg_correct;
               best_sum_C = C;
            }

            cout << "  Cross validation accuracy: " 
                 << cross_validate_trainer(trainer, samples, labels, 3)
                 << endl;
            cout << "  Product_correct = " << product_correct
                 << " sum_correct = " << sum_correct << endl << endl;

         } // while loop over log_C_param
         double frac=0.3;
         log_C_param.shrink_search_interval(
            log_C_param.get_best_value(),frac);
      } // loop over C parameter iterations

      cout << "max_product_correct = " << max_product_correct << endl;
      cout << "best_product_C = " << best_product_C << endl << endl;

      cout << "Nmatches = " << Nmatches
           << " Nnonmatches = " << Nnonmatches
           << " N = " << N << endl;
      cout << "n_randomize = " << n_randomize 
           << "; C_positive = " << best_product_C
           << " C_negative = " 
           << best_product_C*positive_to_negative_data_ratio << endl;
      cout << "Cross validation accuracy: " << best_product_pos_correct 
           << " " << best_product_neg_correct << endl;
      cout << endl;

      cout << "max_sum_correct = " << max_sum_correct << endl;
      cout << "best_sum_C = " << best_sum_C << endl << endl;

      cout << "Nmatches = " << Nmatches
           << " Nnonmatches = " << Nnonmatches
           << " N = " << N << endl;
      cout << "n_randomize = " << n_randomize 
           << "; C_positive = " << best_sum_C
           << " C_negative = " 
           << best_sum_C*positive_to_negative_data_ratio << endl;
      cout << "Cross validation accuracy: " << best_sum_pos_correct 
           << " " << best_sum_neg_correct << endl;

      exit(-1);

// .................
// Fri, Dec 27, 2013 at 3 pm, square-root dotproduct 
// Histogram intersection kernel
// Nmatches = 1001 Nnonmatches = 1784 N = 1336; n_HOG_words=2048

// n_randomize=1; C_positive = 0.00153286 C_negative = 0.000860086
// Cross validation accuracy: 0.833834 0.929854

// n_randomize=3; C_positive = 0.00298711 C_negative = 0.00167606
// Cross validation accuracy: 0.846847 0.939394

// n_randomize=5; C_positive = 0.00185476 C_negative = 0.0010407
// Cross validation accuracy: 0.851852 0.928732

// .................
// Fri, Dec 27, 2013 at 11 pm, diff_over_sum
// Histogram intersection kernel
// Nmatches = 1001 Nnonmatches = 1784 N = 1336; n_HOG_words=2048

// n_randomize=1; C_positive = 0.000590984 C_negative = 0.0003316
// Cross validation accuracy: 0.774775 0.799102

// n_randomize=3;  C_positive = 0.000650082 C_negative = 0.00036476
// Cross validation accuracy: 0.75976 0.795174

// n_randomize=5;  C_positive = 0.000303268 C_negative = 0.000170163
// Cross validation accuracy: 0.781782 0.768238

// .................
// Sat, Dec 28, 2013 at 2:15 pm, square-root bin matching
// Histogram intersection kernel
// Nmatches = 1001 Nnonmatches = 1784 N = 1336; n_HOG_words=2048

// n_randomize=1; C_positive = 0.00153286 C_negative = 0.000860086
// Cross validation accuracy: 0.833834 0.929854

// n_randomize=3; C_positive = 0.00298711 C_negative = 0.00167606
// Cross validation accuracy: 0.846847 0.939394

// n_randomize=5; C_positive = 0.00185476 C_negative = 0.0010407
// Cross validation accuracy: 0.851852 0.928732

// .................
// Sat, Dec 28, 2013 at 7:20 pm, diff_over_sum w/ 0.7 comparison threshold
// Histogram intersection kernel
// Nmatches = 1001 Nnonmatches = 1784 N = 1336; n_HOG_words=2048

// n_randomize=1; C_positive = 0.000424744 C_negative = 0.000238323
// Cross validation accuracy: 0.851852 0.770483

// n_randomize=3; C_positive = 0.000830316 C_negative = 0.000465889
// Cross validation accuracy: 0.820821 0.806958

// n_randomize=5; C_positive = 0.000491898 C_negative = 0.000276003
// Cross validation accuracy: 0.826827 0.772727

// .................
// Sun, Dec 29, 2013 at 7 am, diff_over_sum 
// Histogram intersection kernel
// Nmatches = 1001 Nnonmatches = 1784 N = 1336; n_HOG_words=2048

// n_randomize = 1; C_positive = 0.000748322 C_negative = 0.000419882
// Cross validation accuracy: 0.834835 0.819304

// n_randomize = 3; C_positive = 0.000936061 C_negative = 0.000525223
// Cross validation accuracy: 0.814815 0.851291

// n_randomize = 5; C_positive = 0.000863461 C_negative = 0.000484487
// Cross validation accuracy: 0.810811 0.838945

// .................
// Sun, Dec 29, 2013 at 1:15 pm, sqrt_diff_over_sum 
// Histogram intersection kernel
// Nmatches = 1001 Nnonmatches = 1943 N = 1394; n_HOG_words=2048

// n_randomize = 1; C_positive = 0.000724011 C_negative = 0.000372998
// Cross validation accuracy: 0.811812 0.757857

// n_randomize = 3; C_positive = 0.00048001 C_negative = 0.000247293
// Cross validation accuracy: 0.830831 0.716641

// n_randomize = 5; C_positive = 0.000863461 C_negative = 0.00044484
// Cross validation accuracy: 0.796797 0.774858

// .................
// Fri, Jan 3, 2014 at 9 pm, sqrt_diff_over_sum HOG + DoS color
// Histogram intersection kernel
// n_HOG_bins=2048; n_color_bins=33
// Nmatches = 1239 Nnonmatches = 2257 N = 1672

// n_randomize = 1; C_positive = 0.000802359 C_negative = 0.000440462
// Cross validation accuracy: 0.803067 0.878103

// n_randomize = 3; C_positive = 0.00131841 C_negative = 0.000723752
// Cross validation accuracy: 0.792575 0.894504

// n_randomize = 5; C_positive = 0.0009569 C_negative = 0.000525299
// Cross validation accuracy: 0.799031 0.87766

// .................
// Sat, Jan 4, 2014 at 8:30 am, sqrt_diff_over_sum HOG + DoS color
// Histogram intersection kernel
// n_HOG_bins=2048; n_color_bins=33
// Nmatches = 1001 Nnonmatches = 2522 N = 1588

// n_randomize = 1; C_positive = 0.000662974 C_negative = 0.000263139
// Cross validation accuracy: 0.702703 0.837302

// n_randomize = 3; C_positive = 0.000464416 C_negative = 0.00018433
// Cross validation accuracy: 0.712713 0.813889

// n_randomize = 5; C_positive = 0.00048001 C_negative = 0.000190519
// Cross validation accuracy: 0.695696 0.824603

// .................
// Sat, Jan 4, 2014 at 9:30 am, sqrt_diff_over_sum HOG + DoS color
// Histogram intersection kernel
// n_HOG_bins=2048; n_color_bins=33
// Nmatches = 590 Nnonmatches = 1561 N = 959
// Just early_Sep + w_transcripts News clips !

// n_randomize = 1; C_positive = 0.000865144 C_negative = 0.000326992
// Cross validation accuracy: 0.77381 0.895513

// n_randomize = 3; C_positive = 0.000865144 C_negative = 0.000326992
// Cross validation accuracy: 0.787415 0.894872

// n_randomize = 5; C_positive = 0.00095155 C_negative = 0.000359651
// Cross validation accuracy: 0.780612 0.902564

// .................
// Sat, Jan 4, 2014 at 10:30 am, sqrt_diff_over_sum HOG + DoS color
// Histogram intersection kernel
// n_HOG_bins=2048; n_color_bins=33
// Nmatches = 411 Nnonmatches = 961 N = 628
// Just BB clips !

// n_randomize = 1; C_positive = 0.00069282 C_negative = 0.000296305
// Cross validation accuracy: 0.63017 0.89375

// n_randomize = 3; C_positive = 0.00104659 C_negative = 0.000447604
// Cross validation accuracy: 0.635036 0.890625

// n_randomize = 5; C_positive = 0.0013069 C_negative = 0.000558935
// Cross validation accuracy: 0.613139 0.902083

// .................
// Sat, Jan 4, 2014 at 1 pm, sqrt_diff_over_sum HOG + DoS color
// Histogram intersection kernel
// n_HOG_bins=2048; n_color_bins=33
// Nmatches = 602 Nnonmatches = 1010 N = 779
// Just BB clips !

// n_randomize = 1; C_positive = 0.000838122 C_negative = 0.000499554
// Cross validation accuracy: 0.766667 0.859127

// n_randomize = 3; C_positive = 0.000865144 C_negative = 0.00051566
// Cross validation accuracy: 0.768333 0.859127

// n_randomize = 5; C_positive = 0.00069282 C_negative = 0.000412948
// Cross validation accuracy: 0.763333 0.852183

// .................
// Sat, Jan 4, 2014 at 3:30 pm, sqrt_diff_over_sum HOG + DoS color
// Histogram intersection kernel
// n_HOG_bins=2048; n_color_bins=33

// Nmatches = 1192 Nnonmatches = 2571 N = 1750
// early_Sep,wtrans,BB clips

// n_randomize = 1; C_positive = 0.00050444 C_negative = 0.000233875
// Cross validation accuracy: 0.742233 0.809413

// n_randomize = 3; C_positive = 0.000554821 C_negative = 0.000257233
// Cross validation accuracy: 0.736356 0.812524

// n_randomize = 5; C_positive = 0.000762016 C_negative = 0.000353296
// Cross validation accuracy: 0.742233 0.833528


// ***********************************************************************
// Tidmarsh:

// Nmatches = 225 Nnonmatches = 133 N = 172
// C_positive/N = 0.335544 C_negative/N = 0.56765
//   Cross validation accuracy: 0.551111 0.840909 

// Nmatches = 225 Nnonmatches = 161 N = 190
// C_positive/N = 0.0838861 C_negative/N = 0.117232
//  Cross validation accuracy: 0.564444 0.842767 

// 7 features (global color, gist, texture, ellipse_area, ellipse_density,
// ellipse_separation, LBP)

// Nmatches = 666 Nnonmatches = 26 N = 131
// C_positive/N = 0.0218416 C_negative/N = 0.559482
// Cross validation accuracy: 0.63964 0.958333

// Nmatches = 383 Nnonmatches = 147 N = 237
// C_positive/N = 245.123 C_negative/N = 638.654
// Cross validation accuracy: 0.905512 0.972789

// Nmatches = 466 Nnonmatches = 265 N = 351
// C_positive/N = 6.37622 C_negative/N = 11.2125
// Cross validation accuracy: 0.827957 0.92803

// ......................
// Tues Dec 31, 2013 at 10:48 am; sqrt_diff_over_sum
// Histogram intersection kernel
// Nmatches = 738 Nnonmatches = 371 N = 523; n_HOG_words=2048

// n_randomize = 1; C_positive = 0.000366757 C_negative = 0.00072956
// Cross validation accuracy: 0.852304 0.859079

// n_randomize = 3; C_positive = 0.000424744 C_negative = 0.000844908
// Cross validation accuracy: 0.861789 0.842818

// n_randomize = 5; C_positive = 0.000411448 C_negative = 0.000818459
// Cross validation accuracy: 0.860434 0.856369

// ......................
// Weds, Jan 1, 2014 at 12:15 am; sqrt_diff_over_sum
// Histogram intersection kernel
// Nmatches = 756 Nnonmatches = 652 N = 702; n_HOG_words=2048

// n_randomize = 1; C_positive = 0.0221891 C_negative = 0.0257285
// Cross validation accuracy: 0.792328 0.81106

// n_randomize = 3; C_positive = 0.0221891 C_negative = 0.0257285
// Cross validation accuracy: 0.794974 0.835637

// n_randomize = 5; C_positive = 0.0221891 C_negative = 0.0257285
// Cross validation accuracy: 0.784392 0.834101

// ......................
// Weds, Jan 1, 2014 at 9 am; sqrt_diff_over_sum
// Histogram intersection kernel
// Nmatches = 805 Nnonmatches = 970 N = 883; n_HOG_words=2048

// n_randomize = 1; C_positive = 5.35728e-05 C_negative = 4.44599e-05
// Cross validation accuracy: 0.574627 0.95872

// n_randomize = 3; C_positive = 0.000114734 C_negative = 9.52171e-05
// Cross validation accuracy: 0.553483 0.9742

// n_randomize = 5; C_positive = 7.35793e-05 C_negative = 6.10632e-05
// Cross validation accuracy: 0.569652 0.96388

// ......................
// Weds, Jan 1, 2014 at 11:30 am; sqrt_diff_over_sum
// Histogram intersection kernel
// Nmatches = 1102 Nnonmatches = 1081 N = 1091; n_HOG_words=2048

// n_randomize = 1; C_positive = 0.000367281 C_negative = 0.000374416
// Cross validation accuracy: 0.647593 0.887963

// n_randomize = 3; C_positive = 0.000554821 C_negative = 0.000565599
// Cross validation accuracy: 0.684832 0.837037

// n_randomize = 5; C_positive = 0.000240575 C_negative = 0.000245248
// Cross validation accuracy: 0.589464 0.923148

// ......................
// Weds, Jan 1, 2014 at 3 pm; sqrt_diff_over_sum
// Histogram intersection kernel
// Nmatches = 1352 Nnonmatches = 1176 N = 1260

// n_randomize = 1; C_positive = 0.000458634 C_negative = 0.000527273
// Cross validation accuracy: 0.712593 0.759354

// n_randomize = 3; C_positive = 0.000416987 C_negative = 0.000479394
// Cross validation accuracy: 0.691111 0.77551

// n_randomize = 5; C_positive = 0.00069282 C_negative = 0.000796508
// Cross validation accuracy: 0.720741 0.727891

// ......................
// Weds, Jan 1, 2014 at 6 pm; sqrt_diff_over_sum
// Histogram intersection kernel
// Nmatches = 1453 Nnonmatches = 1263 N = 1354

// n_randomize = 1; C_positive = 0.000403963 C_negative = 0.000464733
// Cross validation accuracy: 0.643251 0.780681

// n_randomize = 3; C_positive = 0.000367281 C_negative = 0.000422533
// Cross validation accuracy: 0.639807 0.809184

// n_randomize = 5; C_positive = 0.000367281 C_negative = 0.000422533
// Cross validation accuracy: 0.63292 0.806809

// ......................
// Weds, Jan 1, 2014 at 10 pm; sqrt_diff_over_sum HOG + DoS color
// Histogram intersection kernel
// Nmatches = 1453 Nnonmatches = 1263 N = 1354

// n_randomize = 1; C_positive = 0.00050444 C_negative = 0.000580326
// Cross validation accuracy: 0.672176 0.748219

// n_randomize = 3; C_positive = 0.000416987 C_negative = 0.000479717
// Cross validation accuracy: 0.661846 0.787807

// n_randomize = 5; C_positive = 0.000367281 C_negative = 0.000422533
// Cross validation accuracy: 0.630854 0.798892

// ......................
// Thurs, Jan 2, 2014 at 8:30 am; sqrt_diff_over_sum HOG + DoS color
// Histogram intersection kernel
// Nmatches = 1520 Nnonmatches = 1381 N = 1448;
// n_HOG_bins=2048; n_color_bins=33

// n_randomize = 1; C_positive = 0.000629908 C_negative = 0.00069331
// Cross validation accuracy: 0.652174 0.72971

// n_randomize = 1; C_positive = 8.35372e-05 C_negative = 9.19453e-05
// Cross validation accuracy: 0.513175 0.894928

// n_randomize = 3; C_positive = 0.000762016 C_negative = 0.000838714
// Cross validation accuracy: 0.675889 0.728986

// n_randomize = 5; C_positive = 0.000458634 C_negative = 0.000504796
// Cross validation accuracy: 0.63307 0.776087

// n_randomize = 5; C_positive = 8.35372e-05 C_negative = 9.19453e-05
// Cross validation accuracy: 0.507246 0.902899


// ***********************************************************************
// All video clips:

// Sun, Jan 5, 2014 at 7:30 pm; sqrt_diff_over_sum HOG + DoS color
// Histogram intersection kernel
// Nmatches = 2712 Nnonmatches = 3952 N = 3273
// n_HOG_bins=2048; n_color_bins=33

// n_randomize = 1; C_positive = 0.00104659 C_negative = 0.000718204
// Cross validation accuracy: 0.728982 0.754746

// n_randomize = 3; C_positive = 0.00095155 C_negative = 0.000652987
// Cross validation accuracy: 0.735988 0.752215

// n_randomize = 5; C_positive = 0.00115111 C_negative = 0.000789935
// Cross validation accuracy: 0.719764 0.765629




// CROSS VALIDATION ENDS HERE

      exit(-1);

   } // input_char=="c" conditional
   
//   double C=3634.64;			// JAVA w transcripts 11-1 features
//   double C=4.27323;			// JAVA w transcripts 11-1+1024 feature
//   double C=6.15344;			// JAVA w transcripts 11-1+1024 features
//   double C=0.000794968;		// JAVA wtrans 2048 features HIK
//   double C=0.000611591;		// JAVA wtrans 2048 features HIK
//    double C=0.000787979;		// early,wtrans,BB 2048 features HIK
//   double C=0.00104696;		// early,wtrans,BB 2048 features HIK
//    double C=0.00298711;		// early,wtrans,BB 2048 features HIK
//   double C=0.000590984;		// early,wtrans,BB 2048 features HIK
//    double C=0.00298711;	       	// early,wtrans,BB 2048 bins,HIK,sqrt
//   double C=0.000830316;		// early,wtrans,BB 2048 bins,HIK,DoS T
//   double C=0.000936061;		// early,wtrans,BB 2048 bins,HIK,DoS

//   double C=0.000863461;		// early,w/,BB 2048 bins,HIK sqrt DoS
//   double C=0.00131841;		// early,w/,BB 2048+33 bins,HIK,sqrtDoS+DoS
//   double  C=0.000662974;	// early,w/BB 2048+33 bins,HIK,sqrtDoS+DoS
//   double C=0.00095155;		// early, wtrans 2048+33 bins,HIK,sqrtDoS+DoS
//    double C=0.00104659;		// just BB 2048+33 bins,HIK,sqrtDoS+DoS
//   double C = 0.000838122;	// just BB 2048+33 bins,HIK,sqrtDoS+DoS
//   double C=0.000762016;	// early,w/BB 2048+33 bins,HIK,sqrtDoS+DoS


//   double C=0.000411448;		// Tidmarsh, 2048 bins,HIK sqrt DoS
//   double C=0.0221891;		// Tidmarsh, 2048 bins,HIK sqrt DoS
//   double C=0.000114734;		// Tidmarsh, 2048 bins,HIK sqrt DoS
//   double C=0.000367281;		// Tidmarsh, 2048 bins,HIK sqrt DoS
//   double C=0.000458634;		// Tidmarsh, 2048 bins,HIK sqrt DoS
//   double C=0.000367281;		// Tidmarsh, 2048 bins, HIK sqrt DoS

//   double C=0.000416987;	// Tidmarsh,2048+33 bins, HIK, sqrtDOS+DOS
//   double C=0.000458634;	// Tidmarsh,2048+33 bins,HIK,sqrtDOS+DOS
//   double C= 8.35372e-05;	// Tidmarsh,2048+33 bins,HIK,sqrtDOS+DOS


   double C=0.00104659;  // all clips, 2048+33 bins,HIK,sqrtDoS+DoS
      
   double C_positive=C;
   double C_negative=C*positive_to_negative_data_ratio;

   cout << "******************************************************" << endl;
   cout << "USING CROSS VALIDATION PARAMETERS C_POSITIVE = "
        << C_positive << " AND C_NEGATIVE = " << C_negative << endl;
   cout << "******************************************************" << endl;
   cout << endl;

//   trainer.set_c(C);
   trainer.set_c_class1(C_positive);
   trainer.set_c_class2(C_negative);

/*
  if (linear_kernel_flag)
  {

// Note; Davis warns that the following non-negative weight condition
// is a VERY strong prior condition.  He recommends that it be used
// when number of features significantly exceeds number of training
// samples (e.g. projection of image onto dictionary containing 1024
// words where we train with O(100) images) to avoid overfitting:

trainer.set_learns_nonnegative_weights(true);   // linear kernels only
//      trainer.be_verbose();			   // linear kernels only 
}
*/

// Now we train on the full set of data and obtain the resulting
// decision function.  The decision function will return values >= 0
// for samples it predicts are in the +1 class and numbers < 0 for
// samples it predicts to be in the -1 class.

   typedef dlib::decision_function<kernel_type> dec_funct_type;
   typedef dlib::normalized_function<dec_funct_type,normalizer_type> 
      funct_type;

// Here we are making an instance of the normalized_function object.
// This object provides a convenient way to store the vector
// normalization information along with the decision function we are
// going to learn.

   funct_type learned_function;
   learned_function.normalizer = normalizer;  // save normalization info

// Perform the actual SVM training and save the results.  Print out
// the number of support vectors in the resulting decision function:

   cout << endl;
   cout << "-----------------------------------------------------" << endl;
   cout << "Starting to train binary decision function:" << endl;

   cout << "samples.size() = " << samples.size()
        << " labels.size() = " << labels.size() << endl;

   learned_function.function = trainer.train(samples, labels); 

   outputfunc::print_elapsed_time();
   cout << "Number of support vectors in our learned_function =  " 
        << learned_function.function.basis_vectors.nr() << endl;
   cout << "Number of training samples: N = " << N << endl;

// Another thing that is worth knowing is that just about everything
// in dlib is serializable. So for example, you can save the
// learned_pfunct object to disk and recall it later like so:

   string learned_funcs_subdir=BoW_matches_subdir+"learned_functions/";
   filefunc::dircreate(learned_funcs_subdir);

   string output_filename=learned_funcs_subdir+
      "bifunc_"+stringfunc::number_to_string(Nmatches)+"_"+
      stringfunc::number_to_string(Nnonmatches)+".dat";
   ofstream fout(output_filename.c_str(),ios::binary);
   serialize(learned_function,fout);
   fout.close();

   string banner="Exported learned binary function to "+output_filename;
   outputfunc::write_banner(banner);

/*
  if (linear_kernel_flag)
  {

// Davis told us on Tues Oct 15, 2013 that the next line will give us
// coeffs in original feature space coordinate system for plane which
// separates positive and negative samples:

dec_funct_type my_dec_funct=
dlib::simplify_linear_decision_function(learned_function);
   
sample_type my_weights=my_dec_funct.basis_vectors(0);
double bias=my_dec_funct.b;
for (int w=0; w<my_weights.size(); w++)
{
cout << "w = " << w << " feature space weight = " << my_weights(w)
<< endl;
}
cout << "bias = " << bias << endl;
outputfunc::enter_continue_char();
}
*/

    Clock clock;
    clock.set_time_based_on_local_computer_clock();
    cout << "Current time: " << clock.YYYY_MM_DD_H_M_S() << endl;

// ***********************************************************************
// PROBABILISTIC DECISION FUNCTION TRAINING AND EXPORT
// ***********************************************************************

// We can also train a decision function that reports a well
// conditioned probability instead of just a number > 0 for the +1
// class and < 0 for the -1 class.  An example of doing that follows:

    typedef dlib::probabilistic_decision_function<kernel_type> 
       probabilistic_funct_type;  
    typedef dlib::normalized_function<probabilistic_funct_type,normalizer_type>
       pfunct_type;

    pfunct_type learned_pfunct; 
    learned_pfunct.normalizer = normalizer;

    cout << endl;
    cout << "-----------------------------------------------------" << endl;
    cout << "Starting to train probabilistic decision function:" << endl;

    learned_pfunct.function = dlib::train_probabilistic_decision_function(
       trainer, samples, labels, 3);
   outputfunc::print_elapsed_time();

// Now we have a function that returns the probability that a given
// sample is of the +1 class.

// Print out the number of support vectors in the resulting decision
// function. (it should be the same as in the one above)

    cout << "Number of support vectors in our learned_pfunct =  " 
         << learned_pfunct.function.decision_funct.basis_vectors.nr() 
         << endl;

// Davis told us on Weds Oct 9, 2013 that we should be able to recover
// the learned weights via the vector
// learned_pfunct.function.decision_funct.basis_vectors(0)

    sample_type fitted_weights;
    fitted_weights=learned_pfunct.function.decision_funct.basis_vectors(0);
    for (int w=0; w<fitted_weights.size(); w++)
    {
       cout << "w = " << w 
            << " fitted probabilistic decision function weight = " 
            << fitted_weights(w)
            << endl;
    }

    cout << "Number of training samples: N = " << N << endl;

// Another thing that is worth knowing is that just about everything
// in dlib is serializable. So for example, you can save the
// learned_pfunct object to disk and recall it later like so:

    output_filename=learned_funcs_subdir
       +"pfunct_"+stringfunc::number_to_string(Nmatches)+"_"
       +stringfunc::number_to_string(Nnonmatches)+".dat";
    ofstream fout2(output_filename.c_str(),ios::binary);
    dlib::serialize(learned_pfunct,fout2);
    fout2.close();

// Generate soft link between output_filename and
// learned_funcs_subdir/pfunct.dat:

    string unix_cmd="/bin/rm "+learned_funcs_subdir+"pfunct.dat";
    sysfunc::unix_command(unix_cmd);
    unix_cmd="ln -s "+output_filename+" "+learned_funcs_subdir+"pfunct.dat";
    sysfunc::unix_command(unix_cmd);

    banner="Exported learned probabilistic function to "+output_filename;
    outputfunc::write_banner(banner);

    clock.set_time_based_on_local_computer_clock();
    cout << "Current time: " << clock.YYYY_MM_DD_H_M_S() << endl;
    cout << "Binary decision function = "
         << dlib::test_binary_decision_function(
            learned_function.function,samples,labels)
         << endl;

    cout << endl;
    cout << "At end of program SVM_GLOBAL_DESCRIPS" << endl;
    outputfunc::print_elapsed_time();
}

