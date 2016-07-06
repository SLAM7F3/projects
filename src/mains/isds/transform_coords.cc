// ==========================================================================
// Program TRANSFORM_COORDS performs a least squares minimization of a
// chisq function that quantifies the discrepancy between measured and
// transformed UTM feature coordinates for the Boston 2005 ALIRT data
// set.  We determine the linear transformation of the relative X and
// Y coordinates of the manually selected 2D features of the form UV =
// M XY + T where M is a 2x2 matrix close to the identity, T is the
// translation 2-vector, XY holds the relative ALIRT X and Y
// coordinates, and UV holds the coordinates in UTM space.  We
// evaluate the closed-form solution for the 4 parameters in M and the
// 2 translations in T.

// We also determine the best quadratic transformation of the X and Y
// coordinates of the form 

// 	            U^i = A^i_jk x^j x^k + B^i_j x^j + C^i 

// which depends only linearly upon the a priori unknown 12 parameters
// in tensors A, B and C.  Since this global quadratic transformation
// has more degrees of freedom than its linear counterpart, the RMS
// residual of the former is (not surprisingly) smaller than that of
// the latter.

// ==========================================================================
// Last updated on 2/5/08; 5/16/08; 4/4/12
// ==========================================================================

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "math/fourvector.h"
#include "math/genmatrix.h"
#include "astro_geo/geopoint.h"
#include "astro_geo/latlong2utmfuncs.h"
#include "geometry/linesegment.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/threevector.h"
#include "math/twovector.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ios;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Method declarations:

twovector avg_trans(const vector<twovector>& XY,const vector<twovector>& UV);
void quadratically_warp_bbox(
   const tensor* A_ptr,const tensor* B_ptr, const tensor* C_ptr,
   const twovector& avg_trans);
void linearly_warp_bbox(const genmatrix* M_ptr,const twovector& trans,
                        const twovector& avg_trans);
void fit_quadratic_warp(
   const vector<twovector>& XY,const vector<twovector>& UV,
   tensor*& A_ptr,tensor*& B_ptr,tensor*& C_ptr);
void fit_linear_warp(
   const vector<twovector>& XY,const vector<twovector>& UV,
   genmatrix* M_ptr,twovector& trans);
twovector quadratic_transformation(
   const tensor* A_ptr,const tensor* B_ptr, const tensor* C_ptr,
   const twovector& X);
double compare_measured_and_quadratically_transformed_UVs(
   const vector<twovector>& XY,const vector<twovector>& UV,
   const vector<int>& ID,
   const tensor* A_ptr,const tensor* B_ptr, const tensor* C_ptr);
double compare_measured_and_linearly_transformed_UVs(
   const vector<twovector>& XY,const vector<twovector>& UV,
   const vector<int>& ID,
   const genmatrix* M_ptr,const twovector& trans);

// ---------------------------------------------------------------------
// Method avg_translation computes the average translation needed to
// bring XY points to overlap with UV points:

twovector avg_trans(const vector<twovector>& XY,const vector<twovector>& UV)
{
   int n_pairs=XY.size();
   twovector avg_trans(0,0);
   for (int i=0; i<n_pairs; i++)
   {
      avg_trans += (UV[i] - XY[i])/double(n_pairs);
   }
   return avg_trans;
}

// ---------------------------------------------------------------------
// Method quadratically_warp_bbox takes in best linear fit tensor
// parameters *A_ptr, B_ptr and C_ptr and uses them to map the 14 km x
// 5 km bounding box surrounding the 2005 Boston ALIRT image.  This
// method writes out the new warped bbox with respect to the original
// grid's midpoint.  The output format can be read in as a set of OSG
// linesegments via program VIEWCITIES.

