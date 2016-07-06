// ==========================================================================
// Program GAMMA starts to simulate 32x32 APD array sampling of road
// ahead of MIT robot.
// ==========================================================================
// Last updated on 11/29/06
// ==========================================================================

#include <iostream>
#include <math.h>
#include <string>
#include <vector>
#include "math/constants.h"
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/twovector.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
   cout.precision(15);

   double h=2.5;	// meters

//   double dC=4.5;		// meter
   double dC=10;		// meter
//   cout << "Enter worst downrange dC value:" << endl;
//   cin >> dC;

   const double alpha=7.125*PI/180;	// rads

   double gamma_lo=0*PI/180;
   double gamma_hi=alpha;
   int nbins=1000;
   double d_gamma=(gamma_hi-gamma_lo)/(nbins-1);
   
   double best_abs_difference=POSITIVEINFINITY;
   double best_gamma;
   for (int n=0; n<nbins; n++)
   {
      double gamma=gamma_lo+n*d_gamma;

      double beta=alpha-gamma;
      double delta=PI-gamma/32.0-beta;
      
      double B=h/sin(beta);
      double term1=sin(delta)/B;
      double term2=sin(gamma/32.0)/dC;
      double abs_difference=fabs(term1-term2);

      if (abs_difference < best_abs_difference)
      {
         best_abs_difference=abs_difference;
         best_gamma=gamma;
      }
   }

   double gamma=best_gamma;
   double beta=alpha-gamma;
   double delta=PI-gamma/32.0-beta;
   double B=h/sin(beta);

   cout.precision(5);
   
   cout << "32x32 array height above ground = " << h << " meters" << endl;
   cout << "Worst instantaneous down range resolution = " 
        << dC << " meters" << endl;

   cout << "best_abs_difference = " << best_abs_difference << endl;
   cout << "Best gamma = " << gamma*180/PI << endl;

   cout << "alpha = " << alpha*180/PI << endl;
   cout << "beta = " << beta*180/PI << endl;
   cout << "delta = " << delta*180/PI << endl;
   cout << "B = " << B << " meters" << endl;

   vector<double> D;
   for (int n=0; n<=32; n++)
   {
      double gamma_n=n*gamma/32.0;
      double theta=PI/2-alpha+gamma_n;
      D.push_back(h*tan(theta));
   }

// Compute location of ground sample points relative to car's
// instantaneous world-space position:
   
   vector<twovector> groundpoint;
   for (int n=1; n<=32; n++)
   {
      cout << "n = " << n 
           << " D = " << D[n]
           << " delta_D = " << D[n]-D[n-1] << endl;

// Assumed lateral angular field-of-view = 22.9 degs = 0.4 rads --> 40
// meter cross range coverage at a range D = 100 meters.

      const double Delta_theta=0.4;	// radians
      double theta_lo=-Delta_theta/2.0;
      double theta_hi=Delta_theta/2.0;
      int ncolumns=32;
      double dtheta=(Delta_theta)/(ncolumns-1);
      for (int i=0; i<32; i++)
      {
         double theta=theta_lo+i*dtheta;
         double x=D[n]*tan(theta);
//         cout << "    i = " << i << " x = " << x << endl;
         groundpoint.push_back(twovector(x,D[n]));
      }
   } // loop over index n labeling APD rows

   exit(-1);

// Write ground sample lattice points to output text file:

   string filename="time0_groundpoints.txt";
   ofstream outstream;
   filefunc::openfile(filename,outstream);
   for (int j=0; j<groundpoint.size(); j++)
   {
      twovector curr_groundpoint=groundpoint[j];
      outstream << curr_groundpoint.get(0) << "    " 
                << curr_groundpoint.get(1)
                << endl;
   }
   filefunc::closefile(filename,outstream);
   

}

