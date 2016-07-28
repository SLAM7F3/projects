// ==========================================================================
// Filter functions 
// ==========================================================================
// Last updated on 2/19/09; 8/18/11; 7/1/13
// ==========================================================================

#include <iomanip>
#include <math.h>
#include "math/basic_math.h"
#include "math/constants.h"
#include "filter/filterfuncs.h"
#include "math/genmatrix.h"
#include "templates/mytemplates.h" 
#include "numrec/nr.h"

using std::cout;
using std::endl;
using std::ios;
using std::vector;

namespace filterfunc
{

// ==========================================================================
// Gaussian filter methods:
// ==========================================================================

// Method gaussian_filter generates a gaussian function with unit
// area.  The physical size dx of each bin must be passed as an input
// parameter into this method.

   void gaussian_filter(int nhbins,double dx,double sigma,double h[])
      {
         double xlo=-nhbins/2*fabs(dx); 
         double hsum=0;
         for (int n=0; n<nhbins; n++)
         {
            double x=xlo+n*fabs(dx);
            h[n]=exp(-0.5*sqr(x/sigma));
            hsum += h[n];
         }
         for (int n=0; n<nhbins; n++)
         {
            h[n] /= hsum;
//            cout << "n = " << n << " x = " << xlo+n*fabs(dx)
//                 << " h = " << h[n] << endl;
         }
      }

   void gaussian_filter(double dx,double sigma,vector<double>& h)
      {

// Force number of output bins in STL filter vector h to be odd:

         int n_hbins=h.capacity();
         cout << "n_hbins = " << n_hbins << endl;
         if (is_even(n_hbins)) n_hbins++;

         double xlo=-n_hbins/2*fabs(dx);
         double hsum=0;
         for (int n=0; n<n_hbins; n++)
         {
            double x=xlo+n*fabs(dx);
            h.push_back(exp(-0.5*sqr(x/sigma)));
            hsum += h[n];
         }
         for (int n=0; n<n_hbins; n++)
         {
            h[n] /= hsum;
//            cout << "n = " << n << " h[n] = " << h[n] << endl;
         }
//         outputfunc::enter_continue_char();
      }

// ---------------------------------------------------------------------
// This overloaded version of gaussian_filter fills up an array with
// values equal to a 1D gaussian or its 1st or 2nd derivative.  The
// size of the array in pixels is forced to be odd and is controlled
// by the input standard deviation.  Input parameter
// e_folding_distance regulates how far away from the mean one bothers
// to keep track of non-negligible gaussian filter values.

   int gaussian_filter_size(double std_dev,double delta_x)
      {
         return gaussian_filter_size(std_dev,delta_x,6.0);
      }

   int gaussian_filter_size(double std_dev,double delta_x,
                            double e_folding_distance)
      {
         int nsize=basic_math::mytruncate(
            e_folding_distance*SQRT_TWO*std_dev/delta_x)+1;

         if (is_even(nsize)) nsize++;
         return nsize;
      }

   void gaussian_filter(
      int nsize,int deriv_order,double std_dev,double delta_x,double filter[])
      {
         double xhi=(nsize/2)*delta_x;
         double xlo=-xhi;
         const double prefactor=1/(SQRT_TWO_PI*std_dev);
         const double std_dev_2=sqr(std_dev);
         const double std_dev_4=sqr(std_dev_2);
         for (int m=0; m<nsize; m++)
         {
            double x=xlo+m*delta_x;
            if (deriv_order==0)
            {
               filter[m]=prefactor*exp(-0.5*sqr(x/std_dev));
            }
            else if (deriv_order==1)
            {
               filter[m]=prefactor*x/std_dev_2*exp(-0.5*sqr(x/std_dev));
            }
            else if (deriv_order==2)
            {
               filter[m]=prefactor*(sqr(x)-std_dev_2)/std_dev_4*
                  exp(-0.5*sqr(x/std_dev));
            }
         } // loop over index m
      }

   vector<double> gaussian_filter(
      int nsize,int deriv_order,double std_dev,double delta_x)
      {
         double xhi=(nsize/2)*delta_x;
         double xlo=-xhi;
         const double prefactor=1/(SQRT_TWO_PI*std_dev);
         const double std_dev_2=sqr(std_dev);
         const double std_dev_4=sqr(std_dev_2);
         vector<double> filter;

         for (int m=0; m<nsize; m++)
         {
            double x=xlo+m*delta_x;
            if (deriv_order==0)
            {
               filter.push_back(prefactor*exp(-0.5*sqr(x/std_dev)));
            }
            else if (deriv_order==1)
            {
               filter.push_back(
                  prefactor*x/std_dev_2*exp(-0.5*sqr(x/std_dev)));
            }
            else if (deriv_order==2)
            {
               filter.push_back((prefactor*(sqr(x)-std_dev_2)/std_dev_4*
                                 exp(-0.5*sqr(x/std_dev))));
            }
         } // loop over index m

         return filter;
      }

// ==========================================================================
// 2D Gaussian filter methods:
// ==========================================================================

// Method gaussian_2D_filter generates a 2D gaussian function with
// unit area.  The physical size dx of each squared pixel must be
// passed as an input parameter into this method.

