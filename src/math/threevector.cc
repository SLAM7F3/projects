// ==========================================================================
// Threevector class member function definitions
// ==========================================================================
// Last modified on 11/5/11; 1/12/13; 5/8/13
// ==========================================================================

#include <math.h>
#include "math/constant_vectors.h"
#include "math/fourvector.h"
#include "math/threevector.h"

using std::ostream;
using std::cout;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

threevector::threevector():
   genvector(3)
{
//   cout << "inside threevector void constructor #1" << endl;
}

threevector::threevector(double x,double y,double z):
   genvector(3)
{
//   cout << "inside threevector constructor #2" << endl;
   put(0,x);
   put(1,y);
   put(2,z);
}

threevector::threevector(const twovector& v):
   genvector(3)
{
//   cout << "inside threevector constructor #3" << endl;
   put(0,v.get(0));
   put(1,v.get(1));
   put(2,0);
}

threevector::threevector(const twovector& v,double z):
   genvector(3)
{
//   cout << "inside threevector constructor #4" << endl;
   put(0,v.get(0));
   put(1,v.get(1));
   put(2,z);
}

// Construct a threevector from a fourvector by copying its first 3
// elements:

threevector::threevector(const fourvector& f):
   genvector(3)
{
//   cout << "inside threevector constructor #5" << endl;
   put(0,f.get(0));
   put(1,f.get(1));
   put(2,f.get(2));
}

threevector::threevector(const osg::Vec3& v):
   genvector(3)
{
//   cout << "inside threevector constructor #6" << endl;
   put(0,v.x());
   put(1,v.y());
   put(2,v.z());
}

threevector::threevector(const osg::Vec4& v):
   genvector(3)
{
//   cout << "inside threevector constructor #7" << endl;
   put(0,v.x());
   put(1,v.y());
   put(2,v.z());
}

// Copy constructor

threevector::threevector(const threevector& v):
   genvector(3)
{
//   cout << "inside threevector copy constructor" << endl;
   put(0,v.get(0));
   put(1,v.get(1));
   put(2,v.get(2));
}

threevector::~threevector()
{
}

// ---------------------------------------------------------------------
void threevector::docopy(const threevector& v)
{
}

// Overload = operator:

threevector& threevector::operator= (const threevector& v)
{
   if (this==&v) return *this;

   if (this->get_mdim() <=0 || this->get_ndim() <= 0)
   {
      cout << "inside threevector::operator=" << endl;
      cout << "v = " << v << endl;
      cout << "v.get_mdim() = " << v.get_mdim() << endl;
      cout << "v.get_ndim() = " << v.get_ndim() << endl;

      cout << "this = " << this << endl;
      cout << "this->get_mdim() = " << this->get_mdim() << endl;
      cout << "this->get_ndim() = " << this->get_ndim() << endl;
      cout << "this->get_rank() = " << this->get_rank() << endl;
      cout << "this->get_dimproduct() = " << this->get_dimproduct() << endl;
      cout << "this->get_dim(0) = " << this->get_dim(0) << endl;
      cout << "this->get_Indices(0) = " << this->get_Indices(0) << endl;

      cout << "*this = " << *this << endl;
   }

   genvector::operator=(v);
   docopy(v);
   return *this;
}

// This next overloaded version of operator= was implemented so our
// templatized average() method can initialize a threevector to
// "zero":

threevector& threevector::operator= (double v)
{
   put(0,v);
   put(1,v);
   put(2,v);
   return *this;
}

// Overload << operator

ostream& operator<< (ostream& outstream,const threevector& X)
{
   outstream << endl;
   outstream << X.get(0) << " , " << X.get(1) << " , " 
             << X.get(2) << endl;
//   outstream << (genvector&)X << endl;
   return outstream;
}

// ==========================================================================
threevector threevector::unitvector() const
{
   double mag=sqrt(sqr(e[0])+sqr(e[1])+sqr(e[2]));
//   if (mag > 0 && isfinite(mag) != 0)
   if (mag > 0)
   {
      return threevector(e[0]/mag,e[1]/mag,e[2]/mag);
   }
   else
   {
      cout << "Error in threevector::unitvector()!" << endl;
      cout << "Input vector = " << *this << endl;
      return Zero_vector;
   }
}

// Member function cross should be called via a command such as
// Z.cross(X,Y) where X,Y and Z are all declared as threevectors.  

void threevector::cross(const threevector& X,const threevector& Y)
{
   put(0,X.get(1)*Y.get(2)-X.get(2)*Y.get(1));
   put(1,X.get(2)*Y.get(0)-X.get(0)*Y.get(2));
   put(2,X.get(0)*Y.get(1)-X.get(1)*Y.get(0));
}