void quadratically_warp_bbox(
   const tensor* A_ptr,const tensor* B_ptr,const tensor* C_ptr,
   const twovector& avg_trans)
{
   twovector grid_origin(-35791.3,-13642.5);
   double grid_z=-137.21;

   const double bbox_x_size=14000;	// m
   const double bbox_y_size=5000;	// m
   vector<twovector> bbox_corner,XY,UV;

   bbox_corner.push_back(grid_origin);
   bbox_corner.push_back(grid_origin+twovector(bbox_x_size,0));
   bbox_corner.push_back(grid_origin+twovector(bbox_x_size,bbox_y_size));
   bbox_corner.push_back(grid_origin+twovector(0,bbox_y_size));
   
   double ds=100;	// meters
   for (int corner=0; corner<4; corner++)
   {
      threevector C1(bbox_corner[corner]);
      threevector C2(bbox_corner[modulo(corner+1,4)]);
      linesegment l(C1,C2);
      int nbins=l.get_length()/ds;
      for (int n=0; n<nbins; n++)
      {
         XY.push_back( twovector(C1+n*ds*l.get_ehat()) );
      }
   } // loop over corner index

   vector<threevector> delta_UV;
   for (unsigned int i=0; i<XY.size(); i++)
   {
      delta_UV.push_back(threevector(
         quadratic_transformation(A_ptr,B_ptr,C_ptr,XY[i])-avg_trans,grid_z));
   }

   string warped_bbox_filename="quadratic_bbox.txt";
   ofstream outstream;
   outstream.precision(10);

   filefunc::openfile(warped_bbox_filename,outstream);
   for (unsigned int i=0; i<delta_UV.size(); i++)
   {
      int j=modulo(i+1,delta_UV.size());
      outstream << "0 " << i << " 0 " 
                << delta_UV[i].get(0) << " " << delta_UV[i].get(1) << " "
                << delta_UV[i].get(2) << " " << delta_UV[j].get(0) << " "
                << delta_UV[j].get(1) << " " << delta_UV[j].get(2) << " 11" 
                << endl;
   }
   filefunc::closefile(warped_bbox_filename,outstream);
}

// ---------------------------------------------------------------------
// Method linearly_warp_bbox takes in best linear fit parameters
// *M_ptr and trans and uses them to map the corners of the 14 km x 5
// km grid surrounding the 2005 Boston ALIRT image.  This method
// writes out the new corner points of the warped quadrilateral with
// respect to the original grid's midpoint.  The output format can be
// read in as a set of OSG linesegments via program VIEWCITIES.

void linearly_warp_bbox(const genmatrix* M_ptr,const twovector& trans,
                        const twovector& avg_trans)
{
   twovector grid_origin(-35791.3,-13642.5);
   double grid_z=-137.21;

   const double bbox_x_size=14000;	// m
   const double bbox_y_size=5000;	// m
   vector<twovector> XY,UV;

   XY.push_back(grid_origin);
   XY.push_back(grid_origin+twovector(bbox_x_size,0));
   XY.push_back(grid_origin+twovector(bbox_x_size,bbox_y_size));
   XY.push_back(grid_origin+twovector(0,bbox_y_size));
   
   vector<threevector> delta_UV;
   for (unsigned int i=0; i<XY.size(); i++)
   {
      delta_UV.push_back(threevector(*M_ptr*XY[i]+trans-avg_trans,grid_z));
   }
   
   for (unsigned int i=0; i<delta_UV.size(); i++)
   {
      int j=modulo(i+1,delta_UV.size());
      cout << "0 " << i << " 0 " 
           << delta_UV[i].get(0) << " " << delta_UV[i].get(1) << " "
           << delta_UV[i].get(2) << " " << delta_UV[j].get(0) << " "
           << delta_UV[j].get(1) << " " << delta_UV[j].get(2) << " 11" 
           << endl;
   }
}

// ---------------------------------------------------------------------
// Method fit_quadratic_warp implements the closed-form least squares
// solution to the system U^i = A^i_jk x^j x^k + B^i_j x^j + C^i which
// depends linearly upon the a priori unknown tensors a, b and c but
// quadratically upon the input X and Y locations.  This function
// returns best fit values for the 6 entries in tensor A, 4 entries in
// matrix B and 2 entries in vector C.

