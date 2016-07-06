// ==========================================================================
// SVM_COLORS reads in three-dimensional RGB features for legible and
// unreadable character image chips.  It then uses Davis King's DLIB
// library to load the input samples along with their positive and
// negative labels.  After the ordering of the input samples is
// randomized, cross-correlation is performed in order to estimate a
// reasonable value for a linear-SVM slack variable C.  DLIB then
// trains binary decision and probabilistic decision
// functions. Serialized versions of the binary and probabilistic
// decision functions are exported to output binary files.

// 				svm_colors

// ==========================================================================
// Last updated on 1/27/16
// ==========================================================================


#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <flann/flann.hpp>
#include <flann/io/hdf5.h>
#include "dlib/svm.h"

#include "astro_geo/Clock.h"
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
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

using namespace dlib;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   Clock clock;
   clock.set_time_based_on_local_computer_clock();
   cout << endl;
   cout << "Starting time: " << clock.YYYY_MM_DD_H_M_S() << endl << endl;

// The svm functions use column vectors to contain a lot of the
// data on which they operate. So the first thing we do here is
// declare a convenient typedef.

// This typedef declares a matrix with K rows and 1 column.  It will
// be the object that contains each of our K dimensional samples. 

   const int K=3;
   typedef matrix<double, K, 1> sample_type;
   sample_type samp;

// This is a typedef for the type of kernel we are going to use in
// this example.  In this case I have selected the linear kernel that
// can operate on our K-dim sample_type objects

    typedef linear_kernel<sample_type> kernel_type;

// Now we make objects to contain our samples and their respective
// labels.

   std::vector<sample_type> samples;
   std::vector<double> labels;
   samples.reserve(5000);
   labels.reserve(5000);

// Import legible and unreadable descriptors:

   int Nlegible = 0;
   int Nunreadable = 0;

   string input_filename="legible_vs_unreadable.txt";
   filefunc::ReadInfile(input_filename);

// Load legible data into samples and labels objects:

   for(unsigned int i = 0; i < filefunc::text_line.size(); i++)
   {
      std::vector<double> column_values = stringfunc::string_to_numbers(
         filefunc::text_line[i]); 

      double curr_P = column_values[3];

      samp(0) = column_values[0];
      samp(1) = column_values[1];
      samp(2) = column_values[2];
      samples.push_back(samp);

      if(curr_P > 0.5)
      {
         labels.push_back(1);
         Nlegible++;
      }
      else
      {
         labels.push_back(-1);
         Nunreadable++;
      }
   }
   
// Define N as geometric mean of Nlegible and Nunreadable:

   cout << "Nlegible = " << Nlegible 
        << " Nunreadable = " << Nunreadable << endl;
   int N=sqrt(Nlegible*Nunreadable);
   cout << "N = sqrt(Nlegible*Nunreadable) = " << N << endl;
   cout << "sqrt(Nlegible)*sqrt(Nunreadable) = "
        << sqrt(Nlegible)*sqrt(Nunreadable) << endl;

   double positive_to_negative_data_ratio=
      double(Nlegible)/double(Nunreadable);
   cout << "Positive-to-negative data ratio = " 
        << positive_to_negative_data_ratio << endl;

// Here we normalize all the samples by subtracting their mean and
// dividing by their standard deviation. This is generally a good idea
// since it often heads off numerical stability problems and also
// prevents one large feature from smothering others.  Doing this
// doesn't matter much in this example so I'm just doing this here so
// you can see an easy way to accomplish this with the library.

   vector_normalizer<sample_type> normalizer;
   // let the normalizer learn the mean and standard deviation of the samples
   normalizer.train(samples);
   // now normalize each sample

   cout << "Normalizing samples:" << endl;
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

   cout << "Before randomizing samples" << endl;
   randomize_samples(samples, labels);

// Here we make an instance of the svm_c_linear_trainer object that uses our
// kernel type:

   svm_c_linear_trainer<kernel_type> trainer;
//   svm_c_ekm_trainer<kernel_type> trainer;

/*
// CROSS VALIDATION STARTS HERE:

   double min_C=0.05*N;
   double max_C=4096*N;

// Now we loop over some different C values to see how good
// they are.  Note that this is a very simple way to try out a few
// possible parameter choices.  You should look at the
// model_selection_ex.cpp program for examples of more sophisticated
// strategies for determining good parameter choices.

   double max_product_correct=-1;
   double best_product_pos_correct,best_product_neg_correct;
   double max_sum_correct=-1;
   double best_sum_pos_correct,best_sum_neg_correct;
   double best_product_C,best_sum_C;

   cout << "Performing cross validation" << endl;
   for (double C = min_C; C < max_C; C *= 2)
   {
      // tell the trainer the parameters we want to use

// If # class2/# class1 = alpha, then c_class1 = alpha c_class2 (rule
// of thumb)

      double C_positive=C;
      double C_negative=C*positive_to_negative_data_ratio;
      trainer.set_c_class1(C_positive);
      trainer.set_c_class2(C_negative);
      cout << "C_positive = " << C_positive/double(N)
           << " C_negative = " << C_negative/double(N) << endl;

// Print out the cross validation accuracy for 3-fold cross validation
// using the current C coeffient. cross_validate_trainer() returns a
// row vector.  The first element of the vector is the fraction of +1
// training examples correctly classified and the second number is the
// fraction of -1 training examples correctly classified.

      matrix<double,1,2> pos_neg_results=
         cross_validate_trainer(trainer,samples,labels,3);
      double pos_correct=pos_neg_results(0,0);
      double neg_correct=pos_neg_results(0,1);
         
      double product_correct=pos_correct*neg_correct;
      if (product_correct > max_product_correct)
      {
         max_product_correct=product_correct;
         best_product_pos_correct=pos_correct;
         best_product_neg_correct=neg_correct;
         best_product_C=C;
      }
         
      double sum_correct=pos_correct+neg_correct;
      if (sum_correct > max_sum_correct)
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
   }

   cout << "max_product_correct = " << max_product_correct << endl;
   cout << "best_product_pos_correct = " << best_product_pos_correct << endl;
   cout << "best_product_neg_correct = " << best_product_neg_correct << endl;
   cout << "best_product_C = " << best_product_C << endl;
   cout << "best_product_C/N = " << best_product_C/N << endl;
   cout << endl;

   cout << "max_sum_correct = " << max_sum_correct << endl;
   cout << "best_sum_pos_correct = " << best_sum_pos_correct << endl;
   cout << "best_sum_neg_correct = " << best_sum_neg_correct << endl;
   cout << "best_sum_C = " << best_sum_C << endl;
   cout << "best_sum_C/N = " << best_sum_C/N << endl;

   exit(-1);



// After performing cross-validation on positive and negative samples
// on 1/27/16, we found C/N=409.6 as the best estimate for the linear
// SVM slack variable:

// C_positive = 409.6 C_negative = 3861.5
//   Cross validation accuracy: 0.791565 0.899225 

// CROSS VALIDATION ENDS HERE
*/

   double C=409.6*N;
   trainer.set_c(C);