// This overloaded version of member function cross allows one to
// execute calls such as Z = X.cross(Y) where X, Y and Z are
// threevectors:

threevector threevector::cross(const threevector& Y) const
{
   threevector X(e[0],e[1],e[2]);
   threevector crossproduct;
   crossproduct.cross(X,Y);
   return crossproduct;
}

twovector threevector::xy_projection() const
{
   return twovector(get(0),get(1));
}

threematrix threevector::outerproduct(const threevector& Y) const
{
//   cout << "inside threevector::outerproduct()" << endl;
   threematrix B;

   for (int i=0; i<3; i++)
   {
      for (int j=0; j<3; j++)
      {
         B.put(i,j,get(i)*Y.get(j));
      }
   }
   return B;
}

// ---------------------------------------------------------------------
// Member function generate_antisymmetric_matrix() returns eps_ijk *
// (*this_k).  Such 3x3 matrix representations for threevectors pop up
// frequently in computer vision literature.

genmatrix* threevector::generate_antisymmetric_matrix() const
{
   double x=get(0);
   double y=get(1);
   double z=get(2);

// In Appendix 1 of "Multi-View Geometry in Computer Vision" by
// Hartley and Zisserman, the antisymmetric matrix corresponding to
// threevector a is defined as 

// 		( [a]_cross )_ik = eps_ijk a^j = - eps_ikj a^j

   genmatrix* antisym_ptr=new genmatrix(3,3);
   antisym_ptr->clear_values();
   antisym_ptr->put(0,1,z);
   antisym_ptr->put(0,2,-y);
   antisym_ptr->put(1,0,-z);
   antisym_ptr->put(1,2,x);
   antisym_ptr->put(2,0,y);
   antisym_ptr->put(2,1,-x);

   *antisym_ptr *= -1;

   return antisym_ptr;
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

threevector operator+ (const threevector& X,const threevector& Y)
{
   return threevector(X.get(0)+Y.get(0),X.get(1)+Y.get(1),X.get(2)+Y.get(2));
}

// Overload - operator:

threevector operator- (const threevector& X,const threevector& Y)
{
   return threevector(X.get(0)-Y.get(0),X.get(1)-Y.get(1),X.get(2)-Y.get(2));
}

// Overload - operator:

threevector operator- (const threevector& X)
{
   return threevector(-X.get(0),-X.get(1),-X.get(2));
}

// Overload * operator:

threevector operator* (int a,const threevector& X)
{
   return threevector(a*X.get(0),a*X.get(1),a*X.get(2));
}

threevector operator* (double a,const threevector& X)
{
   return threevector(a*X.get(0),a*X.get(1),a*X.get(2));
}

// On 7/31/06, we learned that the following overloaded version of
// operator* appears to conflict with another overloading of operator*
// in line 65 of osgParticle/range within OSG1.0.  This strikes us as
// an OSG defect.  But unless and until it is fixed within OSG, we
// have to be wiling to comment out the next method...

/*
threevector operator* (const threevector& X,int a)
{
   return a*X;
}
*/

threevector operator* (const threevector& X,double a)
{
   return a*X;
}

// Overload / operator:

threevector operator/ (const threevector& X,double a)
{
   return threevector(X.get(0)/a,X.get(1)/a,X.get(2)/a);
}

// This overloaded version of friend function operator* takes in a
// column threevector X, multiplies it by 3x3 matrix A and returns a
// column threevector:

threevector operator* (const genmatrix& A,const threevector& X)
{
   if (A.get_mdim()==3 && A.get_ndim()==3)
   {
      threevector Y;
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
      cout << "Error in operator* friend function of threevector class!"
           << endl;
      cout << "A.mdim = " << A.get_mdim() 
           << " A.ndim = " << A.get_ndim() << endl;
      cout << "Cannot multiply matrix A by column threevector X!" << endl;
      exit(-1);
   }
}

// This overloaded version of operator* takes in a row threevector X,
// multiplies it by 3x3 matrix A, and returns a row threevector:

threevector operator* (const threevector& X,const genmatrix& A)
{
   if (A.get_mdim()==3 && A.get_ndim()==3)
   {
      threevector Ytrans(0,0,0);
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
      cout << "Error in operator* friend function of threevector class!"
           << endl;
      cout << "A.mdim = " << A.get_mdim() 
           << " A.ndim = " << A.get_ndim() << endl;
      cout << "Cannot multiply matrix A by row threevector X!" << endl;
      exit(-1);
   }
}
