// ==========================================================================
// Fundamental matrix class member functions
// ==========================================================================
// Last modified on 4/29/13; 5/23/13; 6/16/13; 7/2/13; 4/5/14
// ==========================================================================

#include <string>
#include <vector>
#include "video/camerafuncs.h"
#include "math/complexfuncs.h"
#include "general/filefuncs.h"
#include "structmotion/fundamental.h"
#include "math/genmatrix.h"
#include "math/genvector.h"
#include "geometry/geometry_funcs.h"
#include "geometry/homography.h"
#include "math/mathfuncs.h"
#include "math/mypolynomial.h"
#include "templates/mytemplates.h"
#include "numerical/newton.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "numerical/param_range.h"
#include "math/rotation.h"
#include "general/stringfuncs.h"

using std::cout;
using std::cin;
using std::endl;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void fundamental::initialize_member_objects()
{
   E_ptr=NULL;
   Ninverse_ptr=NULL;
   P_ptr=NULL;
   Pprime_ptr=NULL;
}		       

void fundamental::allocate_member_objects()
{
   A_ptr=new genmatrix(1,9);
   B_ptr=new genmatrix(1,9);

   T_XY_ptr=new genmatrix(3,3);
   T_UV_ptr=new genmatrix(3,3);

   T_XY_ptr->identity();
   T_UV_ptr->identity();
   
   F_ptr=new genmatrix(3,3);
   for (unsigned int f=0; f<3; f++)
   {
      F_ptrs.push_back(new genmatrix(3,3));
   }
   Fbest_ptr=new genmatrix(3,3);
}

fundamental::fundamental()
{
   allocate_member_objects();
   initialize_member_objects();
}

fundamental::~fundamental()
{
   delete A_ptr;
   delete B_ptr;
   delete T_XY_ptr;
   delete T_UV_ptr;
   delete F_ptr;
   for (unsigned int f=0; f<F_ptrs.size(); f++)
   {
      delete F_ptrs[f];
   }
   delete Fbest_ptr;

   delete E_ptr;
   delete Ninverse_ptr;
   delete P_ptr;
   delete Pprime_ptr;
   
   A_ptr=NULL;
   B_ptr=NULL;
   T_XY_ptr=NULL;
   T_UV_ptr=NULL;
   F_ptr=NULL;
   E_ptr=NULL;
   Ninverse_ptr=NULL;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,fundamental& f)
{
   f.renormalize_F_entries();
   
   outstream << endl;
   outstream << "*F_ptr = " << *(f.F_ptr) << endl;
   cout << "Det(F) = " << f.F_ptr->determinant() << endl;
   return outstream;
}

// ==========================================================================
// Least-squares determination of fundamental matrix member functions
// ==========================================================================
   
void fundamental::set_F_values(genmatrix& Finput)
{
   for (unsigned int i=0; i<3; i++)
   {
      for (unsigned int j=0; j<3; j++)
      {
         F_ptr->put(i,j,Finput.get(i,j));
      }
   }
}

void fundamental::parse_fundamental_inputs(
   const vector<threevector>& XY,const vector<threevector>& UV)
{
   parse_fundamental_inputs(XY,UV,XY.size());
}

void fundamental::parse_fundamental_inputs(
   const vector<threevector>& XY,const vector<threevector>& UV,
   unsigned int n_inputs)
{
   vector<twovector> XY2,UV2;
   for (unsigned int n=0; n<n_inputs; n++)
   {
      XY2.push_back(twovector(XY[n]));
      UV2.push_back(twovector(UV[n]));
   }
   parse_fundamental_inputs(XY2,UV2,n_inputs);
}
   
void fundamental::parse_fundamental_inputs(
   const vector<twovector>& XY,const vector<twovector>& UV)
{
   parse_fundamental_inputs(XY,UV,XY.size());
}

// ---------------------------------------------------------------------
void fundamental::parse_fundamental_inputs(
   const vector<twovector>& XY_init,const vector<twovector>& UV_init,
   unsigned int n_inputs)
{
//   cout << "inside fundamental::parse_fundamental_inputs()" << endl;

   if (XY_init.size() != UV_init.size())
   {
      cout << "Error in fundamental::parse_fundamental_inputs()"
           << endl;
      cout << "XY.size() = " << XY_init.size() 
           << " UV.size() = " << UV_init.size() << endl;
      exit(-1);
   }

   vector<twovector> XY=Hartley_normalize_homogenous_vectors(
      XY_init,n_inputs,T_XY_ptr);
//   cout << "T_XY = " << *T_XY_ptr << endl;

   vector<twovector> UV=Hartley_normalize_homogenous_vectors(
      UV_init,n_inputs,T_UV_ptr);
//   cout << "T_UV = " << *T_UV_ptr << endl;

   if (A_ptr->get_mdim() != n_inputs)
   {
      delete A_ptr;
      delete B_ptr;

      int nrows=n_inputs;
      int ncolumns=9;
      A_ptr=new genmatrix(nrows,ncolumns);
      B_ptr=new genmatrix(nrows,ncolumns);
   }

   for (unsigned int i=0; i<n_inputs; i++)
   {
      double x=XY[i].get(0);
      double y=XY[i].get(1);
      double u=UV[i].get(0);
      double v=UV[i].get(1);

//      cout << "i = " << i
//           << " X = " << x << " Y = " << y 
//           << " U = " << u << " V = " << v << endl;

      int row=i;
      A_ptr->put(row,0,x*u);
      A_ptr->put(row,1,x*v);
      A_ptr->put(row,2,x);
      A_ptr->put(row,3,y*u);
      A_ptr->put(row,4,y*v);
      A_ptr->put(row,5,y);
      A_ptr->put(row,6,u);
      A_ptr->put(row,7,v);
      A_ptr->put(row,8,1);

      B_ptr->put(row,0,u*x);
      B_ptr->put(row,1,u*y);
      B_ptr->put(row,2,u);
      B_ptr->put(row,3,v*x);
      B_ptr->put(row,4,v*y);
      B_ptr->put(row,5,v);
      B_ptr->put(row,6,x);
      B_ptr->put(row,7,y);
      B_ptr->put(row,8,1);

      row++;

   } // loop over index i

//   cout << "n_inputs = " << n_inputs << endl;
//   cout << "A = " << *A_ptr << endl;
//   cout << "A.mdim = " << A_ptr->get_mdim() << " A.ndim = " 
//        << A_ptr->get_ndim() << endl;
}

// ---------------------------------------------------------------------
// This overloaded version of parse_fundamental_inputs() is
// intentionally designed to avoid expensive twovectors and to
// minimize computation.

bool fundamental::parse_fundamental_inputs(
   const vector<double>& X_init,const vector<double>& Y_init,
   const vector<double>& U_init,const vector<double>& V_init,
   unsigned int n_inputs)
{
//   cout << "inside fundamental::parse_fundamental_inputs()" << endl;

   if (X_init.size() != U_init.size())
   {
      cout << "Error in fundamental::parse_fundamental_inputs()"
           << endl;
      cout << "X_init.size() = " << X_init.size() 
           << " U_init.size() = " << U_init.size() << endl;
      exit(-1);
   }

   int n_init=X_init.size();
   vector<double> X,Y,U,V;
   X.reserve(n_init);
   Y.reserve(n_init);
   U.reserve(n_init);
   V.reserve(n_init);

   if (!Hartley_normalize_homogenous_vectors(
      X_init,Y_init,n_inputs,X,Y,T_XY_ptr))
   {
      return false;
   }
//   cout << "T_XY = " << *T_XY_ptr << endl;

   if (!Hartley_normalize_homogenous_vectors(
      U_init,V_init,n_inputs,U,V,T_UV_ptr))
   {
      return false;
   }
//   cout << "T_UV = " << *T_UV_ptr << endl;

   if (A_ptr->get_mdim() != n_inputs)
   {
      delete A_ptr;
      int nrows=n_inputs;
      int ncolumns=9;
      A_ptr=new genmatrix(nrows,ncolumns);
   }

   for (unsigned int i=0; i<n_inputs; i++)
   {
      double x=X[i];
      double y=Y[i];
      double u=U[i];
      double v=V[i];

      int row=i;
      A_ptr->put(row,0,x*u);
      A_ptr->put(row,1,x*v);
      A_ptr->put(row,2,x);
      A_ptr->put(row,3,y*u);
      A_ptr->put(row,4,y*v);
      A_ptr->put(row,5,y);
      A_ptr->put(row,6,u);
      A_ptr->put(row,7,v);
      A_ptr->put(row,8,1);

      row++;

   } // loop over index i

//   cout << "n_inputs = " << n_inputs << endl;
//   cout << "A = " << *A_ptr << endl;
//   cout << "A.mdim = " << A_ptr->get_mdim() << " A.ndim = " 
//        << A_ptr->get_ndim() << endl;
   return true;
}

// ---------------------------------------------------------------------
// This overloaded version of Hartley_normalize_homogeneous_vectors()
// returns false if a division-by-zero error would have occurred given
// the inputs.