void fit_quadratic_warp(
   const vector<twovector>& XY,const vector<twovector>& UV,
   tensor*& A_ptr,tensor*& B_ptr,tensor*& C_ptr)
{
   tensor U(2);
   genmatrix V(2,2);
   tensor W(2,2,2);
   tensor X(2);
   genmatrix S(2,2);
   tensor T(2,2,2);
   tensor Q(2,2,2,2);

   U.clear_values();
   V.clear_values();
   W.clear_values();
   X.clear_values();
   S.clear_values();
   T.clear_values();
   Q.clear_values();

   int N=XY.size();
   for (unsigned int i=0; i<N; i++)
   {
      double x=XY[i].get(0);
      double y=XY[i].get(1);
      double u=UV[i].get(0);
      double v=UV[i].get(1);

      U.increment(0,u);
      U.increment(1,v);

      V.increment(0,0,u*x);
      V.increment(0,1,u*y);
      V.increment(1,0,v*x);
      V.increment(1,1,v*y);
      
      W.increment(0,0,0,u*x*x);
      W.increment(0,0,1,u*x*y);
      W.increment(0,1,0,u*y*x);
      W.increment(0,1,1,u*y*y);
      W.increment(1,0,0,v*x*x);
      W.increment(1,0,1,v*x*y);
      W.increment(1,1,0,v*y*x);
      W.increment(1,1,1,v*y*y);

      X.increment(0,x);
      X.increment(1,y);
      
      S.increment(0,0,x*x);
      S.increment(0,1,x*y);
      S.increment(1,0,y*x);
      S.increment(1,1,y*y);
      
      T.increment(0,0,0,x*x*x);
      T.increment(0,0,1,x*x*y);
      T.increment(0,1,0,x*y*x);
      T.increment(0,1,1,x*y*y);
      T.increment(1,0,0,y*x*x);
      T.increment(1,0,1,y*x*y);
      T.increment(1,1,0,y*y*x);
      T.increment(1,1,1,y*y*y);

      Q.increment(0,0,0,0,x*x*x*x);
      Q.increment(0,0,0,1,x*x*x*y);
      Q.increment(0,0,1,0,x*x*y*x);
      Q.increment(0,0,1,1,x*x*y*y);
      Q.increment(0,1,0,0,x*y*x*x);
      Q.increment(0,1,0,1,x*y*x*y);
      Q.increment(0,1,1,0,x*y*y*x);
      Q.increment(0,1,1,1,x*y*y*y);
      Q.increment(1,0,0,0,y*x*x*x);
      Q.increment(1,0,0,1,y*x*x*y);
      Q.increment(1,0,1,0,y*x*y*x);
      Q.increment(1,0,1,1,y*x*y*y);
      Q.increment(1,1,0,0,y*y*x*x);
      Q.increment(1,1,0,1,y*y*x*y);
      Q.increment(1,1,1,0,y*y*y*x);
      Q.increment(1,1,1,1,y*y*y*y);
   }

//   cout << "U = " << U << endl;
//   cout << "V = " << V << endl;
//   cout << "W = " << W << endl;

//   cout << "X = " << X << endl;
//   cout << "S = " << S << endl;
//   cout << "T = " << T << endl;
//   cout << "Q = " << Q << endl;

   tensor E(2,2,2);
   genmatrix G(2,2),Ginv(2,2);
   genmatrix Y(2,2);

   E=T-1.0/N*S.outerproduct(X);
   G=S-1.0/N*genmatrix(X.outerproduct(X));
   Y=V-1.0/N*genmatrix(U.outerproduct(X));

   G.inverse(Ginv);
//   cout << "G = " << G << " Ginv = " << Ginv << " G*Ginv = " << G*Ginv
//        << endl;

   tensor F(2,2,2,2);
   tensor H(2,2,2);
   tensor Z(2,2,2);

   F=Q-1.0/N*S.outerproduct(S);
   H=T-1.0/N*X.outerproduct(S);
   Z=W-1.0/N*U.outerproduct(S);
 
//   cout << "F = " << F << endl;
//   cout << "H = " << H << endl;
//   cout << "Z = " << Z << endl;

   tensor EGinv=E.outerproduct(Ginv);
   tensor EGinvcontract=EGinv.contract(2,3);
   tensor EGinvcontractH=EGinvcontract.outerproduct(H);
   tensor EGinvH=EGinvcontractH.contract(2,3);
//   cout << "EGinvH = " << EGinvH << endl;

   tensor C(2,2,2,2);
   C=F-EGinvH;
//   cout << "C = " << C << endl;

   genmatrix Cmat(4,4),Cmatinv(4,4);
   Cmat.put(0,0,C.get(0,0,0,0));
   Cmat.put(0,1,C.get(0,1,0,0));
   Cmat.put(0,2,C.get(1,0,0,0));
   Cmat.put(0,3,C.get(1,1,0,0));

   Cmat.put(1,0,C.get(0,0,0,1));
   Cmat.put(1,1,C.get(0,1,0,1));
   Cmat.put(1,2,C.get(1,0,0,1));
   Cmat.put(1,3,C.get(1,1,0,1));

   Cmat.put(2,0,C.get(0,0,1,0));
   Cmat.put(2,1,C.get(0,1,1,0));
   Cmat.put(2,2,C.get(1,0,1,0));
   Cmat.put(2,3,C.get(1,1,1,0));

   Cmat.put(3,0,C.get(0,0,1,1));
   Cmat.put(3,1,C.get(0,1,1,1));
   Cmat.put(3,2,C.get(1,0,1,1));
   Cmat.put(3,3,C.get(1,1,1,1));

//   cout << "Cmat = " << Cmat << endl;
   genmatrix Umat(4,4),Wmat(4,4),Vmat(4,4);
   Cmat.sorted_singular_value_decomposition(Umat,Wmat,Vmat);
//   cout << "Umat = " << Umat << endl;
//   cout << "Umat * Umattrans = " << Umat*Umat.transpose() << endl;
//   cout << "Wmat = " << Wmat << endl;
//   cout << "Vmat = " << Vmat << endl;
//   cout << "Vmat * Vmattrans = " << Vmat * Vmat.transpose() << endl;

   genmatrix Cpseudo_inv(4,4);
   double min_abs_singular_value=0.01;
   Cmat.pseudo_inverse(0.1,Cpseudo_inv);
//      cout << "Cpseudo_inv = " << Cpseudo_inv << endl;
//      cout << "Cpseudo_inv * Cmat = " << Cpseudo_inv * Cmat << endl;
   
   tensor YGinv=Y.outerproduct(Ginv);
   tensor YGinvcontract=YGinv.contract(1,2);
   tensor YGinvcontractH=YGinvcontract.outerproduct(H);
   tensor YGinvH=YGinvcontractH.contract(1,2);
//   cout << "YGinvH = " << YGinvH << endl;

   tensor D(2,2,2);
   D=Z-YGinvH;
//   cout << "D = " << D << endl;

   fourvector D0,D1;
   D0.put(0,D.get(0,0,0));
   D0.put(1,D.get(0,0,1));
   D0.put(2,D.get(0,1,0));
   D0.put(3,D.get(0,1,1));

   D1.put(0,D.get(1,0,0));
   D1.put(1,D.get(1,0,1));
   D1.put(2,D.get(1,1,0));
   D1.put(3,D.get(1,1,1));
   
//   cout << "D0 = " << D0 << endl;
//   cout << "D1 = " << D1 << endl;

   fourvector PD0(Cpseudo_inv*D0);
   fourvector PD1(Cpseudo_inv*D1);
   
//   cout << "pseudoinv*D0 = " << PD0 << endl;
//   cout << "pseudoinv*D1 = " << PD1 << endl;
   
   A_ptr=new tensor(2,2,2);
   A_ptr->put(0,0,0,PD0.get(0));
   A_ptr->put(0,0,1,PD0.get(1));
   A_ptr->put(0,1,0,PD0.get(2));
   A_ptr->put(0,1,1,PD0.get(3));

   A_ptr->put(1,0,0,PD1.get(0));
   A_ptr->put(1,0,1,PD1.get(1));
   A_ptr->put(1,1,0,PD1.get(2));
   A_ptr->put(1,1,1,PD1.get(3));
   
//   cout << "*A_ptr = " << *A_ptr << endl;

   tensor AEGinvcontract=A_ptr->outerproduct(EGinvcontract);
   tensor AEGinv=AEGinvcontract.contract_adjacent_pairs(1);
   
   B_ptr=new tensor(YGinvcontract-AEGinv);
//   cout << "*B_ptr = " << *B_ptr << endl;

   tensor AS=A_ptr->outerproduct(S);
   tensor AScontract=AS.contract_adjacent_pairs(1);
//   cout << "AScontract = " << AScontract << endl;

   tensor BX=B_ptr->outerproduct(X);
   tensor BXcontract=BX.contract(1,2);
//   cout << "BX = " << BXcontract << endl;

   C_ptr=new tensor(1.0/N*(U-AScontract-BXcontract));
//   cout << "*C_ptr = " << *C_ptr << endl;
}

