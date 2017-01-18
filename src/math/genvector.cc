// ==========================================================================
// Genvector class member function definitions
// ==========================================================================
// Last modified on 10/19/16; 11/29/16; 12/13/16; 1/18/17
// ==========================================================================

#include <math.h>
#include "math/basic_math.h"
#include "datastructures/descriptor.h"
#include "math/genvector.h"
#include "general/outputfuncs.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

genvector::genvector(int m):
   genmatrix(m)
{
   if (m <= 0)
   {
      cout << "Error in genvector constructor #1: m = " << m << endl;
      cout << "this = " << this << endl;
   }
   if (get_ndim() != 1)
   {
      cout << "Error in genvector constructor #1" << endl;
      cout << "ndim = " << get_ndim() << endl;
      exit(-1);
   }
}

genvector::genvector(const descriptor& descript):
   genmatrix(descript.get_mdim())
{
   for (unsigned int m=0; m<mdim; m++)
   {
      put(m,descript.get(m));
   }
}

genvector::genvector(const genmatrix& M):
   genmatrix(M.get_mdim())
{
   if (M.get_mdim() != mdim || M.get_ndim() != 1)
   {
      cout << "Error in genvector constructor #2" << endl;
      cout << "M.mdim = " << M.get_mdim() 
           << " M.ndim = " << M.get_ndim() << endl;
      exit(-1);
   }
   else
   {
      for (unsigned int m=0; m<mdim; m++)
      {
         put(m,M.get(m,0));
      }
   }
}

genvector::genvector(const tensor& T):
   genmatrix(T)
{
}

// Copy constructor:

genvector::genvector(const genvector& v):
   genmatrix(v.mdim)
{
   tensor::docopy(v);
   genarray::docopy(v);
   genmatrix::docopy(v);
   docopy(v);
}

genvector::~genvector()
{
}

// ---------------------------------------------------------------------
void genvector::docopy(const genvector & v)
{
}

// Overload = operator:

genvector& genvector::operator= (const genvector& v)
{
   if (this==&v) return *this;

   if (this->get_mdim() != v.get_mdim() ||
   v.get_ndim() != 1 ||this->get_ndim() != 1)
   {
      cout << "Error inside genvector::operator= !!!" << endl;
      cout << "v = " << v << endl;
      cout << "v.get_mdim() = " << v.get_mdim() << endl;
      cout << "v.get_ndim() = " << v.get_ndim() << endl;

      cout << "this = " << this << endl;
      cout << "this->get_mdim() = " << this->get_mdim() << endl;
      cout << "this->get_ndim() = " << this->get_ndim() << endl;
      cout << "*this = " << *this << endl;
   }

   genmatrix::operator=(v);
   docopy(v);
   return *this;
}

genvector& genvector::operator= (const genmatrix& M)
{
   if (M.get_mdim() != mdim || M.get_ndim() > 1)
   {
      cout << "Error in genvector operator=" << endl;
      cout << "M.mdim = " << M.get_mdim() 
           << " M.ndim = " << M.get_ndim() << endl;
      exit(-1);
   }
   genmatrix::operator=(M);
   return *this;
}

// Overload << operator

// James Wanken reminded us on 2/16/01 that it's not really a good
// idea to try to incorporate the "<<" operator as a member function
// of any class.  If we were to do so, then it would have to be called
// via syntax such as "V.<<" where V is a vector object.  It is
// instead much better to leave "<<" as a friend of a class and to
// overload it as Ed Broach taught us below:

ostream& operator<< (ostream& outstream,const genvector& X)
{
   outstream << endl;
   for (unsigned int i=0; i<X.mdim; i++)
   {
//      if(X.mdim < 10)
      {
         outstream << X.get(i) << endl;
      }
//      else
//      {
//         outstream << i << "  " << X.get(i) << endl;
//      }
   }
   return outstream;
}

// ==========================================================================

double genvector::dot(const genvector& X) const
{
   double dotproduct=0;
   for (unsigned int i=0; i<mdim; i++)
   {
      dotproduct += X.get(i)*get(i);
   }
   return dotproduct;
}

double genvector::sqrd_magnitude() const
{
   double magsqr=0;
   for (unsigned int i=0; i<mdim; i++)
   {
      magsqr += sqr(get(i));
   }
   return magsqr;
}

genvector genvector::unitvector() const
{
   genvector unitvec(mdim);
   
   double mag=magnitude();
//   if (mag > 0 && isfinite(mag) != 0)
   if (mag > 0)
   {
      for (unsigned int i=0; i<mdim; i++)
      {
         unitvec.put(i,get(i)/mag);
      }
   }
   else
   {
      cout << "Inside genvector::unitvector():" << endl;
      cout << "Input vector has zero magnitude!" << endl;
      cout << "mag = " << mag << endl;
//      cout << "*this = " << *this << endl;
      outputfunc::newline();
//      outputfunc::enter_continue_char();

      for (unsigned int i=0; i<mdim; i++)
      {
         unitvec.put(i,0);
      }
   }
   return unitvec;
}

