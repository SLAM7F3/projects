// ==========================================================================
// Program AFFINE
// ==========================================================================
// Last updated on 7/26/13
// ==========================================================================

#include  <iostream>
#include  <string>
#include  <vector>

#include "geometry/affine_transform.h"
#include "math/threevector.h"
#include "math/twovector.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(12);

   vector<twovector> q_vecs,p_vecs;

//   q_vecs.push_back(twovector(0,0));
//   q_vecs.push_back(twovector(1,0));
//   q_vecs.push_back(twovector(0,1));

//   p_vecs.push_back(twovector(1,1));
//   p_vecs.push_back(twovector(2,1));
//   p_vecs.push_back(twovector(1,2));
   

   q_vecs.push_back(twovector(0.8893847466,0.5902867913)); 	// NE  
   q_vecs.push_back(twovector(0.8631272316,0.3356850147)); 	// SE
   q_vecs.push_back(twovector(0.6019337177,0.3675101995));   	// SW
   q_vecs.push_back(twovector(0.6420110464,0.6428675652));   	// NW

   p_vecs.push_back(twovector(25.39397778,23.99390857));	// NE
   p_vecs.push_back(twovector(24.70671364,-21.60018909));	// SE
   p_vecs.push_back(twovector(-23.54938968,-24.71375663));	// SW
   p_vecs.push_back(twovector(-23.8489854,30.19430996));	// NW

   affine_transform* affine_transform_ptr=new affine_transform();
   affine_transform_ptr->parse_affine_transform_inputs(q_vecs,p_vecs);
   affine_transform_ptr->fit_affine_transformation();
   double RMS_residual=
      affine_transform_ptr->check_affine_transformation(q_vecs,p_vecs);
   cout << "RMS_residual = " << RMS_residual << endl;

   delete affine_transform_ptr;


/*

   twovector trans;
   genmatrix A(2,2);
   double S=mathfunc::fit_2D_affine_transformation(q_vecs,p_vecs,A,trans);

   cout << "A = " << A << endl;
   cout << "trans = " << trans << endl;
   cout << "S = " << S << endl;

   double RMS=0;
   for (int i=0; i<q_vecs.size(); i++)
   {
      twovector p_calc=A*q_vecs[i]+trans;
      double sqr_delta=(p_calc-p_vecs[i]).sqrd_magnitude();
      RMS += sqr_delta;
   }
   RMS /= q_vecs.size();
   RMS=sqrt(RMS);
   cout << "RMS error = " << RMS << endl;
*/


/*
   double Umin=0;
   double Umax=1983.0/1299.0;
   double Vmin=0;
   double Vmax=1;
   q_vecs.clear();
   q_vecs.push_back(twovector(Umax,Vmax));
   q_vecs.push_back(twovector(Umax,Vmin));
   q_vecs.push_back(twovector(Umin,Vmin));
   q_vecs.push_back(twovector(Umin,Vmax));

   cout << "Umax = " << Umax << endl;
   
   for (int i=0; i<q_vecs.size(); i++)
   {
      twovector p_calc=A*q_vecs[i]+trans;
      cout << "i = " << i << " p_corner = " << p_calc << endl;
   }
*/


}

