// ==========================================================================
// Program AB is a testing grounds for alpha-beta filtering
// ==========================================================================
// Last updated on 3/21/07
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "filter/filterfuncs.h"
#include "math/mathfuncs.h"
#include "numrec/nrfuncs.h"

// ==========================================================================
int main (int argc, char* argv[])
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;

   const double dt=1;
   const double sigma=1;
   const int nbins=100;
   const double x_start=0;
   const double vx=1;
   const double x_stop=x_start+nbins*dt*vx;
   double dx=(x_stop-x_start)/(nbins-1);

   vector<double> x_true,x_raw;
   for (int n=0; n<nbins; n++)
   {
      double t=n*dt;
      x_true.push_back(x_start+vx*t);
      double f=1;
      double S2N=10;
      double noise=sigma*nrfunc::gasdev();
      x_raw.push_back(x_true.back()+f/sqrt(S2N)*noise);
   }

   double alpha,beta,theta;
   cout << "Enter theta:" << endl;
   cin >> theta;
//   cout << "Enter alpha:" << endl;
//   cin >> alpha;
//   cout << "Enter beta:" << endl;
//   cin >> beta;
   
   alpha=1-sqr(theta);
   beta=sqr(1-theta);
   
   double curr_filtered_x,curr_filtered_xdot;
   double prev_filtered_x=0;
   double prev_filtered_xdot=0;
   vector<double> x_filtered,xdot_filtered,delta_x;
   for (int n=0; n<nbins; n++)
   {
      double curr_raw_x=x_raw[n];
      filterfunc::alphabeta_filter(
         curr_raw_x,prev_filtered_x,prev_filtered_xdot,
         curr_filtered_x,curr_filtered_xdot,alpha,beta,dt);
      x_filtered.push_back(curr_filtered_x);
      xdot_filtered.push_back(curr_filtered_xdot);

      delta_x.push_back(x_filtered[n]-x_true[n]);

      cout << n 
           << " x_true = " << x_true[n] 
           << " x_raw-x_true = " << x_raw[n]-x_true[n]
           << " x_filtered-x_true = " << delta_x[n] << endl;
//           << " xdot_filtered = " << xdot_filtered[n] << endl;

      prev_filtered_x=curr_filtered_x;
      prev_filtered_xdot=curr_filtered_xdot;
      
   } // loop over index n

   
   double mean_delta_x=mathfunc::mean(delta_x);
   double std_dev_delta_x=mathfunc::std_dev(delta_x);

   cout << "delta_x = " << mean_delta_x << " +/- "
        << std_dev_delta_x << endl;

}