bool fundamental::Hartley_normalize_homogenous_vectors(
   const vector<double>& X,const vector<double>& Y,unsigned int n_inputs,
   vector<double>& X_ren,vector<double>& Y_ren,genmatrix* T_ptr)
{
//   cout << "inside fundamental::Hartley_normalize_homogeneous_vecs()" << endl;
   double X_COM=0;
   double Y_COM=0;
   for (unsigned int i=0; i<n_inputs; i++)
   {
      X_COM += X[i];
      Y_COM += Y[i];
   }
   X_COM /= n_inputs;
   Y_COM /= n_inputs;

   X_ren.reserve(n_inputs);
   Y_ren.reserve(n_inputs);
   double avg_magnitude=0;
   for (unsigned int i=0; i<n_inputs; i++)
   {
      X_ren.push_back(X[i]-X_COM);
      Y_ren.push_back(Y[i]-Y_COM);
      avg_magnitude += sqrt(sqr(X_ren.back())+sqr(Y_ren.back()));
   }
   avg_magnitude /= n_inputs;

   if (nearly_equal(avg_magnitude,0)) return false;

   double r=sqrt(2.0)/avg_magnitude;
   for (unsigned int i=0; i<n_inputs; i++)
   {
      X_ren[i] *= r;
      Y_ren[i] *= r;
   }

// Store Hartley translation and scaling transformation within 3x3
// matrix T:

   T_ptr->put(0,0,r);
   T_ptr->put(0,1,0);
   T_ptr->put(0,2,-r*X_COM);

   T_ptr->put(1,0,0);
   T_ptr->put(1,1,r);
   T_ptr->put(1,2,-r*Y_COM);
   
   T_ptr->put(2,0,0);
   T_ptr->put(2,1,0);
   T_ptr->put(2,2,1);

   return true;
}

// ---------------------------------------------------------------------
vector<twovector> fundamental::Hartley_normalize_homogenous_vectors(
   const vector<twovector>& XY,unsigned int n_inputs,genmatrix* T_ptr)
{
//   cout << "inside fundamental::Hartley_normalize_homogeneous_vecs()" << endl;
   twovector COM(0,0);
   for (unsigned int i=0; i<n_inputs; i++)
   {
      COM += XY[i];
   }
   COM /= n_inputs;

   vector<twovector> XY_ren;
   double avg_magnitude=0;
   for (unsigned int i=0; i<n_inputs; i++)
   {
      twovector curr_XY=XY[i]-COM;
      XY_ren.push_back(curr_XY);
      avg_magnitude += curr_XY.magnitude();
   }
   avg_magnitude /= n_inputs;

   double r=sqrt(2.0)/avg_magnitude;
   
   for (unsigned int i=0; i<n_inputs; i++)
   {
      XY_ren[i] *= r;
   }

/*
// Recompute mean and std_dev for XY_ren vecs:

   double mean_mag=0;
   twovector mean(0,0);
   for (unsigned int i=0; i<n_inputs; i++)
   {
      mean += XY_ren[i];
      mean_mag += XY_ren[i].magnitude();
   }
   mean /= n_inputs;
   mean_mag /= n_inputs;
   
   cout << "mean = " << mean << endl;
   cout << "mean_mag = " << mean_mag << endl;
*/

// Store Hartley translation and scaling transformation within 3x3
// matrix T:

   T_ptr->put(0,0,r);
   T_ptr->put(0,1,0);
   T_ptr->put(0,2,-r*COM.get(0));

   T_ptr->put(1,0,0);
   T_ptr->put(1,1,r);
   T_ptr->put(1,2,-r*COM.get(1));
   
   T_ptr->put(2,0,0);
   T_ptr->put(2,1,0);
   T_ptr->put(2,2,1);

/*
   cout << "T = " << *T_ptr << endl;
   for (unsigned int i=0; i<n_inputs; i++)
   {
      cout << "i = " << i 
           << " XY_ren[i] = " << XY_ren[i]
           << " T*XY[i] = " << (*T_ptr)*threevector(XY[i],1) << endl;
   }
*/

   return XY_ren;
}

// ---------------------------------------------------------------------
// Method compute_fundamental_matrix() works with the nx9
// genmatrix *A_ptr generated by method
// parse_fundamental_inputs().  It first performs a singular
// value decomposition of *A_ptr = U diag(w) V^T.  It then identifies
// the smallest singular value (which should be close to zero) as well
// as the corresponding column eigenvector v within matrix V.  This
// column vector should approximately satisfy the homogenous system of
// equations *A_ptr v = 0.  The entries of v are rearranged into
// 3x3 matrix *F_ptr, this method computes a second SVD of *F_ptr.  After
// setting the smallest singular value to zero to force the
// fundamental matrix to have rank 2, this method returns *F_ptr.
   
bool fundamental::compute_fundamental_matrix(bool print_flag)
{
//   cout << "inside fundamental::compute_fundamental_matrix()" << endl;
   
   const int mdim=A_ptr->get_mdim();
   const int ndim=A_ptr->get_ndim();
//   cout << "mdim = " << mdim << " ndim = " << ndim << endl;

   genmatrix U(mdim,ndim),W(ndim,ndim),V(ndim,ndim);
   if (!A_ptr->sorted_singular_value_decomposition(U,W,V)) return false;

//   cout << "W = " << W << endl;
//   cout << "V = " << V << endl;
//   cout << "A-U.W.VT = " 
//        << *A_ptr - (U*W*V.transpose()) << endl;

   int n_smallest=ndim-1;
   if (mdim < ndim) n_smallest=mdim-1;
//   cout << "n_smallest = " << n_smallest << endl;

   genvector column(ndim);
   V.get_column(n_smallest,column);

//   cout << "column = " << column << endl;
//   cout << "A*column = " << *A_ptr * column << endl;

// Rearrange column vector's contents within 3x3 fundamental matrix
// *F_ptr:

   F_ptr->put(0,0,column.get(0));
   F_ptr->put(0,1,column.get(1));
   F_ptr->put(0,2,column.get(2));

   F_ptr->put(1,0,column.get(3));
   F_ptr->put(1,1,column.get(4));
   F_ptr->put(1,2,column.get(5));

   F_ptr->put(2,0,column.get(6));
   F_ptr->put(2,1,column.get(7));
   F_ptr->put(2,2,column.get(8));
   renormalize_F_entries();

   if (print_flag)
      cout << "Candidate fundamental matrix F = " << *F_ptr << endl;

// Compute SVD of candidate F:

   genmatrix u(3,3),w(3,3),v(3,3);
   if (!F_ptr->sorted_singular_value_decomposition(u,w,v)) return false;
   
   if (print_flag)
      cout << "w = " << w << endl;

// Check magnitude of smallest eigenvalue in diagonal matrix w.  If it
// exceeds a threshold, modified F matrix which has this smallest
// eigenvalue replaced with zero to enforce a rank-2 condition is
// unlikely to be accurate (i.e. rprime . F_rank_2 . r is not likely
// to be close to zero where r and rprime represent imageplane
// tiepoints).  In this case, set boolean possibly_good_Fmatrix_flag
// to false;

   bool possibly_good_Fmatrix_flag=true;
   const double min_eigenvalue_ceiling=0.1;
//   const double min_eigenvalue_ceiling=1.0;
   double wmin=w.get(2,2);
   if (wmin > min_eigenvalue_ceiling) 
   {
      possibly_good_Fmatrix_flag=false;
   }
   
   w.put(2,2,0);
   *F_ptr=u*w*v.transpose();

// Denormalize fundamental matrix:

   genmatrix F_init(3,3);
//   F_init = T_UV_ptr->transpose() * (*F_ptr) * (*T_XY_ptr);
   F_init = T_XY_ptr->transpose() * (*F_ptr) * (*T_UV_ptr);
   *F_ptr=F_init;
   renormalize_F_entries();

   if (print_flag)
      cout << "Rank-2 fundamental matrix F = " << *F_ptr << endl;

   return possibly_good_Fmatrix_flag;
}

// ---------------------------------------------------------------------
// Method check_fundamental_matrix() reads in the original XY-UV
// information.  It computes the scalar (x,y,1) * (*F_ptr) * (u,v,1)
// which should equal zero.  If this scalar for any tiepoint pair
// exceeds a relatively small threshold value, this boolean method
// returns false.

bool fundamental::check_fundamental_matrix(
   const vector<twovector>& XY,const vector<twovector>& UV,bool print_flag)
{
   return check_fundamental_matrix(XY,UV,XY.size(),print_flag);
}

bool fundamental::check_fundamental_matrix(
   const vector<twovector>& XY,const vector<twovector>& UV,
   unsigned int n_inputs,bool print_flag)
{
//   cout << "inside fundamental::check_fundamental_matrix()" << endl;

   bool valid_fundamental_matrix_flag=true;
//   const double max_scalar_product_mag=0.1;
   const double max_scalar_product_mag=1.0;
   
   vector<int> label;
   vector<double> scalar_product_mags;
   XY_sorted.clear();
   UV_sorted.clear();

   for (unsigned int i=0; i<XY.size(); i++)
   {
      label.push_back(i);
      XY_sorted.push_back(XY[i]);
      UV_sorted.push_back(UV[i]);
      scalar_product_mags.push_back(fabs(scalar_product(XY[i],UV[i])));

      if (scalar_product_mags.back() > max_scalar_product_mag)
      {
         valid_fundamental_matrix_flag=false;
         if (!print_flag) return valid_fundamental_matrix_flag;
      }
   }

   if (print_flag)
   {
      templatefunc::Quicksort(scalar_product_mags,label,XY_sorted,UV_sorted);
      for (unsigned int i=0; i<n_inputs; i++)
      {
         cout << "i = " << i 
              << " X=" << XY_sorted[i].get(0)
              << " Y=" << XY_sorted[i].get(1)
              << " U=" << UV_sorted[i].get(0)
              << " V=" << UV_sorted[i].get(1)
              << " (X,Y,1)F(U,V,1) = " 
              << scalar_product(XY_sorted[i],UV_sorted[i]) << endl;
      } // loop over index i 
      cout << endl;
      cout << "F = " << *F_ptr << endl;
   }
   
   return valid_fundamental_matrix_flag;
}

// ==========================================================================
// 7-point algorithm member functions
// ==========================================================================

