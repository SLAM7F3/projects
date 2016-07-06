// ==========================================================================
// Header file for complex class
// ==========================================================================
// Last modified on 3/31/12
// ==========================================================================

#ifndef MY_COMPLEX_H
#define MY_COMPLEX_H

#include <complex>

class complex
{

   typedef std::complex<double> double_complex;

  public:

   complex();
   complex(double real);
   complex(double real,double imag);
   complex polar_form(double modulus, double argument) const;
   complex(double_complex zdc);

   ~complex();
   complex(const complex& c);
   complex& operator= (const complex& c);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const complex& c);

// Set and get member functions:

   void set_real(double x);
   double get_real() const;
   void set_imag(double y);
   double get_imag() const;
   double get_mod() const;
   double get_arg() const;

// Complex number properties member functions:

   bool is_real() const;
   bool is_imaginary() const;

// Complex number manipulation member functions:

   complex Conjugate();
   complex power(int n) const;
   complex power(double alpha) const;

   void operator+= (double x1);
   void operator+= (complex z1);
   void operator-= (double x1);
   void operator-= (complex z1);
   void operator*= (double  x1);
   void operator*= (complex z1);
   void operator/= (double  x1);
   void operator/= (complex z1);

   complex operator= (int a);
   complex operator= (double a);

   friend complex operator+ (double a, complex z);
   friend complex operator+ (complex z, double a);
   friend complex operator+ (complex z1, complex z2);

   friend complex operator- (complex z);
   friend complex operator- (double a, complex z);
   friend complex operator- (complex z, double a);
   friend complex operator- (complex z1, complex z2);

   friend complex operator* (double a, complex z);
   friend complex operator* (complex z, double a);
   friend complex operator* (complex z1, complex z2);

   friend complex operator/ (double a, complex z);
   friend complex operator/ (complex z, double a);
   friend complex operator/ (complex z1, complex z2);

  private: 

   double_complex z,zbar;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const complex& c);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void complex::set_real(double x)
{
   z=double_complex(x,get_imag());
}

inline double complex::get_real() const
{
   return z.real();
}

inline void complex::set_imag(double y)
{
   z=double_complex(get_real(),y);
}

inline double complex::get_imag() const
{
   return z.imag();
}

inline double complex::get_mod() const
{
   return abs(z);
}

inline double complex::get_arg() const
{
   return arg(z);
}



#endif  // my_complex.h
