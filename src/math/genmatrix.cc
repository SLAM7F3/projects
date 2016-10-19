// ==========================================================================
// Genmatrix class member function definitions
// ==========================================================================
// Last modified on 10/4/16; 10/12/16; 10/17/16; 10/19/16
// =========================================================================

#include <Eigen/Dense>
#include <vector>

#include "math/constants.h"
#include "general/filefuncs.h"
#include "math/genmatrix.h"
#include "math/genvector.h"
#include "templates/mytemplates.h"
#include "newmat/newmatap.h"
#include "numrec/nr.h"
#include "numrec/nrutil.h"
#include "general/outputfuncs.h"
#include "math/rotation.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/threevector.h"

using std::cout;
using std::endl;
using std::ostream;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void genmatrix::allocate_member_objects()
{
}		       

void genmatrix::initialize_member_objects()
{
}

genmatrix::genmatrix(int m):
   genarray(m)
{
   allocate_member_objects();
   initialize_member_objects();
   if (m <= 0)
   {
      cout << "Error in genmatrix constructor #1, m = " << m << endl;
   }
}

genmatrix::genmatrix(int m,int n):
   genarray(m,n)
{
   allocate_member_objects();
   initialize_member_objects();

   if (m <=0 || n <=0)
   {
      cout << "Error in genmatrix constructor #2" << endl;
      cout << "m = " << m << " n = " << n << endl;
   }
}

// ---------------------------------------------------------------------
genmatrix::genmatrix(const rotation& R):
   genarray(3,3)
{
   allocate_member_objects();
   initialize_member_objects();

   for (unsigned int i=0; i<3; i++)
   {
      for (unsigned int j=0; j<3; j++)
      {
         put(i,j,R.get(i,j));
      }
   }
}

// ---------------------------------------------------------------------
genmatrix::genmatrix(const tensor& T):
   genarray(T.get_dim(0),T.get_dim(1))
{
//    cout << "inside genmatrix(Tensor) constructor" << endl
   allocate_member_objects();
   initialize_member_objects();

   if (T.get_rank() > 2)
   {
      cout << "Error in genmatrix(tensor) constructor!" << endl;
      cout << "Tensor rank = " << T.get_rank() << endl;
      exit(-1);
   }
   else
   {
      for (unsigned int i=0; i<T.get_dim(0); i++)
      {
         for (unsigned int j=0; j<T.get_dim(1); j++)
         {
            put(i,j,T.get(i,j));
         }
      }
   }
}

// ---------------------------------------------------------------------
genmatrix::genmatrix(const genvector& V):
  genarray(V.get_mdim())
{
   allocate_member_objects();
   initialize_member_objects();

   for (unsigned int i=0; i<V.get_mdim(); i++)
   {
      put(i,0,V.get(i));
   }
}

// ---------------------------------------------------------------------
// Copy constructor

genmatrix::genmatrix(const genmatrix& m):
   genarray(m)
{
//   cout << "inside genmatrix copy constructor" << endl;
   docopy(m);
}

genmatrix::~genmatrix()
{
}

// ---------------------------------------------------------------------
void genmatrix::docopy(const genmatrix& m)
{
//   cout << "inside genmatrix::docopy()" << endl;
   genarray::docopy(m);
}	

// Overload = operator:

genmatrix& genmatrix::operator= (const genmatrix& m)
{
//   cout << "inside genmatrix::operator==" << endl;
   if (this==&m) return *this;
   genarray::operator=(m);
   docopy(m);
   return *this;
}

// Overload << operator:

ostream& operator<< (ostream& outstream,const genmatrix& A)
{
   outstream << endl;


   for (unsigned int i=0; i<A.mdim; i++)
   {
      for (unsigned int j=0; j<A.ndim; j++)
      {
         outstream << A.get(i,j) 
//                   << " ";
                   << "\t";
      }
      outstream << endl;
   }
   return(outstream);
}

// ==========================================================================
// Set and get methods

// Member function get_row [get_column] fills a 1xndim [mdimx1]
// genvector containing the contents of the current genmatrix object's
// mth row [nth column].

genvector genmatrix::get_row(int m) const
{
   genvector row(ndim);
   const double *a = get_e_ptr() + m * ndim;
   const double *aMax = a + ndim;

   int c = 0;
   while(a < aMax)
   {
      row.put(c++, *a++);
   }
   
   return row;
}

void genmatrix::get_row(int m,genvector& row) const
{
   const double *a = get_e_ptr() + m * ndim;
   const double *aMax = a + ndim;

   int c = 0;
   while(a < aMax)
   {
      row.put(c++, *a++);
   }
}

void genmatrix::put_row(int m,const genvector& row) 
{
   for (unsigned int n=0; n<ndim; n++)
   {
      put(m,n,row.get(n));
   }
}

void genmatrix::renormalize_row(int m)
{
   put_row(m, get_row(m).unitvector());
}

genvector genmatrix::get_column(int n) const
{
   genvector column(mdim);

   const double* a = get_e_ptr() + n;
   const double *aMax = a + ndim*mdim;

   int c = 0;
   while(a < aMax)
   {
      column.put(c++, *a);
      a += ndim;
   }

   return column;
}

void genmatrix::get_column(int n,genvector& column) const
{
   const double* a = get_e_ptr() + n;
   const double *aMax = a + ndim*mdim;

   int c = 0;
   while(a < aMax)
   {
      column.put(c++, *a);
      a += ndim;
   }
}

void genmatrix::put_column(int n,const genvector& column) 
{
   for (unsigned int m=0; m<mdim; m++)
   {
      put(m,n,column.get(m));
   }
}

void genmatrix::renormalize_column(int n)
{
   put_row(n, get_column(n).unitvector());
}



void genmatrix::put_smaller_column(int n,const genvector& column) 
{
   put_smaller_column(0,n,column);
}

void genmatrix::put_smaller_column(int m_start,int n,const genvector& column) 
{
   for (unsigned int m=0; m<column.get_mdim(); m++)
   {
      put(m+m_start,n,column.get(m));
   }
}

// Member function put_smaller_matrix() copies the contents of input
// matrix M into *this where the M's upper left entry is placed into
// m=m_start and n=n_start:

void genmatrix::put_smaller_matrix(const genmatrix& M)
{
   put_smaller_matrix(0,0,M);
}

void genmatrix::put_smaller_matrix(
   int m_start,int n_start,const genmatrix& M)
{
   for (unsigned int m=0; m<M.get_mdim(); m++)
   {
      for (unsigned int n=0; n<M.get_ndim(); n++)
      {
         put(m+m_start,n+n_start,M.get(m,n));
      }
   }
}

// Member function get_smaller_matrix() takes in genmatrix M whose
// mdim <= this->mdim and ndim <= this->ndim.  It copies the reduced
// rows and columns of *this into M.

void genmatrix::get_smaller_matrix(genmatrix& M)
{
   for (unsigned int m=0; m<M.get_mdim(); m++)
   {
      for (unsigned int n=0; n<M.get_ndim(); n++)
      {
         M.put(m,n,get(m,n));
      }
   }
}

double genmatrix::rows_dotproduct(int r1, int r2)
{
   double* a = get_e_ptr() + r1 * ndim;
   double* b = get_e_ptr() + r2 * ndim;
   double *aMax = a + ndim;

   double dotproduct = 0;
   while(a < aMax)
   {
      dotproduct += (*a++) * (*b++);
   }
   
   return dotproduct;
}

double genmatrix::columns_dotproduct(int c1, int c2)
{
   double* a = get_e_ptr() + c1;
   double* b = get_e_ptr() + c2;
   double *aMax = a + ndim*mdim;

   double dotproduct = 0;
   while(a < aMax)
   {
      dotproduct += (*a) * (*b);
      a += ndim;
      b += ndim;
      
   }
   return dotproduct;
}