// Method seven_point_algorithm() works with the 7x9 genmatrix *A_ptr
// generated by method parse_fundamental_inputs(). It first performs a
// singular value decomposition of *A_ptr = U diag(w) V^T.  If the SVD
// fails, this method returns a sentinel -1 value. Otherwise, it
// identifies the smallest two singular values (which should be close
// to zero) as well as their corresponding column eigenvectors within
// matrix V.  Both column vectors should approximately satisfy the
// homogenous system of equations *A_ptr v = 0.  The entries of v are
// rearranged into 3x3 matrices F1 and F2.  We next solve for the real
// roots of the cubic polynomial Det(alpha F1 + (1-alpha) F2) = 0.
// The one or three real roots of the cubic yield one or three linear
// combinations of F1 and F2 which have vanishing determinant and
// represent candidate fundamental matrices.  The candidates are
// stored within member STL vector F_ptrs.  This method returns the
// number of candidate fundamental matrices (1 or 3) which it found.
   
// As of 1/31/13, we do not believe this method is the rate-limiting
// step for matching SIFT/ASIFT features...

int fundamental::seven_point_algorithm(bool ignore_triple_roots_flag)
{
//   cout << "inside fundamental::seven_point_algorithm()" << endl;
//   cout << "*A_ptr = " << *A_ptr << endl;

   const int mdim=A_ptr->get_mdim();
   const int ndim=A_ptr->get_ndim();
//   cout << "mdim = " << mdim << " ndim = " << ndim << endl;

   genmatrix U(mdim,ndim),W(ndim,ndim),V(ndim,ndim);
   if (!A_ptr->sorted_singular_value_decomposition(U,W,V)) return -1;

//   cout << "W = " << W << endl;
//   cout << "V = " << V << endl;
//   cout << "A-U.W.VT = " 
//        << *A_ptr - (U*W*V.transpose()) << endl;

   int n_smallest=ndim-1;
   if (mdim < ndim) n_smallest=mdim-1;
//   cout << "n_smallest = " << n_smallest << endl;

   genvector F1(ndim),F2(ndim);
   V.get_column(n_smallest-1,F1);
   V.get_column(n_smallest,F2);

//   cout << "F1 = " << F1 << endl;
//   cout << "A*F1 = " << *A_ptr * F1 << endl;
//   cout << "F2 = " << F2 << endl;
//   cout << "A*F2 = " << *A_ptr * F2 << endl;

// Rearrange F1 and F2 vector contents within 3x3 fundamental matrices
// F1_mat and F2_mat:

   genmatrix F1_mat(3,3),F2_mat(3,3);

   F1_mat.put(0,0,F1.get(0));
   F1_mat.put(0,1,F1.get(1));
   F1_mat.put(0,2,F1.get(2));

   F1_mat.put(1,0,F1.get(3));
   F1_mat.put(1,1,F1.get(4));
   F1_mat.put(1,2,F1.get(5));

   F1_mat.put(2,0,F1.get(6));
   F1_mat.put(2,1,F1.get(7));
   F1_mat.put(2,2,F1.get(8));

   F2_mat.put(0,0,F2.get(0));
   F2_mat.put(0,1,F2.get(1));
   F2_mat.put(0,2,F2.get(2));

   F2_mat.put(1,0,F2.get(3));
   F2_mat.put(1,1,F2.get(4));
   F2_mat.put(1,2,F2.get(5));

   F2_mat.put(2,0,F2.get(6));
   F2_mat.put(2,1,F2.get(7));
   F2_mat.put(2,2,F2.get(8));

   double p_neg1=compute_linear_combo_det(-1,F1_mat,F2_mat);
   double p_0=compute_linear_combo_det(0,F1_mat,F2_mat);
   double p_1=compute_linear_combo_det(1,F1_mat,F2_mat);
   double p_2=compute_linear_combo_det(2,F1_mat,F2_mat);
   fourvector P(p_neg1,p_0,p_1,p_2);

   if (Ninverse_ptr==NULL) initialize_Ninverse_matrix();

   fourvector C=(*Ninverse_ptr)*P;

// On 1/31/13, we empirically found that SIFT/ASIFT feature matching
// proceeds faster if we simply ignore triple-root solutions of the
// cubic polynomial within the RANSAC loop.  Finding the best of 3
// solutions requires identifying inliers three times which is
// expensive.  So it's faster to randomly regenerate a new set of 7
// tiepoints and hope that it yields a unique real root of the cubic
// polynomial...

   if (ignore_triple_roots_flag)
   {
      double Delta=mathfunc::cubic_poly_discriminant(
         C.get(0),C.get(1),C.get(2),C.get(3));
      if (mathfunc::n_real_cubic_roots(Delta) > 1) return -1;
   }

   vector<double> poly_roots=mathfunc::real_cubic_roots(
      C.get(0),C.get(1),C.get(2),C.get(3));

   unsigned int n_roots=poly_roots.size();
   for (unsigned int r=0; r<n_roots; r++)
   {
//      cout << "r = " << r << " real poly root = " << poly_roots[r]
//           << endl;
      double alpha=poly_roots[r];
      *F_ptr=alpha*F1_mat + (1-alpha)*F2_mat;
//      cout << "F = " << *F_ptr << endl;
//      cout << "F.det = " << F_ptr->determinant() << endl;

// Denormalize fundamental matrix:

      *F_ptr = T_XY_ptr->transpose() * (*F_ptr) * (*T_UV_ptr);

// Save current fundamental matrix candidate into STL vector F_ptrs:

      (*F_ptrs[r])=*F_ptr;

   } // loop over index r labeling real polynomial roots

   return n_roots;
}

// ---------------------------------------------------------------------
// Method initialize_Ninverse_matrix() instantiates and fills 4x4
// matrix *Ninverse_ptr which is needed to recover cubic polynomial
// coeffs in the 7-point algorithm.
   
void fundamental::initialize_Ninverse_matrix()
{
//   cout << "inside fundamental::initialize_Ninverse_matrix()" << endl;
   
   Ninverse_ptr=new genmatrix(4,4);

   double sixth=1.0/6.0;
   double third=1.0/3.0;
   double half=1.0/2.0;

   Ninverse_ptr->put(0,0,-sixth);
   Ninverse_ptr->put(0,1,half);
   Ninverse_ptr->put(0,2,-half);
   Ninverse_ptr->put(0,3,sixth);

   Ninverse_ptr->put(1,0,half);
   Ninverse_ptr->put(1,1,-1);
   Ninverse_ptr->put(1,2,half);
   Ninverse_ptr->put(1,3,0);

   Ninverse_ptr->put(2,0,-third);
   Ninverse_ptr->put(2,1,-half);
   Ninverse_ptr->put(2,2,1);
   Ninverse_ptr->put(2,3,-sixth);

   Ninverse_ptr->put(3,0,0);
   Ninverse_ptr->put(3,1,1);
   Ninverse_ptr->put(3,2,0);
   Ninverse_ptr->put(3,3,0);
}

// ==========================================================================
// Fundamental matrix linear algebra member functions
// ==========================================================================

// Member function get_null_vector() returns unit vector e0_hat where
// (*F_ptr) * e0_hat = 0.  *e0_hat = epipole of fundamental matrix
// *F_ptr.  This method assumes that *F_ptr has precisely rank 2.  

threevector fundamental::get_null_vector() const
{
   return get_null_vector(F_ptr);
}

threevector fundamental::get_null_vector(genmatrix* f_ptr) const
{
//   cout << "inside fundamental::get_null_vector()" << endl;
//   cout << "*f_ptr = " << *f_ptr << endl;

   genmatrix U(3,3),W(3,3),V(3,3);
   f_ptr->sorted_singular_value_decomposition(U,W,V);

//   cout << "W = " << W << endl;
//   cout << "V = " << V << endl;

   threevector e0_hat( V.get(0,2) , V.get(1,2), V.get(2,2) );
//   cout << "e0_hat = " << e0_hat << endl;
//   cout << "e0_hat.magnitude() = " << e0_hat.magnitude() << endl;
//   cout << "f*e0_hat = " << (*f_ptr) * e0_hat << endl;

   return e0_hat;
}

// ---------------------------------------------------------------------
// Member function scalar_product() computes and returns (x,y,1) * F *
// (u,v,1).

double fundamental::scalar_product(const twovector& xy,const twovector& uv)
{
   threevector r(xy.get(0),xy.get(1),1);
   threevector rprime(uv.get(0),uv.get(1),1);
   double scalar_product=r.dot( *F_ptr * rprime );

   return scalar_product;
}

// ==========================================================================
// Iterative determination of fundamental matrix member functions
// ==========================================================================

// Member function solve_for_fundamental()

genvector* fundamental::solve_for_fundamental(const threevector& epipole) 
{
//   cout << "inside fundamental::solve_for_fundamental()" << endl;

   genmatrix* N_ptr=generate_bigE_matrix(epipole);

   genmatrix U(9,9),W(9,9),V(9,9);
   if (!N_ptr->sorted_singular_value_decomposition(U,W,V)) return NULL;
   delete N_ptr;

   int N_rank=6;
   genmatrix Uprime(9,N_rank);
   U.get_smaller_matrix(Uprime);

//   cout << "U = " << U << endl;
//   cout << "Uprime = " << Uprime << endl;

   genmatrix AUprime(A_ptr->get_mdim(),N_rank);
   AUprime=*A_ptr * Uprime;

   genvector Xprime(N_rank);
   AUprime.homogeneous_soln(Xprime);
   
   genvector f(9);
   f=Uprime*Xprime;
//   cout << "f = " << f << endl;

   genvector* epsilon_ptr=new genvector(A_ptr->get_mdim());
   *epsilon_ptr=(*A_ptr) * f;
//   cout << "*epsilon_ptr = " << *epsilon_ptr << endl;
//   cout << "epsilon_ptr->magnitude() = " << epsilon_ptr->magnitude() 
//        << endl;

   F_ptr->put(0,0,f.get(0));
   F_ptr->put(0,1,f.get(1));
   F_ptr->put(0,2,f.get(2));

   F_ptr->put(1,0,f.get(3));
   F_ptr->put(1,1,f.get(4));
   F_ptr->put(1,2,f.get(5));

   F_ptr->put(2,0,f.get(6));
   F_ptr->put(2,1,f.get(7));
   F_ptr->put(2,2,f.get(8));

//   cout << "*F_ptr = " << *F_ptr << endl;
//   cout << "detF = " << F_ptr->determinant() << endl;

   return epsilon_ptr;
}

