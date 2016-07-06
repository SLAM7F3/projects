// =========================================================================
// Complex class member function definitions
// =========================================================================
// Last modified on 3/31/12
// =========================================================================

#include <iostream>
#include "math/basic_math.h"
#include "math/complex.h"
#include "math/constants.h"

using std::cout;
using std::endl;
using std::flush;
using std::ostream;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void complex::allocate_member_objects()
{
}

void complex::initialize_member_objects()
{
   z=double_complex(0,0);
}		 

// ---------------------------------------------------------------------
complex::complex()
{
   allocate_member_objects();
   initialize_member_objects();
}

complex::complex(double r)
{
   allocate_member_objects();
   initialize_member_objects();
   z=double_complex(r,0);
}

complex::complex(double r,double i)
{
   allocate_member_objects();
   initialize_member_objects();
   z=double_complex(r,i);
}

complex::complex(double_complex zdc)
{
   allocate_member_objects();
   initialize_member_objects();
   z=zdc;
}

complex complex::polar_form(double modulus, double argument) const
{
   return complex(modulus*cos(argument),modulus*sin(argument));
}

// ---------------------------------------------------------------------
// Copy constructor:

complex::complex(const complex& c)
{
   docopy(c);
}

complex::~complex()
{
}

// ---------------------------------------------------------------------
void complex::docopy(const complex& c)
{
   z=c.z;
}

// Overload = operator:

complex& complex::operator= (const complex& c)
{
   if (this==&c) return *this;
   docopy(c);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const complex& z)
{
   outstream << z.get_real() << " + " << z.get_imag() << " I ";
   return outstream;
}

// =========================================================================
// Complex number property member functions
// =========================================================================

bool complex::is_real() const
{
   return nearly_equal(get_imag(),0,1E-10);
}

bool complex::is_imaginary() const
{
   return nearly_equal(get_real(),0,1E-10);
}

// =========================================================================
// Complex number manipulation member functions
// =========================================================================

complex complex::Conjugate() 
{
   zbar=conj(z);
   return complex(zbar);
}

complex complex::power(int n) const
{
   return complex(pow(z,n));
}

complex complex::power(double alpha) const
{
   double r=get_mod();
   double wr=pow(r,alpha);

// Reset negative values for theta to positive values before taking
// powers.  This method then returns fractional powers on the first
// Riemann sheet:

   double theta=get_arg();
   while (theta <0)
   {
      theta += 2*PI;
   }

   double wtheta=alpha*theta;
   return polar_form(wr,wtheta);
}

// Overload += operator:

void complex::operator+= (double x1)
{
   z += double_complex(x1,0);
}
  
void complex::operator+= (complex z1)
{
   z += double_complex(z1.get_real(),z1.get_imag());
}

// Overload -= operator:

void complex::operator-= (double x1)
{
   z -= double_complex(x1,0);
}
  
void complex::operator-= (complex z1)
{
   z -= double_complex(z1.get_real(),z1.get_imag());
}

// Overload *= operator:

void complex::operator*= (double x1)
{
   z *= double_complex(x1,0);
}
  
void complex::operator*= (complex z1)
{
   z *= double_complex(z1.get_real(),z1.get_imag());
}

// Overload /= operator:

void complex::operator/= (double x1)
{
   z /= double_complex(x1,0);
}
  
void complex::operator/= (complex z1)
{
   z /= double_complex(z1.get_real(),z1.get_imag());
}

// Overload = operator:

complex complex::operator= (int a)
{
   return complex(a,0);
}

complex complex::operator= (double a)
{
   return complex(a,0);
}

// Overload + operator:

complex operator+ (double a, complex z)
{
   return complex(a+z.get_real(),z.get_imag());
}

complex operator+ (complex z,double a)
{
   return complex(z.get_real()+a,z.get_imag());
}

complex operator+ (complex z1, complex z2)
{
   return complex(z1.get_real()+z2.get_real(),z1.get_imag()+z2.get_imag());
}

// Overload - operator:

complex operator- (complex z)
{
   return complex(-z.get_real(),-z.get_imag());
}

complex operator- (double a, complex z)
{
   return complex(a-z.get_real(),-z.get_imag());
}

complex operator- (complex z, double a)
{
   return complex(z.get_real()-a,z.get_imag());
}

complex operator- (complex z1, complex z2)
{
   return complex(z1.get_real()-z2.get_real(),z1.get_imag()-z2.get_imag());
}

// Overload * operator:

complex operator* (double a, complex z)
{
   return complex(a*z.get_real(),a*z.get_imag());
}

complex operator* (complex z,double a)
{
   return complex(z.get_real()*a,z.get_imag()*a);
}

complex operator* (complex z1, complex z2)
{
   return complex(
      z1.get_real()*z2.get_real()-z1.get_imag()*z2.get_imag(),
      z1.get_real()*z2.get_imag()+z1.get_imag()*z2.get_real());
}

// Overload / operator:

complex operator/ (double a, complex z)
{
   double denom=z.get_real()*z.get_real()+z.get_imag()*z.get_imag();
   return complex(a*z.get_real()/denom,-a*z.get_imag()/denom);
}

complex operator/ (complex z, double a)
{
   return complex(z.get_real()/a,z.get_imag()/a);
}

complex operator/ (complex z1, complex z2)
{
   double denom=z2.get_real()*z2.get_real()+z2.get_imag()*z2.get_imag();
   return complex(
      (z1.get_real()*z2.get_real()+z1.get_imag()*z2.get_imag())/denom,
      (z1.get_imag()*z2.get_real()-z1.get_real()*z2.get_imag())/denom);
}