// ==========================================================================
// Basic matrix manipulation member functions:
// ==========================================================================

void genmatrix::clear_matrix_values()
{
   for (unsigned int j=0; j<ndim; j++)
   {
      for (unsigned int i=0; i<mdim; i++)
      {
         put(i,j,0);
      }
   }
}

genmatrix genmatrix::transpose() const
{
   genmatrix Atrans(ndim,mdim);
   
   for (unsigned int i=0; i<ndim; i++)
   {
      for (unsigned int j=0; j<mdim; j++)
      {
         Atrans.put(i,j,get(j,i));
      }
   }
   return Atrans;
}

// ---------------------------------------------------------------------
genmatrix genmatrix::absolute_value() const
{
   genmatrix A_abs(ndim,mdim);
   
   for (unsigned int i=0; i<ndim; i++)
   {
      for (unsigned int j=0; j<mdim; j++)
      {
         A_abs.put(i,j,fabs(get(j,i)));
      }
   }
   return A_abs;
}

// ---------------------------------------------------------------------
void genmatrix::identity()
{
   if (mdim==ndim)
   {
      clear_values();
      for (unsigned int i=0; i<mdim; i++)
      {
         put(i,i,1);
      }
   }
   else
   {
      cout << "Error inside genmatrix::identity()!" << endl;
      cout << "mdim = " << mdim << " ndim = " << ndim << endl;
      cout << "Identity is ill-defined when mdim != ndim" << endl;
   }
}

// ---------------------------------------------------------------------
// Member functions commutator and anticommutator respectively compute
// [*this,A] and {*this,A} : 

genmatrix genmatrix::commutator(const genmatrix& A) const
{
   if (mdim==A.mdim && ndim==A.ndim)
   {
      genmatrix C(mdim,ndim);
      C=*this*A-A*(*this);
      return C;
   }
   else
   {
      cout << "Error inside genmatrix::commutator() !" << endl;
      cout << "mdim = " << mdim << " ndim = " << ndim << endl;
      cout << "Commutator is ill-defined when mdim != ndim" << endl;
      exit(-1);
   }
}

genmatrix genmatrix::anticommutator(const genmatrix& A) const
{
   if (mdim==A.mdim && ndim==A.ndim)
   {
      genmatrix C(mdim,ndim);
      C=*this*A+A*(*this);
      return C;
   }
   else
   {
      cout << "Error inside genmatrix::anticommutator() !" << endl;
      cout << "mdim = " << mdim << " ndim = " << ndim << endl;
      cout << "Anticommutator is ill-defined when mdim != ndim" << endl;
      exit(-1);
   }
}

// ---------------------------------------------------------------------
double genmatrix::trace() const
{
   if (mdim==ndim)
   {
      double tr=0;
   
      for (unsigned int i=0; i<mdim; i++)
      {
         tr += get(i,i);
      }
      return tr;
   }
   else
   {
      cout << "Error in genmatrix::trace()!" << endl;
      cout << "mdim = " << mdim << " ndim = " << ndim << endl;
      cout << "Trace is ill-defined when mdim != ndim" << endl;
      return NEGATIVEINFINITY;
   }
}

// ---------------------------------------------------------------------
genmatrix genmatrix::power(unsigned int n)
{
   if (mdim==ndim)
   {
      genmatrix B(mdim,ndim);

      B.identity();
      for (unsigned int i=0; i<n; i++)
      {
         B=B*(*this);
      }
      return B;
   }
   else
   {
      cout << "Error in genmatrix::power()!" << endl;
      cout << "mdim = " << mdim << " ndim = " << ndim << endl;
      cout << "Matrix power is ill-defined when mdim != ndim" << endl;
      exit(-1);
   }
}

// ---------------------------------------------------------------------
// Member function elementwise_product returns N_ij = A_ij * M_ij
// where no sum on repeated i and j indices is performed.  This
// multiplication of two matrices is also known as a Hadamard product.

genmatrix genmatrix::elementwise_product(const genmatrix& M)
{
   genmatrix product(mdim,ndim);
   if (mdim != M.get_mdim() || ndim != M.get_ndim())
   {
      cout << "Error in genmatrix::elementwise_product()!" << endl;
      cout << "mdim = " << mdim << " ndim = " << ndim << endl;
      cout << "M.mdim = " << M.get_mdim() << " M.ndim = " << M.get_ndim()
           << endl;
   }
   else
   {
      for (unsigned int m=0; m<mdim; m++)
      {
         for (unsigned int n=0; n<ndim; n++)
         {
            product.put(m,n,get(m,n)*M.get(m,n));
         } // loop over index n
      } // loop over index m
   }
   return product;
}

// ---------------------------------------------------------------------
// Member function elementwise_power sets *this_ij = M_ij**alpha where
// no sum on i and j indices is performed.

void genmatrix::elementwise_power(const genmatrix& M, double alpha)
{
   if (mdim != M.get_mdim() || ndim != M.get_ndim())
   {
      cout << "Error in genmatrix::elementwise_sqr()!" << endl;
      cout << "mdim = " << mdim << " ndim = " << ndim << endl;
      cout << "M.mdim = " << M.get_mdim() << " M.ndim = " << M.get_ndim()
           << endl;
   }
   else
   {
      for (unsigned int m=0; m<mdim; m++)
      {
         for (unsigned int n=0; n<ndim; n++)
         {
            put(m,n,pow(M.get(m,n), alpha));
         } // loop over index n
      } // loop over index m
   }
}

// ---------------------------------------------------------------------
genmatrix genmatrix::hadamard_power(double alpha)
{
   genmatrix B(mdim, ndim);
   
   for (unsigned int m=0; m<mdim; m++)
   {
      for (unsigned int n=0; n<ndim; n++)
      {
         B.put(m,n,pow(get(m,n), alpha));
      } // loop over index n
   } // loop over index m
   return B;
}

// ---------------------------------------------------------------------
// Member function hadamard_sum() adds alpha to each element of *this.

void genmatrix::hadamard_sum(double alpha) 
{
   for (unsigned int m=0; m<mdim; m++)
   {
      for (unsigned int n=0; n<ndim; n++)
      {
         put(m, n, get(m,n) + alpha);
      } // loop over index n
   } // loop over index m
}

// ---------------------------------------------------------------------
// Member function hadamard_division() divides each element of *this
// by its corresponding element in input matrix D.  

genmatrix genmatrix::hadamard_division(const genmatrix& D)
{
   genmatrix B(mdim, ndim);
   
   for (unsigned int m=0; m<mdim; m++)
   {
      for (unsigned int n=0; n<ndim; n++)
      {
         B.put(m,n,get(m,n) / D.get(m,n));
      } // loop over index n
   } // loop over index m
   return B;
}

// ---------------------------------------------------------------------
// Member function sqrd_Frobenius_norm returns the sum of every
// squared matrix element within the current genmatrix object.

double genmatrix::sqrd_Frobenius_norm()
{
   return (*this * this->transpose()).trace();
}

// ===================================================================
// Extremal element member functions
// ===================================================================

double genmatrix::max_element_value()
{
   double max_value=NEGATIVEINFINITY;
   for (unsigned int m=0; m<mdim; m++)
   {
      for (unsigned int n=0; n<ndim; n++)
      {
         double curr_value=get(m,n);
         if (curr_value > max_value) max_value=curr_value;
      }
   }
   return max_value;
}

double genmatrix::max_abs_element_value()
{
   int m_max,n_max;
   return max_abs_element_value(m_max,n_max);
}