genmatrix genvector::outerproduct(const genvector& Y) const
{
   genmatrix B(mdim,Y.mdim);

   for (unsigned int i = 0; i < mdim; i++)
   {
      double curr_X = get(i);
      for (unsigned int j = 0; j < Y.mdim; j++)
      {
         B.put(i, j, curr_X * Y.get(j));
      }
   }

   return B;
}

void genvector::self_outerproduct(genmatrix& B) const
{
   double curr_row[mdim];
   
   for (unsigned int i = 0; i < mdim; i++)
   {
      double curr_X = get(i);
      for (unsigned int j = 0; j < mdim; j++)
      {
         curr_row[j] = curr_X * get(j);
      }
      memcpy(B.get_e_ptr() + i * mdim, curr_row, mdim * sizeof(double));
   }
}

// ---------------------------------------------------------------------
// Member function hadamard_product() returns a genvector whose
// elements are component-wise products of *this with Y:

genvector genvector::hadamard_product(const genvector& Y) const
{
   genvector B(mdim);
   for (unsigned int i=0; i<mdim; i++)
   {
      B.put(i,get(i)*Y.get(i));
   }
   return B;
}

// ---------------------------------------------------------------------
// Member function hadamard_division() returns a genvector whose
// ith element = this->i / Y[i]

genvector genvector::hadamard_division(const genvector& Y) const
{
   genvector B(mdim);
   for (unsigned int i=0; i<mdim; i++)
   {
      B.put(i,get(i) / Y.get(i));
   }
   return B;
}

// ---------------------------------------------------------------------
// Member function hadamard_power() returns a genvector whose
// elements are component-wise powers of Y:

genvector genvector::hadamard_power(const genvector& Y, double alpha) const
{
   genvector B(mdim);
   for (unsigned int i=0; i<mdim; i++)
   {
      B.put(i, pow(Y.get(i), alpha));
   }
   return B;
}

genvector genvector::hadamard_power(double alpha) const
{
   genvector B(mdim);
   for (unsigned int i=0; i<mdim; i++)
   {
      B.put(i, pow(get(i), alpha));
   }
   return B;
}

// ---------------------------------------------------------------------
void genvector::hadamard_sqrt(const genvector& Y) 
{
   for (unsigned int i=0; i<mdim; i++)
   {
      put(i, sqrt(Y.get(i)));
   }
}

// ---------------------------------------------------------------------
// Member function hadamard_sum() adds alpha to each element of *this.

void genvector::hadamard_sum(double alpha) 
{
   for (unsigned int i=0; i<mdim; i++)
   {
      put(i, get(i) + alpha);
   } // loop over index m
}

// ---------------------------------------------------------------------
// Member function hadamard_ratio() divides each element of *this
// by its corresponding element in input genvector D.  

void genvector::hadamard_ratio(const genvector& D)
{
   for (unsigned int i=0; i<mdim; i++)
   {
      put(i, get(i) / D.get(i));
   } 
}

// ==========================================================================
// Vector export member functions
// ==========================================================================

void genvector::export_to_dense_text_format(string output_filename)
{
   genmatrix M(*this);
   M.export_to_dense_text_format(output_filename);
}

void genvector::export_to_dense_binary_format(string output_filename)
{
   genmatrix M(*this);
   M.export_to_dense_binary_format(output_filename);
}

void genvector::export_to_sparse_text_format(string output_filename)
{
   genmatrix M(*this);
   M.export_to_sparse_text_format(output_filename);
}

void genvector::export_to_sparse_binary_format(string output_filename)
{
   genmatrix M(*this);
   M.export_to_sparse_binary_format(output_filename);
}

// ---------------------------------------------------------------------
// Member function scale multiplies the values of each component
// within the current genvector object by the corresponding
// component within input genvector X:

void genvector::scale(const genvector& X)
{
   for (unsigned int i=0; i<mdim; i++)
   {
      put(i,get(i)*X.get(i));
   }
}

// As of 10/19/16, we believe that tensor class +=, -=, *= and /=
// operators are more efficient than these deprecated genvector
// versions:

/*

// Overload += operator:

void genvector::operator+= (const genvector& X)
{
  for (unsigned int i=0; i<mdim; i++)
  {
     tensor::increment(i,X.get(i));
  }
}

// Overload -= operator:

void genvector::operator-= (const genvector& X)
{
  for (unsigned int i=0; i<mdim; i++)
  {
     tensor::increment(i,-X.get(i));
  }
}

// Overload *= operator:

void genvector::operator*= (double a)
{
  for (unsigned int i=0; i<mdim; i++)
  {
     put(i,a*get(i));
  }
}

// Overload /= operator:

void genvector::operator/= (double a)
{
  for (unsigned int i=0; i<mdim; i++)
  {
     put(i,get(i)/a);
  }
}
*/

// ==========================================================================
// Note: Keyword friend should appear in class declaration file and not 
// within class member function definition file.  Friendly functions should
// not be declared as member functions of a class.  So their definition syntax
// is not 

// returntype classname::memberfunctioname(argument list)

// but rather

// returntype friendlyfunctionname(argument list)
// ==========================================================================

// Overload + operator:

