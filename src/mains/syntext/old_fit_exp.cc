// ==========================================================================
// Program FIT_EXP
// ==========================================================================
// Last updated on 1/30/16
// ==========================================================================

#include <iostream>
#include <math.h>
#include <set>
#include <string>
#include <vector>

#include "color/colorfuncs.h"
#include "general/filefuncs.h"
#include "math/mypolynomial.h"
#include "numrec/nrfuncs.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::set;
using std::string;
using std::vector;

// --------------------------------------------------------------------------
int main (int argc, char* argv[])
{
   std::set_new_handler(sysfunc::out_of_memory);

   string input_filename="decay.dat";
   filefunc::ReadInfile(input_filename);

   const double min_x = 23.0;
   vector<double> X, Xprime, Y, logY;
   for(unsigned int i = 0; i < filefunc::text_line.size(); i++)
   {
      double curr_x, curr_y;
      stringfunc::string_to_two_numbers(filefunc::text_line[i],curr_x,curr_y);

      double log_y = log(curr_y);
      cout << i << " " << curr_x << " " << curr_y 
           << " " << log_y << " exp(log_y) = " << exp(log_y) << endl;

      X.push_back(curr_x);
      Xprime.push_back(curr_x - min_x);
      Y.push_back(curr_y);
      logY.push_back(log_y);
   }

   double chisq;
   mypolynomial line_poly(1);
   line_poly.fit_coeffs(Xprime, logY, chisq);
   cout << "Fitted line_poly = " << line_poly << endl;
   cout << "chisq = " << chisq << endl;

   double log_alpha = line_poly.get_coeff(0);
   double alpha = exp(log_alpha);
   double beta = line_poly.get_coeff(1);

   cout << "log_alpha = " << log_alpha << " alpha = " << alpha << endl;
   cout << "beta = " << beta << endl;

   for(unsigned int i = 0; i < X.size(); i++)
   {
      double curr_xprime = X[i] - min_x;
      double curr_fitted_y = alpha * exp(beta * curr_xprime);

/*      
      cout << "x = " << X[i]
           << " y = " << Y[i]
           << " fitted y = " << curr_fitted_y 
           << endl;
*/

      cout << X[i] << "  " << curr_fitted_y << endl;
   }
   
   double lambda = 0.111556;
   double min_width = 27;
   double max_width = 64;
   double curr_width;

   int n_iters = 20000;
   vector<double> generated_widths;
   for(int iter = 0; iter < n_iters; iter++)
   {
      curr_width = min_width + nrfunc::expdev(lambda);
      generated_widths.push_back(curr_width);
   }      

   prob_distribution prob_widths(generated_widths, 200);
//   prob_distribution prob_widths(generated_widths, 500);
   prob_widths.set_xmin(0);
   prob_widths.set_xmax(100);
   prob_widths.set_xtic(20);
   prob_widths.set_xsubtic(10);
   prob_widths.set_color(colorfunc::red);
   prob_widths.writeprobdists(false,true);

   string unix_cmd = "cat prob_cum.meta xdim_cumulative.meta > total.meta";
   sysfunc::unix_command(unix_cmd);
   unix_cmd="meta_to_jpeg total";
   sysfunc::unix_command(unix_cmd);

}

