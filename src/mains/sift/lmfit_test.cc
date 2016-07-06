// ==========================================================================
// LMFIT_TEST is a playground for Joachim Wuttke's LMFIT C/C++ library
// which performs LevenBerg-Marquardt least-squares minimization and
// curve fitting.  As of April 2012, we suspect this package is better
// and much easier to use than the older LEVMAR library!
// ==========================================================================
// Last updated on 4/6/12
// ==========================================================================

#include <iostream> 
#include <lmcurve.h>
#include "math/basic_math.h"
#include "general/sysfuncs.h"

using std::cout;
using std::endl;


// Rosenbrock function, global minimum at (1, 1) 

// n_params = dimension of parameter vector param

// par = parameter vector.  On input, it must contain reasonable guess
// entries. On output, it contains soln found to minimize |fvec| .

// m_dat = dimension of residue vector fvec
// m_dat >= n_params

// data  = pointer forwarded to evaluate and printout

// evaluate = routine that calculates residue vector fvec for given
// parameter vector par

// *info = setting *info to negative value causes lm_minimize to terminate


void evaluate_rosenbrock(
   const double* par, int m_dat, const void* data, double* fvec, int* info)
{
   const double ROSD=105;

   fvec[0]=sqr(1.0-par[0]) + ROSD*sqr(par[1]-par[0]*par[0]);
   fvec[1]=sqr(1.0-par[0]) + ROSD*sqr(par[1]-par[0]*par[0]);
}

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

// Parameter vector 

   int n_params = 2; // number of parameters in model function f
//   double param[2] = { 0, 0 }; // arbitrary starting value
   double param[2] = { -1.2, -1.0 }; // arbitrary starting value

// Data points 

   int m_dat = 2;
   void* data_ptr=NULL;

// Auxiliary parameters:

   lm_status_struct status;
   lm_control_struct control = lm_control_double;
   control.printflags = 0;
//   control.printflags = 3; // monitor status (+1) and parameters (+2)
   control.ftol=1E-9;
   control.xtol=1E-9;
   control.gtol=1E-9;
   control.maxcall=50000;

// Perform the LM fit:

   cout << "Performing LM fit:" << endl;
   lmmin( 
      n_params, param, m_dat, data_ptr,
      evaluate_rosenbrock, &control, &status, lm_printout_std );

// Print LM minimization results:

   cout << "Fit results:" << endl;
   cout << "Status after " << status.nfev << " function evaluations : "
        << lm_infmsg[status.info] << endl;
    
   cout << "Fitted parameter values:" << endl;
   for ( int i=0; i<n_params; i++ )
   {
      cout << "param[" << i << "] = " << param[i] << endl;
   }
    
   cout << "Residual norm = " << status.fnorm << endl;

}