genvector operator+ (const genvector& X,const genvector& Y)
{
   if (X.mdim==Y.mdim)
   {
      genvector Z(X.mdim);
      for (unsigned int i=0; i<X.mdim; i++)
      {
         Z.put(i,X.get(i)+Y.get(i));
      }
      return Z;
   }
   else
   {
      cout << "Error inside operator+ friend function of genvector class!"
           << endl;
      cout << "X.mdim = " << X.mdim << " does not equal Y.mdim = " 
           << Y.mdim << endl;
      exit(-1);
   }
}

// Overload - operator:

genvector operator- (const genvector& X,const genvector& Y)
{
   if (X.mdim==Y.mdim)
   {
      genvector Z(X.mdim);
      for (unsigned int i=0; i<X.mdim; i++)
      {
         Z.put(i,X.get(i)-Y.get(i));
      }
      return Z;
   }
   else
   {
      cout << "Error inside operator- friend function of genvector class!"
           << endl;
      cout << "X.mdim = " << X.mdim << " does not equal Y.mdim = " 
           << Y.mdim << endl;
      exit(-1);
   }
}

// Overload - operator:

genvector operator- (const genvector& X)
{
   genvector Z(X.mdim);

   for (unsigned int i=0; i<X.mdim; i++)
   {
      Z.put(i,-X.get(i));
   }
   return Z;
}

// Overload * operator:

genvector operator* (double a,const genvector& X)
{
   genvector Z(X.mdim);
   
   for (unsigned int i=0; i<X.mdim; i++)
   {
      Z.put(i,a*X.get(i));
   }
   return Z;
}

// Overload * operator:

genvector operator* (const genvector& X,double a)
{
   return a*X;
}

// Overload / operator:

genvector operator/ (const genvector& X,double a)
{
   genvector Z(X.mdim);
   
   for (unsigned int i=0; i<X.mdim; i++)
   {
      Z.put(i,X.get(i)/a);
   }
   return Z;
}

// Multiply matrix A by column vector X on the right:

genvector operator* (const genmatrix& A,const genvector& X)
{
   if (A.get_ndim()==X.get_mdim())
   {
      genvector Y(A.get_mdim());
	 
      for (unsigned int i=0; i<A.get_mdim(); i++)
      {
         Y.put(i,0);
         for (unsigned int j=0; j<A.get_ndim(); j++)
         {
            Y.put(i,Y.get(i)+A.get(i,j)*X.get(j));
         }
      }
      Y.magnitude();
      return Y;
   }
   else
   {
      cout << "Error in operator* friend function of genvector class!"
           << endl;
      cout << "X.mdim = " << X.mdim << " A.get_ndim() = " << A.get_ndim() 
           << endl;
      cout << "Cannot multiply matrix A by column vector X!" << endl;
      exit(-1);
   }
}

// Multiply matrix A by row vector X on the left:

genvector operator* (const genvector& X,const genmatrix& A)
{
   if (X.mdim==A.get_mdim())
   {
      genvector Ytrans(A.get_ndim());
	 
      for (unsigned int j=0; j<A.get_ndim(); j++)
      {
         Ytrans.put(j,0);
         for (unsigned int i=0; i<A.get_mdim(); i++)
         {
            Ytrans.put(j,Ytrans.get(j)+X.get(i)*A.get(i,j));
         }
      }
      Ytrans.magnitude();
      return Ytrans;
   }
   else
   {
      cout << "Error in operator* friend function of genvector class!"
           << endl;
      cout << "X.mdim = " << X.mdim << " A.get_ndim() = " << A.get_ndim() 
           << endl;
      cout << "Cannot multiply matrix A by row vector X!" << endl;
      exit(-1);
   }
}

// ---------------------------------------------------------------------
// This next implementation of matrix-vector multiplication is
// intentionally stripped down to run as fast as possible.  It assumes
// input matrix A and vector X have correct dimensions to be
// multiplied together and put into *this.

void genvector::matrix_vector_mult(const genmatrix& A,const genvector& X)
{
   for (unsigned int i = 0; i < A.get_mdim(); i++)
   {
      double sum = 0;
      for (unsigned int j = 0; j < A.get_ndim(); j++)
      {
         sum += A.get(i,j) * X.get(j);
      }
      put(i, sum);
   }
}

void genvector::matrix_vector_mult_sum(
   const genmatrix& A,const genvector& X, const genvector& V)
{
   for (unsigned int i = 0; i < A.get_mdim(); i++)
   {
      double sum = V.get(i);
      for (unsigned int j = 0; j < A.get_ndim(); j++)
      {
         sum += A.get(i,j) * X.get(j);
      }
      put(i, sum);
   }
}

void genvector::vector_increment(double alpha, const genvector& B)
{
   for (unsigned int i = 0; i < B.mdim; i++)
   {
      put(i, get(i) + alpha * B.get(i));
   }
}

// Member function copy_matrix_column() copies the contents of column
// c from genmatrix A directly into *this.

void genvector::copy_matrix_column(const genmatrix& A, int c)
{
   for (unsigned int i = 0; i < A.get_mdim(); i++)
   {
      put(i, A.get(i,c));
   }
}