double genmatrix::max_abs_element_value(int& m_max,int& n_max)
{
   double max_abs_value=NEGATIVEINFINITY;
   for (unsigned int m=0; m<mdim; m++)
   {
      for (unsigned int n=0; n<ndim; n++)
      {
         double curr_abs_value=fabs(get(m,n));
         if (curr_abs_value > max_abs_value) 
         {
            max_abs_value=curr_abs_value;
            m_max=m;
            n_max=n;
         }
      } // loop over index n
   } // loop over index m
   return max_abs_value;
}

// ---------------------------------------------------------------------
// Member function max_abs_element_difference scans over pairs of
// matrix elements labeled by their 1D indices I and J.  It finds the
// index values which maximizes the absolute difference between matrix
// elements.  The maximal absolute difference is returned by this
// method.

double genmatrix::max_abs_element_difference()
{
   double max_abs_diff=NEGATIVEINFINITY;
   for (unsigned int i=0; i<mdim*ndim; i++)
   {
      double value_i=get(i);
      for (unsigned int j=i+1; j<mdim*ndim; j++)
      {
         double value_j=get(j);
         double abs_diff=fabs(value_j-value_i);
         if (abs_diff > max_abs_diff)
         {
            max_abs_diff=abs_diff;
         }
      } // loop over 1D index j
   } // loop over 1D index i
   return max_abs_diff;
}

// ===================================================================
// Inversion member functions
// ===================================================================

// Member function geometric_mean_scale() iterates over all non-zero
// entries within the current genmatrix.  After computing the median
// logarithm of their absolute values, this method returns a geometric
// mean which represents a characteristic scale for the current
// genmatrix.  For determinant and matrix inverse computations, it is
// useful to first factor out the geometric mean scale if it
// significantly differs from 1.

double genmatrix::compute_geometric_mean_scale() const
{
   vector<double> log_values;

   for (unsigned int i=0; i<mdim; i++)
   {
      for (unsigned int j=0; j<mdim; j++)
      {
         double curr_value=fabs(get(i,j));
         if (curr_value > 0)
         {
            log_values.push_back(log(curr_value));
         }
      }
   }
   double median_log=mathfunc::median_value(log_values);
   double geometric_mean_scale=exp(median_log);
//   cout << "geometric_mean_scale = " << geometric_mean_scale << endl;
   return geometric_mean_scale;
}

// ---------------------------------------------------------------------
// Member function determinant calculates the determinant of the
// current matrix object.  As of Mar 2012, we call the NEWMAT11
// rather than Numerical Recipes library in order to compute matrix
// determinants.

double genmatrix::determinant() const
{
//   cout << "inside genmatrix::determinant() #1" << endl;

   if (mdim==2)
   {
      return two_determinant();
   }
   else if (mdim==3)
   {
      return three_determinant();
   }
   else
   {
      double det;
      bool det_OK_flag=determinant(det);
      if (!det_OK_flag)
      {
         cout << "Error in genmatrix::determinant() !!!" << endl;
         cout << "Determinant NOT successfully calculated" << endl;
         det=NEGATIVEINFINITY;
      }
      
      return det;
   }
}

// ---------------------------------------------------------------------
// For speed purposes, explicitly evaluate determinant when mdim==2 or
// mdim==3:

double genmatrix::two_determinant() const
{
   double a=get(0,0);
   double b=get(0,1);
   double c=get(1,0);
   double d=get(1,1);
   return a*d-b*c;
}

double genmatrix::three_determinant() const
{
   double a=get(0,0);
   double b=get(0,1);
   double c=get(0,2);

   double d=get(1,0);
   double e=get(1,1);
   double f=get(1,2);

   double g=get(2,0);
   double h=get(2,1);
   double i=get(2,2);

   return a*(e*i-f*h)-b*(d*i-f*g)+c*(d*h-e*g);
}

// ---------------------------------------------------------------------
bool genmatrix::determinant(double& d) const
{
//   cout << "inside genmatrix::determinant() #2" << endl;
//   cout << "mdim = " << mdim << " ndim = " << ndim << endl;

   double scaleless_det;
   return determinant(d,scaleless_det);
}

// ---------------------------------------------------------------------
bool genmatrix::determinant(double& d,double& scaleless_det) const
{
//   cout << "inside genmatrix::determinant() #3" << endl;
//   cout << "mdim = " << mdim << " ndim = " << ndim << endl;

   if (mdim != ndim) return false;

   double geometric_mean_scale=compute_geometric_mean_scale();

   Matrix A(mdim,ndim);		// NEWMAT11 object !
   for (unsigned int i=0; i<mdim; i++)
   {
      for (unsigned int j=0; j<ndim; j++)
      {
         A(i+1,j+1)=get(i,j)/geometric_mean_scale;
      } // loop over index j labeling columns
   } // loop over index i labeling rows

   scaleless_det=A.determinant();
   d=scaleless_det*pow(geometric_mean_scale,mdim);
   return true;
}

// ---------------------------------------------------------------------
// Member function inverse() inverts the current matrix object via
// a Singular Value Decomposition.

bool genmatrix::inverse(genmatrix& Ainv) const
{
//   cout << "inside genmatrix::inverse()" << endl;
   if (mdim != ndim) return false;

   double det,scaleless_det;
   determinant(det,scaleless_det);
   if ( fabs(scaleless_det) < 1E-50 ) return false;
//   if ( fabs(determinant()) < 1E-50 ) return false;

   genmatrix U(mdim,mdim),W(mdim,mdim),Winv(mdim,mdim),V(mdim,mdim);
   if (!sorted_singular_value_decomposition(U,W,V)) return false;
//   cout << "U = " << U << " W  = " << W << " V = " << V << endl;

   Winv.clear_values();   
   for (unsigned int m=0; m<mdim; m++)
   {
      Winv.put(m,m,1.0/W.get(m,m));
   }
//   cout << "Winv = " << Winv << endl;

   Ainv=V*Winv*U.transpose();
//   cout << "Ainv = " << Ainv << endl;

   return true;
}

// ---------------------------------------------------------------------
// Member function two_inverse() inverts a 2x2 matrix using the
// simple, closed-form solution.  

bool genmatrix::two_inverse(genmatrix& Ainv) const
{
//   cout << "inside genmatrix::inverse()" << endl;
   if (mdim != 2 || ndim != 2) return false;

   double a=get(0,0);
   double b=get(0,1);
   double c=get(1,0);
   double d=get(1,1);
   double det=a*d-b*c;
   if ( fabs(det) < 1E-50 ) return false;

   Ainv.put(0,0,d);
   Ainv.put(0,1,-b);
   Ainv.put(1,0,-c);
   Ainv.put(1,1,a);
   Ainv /= det;
//   cout << "Ainv = " << Ainv << endl;
//   cout << "this * Ainv = " << *this * Ainv << endl;

   return true;
}

// ==========================================================================
// Decomposition member functions:
// ==========================================================================

// Member function sorted_singular_value_decomposition() uses the
// NEWMAT11 library rather than Numerical Recipes implementation of
// Singular Value Decomposition.  In Jan 2006, we definitely found
// several cases where Numerical Recipes SVD results were unstable.
// So we no longer can trust our old singular_value_decomposition()
// method which calls the Numerical Recipes routines.  This method
// returns the singular values within Wsorted in descending order.

// Recall *this = A = Usorted * Wsorted * Vsorted.transpose()

// A: mdim x ndim
// Usorted: mdim x ndim
// Wsorted: ndim x ndim
// Vsorted: ndim x ndim

