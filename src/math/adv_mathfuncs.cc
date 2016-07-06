// ==========================================================================
// "Advanced" math functions which depend upon various non-primitive
// objects such as mypolynomial & prob_distribution.
// ==========================================================================
// Last updated on 6/19/06; 12/4/10; 5/17/11
// ==========================================================================

#include <iostream>
#include "math/adv_mathfuncs.h"
#include "math/genmatrix.h"
#include "datastructures/Linkedlist.h"
#include "math/mathfuncs.h"
#include "datastructures/Mynode.h"
#include "math/mypolynomial.h"
#include "numrec/nrfuncs.h"
#include "math/prob_distribution.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

namespace advmath
{

// Method interpolate_function_or_deriv uses a local Chebyshev
// polynomial approximation to determine the value of a regularly
// sampled function or one of its derivatives at some specified
// location curr_x which need not correspond to a sample point.  The
// function is passed in input array f[] whose number of bins equals
// input integer nbins.  The starting and stopping values for the
// function's independent variables are contained in input parameters
// xstart and xstop.  The desired derivative order of the interpolated
// function is specified by input parameter deriv_order.

   double interpolate_function_or_deriv(
      int nbins,double x[],double f[],double curr_x,int deriv_order)
      {
         const int poly_order=3;
         mypolynomial interp_poly(poly_order);
         interp_poly.set_basis(mypolynomial::chebyshev);

         const int ninterp_pnts=5;
         const int m_max=(ninterp_pnts-1)/2;
         double dx=x[1]-x[0];
         double x_local[ninterp_pnts],f_local[ninterp_pnts];
         int n=basic_math::round((curr_x-x[0])/dx);

         if (n >= m_max && n < nbins-m_max)
         {
            for (int m=-m_max; m<=m_max; m++)
            {
               x_local[m+m_max]=x[n+m];
               f_local[m+m_max]=f[n+m];
            }
            interp_poly.fit_coeffs(ninterp_pnts,x_local,f_local);
            return interp_poly.derivative(curr_x,deriv_order);
         }
         else
         {
            cout << "Error in adv_mathfunc::interpolate_function_or_deriv()" 
                 << endl;
            cout << "Function bin n = " << n 
                 << " nbins = " << nbins 
                 << " m_max = " << m_max << endl;
            return NEGATIVEINFINITY;
         }
      }

   double interpolate_function_or_deriv(
      int nbins,double xstart,double xstop,double f[],
      double curr_x,int deriv_order)
      {
         double dx=(xstop-xstart)/(nbins-1);
         double x[nbins];
         for (int n=0; n<nbins; n++) x[n]=xstart+n*dx;

         double interp_value=interpolate_function_or_deriv(
            nbins,x,f,curr_x,deriv_order);
         return interp_value;
      }

   double interpolate_function_or_deriv(
      vector<double> f,double xstart,double xstop,
      double curr_x,int deriv_order)
      {
         int nbins=f.size();
         double farray[nbins];
         for (int n=0; n<nbins; n++) 
         {
            farray[n]=f[n];
         }
         return interpolate_function_or_deriv(
            nbins,xstart,xstop,farray,curr_x,deriv_order);
      }

// ---------------------------------------------------------------------
// Method locate_zero_crossings uses Newton's method to locate all
// zeroes of the function contained within input array f[].  It
// returns linkedlist zerolist which contains the zeros found within
// the range spanned by input independent variable array x[].

   void locate_zero_crossings(
      int nbins,double xstart,double xstop,double f[],linkedlist& zerolist)
      {
         double dx=(xstop-xstart)/(nbins-1);
         double x[nbins];
         for (int n=0; n<nbins; n++) x[n]=xstart+n*dx;
         locate_zero_crossings(nbins,x,f,zerolist);
      }

   void locate_zero_crossings(
      int nbins,double x[],double f[],linkedlist& zerolist)
      {
         locate_zero_crossings(0,nbins-1,nbins,x,f,zerolist);
      }

// In this overloaded version of locate_zero_crossings, Newton's
// method is applied to array f[] only from bins istart to istop.
// Note that xstart still corresponds to the 0th bin, while xstop
// still corresponds to the nbins-1th bin!
   
   void locate_zero_crossings(
      int istart,int istop,int nbins,double xstart,double xstop,
      double f[],linkedlist& zerolist)
      {
         double dx=(xstop-xstart)/(nbins-1);
         double x[nbins];
         for (int n=0; n<nbins; n++) 
         {
            x[n]=xstart+n*dx;
         }
         locate_zero_crossings(istart,istop,nbins,x,f,zerolist);
      }

