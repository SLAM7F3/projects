// ========================================================================
// Program HORIZON imports a set of 2D feature pixel coordinates
// (px,py) that were manually selected via GIMP from a Wisp-360
// panorama.  The pixels all lie along the horizon separating sky from
// sea.  HORIZION performs a brute-force sinusoidal fit of the form

//		py = py_avg + A sin[ (2*PI/lambda)*px + phi_0 ]

// for parameters A, phi_0 and py_avg.  

//				./horizon

// ========================================================================
// Last updated on 3/15/13; 3/16/13; 3/22/13
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "math/mathfuncs.h"
#include "plot/metafile.h"
#include "numerical/param_range.h"
#include "general/stringfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::string;
using std::vector;

// ==========================================================================
int main( int argc, char** argv )
{

   double lambda=40000;
   vector<double> theta,py;

//   string horizon_filename="horizon.features";
   string horizon_filename="horizon_000.features";
//   string horizon_filename="horizon_420.features";
   filefunc::ReadInfile(horizon_filename);

//   double py_min=POSITIVEINFINITY;
//   double py_max=NEGATIVEINFINITY;
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      double curr_px=column_values[0];
      double curr_theta=2*PI/lambda*curr_px;
//      curr_theta *= 180/PI;
      double curr_py=column_values[1];

      theta.push_back(curr_theta);
      py.push_back(curr_py);

//      py_min=basic_math::min(py_min,curr_py);
//      py_max=basic_math::max(py_max,curr_py);

      cout << "i = " << i 
//           << " theta = " << theta.back()*180/PI
           << " theta = " << theta.back()
           << " py = " << py.back() << endl;
   }

   double delta_theta=theta.back()-theta.front();
   cout << "delta_theta = " << delta_theta*180/PI << endl;
   double delta_frac=delta_theta/(2*PI);
   cout << "delta_frac = " << delta_frac << endl;

/*
   string meta_filename="swaths";
   string title="swath corner vs azimuthal angle";
   string x_label="Azimuthal angle";
   string y_label="py for swath corner";
   double theta_min=0;
   double theta_max=360;
   metafile meta;
   meta.set_parameters(
      meta_filename,title,x_label,y_label,theta_min,theta_max,py_min,py_max);
   meta.openmetafile();
   meta.write_header();
   meta.write_curve(theta,py);
   meta.closemetafile();
*/

   double Astart=0;
   double Astop=400;
   param_range A(Astart,Astop,10);

   double phi_start=0;
   double phi_stop=2*PI;
   param_range phi(phi_start,phi_stop,60);

   double py_avg_start=1100-100;
   double py_avg_stop=1100+100;
   param_range py_avg(py_avg_start,py_avg_stop,10);

   double min_residual_sum=POSITIVEINFINITY;

//   int n_iters=1;
//   int n_iters=2;
//   int n_iters=5;
//   int n_iters=8;
//   int n_iters=10;
   int n_iters=500;

   cout << "Enter number of search iterations to perform:" << endl;
   cin >> n_iters;
   for (int iter=0; iter<n_iters; iter++)
   {
      cout << "iter = " << iter << " of " << n_iters << endl;

// ========================================================================
// Begin while loop over camera position parameters
// ========================================================================

      while (A.prepare_next_value())
      {
         while (phi.prepare_next_value())
         {
            while (py_avg.prepare_next_value())
            {

               double curr_residual_sum=0;
               for (unsigned int i=0; i<theta.size(); i++)
               {
                  double curr_theta=theta[i];
                  double measured_py=py[i];
                  
                  double fitted_py=py_avg.get_value()+
                     A.get_value()*sin(curr_theta+phi.get_value());
                  curr_residual_sum += fabs(measured_py-fitted_py);
               }

               if (curr_residual_sum < min_residual_sum)
               {
                  min_residual_sum=curr_residual_sum;
                  A.set_best_value();
                  phi.set_best_value();
                  py_avg.set_best_value();
//                  cout << "min_residual_sum = " << min_residual_sum << endl;
               }

            } // py_avg while loop
         } // phi while loop
      } // A while loop

//      cout << "******************************************************" << endl;
//      cout << "min_residual_sum = " << min_residual_sum << endl;
//      cout << "******************************************************" << endl;

// ========================================================================
// End while loop over camera position parameters
// ========================================================================

//      double frac=0.45;
//      double frac=0.55;
      double frac=0.90;
      A.shrink_search_interval(A.get_best_value(),frac);
      phi.shrink_search_interval(phi.get_best_value(),frac);
      py_avg.shrink_search_interval(py_avg.get_best_value(),frac);

//      outputfunc::enter_continue_char();

   } // loop over iter index

   cout << "Number of input theta,py horizon pairs = " << theta.size()
        << endl;
   cout << "Average residual = " << min_residual_sum/theta.size() 
        << " pixels" << endl << endl;

   cout << "Best A value = " << A.get_best_value() << " pixels " << endl;
   cout << "Best phi value = " << phi.get_best_value()*180/PI 
        << " degs" << endl;
   cout << "Best py_avg value = " << py_avg.get_best_value() << " pixels"
        << endl;

/*

Number of input theta,py horizon pairs = 25
Average residual = 0.246382 pixels

Best A value = 10.1289 pixels 
Best phi value = 309.848 degs
Best py_avg value = 1114.04 pixels

*/


/*

For wisp_res0_00000.jpg:

Number of input theta,py horizon pairs = 29
Average residual = 1.15102 pixels

Best A value = -13.4095 pixels 
Best phi value = 138.269 degs
Best py_avg value = 1117.31 pixels



For wisp_res0_00420.jpg:

Number of input theta,py horizon pairs = 28
Average residual = 1.608 pixels

Best A value = 343.331 pixels 
Best phi value = 156.858 degs
Best py_avg value = 1118.98 pixels

*/


}