   genmatrix* gaussian_2D_filter(
      double dx,double sigma,double e_folding_distance)
      {
         int nhbins=gaussian_filter_size(sigma,dx,e_folding_distance);
         return gaussian_2D_filter(dx,sigma,nhbins);
      }

   genmatrix* gaussian_2D_filter(double dx,double sigma,int nhbins)
      {
         genmatrix* h_ptr=new genmatrix(nhbins,nhbins);
         double xlo=-nhbins/2*dx;
         double ylo=xlo;
         double dy=dx;

         double hsum=0;
         for (int m=0; m<nhbins; m++)
         {
            double x=xlo+m*dx;
            for (int n=0; n<nhbins; n++)
            {
               double y=ylo+n*dy;
               double rsq=x*x+y*y;
               double h=exp(-0.5*rsq/sqr(sigma));
               h_ptr->put(m,n,h);
               hsum += h;
            }
         }

// Renormalize *h_ptr values so that their integral equals unity:

         for (int m=0; m<nhbins; m++)
         {
            for (int n=0; n<nhbins; n++)
            {
               h_ptr->put(m,n,h_ptr->get(m,n)/hsum);
            }
         }

//         cout << "2D gaussian filter = " << *h_ptr << endl;
         return h_ptr;
      }

// ==========================================================================
// Brute force filter methods:
// ==========================================================================

// Method brute_force_filter explicitly convolves an input function
// x[] with an input filter h[] in one-dimension.  

   void brute_force_filter(
      int nxbins,int nhbins,double x[],double h[],double y[],
      bool wrap_around_input_values)
      {
// Force number of bins in filter array h[] to be odd:

         if (is_even(nhbins)) nhbins--;
         
// First reverse entries within filter array h prior to performing
// convolution:

         double hflipped[nhbins];
         for (int i=0; i<nhbins; i++)
         {
            hflipped[i]=h[nhbins-1-i];
         }

// Next shift zero-point of filter array so that filter weights start
// at index -nhbins/2 and run up to +nhbins/2:

         double *hshifted=hflipped+nhbins/2;
         
         double xsum=0;
         double ysum=0;
         for (int n=0; n<nxbins; n++)
         {
            xsum += x[n];
            y[n]=0;
            for (int i=-nhbins/2; i<=nhbins/2; i++)
            {
               double currx;
               if (wrap_around_input_values)
               {
                  currx=x[modulo(n+i,nxbins)];                  
               }
               else if (n+i < 0 && !wrap_around_input_values)
               {
                  currx=0;
               }
               else if (n+i >= nxbins && !wrap_around_input_values)
               {
                  currx=0;
               }
               else
               {
                  currx=x[n+i];
               }
               y[n] += currx*hshifted[i];
            } // loop over index i
            ysum += y[n];
         } // loop over index n
//         cout << "Integral of input signal = " << xsum << endl;
//         cout << "Integral of filtered output signal = " << ysum << endl;
      }

// ---------------------------------------------------------------------
// In this overloaded version of brute_force_filter, we pass the raw
// data and filter in and the smoothed data out via STL vectors.
// Provided the integral of filter h does not equal zero (as for 1st
// derivative filters), we explicitly divide by the integral of the h
// function which is actually used to smooth the raw data.  (The input
// filter h is assumed to be normalized.)  This renormalization
// ensures that the filtering operation returns reasonable values at
// the beginning and ends of the data set.

   void brute_force_filter(
      const vector<double>& x,const vector<double>& h,vector<double>& y,
      bool wrap_around_input_values)
   {
      vector<bool> xOK_flag;
      for (unsigned int i=0; i<x.size(); i++)
      {
         xOK_flag.push_back(true);
      }
      brute_force_filter(x,xOK_flag,h,y,wrap_around_input_values);
   }
   