// ---------------------------------------------------------------------
// Method fit_linear_warp implements the closed-form least squares solution
// to the linear system UV = M XY + T for the 4 parameters in M and
// the 2 translations in T:

void fit_linear_warp(
   const vector<twovector>& XY,const vector<twovector>& UV,
   genmatrix* M_ptr,twovector& trans)
{
   double X,Y,U,V;
   double Asq,Bsq,AB,AC,AD,BC,BD;
   X=Y=U=V=0;
   Asq=Bsq=AB=AC=AD=BC=BD=0;
   
   int N=XY.size();
   for (unsigned int i=0; i<N; i++)
   {
      double x=XY[i].get(0);
      double y=XY[i].get(1);
      double u=UV[i].get(0);
      double v=UV[i].get(1);
      
      X += x;
      Y += y;
      U += u;
      V += v;

      Asq += sqr(x);
      Bsq += sqr(y);
      AB += x*y;
      AC += x*u;
      AD += x*v;
      BC += y*u;
      BD += y*v;
   }
   
   double Isq=Asq-sqr(X)/N;
   double IJ=AB-X*Y/N;
   double Jsq=Bsq-sqr(Y)/N;
   double IK=AC-X*U/N;
   double JK=BC-Y*U/N;
   double IL=AD-X*V/N;
   double JL=BD-Y*V/N;
   
   double denom=Isq*Jsq-sqr(IJ);
   M_ptr->put(0,0,(Jsq*IK-IJ*JK)/denom);
   M_ptr->put(0,1,(-IJ*IK+Isq*JK)/denom);
   M_ptr->put(1,0,(Jsq*IL-IJ*JL)/denom);
   M_ptr->put(1,1,(-IJ*IL+Isq*JL)/denom);

   trans.put(0,(U-M_ptr->get(0,0)*X-M_ptr->get(0,1)*Y)/N);
   trans.put(1,(V-M_ptr->get(1,0)*X-M_ptr->get(1,1)*Y)/N);
}

