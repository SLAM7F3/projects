// ==========================================================================
// Program REGISTER is a simple simulation tool which we put together
// to assess the benefit of fitting an model's edge to a string of
// noisy points compared to fitting just the two endpoints.  The
// number of pixels along the line segment is taken to be nbins=100
// which approximately matches the number of pixels between KLT
// features in the Clewiston data set (i.e. ballpark 1000 features in
// a (3 km)**2 = (5000 pixel)**2 orthorectified image).  All
// continuous parameters (e.g. sigma) are measured in pixel-sized
// units.  We then find that setting sigma=0.555 yields "catastrophic"
// false alarm events in 1% of all trial simulations.  This rate
// approximately matches the rate of "catastrophic" false alarm
// explosions in the Group 99 PSS as of November 2005.  

// Upshot: By fitting an entire linesegment to 100 pixels rather than
// to just its two endpoints, the fitted line's deviation away from
// truth decreases by 1/sqrt(100).  The false alarm explosion rate
// then diminishes exponentially and becomes negligibly small in this
// idealized situation.

// ==========================================================================
// Last updated on 12/11/05
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "image/binaryimagefuncs.h"
#include "geometry/contour.h"
#include "geometry/convexhull.h"
#include "image/drawfuncs.h"
#include "ladar/featurefuncs.h"
#include "general/filefuncs.h"
#include "geometry/geometry_funcs.h"
#include "image/graphicsfuncs.h"
#include "image/imagefuncs.h"
#include "ladar/ladarfuncs.h"
#include "datastructures/Linkedlist.h"
#include "math/mypolynomial.h"
#include "image/myimage.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "image/recursivefuncs.h"
#include "geometry/regular_polygon.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "geometry/triangulate_funcs.h"
#include "image/TwoDarray.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::ios;
   using std::ofstream;
   using std::ostream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

   const int n_iters=100000;
//   const int n_iters=1000000;
//   const int n_iters=1000000;
   
   const int nbins=100;
//   const double sigma=1;
//   const double sigma=0.555;

   double sigma;
   cout << "Enter sigma:" << endl;
   cin >> sigma;

   int n_catastrophes=0;
   int n_model_catastrophes=0;
   int n_strict_model_catastrophes=0;
   double chisq;
   mypolynomial poly(1);

   vector<double> x;
   for (int n=0; n<nbins; n++)
   {
      x.push_back(n);
   }
      
   for (int iter=0; iter<n_iters; iter++)
   {
      if (iter%10000==0) cout << iter/10000 << " " << flush;
      vector<double> y;
      double y_avg=0;
      for (int n=0; n<nbins; n++)
      {
         double curr_y=sigma*nrfunc::gasdev();
         y_avg += curr_y;
         y.push_back(curr_y);
//         cout << "bin = " << n << " y = " << y.back() << endl;
      } // loop over index n labeling bins
      y_avg /= double(nbins);

// Fit line between y[0] and y[nbins-1].  Then compute how many of the
// fitted bins lie more than 1 vertical unit away from the horizontal
// axis (which is supposed to represent some edge):

      int n_false_alarms=0;
      for (int n=0; n<nbins; n++)
      {
         double curr_y=y[0]+n*(y[nbins-1]-y[0])/(nbins-1-0);
         if (fabs(curr_y) > 1)
         {
            n_false_alarms++;
         }
      } // loop over index n labeling bins

/*
      if (n_false_alarms > 0)
      {
         cout << "iter = " << iter << " n_false_alarms = " << n_false_alarms
              << " y_avg = " << y_avg << endl;
      }
//      cout << endl;
*/

      int n_fitted_false_alarms=0;
      if (poly.fit_coeffs(x,y,chisq))
      {
         for (int n=0; n<nbins; n++)
         {
            double curr_y=poly.value(x[n]);
            if (fabs(curr_y) > 1)
            {
               n_fitted_false_alarms++;
            }
         }
      }

      cout << "n_fitted_false_alarms = " 
           << n_fitted_false_alarms << endl;

      if (n_false_alarms > 0.5*nbins) n_catastrophes++;
      if (n_fitted_false_alarms > 0) n_model_catastrophes++;
//      if (n_fitted_false_alarms > 0.5*nbins) n_model_catastrophes++;
      if (fabs(y_avg) > 1) n_strict_model_catastrophes++;
   } // loop over iter index

   cout << "Percentage of catastrophic false alarm events = "
        << double(n_catastrophes)/double(n_iters)*100 << "%" << endl;
   cout << "Percentage of catastrophic model false alarm events = "
        << double(n_model_catastrophes)/double(n_iters)*100 << "%" << endl;
   cout << "Percentage of catastrophic strict model false alarm events = "
        << double(n_strict_model_catastrophes)/double(n_iters)*100 << "%" 
        << endl;
   
}
