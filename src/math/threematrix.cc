// ==========================================================================
// Threematrix class member function definitions
// ==========================================================================
// Last modified on 1/29/12; 5/8/13
// =========================================================================

#include <vector>
#include "math/constants.h"
#include "math/threematrix.h"
#include "math/rotation.h"
#include "math/threevector.h"

using std::cout;
using std::endl;
using std::ostream;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

threematrix::threematrix():
genmatrix(3,3)
{
}

// ---------------------------------------------------------------------
threematrix::threematrix(const rotation& R):
genmatrix(R)
{
   for (int i=0; i<3; i++)
   {
      for (int j=0; j<3; j++)
      {
         put(i,j,R.get(i,j));
      }
   }
}

// ---------------------------------------------------------------------
// Copy constructor

threematrix::threematrix(const threematrix& m):
genmatrix(m)
{
//   cout << "inside threematrix copy constructor" << endl;
   docopy(m);
}

threematrix::~threematrix()
{
}

// ---------------------------------------------------------------------
void threematrix::docopy(const threematrix& m)
{
//   cout << "inside threematrix::docopy()" << endl;
   genmatrix::docopy(m);
}	

// Overload = operator:

threematrix& threematrix::operator= (const threematrix& m)
{
//   cout << "inside threematrix::operator==" << endl;
   if (this==&m) return *this;
   genmatrix::operator=(m);
   docopy(m);
   return *this;
}

// Overload << operator:

ostream& operator<< (ostream& outstream,const threematrix& A)
{
   outstream << endl;
   for (int i=0; i<3; i++)
   {
      for (int j=0; j<3; j++)
      {
         outstream << A.get(i,j) << "\t";
      }
      outstream << endl;
   }
   return outstream;
}

// Overload - operator:

threematrix operator- (const threematrix& A)
{
   threematrix B;
	 
   for (int i=0; i<3; i++)
   {
      for (int j=0; j<3; j++)
      {
         B.put(i,j,-A.get(i,j));
      }
   }
   return B;
}

// Overload * operator:

threevector operator* (const threematrix& A,const threevector& X)
{
   threevector Y(0,0,0);
	 
   for (int i=0; i<3; i++)
   {
      for (int j=0; j<3; j++)
      {
         Y.put(i,Y.get(i)+A.get(i,j)*X.get(j));
      }
   }
   return Y;
}

threevector operator* (const threevector& X,const threematrix& A)
{
   threevector Y(0,0,0);
	 
   for (int i=0; i<3; i++)
   {
      for (int j=0; j<3; j++)
      {
         Y.put(i,Y.get(i)+X.get(j)*A.get(j,i));
      }
   }
   return Y;
}

threematrix operator* (const threematrix& A,const threematrix& B)
{
   threematrix C;
	 
   for (unsigned int i=0; i<3; i++)
   {
      for (unsigned int j=0; j<3; j++)
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