   void locate_zero_crossings(
      int istart,int istop,int nbins,double x[],double f[],
      linkedlist& zerolist)
      {
         const int max_iters=10;
         const int ninterp_pnts=5;
         const int m_max=(ninterp_pnts-1)/2;
         const double TINY=1E-6;

         int zerocounter=0;
         double dx=x[istart+1]-x[istart];
         for (int n=istart+m_max; n<istop-m_max; n++)
         {
            double curr_x=x[istart]+n*dx;
            double f0,xnew;

            int iter=0;
            double frac_change=POSITIVEINFINITY;
            do
            {
               f0=interpolate_function_or_deriv(nbins,x,f,curr_x,0);
               double f1=interpolate_function_or_deriv(nbins,x,f,curr_x,1);
               xnew=curr_x-f0/f1;
               iter++;
               if (curr_x != 0) frac_change=fabs((xnew-curr_x)/curr_x);
               curr_x=xnew;
            }
            while (frac_change > TINY && iter < max_iters &&
                   xnew >= x[istart]+m_max*dx && xnew <= x[istop-1]-m_max*dx);
            if (fabs(f0) < TINY)
            {
               bool zero_already_in_list=false;
               for (unsigned int j=0; j<zerolist.size(); j++)
               {
                  if (fabs(zerolist.get_node(j)->get_data().get_func(0)-xnew) 
                      < TINY) zero_already_in_list=true;
               }
               if (!zero_already_in_list) 
               {
                  zerolist.append_node(datapoint(zerocounter,xnew));
                  zerocounter++;
               }
            } // fabs(f0) < TINY conditional
         } // loop over index bin number n 
      }

// ---------------------------------------------------------------------
// Method distributed_random_number returns random numbers distributed
// according to some input probability distribution.  This method
// first computes the cumulative distribution p_cumulative.  It next
// generates a uniform random number u over the interval [0,1].  This
// method then returns xstar such that p_cumulative(xstar) = u.

   double distributed_random_number(int thread_number,prob_distribution& prob)
      {
         prob.compute_cumulative_distribution();
         double p_cumulative[prob.get_nbins()];
         for (int n=0; n<prob.get_nbins(); n++)
         {
            p_cumulative[n]=prob.get_pcum(n);
         }
         double u=nrfunc::ran2(thread_number);  
         int n=mathfunc::binary_locate(p_cumulative,0,prob.get_nbins()-1,u);
         return prob.get_x(basic_math::min(n+1,prob.get_nbins()-1));
      }

// ---------------------------------------------------------------------
// Function initialize_real_SOthree_generators dynamically allocates
// memory for 3x3 SO(3) generators.  It then initializes these
// matrices to sqrt(-1) times the conventional SO(3) generator values.

   void initialize_real_SOthree_generators(genmatrix* iJ_ptr[])
      {
         for (int j=0; j<3; j++)
         {
            iJ_ptr[j]=new genmatrix(3,3);
         }
          iJ_ptr[0]->put(1,2,-1);
          iJ_ptr[0]->put(2,1,1);
          iJ_ptr[1]->put(0,2,1);
          iJ_ptr[1]->put(2,0,-1);
          iJ_ptr[2]->put(0,1,-1);
          iJ_ptr[2]->put(1,0,1);
      }

// ==========================================================================
// Gaussian density methods:
// ==========================================================================

// Method generate_gaussian_density constructs a gaussian density
// distribution.  It returns the probability distribution object
// corresponding to this gaussian density.

   prob_distribution generate_gaussian_density(
      int nbins,double mu,double sigma)
      {
         double xlo=0;		// pure black
         double xhi=1;		// pure white
         double dx=(xhi-xlo)/(nbins-1);
         double gaussian_density[nbins];
         for (int n=0; n<nbins; n++)
         {
            double x=xlo+n*dx;
            gaussian_density[n]=exp(-0.5*sqr((x-mu)/sigma))/
               (sqrt(2*PI)*sigma);
         }
         return prob_distribution(nbins,xlo,xhi,gaussian_density);
      }

// ----------------------------------------------------------------
// Method poor_mans_gaussian_density models a gaussian distribution as
// a symmetrical triangle centered at x=mu with FWHM = 2 * sigma.  The
// area underneath the triangle equals unity. 
   
// See notes entitled "Poor man's gaussian distribution dated
// 6/16/06".

   double poor_mans_gaussian_density(double x,double mu,double sigma)
      {
         if (x <= mu-2*sigma)
         {
            return 0;
         }
         else if (x > mu-2*sigma && x <= mu)
         {
            return sqr(x-(mu-2*sigma))/(8.0*sqr(sigma));
         }
         else if (x > mu && x <= mu+2*sigma)
         {
            return 1-sqr(x-(mu+2*sigma))/(8.0*sqr(sigma));
         }
         else 
         {
            return 1;
         }
      }

// ----------------------------------------------------------------
// Method poor_mans_cum_gaussian_inverse takes in a Pcum value ranging
// from 0 to 1.  It returns the corresponding x value obtained by
// inverting the cumulative distribution for our poor man's gaussian
// density.
   
   double poor_mans_cum_gaussian_inverse(double Pcum,double mu,double sigma)
      {
         if (Pcum <= 0)
         {
            return 0;
         }
         else if (Pcum > 0 && Pcum <= 0.5)
         {
            return mu-2*sigma+sqrt(8*Pcum)*sigma;
         }
         else if (Pcum > 0.5 && Pcum < 1)
         {
            return mu+2*sigma-sqrt(8*(1-Pcum))*sigma;
         }
         else
         {
            return 1.0;
         }
      }
   

} // advmath namespace