//    trainer.be_verbose();

// Now we train on the full set of data and obtain the resulting
// decision function.  The decision function will return values >= 0
// for samples it predicts are in the +1 class and numbers < 0 for
// samples it predicts to be in the -1 class.

   typedef decision_function<kernel_type> dec_funct_type;
   typedef normalized_function<dec_funct_type> funct_type;

// Here we are making an instance of the normalized_function object.
// This object provides a convenient way to store the vector
// normalization information along with the decision function we are
// going to learn.

   funct_type learned_function;
   learned_function.normalizer = normalizer;  // save normalization 
					      //   information

// Perform the actual SVM training and save the results.  Print out
// the number of support vectors in the resulting decision function:

   timefunc::initialize_timeofday_clock();
   cout << endl;
   cout << "-----------------------------------------------------" << endl;
   cout << "Starting to train binary decision function:" << endl;
   learned_function.function = trainer.train(samples, labels); 
   cout << "Elapsed time = " 
        << timefunc::elapsed_timeofday_time()/60 << " mins = " 
        << timefunc::elapsed_timeofday_time()/3600 << " hours" << endl;
   cout << "Number of support vectors in our learned_function =  " 
        << learned_function.function.basis_vectors.nr() << endl;
   cout << "Number of training samples: N = " << N << endl;

// Another thing that is worth knowing is that just about everything
// in dlib is serializable. So for example, you can save the
// learned_pfunct object to disk and recall it later like so:

   string learned_funcs_subdir="./learned_functions/";
   filefunc::dircreate(learned_funcs_subdir);

   string output_filename=learned_funcs_subdir+
      "colors_bifunc_"+stringfunc::number_to_string(Nlegible)+"_"+
      stringfunc::number_to_string(Nunreadable)+".dat";
   ofstream fout(output_filename.c_str(),ios::binary);
   serialize(learned_function,fout);
   fout.close();

   string banner="Exported learned binary function to "+output_filename;
   outputfunc::write_banner(banner);

   cout << "Current time: " << clock.YYYY_MM_DD_H_M_S() << endl;

// We can also train a decision function that reports a well
// conditioned probability instead of just a number > 0 for the +1
// class and < 0 for the -1 class.  An example of doing that follows:

   typedef probabilistic_decision_function<kernel_type> 
      probabilistic_funct_type;  
   typedef normalized_function<probabilistic_funct_type> pfunct_type;

   pfunct_type learned_pfunct; 
   learned_pfunct.normalizer = normalizer;

   timefunc::initialize_timeofday_clock();
   cout << endl;
   cout << "-----------------------------------------------------" << endl;
   cout << "Starting to train probabilistic decision function:" << endl;
   learned_pfunct.function = train_probabilistic_decision_function(
      trainer, samples, labels, 3);
   cout << "Elapsed time = " 
        << timefunc::elapsed_timeofday_time()/60 << " mins = " 
        << timefunc::elapsed_timeofday_time()/3600 << " hours" << endl;

// Now we have a function that returns the probability that a given
// sample is of the +1 class.

// print out the number of support vectors in the resulting decision
// function. (it should be the same as in the one above)

   cout << "Number of support vectors in our learned_pfunct =  " 
        << learned_pfunct.function.decision_funct.basis_vectors.nr() << endl;
   cout << "Number of training samples: N = " << N << endl;


// Another thing that is worth knowing is that just about everything
// in dlib is serializable. So for example, you can save the
// learned_pfunct object to disk and recall it later like so:

   output_filename=learned_funcs_subdir
      +"colors_pfunct_"+stringfunc::number_to_string(Nlegible)+"_"
      +stringfunc::number_to_string(Nunreadable)+".dat";
   ofstream fout2(output_filename.c_str(),ios::binary);
   serialize(learned_pfunct,fout2);
   fout2.close();

   banner="Exported learned probabilistic function to "+output_filename;
   outputfunc::write_banner(banner);

   cout << "Current time: " << clock.YYYY_MM_DD_H_M_S() << endl;

// Now lets open that file back up and load the function object it
// contains:

   ifstream fin(output_filename.c_str(),ios::binary);
   deserialize(learned_pfunct, fin);

   cout << "Colors binary decision function = "
        << test_binary_decision_function(
           learned_function.function,samples,labels)
        << endl;

}

