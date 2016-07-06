// ==========================================================================
// Header file for stand-alone "advanced" math functions which depend
// upon various non-primitive objects such as mypolynomial.
// ==========================================================================
// Last updated on 9/29/05; 6/16/06; 5/17/11
// ==========================================================================

#ifndef ADVANCED_MATHFUNCS_H
#define ADVANCED_MATHFUNCS_H

#include <vector>
#include "datastructures/datapoint.h"

class prob_distribution;
template <class T> class Linkedlist;
typedef Linkedlist<datapoint> linkedlist;
class genmatrix;

namespace advmath
{
   double interpolate_function_or_deriv(
      int nbins,double x[],double f[],double curr_x,int deriv_order);
   double interpolate_function_or_deriv(
      int nbins,double xstart,double xstop,double f[],
      double curr_x,int deriv_order);
   double interpolate_function_or_deriv(
      std::vector<double> f,double xstart,double xstop,
      double curr_x,int deriv_order);

   void locate_zero_crossings(
      int nbins,double xstart,double xstop,double f[],linkedlist& zerolist);
   void locate_zero_crossings(
      int nbins,double x[],double f[],linkedlist& zerolist);
   void locate_zero_crossings(
      int istart,int istop,int nbins,double xstart,double xstop,
      double f[],linkedlist& zerolist);
   void locate_zero_crossings(
      int istart,int istop,int nbins,double x[],double f[],
      linkedlist& zerolist);
   double distributed_random_number(
      int thread_number,prob_distribution& prob);
   void initialize_real_SOthree_generators(genmatrix* iJ_ptr[]);

// Gaussian density methods:

   prob_distribution generate_gaussian_density(
      int nbins,double mu,double sigma);
   double poor_mans_gaussian_density(double x,double mu,double sigma);
   double poor_mans_cum_gaussian_inverse(double Pcum,double mu,double sigma);
}

#endif  // advmath namespace

