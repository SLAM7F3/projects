// ==========================================================================
// NRFUNCS stand-alone methods
// ==========================================================================
// Last modified on 1/4/07; 4/5/14; 4/12/14
// ==========================================================================

#include <iostream>
#include <string>
#include <time.h>
#include <vector>
#include "math/basic_math.h"
#include "numrec/nr.h"
#include "numrec/nrfuncs.h"

#include "general/outputfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;

namespace nrfunc
{

// Experiment:  Thu Feb  6 19:48:56 EST 2003
// Set default_seed = -2000 to test sensitivity of results to random numbers
//   long default_seed=-2000;

// Long integer default_seed is effectively a global variable whose
// default initial value equals -1000.  Anytime a call is made to a
// method such as ran1(), ran2(), etc with no arguments, the value for
// default_seed is altered by the corresponding Numerical Recipes
// subroutine.  Default_seed should therefore never be seen or
// accessed outside of the numrec namespace.

   long default_seed=-1000;

// In Aug 03, we realized it would be useful to introduce an entire
// array of long integer seeds within this namespace for threading
// purposes.  We generalize the ran1(), ran2(), etc methods so that
// they can take an integer thread_number input argument.  Each thread
// is then associated with some particular seed which should not be
// affected by calls by any other thread to the methods within this
// namespace...

   const unsigned int max_nthreads=5000;
   long seed[max_nthreads];

// In order to increase execution speed, we sometimes want to store a
// bunch of random numbers within a vector which may be initialized
// just once via a call to fill_random_array() and whose values may be
// recycled.  Internal integer index random_number_index labels which
// random entry within the vector is to be returned by a subsequent
// call to get_next_random_array_element().

   std::vector<double> random_vector;
   int random_number_index=0;

// ==========================================================================

   void init_default_seed(long s)
      {
         default_seed=s;

//         cout << "inside nrfunc::init_default_seed()" << endl;
//         cout << "Default_seed = " << default_seed << endl;

//         string message=
//            "Enter any non-white character followed by carriage return to continue:";
//         char junkchar;
//         cout << message << endl;
//         cin >> junkchar;
      }
   
   long init_time_based_seed()
      {
         time_t curr_time=time(NULL);
         default_seed=-curr_time%100000;
//         cout << "inside nrfunc::init_time_based_seed()" << endl;
//         cout << "Default_seed = " << default_seed << endl;

//         string message=
//            "Enter any non-white character followed by carriage return to continue:";
//         char junkchar;
//         cout << message << endl;
//         cin >> junkchar;
         return default_seed;
      }

   void init_thread_seeds()
      {
         for (unsigned int nthread=0; nthread<max_nthreads; nthread++)
         {
            seed[nthread]=-1000-nthread;
         }
      }

// Method expdev returns a random variable obeying the exponential
// distribution p(x) = lambda exp(-lambda x):

   double expdev(double lambda)
      {
         return numrec::expdev(&default_seed)/lambda;
      }

   double expdev(int thread_number,double lambda)
      {
         return numrec::expdev(seed+thread_number)/lambda;
      }

   double gasdev()
      {
         return numrec::gasdev(&default_seed);
      }

   double gasdev(int thread_number)
      {
         return numrec::gasdev(seed+thread_number);
      }

   double ran1()
      {
//         cout << "inside nrfunc::ran1()" << endl;
//         outputfunc::enter_continue_char();
         return numrec::ran1(&default_seed);
      }

   double ran1(int thread_number)
      {
//         cout << "inside nrfunc::ran1(thread_number)" << endl;
//         outputfunc::enter_continue_char();
         return numrec::ran1(seed+thread_number);
      }

   double ran2()
      {
         return numrec::ran2(&default_seed);
      }

   double ran2(int thread_number)
      {
         return numrec::ran2(seed+thread_number);
      }

   void fill_random_array(unsigned int n_entries)
      {
         random_vector.resize(n_entries);
         for (unsigned int n=0; n<n_entries; n++) random_vector[n]=ran1();
//         ostream_iterator<double> output(cout," ");
//         std::copy(random_vector.begin(),random_vector.end(),output);
//         cout << endl;
      }

   double get_next_random_array_element()
      {
         return random_vector[
            modulo(random_number_index++,random_vector.size())];
      }
 
} // nrfunc namespace