// ---------------------------------------------------------------------
// Member function generate_bigE_matrix() computes and returns the 9x9
// antisymmetric matrix corresponding to eqn 11.4 within "Multiple view
// geometry in computer vision" by Hartley and Zisseman (2nd edition).

genmatrix* fundamental::generate_bigE_matrix(const threevector& e_hat) const
{
//   cout << "inside fundamental::generate_null_matrix()" << endl;
   
   genmatrix* antisym_e_ptr=e_hat.generate_antisymmetric_matrix();

   genmatrix* N_ptr=new genmatrix(9,9);
   N_ptr->clear_values();
   
   N_ptr->put_smaller_matrix(0,0,*antisym_e_ptr);
   N_ptr->put_smaller_matrix(3,3,*antisym_e_ptr);
   N_ptr->put_smaller_matrix(6,6,*antisym_e_ptr);
   delete antisym_e_ptr;

//   cout << "*N_ptr = " << *N_ptr << endl;

   return N_ptr;
}

// ==========================================================================
// Epipole manipulation member functions
// ==========================================================================

threevector fundamental::get_epipole_XY() const
{
//   cout << "inside fundamental::get_epipole_XY()" << endl;
//   cout << "*F_ptr = " << *F_ptr << endl;
   genmatrix* Ftrans_ptr=new genmatrix(3,3);
   *Ftrans_ptr=F_ptr->transpose();
   threevector eprime=get_null_vector(Ftrans_ptr);
   eprime /= eprime.get(2);
//   cout << "eprime = " << eprime << endl;
//   cout << "Ftrans * eprime = " << *Ftrans_ptr * eprime << endl;
   delete Ftrans_ptr;
   return eprime;
}

threevector fundamental::get_epipole_UV() const
{
//   cout << "inside fundamental::get_epipole_UV()" << endl;
//   cout << "*F_ptr = " << *F_ptr << endl;
   threevector e=get_null_vector();
   e /= e.get(2);
//   cout << "e = " << e << endl;
//   cout << "F * e = " << *F_ptr * e << endl;
   return e;
}

// ---------------------------------------------------------------------
// Member function map_epipole_to_infinity() implements the algorithm
// presented in section 11.12.1 of "Multi view geometry for computer
// vision" by Hartley and Zisserman (2nd edition).  

homography* fundamental::map_epipole_to_infinity(
   double Umax,const threevector& e)
{
   threevector image_center(0.5*Umax , 0.5 , 1);
   genmatrix T(3,3),Tinv(3,3);
   T.identity();
   T.put(0,2,-image_center.get(0));
   T.put(1,2,-image_center.get(1));

   Tinv.identity();
   Tinv.put(0,2,image_center.get(0));
   Tinv.put(1,2,image_center.get(1));
   
//   cout << "T = " << T << endl;
//   cout << "image_center = " << image_center << endl;
//   cout << "T * image_center = " << T*image_center << endl;

   twovector e_modified(e);
   e_modified=e_modified.unitvector();
   double cos_theta=e_modified.get(0);
   double sin_theta=e_modified.get(1);
//   cout << "sqr(cos_theta)+sqr(sin_theta) = "
//        << sqr(cos_theta)+sqr(sin_theta) << endl;
//   double theta=atan2(sin_theta,cos_theta);
//   cout << "theta = " << theta*180/PI << endl;

   rotation R;   
   R.identity();
   R.put(0,0,cos_theta);
   R.put(0,1,sin_theta);
   R.put(1,0,-sin_theta);
   R.put(1,1,cos_theta);

//   cout << "e = " << e << endl;
//   cout << "e_modified = " << e_modified << endl;

//   cout << "R = " << R << endl;
//   cout << "R*e = " << R*e << endl;
   threevector e_rotated=R*e;
   for (unsigned int i=0; i<3; i++)
   {
      e_rotated.put(i,e_rotated.get(i)/e_rotated.get(2));
   }
//   cout << "e_rotated = " << e_rotated << endl;

   genmatrix G(3,3);
   G.identity();
   G.put(2,0,-1/e_rotated.get(0));
//   cout << "G = " << G << endl;

   genmatrix h(3,3);
//   h = T;
//   h = R*T;
//   h = G*R*T;
   h = Tinv*G*R*T;
//   outputfunc::enter_continue_char();

   homography* H_ptr=new homography();

   for (unsigned int i=0; i<3; i++)
   {
      for (unsigned int j=0; j<3; j++)
      {
         H_ptr->set_H_ptr_element(i,j,h.get(i,j));
      }
   }
   H_ptr->compute_homography_inverse();

   return H_ptr;
}

// ---------------------------------------------------------------------
// Member function decompose_fundamental_as_M_times_eUV() extracts and 
// returns the 3x3 matrix M which satisfies F = M [e_UV]. 

genmatrix* fundamental::decompose_fundamental_as_M_times_eUV()
{
//   cout << "inside fundamental::decompose_fundamental_as_M_times_eUV()" 
//        << endl;

   threevector e_UV=get_epipole_UV();   
   threevector e_XY=get_epipole_XY();   
   genmatrix* e_UV_antisym_ptr=
      e_UV.generate_antisymmetric_matrix();
//   genmatrix* e_XY_antisym_ptr=
//      e_XY.generate_antisymmetric_matrix();

//   cout << "e_XY = " << e_XY << endl;
//   cout << "e_UV = " << e_UV << endl;
//   cout << "e_UV_antisym = " << *e_UV_antisym_ptr << endl;
//   cout << "e_UV_antisym det = " << e_UV_antisym_ptr->determinant() << endl;
//   cout << "e_XY * F = " <<  e_XY * (*F_ptr) << endl;
//   cout << "F * e_UV = " <<  (*F_ptr) * e_UV << endl;

   genmatrix E(9,9);
   E.clear_values();
   E.put_smaller_matrix(0,0,*e_UV_antisym_ptr);
   E.put_smaller_matrix(3,3,*e_UV_antisym_ptr);
   E.put_smaller_matrix(6,6,*e_UV_antisym_ptr);
//   cout << "E = " << E << endl;

// Minimize ||AEm|| subject to ||Em|| = 1

// x=Em=f  --> G = E  and xhat = m

// Compute SVD of E:

   genmatrix U(9,9),W(9,9),V(9,9);          
   if (!E.sorted_singular_value_decomposition(U,W,V)) return NULL;

   int E_rank=6;
   genmatrix Uprime(9,E_rank);
   U.get_smaller_matrix(Uprime);
//   cout << "U = " << U << endl;
//   cout << "Uprime = " << Uprime << endl;

   genmatrix AUprime(A_ptr->get_mdim(),E_rank);
   AUprime=*A_ptr * Uprime;

   genvector Xprime(E_rank);
   AUprime.homogeneous_soln(Xprime);
   
   genvector f(9),Em(9);
   f=Uprime*Xprime;

   genmatrix Vprime(9,E_rank);
   V.get_smaller_matrix(Vprime);
//   cout << "V = " << V << endl;
//   cout << "Vprime = " << Vprime << endl;

   genmatrix Wprime(E_rank,E_rank),Wprime_inverse(E_rank,E_rank);
   W.get_smaller_matrix(Wprime);
   Wprime.inverse(Wprime_inverse);
   
   genvector m(9);
   m=Vprime * Wprime_inverse * Xprime;

//   cout << "m = " << m << endl;
//   cout << "E * m = " << E*m << endl;
//   cout << "f = " << f << endl;
//   cout << "f.mag = " << f.magnitude() << endl;

   genmatrix* M_ptr=new genmatrix(3,3);
   for (unsigned int i=0; i<3; i++)
   {
      for (unsigned int j=0; j<3; j++)
      {
         int n=i*3+j;
         M_ptr->put(i,j,m.get(n));
      }
   }

   cout << "M = " << *M_ptr << endl;
   cout << "M*eUV_antisym = " << *M_ptr * (*e_UV_antisym_ptr) << endl;
   cout << "F = " << *F_ptr << endl;

   return M_ptr;
}

// ---------------------------------------------------------------------
// Member function decompose_fundamental_as_eXY_times_M() extracts and
// returns the 3x3 matrix M which satisfies F = [eXY]_x M . 

