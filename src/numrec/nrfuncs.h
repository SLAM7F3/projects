// =========================================================================
// Header file for stand-alone Numerical Recipes functions.
// =========================================================================
// Last modified on 12/29/05; 4/5/14; 4/12/14
// =========================================================================

#ifndef NRFUNCS_H
#define NRFUNCS_H

namespace nrfunc
{
   void init_default_seed(long s);
   long init_time_based_seed();
   void init_thread_seeds();
   double expdev(double lamba=1.0);
   double expdev(int thread_number,double lamba=1.0);
   double gasdev();
   double gasdev(int thread_number);
   double ran1();
   double ran1(int thread_number);
   double ran2();
   double ran2(int thread_number);

   void fill_random_array(unsigned int n_entries);
   double get_next_random_array_element();

// ==========================================================================
// Inlined methods:
// ==========================================================================

} // nrfunc namespace

#endif // numrec/nrfuncs.h