   void brute_force_filter(
      const vector<double>& x,const vector<bool>& xOK_flag,
      const vector<double>& h,vector<double>& y,
      bool wrap_around_input_values)
      {
         int nxbins=x.size();
         int nhbins=h.size();

// Force number of bins in filter array h[] to be odd:

         if (is_even(nhbins)) nhbins--;
         
// First reverse entries within filter array h prior to performing
// convolution:

         double hflipped[nhbins];
         for (int i=0; i<nhbins; i++)
         {
            hflipped[i]=h[nhbins-1-i];
         }

// Next shift zero-point of filter array so that filter weights start
// at index -nhbins/2 and run up to +nhbins/2:

         double *hshifted=hflipped+nhbins/2;
         
         double xsum=0;
         double ysum=0;
         for (int n=0; n<nxbins; n++)
         {
            double curr_y=0;
            double hsum=0;

            xsum += x[n];
            for (int i=-nhbins/2; i<=nhbins/2; i++)
            {
               double currx=0;
               if (wrap_around_input_values)
               {
                  int j=modulo(n+i,nxbins);
                  if (xOK_flag[j])
                  {
                     currx=x[j];
                  }
               }
               else if (n+i < 0 && !wrap_around_input_values)
               {
                  currx=0;
               }
               else if (n+i >= nxbins && !wrap_around_input_values)
               {
                  currx=0;
               }
               else
               {
                  int j=n+i;
                  if (xOK_flag[j])
                  {
                     currx=x[n+i];
                     hsum += hshifted[i];
                  }
               }
               curr_y += currx*hshifted[i];
//               cout << "i = " << i << " hshifted = " << hshifted[i]
//                    << " currx = " << currx << " curry = " << curr_y
//                    << " y-x = " << curr_y-currx 
//                    << " hsum = " << hsum << endl;
            } // loop over index i

            const double TINY=1E-8;
            if (nearly_equal(hsum,0,TINY))
            {
               y.push_back(curr_y);
            }
            else
            {
               y.push_back(curr_y/hsum);
            }
            
            ysum += y[n];
         } // loop over index n
//         cout << "Integral of input signal = " << xsum << endl;
//         cout << "Integral of filtered output signal = " << ysum << endl;
      }

// ---------------------------------------------------------------------
// Method boxcar_filter generates a boxcar function with unit area:

   void boxcar_filter(int nhbins,vector<double>& h)
      {
         h.clear();
         for (int n=0; n<nhbins; n++)
         {
            h.push_back(1.0/double(nhbins));
         }
      }

   void boxcar_filter(int nhbins,double h[])
      {
         for (int n=0; n<nhbins; n++)
         {
            h[n]=1.0/double(nhbins);
         }
      }

// ---------------------------------------------------------------------
// Method circularly_shift_filter takes in a set of filter values
// within input array h[] along with a shift parameter x_shift.  It
// returns a cyclic permutation of the filter values within
// h_shifted[].

   void circularly_shift_filter(
      int nhbins,double dx,double x_shift,const double h[],double h_shifted[])
      {
         int n_shift=basic_math::round(x_shift/dx);
         for (int n=0; n<nhbins; n++)
         {
            h_shifted[n]=h[modulo(n-n_shift,nhbins)];
         }
      }

// ---------------------------------------------------------------------
// Method circularly_replicate_filter takes in set of filter values
// within input array h[] along with shift parameter x_shift and
// n_copies number of replicated filter copies to produce.  It
// circularly distributes n_copies of the filter within output array
// h_replicate[].

   void circularly_replicate_filter(
      int nhbins,double dx,int n_copies,double x_shift,
      double h[],double h_replicate[])
      {
         for (int n=0; n<nhbins; n++) h_replicate[n]=0;
         
         double h_shifted[nhbins];
         for (int i=-1; i<n_copies-1; i++)
         {
            double total_xshift=i*x_shift;
            circularly_shift_filter(nhbins,dx,total_xshift,h,h_shifted);
            for (int n=0; n<nhbins; n++)
            {
               h_replicate[n] += h_shifted[n];
            }
         } // loop over filter copy index i
      }

// ==========================================================================
// Alpha-beta-gamma filter methods:
// ==========================================================================

// First order alpha filter.  Recall this filter is stable if |alpha|
// < 1.  See "First order filter" notes dated 2/21/99 in "Tracking
// accelerating targets at low S/N" folder.

   double alpha_filter(double curr_raw_x,double prev_filtered_x,double alpha)
      {
//         double curr_filtered_x=alpha*curr_raw_x+(1-alpha)*prev_filtered_x;
//         return curr_filtered_x;
         return alpha*curr_raw_x+(1-alpha)*prev_filtered_x;
      }