genmatrix* fundamental::decompose_fundamental_as_eXY_times_M()
{
//   cout << "inside fundamental::decompose_fundamental_as_eXY_times_M()" 
//        << endl;

   threevector e_XY=get_epipole_XY();   
   genmatrix* e_XY_antisym_ptr=e_XY.generate_antisymmetric_matrix();

//   cout << "e_XY = " << e_XY << endl;
//   cout << "e_UV = " << e_UV << endl;
//   cout << "e_XY_antisym = " << *e_XY_antisym_ptr << endl;
//   cout << "e_XY_antisym det = " << e_XY_antisym_ptr->determinant() << endl;
//   cout << "e_XY * F = " <<  e_XY * (*F_ptr) << endl;
//   cout << "F * e_UV = " <<  (*F_ptr) * e_UV << endl;

   genmatrix E(9,9);
   E.clear_values();
   E.put_smaller_matrix(0,0,*e_XY_antisym_ptr);
   E.put_smaller_matrix(3,3,*e_XY_antisym_ptr);
   E.put_smaller_matrix(6,6,*e_XY_antisym_ptr);
//   cout << "E = " << E << endl;

// Compute SVD of E:

   genmatrix U(9,9),W(9,9),V(9,9);          
   if (!E.sorted_singular_value_decomposition(U,W,V)) return NULL;

   int E_rank=6;
   genmatrix Uprime(9,E_rank);
   U.get_smaller_matrix(Uprime);
//   cout << "U = " << U << endl;
//   cout << "Uprime = " << Uprime << endl;

   genmatrix BUprime(B_ptr->get_mdim(),E_rank);
   BUprime=*B_ptr * Uprime;
//   cout << "BUprime = " << BUprime << endl;

   genvector Xprime(E_rank);
   BUprime.homogeneous_soln(Xprime);
   
   genvector g(9);
   g=Uprime*Xprime;

   genmatrix Vprime(9,E_rank);
   V.get_smaller_matrix(Vprime);
//   cout << "V = " << V << endl;
//   cout << "Vprime = " << Vprime << endl;

   genmatrix Wprime(E_rank,E_rank),Wprime_inverse(E_rank,E_rank);
   W.get_smaller_matrix(Wprime);
   Wprime.inverse(Wprime_inverse);
   
   genvector n(9);
   n=Vprime * Wprime_inverse * Xprime;

//   cout << "n = " << n << endl;
//   cout << "E * n = " << E*n << endl;
//   cout << "g = " << g << endl;
//   cout << "g.mag = " << g.magnitude() << endl;

   genmatrix N(3,3);
   for (unsigned int i=0; i<3; i++)
   {
      for (unsigned int j=0; j<3; j++)
      {
         int k=i*3+j;
         N.put(i,j,n.get(k));
      }
   }

//   cout << "N = " << N << endl;
//   cout << "N * e_XY_antisym = " << N * (*e_XY_antisym_ptr) << endl;
//   cout << "G = " << F_ptr->transpose() << endl;

   genmatrix* M_ptr=new genmatrix(3,3);
   *M_ptr=N.transpose();
   
   cout << "M = " << *M_ptr << endl;
   cout << "e_XY_antisym * M = " << (*e_XY_antisym_ptr) * (*M_ptr) << endl;
   cout << "F = " <<  *F_ptr << endl;

   return M_ptr;
}

// ---------------------------------------------------------------------
// Member function compute_matching_UV_homography() implements the
// algorithm presented in section 11.12.2 of "Multi view geometry for
// computer vision" by Hartley and Zisserman (2nd edition).

homography* fundamental::compute_matching_UV_homography(
   homography* H_XY_ptr)
{
   cout << "inside fundamental::compute_matching_UV_homography()" << endl;

   genmatrix H0(3,3);
   genmatrix* M_ptr=decompose_fundamental_as_eXY_times_M();
   H0 = *(H_XY_ptr->get_H_ptr()) * (*M_ptr);
   delete M_ptr;

// First transform XY_sorted according to H_XY:

   vector<threevector> XY_hat;
   for (unsigned int i=0; i<XY_sorted.size(); i++)
   {
      threevector curr_XY(XY_sorted[1],1);
      XY_hat.push_back(*(H_XY_ptr->get_H_ptr()) * curr_XY);
   } // loop over index i labeling XY-UV tiepoints
   
// Next transform UV_sorted according to H0:

   vector<threevector> UV_hat;
   for (unsigned int i=0; i<UV_sorted.size(); i++)
   {
      threevector curr_UV(UV_sorted[i],1);
      UV_hat.push_back(H0 * curr_UV);
   }
   
// Define 3x3 matrix coefficents:

   double Usq_sum=0;
   double Vsq_sum=0;
   double UV_sum=0;
   double U_sum=0;
   double V_sum=0;
   double UX_sum=0;
   double VX_sum=0;
   double X_sum=0;
   
   for (unsigned int i=0; i<XY_hat.size(); i++)
   {
      double xhat=XY_hat[i].get(0);
//      double yhat=XY_hat[i].get(1);
      double uhat=UV_hat[i].get(0);
      double vhat=UV_hat[i].get(1);
      
      Usq_sum += sqr(uhat);
      Vsq_sum += sqr(vhat);
      UV_sum += uhat*vhat;
      U_sum += uhat;
      V_sum += vhat;
      UX_sum += uhat*xhat;
      VX_sum += vhat*xhat;
      X_sum += xhat;
   }

   genmatrix M(3,3);
   M.put(0,0,Usq_sum);
   M.put(0,1,UV_sum);
   M.put(0,2,U_sum);
   M.put(1,0,UV_sum);
   M.put(1,1,Vsq_sum);
   M.put(1,2,V_sum);
   M.put(2,0,U_sum);
   M.put(2,1,V_sum);
   M.put(2,2,1);
  
   threevector A;
   threevector B(UX_sum,VX_sum,X_sum);
   M.inhomogeneous_soln(B,A);

   double a=A.get(0);
   double b=A.get(1);
   double c=A.get(2);
   
   genmatrix HA(3,3);
   HA.identity();
   HA.put(0,0,a);
   HA.put(0,1,b);
   HA.put(0,2,c);

   homography* H_UV_ptr=new homography();
   *(H_UV_ptr->get_H_ptr()) = HA * H0;
   H_UV_ptr->compute_homography_inverse();

   return H_UV_ptr;
}

// ---------------------------------------------------------------------
// Member function projection_matrix_rows_determinant()

double fundamental::projection_matrix_rows_determinant(
   int p,int q,int r,int s,const genmatrix& P_UV,const genmatrix& P_XY)
{
//   cout << "inside fundamental::projection_matrix_rows_determinant()" << endl;
//   cout << "P_UV = " << P_UV << endl;
//   cout << "P_XY = " << P_XY << endl;
//   cout << "p = " << p << " q = " << q << " r = " << r << " s = " << s << endl;

   fourvector curr_row;
   vector<fourvector> a,b;
   for (unsigned int i=0; i<3; i++)
   {
      P_UV.get_row(i,curr_row);
      a.push_back(curr_row);
      P_XY.get_row(i,curr_row);
      b.push_back(curr_row);
   }
   
//   cout << "a[0] = " << a[0] << endl;
//   cout << "a[1] = " << a[1] << endl;
//   cout << "a[2] = " << a[2] << endl;

//   cout << "b[0] = " << b[0] << endl;
//   cout << "b[1] = " << b[1] << endl;
//   cout << "b[2] = " << b[2] << endl;

   genmatrix M(4,4);
   M.put_row(0,a[p]);
   M.put_row(1,a[q]);
   M.put_row(2,b[r]);
   M.put_row(3,b[s]);
//   cout << "M = " << M << endl;

   double det=M.determinant();
//   cout << "det M = " << det << endl;

//   outputfunc::enter_continue_char();
   return det;
}

// ---------------------------------------------------------------------
// Member function compute_from_projection_matrices() the fundamental
// matrix from two cameras 3x4 projection matrices.  It implements eqn
// (17.4) in "Multiple View Geometry in Computer Vision" by Harlety
// and Zisserman (2nd edition).

// Note: This member function is essentially identical to
// camerafunc::calculate_fundamental_matrix().

void fundamental::compute_from_projection_matrices(
   const genmatrix& P_UV,const genmatrix& P_XY)
{
//   cout << "inside fundamental::compute_from_projection_matrices()" << endl;
   genmatrix F(3,3);
   F.clear_values();

   for (unsigned int j=0; j<3; j++)
   {
      for (unsigned int i=0; i<3; i++)
      {
         double curr_sum=0;
         
         for (unsigned int p=0; p<3; p++)
         {
            for (unsigned int q=0; q<3; q++)
            {
               int eps_ipq=mathfunc::LeviCivita(i,p,q);
               if (eps_ipq==0) continue;
               for (unsigned int r=0; r<3; r++)
               {
                  for (unsigned int s=0; s<3; s++)
                  {
                     int eps_jrs=mathfunc::LeviCivita(j,r,s);
                     if (eps_jrs==0) continue;
                     
                     double curr_det=projection_matrix_rows_determinant(
                        p,q,r,s,P_UV,P_XY);
/*
                     if (eps_ipq != 0 && eps_jrs != 0)
                     {
                        cout << "eps_ipq = " << eps_ipq
                             << " j = " << j << " r = " << r << " s = " << s
                             << " eps_jrs = " << eps_jrs
                             << " curr_det = " << curr_det << endl;
                     }
*/
                     curr_sum += eps_ipq * eps_jrs * curr_det;

                  } // loop over index s
               } // loop over index r
            } // loop over index q
         } // loop over index p
         
         F.put(j,i,curr_sum);
//         cout << "j = " << j << " i = " << i << " curr_sum = " << curr_sum
//              << endl;
      
      } // loop over index i
   } // loop over index j    

   set_F_values(F);
   renormalize_F_entries();
   cout << "F_ptr->rank = " << F_ptr->rank() << endl;
   cout << "*F_ptr = " << *F_ptr << endl;

//   vector<twovector> XY=get_XY_sorted();
//   vector<twovector> UV=get_UV_sorted();
//   for (unsigned int i=0; i<XY.size(); i++)
//   {
//      cout << "i = " << i << " XY = " << XY[i] << " UV = " << UV[i]
//           << endl;
//   }
//   check_fundamental_matrix(XY,UV,true);

   cout << "epipole_XY = " << get_epipole_XY() << endl;
   cout << "epipole_UV = " << get_epipole_UV() << endl;
}

// ---------------------------------------------------------------------
// Member function renormalize_F_entries() checks if the last (2,2)
// element in *F_ptr is nonzero.  If so, it divides F by F(2,2).

