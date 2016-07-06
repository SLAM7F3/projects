// ==========================================================================
// Program CROSSENTROPY is a playground for learning about the Cross
// Entropy Method for numerically solving difficult optimization
// problems.  This example comes from
// http://en.wikipedia.org/wiki/Cross-entropy_method.

//			crossentropy


// ==========================================================================
// Last updated on 6/15/10
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <math.h>
#include <string>
#include <vector>
#include "math/basic_math.h"
#include "math/mathfuncs.h"
#include "numrec/nrfuncs.h"
#include "general/sysfuncs.h"
#include "templates/mytemplates.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::ios;
   using std::ofstream;
   using std::ostream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

   double mu=-6;
   double sigma2=100;
   double sigma=sqrt(sigma2);
   int iter=0;
   int max_iters=100;
   
   int n_samples=100;
   int ne=10; 
   
   double epsilon=1E-6;
   
   cout.precision(12);

   double factor=0.1;
   
   while (iter < max_iters && sigma2 > epsilon)
   {

      vector<double> X,S;
      for (int i=0; i<n_samples; i++)
      {
         double curr_x=mu+sigma*nrfunc::gasdev();
         X.push_back(curr_x);
         double curr_s=exp(-sqr(curr_x))+factor*exp(-sqr(curr_x+2));
         S.push_back(curr_s);
      } // loop over i labeling samples
      templatefunc::Quicksort_descending(S,X);
      
      vector<double> X_best;
      for (int i=0; i<ne; i++)
      {
         X_best.push_back(X[i]);
      }
      double mean_new=mathfunc::mean(X_best);
      double sigma_new=mathfunc::std_dev(X_best);
      mu=mean_new;
      sigma=sigma_new;

      cout << "iter = " << iter << " mu = " << mu << endl;
       
      iter++;
   } // while loop over iter

   int nbins=1000000;
   double x_lo=-0.1;
   double x_hi=0.1;
   double delta_x=(x_hi-x_lo)/(nbins-1);
   double max_s=NEGATIVEINFINITY;
   double xstar=NEGATIVEINFINITY;
   for (int i=0; i<nbins; i++)
   {
      double x=x_lo+i*delta_x;
      double s=exp(-sqr(x))+factor*exp(-sqr(x+2));
      if (s > max_s)
      {
         max_s=s;
         xstar=x;
      }
   }

   cout << "xstar = " << xstar << endl;
   
   
}