bool genmatrix::sorted_singular_value_decomposition(
   genmatrix& Usorted,genmatrix& Wsorted,genmatrix& Vsorted) const
{
//   cout << "inside genmatrix::sorted_singular_value_decomposition()" << endl;
//   cout << "mdim = " << mdim << " ndim = " << ndim << endl;
   if (mdim < ndim)
   {
      genmatrix transposed(ndim,mdim);
      genmatrix X(ndim,mdim);
      genmatrix Y(mdim,mdim);
      genmatrix Z(mdim,mdim);
      
      transposed=transpose();
      bool flag=
         transposed.sorted_singular_value_decomposition_w_mdim_less_than_ndim(
            X,Y,Z);
      if (!flag) return false;

      Usorted.clear_values();
      Wsorted.clear_values();
      Vsorted.clear_values();
      
      for (unsigned int m=0; m<mdim; m++)
      {
         for (unsigned int n=0; n<mdim; n++)
         {
            Usorted.put(m,n,Z.get(m,n));
         }
      }

      for (unsigned int m=0; m<mdim; m++)
      {
         for (unsigned int n=0; n<mdim; n++)
         {
            Wsorted.put(m,n,Y.get(m,n));
         }
      }

      for (unsigned int m=0; m<ndim; m++)
      {
         for (unsigned int n=0; n<mdim; n++)
         {
            Vsorted.put(m,n,X.get(m,n));
         }
      }
      return true;
   }
   else
   {
      return sorted_singular_value_decomposition_w_mdim_less_than_ndim(
         Usorted,Wsorted,Vsorted);
   }
}

bool genmatrix::sorted_singular_value_decomposition_w_mdim_less_than_ndim(
   genmatrix& Usorted,genmatrix& Wsorted,genmatrix& Vsorted) const
{
   Matrix A(mdim,ndim);
   for (unsigned int i=0; i<mdim; i++)
   {
      for (unsigned int j=0; j<ndim; j++)
      {
         A(i+1,j+1)=get(i,j);
//         cout << get(i,j) << "  ";
      } // loop over index j labeling columns
//      cout << endl;
   } // loop over index i labeling rows
   
   Matrix U,V;
   DiagonalMatrix W;

   try
   {
      SVD(A,W,U,V);
   }
   catch(...)
   {
      string banner="Exception occurred in NewMat::SVD()!";
      outputfunc::write_big_banner(banner);
      return false;
   }

   for (unsigned int i=0; i<mdim; i++)
   {
      for (unsigned int j=0; j<ndim; j++)
      {
         Usorted.put(i,j,U(i+1,j+1));
      } // loop over index j labeling columns
   } // loop over index i labeling rows

   Wsorted.clear_values();
   for (unsigned int i=0; i<ndim; i++)
   {
      Wsorted.put(i,i,W(i+1,i+1));
   } // loop over index i labeling rows

   for (unsigned int i=0; i<ndim; i++)
   {
      for (unsigned int j=0; j<ndim; j++)
      {
         Vsorted.put(i,j,V(i+1,j+1));
      } // loop over index j labeling columns
   } // loop over index i labeling rows
   
//   cout << "Usorted = " << Usorted << endl;
//   cout << "Wsorted = " << Wsorted << endl;
//   cout << "Vsorted = " << Vsorted << endl;

   genmatrix Anew(mdim,ndim);
   Anew=Usorted*Wsorted*Vsorted.transpose();
//   cout << "Anew = " << Anew << endl;
//   cout << "*this-Anew = " << *this-Anew << endl;

   return true;
}

// ---------------------------------------------------------------------
// Member function pseudo_inverse performs an SVD decomposition
// U*W*Vtrans on the current genmatrix object.  It then sets to zero
// within Winverse any singular values whose absolute values are less
// than input parameter min_abs_singular_value.  This method returns
// the matrix product V*Wpseudo_inverse*Utrans.

bool genmatrix::pseudo_inverse(
   double min_abs_singular_value,genmatrix& Apseudo_inverse) const
{
//   cout << "inside genmatrix::pseudo_inverse()" << endl;
   genmatrix U(mdim,ndim),W(ndim,ndim),V(ndim,ndim);

   if (!sorted_singular_value_decomposition(U,W,V)) return false;

//   cout << "*this = " << *this << endl;
//   cout << "W = " << W << endl;
//   cout << "U = " << U << " V = " << V << endl;
//   cout << "U*UT = " << U*U.transpose() << endl;
//   cout << "UT*U = " << U.transpose()*U << endl;
//   cout << "V*VT = " << V*V.transpose() << endl;
//   cout << "VT*V = " << V.transpose()*V << endl;
   
   genmatrix Wpseudo_inverse(ndim,ndim);
   Wpseudo_inverse.clear_values();

   for (unsigned int n=0; n<ndim; n++)
   {
      double currw=W.get(n,n);
      if (fabs(currw) > min_abs_singular_value) 
      {
         Wpseudo_inverse.put(n,n,1.0/currw);
      }
      else
      {
         Wpseudo_inverse.put(n,n,0);
      }
   }
   Apseudo_inverse=V*Wpseudo_inverse*U.transpose();

//      cout << "Wpseudo_inverse = " << Wpseudo_inverse << endl;
//      cout << "W*Wpseudo_inverse = " << W*Wpseudo_inverse << endl;
   return true;
}

// ---------------------------------------------------------------------
// This overloaded version of pseudo_inverse() works with current
// genmatrix A.  It returns (Atrans A)**-1 Atrans which yields the least-
// squares solution to A x = b.

genmatrix genmatrix::pseudo_inverse()
{
//   cout << "inside genmatrix::pseudo_inverse()" << endl;

   genmatrix trans=transpose();	// ndim x mdim
   genmatrix product=trans * (*this); // ndim x ndim
   genmatrix inv(ndim,ndim);
   product.inverse(inv);	// ndim x ndim
   genmatrix pseudo=inv*trans;	// ndim x mdim
   
//   cout << "*this = " << *this  << endl;
//   cout << "transpose = " << trans << endl;
//   cout << "trans * *this = " << product << endl;
//   cout << "product det = " << product.determinant() << endl;

//   cout << "*this = " << *this << endl;
//   cout << "pseudo = " << pseudo << endl;
//   cout << "pseudo * *this = " << pseudo * *this << endl;
//   cout << "*this * pseudo = " << *this * pseudo << endl;

   return pseudo;
}

// ---------------------------------------------------------------------
// Member function cholesky_decomposition uses the NEWMAT11 library to
// decompose a symmetric, positive-definite matrix into the product of
// a lower triangular matrix and its transpose: S = L * Ltrans.  L
// can be regarded as the "square root" of S.

bool genmatrix::cholesky_decomposition(genmatrix& L) const
{
   if (mdim != ndim)
   {
      cout << "Error in genmatrix::cholesky_decomposition()" << endl;
      cout << "mdim = " << mdim << " does not equal ndim = " << ndim
           << endl;
      return false;
   }

   SymmetricMatrix S(mdim);
   for (unsigned int i=0; i<mdim; i++)
   {
      for (unsigned int j=0; j<ndim; j++)
      {
         S(i+1,j+1)=get(i,j);
      } // loop over index j labeling columns
   } // loop over index i labeling rows
   

//   cout << "Before LTM call in genmatrix::cholesky_decomposition()" << endl;
   LowerTriangularMatrix LTM=Cholesky(S);
//   cout << "After LTM call in genmatrix::cholesky_decomposition()" << endl;
   
   L.clear_values();
   for (unsigned int i=0; i<mdim; i++)
   {
      for (unsigned int j=0; j<=i; j++)
      {
         L.put(i,j,LTM(i+1,j+1));
      } // loop over index j labeling columns
   } // loop over index i labeling rows

//   cout << "S = " << *this << endl;
//   cout << "L = " << L << endl;
//   cout << "L*Ltrans = " << L*L.transpose() << endl;
   return true;
}