// ---------------------------------------------------------------------
// Method quadratic_transformation takes in the fitted entries in
// tensor *A_ptr, B_ptr and C_ptr. 

twovector quadratic_transformation(
   const tensor* A_ptr,const tensor* B_ptr, const tensor* C_ptr,
   const twovector& X)
{
   tensor AXX( (A_ptr->outerproduct(X,X)). contract_adjacent_pairs(1) );
   tensor BX(  (B_ptr->outerproduct(X)).contract(1,2)  );
   return twovector(AXX+BX+*C_ptr);
}

// ---------------------------------------------------------------------
// Method compare_measured_and_quadratically_transformed_UVs takes in
// the fitted entries in tensor *A_ptr, B_ptr and C_ptr.  It then
// computes the differences between the fitted and manually measured
// UV points.  This method returns the RMS residual between the fitted
// and measured positions which provides a quantitative measure of the
// goodness-of-fit.

double compare_measured_and_quadratically_transformed_UVs(
   const vector<twovector>& XY,const vector<twovector>& UV,
   const vector<int>& ID,
   const tensor* A_ptr,const tensor* B_ptr, const tensor* C_ptr)
{
   string banner="Comparing measured and quadratically transformed UVs";
   outputfunc::write_big_banner(banner);

   const int N=XY.size();
   vector<double> d_chisq,delta_mag;
   d_chisq.reserve(N);
   delta_mag.reserve(N);
   for (unsigned int i=0; i<N; i++)
   {
      twovector UV_transformed(quadratic_transformation(
         A_ptr,B_ptr,C_ptr,XY[i]));
      twovector Delta(UV[i] - UV_transformed);
      d_chisq.push_back(Delta.sqrd_magnitude());
      delta_mag.push_back(Delta.magnitude());

      cout << "i = " << i 
           << " ID = " << ID[i] 
           << " Utrans = " << UV_transformed.get(0)
           << " U = " << UV[i].get(0)
           << " Vtrans = " << UV_transformed.get(1)
           << " V = " << UV[i].get(1) 
           << " Delta.mag = " << delta_mag.back()
           << endl;
   } // loop over index i 
   cout << endl;

   double mean=mathfunc::mean(delta_mag);
   double std_dev=mathfunc::std_dev(delta_mag);
   cout << "Residual = " << mean << " +/- " << std_dev << endl;

   std::sort(d_chisq.begin(),d_chisq.end());
   
   int npoints_to_discard=0;
   double chisq=0;
   for (unsigned int i=0; i<N-npoints_to_discard; i++)
   {
      chisq += d_chisq[i];
   }

   double MS_residual=chisq/(N-npoints_to_discard);
   double RMS_residual=sqrt(MS_residual);
   return RMS_residual;
}

