// ==================================================================== 
// Numerical Recipes subroutine header declarations
// ==================================================================== 
// Last updated on 8/25/05; 2/26/06
// ==================================================================== 

#ifndef SPLINE_H_
#define SPLINE_H_

#include <vector>

namespace numrec
{
   extern std::vector<double> y2;

   void init_spline_2nd_derivs(
      const std::vector<double>& x,const std::vector<double>& y);
   void init_spline_2nd_derivs(double x[], double y[], int n, double y2[]);
   void init_spline_2nd_derivs(
      double x[], double y[], int n, double yp1, double ypn, 
      double y2[], bool natural_spline_flag=true);

   double spline_interp(
      std::vector<double>& xa,std::vector<double>& ya,double x);
   double spline_interp(
      double xa[], double ya[], double y2a[], int n, double x);

} // numrec namespace

#endif /* SPLINE_H_ */