// ---------------------------------------------------------------------
// Member function sym_eigen_decomposition uses the NEWMAT11 library
// to decompose a symmetric matrix into the product S = U * D * Utrans
// of its eigenvalues (contained within diagonal matrix D) and its
// eigenvectors (within the columns of orthogonal matrix U).  Recall
// matrix U may correspond to an improper rotation (i.e. det U = -1).

bool genmatrix::sym_eigen_decomposition(genmatrix& D,genmatrix& U) const
{
   if (mdim != ndim)
   {
      cout << "Error in genmatrix::sym_eigen_decomposition()" << endl;
      cout << "mdim = " << mdim << " does not equal ndim = " << ndim
           << endl;
      return false;
   }

   SymmetricMatrix S(mdim);
   for (unsigned int i=0; i<mdim; i++)
   {
      for (unsigned int j=0; j<ndim; j++)
      {
         S(i+1,j+1)=get(i,j);
      } // loop over index j labeling columns
   } // loop over index i labeling rows
   
   Matrix Umat;
   DiagonalMatrix Dmat;
   eigenvalues(S,Dmat,Umat);

   D.clear_values();
   U.clear_values();
   
   for (unsigned int i=0; i<mdim; i++)
   {
      D.put(i,i,Dmat(i+1));
   }
   
   for (unsigned int i=0; i<mdim; i++)
   {
      for (unsigned int j=0; j<ndim; j++)
      {
         U.put(i,j,Umat(i+1,j+1));
      } // loop over index j labeling columns
   } // loop over index i labeling rows

//   cout << "S = " << *this << endl;
//   cout << "U = " << U << endl;
//   cout << "U*Utrans = " << U*U.transpose() << endl;
//   cout << "D = " << D << endl;
//   cout << "U*D*Utrans = " << U*D*U.transpose() << endl;
   return true;
}

// ---------------------------------------------------------------------
// Member function square_root() works with an nxn matrix which is
// assumed to be symmetric.  If the eigenvalues of the input matrix
// are all non-negative, this boolean method computes the square-root
// of the input matrix and returns true.

bool genmatrix::square_root(genmatrix& Sqrt)
{
//   cout << "inside genmatrix::square_root()" << endl;
   if (mdim != ndim) return false;

   genmatrix D(mdim,mdim),U(mdim,mdim),Dsqrt(mdim,mdim);
   sym_eigen_decomposition(D,U);
//   cout << "D = " << D << " U = " << U << endl;

   Dsqrt.clear_values();
   for (unsigned int d=0; d<mdim; d++)
   {
      double curr_diag_value=D.get(d,d);
      if (curr_diag_value < 0) return false;
      Dsqrt.put(d,d,sqrt(curr_diag_value));
   }
//   cout << "Dsqrt = " << Dsqrt << endl;
   Sqrt=U*Dsqrt*U.transpose();
   return true;
}

// ---------------------------------------------------------------------
// Member function decompose_sym works with the current genmatrix
// object which is assumed to be a symmetric 3x3 matrix.  It returns 6
// continuous plus 1 discrete variables from which the symmetric
// matrix can be reconstructed.

// This method first decomposes S = U D Utrans.  It places the
// diagonal eigenvalues into output threevector lambda.  It further
// decomposes orthogonal matrix U into its rotation axis n_hat, total
// rotation angle chi and determinant (recall det U may equal -1).

bool genmatrix::decompose_sym(
   threevector& lambda,double& theta,double& phi,double& chi,double& detU) 
   const
{
   if (mdim != 3 || ndim != 3)
   {
      cout << "Error in genmatrix::decompose_sym()" << endl;
      cout << "mdim = " << mdim << " does not equal 3 or ndim = " << ndim
           << " does not equal 3" << endl;
      return false;
   }

   genmatrix D(3,3),U(3,3);
   if (!sym_eigen_decomposition(D,U)) return false;
   
   if (!U.determinant(detU)) return false;
//   cout << "D = " << D << endl;
//   cout << "U = " << U << endl;
//   cout << "U*Utrans = " << U*U.transpose() << endl;
//   cout << "detU = " << detU << endl;
   lambda.put(0,D.get(0,0));
   lambda.put(1,D.get(1,1));
   lambda.put(2,D.get(2,2));

   mathfunc::decompose_orthogonal_matrix(U,theta,phi,chi);
//   cout << "chi = " << chi*180/PI << endl;
//   cout << "theta = " << theta*180/PI << " phi = " << phi*180/PI << endl;

   genmatrix U_reconstructed(3,3);
   mathfunc::construct_orthogonal_matrix(detU,theta,phi,chi,U_reconstructed);
//   cout << "U_reconstructed = " << U_reconstructed << endl;
   return true;
}

// ---------------------------------------------------------------------
// Member function reconstruct_sym takes in three eigenvalues within
// threevector lambda, rotation axis n_hat via angles (theta,phi),
// total rotation angle chi and rotation determinant within detU.
// From these 6 continuous plus 1 discrete parameters, it reconstructs
// a unique 3x3 symmetric matrix.

void genmatrix::reconstruct_sym(
   const threevector& lambda,double theta,double phi,double chi,double detU) 
{
   genmatrix D(3,3),U(3,3);

   D.put(0,0,lambda.get(0));
   D.put(1,1,lambda.get(1));
   D.put(2,2,lambda.get(2));
   mathfunc::construct_orthogonal_matrix(detU,theta,phi,chi,U);
   *this=U*D*U.transpose();
//   cout << "sym = " << *this << endl;
}

void genmatrix::reconstruct_inverse_sym(
   const threevector& lambda,double theta,double phi,double chi,double detU) 
{
   genmatrix D(3,3),U(3,3);

   D.put(0,0,1.0/lambda.get(0));
   D.put(1,1,1.0/lambda.get(1));
   D.put(2,2,1.0/lambda.get(2));
   mathfunc::construct_orthogonal_matrix(detU,theta,phi,chi,U);
   *this=U*D*U.transpose();
//   cout << "inverse sym = " << *this << endl;
}

// ---------------------------------------------------------------------
// Member function project_rotation performs a Singular Value
// Decomposition on 3x3 matrix *this = U D Vtrans.  Recall U and V are
// orthogonal matrices, while D is a nonnegative diagonal matrix.  In
// order for *this to represent a true rotation, D must in fact equal
// the identity.  So we return R = U * Vtrans as the best fit rotation
// projection from *this.

rotation genmatrix::projection_rotation()
{
//   cout << "inside genmatrix::projection_rotation()" << endl;
   
   genmatrix U(3,3),W(3,3),V(3,3);
   rotation R;
   sorted_singular_value_decomposition(U,W,V);
//      cout << "U = " << U << " U*Utrans = " << U*U.transpose() << endl;
//      cout << "V = " << V << " V*Vtrans = " << V*V.transpose() << endl;
//      cout << "W = " << W << endl;
//      cout << "U.det = " << U.determinant() 
//           << " V.det = " << V.determinant() 
//           << " W.det = " << W.determinant() << endl;
   R=U*V.transpose();
//      cout << "R = " << R << endl;
//      cout << "R*Rtranspose = " << R*R.transpose() << endl;
//      cout << "R.det = " << R.determinant() << endl;
   return R;
}

// ==========================================================================
// Triangular decomposition member functions
// ==========================================================================

// Member function flipud() intentionally mimics matlab's "flip
// upside-down" function.  It swaps the current matrix's top row with
// its bottom, its next-to-top row with its next-to-bottom, etc.