   threevector alpha_filter(
      const threevector& curr_raw_r,const threevector& prev_filtered_r,
      double alpha)
   {
      double curr_filtered_x=alpha_filter(
         curr_raw_r.get(0),prev_filtered_r.get(0),alpha);
      double curr_filtered_y=alpha_filter(
         curr_raw_r.get(1),prev_filtered_r.get(1),alpha);
      double curr_filtered_z=alpha_filter(
         curr_raw_r.get(2),prev_filtered_r.get(2),alpha);
      return threevector(curr_filtered_x,curr_filtered_y,curr_filtered_z);
   }

// ---------------------------------------------------------------------
// Second order alpha-beta filter.  Recall this filter is stable if

//    |lambda_1,2| = 0.5 | alpha +/- sqrt(alpha**2+4*beta)| < 1

// See "Second order filter" notes dated 2/21/99.  Smaller values of
// alpha imply greater amounts of filtering.

// Optimal filter has beta=a**2/(2-a)

   void alphabeta_filter(
      double curr_raw_x,double prev_filtered_x,double prev_filtered_xdot,
      double& curr_filtered_x,double& curr_filtered_xdot,
      double alpha,double beta,double dt)
      {
// Filtered estimate for velocity at present time:   
         curr_filtered_xdot=beta*(curr_raw_x-prev_filtered_x)/dt+
            (1-beta)*prev_filtered_xdot;

// Filtered estimate for position at present time:
         curr_filtered_x=alpha*curr_raw_x+(1-alpha)*(
            prev_filtered_x+dt*prev_filtered_xdot);
      }

// ---------------------------------------------------------------------
// Third order alpha-beta-gamma filter:

// Optimal relations derived by Paul Kalata in "The Tracking Index: A
// Generalized Parameter for alpha-beta and alpha-beta-gamma Target
// Trackers", 1984

// beta = 2(2-alpha) - 4 sqrt(1-alpha)

// gamma = beta**2/(2*alpha)  [not certain about factor of 2 in denom]

   void alphabetagamma_filter(
      double curr_raw_x,double prev_filtered_x,double prev_filtered_xdot,
      double prev_filtered_xdotdot,double& curr_filtered_x,
      double& curr_filtered_xdot,double& curr_filtered_xdotdot,
      double alpha,double beta,double gamma,double dt)
      {

// Filtered estimate for acceleration at present time:   
         curr_filtered_xdotdot=(1-gamma/2)*prev_filtered_xdotdot
            -gamma*prev_filtered_xdot/dt
            +gamma*(curr_raw_x-prev_filtered_x)/sqr(dt);

// Filtered estimate for velocity at present time:   
         curr_filtered_xdot=beta*(curr_raw_x-prev_filtered_x)/dt+
            (1-beta)*prev_filtered_xdot
            +(1-beta/2)*prev_filtered_xdotdot*dt;

// Filtered estimate for position at present time:
         curr_filtered_x=alpha*curr_raw_x+(1-alpha)*(
            prev_filtered_x+prev_filtered_xdot*dt
            +0.5*prev_filtered_xdotdot*sqr(dt));
      }

// ==========================================================================
// Savitzky-Golay filter methods:
// ==========================================================================

// Method savitzky_golay_filter takes in SG filter parameters
// derivative_order, polynomial_order and n_filter_points.  It fills
// output array h_savitzky with filter values in UNWRAPPED order -
// i.e. the main portion of the filter will occur within the middle of
// the array (rather than being split across the beginning and end of
// the array).  The input size of the array nbins should be ODD.

   void savitzky_golay_filter
     (int nbins,double h_savitzky[],int deriv_order,int poly_order,
      int n_filter_points)
     {
       double *tmp_h_savitzky;
       new_clear_array(tmp_h_savitzky,nbins);
       clear_array(h_savitzky,nbins);

       if (is_even(nbins)) nbins--;

       numrec::savgol(tmp_h_savitzky,nbins,n_filter_points,n_filter_points,
                      deriv_order,poly_order);

       // Unwrap filter values:


       for (int i=0; i<nbins/2+1; i++)
         {
           int iskip=i+nbins/2;
           h_savitzky[iskip]=tmp_h_savitzky[i];
         }
       for (int i=0; i<nbins/2; i++)
         {
           int iskip=i+nbins/2;
           h_savitzky[i]=tmp_h_savitzky[iskip+1];
         }
       delete [] tmp_h_savitzky;
     }

// ---------------------------------------------------------------------
// Method savitzky_smooth takes in array func[] of size nbins which
// contains some noisy function with one or more peaks.  This method
// applies a Savitzky-Golay filter to this noisy function.  It returns
// the smoothed function values within output array filtered_func[].

   void savitzky_smooth
     (int nbins,double func[],double filtered_func[],
      int deriv_order,int poly_order,int n_filter_points)
     {
       double *h_savitzky=new double [nbins];
       savitzky_golay_filter
         (nbins,h_savitzky,deriv_order,poly_order,n_filter_points);

       brute_force_filter(nbins,nbins,func,h_savitzky,
                          filtered_func,false);
       delete [] h_savitzky;
     }

} // filterfunc namespace





