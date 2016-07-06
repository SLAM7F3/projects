// ==========================================================================
// Header file for stand-alone complex variable methods
// ==========================================================================
// Last updated on 3/31/12; 4/2/12; 4/8/12
// ==========================================================================

#ifndef COMPLEXFUNC_H
#define COMPLEXFUNC_H

#include <set>
#include <vector>
#include "math/complex.h"

class genmatrix;
class mypolynomial;

namespace complexfunc
{
   std::vector<double> find_real_polynomial_roots(const mypolynomial& poly);
   std::vector<complex> find_polynomial_roots(const mypolynomial& poly);
   std::vector<complex> compute_eigenvalues(genmatrix* M_ptr);
   std::pair<std::vector<complex>,genmatrix*> 
      eigen_system(genmatrix* M_ptr);

   complex polynomial_value(
      double x,double c_order,const std::vector<complex>& poly_roots);
   complex polynomial_value(
      complex z,complex c_order,const std::vector<complex>& poly_roots);
   
} // complexfunc namespace

#endif  // complexfuncs.h