void fundamental::renormalize_F_entries()
{
   double last_entry=F_ptr->get(2,2);
   if (nearly_equal(last_entry,0))
   {
      cout << "Trouble in fundamental::renormalize_F_entries()!" << endl;
      cout << "last_entry = F(2,2) = " << last_entry << endl;
      return;
   }

   for (unsigned int i=0; i<3; i++)
   {
      for (unsigned int j=0; j<3; j++)
      {
         F_ptr->put(i,j,F_ptr->get(i,j)/last_entry);
      }
   }
//   cout << "Renormalized *F_ptr = " << *F_ptr << endl;
}

void fundamental::renormalize_Fbest_entries()
{
   double last_entry=Fbest_ptr->get(2,2);
   if (nearly_equal(last_entry,0))
   {
      cout << "Trouble in fundamental::renormalize_Fbest_entries()!" << endl;
      cout << "last_entry = F(2,2) = " << last_entry << endl;
      return;
   }

   for (unsigned int i=0; i<3; i++)
   {
      for (unsigned int j=0; j<3; j++)
      {
         Fbest_ptr->put(i,j,Fbest_ptr->get(i,j)/last_entry);
      }
   }
//   cout << "Renormalized *Fbest_ptr = " << *Fbest_ptr << endl;
}

// ==========================================================================
// Projection matrix member functions
// ==========================================================================

// Member function compute_trivial_projection_matrix() returns the 3x4
// projection matrix P = [ I_3x3 | 0 ] .  

genmatrix* fundamental::compute_trivial_projection_matrix()
{
   cout << "inside fundamental::compute_trivial_projection_matrix()" << endl;

   P_ptr=new genmatrix(3,4);
   P_ptr->clear_values();
   for (unsigned int i=0; i<3; i++)
   {
      P_ptr->put(i,i,1);
   }
   return P_ptr;
}

// --------------------------------------------------------------------------
// Member function compute_nontrivial_projection_matrix() takes in a
// fundamental matrix.  The first camera's 3x4 projection matrix may
// be taken to equal P = [ I_3x3 | 0 ] .  This method returns the
// "simplest" 3x4 projection matrix for the 2nd camera which is
// consistent with the input fundamental matrix.  This method
// implements Result 9.14 in "Multiple view geometry in computer
// vision" by Hartley and Zisserman (2nd edition).  Recall that this
// nontrivial projection matrix must be multiplied on its right by
// 4x4 matrix

//		 	H=[ K0  | 0 ]
//      		  [ v^T | 1 ]

// in order to upgrade the projective reconstruction to a (useful)
// Euclidean reconstruction.  The 3-vector v corresponding to the
// "plane at infinity" must be found to perform this Euclidean
// upgrade.

genmatrix* fundamental::compute_nontrivial_projection_matrix()
{
   cout << "inside fundamental::compute_nontrivial_projection_matrix()" 
        << endl;

   threevector eprime=get_epipole_XY();
   genmatrix* A_ptr=eprime.generate_antisymmetric_matrix();
   genmatrix AF(3,3);
   AF=(*A_ptr) * (*F_ptr);
   delete A_ptr;

//   genmatrix outerproduct(3,3);
//   outerproduct.clear_values();

   double lambda=1;
//      cout << "Enter lambda != 0" << endl;
//      cin >> lambda;
//      double lambda=200*nrfunc::ran1()-100;
//      cout << "random lambda = " << lambda << endl;

   Pprime_ptr=new genmatrix(3,4);
   for (unsigned int i=0; i<3; i++)
   {
      for (unsigned int j=0; j<3; j++)
      {
         Pprime_ptr->put(i,j,AF.get(i,j));
//         Pprime_ptr->put(i,j,AF.get(i,j)+outerproduct.get(i,j));
      }
      Pprime_ptr->put(i,3,lambda*eprime.get(i));
   }

//      cout << "eprime = " << eprime << endl;
//      cout << "Pprime = " << *Pprime_ptr << endl;

   return Pprime_ptr;
}

// ==========================================================================
// Essential matrix member functions
// ==========================================================================

void fundamental::set_E_ptr(genmatrix* E_ptr)
{
   delete this->E_ptr;
   this->E_ptr=new genmatrix(3,3);
   *(this->E_ptr)=*E_ptr;
}

// ---------------------------------------------------------------------
// Member function generate_essential_matrix() takes in 3x3 intrinsic
// camera matrices K_xy and K_uv.  It instantiates and returns the
// essential matrix *E_ptr corresponding to fundamental matrix *F_ptr.

genmatrix* fundamental::generate_essential_matrix(
   const genmatrix* K_xy_ptr,const genmatrix* K_uv_ptr)
{
   cout << "inside fundamental::generate_essential_matrix()" << endl;
   
   delete E_ptr;
   E_ptr=new genmatrix(3,3);
   *E_ptr=K_xy_ptr->transpose() * (*F_ptr) * (*K_uv_ptr);
   cout << "Raw *E_ptr = " << *E_ptr << endl;

   correct_essential_matrix_singular_values();

   return E_ptr;
}

// ---------------------------------------------------------------------
// Member function correct_essential_matrix_singular_values() forces
// the two nonzero singular values of *E_ptr to be equal and the 3rd
// singular value to precisely equal 0.  Since *E_ptr is only defined
// up to a scale factor, its nonzero singular values are subsequently
// set equal to 1. 

bool fundamental::correct_essential_matrix_singular_values()

{
   cout << "inside fundamental::correct_essential_matrix_singular_values()" << endl;
   
   genmatrix U(3,3),W(3,3),V(3,3);
   if (!E_ptr->sorted_singular_value_decomposition(U,W,V)) return false;
   cout << "W = " << W << endl;

   double avg_singular_value=0.5*(W.get(0,0)+W.get(1,1));
   cout << "Average singular value = " << avg_singular_value << endl;

// Recall essential matrix is only defined up to a scale.  So without
// we may reset avg_singular_value to unity:

   avg_singular_value=1;
   W.put(0,0,avg_singular_value);
   W.put(1,1,avg_singular_value);
   W.put(2,2,0);

   *E_ptr=U*W*V.transpose();

   cout << "Corrected *E_ptr = " << *E_ptr << endl;
//   outputfunc::enter_continue_char();
   return true;
}

// ---------------------------------------------------------------------
// Member function compute_four_relative_projection_matrix_candidates()

bool fundamental::compute_four_relative_projection_matrix_candidates(
   vector<genmatrix*> projection_matrix_ptrs)
{
   cout << "inside fundamental::compute_four_relative_projection_matrix_candidates()" << endl;

   genmatrix U(3,3),W(3,3),V(3,3);
   if (!E_ptr->sorted_singular_value_decomposition(U,W,V)) return false;
   
   W.clear_values();
   W.put(0,1,-1);
   W.put(1,0,1);
   W.put(2,2,1);
   
   threevector u3;
   U.get_column(2,u3);
   cout << "u3 = " << u3 << endl;

// Recall essential matrix *E_ptr is only known up to a scale factor.
// So we are free to multiple *E_ptr by -1.  We do so to guarantee
// rotation matrices R1 and R2 have determinant = +1:

   double det_sgn=U.determinant()*V.determinant();
   cout << "det sgn = " << det_sgn << endl;

   rotation R1,R2;
   R1=det_sgn*U*W*V.transpose();
   R2=det_sgn*U*W.transpose()*V.transpose();

   cout << "R1 = " << R1 << endl;
   cout << "R2 = " << R2 << endl;
   cout << "R1.determinant() = " << R1.determinant() << endl;
   cout << "R2.determinant() = " << R2.determinant() << endl;

   for (unsigned int r=0; r<3; r++)
   {
      for (unsigned int c=0; c<3; c++)
      {
         projection_matrix_ptrs[0]->put(r,c,R1.get(r,c));
         projection_matrix_ptrs[1]->put(r,c,R1.get(r,c));
         projection_matrix_ptrs[2]->put(r,c,R2.get(r,c));
         projection_matrix_ptrs[3]->put(r,c,R2.get(r,c));
      } // loop over index c labeling columns
      projection_matrix_ptrs[0]->put(r,3,u3.get(r));
      projection_matrix_ptrs[1]->put(r,3,-u3.get(r));
      projection_matrix_ptrs[2]->put(r,3,u3.get(r));
      projection_matrix_ptrs[3]->put(r,3,-u3.get(r));
   } // loop over index r labeling rows
   return true;
}

// ---------------------------------------------------------------------
// Member function Horns_decomposition() implements B. Horn's 
// decomposition of an essential matrix into a baseline translation
// and rotation.  The method is reported in "Recovering Baseline and
// Orientation from 'Essential Matrix'" (1990) by Horn.