// ---------------------------------------------------------------------
// Method compare_measured_and_linearly_transformed_UVs takes in the
// fitted entries in matrix *M_ptr and vector trans.  It then computes
// the differences between the fitted and manually measured UV points.
// This method returns the RMS residual between the fitted and
// measured positions which provides a quantitative measure of the
// goodness-of-fit.

double compare_measured_and_linearly_transformed_UVs(
   const vector<twovector>& XY,const vector<twovector>& UV,
   const vector<int>& ID,const genmatrix* M_ptr,const twovector& trans)
{
   string banner="Comparing measured and linearly transformed UVs";
   outputfunc::write_big_banner(banner);
   
   const int N=XY.size();
   vector<double> d_chisq,delta_mag;
   d_chisq.reserve(N);
   delta_mag.reserve(N);
   for (unsigned int i=0; i<N; i++)
   {
      twovector UV_transformed( *M_ptr * XY[i] + trans );
      twovector Delta(UV[i] - UV_transformed);
      d_chisq.push_back(Delta.sqrd_magnitude());
      delta_mag.push_back(Delta.magnitude());

      cout << "i = " << i 
           << " ID = " << ID[i] 
           << " Utrans = " << UV_transformed.get(0)
           << " U = " << UV[i].get(0)
           << " Vtrans = " << UV_transformed.get(1)
           << " V = " << UV[i].get(1) 
           << " Delta.mag = " << delta_mag.back()
           << endl;
   } // loop over index i 
   cout << endl;

   double mean=mathfunc::mean(delta_mag);
   double std_dev=mathfunc::std_dev(delta_mag);
   cout << "Residual = " << mean << " +/- " << std_dev << endl;

   std::sort(d_chisq.begin(),d_chisq.end());
   
   int npoints_to_discard=0;
   double chisq=0;
   for (unsigned int i=0; i<N-npoints_to_discard; i++)
   {
      chisq += d_chisq[i];
   }

   double MS_residual=chisq/(N-npoints_to_discard);
   double RMS_residual=sqrt(MS_residual);
   return RMS_residual;
}

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
   cout.precision(10);