void genmatrix::flipud(genmatrix& A_flipud) const
{
   for (unsigned int r=0; r<mdim; r++)
   {
      for (unsigned int c=0; c<ndim; c++)
      {
         A_flipud.put(mdim-1-r,c,get(r,c));
      }
   }
}

// ---------------------------------------------------------------------
// Member function reverselr() swaps the current matrix's first column
// with its last, its second column with its next-to-last, etc.

void genmatrix::reverselr(genmatrix& A_reverselr) const
{
   for (unsigned int r=0; r<mdim; r++)
   {
      for (unsigned int c=0; c<ndim; c++)
      {
         A_reverselr.put(r,ndim-1-c,get(r,c));
      }
   }
}

// ---------------------------------------------------------------------
// Member function QR_decomposition() calls the Householder method in
// the Eigen library in order to decompose the current genmatrix into
// a product Q*R where Q is an orthogonal matrix and R is an upper
// triangular matrix.

void genmatrix::QR_decomposition(genmatrix& Q,genmatrix& R) const
{
   Eigen::MatrixXd AE(mdim,ndim),QE(mdim,ndim);

   for (unsigned int r=0; r<get_mdim(); r++)
   {
      for (unsigned int c=0; c<get_ndim(); c++)
      {
         AE(r,c)=get(r,c);
      }
   }
   Eigen::HouseholderQR<Eigen::MatrixXd> qr(AE);
   QE=qr.householderQ();

   for (unsigned int r=0; r<mdim; r++)
   {
      for (unsigned int c=0; c<ndim; c++)
      {
         Q.put(r,c,QE(r,c));
      }
   }

   R=Q.transpose() * (*this);

//   cout << "Q = " << Q << endl;   
//   cout << "R = " << R << endl;
//   cout << "Q*Qtrans = " << Q * Q.transpose() << endl;
//   cout << "*this - Q R = " << *this - Q*R << endl;
}

// ---------------------------------------------------------------------
// Member function RQ_decomposition() performs a series of row &
// column upside-down flips, left-right reversals and transpositions
// following the approach reported in
// http://leohart.wordpress.com/2010/07/23/rq-decomposition-from-qr-decomposition/
// .  It yields a decomposition of *this into the product R*Q where R
// is an upper triangular matrix while Q is an orthogonal matrix.

void genmatrix::RQ_decomposition(genmatrix& R,genmatrix& Q) const
{
//   cout << "inside genmatrix::RQ_decomposition()" << endl;
   
   genmatrix A_flipud(mdim,ndim);
   flipud(A_flipud);
//   cout << "A_flipud = " << A_flipud << endl;
   
   genmatrix Atransflip(ndim,mdim);
   Atransflip=A_flipud.transpose();
//   cout << "Atransflip = " << Atransflip << endl;

   genmatrix Qtransflip(ndim,mdim),Rtransflip(ndim,mdim);
   Atransflip.QR_decomposition(Qtransflip,Rtransflip);

//   cout << "Qtransflip = " << Qtransflip << endl;
//   cout << "Qtransflip * Qtransflip_trans = " 
//        << Qtransflip*Qtransflip.transpose() << endl;
//   cout << "Rtransflip = " << Rtransflip << endl;
//   cout << "Qtransflip * Rtransflip = " 
//        << Qtransflip * Rtransflip << endl;

   genmatrix Rflipped(mdim,ndim),Qflipped(mdim,ndim);
   Rflipped=Rtransflip.transpose();
//   cout << "Rflipped = " << Rflipped << endl;

   genmatrix Rreversed(mdim,ndim);
   Rflipped.flipud(Rreversed);
   Rreversed.reverselr(R);
//   cout << "R = " << R << endl;

   Qflipped=Qtransflip.transpose();
//   cout << "Qflipped = " << Qflipped << endl;
   Qflipped.flipud(Q);
//   cout << "Q = " << Q << endl;   

//   cout << "R*Q = " << R*Q << endl;
}

// ==========================================================================
// Linear system solution member functions
// ==========================================================================

// Member function homogeneous_soln performs a singular value
// decomposition U * W * Vtrans on the current genmatrix object M.  If
// there genuinely exists a vector X satisfying M X = 0, matrix W
// should have at least one zero-valued diagonal entry.  So this
// method returns the column in V corresponding to the minimal
// singular value in W.

bool genmatrix::homogeneous_soln(genvector& X) const
{
//   cout << "inside genmatrix::homogenous_soln()" << endl;
   genmatrix U(mdim,ndim),W(ndim,ndim),V(ndim,ndim);
   if (!sorted_singular_value_decomposition(U,W,V)) return false;

//   cout << "U = " << U << endl;
//   cout << "W = " << W << endl;
//   cout << "V = " << V << endl;

   V.get_column(ndim-1,X);
//   cout << "X = " << X << endl;

// Check whether X equals the zero vector.  If so, return false
// boolean flag.  In the future, we could try searching for the first
// nontrivial homogenous solution...

   bool nontrivial_soln_found=false;
   for (unsigned int i=0; i<X.get_mdim(); i++)
   {
      if (fabs(X.get(i) > 1E-8)) nontrivial_soln_found=true;
   }

   return nontrivial_soln_found;
}

// ---------------------------------------------------------------------
// Member function inhomogeneous_soln solves the system M.X=B where
// genmatrix M=*this, genvector B is passed as input and genvector X
// is returned.  It performs a singular value decomposition U * W *
// Vtrans on M.  Provided W contains no zero singular values, X = V *
// W^-1 * Utrans * B.

bool genmatrix::inhomogeneous_soln(const genvector& B,genvector& X) const
{
   bool soln_found=true;
   genmatrix U(mdim,ndim),W(ndim,ndim),V(ndim,ndim);
   
   if (!sorted_singular_value_decomposition(U,W,V)) return false;

   genmatrix Winv(ndim,ndim);
   for (unsigned int n=0; n<ndim; n++)
   {
      const double TINY=1E-10;
      if (fabs(W.get(n,n)) > TINY)
      {
         Winv.put(n,n,1.0/W.get(n,n));
      }
      else
      {
         soln_found=false;
      }
   } // loop over index n labeling entries in diagonal W matrix
   if (soln_found)
   {
      X=V*Winv*U.transpose()*B;
   }
   return soln_found;
}

// ---------------------------------------------------------------------
// Member function solve_linear_system solves the system A.X=B where
// genmatrix A=*this, while genvector B is passed as input.  Genvector
// X is returned by this method.  Use this method to solve for X
// rather than inverting A in order to speed up processing time!

bool genmatrix::solve_linear_system(const genvector& B,genvector& X) const
{
   if (mdim==ndim)
   {

// As described in section 1.2 entitled "Some C conventions for
// scientific computing", all numerical recipes in the C book have
// vectors and matrix index ranges which run from 1 to N rather than
// from 0 to N-1.  Numerical recipe matrices are also arrays of
// pointers-to-pointers.  In order to use the Numerical Recipes
// routines which contain vectors, we need to first declare vectors as
// double *vec and then call the Numerical Recipes function
// vec=vector(1,mdim).  Similarly, matrices are initially declared as
// double **mat and then converted to Numerical Recipes routine form
// via the call mat=matrix(1,mdim,1,mdim).  
    
      int *indx=numrec::ivector(1,mdim);
      double *b=numrec::vector(1,mdim);
      double **A1=numrec::matrix(1,mdim,1,mdim);

// First copy contents of current matrix object into array A1[][]: 

      for (unsigned int i=0; i<mdim; i++)
      {
         for (unsigned int j=0; j<mdim; j++)
         {
	    A1[i+1][j+1]=get(i,j);
         }
         b[i+1]=B.get(i);
      }

      double d;
      bool ludcmp_successfully_calculated=numrec::ludcmp(A1,mdim,indx,&d);
      if (ludcmp_successfully_calculated)
      {
         numrec::lubksb(A1,mdim,indx,b);

//  Load the values of b into output genvector X:

         for (unsigned int i=0; i<mdim; i++)
         {
            X.put(i,b[i+1]);
         }
      }
      numrec::free_ivector(indx,1,mdim);
      numrec::free_vector(b,1,mdim);
      numrec::free_matrix(A1,1,mdim,1,mdim);
      return ludcmp_successfully_calculated;
   }
   else
   {
      cout << "Error in genmatrix::solve_linear_system()!" << endl;
      cout << "mdim = " << mdim << " ndim = " << ndim << endl;
      cout << "Matrix inverse is ill-defined when mdim != ndim" << endl;
      return false;
   }
}

