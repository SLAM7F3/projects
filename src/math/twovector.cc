// ==========================================================================
// Twovector class member function definitions
// ==========================================================================
// Last modified on 4/13/06; 7/6/06; 6/29/07; 5/25/10
// ==========================================================================

#include "math/threevector.h"
#include "math/twovector.h"

using std::ostream;
using std::cout;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

twovector::twovector():
   genvector(2)
{
}

twovector::twovector(double x,double y):
   genvector(2)
{
   put(0,x);
   put(1,y);
}

twovector::twovector(const threevector& v):
   genvector(2)
{
   put(0,v.get(0));
   put(1,v.get(1));
}

twovector::twovector(const genvector& v):
   genvector(2)
{
   put(0,v.get(0));
   put(1,v.get(1));
}

twovector::twovector(const tensor& T):
   genvector(2)
{
   put(0,T.get(0));
   put(1,T.get(1));
}

// Copy constructor:

twovector::twovector(const twovector& v):
   genvector(2)
{
   put(0,v.get(0));
   put(1,v.get(1));
}

twovector::~twovector()
{
}

// ---------------------------------------------------------------------
void twovector::docopy(const twovector & v)
{
}

// Overload = operator:

twovector& twovector::operator= (const twovector& v)
{
   if (this==&v) return *this;
   genvector::operator=(v);
   docopy(v);
   return *this;
}

// Overload << operator

ostream& operator<< (ostream& outstream,const twovector& X)
{
   outstream << (genvector&)X << endl;
   return(outstream);
}

twovector twovector::unitvector() const
{
   genvector gen_unitvector=genvector::unitvector();
   return twovector(gen_unitvector.get(0),gen_unitvector.get(1));
}

// ==========================================================================
// Note: Keyword friend should appear in class declaration file and
// not within class member function definition file.  Friendly
// functions should not be declared as member functions of a class.
// So their definition syntax is not

// returntype classname::memberfunctioname(argument list)

// but rather

// returntype friendlyfunctionname(argument list) 

// We learned from Vadim on 5/28/02 that there is no inheritance for
// friend functions.  So the following section of code is lifted
// almost verbatim from the genvector class.
// ==========================================================================

// Overload + operator:

twovector operator+ (const twovector& X,const twovector& Y)
{
   return twovector(X.get(0)+Y.get(0),X.get(1)+Y.get(1));
}

// Overload - operator:

twovector operator- (const twovector& X,const twovector& Y)
{
   return twovector(X.get(0)-Y.get(0),X.get(1)-Y.get(1));
}

// Overload - operator:

twovector operator- (const twovector& X)
{
   return twovector(-X.get(0),-X.get(1));
}

// Overload * operator:

twovector operator* (double a,const twovector& X)
{
   return twovector(a*X.get(0),a*X.get(1));
}

// Overload * operator:

twovector operator* (const twovector& X,double a)
{
   return a*X;
}

// Overload / operator:

twovector operator/ (const twovector& X,double a)
{
   return twovector(X.get(0)/a,X.get(1)/a);
}

// This overloaded version of friend function operator* takes in a
// column twovector X, multiplies it by 2x2 matrix A and returns a
// column twovector:

twovector operator* (const genmatrix& A,const twovector& X)
{
   if (A.get_mdim()==2 && A.get_ndim()==2)
   {
      twovector Y;
      for (unsigned int i=0; i<A.get_mdim(); i++)
      {
         for (unsigned int j=0; j<A.get_ndim(); j++)
         {
            Y.put(i,Y.get(i)+A.get(i,j)*X.get(j));
         }
      }
      return Y;
   }
   else
   {
      cout << "Error in operator* friend function of twovector class!"
           << endl;
      cout << "A.mdim = " << A.get_mdim() << " A.ndim = " << A.get_ndim() 
           << endl;
      cout << "Cannot multiply matrix A by column twovector X!" << endl;
      exit(-1);
   }
}

// This overloaded version of operator* takes in a row twovector X,
// multiplies it by 2x2 matrix A, and returns a row twovector:

twovector operator* (const twovector& X,const genmatrix& A)
{
   if (A.get_mdim()==2 && A.get_ndim()==2)
   {
      twovector Ytrans;
      for (unsigned int j=0; j<A.get_ndim(); j++)
      {
         for (unsigned int i=0; i<A.get_mdim(); i++)
         {
            Ytrans.put(j,Ytrans.get(j)+X.get(i)*A.get(i,j));
         }
      }
      return Ytrans;
   }
   else
   {
      cout << "Error in operator* friend function of twovector class!"
           << endl;
      cout << "A.mdim = " << A.get_mdim() 
           << " A.ndim = " << A.get_ndim() << endl;
      cout << "Cannot multiply matrix A by row twovector X!" << endl;
      exit(-1);
   }
}