//   string filename="total_consolidated.txt";
   string filename="features_consolidated.txt";
   cout << "Enter name of file containing consolidated feature information:"
        << endl;
   cin >> filename;
   filefunc::ReadInfile(filename);
   
   vector<int> ID;
   vector<twovector> XY,UV;
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<string> substring=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);
      double X=stringfunc::string_to_number(substring[0]);
      double Y=stringfunc::string_to_number(substring[1]);
      double U=stringfunc::string_to_number(substring[2]);
      double V=stringfunc::string_to_number(substring[3]);
      if (substring.size() > 4)
      {
         ID.push_back(stringfunc::string_to_number(substring[4]));
      }
      else
      {
         ID.push_back(-1);
      }

      XY.push_back(twovector(X,Y));
      UV.push_back(twovector(U,V));
      cout << "i = " << i 
           << " X = " << XY.back().get(0) 
           << " Y = " << XY.back().get(1) 
           << " U = " << UV.back().get(0) 
           << " V = " << UV.back().get(1)
           << " ID = " << ID.back() 
           << endl;
   }

// Use closed form least squares solution to determine matrix and
// translation parameters:

   tensor *A_ptr,*B_ptr,*C_ptr;
   fit_quadratic_warp(XY,UV,A_ptr,B_ptr,C_ptr);

   cout << "*A_ptr = " << *A_ptr << endl;
   cout << "*B_ptr = " << *B_ptr << endl;
   cout << "*C_ptr = " << *C_ptr << endl;

   genmatrix* M_ptr=new genmatrix(2,2);
   genmatrix* Minv_ptr=new genmatrix(2,2);

   M_ptr->identity();
   twovector trans;
   fit_linear_warp(XY,UV,M_ptr,trans);

   cout << "*M_ptr = " << *M_ptr << endl;

   if (M_ptr->inverse(*Minv_ptr))
   {
      cout << "*Minv_ptr = " << *Minv_ptr << endl;
      cout << "M*Minv = " << (*M_ptr) * (*Minv_ptr) << endl;
   }

   cout << "trans = " << trans << endl;

   double quadratic_RMS_residual=
      compare_measured_and_quadratically_transformed_UVs(
      XY,UV,ID,A_ptr,B_ptr,C_ptr);
   double linear_RMS_residual=
      compare_measured_and_linearly_transformed_UVs(XY,UV,ID,M_ptr,trans);

   cout << "RMS residual for quadratic fit = " << quadratic_RMS_residual
        << endl << endl;
   
   cout << "RMS residual for linear fit = " << linear_RMS_residual
        << endl << endl;

// Compute naive translation needed to bring XY points to overlap with
// UV points:

   twovector naive_trans=avg_trans(XY,UV);
   cout << "Naive translation = " << naive_trans << endl;

// Compute video texture corner locations in easting & northing
// coordinates:

   double minU=0;
   double minV=0;
   double maxU=1;
   double maxV=1;
   cout << "Enter min_U:" << endl;
   cin >> minU;
   cout << "Enter max_U:" << endl;
   cin >> maxU;

   twovector screen_lower_left(minU,minV);
   twovector screen_lower_right(maxU,minV);
   twovector screen_upper_right(maxU,maxV);
   twovector screen_upper_left(minU,maxV);

   twovector geo_lower_left=(*Minv_ptr)*(screen_lower_left-trans);
   twovector geo_lower_right=(*Minv_ptr)*(screen_lower_right-trans);
   twovector geo_upper_right=(*Minv_ptr)*(screen_upper_right-trans);
   twovector geo_upper_left=(*Minv_ptr)*(screen_upper_left-trans);
   
   cout << "geo_lower_left = " << geo_lower_left << endl;
   cout << "geo_lower_right = " << geo_lower_right << endl;
   cout << "geo_upper_right = " << geo_upper_right << endl;
   cout << "geo_upper_left = " << geo_upper_left << endl;

   bool northern_hemisphere_flag=true;		
   int UTM_zonenumber=14;			// Lubbock, TX	
