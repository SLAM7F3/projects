// ========================================================================
// Program PLANAR_TRANS is a specialized tool meant for one-time use
// to determine the UV scalings and translations necessary to match
// SPASE PNG images onto the canonical SPASE 3D model.  It performs a
// least-squares fit to the linear functions which map uv points
// manually selected in Hyrum's 9 SPASE ISAR images onto their 3D
// counterpoints within the model.  We have hardwired the results
// within program IMAGEPLANE for the SPASE case.
// ========================================================================
// Last updated on 3/28/06
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "math/mypolynomial.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "math/twovector.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

// ========================================================================
int main( int argc, char** argv )
{

   int imagenumber=-1;
   cout << "Enter SPASE ISAR image number (0-8)" << endl;
   cin >> imagenumber;
   string imagenumber_str=stringfunc::number_to_string(imagenumber);
   
   string features2D_filename="image"+imagenumber_str+"_2Dfeatures.txt";
   string features3D_filename="image"+imagenumber_str+"_3Dfeatures.txt";
   cout << "features2D_filename = " << features2D_filename << endl;
   cout << "features3D_filename = " << features3D_filename << endl;
   
   vector<double> u,v,U,V;

   filefunc::ReadInfile(features2D_filename);
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      string curr_line=filefunc::text_line[i];
      vector<double> number=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      u.push_back(number[3]);
      v.push_back(number[4]);
   }

   filefunc::ReadInfile(features3D_filename);
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      string curr_line=filefunc::text_line[i];
      vector<double> number=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      U.push_back(number[3]);
      V.push_back(number[4]);
   }

   double chisq;
   mypolynomial u_poly(1),v_poly(1);
   u_poly.fit_coeffs(u,U,chisq);
   cout << "u_poly = " << u_poly << " chisq = " << chisq << endl;
   v_poly.fit_coeffs(v,V,chisq);
   cout << "v_poly = " << v_poly << " chisq = " << chisq << endl;

   vector<double> u_residual=u_poly.compute_residuals(u,U);
   vector<double> v_residual=v_poly.compute_residuals(v,V);
   for (int i=0; i<u.size(); i++)
   {
      cout << "i = " << i << " u_residual = " << u_residual[i]
           << " v_residual = " << v_residual[i] << endl;
   }

}