// ==========================================================================
// Rank deficient matrix approximation member functions
// ==========================================================================

// Member function rank returns the number of nonzero singular values
// of the current genmatrix object:

unsigned int genmatrix::rank() const
{
//   cout << "inside genmatrix::rank()" << endl;
//   cout << "*this = " << *this << endl;
   
   genmatrix U(mdim,ndim),W(ndim,ndim),V(ndim,ndim);
   
   sorted_singular_value_decomposition(U,W,V);
//      cout << "W = " << W << endl;

   unsigned int rank=0;
   const double TINY=1E-8;
   for (unsigned int n=0; n<ndim; n++)
   {
      if (fabs(W.get(n,n)) > TINY) rank++;
   } // loop over index n

   return rank;
}

// ---------------------------------------------------------------------
// Member function rank_approximation takes in some rank r which
// should be less than or equal to the true rank of the current
// genmatrix object.  It performs a singular value decomposition and
// nulls r+1, r+2, ..., ndim singular values in matrix W.  This method
// returns the product U W_reduced Vtrans which represents the best
// rank r approximation to the current genmatrix object.

bool genmatrix::rank_approximation(
   unsigned int r,genmatrix& Ur,genmatrix& Wr,genmatrix& Vr)
{
//   cout << "inside genmatrix::rank_approximation()" << endl;
   
   if (r > rank()) r=rank();
   
   genmatrix U(mdim,ndim),W(ndim,ndim),V(ndim,ndim);
   if (!sorted_singular_value_decomposition(U,W,V)) return false;

   Ur.clear_values();
   for (unsigned int m=0; m<mdim; m++)
   {
      for (unsigned int n=0; n<r; n++)
      {
         Ur.put(m,n,U.get(m,n));
      }
   }
      
   for (unsigned int n=0; n<r; n++)
   {
      Wr.put(n,n,W.get(n,n));
   }
      
   for (unsigned int m=0; m<r; m++)
   {
      for (unsigned int n=0; n<ndim; n++)
      {
         Vr.put(m,n,V.get(m,n));
      }
   }
   return true;
}

bool genmatrix::rank_approximation(
   unsigned int r,genmatrix& reduced_rank_matrix)
{
//   cout << "inside genmatrix::rank_approximation()" << endl;
   if (r > rank()) r=rank();
   
   genmatrix U(mdim,ndim),W(ndim,ndim),V(ndim,ndim);
   if (!sorted_singular_value_decomposition(U,W,V)) return false;

   for (unsigned int n=r; n<ndim; n++)
   {
      W.put(n,n,0);
   }
   reduced_rank_matrix=U*W*V.transpose();
   return true;
}

// ---------------------------------------------------------------------
// Member function generate_mask_matrix loops over all entries within
// the current genmatrix object and searches for values equal to the
// input null_value.  It sets the corresponding location within the
// output binary matrix M equal to 0.  Otherwise, the output entries
// in M equal 1.  The binary matrix is used in missing data recovery
// methods below.

void genmatrix::generate_mask_matrix(double null_value,genmatrix& M)
{
   for (unsigned int m=0; m<mdim; m++)
   {
      for (unsigned int n=0; n<ndim; n++)
      {
         M.put(m,n,1);
         if (::nearly_equal(get(m,n),null_value)) M.put(m,n,0);
      }
   }
   cout << "Mask matrix M = " << M << endl;
}

// ==========================================================================
// Matrix import/export member functions
// ==========================================================================

int genmatrix::number_nonzero_entries(double TINY)
{
   int n_nonzero_values=0;
   for (unsigned int m=0; m<mdim; m++)
   {
      for (unsigned int n=0; n<ndim; n++)
      {
         if (fabs(get(m,n)) > TINY) n_nonzero_values++;
      }
   }
   return n_nonzero_values;
}

// ---------------------------------------------------------------------
// Member function export_to_dense[sparse]_text_format() writes
// the current genmatrix to an output file following the formats used
// by the SVDLIBC library.  See http://tedlab.mit.edu/~dr/SVDLIBC/

void genmatrix::export_to_dense_text_format(string output_filename)
{
   string banner="Exporting genmatrix to dense text format";
   outputfunc::write_banner(banner);
   
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);
   outstream << mdim << " " << ndim << endl << endl;
   for (unsigned int m=0; m<mdim; m++)
   {
      for (unsigned int n=0; n<ndim; n++)
      {
         outstream << get(m,n) << " ";
      }
      outstream << endl;
   }
   filefunc::closefile(output_filename,outstream);
}

// ---------------------------------------------------------------------
// Member function export_to_dense[sparse]_binary_format() executes
// command-line SVDLIBC calls in order to save the current genmatrix
// contents into output binary files.

void genmatrix::export_to_dense_binary_format(string output_filename)
{
   string banner="Exporting genmatrix to dense binary format";
   outputfunc::write_banner(banner);

   string dense_text_filename="./dense_text.matrix";
   export_to_dense_text_format(dense_text_filename);
   string unix_cmd="svd -r dt -w db -c "+dense_text_filename+" "
      +output_filename;
   sysfunc::unix_command(unix_cmd);
   filefunc::deletefile(dense_text_filename);
}


// ---------------------------------------------------------------------
void genmatrix::export_to_sparse_text_format(string output_filename)
{
   string banner="Exporting genmatrix to sparse text format";
   outputfunc::write_banner(banner);

   double TINY=1E-9;
   int n_nonzero_values=number_nonzero_entries(TINY);
   
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);
   outstream << mdim << " " << ndim << " " << n_nonzero_values << endl << endl;
   for (unsigned int n=0; n<ndim; n++)
   {
      int n_nonzero_entries_in_curr_column=0;
      for (unsigned int m=0; m<mdim; m++)
      {
         if (fabs(get(m,n)) > TINY) n_nonzero_entries_in_curr_column++;
      }
      outstream << n_nonzero_entries_in_curr_column << endl;
      for (unsigned int m=0; m<mdim; m++)
      {
         if (fabs(get(m,n)) <= TINY) continue;
         outstream << m << " " << get(m,n) << endl;
      }
   } // loop over index n labeling genmatrix columns

   filefunc::closefile(output_filename,outstream);
}

// ---------------------------------------------------------------------
void genmatrix::export_to_sparse_binary_format(string output_filename)
{
   string banner="Exporting genmatrix to sparse binary format";
   outputfunc::write_banner(banner);

   string sparse_text_filename="./sparse_text.matrix";
   export_to_sparse_text_format(sparse_text_filename);
   string unix_cmd="svd -r st -w sb -c "+sparse_text_filename+" "
      +output_filename;
   sysfunc::unix_command(unix_cmd);
   filefunc::deletefile(sparse_text_filename);
}

