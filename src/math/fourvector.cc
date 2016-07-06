// ==========================================================================
// Fourvector class member function definitions
// ==========================================================================
// Last modified on 3/1/07; 6/29/07; 12/20/11
// ==========================================================================

#include <iostream>
#include "math/fourvector.h"
#include "math/threevector.h"

using std::cout;
using std::endl;
using std::ostream;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

fourvector::fourvector():
   genvector(4)
{
}

fourvector::fourvector(double x,double y,double z):
   genvector(4)
{
   put(0,x);
   put(1,y);
   put(2,z);
   put(3,0);
}

fourvector::fourvector(double x,double y,double z,double p):
   genvector(4)
{
   put(0,x);
   put(1,y);
   put(2,z);
   put(3,p);
}

fourvector::fourvector(const threevector& r):
   genvector(4)
{
   put(0,r.get(0));
   put(1,r.get(1));
   put(2,r.get(2));
   put(3,0);
}

fourvector::fourvector(const threevector& r,double p):
   genvector(4)
{
   put(0,r.get(0));
   put(1,r.get(1));
   put(2,r.get(2));
   put(3,p);
}

fourvector::fourvector(const genvector& v):
   genvector(4)
{
//   cout << "inside fourvector constructor with genvector arg" << endl;
   put(0,v.get(0));
   put(1,v.get(1));
   put(2,v.get(2));
   put(3,v.get(3));

//   cout << "*this = " << *this << endl;
}

// Copy constructor

fourvector::fourvector(const fourvector& v):
   genvector(v)
{
   docopy(v);
}

fourvector::~fourvector()
{
}

// ---------------------------------------------------------------------
void fourvector::docopy(const fourvector& v)
{
}

// Overload = operator:

fourvector& fourvector::operator= (const fourvector& v)
{
   if (this==&v) return *this;
   genvector::operator=(v);
   docopy(v);
   return *this;
}

fourvector& fourvector::operator= (const genvector& v)
{
   fourvector V4(v);
   genvector::operator=(V4);
   docopy(V4);
   return *this;
}

// Overload << operator

ostream& operator<< (ostream& outstream,const fourvector& X)
{
   outstream << (genvector&)X << endl;
   return(outstream);
}

// ==========================================================================
fourvector fourvector::unitvector() const
{
   genvector gen_unitvector=genvector::unitvector();
   return fourvector(gen_unitvector.get(0),gen_unitvector.get(1),
                     gen_unitvector.get(2),gen_unitvector.get(3));
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

fourvector operator+ (const fourvector& X,const fourvector& Y)
{
   return fourvector(X.get(0)+Y.get(0),X.get(1)+Y.get(1),X.get(2)+Y.get(2),
                     X.get(3)+Y.get(3));
}

// Overload - operator:

fourvector operator- (const fourvector& X,const fourvector& Y)
{
   return fourvector(X.get(0)-Y.get(0),X.get(1)-Y.get(1),X.get(2)-Y.get(2),
                     X.get(3)-Y.get(3));
}

// Overload - operator:

fourvector operator- (const fourvector& X)
{
   return fourvector(-X.get(0),-X.get(1),-X.get(2),-X.get(3));
}

// Overload * operator:

fourvector operator* (double a,const fourvector& X)
{
   return fourvector(a*X.get(0),a*X.get(1),a*X.get(2),a*X.get(3));
}

// Overload * operator:

fourvector operator* (const fourvector& X,double a)
{
   return a*X;
}

// Overload / operator:

fourvector operator/ (const fourvector& X,double a)
{
   return fourvector(X.get(0)/a,X.get(1)/a,X.get(2)/a,X.get(3)/a);
}

// This overloaded version of friend function operator* takes in a
// column fourvector X, multiplies it by an mx4 matrix A and returns a
// column fourvector (whose last entries 4-m entries equal 0).

fourvector operator* (const genmatrix& A,const fourvector& X)
{
   if (A.get_ndim()==4)
//   if (A.get_mdim()==4 && A.get_ndim()==4)
   {
      fourvector Y;
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
      cout << "Error in operator* friend function of fourvector class!"
           << endl;
      cout << "A.mdim = " << A.get_mdim() 
           << " A.ndim = " << A.get_ndim() << endl;
      cout << "Cannot multiply matrix A by column fourvector X!" << endl;
      exit(-1);
   }
}

// This overloaded version of operator* takes in a row fourvector X,
// multiplies it by 4xn matrix A, and returns a row fourvector (whose
// last 4-n entries equal 0)

fourvector operator* (const fourvector& X,const genmatrix& A)
{
//   if (A.get_mdim()==4 && A.get_ndim()==4)
   if (A.get_mdim()==4)
   {
      fourvector Ytrans;
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
      cout << "Error in operator* friend function of fourvector class!"
           << endl;
      cout << "A.mdim = " << A.get_mdim() 
           << " A.ndim = " << A.get_ndim() << endl;
      cout << "Cannot multiply matrix A by row fourvector X!" << endl;
      exit(-1);
   }
}
