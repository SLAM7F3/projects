// =======================================================================
// Program LEARN_METRIC

//			./learn_metric

// =======================================================================
// Last updated on 10/3/13
// =======================================================================

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <dlib/svm.h>

#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "video/RGB_analyzer.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cout;
using std::endl;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

int main( int argc, char** argv ) 
{
   std::set_new_handler(sysfunc::out_of_memory);

   string JAV_subdir=
      "/data/video/JAV/NewsWraps/early_Sep_2013/jpg_frames/";
   string images_subdir=JAV_subdir;
   string matching_images_filename=images_subdir+"matching_images.dat";
   string nonmatching_images_filename=images_subdir+"nonmatching_images.dat";

   // Make a typedef for the kind of object we will be ranking.  In this
   // example, we are ranking 3-dimensional vectors.  
   typedef dlib::matrix<double,3,1> sample_type;

   // Now let's make some testing data.  To make it really simple, lets
   // suppose that vectors with positive values in the first dimension
   // should rank higher than other vectors.  So what we do is make
   // examples of relevant (i.e. high ranking) and non-relevant (i.e. low
   // ranking) vectors and store them into a ranking_pair object like so:

   dlib::ranking_pair<sample_type> data;
   sample_type samp;

   vector<vector<double> > matching_row_numbers=filefunc::ReadInRowNumbers(
      matching_images_filename);
   for (int r=0; r<matching_row_numbers.size(); r++)
   {
      for (int c=0; c<matching_row_numbers[r].size(); c++)
      {
         samp(c)=matching_row_numbers[r].at(c);
      }
      data.relevant.push_back(samp);
   } // loop over index r labeling rows within input file
   cout << "data.relevant.size() = " << data.relevant.size() << endl;

   vector<vector<double> > nonmatching_row_numbers=filefunc::ReadInRowNumbers(
      nonmatching_images_filename);
   for (int r=0; r<nonmatching_row_numbers.size(); r++)
   {
      for (int c=0; c<nonmatching_row_numbers[r].size(); c++)
      {
         samp(c)=nonmatching_row_numbers[r].at(c);
      }
      data.nonrelevant.push_back(samp);
   } // loop over index r labeling rows within input file
   cout << "data.nonrelevant.size() = " << data.nonrelevant.size() << endl;

   // Now that we have some data, we can use a machine learning method to
   // learn a function that will give high scores to the relevant vectors
   // and low scores to the non-relevant vectors.

   // The first thing we do is select the kernel we want to use.  For the
   // svm_rank_trainer there are only two options.  The linear_kernel and
   // sparse_linear_kernel.  The latter is used if you want to use sparse
   // vectors to represent your objects.  Since we are using dense vectors
   // (i.e. dlib::matrix objects to represent the vectors) we use the
   // linear_kernel.

   typedef dlib::linear_kernel<sample_type> kernel_type;

   // Now make a trainer and tell it to learn a ranking function based on
   // our data.
   dlib::svm_rank_trainer<kernel_type> trainer;
   dlib::decision_function<kernel_type> rank = trainer.train(data);

   // Now if you call rank on a vector it will output a ranking score.  In
   // particular, the ranking score for relevant vectors should be larger
   // than the score for non-relevant vectors.  

   for (int t=0; t<75; t++)
   {
      double relevant_score=rank(data.relevant[t]);
      double nonrelevant_score=rank(data.nonrelevant[t]);
      
      cout << "t = " << t 
           << " relevant score = " << relevant_score
           << " non-relevant score = " << nonrelevant_score;
      if (relevant_score < nonrelevant_score) 
         cout << " !!!";
      cout << endl;
   }
   
   // If we want an overall measure of ranking accuracy we can compute the
   // ordering accuracy and mean average precision values by calling
   // test_ranking_function().  In this case, the ordering accuracy tells
   // us how often a non-relevant vector was ranked ahead of a relevant
   // vector.  This function will return a 1 by 2 matrix containing these
   // measures.  In this case, it returns 1 1 indicating that the rank
   // function outputs a perfect ranking.

   cout << "testing (ordering accuracy, mean average precision): " 
        << test_ranking_function(rank, data) << endl;

   // We can also see the ranking weights:
   cout << "learned ranking weights: \n" << rank.basis_vectors(0) << endl;
   // In this case they are:
   //  0.5 
   // -0.5 


/*
   // In the above example, our data contains just two sets of objects.
   // The relevant set and non-relevant set.  The trainer is attempting to
   // find a ranking function that gives every relevant vector a higher
   // score than every non-relevant vector.  Sometimes what you want to do
   // is a little more complex than this. 
   //
   // For example, in the web page ranking example we have to rank pages
   // based on a user's query.  In this case, each query will have its own
   // set of relevant and non-relevant documents.  What might be relevant
   // to one query may well be non-relevant to another.  So in this case
   // we don't have a single global set of relevant web pages and another
   // set of non-relevant web pages.  
   //
   // To handle cases like this, we can simply give multiple ranking_pair
   // instances to the trainer.  Therefore, each ranking_pair would
   // represent the relevant/non-relevant sets for a particular query.  An
   // example is shown below (for simplicity, we reuse our data from above
   // to make 4 identical "queries").

   vector<dlib::ranking_pair<sample_type> > queries;
   queries.push_back(data);
   queries.push_back(data);
   queries.push_back(data);
   queries.push_back(data);

   // We train just as before.  
   rank = trainer.train(queries);


   // Now that we have multiple ranking_pair instances, we can also use
   // cross_validate_ranking_trainer().  This performs cross-validation by
   // splitting the queries up into folds.  That is, it lets the trainer
   // train on a subset of ranking_pair instances and tests on the rest.
   // It does this over 4 different splits and returns the overall
   // ranking accuracy based on the held out data.  Just like 
   // test_ranking_function(), it reports both the ordering accuracy and 
   // mean average precision.

   cout << "cross-validation (ordering accuracy, mean average precision): " 
        << cross_validate_ranking_trainer(trainer, queries, 4) << endl;
*/



}