//   int UTM_zonenumber=19;			// Boston, MA

   geopoint lower_left_corner(
      northern_hemisphere_flag,UTM_zonenumber,
      geo_lower_left.get(0),geo_lower_left.get(1));
   geopoint lower_right_corner(
      northern_hemisphere_flag,UTM_zonenumber,
      geo_lower_right.get(0),geo_lower_right.get(1));
   geopoint upper_right_corner(
      northern_hemisphere_flag,UTM_zonenumber,
      geo_upper_right.get(0),geo_upper_right.get(1));
   geopoint upper_left_corner(
      northern_hemisphere_flag,UTM_zonenumber,
      geo_upper_left.get(0),geo_upper_left.get(1));

   cout << "lower_left_corner = " << lower_left_corner << endl;
   cout << "lower_right_corner = " << lower_right_corner << endl;
   cout << "upper_right_corner = " << upper_right_corner << endl;
   cout << "upper_left_corner = " << upper_left_corner << endl;

   quadratically_warp_bbox(A_ptr,B_ptr,C_ptr,naive_trans);
   linearly_warp_bbox(M_ptr,trans,naive_trans);

// Use linear transformation parameters to compute easting-northing
// coordinates corresponding to UV image plane corners:
   
   double min_U=0;
   double max_U=1; // true for Lubbock activity region #1 3600x3600 video

   double min_V=0;
   double max_V=1;

   vector<twovector> corners;
   corners.push_back(twovector(min_U,min_V));
   corners.push_back(twovector(max_U,min_V));
   corners.push_back(twovector(max_U,max_V));
   corners.push_back(twovector(min_U,max_V));
   
//   int UTM_zonenumber=14;			// Lubbock, TX	
//   bool northern_hemisphere_flag=true;		// Lubbock, TX

   for (int c=0; c<corners.size(); c++)
   {
      twovector curr_UV=corners[c];
      twovector curr_XY=(*Minv_ptr) * (curr_UV - trans);
      cout << "c = " << c << " curr_UV = " << curr_UV
           << " curr_XY = " << curr_XY << endl;

      double curr_longitude,curr_latitude;
      latlongfunc::UTMtoLL(
         UTM_zonenumber,northern_hemisphere_flag,
         curr_XY.get(1),curr_XY.get(0),curr_latitude,curr_longitude);
      cout << "longitude = " << curr_longitude
           << " latitude = " << curr_latitude << endl;

   }

/*
// Reconstruct M and trans given input and output corner twovectors:

   twovector Delta_screen_H=screen_lower_right-screen_lower_left;
   twovector Delta_screen_V=screen_upper_left-screen_lower_left;

   twovector Delta_geo_H=geo_lower_right-geo_lower_left;
   twovector Delta_geo_V=geo_upper_left-geo_lower_left;

   double alpha=Delta_geo_H.get(0);
   double beta=Delta_geo_H.get(1);
   double gamma=Delta_geo_V.get(0);
   double delta=Delta_geo_V.get(1);
   double determinant=alpha*delta-beta*gamma;
   
   double Mxx=(delta*Delta_screen_H-beta*Delta_screen_V).get(0)/determinant;
   double Myx=(delta*Delta_screen_H-beta*Delta_screen_V).get(1)/determinant;
   
   double Mxy=-(gamma*Delta_screen_H-alpha*Delta_screen_V).get(0)/determinant;
   double Myy=-(gamma*Delta_screen_H-alpha*Delta_screen_V).get(1)/determinant;

   cout << "Mxx = " << Mxx << " Mxy = " << Mxy << endl;
   cout << "Myx = " << Myx << " Myy = " << Myy << endl;

   genmatrix Mnew(2,2);
   Mnew.put(0,0,Mxx);
   Mnew.put(0,1,Mxy);
   Mnew.put(1,0,Myx);
   Mnew.put(1,1,Myy);
   
   cout << "Mnew = " << Mnew << endl;
   cout << "*M_ptr = " << *M_ptr << endl;
   cout << "Mnew - *M_ptr = " << Mnew - *M_ptr << endl;

   twovector trans_new=screen_lower_left-Mnew*geo_lower_left;
   cout << "trans = " << trans 
        << " trans_new = " << trans_new << endl;
*/

}