void fundamental::Horns_decomposition(
   double sgn_E,double sgn_b,rotation& R,threevector& b)
{
   cout << "inside fundamental::Horns_decomposition()" << endl;

   genmatrix* signed_E_ptr=new genmatrix(3,3);
   *signed_E_ptr = (sgn_E) * (*E_ptr);

   genmatrix I(3,3),EEtrans(3,3),term1(3,3),bbtrans(3,3);
   
   I.identity();
   EEtrans=(*signed_E_ptr) * (signed_E_ptr->transpose());
//   cout << "EEtrans = " << EEtrans << endl;
   
   double alpha=0.5*EEtrans.trace();
   term1=alpha*I;
//   cout << "term1 = " << term1 << endl;
   bbtrans=term1-EEtrans;
//   cout << "bbtrans = " << bbtrans << endl;

   double r0=sqrt(fabs(bbtrans.get(0,0)));
   double r1=sqrt(fabs(bbtrans.get(1,1)));
   double r2=sqrt(fabs(bbtrans.get(2,2)));

   int sgn01=bbtrans.get(0,1)/fabs(bbtrans.get(0,1));
   int sgn02=bbtrans.get(0,2)/fabs(bbtrans.get(0,2));
//   int sgn12=bbtrans.get(1,2)/fabs(bbtrans.get(1,2));
//   cout << "sgn01 = " << sgn01
//        << " sgn02 = " << sgn02
//        << " sgn12 = " << sgn12 << endl;

   int sgn0=1;
   int sgn1=sgn01/sgn0;
   int sgn2=sgn02/sgn0;
   
   double b0=sgn0*r0;
   double b1=sgn1*r1;
   double b2=sgn2*r2;
   
   b=sgn_b*threevector(b0,b1,b2);
//   cout << "b vector = " << b << endl;
//   cout << "b bTrans = " << b.outerproduct(b) << endl;

   genmatrix B(3,3);
   B.clear_values();
   B.put(0,1,-b2);
   B.put(1,2,-b0);
   B.put(2,0,-b1);
   B.put(1,0,b2);
   B.put(2,1,b0);
   B.put(0,2,b1);
//   cout << "B = " << B << endl;

   threevector e0,e1,e2;
   signed_E_ptr->get_column(0,e0);
   signed_E_ptr->get_column(1,e1);
   signed_E_ptr->get_column(2,e2);
   
   threevector cross12=e1.cross(e2);
   threevector cross20=e2.cross(e0);
   threevector cross01=e0.cross(e1);
   
   genmatrix cofactors(3,3);
   cofactors.put_row(0,cross12);
   cofactors.put_row(1,cross20);
   cofactors.put_row(2,cross01);
   
   R=( cofactors.transpose()-B * (*signed_E_ptr) ) / (b.sqrd_magnitude());

//   cout << "R = " << R << endl;
   cout << "R.det = " << R.determinant() << endl;

   cout << "*signed_E_ptr = " << *signed_E_ptr << endl;
   cout << "B*R = " << B*R << endl;

   cout << "E - B*R = " << *signed_E_ptr - B*R << endl;
   delete signed_E_ptr;
}

// ==========================================================================
// Triangulation member functions
// ==========================================================================

// Member function correct_tiepoint_coordinates() implements algorithm 12.1
// from Hartley and Zisseman, "Multi-view geometry for computer
// vision", 2nd edition.  It takes in a set of corresponding 2D
// tiepoints curr_XY and curr_UV which are assumed to be noisy.  In
// contrast, the current fundamental matrix F is assumed to NOT be
// noisy.  This method computes corrected tiepoints which minimize the 
// geometric error function d(x,xhat)**2 + d(x',xhat')**2 and
// (nearly exactly) satisfy xhat'^T * F * xhat = 0. 

void fundamental::correct_tiepoint_coordinates(
   const twovector& curr_XY,const twovector& curr_UV,
   twovector& corrected_XY,twovector& corrected_UV)
{
//   cout << "inside fundamental::correct_tiepoint_coordinates()" << endl;
  
// First translate tiepoints to 2D origin:
 
   genmatrix T_XY(3,3),T_XY_inverse(3,3);
   genmatrix T_UV(3,3),T_UV_inverse(3,3);

   T_XY.identity();
   T_UV.identity();
   
   T_XY.put(0,2,-curr_XY.get(0));
   T_XY.put(1,2,-curr_XY.get(1));

   T_UV.put(0,2,-curr_UV.get(0));
   T_UV.put(1,2,-curr_UV.get(1));

   T_XY.inverse(T_XY_inverse);
   T_UV.inverse(T_UV_inverse);
   
// Store current fundamental matrix values.  Then work with translated
// version of *F_ptr:

   genmatrix F_orig(3,3);
   F_orig=*F_ptr;

   *F_ptr=T_XY_inverse.transpose() * (*F_ptr) * T_UV_inverse;

// Compute normalized left and right epipoles:

   threevector eprime=get_epipole_XY();
   threevector e=get_epipole_UV();
   
   e /= sqrt(sqr(e.get(0))+sqr(e.get(1)));
   eprime /= sqrt(sqr(eprime.get(0))+sqr(eprime.get(1)));

//   cout << "e = " << e << endl;
//   cout << "e.get(0)**2+e.get(1)**2 = "
//        << sqr(e.get(0))+sqr(e.get(1)) << endl;

//   cout << "eprime = " << eprime << endl;
//   cout << "eprime.get(0)**2+eprime.get(1)**2 = "
//        << sqr(eprime.get(0))+sqr(eprime.get(1)) << endl;

// Rotate fundamental matrix:
   
   rotation R,Rprime;
   R.clear_values();
   R.put(0,0,e.get(0));
   R.put(0,1,e.get(1));
   R.put(1,0,-e.get(1));
   R.put(1,1,e.get(0));
   R.put(2,2,1);

   Rprime.clear_values();
   Rprime.put(0,0,eprime.get(0));
   Rprime.put(0,1,eprime.get(1));
   Rprime.put(1,0,-eprime.get(1));
   Rprime.put(1,1,eprime.get(0));
   Rprime.put(2,2,1);

//   cout << "R = " << R << endl;
//   cout << "R*RT = " << R*R.transpose() << endl;
//   cout << "R*e = " << R*e << endl;

//   cout << "Rprime = " << Rprime << endl;
//   cout << "Rprime*RprimeT = " << Rprime*Rprime.transpose() << endl;
//   cout << "Rprime*eprime = " << Rprime*eprime << endl;

   *F_ptr=Rprime * (*F_ptr) * R.transpose();
   
//   cout << "Rotated F = " << *F_ptr << endl;

   double f=e.get(2);
   double fprime=eprime.get(2);
   double a=F_ptr->get(1,1);
   double b=F_ptr->get(1,2);
   double c=F_ptr->get(2,1);
   double d=F_ptr->get(2,2);
  
/* 
   genmatrix Fnew(3,3);
   Fnew.put(0,0,f*fprime*d);
   Fnew.put(0,1,-fprime*c);
   Fnew.put(0,2,-fprime*d);

   Fnew.put(1,0,-f*b);
   Fnew.put(1,1,a);
   Fnew.put(1,2,b);

   Fnew.put(2,0,-f*d);
   Fnew.put(2,1,c);
   Fnew.put(2,2,d);
//   cout << "Fnew = " << Fnew << endl;
//   cout << "Fnew.det = " << Fnew.determinant() << endl;
*/

//   vector<complex> eigenvalues=complexfunc::eigen_decomposition(F_ptr);
//   for (unsigned int i=0; i<eigenvalues.size(); i++)
//   {
//      cout << "i = " << i << " eigenvalue = " << eigenvalues[i]
//           << endl;
//   }

// Set up 6th order polynomial g and find its real roots:

   vector<double> g_roots=find_real_g_poly_roots(a,b,c,d,f,fprime);

// Evaluate cost function s at g-poly's roots:

   double t_min=minimize_s_cost_function(a,b,c,d,f,fprime,g_roots);
//   cout << "t_min = " << t_min << endl;

   threevector x_hat,xprime_hat;
   find_corrected_correspondences(t_min,a,b,c,d,f,fprime,x_hat,xprime_hat);

// Transform corrected tiepoints back to original coordinates:

   x_hat=T_UV_inverse*R.transpose()*x_hat;
   xprime_hat=T_XY_inverse*Rprime.transpose()*xprime_hat;

// Renormalize x_hat and xprime_hat so that their 3rd components equal
// unity:

   x_hat /= x_hat.get(2);
   xprime_hat /= xprime_hat.get(2);

   corrected_UV=x_hat;
   corrected_XY=xprime_hat;

// Reset *F_ptr back to its original values before this method was called:

   *F_ptr = F_orig;

// Compare scalar products of original and corrected tiepoint pairs
// with original fundamental matrix:

//   threevector Fx = (*F_ptr) * threevector(curr_UV,1);
//   double original_scalar_product=threevector(curr_XY,1).dot(Fx);
//   cout << "Original scalar product: xprime . F . x = " 
//        << original_scalar_product << endl;

//   threevector Fx_hat=(*F_ptr) * x_hat;
//   double corrected_scalar_product=xprime_hat.dot(Fx_hat);
//   cout << "Corrected scalar product: xprime_hat . F . x_hat = " 
//        << corrected_scalar_product << endl;
}

// ---------------------------------------------------------------------
// Member function g() is a little utility function which implements
// the polynomial in eqn 12.7 of Hartley and Zisseman, 2nd edition.

double fundamental::g(
   double t,double a,double b,double c,double d,double f,double fprime) const
{
   double term1=sqr(a*t+b);
   double term2=sqr(fprime)*sqr(c*t+d);
   double term3=t*sqr(term1+term2);
   
   double term4=a*d-b*c;
   double term5=sqr(1+sqr(f*t));
   double term6=(a*t+b)*(c*t+d);
   double term7=term4*term5*term6;
   
   double g=term3-term7;
   return g;
}

// ---------------------------------------------------------------------
// Member function find_real_g_poly_roots() 

vector<double> fundamental::find_real_g_poly_roots(
   double a,double b,double c,double d,double f,double fprime) const
{
   genmatrix M(7,7),Minverse(7,7);
   for (unsigned int r=0; r<7; r++)
   {
      M.put(r,0,1);
      for (unsigned int c=1; c<7; c++)
      {
         M.put(r,c,pow(r,c));
      } // loop over index c labeling columns
   } // loop over index r labeling rows
//   cout << "det(M) = " << M.determinant() << endl;
   
   M.inverse(Minverse);
//   cout << "M = " << M << endl;
//   cout << "Minv = " << Minverse << endl;
//   cout << "M*Minv = " << M*Minverse << endl;

   genvector gvec(7),poly_coeffs(7);
   for (unsigned int r=0; r<7; r++)
   {
      gvec.put(r,g(r,a,b,c,d,f,fprime));
   }

   poly_coeffs=Minverse*gvec;
//   cout << "poly_coeffs = " << poly_coeffs << endl;

   vector<double> coeffs;
   for (unsigned int r=0; r<7; r++)
   {
      coeffs.push_back(poly_coeffs.get(r));
   }

   mypolynomial g_poly(6,coeffs);
//   cout << "poly = " << g_poly << endl;

   vector<double> real_poly_roots=complexfunc::find_real_polynomial_roots(
      g_poly);

//   for (unsigned int i=0; i<real_poly_roots.size(); i++)
//   {
//      cout << "i = " << i 
//           << " real poly root = " << real_poly_roots[i]
//           << " g(root) = " << g_poly.value(real_poly_roots[i])
//           << endl;
//   }
   
   return real_poly_roots;
}