// ---------------------------------------------------------------------
void genmatrix::sparse_SVD_approximation(int k_dims)
{
   string sparse_binary_filename="./sb.matrix";
   export_to_sparse_binary_format(sparse_binary_filename);

   string unix_cmd="svd -r sb -o svd -d "+stringfunc::number_to_string(k_dims)
      +" sb.matrix";
//   string unix_cmd="svd -r sb -o svd sb.matrix";
   cout << "unix_cmd = " << unix_cmd << endl;
   sysfunc::unix_command(unix_cmd);
   
   filefunc::deletefile(sparse_binary_filename);
}




// ==========================================================================

// Overload *= and /= operators:

void genmatrix::operator*= (double a)
{
  for (unsigned int i=0; i<mdim; i++)
  {
     for (unsigned int j=0; j<ndim; j++)
     {
        put(i,j,a*get(i,j));
     }
  }
}

void genmatrix::operator/= (double a)
{
   if (a != 0)
   {
      operator*= (1/a);
   }
   else
   {
      cout << "Division by zero error inside genmatrix::operator/= !"
           << endl;
   }
}

// ==========================================================================
// Note: Keyword friend should appear in class declaration file and not 
// within class member function definition file.  Friendly functions should
// not be declared as member functions of a class.  So their definition syntax
// is not 

// returntype classname::memberfunctioname(argument list)

// but rather

// returntype friendlyfunctionname(argument list)
// ==========================================================================

// ==========================================================================
// Friend methods
// ==========================================================================

// Overload + operator:

genmatrix operator+ (const genmatrix& A,const genmatrix& B)
{
   if (A.mdim==B.mdim && A.ndim==B.ndim)
   {
      genmatrix C(A.mdim,A.ndim);

      for (unsigned int i=0; i<A.mdim; i++)
      {
         for (unsigned int j=0; j<A.ndim; j++)
         {
            C.put(i,j,A.get(i,j)+B.get(i,j));
         }
      }
      return C;
   }
   else
   {
      cout << "Error inside operator+ in genmatrix.cc class!" << endl;
      cout << "A.mdim = " << A.mdim << " A.ndim = " << A.ndim << endl;
      cout << "B.mdim = " << B.mdim << " B.ndim = " << B.ndim << endl;
      cout << "Cannot add together two matrices of different dimensions!"
           << endl;
      exit(-1);
   }
}

// Overload - operator:

genmatrix operator- (const genmatrix& A,const genmatrix& B)
{
   if (A.mdim==B.mdim && A.ndim==B.ndim)
   {
      genmatrix C(A.mdim,A.ndim);

      for (unsigned int i=0; i<A.mdim; i++)
      {
         for (unsigned int j=0; j<A.ndim; j++)
         {
            C.put(i,j,A.get(i,j)-B.get(i,j));
         }
      }
      return C;
   }
   else
   {
      cout << "Error inside operator- in genmatrix.cc class!" << endl;
      cout << "A.mdim = " << A.mdim << " A.ndim = " << A.ndim << endl;
      cout << "B.mdim = " << B.mdim << " B.ndim = " << B.ndim << endl;
      cout << "Cannot subtract two matrices of different dimensions!"
           << endl;
      exit(-1);
   }
}

// Overload - operator:

genmatrix operator- (const genmatrix& A)
{
   genmatrix B(A.mdim,A.ndim);
	 
   for (unsigned int i=0; i<A.mdim; i++)
   {
      for (unsigned int j=0; j<A.ndim; j++)
      {
         B.put(i,j,-A.get(i,j));
      }
   }
   return B;
}

// ==========================================================================
// Overload * operator for multiplying a genmatrix by a scalar
// ==========================================================================

genmatrix operator* (double a,const genmatrix& A)
{
   genmatrix B(A.mdim,A.ndim);

   for (unsigned int i=0; i<A.mdim; i++)
   {
      for (unsigned int j=0; j<A.ndim; j++)
      {
         B.put(i,j,a*A.get(i,j));
      }
   }
   return B;
}

genmatrix operator* (const genmatrix& A,double a)
{
   return a*A;
}

genmatrix operator/ (const genmatrix& A,double a)
{
   if (a != 0)
   {
      return (1/a)*A;
   }
   else
   {
      cout << "Division by zero error inside operator/ friend function of genmatrix class!"
           << endl;
      exit(-1);
   }
}

/*

// Brute force & very inefficient approach to matrix multiplication:

genmatrix operator* (const genmatrix& A,const genmatrix& B)
{
   if (A.ndim==B.mdim)
   {
      genmatrix C(A.mdim,B.ndim);
	 
      for (unsigned int i=0; i<A.mdim; i++)
      {
         for (unsigned int j=0; j<B.ndim; j++)
         {
            C.put(i,j,0);
            for (unsigned int k=0; k<A.ndim; k++)
            {
               C.put(i,j,C.get(i,j)+A.get(i,k)*B.get(k,j));
            }
         }
      }
      return C;
   }
   else
   {
      cout << "Error in operator* friend function of genmatrix class!"
           << endl;
      cout << "A.ndim = " << A.ndim << " B.mdim = " << B.mdim << endl;
      cout << "Cannot multiply matrices A and B!" << endl;
      exit(-1);
   }
}
*/

// Efficient approach to matrix multiplication described by "Rusty
// Horse" on July 18, 2014 at end of
// http://stackoverflow.com/questions/4300663/speed-up-matrix-multiplication

genmatrix operator* (const genmatrix& A, const genmatrix& B)
{
   unsigned int A_mdim = A.mdim;
   unsigned int A_ndim = A.ndim;
   unsigned int B_mdim = B.mdim;
   unsigned int B_ndim = B.ndim;

   if (A_ndim != B_mdim)
   {
      cout << "Error in operator* friend function of genmatrix class!"
           << endl;
      cout << "A_ndim = " << A_ndim << " B_mdim = " << B_mdim << endl;
      cout << "Cannot multiply matrices A and B!" << endl;
      exit(-1);
   }

   genmatrix C(A_mdim,B_ndim);
   C.clear_values();

   for (unsigned int i = 0; i < A_mdim; i++)
   {
      for (unsigned int k = 0; k < A_ndim; k++)
      {
         const double* a = A.get_e_ptr() + i * A_ndim + k;
         const double* b = B.get_e_ptr() + k * B_ndim;
         double *c = C.get_e_ptr() + i * B_ndim;
         double *cMax = c + B_ndim;
         
         while(c < cMax)
         {
            *c++ += (*a) * (*b++);
         }
         
      } // loop over index k
   } // loop over index i 

   return C;
}

// ---------------------------------------------------------------------
// These next two implementation of matrix addition and multiplication
// are intentionally stripped down to run as fast as possible.  They
// assumes that input matrices A and B have correct dimensions to be
// added/multiplied together and put into *this.

void genmatrix::matrix_sum(const genmatrix& A,const genmatrix& B)
{
   for (unsigned int i = 0; i < A.mdim; i++)
   {
      for (unsigned int j = 0; j < A.ndim; j++)
      {
         put(i, j, A.get(i,j) + B.get(i,j));
      }
   }
}

void genmatrix::matrix_mult(const genmatrix& A, const genmatrix& B)
{
   clear_values();

   for (unsigned int i = 0; i < A.mdim; i++)
   {
      for (unsigned int k = 0; k < A.ndim; k++)
      {
         const double* a = A.get_e_ptr() + i * A.ndim + k;
         const double* b = B.get_e_ptr() + k * B.ndim;
         double *c = get_e_ptr() + i * B.ndim;
         double *cMax = c + B.ndim;
         
         while(c < cMax)
         {
            *c++ += (*a) * (*b++);
         }
         
      } // loop over index k
   } // loop over index i 
}