// ---------------------------------------------------------------------
// Member function minimize_s_cost_function()

double fundamental::minimize_s_cost_function(
   double a,double b,double c,double d,double f,double fprime,
   vector<double>& g_roots) const
{
//   cout << "inside fundamental::minimize_s_cost_function()" << endl;
   
   vector<double> s_values,t_values;
   for (unsigned int i=0; i<g_roots.size(); i++)
   {
      t_values.push_back(g_roots[i]);
      s_values.push_back(s_cost_function(t_values.back(),a,b,c,d,f,fprime));
   }

   double term1=1/sqr(f);
   double term2=sqr(c)/(sqr(a)+sqr(fprime*c));
   double s_infinity=term1+term2;
   s_values.push_back(s_infinity);
   t_values.push_back(POSITIVEINFINITY);

   templatefunc::Quicksort(s_values,t_values);

//   for (unsigned int i=0; i<s_values.size(); i++)
//   {
//      cout << "i = " << i
//           << " s(t) = " << s_values[i]
//           << " t = " << t_values[i] << endl;
//   }
   
   return t_values.front();
}

// ---------------------------------------------------------------------
// Member function s_cost_function()

double fundamental::s_cost_function(
   double t,double a,double b,double c,double d,double f,double fprime) const
{
   double term1=sqr(t)/(1+sqr(f*t));
   double numer2=sqr(c*t+d);
   double denom2=sqr(a*t+b)+sqr(fprime)*sqr(c*t+d);
   double term2=numer2/denom2;
   double s=term1+term2;
   return s;
}

// ---------------------------------------------------------------------
// Member function find_corrected_correspondences()

void fundamental::find_corrected_correspondences(
   double t_min,double a,double b,double c,double d,double f,double fprime,
   threevector& xhat,threevector& xprime_hat) const
{
   threevector l(t_min*f,1,-t_min);
   threevector lprime(-fprime*(c*t_min+d),a*t_min+b,c*t_min+d);

   xhat=geometry_func::closest_point_to_origin(l);
   xprime_hat=geometry_func::closest_point_to_origin(lprime);
}

// ---------------------------------------------------------------------
// Member function triangulate() takes in a pair of corrected 2D
// tiepoints UV and XY which are assumed to nearly exactly satisfy
// curr_XY * F * curr_UV = 0.  It implements the homogeneous (DLT)
// method described in section 12.2 of Hartley and Zisserman,
// "Multi-view geometry in computer vision", 2nd edition to return the
// threevector corresponding to the triangulated location of the two
// corrected tiepoints.

threevector fundamental::triangulate(
   const twovector& corrected_UV,const twovector& corrected_XY)
{
//   cout << "inside fundamental::triangulate()" << endl;
   
   fourvector p1,p2,p3,p1_prime,p2_prime,p3_prime;
   P_ptr->get_row(0,p1);
   P_ptr->get_row(1,p2);
   P_ptr->get_row(2,p3);
   Pprime_ptr->get_row(0,p1_prime);
   Pprime_ptr->get_row(1,p2_prime);
   Pprime_ptr->get_row(2,p3_prime);
 
//   cout << "p1 = " << p1 << endl;
//   cout << "p2 = " << p2 << endl;
//   cout << "p3 = " << p3 << endl;
   
//   cout << "p1_prime = " << p1_prime << endl;
//   cout << "p2_prime = " << p2_prime << endl;
//   cout << "p3_prime = " << p3_prime << endl;

   genmatrix A(4,4);
   vector<fourvector> A_rows;
   A_rows.push_back(corrected_UV.get(0)*p3-p1);
   A_rows.push_back(corrected_UV.get(1)*p3-p2);
   A_rows.push_back(corrected_XY.get(0)*p3_prime-p1_prime);
   A_rows.push_back(corrected_XY.get(1)*p3_prime-p2_prime);
   for (unsigned int r=0; r<A_rows.size(); r++)
   {
      A.put_row(r,A_rows[r]);
   }
//   cout << "A = " << A << endl;
//   cout << "A.det = " << A.determinant() << endl;
//   cout << "A.rank() = " << A.rank() << endl;

//   vector<complex> eigenvalues=complexfunc::eigen_decomposition(&A);
//   for (unsigned int c=0; c<eigenvalues.size(); c++)
//  {
//      cout << "c = " << c << " A eigenvalue = " << eigenvalues[c]
//           << endl;
//   }

   fourvector X;
   A.homogeneous_soln(X);
   X /= X.get(3);

//   cout << "X = " << X << endl;
//   cout << "A*X = " << A*X << endl;

   threevector PX=(*P_ptr) * X;
   PX /= PX.get(2);
//   cout << "P * X = " << PX << endl;
//   cout << "corrected_UV = " << corrected_UV << endl;

   threevector PprimeX=(*Pprime_ptr) * X;
   PprimeX /= PprimeX.get(2);
//   cout << "Pprime * X = " << PprimeX << endl;
//   cout << "corrected_XY = " << corrected_XY << endl;

   return threevector(X);
}

// ---------------------------------------------------------------------
// Member function triangulate_noisy_tiepoints() takes in 
// a pair of 2D tiepoints UV and XY that only approximately satisfy XY
// * F * UV = 0 .  We assume that *P_ptr and *Pprime_ptr corresponding
// to the current fundamental matrix have already been calculated.
// This method first computes a corrected set of tiepoints that
// nearly precisely obey the epipolar relation.  It then returns the
// corrected tiepoints' 3D progenitor.

threevector fundamental::triangulate_noisy_tiepoints(
   const twovector& UV,const twovector& XY)
{
//   cout << "inside fundamental::triangulate_noisy_tiepoints()" << endl;

   twovector corrected_XY,corrected_UV;
   correct_tiepoint_coordinates(XY,UV,corrected_XY,corrected_UV);
   return triangulate(corrected_UV,corrected_XY);
}

// ---------------------------------------------------------------------
// Member function reprojection_error()

double fundamental::reprojection_error(
   const twovector& curr_XY,const twovector& curr_UV,
   const twovector& corrected_XY,const twovector& corrected_UV)
{
   twovector delta_XY=curr_XY-corrected_XY;
   twovector delta_UV=curr_UV-corrected_UV;
//   return delta_XY.sqrd_magnitude()+delta_UV.sqrd_magnitude();
   return sqrt(delta_XY.sqrd_magnitude()+delta_UV.sqrd_magnitude());
}

double fundamental::reprojection_error(
   const twovector& curr_XY,const twovector& curr_UV)
{
   twovector corrected_XY,corrected_UV;
   correct_tiepoint_coordinates(curr_XY,curr_UV,corrected_XY,corrected_UV);
   return reprojection_error(curr_XY,curr_UV,corrected_XY,corrected_UV);
}

// ---------------------------------------------------------------------
// Member function sampson_error() implements the Sampson cost
// function which provides a first-order approximation to the
// geometric error.  See eqn 11.9 in 2nd edition of Hartley and
// Zisserman.

double fundamental::sampson_error(
   const twovector& curr_XY,const twovector& curr_UV)
{
   double X=curr_XY.get(0);
   double Y=curr_XY.get(1);
   double U=curr_UV.get(0);
   double V=curr_UV.get(1);

   double F00=F_ptr->get(0,0);
   double F01=F_ptr->get(0,1);
   double F02=F_ptr->get(0,2);
   double F10=F_ptr->get(1,0);
   double F11=F_ptr->get(1,1);
   double F12=F_ptr->get(1,2);
   double F20=F_ptr->get(2,0);
   double F21=F_ptr->get(2,1);
   double F22=F_ptr->get(2,2);

   double XY_new_0=X*F00+Y*F10+F20;
   double XY_new_1=X*F01+Y*F11+F21;
//   double XY_new_2=X*F02+Y*F12+F22;

   double UV_new_0=F00*U+F01*V+F02;
   double UV_new_1=F10*U+F11*V+F12;
   double UV_new_2=F20*U+F21*V+F22;

   double numer=sqr(X*UV_new_0+Y*UV_new_1+UV_new_2);
   double denom=sqr(XY_new_0)+sqr(XY_new_1)+sqr(UV_new_0)+sqr(UV_new_1);
   double sampson_error=numer/denom;

   return sampson_error;
}

// ==========================================================================
// Import/export member functions
// ==========================================================================

void fundamental::export_best_matrix(int i,int j,string output_filename)
{
//   cout << "inside fundamental::export_best_matrix(), i = " << i 
//        << " j = " << j << endl;
// Reset any entry in *Fbest_ptr to zero if its value lies very close to 0:

   for (unsigned int m=0; m<Fbest_ptr->get_mdim(); m++)
   {
      for (unsigned int n=0; n<Fbest_ptr->get_ndim(); n++)
      {
         double curr_value=Fbest_ptr->get(m,n);
         if (nearly_equal(curr_value,0,1E-9))
         {
            Fbest_ptr->put(m,n,0);
         }
      }
   }
   renormalize_Fbest_entries();
//   cout << "*Fbest_ptr = " << *Fbest_ptr << endl;

   Fbest_ptr->export_to_dense_text_format(output_filename);
}

void fundamental::import_matrix(string input_filename)
{
}

