// ==========================================================================
// "Primitive" math functions 
// ==========================================================================
// Last updated on 6/26/14; 8/10/15; 8/12/15; 2/10/16
// ==========================================================================

#include <algorithm>
#include <iomanip>
#include <math.h>
#include <new>
#include <set>
#include <stdlib.h> // Needed for abs() & system() to perform Unix calls
#include <string>
#include "math/complex.h"
#include "math/constant_vectors.h"
#include "datastructures/descriptor.h"
#include "general/filefuncs.h"
#include "math/fourvector.h"
#include "math/genmatrix.h"
#include "math/genvector.h"
#include "math/mathfuncs.h"
#include "templates/mytemplates.h" 
#include "numrec/nr.h"
#include "numrec/nrfuncs.h"
#include "numrec/nrutil.h"
#include "general/outputfuncs.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/threevector.h"
#include "math/twovector.h"

using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

namespace mathfunc
{

// ==========================================================================
// Base-32 methods:
// ==========================================================================

// Method encode_base32() takes in nonnegative integer i in base 10.
// It returns a string which does not contain I,L,O or U letters that
// represents the integer in base 32.  See
// http://www.crockford.com/wrmg/base32.html. 

   string encode_base32(int i)
   {
      const string base32_encode="0123456789ABCDEFGHJKMNPQRSTVWXYZ";

      string s;

      if (i==0) s="0";

//      int iter=0;
      while (i > 0)
      {
//         cout << "iter = " << iter++ << " i = " << i
//              << " s = " << s << endl;
         int digit=i%32;
         i=basic_math::mytruncate(i/32);
         char curr_char=base32_encode[digit];
         s=curr_char+s;
      }

      return s;
   }

// ---------------------------------------------------------------------   
// Method decode_base32() takes in string s which is assumed to
// contain some nonnegative integer in base32 representation.  It
// returns the integer's value in conventional base 10 format.

   int decode_base32(string s)
   {
      const string base32_encode="0123456789ABCDEFGHJKMNPQRSTVWXYZ";

      int decoded_integer=0;
      for (unsigned int j=0; j<s.size(); j++)
      {
//         cout << "j = " << j << " s[j] = " << s[j] << endl;

         string curr_char_str=stringfunc::char_to_string(s[j]);
         int char_posn=stringfunc::first_substring_location(
            base32_encode,curr_char_str,0);
//         cout << "char_posn = " << char_posn << endl;

         int pow=1;
         for (unsigned int k=0; k<s.size()-1-j; k++)
         {
            pow *= 32;
         }

         decoded_integer += char_posn*pow;
      } // loop over index j
      return decoded_integer;
   }

// ==========================================================================
// Casting methods:
// ==========================================================================

// In Dec 2005, we were unpleasantly surprised to discover that
// performing conventional C++ casts of doubles down to floats leads
// to significant numerical errors.  So we have written the following
// little method which attempts to return a float whose numerical
// value is closer to its double progenitor.  Sometimes this algorithm
// works and sometimes it doesn't!  As of Dec 2005, we have concluded
// that working with floats inevitably raises the danger of
// significant roundoff error...

   float double_to_float(const double& d)
   {
      float f=static_cast<float>(d);
      double diff=d-f;
      f += static_cast<float>(diff);
      return f;
   }

// ==========================================================================
// Combinatoric methods:
// ==========================================================================

// Method correlation_coeff computes the linear correlation
// coefficient -1 <= rho <= 1.  rho = 1 when the data pairs in input
// arrays x and y lie on a perfect straight line with positive slope.
// If the data points lie on a perfect straight line with negative
// slope, r has value -1.  See section 14.5 in Numerical Recipes.

   double correlation_coeff(int npoints,double x[],double y[])
   {
      double xterm,yterm;
      double numer=0;
      double denom1=0;
      double denom2=0;
      double xavg=templatefunc::average(x,npoints);
      double yavg=templatefunc::average(y,npoints);
   
      for (int i=0; i<npoints; i++)
      {
         xterm=x[i]-xavg;
         yterm=y[i]-yavg;
         numer += xterm*yterm;
         denom1 += sqr(xterm);
         denom2 += sqr(yterm);
      }
      double rho=numer/sqrt(denom1*denom2);
      return rho;
   }

// ---------------------------------------------------------------------
// Method choose_numerator returns the product (n choose i) * i! =
// n*(n-1)*(n-2)*...*(n-i+1)

   int choose_numerator(int n, int i)
   {
      if (i > n)
      {
         return 0;
      }
      else
      {
         int numer=1;
         for (int j=0; j<i; j++)
         {
            numer *= (n-j);
         }
         return numer;
      }
   }

   int choose(int n,int i)
   {
      if (n < i)
      {
         return 0;
      }
      else
      {
         int j;
         if (i < n-i)
         {
            j=i;
         }
         else
         {
            j=n-i;
         }
         return basic_math::round(choose_numerator(n,j)/factorial(j));
      }
   }

/* 
// ---------------------------------------------------------------------
// Note: Our original simple minded recursive factorial routine
// overflows for n > 12.  So on 8/4/99, we replaced it with a call to
// Gamma(n+1) which appears to return valid factorials up to n=170.
// For n > 170, Gamma(n+1) returns "Infinity".  

// In order to speed up processing, we hard-wire in factorial results
// for the first 12 integers:

int factorial(int n)
{
const int nfactorials=12;
static int f[nfactorials+1]=
{1,1,2,6,24,120,720,5040,40320,362880,3628800,39916800,479001600
};
         
if (n >= 0 && n <= nfactorials)
{
return f[n];
}
//         else if (n >= nfactorials+1 && n < 170)
//         {
//            return Gamma(n+1);
//         }
else
{
cout << "Method mathfunc::factorial() can't return value for ";
cout << "n = " << n << " !!!" << endl;
return POSITIVEINFINITY;
}
}
*/

// ---------------------------------------------------------------------
// Method lower_incomplete_gamma() returns the lower incomplete gamma
// function defined by 

// 	gamma(s,z) = integral_0^z t^(s-1) exp(-t) dt

// Special values:  gamma(s,0) = 0
//		    gamma(1,z) = 1 - exp(-z)
//		    gamma(0.5,z) = sqrt(PI) * erf(sqrt(z))

     double lower_incomplete_gamma(double s, double z)
     {
       double Gamma = mathfunc::Gamma(s);
       return Gamma * numrec::gammp(s,z);
     }
     
// As of 8/12/15, this next implementation of incomplete_gamma() is deprecated!

/*
// Source taken from "How to Calculate the Chi-Squared P-Value" by
// Jacob F.W, 5 Nov 2012,
// http://www.codeproject.com/Articles/432194/How-to-Calculate-the-Chi-Squared-P-Value

      double incomplete_gamma(double S, double Z)
      {
         if(Z < 0.0)
         {
            return 0.0;
         }
         double Sc = (1.0 / S);
         Sc *= pow(Z, S);
         Sc *= exp(-Z);
 
         double Sum = 1.0;
         double Nom = 1.0;
         double Denom = 1.0;
 
         for(int I = 0; I < 200; I++)
         {
            Nom *= Z;
            S++;
            Denom *= S;
            Sum += (Nom / Denom);
         }
         return Sum * Sc;
      }
*/

// ---------------------------------------------------------------------
// Method cumulative_chisq_prob() returns chisq cumulative distribution
// function F(x; k) = lower_incomplete_gamma(0.5 k, 0.5 x)/ Gamma(0.5 k).

      double cumulative_chisq_prob(double x, double k)
      {
// 	 cout << "inside cumulative_chisq_prob() " << endl;
         double s = 0.5 * k;
         double z = 0.5 * x;
//         cout << "s = " << s << " z = " << z << endl;
         double cum_prob = numrec::gammp(s, z);
//         cout << "cum_prob = " << cum_prob << endl;
         return cum_prob;
      }

// ---------------------------------------------------------------------
// Function stirling returns Stirling's approximation to log(z!):

    double stirling(double x)
    {
       double term1=(x+0.5)*log(x+1);
       double term2=-(x+1);
       double term3=0.5*log(2*PI);
   
       double subterm1=1;
       double subterm2=1.0/(12.0*(x+1));
       double subterm3=1.0/(288.0*sqr(x+1));
       double subterm4=-139/(51840.0*real_power(x+1,3));
       double subterm5=-571.0/(2488320*real_power(x+1,4));
       double term4=log(subterm1+subterm2+subterm3+subterm4+subterm5);

       return term1+term2+term3+term4;
    }

// ==========================================================================
// Digit methods:
// ==========================================================================

// Method cubic_poly_discriminant() computes the discriminant for a
// cubic polynomial from its coefficients.

    double cubic_poly_discriminant(const vector<double>& coeffs)
    {
       double a=coeffs[3];
       double b=coeffs[2];
       double c=coeffs[1];
       double d=coeffs[0];
       return cubic_poly_discriminant(a,b,c,d);
    }
   
    double cubic_poly_discriminant(double a,double b,double c,double d)
    {

// Note: cubic poly = a x**3 + b x**2 + c x + d

       double Delta=
          18*a*b*c*d - 4*b*b*b*d + b*b*c*c - 4*a*c*c*c - 27*a*a*d*d;
       return Delta;
    }

// ---------------------------------------------------------------------
// Method n_real_cubic_roots() returns the number of real roots of a
// cubic polynomial based upon the sign of its discriminant.

    int n_real_cubic_roots(double Delta)
    {
       if (Delta==0)
       {
          return 2;
       }
       else if (Delta > 0)
       {
          return 3;
       }
       else // if (Delta < 0)
       {
          return 1;
       }
    }

// ---------------------------------------------------------------------
// Method real_cubic_roots() implements the "general formula of roots"
// for a cubic polynomial presented in
// http://en.wikipedia.org/wiki/Cubic_function .  It returns either
// one or three real roots within an STL vector.

    vector<double> real_cubic_roots(const vector<double>& coeffs)
    {
//       cout << "inside mathfunc::real_cubic_roots()" << endl;
       return real_cubic_roots(coeffs[3],coeffs[2],coeffs[1],coeffs[0]);
    }
   
    vector<double> real_cubic_roots(double a,double b,double c,double d)
    {
//       cout << "inside mathfunc::real_cubic_roots()" << endl;

// Note: cubic poly = a x**3 + b x**2 + c x + d

       double Delta=cubic_poly_discriminant(a,b,c,d);
       int n_real_roots=n_real_cubic_roots(Delta);
//      cout << "n_real_roots = " << n_real_roots << endl;

       complex sqrQ=-27*a*a*Delta;
//      cout << "sqrQ = " << sqrQ << endl;
       double sqr_rQ=sqrQ.get_mod();
       double two_theta_Q=sqrQ.get_arg();
      
       double rQ=sqrt(sqr_rQ);
       double theta_Q=0.5*two_theta_Q;
       complex Q;
       Q=Q.polar_form(rQ,theta_Q);
       if (Delta < 0)
       {
          Q.set_imag(0);
       }
       else
       {
          Q.set_real(0);
       }
//      cout << "Q = " << Q << endl;
//      cout << "Q*Q = " << Q*Q << endl;

       complex cubeC=0.5*(Q+2*b*b*b-9*a*b*c+27*a*a*d);
//      cout << "cubeC = " << cubeC << endl;
       double cube_rC=cubeC.get_mod();
       double three_theta_C=cubeC.get_arg();
       double rC=pow(cube_rC,1.0/3.0);
       double theta_C=three_theta_C/3.0;
       complex C;
       C=C.polar_form(rC,theta_C);
//      cout << "C = " << C << endl;
//      cout << "C*C*C = " << C*C*C << endl;
      
       double term1=-b/(3*a);
       complex term2=C/(3*a);
       complex term3=(b*b-3*a*c)/(3*a*C);
       complex term4(1,sqrt(3.0));
       complex term5(1,-sqrt(3.0));

       complex z1=term1-term2-term3;
       complex z2=term1+0.5*term2*term4+0.5*term5*term3;
       complex z3=term1+0.5*term2*term5+0.5*term4*term3;

//      cout << "z1 = " << z1 << endl;
//      cout << "z2 = " << z2 << endl;
//      cout << "z3 = " << z3 << endl;

       vector<double> real_cubic_roots;
       if (n_real_roots==3)
       {
          real_cubic_roots.push_back(z1.get_real());
          real_cubic_roots.push_back(z2.get_real());
          real_cubic_roots.push_back(z3.get_real());
       }
       else if (n_real_roots==1)
       {
          vector<double> imag_magnitudes;
          imag_magnitudes.push_back(fabs(z1.get_imag()));
          imag_magnitudes.push_back(fabs(z2.get_imag()));
          imag_magnitudes.push_back(fabs(z3.get_imag()));
          vector<complex> cubic_roots;
          cubic_roots.push_back(z1);
          cubic_roots.push_back(z2);
          cubic_roots.push_back(z3);
          templatefunc::Quicksort(imag_magnitudes,cubic_roots);
          real_cubic_roots.push_back(cubic_roots.front().get_real());
       }

       return real_cubic_roots;
    }
   
   
// ==========================================================================
// Digit methods:
// ==========================================================================

// Method digit_decomposition takes in an integer and returns an STL
// vector containing the digits of its absolute value.

    vector<int> digit_decomposition(int number)
    {
       vector<int> digit;
       if (number == 0)
       {
          digit.push_back(0);
       }
       else
       {
          if (number < 0) number *= -1;
          int power_of_ten=basic_math::round(trunclog(number));
          int ndigits=basic_math::round(log10(power_of_ten))+1;
          for (int n=ndigits-1; n>=0; n--)
          {
             power_of_ten=basic_math::round(pow(10,n));
             digit.push_back(number/power_of_ten);
             number -= power_of_ten*digit.back();
          }
       }
       return digit;
    }

// ---------------------------------------------------------------------
// Method digit_sum takes in an integer and returns the sum of its digits.

    int digit_sum(int number)
    {
       vector<int> digit=digit_decomposition(number);
       int digitsum=0;
       for (unsigned int i=0; i<digit.size(); i++)
       {
          digitsum += digit[i];
       }
       return digitsum;
    }

// ---------------------------------------------------------------------
// Method ndigits_before_decimal_point() takes in x and strips off its
// fractional part.  It then decomposes the digits within the integer
// part of x and returns the number of digits.

    int ndigits_before_decimal_point(double x)
    {
       if (x < 0)
       {
          x=-x;
       }
       vector<int> V=mathfunc::digit_decomposition(
          basic_math::mytruncate(x));
       return int(V.size());
    }

// ---------------------------------------------------------------------
// Method ndigits_after_decimal_point() takes in x and strips off its
// integer part.  It then counts the number of powers of 10 needed to
// convert its fractional remainder into a whole number.  

    int ndigits_after_decimal_point(double x)
    {
//         int sgn=1;
       double fabs_x=fabs(x);
       if (fabs_x != x)
       {
//            sgn=-1;
          x=-x;
       }
//         cout << "sgn = " << sgn << endl;

       cout.precision(12);
       int x_int=basic_math::mytruncate(x);
//         cout << "x_int = " << x_int << endl;
       double frac=x-x_int;
       x=frac;

       int ndigits_after_decimal=0;
       const double TINY=1E-8;

       while (!basic_math::is_int(x,TINY))
       {
//            cout << "x = " << x << endl;
          x_int=basic_math::mytruncate(x);
//            cout << "x_int = " << x_int << endl;
          frac=x-x_int;
//            cout << "frac = " << frac << endl;

          ndigits_after_decimal++;
          x =frac*10;
       } // !done_flag while loop

       return ndigits_after_decimal;
    }

// ==========================================================================
// Discrete integer methods:
// ==========================================================================

// Method unique_integer takes in two ints i and j along with some
// upper bound on all possible i values.  It returns a unique integer
// which can replace (i,j) as a label.

    long long unique_integer(long long i,long long j,long long Imax)
    {
       return j*Imax+i;
    }

    void decompose_unique_integer(
       long long unique_int,long long Imax,
       long long& i, long long& j)
    {
       j=unique_int/Imax;
       i=unique_int%Imax;
    }

    long long unique_integer(
       long long i,long long j,long long k,long long l,
       long long Imax,long long Jmax,long long Kmax)
    {
       long long a=unique_integer(i,j,Imax);
       long long b=unique_integer(k,l,Kmax);
       long long Amax=Imax*Jmax;
       return b*Amax+a;
    }
   
    void decompose_unique_integer(
       long long unique_int,long long Imax,long long Jmax,long long Kmax,
       long long& i, long long& j,long long& k,long long& l)
    {
       long long Amax=Imax*Jmax;
       long long b=unique_int/Amax;
       long long a=unique_int%Amax;
       decompose_unique_integer(a,Imax,i,j);
       decompose_unique_integer(b,Kmax,k,l);
    }

// ==========================================================================
// Distribution methods:
// ==========================================================================

// Method gaussian returns a normalized gaussian with mean mu and
// standard deviation sigma:

    double gaussian(double x,double mu,double sig)
    {
       double numer=exp(-0.5*sqr((x-mu)/sig));
       double denom=SQRT_TWO_PI*sig;
       double frac=numer/denom;
       return frac;
    }

// ---------------------------------------------------------------------
// Method gaussian_random_var() returns a random variable which is
// distributed according to a normal distribution with mean mu and
// standard deviation sigma.  Since gaussian random vars are calculated in 
// pairs, we return one and save the other for the next call to this
// method.

    bool calculate_next_gaussian_random_vars_pair=false;
    double r1, r2;
    double gaussian_random_var(double mu, double sigma)
    {
       calculate_next_gaussian_random_vars_pair = 
          !calculate_next_gaussian_random_vars_pair;
       if (calculate_next_gaussian_random_vars_pair)
       {
          double x1=nrfunc::ran1();  
          double x2=nrfunc::ran1();  
          double term1=sigma*sqrt(-2*log(x1));
          double term2a=cos(2*PI*x2);
          double term2b=sin(2*PI*x2);
          r1=mu+term1*term2a;
          r2=mu+term1*term2b;
          return r1;
       }
       else
       {
          return r2;
       }
    }

// ---------------------------------------------------------------------
// Method lognormal returns *two* random variables which are
// distributed according to a lognormal distribution with mean mu and
// standard deviation sigma.  The lognormal distribution is given by
// p(x) = 1/(sqrt(2*PI) sigma x) exp[ -1/2 (log(x)-mu)^2/sigma^2 ].
// This routine essentially just returns the exponential of two random
// variables which are distributed according to a gaussian with mean
// mu and standard deviation sigma:

    void lognormal(double mu,double sigma,double& r1,double& r2)
    {
       double x1=nrfunc::ran1();  
       double x2=nrfunc::ran1();  
       double term1=sigma*sqrt(-2*log(x1));
       double term2a=cos(2*PI*x2);
       double term2b=sin(2*PI*x2);
       r1=exp(mu+term1*term2a);
       r2=exp(mu+term1*term2b);
    }

// ==========================================================================
// Factoring methods:
// ==========================================================================

// Method factor_integer implements a brute force search for the
// factors of an input positive integer.  It returns an array
// containing the factors which is sorted in ascending order.

    int* factor_integer(int number,int& nfactors)
    {
       const int max_nfactors=100;

       int *factor=new int[max_nfactors];
       int number_of_bytes=max_nfactors*sizeof(*factor);
       memset(factor,0,number_of_bytes);

       nfactors=0;
       factor[nfactors++]=1;

       int istart,istop;
       if (number > 2)
       {
          istart=2;
          istop=basic_math::mytruncate(sqrt(double(number)))+1;
          for (int i=istart; i<=istop; i++)
          {
             if (number%i==0)
             {
                factor[nfactors++]=i;
                if (number/i != i)
                {
                   factor[nfactors++]=number/i;
                }
         
             }
          } // loop over index i 
       } // number > 0 conditional

       factor[nfactors++]=number;

       Quicksort(factor,nfactors);
       return factor;
    }


    bool is_prime(int n)
    {
       if (n<2) 	// Recall 0, 1 and all negative integers are NOT prime
       {
          return false;
       }
       else
       {
          int nfactors;
          factor_integer(n,nfactors);
          return (nfactors==2);
       }
    }

// ---------------------------------------------------------------------
// Method prime_factors implements a brute force search for the prime
// factors of an input positive integer.  It returns an STL vector
// containing just the individual prime factors (but not their
// frequency) which is sorted in ascending order.

    vector<int> prime_factors(int number)
    {
       vector<int> factor;

       if (number > 1)
       {
          int istart=2;
          int istop=basic_math::mytruncate(number/2)+1;
          for (int i=istart; i<istop; i++)
          {
             if (is_prime(i) && number%i==0)
             {
                factor.push_back(i);
             }
          } // loop over index i 
       } // number > 1 conditional

       std::sort(factor.begin(),factor.end());

       return factor;
    }

// ==========================================================================
// Fitting methods:
// ==========================================================================

// Method fit_plane takes in n (x,y,z) triples within input arrays
// x[], y[] and z[].  It returns the coefficients (a,b,c) of the
// best-fit plane (obtained via chisq minimization) z=ax+by+c.

// As of Jan 2006, this method is deprecated.  Use src/geometry/plane
// class instead...

    void fit_plane(int n,double x[],double y[],double z[],
                   double& a,double& b,double &c)
    {
       double S=0;
       double Sx=0;
       double Sy=0;
       double Sz=0;
       double Sxx=0;
       double Syy=0;
       double Sxy=0;
       double Sxz=0;
       double Syz=0;
         
       for (int i=0; i<n; i++)
       {
          S += 1;
          Sx += x[i];
          Sy += y[i];
          Sz += z[i];
          Sxx += sqr(x[i]);
          Syy += sqr(y[i]);
          Sxy += x[i]*y[i];
          Sxz += x[i]*z[i];
          Syz += y[i]*z[i];
       } // loop over index i
       double detMinv=1.0/(S*Sxx*Syy+2*Sx*Sy*Sxy-Sxx*sqr(Sy)-Syy*sqr(Sx)
                           -S*sqr(Sxy));
       a=(Sxz*(S*Syy-sqr(Sy))+Syz*(Sx*Sy-S*Sxy)+Sz*(Sy*Sxy-Sx*Syy))
          *detMinv;
       b=(Sxz*(Sx*Sy-S*Sxy)+Syz*(S*Sxx-sqr(Sx))+Sz*(Sx*Sxy-Sy*Sxx))
          *detMinv;
       c=(Sxz*(Sy*Sxy-Sx*Syy)+Syz*(Sx*Sxy-Sy*Sxx)+Sz*(Sxx*Syy-sqr(Sxy)))
          *detMinv;
    }

// ---------------------------------------------------------------------
// Method simple_linear_fit takes in two STL vectors which are assumed
// to be filled with measured (x,y) data points to which we want to
// fit a straight line.  The sigma_y values for each of these points
// is assumed to equal a constant.  This method returns coefficents a
// and b for the best-line fit y(x)=a+bx.  See section 15.2 in
// Numerical Recipes.  

    void simple_linear_fit(
       vector<double>& x,vector<double>& y,double& a,double& b)
    {
       double S,Sx,Sy,Sxx,Sxy;
       S=Sx=Sy=Sxx=Sxy=0;
       for (unsigned int i=0; i<x.size(); i++)
       {
          S += 1;
          Sx += x[i];
          Sy += y[i];
          Sxx += sqr(x[i]);
          Sxy += x[i]*y[i];
       }
       double Delta=S*Sxx-sqr(Sx);
       a=(Sxx*Sy-Sx*Sxy)/Delta;
       b=(S*Sxy-Sx*Sy)/Delta;
    }

/*
// ---------------------------------------------------------------------
// Method quadratic_fit returns the coeffs of the unique quadratic
// which passes through 3 specified (x,y) points.  The 3 inputs are
// contained within STL vector P, while the output quadratic is of the
// form y(x) = a x**2 + b x + c.

void quadratic_fit(const vector<twovector>& P,double& a,double& b,
double& c)
{
double x0=P[0].get(0);
double x1=P[1].get(0);
double x2=P[2].get(0);
double y0=P[0].get(1);
double y1=P[1].get(1);
double y2=P[2].get(1);
         
genmatrix M(3,3),Minv(3,3);
M.put(0,0,sqr(x0));
M.put(0,1,x0);
M.put(0,2,1);
M.put(1,0,sqr(x1));
M.put(1,1,x1);
M.put(1,2,1);
M.put(2,0,sqr(x2));
M.put(2,1,x2);
M.put(2,2,1);
if (M.inverse(Minv))
{
threevector Y(y0,y1,y2);
threevector coeffs=Minv*Y;
a=coeffs.get(0);
b=coeffs.get(1);
c=coeffs.get(2);
}
else
{
cout << "Error in mathfunc::quadratic_fit()" << endl;
cout << "Cannot invert matrix M = " << M << endl;
}
         
//         cout << "a = " << a << " b = " << b << " c = " << c << endl;
}
*/

// ---------------------------------------------------------------------

/*
// As of 7/26/13, we believe this next method is messed up!  Use
// affine_transform class instead.


// Method fit_2D_affine_transformation() takes in at least 3 pairs of
// twovectors q_vecs and p_vecs.  It performs a least-squares fit for
// 2x2 matrix A and translation twovector trans which optimally
// satisfy

//	 		p_i = A_ij * q_j + trans_i

// This method computes score function S = sum_i (p_i - [Aq+t]_i )**2.
// It returns the root-mean-square RMS=sqrt(S/q_vecs.size()).

// See H. Spath, "Fitting affine and orthogonal transformations
// between two sets of points", Mathematical Communications 9 (2004)
// 27-34.

double fit_2D_affine_transformation(
const vector<twovector>& q_vecs,const vector<twovector>& p_vecs,
genmatrix& A,twovector& trans)
{
int m=q_vecs.size();
if (m < 3) return POSITIVEINFINITY;

vector<threevector> qtilde_vecs;
for (int i=0; i<m; i++)
{
qtilde_vecs.push_back(threevector(q_vecs[i],1));
}
   
genmatrix Qtilde(3,3),Ctilde(3,2);
Qtilde.clear_values();
Ctilde.clear_values();
for (int i=0; i<m; i++)
{
Qtilde += qtilde_vecs[i].outerproduct(qtilde_vecs[i]);
Ctilde += qtilde_vecs[i].outerproduct(p_vecs[i]);
}
//      cout << "Qtilde = " << Qtilde << endl;
//      cout << "Ctilde = " << Ctilde << endl;

genmatrix Qtilde_inverse(3,3);
Qtilde.inverse(Qtilde_inverse);
//      cout << "Qtilde_inverse = " << Qtilde_inverse << endl;
//      cout << "Qtilde_inverse*Qtilde = " << Qtilde_inverse * Qtilde << endl;
   
genmatrix Atilde(3,2);
Atilde=Qtilde_inverse*Ctilde;
//      cout << "Atilde = " << Atilde << endl;

genmatrix Atrans(2,2);
Atilde.get_smaller_matrix(Atrans);
A=Atrans.transpose();
//      cout << "A = " << A << endl;

trans=twovector(Atilde.get(2,0),Atilde.get(2,1));
//      cout << "trans = " << trans << endl;

return score_2D_affine_transformation_fit(q_vecs,p_vecs,A,trans);
}

// ---------------------------------------------------------------------
// Method score_2D_affine_transformation_fit() takes in 2x2 matrix A
// and translation twovector trans which map twovector q_j to
// twovector p_i via

//	 		p_i = A_ij * q_j + trans_i

// It computes score function S = sum_i (p_i - [Aq+t]_i )**2.
// and returns the root-mean-square RMS=sqrt(S/q_vecs.size()).

double score_2D_affine_transformation_fit(
const vector<twovector>& q_vecs,const vector<twovector>& p_vecs,
const genmatrix& A,const twovector& trans)
{
unsigned int m=q_vecs.size();
double S=0;
for (int i=0; i<m; i++)
{
twovector p_comp=A*q_vecs[i]+trans;
S += (p_comp-p_vecs[i]).sqrd_magnitude();
//         cout << "i = " << i << " p_comp = " << p_comp
//              << " p_vec = " << p_vecs[i] << endl;
}
//      cout << "S = " << S << endl;
double RMS=sqrt(S/m);
return RMS;
}
*/

// ---------------------------------------------------------------------
// Method fit_2D_translation() takes in at least 1 pair of twovectors
// q_vecs and p_vecs.  It returns the averaged translation twovector
// trans which satisfies

//	 		p_i = q_i + trans_i

// This method computes score function S = sum_i (p_i - [Aq+t]_i )**2.
// It returns the root-mean-square RMS=sqrt(S/q_vecs.size()).

   double fit_2D_translation(
      const vector<twovector>& q_vecs,const vector<twovector>& p_vecs,
      genmatrix& A,twovector& trans)
   {
      trans.clear_values();

      unsigned int m=q_vecs.size();
      for (unsigned int i=0; i<m; i++)
      {
         trans += p_vecs[i]-q_vecs[i];
      }
      trans /= m;

      double S=0;
      for (unsigned int i=0; i<m; i++)
      {
         twovector p_comp=q_vecs[i]+trans;
         S += (p_comp-p_vecs[i]).sqrd_magnitude();
//         cout << "i = " << i << " p_comp = " << p_comp
//              << " p_vec = " << p_vecs[i] << endl;
      }
//      cout << "S = " << S << endl;
      double RMS=sqrt(S/m);
      return RMS;
   }

// ==========================================================================
// Quadrature methods:
// ==========================================================================

// Method simpsonsum returns "Simpson's" value for the sum of the
// elements within an array.  In order to convert this sum into a true
// integral, the result must be multiplied by the value for a single
// abscissa bin's stepsize.  The first 7 finite range formulas come
// from section 25.4 in "Handbook of mathematical functions" by
// Abramowitz and Stegun.  Unfortunately, as of 1/20/00, we no longer
// recall where the extended "Simpson" rule which we use to integrate
// arrays with 8 or more bins comes from.  Boo hoo...

   double simpsonsum(double f[],int startbin,int stopbin)
   {
      int nbins=stopbin-startbin+1;
      double currsum=0;
   
      if (nbins<=1)
      {
         currsum=0;
      }
      else if (nbins==2)
      {
         currsum=0.5*(f[startbin]+f[startbin+1]);
      }
      else if (nbins==3)
      {
         currsum=0.3333333333*(f[startbin]+4*f[startbin+1]+f[startbin+2]);
      }
      else if (nbins==4)
      {
         currsum=0.375*(f[startbin]+3*f[startbin+1]+3*f[startbin+2]
                        +f[startbin+3]);
      }
      else if (nbins==5)
      {
// 2/45 = 0.04444
         currsum=0.04444444444*(
            7*f[startbin]+32*f[startbin+1]+12*f[startbin+2]
            +32*f[startbin+3]+7*f[startbin+4]);
      }
      else if (nbins==6)
      {
// 5/288 = 0.017361111111
         currsum=0.01736111111*(
            19*f[startbin]+75*f[startbin+1]+50*f[startbin+2]
            +50*f[startbin+3]+75*f[startbin+4]+19*f[startbin+5]);
      }
      else if (nbins==7)
      {
// 1/140 = 0.0071242857143            
         currsum=0.007142857143*(
            41*f[startbin]+216*f[startbin+1]+27*f[startbin+2]
            +272*f[startbin+3]+27*f[startbin+4]
            +216*f[startbin+5]+41*f[startbin+6]);
      }
      else
      {
         double c1=0.3541666667;	// = 17/48
         double c2=1.229166667;	// = 59/48
         double c3=0.8958333333;	// = 43/48
         double c4=1.020833333;	// = 49/48
    
         currsum  = c1*(f[startbin]+f[stopbin]);
         currsum += c2*(f[startbin+1]+f[stopbin-1]);
         currsum += c3*(f[startbin+2]+f[stopbin-2]);
         currsum += c4*(f[startbin+3]+f[stopbin-3]);
   
         for (int i=startbin+4; i<=stopbin-4; i++)
         {
            currsum += f[i];
         }
      }
      return(currsum);
   }

   double simpsonsum(const vector<double>& f)
   {
      return simpsonsum(f,0,f.size()-1);
   }

   double simpsonsum(const vector<double>& f,int startbin,int stopbin)
   {
      int nbins=stopbin-startbin+1;
      double currsum=0;
   
      if (nbins<=1)
      {
         currsum=0;
      }
      else if (nbins==2)
      {
         currsum=0.5*(f[startbin]+f[startbin+1]);
      }
      else if (nbins==3)
      {
         currsum=0.3333333333*(f[startbin]+4*f[startbin+1]+f[startbin+2]);
      }
      else if (nbins==4)
      {
         currsum=0.375*(f[startbin]+3*f[startbin+1]+3*f[startbin+2]
                        +f[startbin+3]);
      }
      else if (nbins==5)
      {
// 2/45 = 0.04444
         currsum=0.04444444444*(
            7*f[startbin]+32*f[startbin+1]+12*f[startbin+2]
            +32*f[startbin+3]+7*f[startbin+4]);
      }
      else if (nbins==6)
      {
// 5/288 = 0.017361111111
         currsum=0.01736111111*(
            19*f[startbin]+75*f[startbin+1]+50*f[startbin+2]
            +50*f[startbin+3]+75*f[startbin+4]+19*f[startbin+5]);
      }
      else if (nbins==7)
      {
// 1/140 = 0.0071242857143            
         currsum=0.007142857143*(
            41*f[startbin]+216*f[startbin+1]+27*f[startbin+2]
            +272*f[startbin+3]+27*f[startbin+4]
            +216*f[startbin+5]+41*f[startbin+6]);
      }
      else
      {
         double c1=0.3541666667;	// = 17/48
         double c2=1.229166667;	// = 59/48
         double c3=0.8958333333;	// = 43/48
         double c4=1.020833333;	// = 49/48
    
         currsum  = c1*(f[startbin]+f[stopbin]);
         currsum += c2*(f[startbin+1]+f[stopbin-1]);
         currsum += c3*(f[startbin+2]+f[stopbin-2]);
         currsum += c4*(f[startbin+3]+f[stopbin-3]);
   
         for (int i=startbin+4; i<=stopbin-4; i++)
         {
            currsum += f[i];
         }
      }
      return currsum;
   }

// ==========================================================================
// Angle computation methods:
// ==========================================================================

// Method absolute_angle_between_unitvectors returns the positive
// angle and output axis direction vector which are needed to rotate
// input unitvector u_hat into v_hat.

   double absolute_angle_between_unitvectors(
      const threevector& u_hat,const threevector& v_hat)
   {
      return angle_and_axis_between_unitvectors(u_hat,v_hat).first;
   }
   
   pair<double,threevector> angle_and_axis_between_unitvectors(
      const threevector& u_hat,const threevector& v_hat)
   {
      threevector cross_product(u_hat.cross(v_hat));
      double costheta=u_hat.dot(v_hat);            
      double abs_sintheta=cross_product.magnitude();
      double abs_theta=atan2(abs_sintheta,costheta);

// Test whether cross product vanishes.  If so, u_hat and v_hat are
// colinear.  Axis between them is then undefined.  In this case, we
// arbitrarily return z_hat as the rotation axis:

      const double TINY=1E-5;
      threevector n_hat=z_hat;
      if (abs_sintheta > TINY)
      {
         n_hat=cross_product.unitvector();
      }
      else
      {
//         cout << "Warning in mathfunc::angle_and_axis_between_unitvectors()"
//              << endl;
//         cout << "Input u_hat and v_hat are collinear!" << endl;
//         cout << "u_hat = " << u_hat << endl;
      }
      return pair<double,threevector>(abs_theta,n_hat);
   }

// This overloaded version of angle_between_unitvectors returns the
// angle which unitvector v_hat makes with respect to unitvector u_hat
// measured in radians.  Both input unitvectors are assumed to lie
// within the xy plane.

   double angle_between_unitvectors(
      const threevector& u_hat,const threevector& v_hat)
   {
      threevector cross_product(u_hat.cross(v_hat));
//      double sgn_theta=sgn(cross_product.dot(z_hat));
      double sgn_theta=sgn(cross_product.get(2));
      double costheta=u_hat.dot(v_hat);            
      double sintheta=sgn_theta*cross_product.magnitude();
//      double theta=atan2(sintheta,costheta);
      return atan2(sintheta,costheta);
   }

// ==========================================================================
// Direction vector & basic construction methods:
// ==========================================================================

// Method decompose_direction_vector takes in unit vector n_hat and
// returns its azimuth and elevation angles phi and theta where

//   n_hat = cos(theta) cos(phi) x_hat + cos(theta) sin(phi) y_hat
// 	    + sin(theta) z_hat   

// In this method, the conventionally defined polar angle = PI/2-theta
// and not theta itself !  Theta represents an elevation angle which
// ranges over the interval [-PI/2, PI/2].

   void decompose_direction_vector(
      const threevector& n_hat,double& phi,double& theta)
   {
      theta=asin(n_hat.get(2));
      phi=0;
      if (!nearly_equal(fabs(n_hat.get(2)),1))
      {
         phi=atan2(n_hat.get(1),n_hat.get(0));
      }
   }

   threevector construct_direction_vector(double phi,double theta)
   {
      if (theta < -PI/2 || theta > PI/2)
      {
//            cout << "Danger in mathfunc::construct_direction_vector!" << endl;
//            cout << "theta = " << theta*180/PI << " degs lies outside"
//                 << "[-90,90]" << endl;
      }
      double cos_theta=cos(theta);
      return cos_theta*cos(phi)*x_hat+
         cos_theta*sin(phi)*y_hat+sin(theta)*z_hat;
   }

// ---------------------------------------------------------------------
// Method generate_orthogonal_basis takes in unit vector n_hat.  It
// generates two more unit vectors p_hat and q_hat which are
// guaranteed to form an orthogonal basis in addition to n_hat.

// Note 1: n_hat [p_hat] {q_hat} is a rotated version of x_hat [y_hat]
// {z_hat}

// Note 2: In this method, the conventionally defined polar angle =
// PI/2-theta and not theta itself !  Theta ranges over the interval
// [-PI/2, PI/2].

   void generate_orthogonal_basis(const threevector& n_hat,threevector& p_hat,
                                  threevector& q_hat)
   {
      double phi,theta;
      decompose_direction_vector(n_hat,phi,theta);
      double cos_theta=cos(theta);
      double sin_theta=sin(theta);
      double cos_phi=cos(phi);
      double sin_phi=sin(phi);

      threevector m_hat(cos_theta*cos_phi,cos_theta*sin_phi,sin_theta);
      p_hat=threevector(-sin_phi,cos_phi,0);
      q_hat=threevector(-sin_theta*cos_phi,-sin_theta*sin_phi,cos_theta);

//         cout << "m_hat = " << m_hat << endl;
//         cout << "p_hat = " << p_hat << endl;
//         cout << "q_hat = " << q_hat << endl;
         
//         cout << "m_hat.p_hat = " << m_hat.dot(p_hat) << endl;
//         cout << "p_hat.q_hat = " << p_hat.dot(q_hat) << endl;
//         cout << "m_hat.m_hat = " << m_hat.dot(m_hat) << endl;
//         cout << "p_hat.p_hat = " << p_hat.dot(p_hat) << endl;
//         cout << "q_hat.q_hat = " << q_hat.dot(q_hat) << endl;
   }

// ==========================================================================
// Orthogonal matrix and quaternion methods:
// ==========================================================================

// Method decompose_orthogonal_matrix takes in a 3x3 orthogonal matrix
// R which may represent an improper rotation (i.e. det R = -1 ).  It
// returns total spherical angles theta and phi which specify the
// rotation axis as n_hat = sin(theta) cos(phi) x_hat + sin(theta)
// sin(phi) y_hat + cos(theta) z_hat.  It also returns the total
// rotation about n_hat within output angle chi.  The inverse to this
// method is provided within construct_orthogonal_matrix() below.

   void decompose_orthogonal_matrix(
      const genmatrix& R,double& chi,threevector& nhat,double TINY)
   {
      double theta,phi;
      decompose_orthogonal_matrix(R,theta,phi,chi,TINY);
      nhat.put(0,sin(theta)*cos(phi));
      nhat.put(1,sin(theta)*sin(phi));
      nhat.put(2,cos(theta));
   }

   void decompose_orthogonal_matrix(
      const genmatrix& R,double& theta,double& phi,double& chi,double TINY)
   {
//         cout << "inside mathfunc::decompose_orthogonal_matrix()" << endl;
      double detR=R.determinant();
//         cout << "detR = " << detR << endl;

// In January 2011, we learned the hard and painful way that we must
// be very careful to ensure arc-cosine arguments never have
// magnitudes that exceed 1!

//         cout.precision(12);
      double argument=0.5*(R.trace()-detR);
//         cout << "argument = " << argument << endl;

      if (!nearly_equal(fabs(argument),1,TINY))
      {         
         chi=acos(argument);
      }
      else
      {
         if (nearly_equal(argument,1,TINY))
         {
            chi=0;
         }
         else if (nearly_equal(argument,-1,TINY))
         {
            chi=PI;
         }
      }
//         cout << "chi = " << chi*180/PI << endl;

      genmatrix I(3,3),D(3,3);
      I.identity();
      D=R-detR*I;

//         cout << "D = " << D << endl;
      threevector n_hat;
//         bool nontrivial_soln_found=
      D.homogeneous_soln(n_hat);

//         cout << "nontrivial_soln_found = " 
//              << nontrivial_soln_found << endl;
//         cout << "n_hat = " << n_hat << endl;
//         cout << "R*n_hat = " << R*n_hat << endl;

      if (!nearly_equal(fabs(n_hat.get(2)),1,TINY))
      {
         theta=acos(n_hat.get(2));
         phi=atan2(n_hat.get(1),n_hat.get(0));
      }
      else
      {
         if (nearly_equal(n_hat.get(2),1,TINY))
         {
            theta=0;
         }
         else if (nearly_equal(n_hat.get(2),-1,TINY))
         {
            theta=PI;
         }
         phi=0;
      }
            
//            cout << "theta = " << theta*180/PI << endl;
//            cout << "phi = " << phi*180/PI << endl;

// Rotation axis n_hat is known only up to a sign.  So we must
// explicitly check whether (theta,phi) or (PI-theta,phi+PI) yields a
// reconstructed orthogonal matrix which matches the input R:

      genmatrix R1(3,3),R2(3,3);
      construct_orthogonal_matrix(detR,theta,phi,chi,R1);
      construct_orthogonal_matrix(detR,PI-theta,phi+PI,chi,R2);
      double diff1,diff2;
      diff1=diff2=0;
      for (int i=0; i<3; i++)
      {
         for (int j=0; j<3; j++)
         {
            diff1 += fabs(R1.get(i,j)-R.get(i,j));
            diff2 += fabs(R2.get(i,j)-R.get(i,j));
         }
      }

//            cout << "diff1 = " << diff1 << " diff2 = " << diff2 << endl;
//            cout << "R1 = " << R1 << endl;
//            cout << "R2 = " << R2 << endl;
//            cout << "R = " << R << endl;
      if (diff2 < diff1)
      {
         theta=PI-theta;
         phi=phi+PI;
      }

   }

// ---------------------------------------------------------------------   
// Method construct_orthogonal_matrix takes in the determinant (+/- 1)
// of some 3x3 orthogonal transformation.  It also takes in a rotation
// axis specified in terms of spherical angles theta and phi as n_hat
// = sin(theta) cos(phi) x_hat + sin(theta) sin(phi) y_hat +
// cos(theta) z_hat.  Finally, it takes in the total rotation angle
// chi about the rotation axis.  This method returns the 3x3
// orthogonal matrix corresponding to these 3 continuous and 1
// discrete pieces of input information.  

   void construct_orthogonal_matrix(
      double det,double theta,double phi,double chi,genmatrix& R)
   {
      threevector n_hat(
         sin(theta)*cos(phi),sin(theta)*sin(phi),cos(theta));
      construct_orthogonal_matrix(det,n_hat,chi,R);
   }

   void construct_orthogonal_matrix(
      double det,const threevector& n_hat,double chi,genmatrix& R)
   {
//         cout << "inside mathfunc::construct_ortho_matrix()" << endl;
//         cout << "det = " << det << endl;
//         cout << "theta = " << theta*180/PI << endl;
//         cout << "phi = " << phi*180/PI << endl;
//         cout << "chi = " << chi*180/PI << endl;

      genmatrix nhat_nhat(n_hat.outerproduct(n_hat));

      threevector p_hat,q_hat;
      generate_orthogonal_basis(n_hat,p_hat,q_hat);

//         cout << "n_hat = " << n_hat << endl;
//         cout << "p_hat = " << p_hat << endl;
//         cout << "q_hat = " << q_hat << endl;
         
      genmatrix phat_phat(p_hat.outerproduct(p_hat));
      genmatrix qhat_qhat(q_hat.outerproduct(q_hat));
      genmatrix qhat_phat(q_hat.outerproduct(p_hat));
      genmatrix phat_qhat(p_hat.outerproduct(q_hat));

//         cout << "qhat_phat = " << qhat_phat << endl;
//         cout << "phat_qhat = " << phat_qhat << endl;
         
      genmatrix R2(3,3);
      R2=cos(chi)*(phat_phat+qhat_qhat)
         +sin(chi)*(qhat_phat-phat_qhat);
      R=det*nhat_nhat+R2;
//         cout << "R_reconstructed = " << R << endl;
   }

// ---------------------------------------------------------------------
// Method az_el_roll_corresponding_to_quaternion
// [quaternion_corresponding_to_az_el_roll] takes in [puts out] angles
// measured in radians and not degrees.

   void az_el_roll_corresponding_to_quaternion(
      const fourvector& q,double& az,double& el,double& roll)
   {
//         cout << "inside mathfunc::az_el_roll_corresponding_to_quat()"
//              << endl;
      double q0=q.get(0);
      double q1=q.get(1);
      double q2=q.get(2);
      double q3=q.get(3);

      az=atan2( 2*(q0*q3+q1*q2) , 1-2*(q2*q2+q3*q3) );
//         az=atan(2*(q0*q3+q1*q2)/(1-2*(q2*q2+q3*q3)));
      el=-asin(2*(q0*q2-q3*q1));
      roll=atan2( 2*(q0*q1+q2*q3) , 1-2*(q1*q1+q2*q2) );
         
//         roll=atan(2*(q0*q1+q2*q3)/(1-2*(q1*q1+q2*q2)));
   }

   fourvector quaternion_corresponding_to_az_el_roll(
      double az,double el,double roll)
   {
//         cout << "inside mathfunc::quat_corresponding_to_az_el_roll()" 
//              << endl;
      double cos_phi=cos(0.5*roll);
      double sin_phi=sin(0.5*roll);
      double cos_theta=cos(-0.5*el);
      double sin_theta=sin(-0.5*el);
      double cos_psi=cos(0.5*az);
      double sin_psi=sin(0.5*az);
         
      fourvector q(cos_phi*cos_theta*cos_psi+sin_phi*sin_theta*sin_psi,
                   sin_phi*cos_theta*cos_psi-cos_phi*sin_theta*sin_psi,
                   cos_phi*sin_theta*cos_psi+sin_phi*cos_theta*sin_psi,
                   cos_phi*cos_theta*sin_psi-sin_phi*sin_theta*cos_psi);
      return q;
   }
   
// ==========================================================================
// Probability distribution methods
// ==========================================================================

// Method find_local_peaks() takes in STL data vector Z.  It searches
// for the highest peak value within Z.  After subtracting out all
// data values located within +/- peak_width of Z_peak, this method
// searches for the next largest peak.  It continues this peak finding
// process until either the number of remaining entries wtihin the STL
// data vector becomes too small or the number of peaks found exceeds
// input parameter max_n_peaks.  

   vector<double> find_local_peaks(
      const vector<double>& Z,double peak_width,unsigned int max_n_peaks)
   {
      vector<double> Zcurr;
      for (unsigned int i=0; i<Z.size(); i++)
      {
         Zcurr.push_back(Z[i]);
      }

      vector<double> peak_Zs;
      while (Zcurr.size() > 10 && peak_Zs.size() < max_n_peaks)
      {
         double curr_Zpeak=find_local_peak(Zcurr);
         peak_Zs.push_back(curr_Zpeak);

//         cout << "Peak #" << peak_Zs.size()
//              << " Z_peak = " << curr_Zpeak << endl;
      
         vector<double> Zreduced;
         for (unsigned int i=0; i<Zcurr.size(); i++)
         {
            double curr_Z=Zcurr[i];
            if (fabs(curr_Z-curr_Zpeak) > peak_width)
            {
               Zreduced.push_back(curr_Z);
            }
         } // loop over index i labeling Zcurr values
//         cout << "Zreduced.size()= " << Zreduced.size() << endl;

         Zcurr.clear();
         for (unsigned int i=0; i<Zreduced.size(); i++)
         {
            Zcurr.push_back(Zreduced[i]);
         }

      } // while loop

      return peak_Zs;
   }
   
// ---------------------------------------------------------------------
// Method find_local_peak() takes in STL data vector Z.  It returns
// the peak Z value within the STL vector.

   double find_local_peak(const vector<double>& Z)
   {
      int n_output_bins=100;
      n_output_bins=basic_math::min(n_output_bins,1000);
      
      prob_distribution prob_Z(Z,n_output_bins);
//      prob_Z.writeprobdists(false);

      int n_peak_bin=0;
//      double p_peak=prob_Z.peak_density_value(n_peak_bin);
      double Z_peak=prob_Z.get_x(n_peak_bin);
//      cout << "Z_peak = " << Z_peak 
//           << " p_peak = " << p_peak << endl;
      
      return Z_peak;
   }

// ---------------------------------------------------------------------
// Method find_x_corresponding_to_pcum() returns the value of x for which
// pcum[x] = cumprob.

   double find_x_corresponding_to_pcum(
      const vector<double>& x, const vector<double>& pcum, double cumprob)
   {
      if (cumprob >= 0 && cumprob < pcum[0])
      {
         return x.front();
      }
      else if (nearly_equal(cumprob,1))
      {
         return x.back();
      }
      else if (cumprob < 0 || cumprob > 1)
      {
         cout << "Trouble in mathfunc::find_x_corresponding_to_pcum()"
              << endl;
         cout << "cumulative probability = " << cumprob << " lies outside"
              << " allowed interval [0,1]" << endl;
         return POSITIVEINFINITY;
      }
      else
      {
         int j=0;
	 double dx=x[1]-x[0];
         double xsum=0;
         for (unsigned int i=0; i<pcum.size()-1; i++)
         {
            if (pcum[i+1] < cumprob)
            {
               continue;
            }
            else if (pcum[i] > cumprob)
            {
               break;
            }
            else // (pcum[i] <= cumprob && cumprob <= pcum[i+1])
            {
               if (nearly_equal(pcum[i],pcum[i+1]))
               {
                  xsum += x[i]+0.5*dx;
               }
               else
               {
                  xsum += x[i]+(cumprob-pcum[i])/(pcum[i+1]-pcum[i])*dx;
               }
               j++;
            }
         } // loop over index i 
         if (j==0)
         {
            cout << "Trouble in mathfunc::find_x_corresponding_to_pcum"
                 << endl;
            cout << "cumprob = " << cumprob << " pcum[0] = " << pcum[0]
                 << endl;
            cout << "x is ill-defined!" << endl;
            return POSITIVEINFINITY;
         }
         else
         {
            double X=xsum/double(j);
//         cout << "pcum = " << cumprob << " n = " << n << " j = " << j 
//              << " x[j] = " << x[j] << " X = " << X << endl;
            return X;
         }
      }
   }
  
// ---------------------------------------------------------------------
// Method contrast_normalize_histogram() takes in an array of H
// floats.  It first computes the mean and standard deviation of the H
// floats.  It next subtracts the mean from each histogram component
// and divides by the standard deviation.  The output histogram values
// have mean 0 and unit standard deviation.

void contrast_normalize_histogram(unsigned int H,float* histogram)
{
//   cout << "inside mathfunc::contrast_normalize_histogram()" << endl;
//   cout << "Number of input histogram bins = " << H << endl;
   
   float mean=0;
   float mean_sqr=0;
   for (unsigned int h=0; h<H; h++)
   {
      mean += histogram[h];
      mean_sqr += sqr(histogram[h]);
   }
   mean /= H;
   mean_sqr /= H;
   float variance = mean_sqr - sqr(mean);
   float sigma=sqrt(variance);
//   cout << "mu = " << mean << " sigma = " << sigma << endl;

   for (unsigned int h=0; h<H; h++)
   {
      float numer=histogram[h]-mean;
      histogram[h] = numer/sigma;
   }
}

// ==========================================================================
// Random integer sequence methods:
// ==========================================================================

// Method getRandomInteger() returns an integer uniformly distributed
// among 0, 1, 2, ..., N-1.

   int getRandomInteger(int N)
   {
      return basic_math::mytruncate(N*nrfunc::ran1());
   }

// ---------------------------------------------------------------------
// Method random_sequence() randomly reorders an integer
// sequence [0,1,...,nsize-1].  It returns the randomized result in an
// STL vector of integers.

   vector<int> random_sequence(int nsize)
   {
      return random_sequence(nsize,nsize);
   }

// ---------------------------------------------------------------------
// This overloaded version of random_sequence() returns the first
// sequence_length entries within a randomized sequence of
// [0,1,...nsize-1].

   vector<int> random_sequence(int nsize,int sequence_length)
   {
//      cout << "inside mathfunc::random_sequence()" << endl;
      return random_sequence(0,nsize-1,sequence_length);
   }
   
// ---------------------------------------------------------------------
// This version of random_sequence() returns the first sequence_length
// entries within a randomized sequence of [istart , istart+1 , ... , istop].

   vector<int> random_sequence(int istart,int istop,int sequence_length)
   {
//      cout << "inside mathfunc::random_sequence()" << endl;

      int nsize=istop-istart+1;
      vector<int> a,a_random;
      a.reserve(nsize);

      for (int i=0; i<nsize; i++) 
      {
         a.push_back(istart+i);
      }

// Durstenfeld's random number shuffling algorithm: Move "struck"
// numbers to end of list by swapping them with last unstruck number at
// each interation.  Time complexity of this algorithm is O(N) rather
// than O(N**2).  See http://en.wikipedia.org/wiki/Fisher-Yates_shuffle.

      for (int i=nsize-1; i>=nsize-sequence_length; i--)
      {
         int j=basic_math::round(i*nrfunc::ran1()); 
         a_random.push_back(a[j]);
         templatefunc::swap(a[j],a[i]);
      }

      return a_random;
   }

// ---------------------------------------------------------------------
// Method get_next_random_int_sequence() returns an STL vector<int>
// containing n_integers where the integer values range between 0 and
// large_prime_number.  The returned integer sequence is guaranteed to
// contain no repeated values.

   int random_int_counter=0;
   vector<int> long_sequence_of_random_integers;

   vector<int> get_next_random_int_sequence(int n_integers)
   {
      const int large_prime_number=10007;

      if (long_sequence_of_random_integers.size()==0)
      {
         long_sequence_of_random_integers=
            mathfunc::random_sequence(large_prime_number);
      }

      vector<int> a_random;
      for (int i=0; i<n_integers; i++)
      {
         int j=modulo(random_int_counter+i,large_prime_number);
         a_random.push_back(long_sequence_of_random_integers[j]);
      }
      random_int_counter += n_integers;
      return a_random;
   }



// ==========================================================================
// Set methods
// ==========================================================================

// C-style method intersect_sorted_integers() takes in 2
// sorted-integer sets A & B of lengths l_a and l_b.  It returns
// within *A_and_B, *only_A and *only_B the decomposition of the 2
// input integer sets.  It also returns the sizes of these decomposed
// integer sets.

// Adapted from figure 1 in "Fast sorted-set intersection using SIMD
// instructions" by Schlegel, Willhalm and Lehner.

// C++ note: STL container "set" provides this method's functionality.

   void intersect_sorted_integers(
      int *A, int *B, int l_a, int l_b, 
      int* A_and_B, int* l_a_and_b_ptr,
      int* only_A, int* l_only_a_ptr,
      int* only_B, int* l_only_b_ptr)
   {
//      cout << "inside mathfunc::intersect_sorted_integers()" << endl;
      
      *l_only_a_ptr = 0;
      *l_only_b_ptr = 0;
      *l_a_and_b_ptr = 0;

      int i_a = 0, i_b = 0;
      while (i_a < l_a && i_b < l_b) 
      {
//         cout << "i_a = " << i_a << " i_b = " << i_b 
//              << " A[i_a] = " << A[i_a] << " B[i_b] = " << B[i_b]
//              << endl;

         if (A[i_a] == B[i_b]) 
         {
            A_and_B[(*l_a_and_b_ptr)++]=A[i_a];
            i_a++;
            i_b++;
         }
         else if (A[i_a] > B[i_b])
         {
            only_B[(*l_only_b_ptr)++]=B[i_b];
            i_b++;
         }
         else
         {
            only_A[(*l_only_a_ptr)++]=A[i_a];
            i_a++;
         }
//         cout << "l_a_and_b = " << *l_a_and_b_ptr
//              << " l_only_a = " << *l_only_a_ptr
//              << " l_only_b = " << *l_only_b_ptr
//              << endl;

      } // while loop

      if (l_b < l_a)
      {
         while (i_a < l_a)
         {
            only_A[(*l_only_a_ptr)++]=A[i_a];
            i_a++;
         }
      }
      else if (l_a < l_b)
      {
         while (i_b < l_b)
         {
            only_B[(*l_only_b_ptr)++]=B[i_b];
            i_b++;
         }
      }
   }

// ---------------------------------------------------------------------
// C-style method difference_between_sorted_integers() takes in 2
// sorted-integer sets A & B of lengths l_a and l_b.  It returns
// within *only_A those elements which are in A but not in B.  It also
// returns the size of *only_A.

// C++ note: STL container "set" provides this method's functionality.

   int difference_between_sorted_integers(
      int *A, int *B, int l_a, int l_b, int* only_A)
   {
//      cout << "inside mathfunc::difference_between_sorted_integers()" << endl;
      int l_only_a=0;

      int i_a = 0, i_b = 0;
      while (i_a < l_a && i_b < l_b) 
      {
//         cout << "i_a = " << i_a << " i_b = " << i_b 
//              << " A[i_a] = " << A[i_a] << " B[i_b] = " << B[i_b]
//              << endl;

         if (A[i_a] == B[i_b]) 
         {
            i_a++;
            i_b++;
         }
         else if (A[i_a] > B[i_b])
         {
            i_b++;
         }
         if (A[i_a] < B[i_b])
         {
            only_A[l_only_a++]=A[i_a];
            i_a++;
         }
      } // while loop

      if (l_b < l_a)
      {
         while (i_a < l_a)
         {
            only_A[l_only_a++]=A[i_a];
            i_a++;
         }
      }

      return l_only_a;
   }

// ==========================================================================
// Sparse matrix methods
// ==========================================================================

// Method import_genvector_from_dense_text_format() returns a
// dynamically-instantiated genvector read in from a specified text
// file.

   genvector* import_genvector_from_dense_text_format(string input_filename)
   {
      genmatrix* M_ptr=import_from_dense_text_format(input_filename);
      genvector* V_ptr=new genvector(*M_ptr);
      delete M_ptr;
      return V_ptr;
   }
   
// ---------------------------------------------------------------------
// Method import_from_dense_text_format() imports a genmatrix from an
// input file following the formats used by the SVDLIBC library.  See
// http://tedlab.mit.edu/~dr/SVDLIBC/

   genmatrix* import_from_dense_text_format(string input_filename)
   {
      string banner="Importing genmatrix from dense text format input file "+
         input_filename;
      outputfunc::write_banner(banner);
      
      ifstream instream;
      filefunc::openfile(input_filename,instream);

      int n_rows,n_columns;
      instream >> n_rows >> n_columns;
      cout << "n_rows = " << n_rows << " n_columns = " << n_columns << endl;
 
      genmatrix* genmatrix_ptr=new genmatrix(n_rows,n_columns);

      double curr_value;
      for (int r=0; r<n_rows; r++)
      {
         outputfunc::update_progress_fraction(r, 1000, n_rows);
         for (int c=0; c<n_columns; c++)
         {
            instream >> curr_value;
            genmatrix_ptr->put(r,c,curr_value);
//         cout << curr_value << " " << flush;
         }
//      cout << endl;
//      outputfunc::enter_continue_char();
      }

      filefunc::closefile(input_filename,instream);
      return genmatrix_ptr;
   }

// ---------------------------------------------------------------------   
// Method import_from_diagonal_text_format()

   genmatrix* import_from_diagonal_text_format(
      int mdim,string input_filename)
   {
      string banner="Importing genmatrix from diagonal text format";
      outputfunc::write_banner(banner);

      ifstream instream;
      filefunc::openfile(input_filename,instream);

      int n_rows;
      instream >> n_rows;
      cout << "n_rows = " << n_rows << endl;
   
      genmatrix* genmatrix_ptr=new genmatrix(mdim,mdim);
      genmatrix_ptr->clear_values();

      double curr_value;
      for (int r=0; r<n_rows; r++)
      {
         instream >> curr_value;
         genmatrix_ptr->put(r,r,curr_value);
      }

      filefunc::closefile(input_filename,instream);

      return genmatrix_ptr;
   }

// ---------------------------------------------------------------------
// Method export_to_sparse_text_format()

   void export_to_sparse_text_format(
      gmm::row_matrix< gmm::wsvector<float> >* sparse_matrix_ptr,
      int n_nonzero_values,string output_filename)
   {
      string banner="Exporting sparse GMM matrix to sparse text format";
      outputfunc::write_banner(banner);

      ofstream outstream;
      filefunc::openfile(output_filename,outstream);

      int n_rows=sparse_matrix_ptr->nrows();
      int n_columns=sparse_matrix_ptr->ncols();
      outstream << n_rows << " " << n_columns << " " << n_nonzero_values 
                << endl << endl;

      double TINY=1E-9;
      for (int n=0; n<n_columns; n++)
      {
         int n_nonzero_entries_in_curr_column=0;
         for (int m=0; m<n_rows; m++)
         {
            if (fabs((*sparse_matrix_ptr)(m,n)) > TINY) 
               n_nonzero_entries_in_curr_column++;
         }
         outstream << n_nonzero_entries_in_curr_column << endl;
         for (int m=0; m<n_rows; m++)
         {
            if (fabs((*sparse_matrix_ptr)(m,n)) <= TINY) continue;
            outstream << m << " " << (*sparse_matrix_ptr)(m,n) << endl;
         }
      } // loop over index n labeling genmatrix columns

      filefunc::closefile(output_filename,outstream);
   }

// ---------------------------------------------------------------------
// Method export_to_sparse_binary_format()

   void export_to_sparse_binary_format(
      gmm::row_matrix< gmm::wsvector<float> >* sparse_matrix_ptr,
      int n_nonzero_values,string output_filename)
   {
      string banner="Exporting sparse GMM matrix to sparse binary format";
      outputfunc::write_banner(banner);

      string sparse_text_filename="./sparse_text.matrix";
      mathfunc::export_to_sparse_text_format(
         sparse_matrix_ptr,n_nonzero_values,sparse_text_filename);
      string unix_cmd="svd -r st -w sb -c "+sparse_text_filename+" "
         +output_filename;
      sysfunc::unix_command(unix_cmd);
      filefunc::deletefile(sparse_text_filename);
   }

// ---------------------------------------------------------------------
   void sparse_SVD_approximation(
      gmm::row_matrix< gmm::wsvector<float> >* sparse_matrix_ptr,
      int k_dims,int n_nonzero_values,string output_subdir)
   {
      string sparse_binary_filename="./sb.matrix";
      mathfunc::export_to_sparse_binary_format(
         sparse_matrix_ptr,n_nonzero_values,sparse_binary_filename);

      string unix_cmd="svd -r sb -o svd -d "+
         stringfunc::number_to_string(k_dims)+" sb.matrix";
//   string unix_cmd="svd -r sb -o svd sb.matrix";
      cout << "unix_cmd = " << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);
   
      unix_cmd="mv svd-Ut "+output_subdir+"svd-Ut.dat";
      sysfunc::unix_command(unix_cmd);      
      unix_cmd="mv svd-Vt "+output_subdir+"svd-Vt.dat";
      sysfunc::unix_command(unix_cmd);      
      unix_cmd="mv svd-S "+output_subdir+"svd-S.dat";
      sysfunc::unix_command(unix_cmd);      

      filefunc::deletefile(sparse_binary_filename);
   }
 
// ==========================================================================
// Spline methods:
// ==========================================================================

   void spline_interp(
      vector<double>& t,vector<threevector>& XYZ,
      vector<double>& t_reg,vector<threevector>& interp_XYZ)
   {
      vector<double> X,Y,Z;
      for (unsigned int i=0; i<XYZ.size(); i++)
      {
         X.push_back(XYZ[i].get(0));
         Y.push_back(XYZ[i].get(1));
         Z.push_back(XYZ[i].get(2));
      }
      vector<double> interp_X,interp_Y,interp_Z;

      numrec::init_spline_2nd_derivs(t,X);
      for (unsigned int n=0; n<t_reg.size(); n++)
      {
         interp_X.push_back(numrec::spline_interp(t,X,t_reg[n]));
      }

      numrec::init_spline_2nd_derivs(t,Y);
      for (unsigned int n=0; n<t_reg.size(); n++)
      {
         interp_Y.push_back(numrec::spline_interp(t,Y,t_reg[n]));
      }

      numrec::init_spline_2nd_derivs(t,Z);
      for (unsigned int n=0; n<t_reg.size(); n++)
      {
         interp_Z.push_back(numrec::spline_interp(t,Z,t_reg[n]));
      }

      for (unsigned int n=0; n<t_reg.size(); n++)
      {
         interp_XYZ.push_back(threevector(interp_X[n],interp_Y[n],
                                          interp_Z[n]));
      }
   }

   void spline_interp(
      vector<double>& t,vector<fourvector>& WXYZ,
      vector<double>& t_reg,vector<fourvector>& interp_WXYZ)
   {
      vector<double> W,X,Y,Z;
      for (unsigned int i=0; i<WXYZ.size(); i++)
      {
         W.push_back(WXYZ[i].get(0));
         X.push_back(WXYZ[i].get(1));
         Y.push_back(WXYZ[i].get(2));
         Z.push_back(WXYZ[i].get(3));
      }
      vector<double> interp_W,interp_X,interp_Y,interp_Z;

      numrec::init_spline_2nd_derivs(t,W);
      for (unsigned int n=0; n<t_reg.size(); n++)
      {
         interp_W.push_back(numrec::spline_interp(t,W,t_reg[n]));
      }

      numrec::init_spline_2nd_derivs(t,X);
      for (unsigned int n=0; n<t_reg.size(); n++)
      {
         interp_X.push_back(numrec::spline_interp(t,X,t_reg[n]));
      }

      numrec::init_spline_2nd_derivs(t,Y);
      for (unsigned int n=0; n<t_reg.size(); n++)
      {
         interp_Y.push_back(numrec::spline_interp(t,Y,t_reg[n]));
      }

      numrec::init_spline_2nd_derivs(t,Z);
      for (unsigned int n=0; n<t_reg.size(); n++)
      {
         interp_Z.push_back(numrec::spline_interp(t,Z,t_reg[n]));
      }

      for (unsigned int n=0; n<t_reg.size(); n++)
      {
         interp_WXYZ.push_back(
            fourvector(interp_W[n],interp_X[n],interp_Y[n],interp_Z[n]));
      }
   }

// ---------------------------------------------------------------------
// Method smoothed_ramp takes in starting [stopping] times t0 [t4] and
// endpoints y0 [y4].  It fits a quadratic start, quadratic end and
// linear middle to y=y(t).  The 0th and 1st derivatives of the
// quadratic and linear segments match at join times t1 and t=t3.  The
// value y1=y(t1) equals input fraction of the total spread y4-y0.
// (Note that y2=y(0.5*(t0+t4)) is neither an input nor output of this
// method.)

// We implemented this smoothed_ramp function for
// acceleration/glide/deceleration of the OpenGL camera as it moves
// from one position in world-space to another along some calculated
// animation path.

   double smoothed_ramp(double frac,double t0,double t4,double y0,double y4,
                        double t)
   {
      double y1=y0+frac*(y4-y0);
      double y3=y4-frac*(y4-y0);
      double K=4*(y1-y0)/(y3-y1)+1;
      double Delta=0.5*(y4-y0)*(K-1)/K;
      double t1=t0+Delta;
      double t3=t4-Delta;

//         cout << "Delta = " << Delta << endl;

      double a=(y1-y0)/sqr(t1-t0);
         
      if (t0 <= t && t <= t1)
      {
         return a*(sqr(t)+sqr(t0))-2*a*t0*t+y0;
      }
      else if (t1 <= t && t <= t3)
      {
         return y1+(y3-y1)/(t3-t1)*(t-t1);
      }
      else if (t3 <= t && t <= t4)
      {
         return -a*(sqr(t)+sqr(t4))+2*a*t4*t+y4;
      }
      else
      {
         cout << "Error in mathfunc::smoothed_ramp()" << endl;
         cout << "t = " << t << " does not lie between t0 and t4"
              << endl;
         return NEGATIVEINFINITY;
      }
         
   }

// ==========================================================================
// Median value and filter methods
// ==========================================================================

// Method median_value() takes in array zarray.  If the number of
// array elements is odd, the median is returned.  Otherwise, the
// average of the array element immediately below and above the
// "middle" value is returned.

   double maximal_value(const vector<double>& A)
   {
//         double Amax=NEGATIVEINFINITY;
      double Amax=-1E25;
      for (unsigned int n=0; n<A.size(); n++)
      {
         Amax=basic_math::max(Amax,A[n]);
      }
      return Amax;
   }

   double minimal_value(const vector<double>& A)
   {
      double Amin=1E25;
      for (unsigned int n=0; n<A.size(); n++)
      {
         Amin=basic_math::min(Amin,A[n]);
      }
      return Amin;
   }

// ---------------------------------------------------------------------
// Method median_value() is a brute-force subroutine which works with
// just 3 input integers.

   int median_value(int a,int b,int c)
   {
//      cout << "inside mathfunc::median_value()" << endl;
      
      if (a==b) return a;
      if (b==c) return b;
      if (c==a) return c;

      if (a < b && b < c) return b;
      if (a < c && c < b) return c;

      if (b < c && c < a) return c;
      if (b < a && a < c) return a;

      if (c < a && a < b) return a;
      if (c < b && b < a) return b;

      cout << "Error in mathfunc::median_value()!" << endl;
      cout << "Should not have reached this point..." << endl;
      exit(-1);
   }
	
// ---------------------------------------------------------------------
   double median_value(const vector<double>& A)
   {
      unsigned int nbins=A.size();
      if (nbins==0) return NEGATIVEINFINITY;

      vector<double> Acopy;
      for (unsigned int i=0; i<nbins; i++)
      {
         Acopy.push_back(A[i]);
      }
      std::sort(Acopy.begin(),Acopy.end());

      double median=0;
      if (is_odd(nbins))
      {
         median=Acopy[nbins/2];
      }
      else
      {
         median=0.5*(Acopy[nbins/2-1]+Acopy[nbins/2]);
      }
      return median;
   }
   
// ---------------------------------------------------------------------
// Method lo_hi_values() sorts the contents for input STL vector A.
// It then computes and returns the 25% and 75% values in the sorted
// vector.  

   void lo_hi_values(
      const vector<double>& A,double& level_25,double& level_75)
   {
      unsigned int nbins=A.size();
      vector<double> Acopy;
      for (unsigned int i=0; i<nbins; i++)
      {
         Acopy.push_back(A[i]);
      }
      std::sort(Acopy.begin(),Acopy.end());

      level_25=Acopy[basic_math::round(0.25*nbins)];
      level_75=Acopy[basic_math::round(0.75*nbins)];
   }
   
// ---------------------------------------------------------------------
// Method median_value_and_quartile_width() sorts the contents for
// input STL vector A.  It then computes the 25%, 50% and 75% values
// in the sorted vector.  The quartile width is set equal to the
// average of the (75-50) and (50-25) percentiles.

   void median_value_and_percentile_width(
      const vector<double>& A,double frac_from_median, 
      double& median,double& percentile_width)
   {
//  	 cout << "mathfunc::inside median_value_and_percentile_width()" << endl;
      unsigned int nbins=A.size();
//	 cout << "nbins = " << nbins << endl;

      vector<double> Acopy;
      for (unsigned int i=0; i<nbins; i++)
      {
         Acopy.push_back(A[i]);
      }
      std::sort(Acopy.begin(),Acopy.end());

      if (is_odd(nbins))
      {
         median=Acopy[nbins/2];
      }
      else
      {
         median=0.5*(Acopy[nbins/2-1]+Acopy[nbins/2]);
      }

      double frac_hi=0.5+frac_from_median;
      double frac_lo=0.5-frac_from_median;
//	 int i_hi = basic_math::round(frac_hi*nbins);
//	 int i_lo = basic_math::round(frac_lo*nbins);
//	 cout << "i_hi = " << i_hi << " i_lo = " << i_lo << endl;
      double level_hi=Acopy[basic_math::round(frac_hi*nbins)];
      double level_lo=Acopy[basic_math::round(frac_lo*nbins)];

      percentile_width=0.5*(level_hi-level_lo);
   }

   void median_value_and_quartile_width(
      const vector<double>& A,double& median,double& quartile_width)
   {
      median_value_and_percentile_width(A,0.25,median,quartile_width);
   }

// ---------------------------------------------------------------------
   void median_value_and_quartile_width(
      const vector<threevector>& RGB,
      threevector& median,threevector& quartile_width)
   {
      unsigned int nbins=RGB.size();
      vector<double> R,G,B;
      for (unsigned int i=0; i<nbins; i++)
      {
         R.push_back(RGB[i].get(0));
         G.push_back(RGB[i].get(1));
         B.push_back(RGB[i].get(2));
      }
      std::sort(R.begin(),R.end());
      std::sort(G.begin(),G.end());
      std::sort(B.begin(),B.end());

      if (is_odd(nbins))
      {
         median=threevector(R[nbins/2],G[nbins/2],B[nbins/2]);
      }
      else
      {
         median=0.5*threevector(R[nbins/2-1]+R[nbins/2],
                                G[nbins/2-1]+G[nbins/2],
                                B[nbins/2-1]+B[nbins/2]);
      }

      double Rlevel_25=R[basic_math::round(0.25*nbins)];
      double Rlevel_75=R[basic_math::round(0.75*nbins)];
      double Glevel_25=G[basic_math::round(0.25*nbins)];
      double Glevel_75=G[basic_math::round(0.75*nbins)];
      double Blevel_25=B[basic_math::round(0.25*nbins)];
      double Blevel_75=B[basic_math::round(0.75*nbins)];

      quartile_width=0.5*threevector(
         Rlevel_75-Rlevel_25,
         Glevel_75-Glevel_25,
         Blevel_75-Blevel_25);
   }

   double median_value(int nbins,double zarray[])
   {
      double zcopy[nbins];
      for (int i=0; i<nbins; i++)
      {
         zcopy[i]=zarray[i];
      }
      Quicksort(zcopy,nbins);

      double median;
      if (is_odd(nbins))
      {
         median=zcopy[nbins/2];
      }
      else
      {
         median=0.5*(zcopy[nbins/2-1]+zcopy[nbins/2]);
      }
      return median;
   }

// ---------------------------------------------------------------------
// Method median_filter takes in an array ain along with a user
// specified median filter window size which should be odd valued.
// For bins i less than window_size/2 or greater than
// nbins-window_size/2, the median value amedian[i] is set equal to
// ain[i].  But for bins within the interval window_size/2 < i <
// nbins-window_size/2, amedian[i] is set equal to the median value of
// the ain array bins within the window centered around bin i.

   void median_filter(int nbins,int window_size,double ain[],double amedian[])
   {
      int odd_window_size;
      if (is_odd(window_size))
      {
         odd_window_size=window_size;
      }
      else
      {
         odd_window_size=window_size+1;
      }

      int w=(odd_window_size-1)/2;
      for (int i=0; i<w; i++)
      {
         amedian[i]=ain[i];
         amedian[nbins-1-i]=ain[nbins-1-i];
      }
   
      double b[window_size+1];
      for (int i=w; i<nbins-w; i++)
      {
         for (int j=-w; j<=w; j++)
         {
            b[j+w]=ain[i+j];
         }

// Recall index on C++ arrays starts at 0.  However, Numerical Recipes
// routines expect data array indices to start at 1.  So we must
// decrement the pointers to these arrays by one when SELECT is
// invoked:

         amedian[i]=numrec::select(w+1,2*w+1,b-1);
      }
   }

// ---------------------------------------------------------------------
// Method circular_median_filter() takes in STL data vector A along
// with a window size.  Vector A is assumed to circularly wrap onto itself.
// Sliding the window along the data vector, this method computes
// median values and returns them within a filtered STL vector.  

   vector<double> circular_median_filter(
      const vector<double>& A,int window_size)
   {
//         cout << "inside mathfunc::circular_median_filter()" << endl;
//         cout << "window_size = " << window_size << endl;
         
      unsigned int nbins=A.size();
//         cout << "nbins = " << nbins << endl;

      vector<double> Afiltered;
      Afiltered.reserve(nbins);
         
      if (is_even(window_size)) window_size++;
      vector<double> Awindow;
      Awindow.reserve(window_size);
      for (unsigned int i=0; i<nbins; i++)
      {
         Awindow.clear();

         for (int n=-window_size/2; n<=window_size/2; n++)
         {
            int j=i+n;
            j=modulo(j,nbins);
            Awindow.push_back(A[j]);
//               cout << "j = " << j << " Awindow = " << Awindow.back()
//                    << endl;
         }
         Afiltered.push_back(median_value(Awindow));
//            cout << "i = " << i << " Afiltered = " << Afiltered.back()
//                 << endl;
            
      } // loop over index i labeling entries in STL vector A

      return Afiltered;
   }

// ==========================================================================
// Statistical methods:
// ==========================================================================

// Method error_function returns the value of the error function
// which is related to the incomplete gamma function.  See section 6.2
// in Numerical Recipes.

   double error_function(double x)
   {
      double erfx;

      if (x > 5)
      {
         erfx=1;
      }
      else if (x < -5)
      {
         erfx=-1;
      }
      else
      {
         erfx=sgn(x)*numrec::gammp(0.5,sqr(x));
      }
      return erfx;
   }

// ---------------------------------------------------------------------
   double mean(const vector<double>& A)
   {
      double avg=0;
      for (unsigned int i=0; i<A.size(); i++)
      {
         avg += A[i];
      }
      return avg/A.size();
   }

   double std_dev(const vector<double>& A)
   {
      if (A.size()==1) return 0;

      if (A.size()==2)
      {
         double sigma=0.5*fabs(A[0]-A[1]);
         return sigma;
      }
         
      double avg=mean(A);
      double avg_sqr=0;
      for (unsigned int i=0; i<A.size(); i++)
      {
         avg_sqr += sqr(A[i]);
      }
      avg_sqr /= A.size();
         
      double variance=avg_sqr-sqr(avg);
      if (variance < 0)
      {
         cout << "Error in mathfunc::std_dev() !!!" << endl;
         cout << "variance = " << variance << endl;
         cout << "Resetting variance to zero" << endl;
         variance=0;            
      }
      return sqrt(variance);
   }

   void mean_and_std_dev(const vector<double>& A,double& mean,double& std_dev)
   {
      unsigned int Asize=A.size();
      if (Asize==1) 
      {
         mean=A[0];
         std_dev=0;
         return;
      }
      else if (Asize==2)
      {
         mean=0.5*(A[0]+A[1]);
         std_dev=0.5*fabs(A[0]-A[1]);
      }
      else
      {
         mean=0;
         double avg_sqr=0;
         for (unsigned int i=0; i<Asize; i++)
         {
            mean += A[i];
            avg_sqr += A[i]*A[i];
         }
         mean /= Asize;
         avg_sqr /= Asize;
            
         std_dev=sqrt(avg_sqr-sqr(mean));
      }
   }

// ---------------------------------------------------------------------
   double variance(double A[], const int Nsize)
   {
      if (Nsize <= 0)
      {
         cout << "Error inside mathfunc::variance()!" << endl;
         cout << "Nsize = " << Nsize << endl;
      }
      double avg=templatefunc::average(A,Nsize);
      double var=0;
      for (int i=0; i<Nsize; i++)
      {
         var += sqr(A[i])-sqr(avg);
      }
      var /= double(Nsize);
      if (var < 0) var=0;
      return var;
   }

// This overloaded version of method variance returns a squared
// standard deviation weighted by the values in input array w:

   double variance(double A[],double w[],const int Nsize)
   {
      const double TINY=1E-10;

// Renormalize weights in array w so that they sum to unity:

      double wsum=0;
      for (int i=0; i<Nsize; i++)
      {
         wsum += w[i];
      }

      double p[Nsize];
      for (int i=0; i<Nsize; i++)
      {
         p[i]=w[i]/wsum;
      }
      double avg=templatefunc::average(A,p,Nsize);
   
      double numer=0;
      for (int i=0; i<Nsize; i++)
      {
         numer += p[i]*sqr(A[i]);
      }
      numer -= sqr(avg);

// Although numer should always be positive semi-definite, numerical
// roundoff error can cause numer to go negative.  In this case, we
// set numer equal to 0:

      if (fabs(numer) < TINY)
      {
         numer=0;
      }
      else if (numer < 0 && fabs(numer) > TINY)
      {
         cout << "Error inside mathfunc::variance(double[],double[],int)"
              << endl;
         cout << "numer = " << numer << " < 0 !!!" << endl;
      }
      double weighted_variance=double(Nsize)/double(Nsize-1)*numer;
      return weighted_variance;
   }

// ---------------------------------------------------------------------
// Methods weight_mean and weighted_std_dev take in STL vectors X and
// sigma containing measurements and their errors.  Assuming that the
// measurements obey a Gaussian distribution, these method return the
// mean and standard deviation of the input values where the inverse
// squared errors are used as weighting factors.

   double weighted_mean(
      const vector<double>& X,const vector<double>& sigma)
   {
      double numer=0;
      double denom=0;
      for (unsigned int i=0; i<X.size(); i++)
      {
         numer += X[i]/sqr(sigma[i]);
         denom += 1.0/sqr(sigma[i]);
      }

      double mu=numer/denom;
      return mu;
   }

   double weighted_std_dev(
      const vector<double>& X,const vector<double>& sigma)
   {
      double numer=0;
      double denom=0;
      for (unsigned int i=0; i<X.size(); i++)
      {
         numer += X[i]/sqr(sigma[i]);
         denom += 1.0/sqr(sigma[i]);
      }

      double variance=1/denom;
      double std_dev=sqrt(variance);
      return std_dev;
   }

// ---------------------------------------------------------------------
// Method moment_of_inertia_2D

   void moment_of_inertia_2D(
      const twovector& origin,
      double& Ixx,double& Iyy,double& Ixy,double& Imin,double& Imax,
      const vector<twovector>& R)
   {
      double xsqsum=0;
      double ysqsum=0;
      double xysum=0;
      double zsum=R.size();
      for (unsigned int i=0; i<R.size(); i++)
      {
         double dx=R[i].get(0)-origin.get(0);
         double dy=R[i].get(1)-origin.get(1);

         xsqsum += sqr(dx);
         ysqsum += sqr(dy);
         xysum += dx*dy;
      } // loop over index i
      Ixx=ysqsum/zsum;
      Iyy=xsqsum/zsum;
      Ixy=-xysum/zsum;

//   cout << "Ixx = " << Ixx << " Iyy = " << Iyy << " Ixy = " << Ixy << endl;

      double I1=(Ixx+Iyy+sqrt(sqr(Ixx-Iyy)+4*sqr(Ixy)))/2;
      double I2=(Ixx+Iyy-sqrt(sqr(Ixx-Iyy)+4*sqr(Ixy)))/2;
      Imin=basic_math::min(I1,I2);
      Imax=basic_math::max(I1,I2);
   }
   
// ---------------------------------------------------------------------
   void moment_of_inertia_2D(
      const twovector& origin,double& Imin,double& Imax,
      twovector& Imin_hat,twovector& Imax_hat,const vector<twovector>& R)
   {
      double Ixx,Iyy,Ixy;
      moment_of_inertia_2D(origin,Ixx,Iyy,Ixy,Imin,Imax,R);

// If Ixy vanishes, then the principle axes lie in the x and y directions:

      const double TINY=1E-8;
      double theta_min;
//         double theta_max,theta_max2,theta_min2;
      if (fabs(Ixy) < TINY)
      {
         if (Ixx < Iyy)
         {
            theta_min=0;
//               theta_max=PI/2;
         }
         else
         {
            theta_min=PI/2;
//               theta_max=0;
         }
      }
      else
      {
         theta_min=-atan2(Ixy,Iyy-Imin);
//            theta_max=-atan2(Ixy,Iyy-Imax);
//            theta_min2=-atan2(Ixx-Imin,Ixy);
//            theta_max2=-atan2(Ixx-Imax,Ixy);
      }
      Imin_hat=twovector(cos(theta_min),sin(theta_min));
      Imax_hat=twovector(-Imin_hat.get(1),Imin_hat.get(0));

//   cout << "Imin = " << Imin << " Imax = " << Imax << endl;
//   cout << "theta_min = " << theta_min*180/PI 
//        << " theta_max = " << theta_max*180/PI << endl;
//   cout << "theta_min2 = " << theta_min2*180/PI 
//        << " theta_max2 = " << theta_max2*180/PI << endl;
//   cout << "Imax_hat = " << Imax_hat << endl;
//   cout << "Imin_hat = " << Imin_hat << endl;
   }

// ---------------------------------------------------------------------
// Method moment_of_inertia_bbox_area takes in a set of twovectors R
// along with their origin and axes' direction vectors Imin_hat and
// Imax_hat.  It loops over each twovector and computes its
// coordinates along the two axes.  The bounding box enclosing all the
// input twovectors which is aligned with the axes is found, and its
// area is returned by this method.

   double moment_of_inertia_bbox_area(
      const twovector& origin,
      const twovector& Imin_hat,const twovector& Imax_hat,
      const vector<twovector>& R)
   {

      double a_min=POSITIVEINFINITY;
      double a_max=NEGATIVEINFINITY;
      double b_min=POSITIVEINFINITY;
      double b_max=NEGATIVEINFINITY;
      for (unsigned int i=0; i<R.size(); i++)
      {
         twovector curr_delta=R[i]-origin;
         double a=curr_delta.dot(Imin_hat);
         double b=curr_delta.dot(Imax_hat);
         a_min=basic_math::min(a_min,a);
         a_max=basic_math::max(a_max,a);
         b_min=basic_math::min(b_min,b);
         b_max=basic_math::max(b_max,b);
      } // loop over index i

//         cout << "a_min = " << a_min << " a_max = " << a_max << endl;
//         cout << "b_min = " << b_min << " b_max = " << b_max << endl;

      double bbox_area=(a_max-a_min)*(b_max-b_min);
      return bbox_area;
   }

// ==========================================================================
// 1D incremental statistical methods
// ==========================================================================

// Method incremental_mean() computes a "running average" for a
// sequence of numbers.  Note that this method can be used to compute
// means, means of squares, means of cubes, etc.

// Input  : n >= 0         Step within running sequence 
//	    curr_x	   Input value at step n
//	    prev_mean	   Running average for n-1 numbers

// Output : return value   Running average for n numbers

   double incremental_mean(int n, double curr_x, double prev_mean)
   {
      if (n==0)
      {
         return curr_x;
      }
      else
      {
	 double frac = (double) n / (double) (n + 1);
	 double curr_mean = frac * prev_mean + (1 - frac) * curr_x;
         return curr_mean;
      }
   }

// ---------------------------------------------------------------------
// Method incremental_std_dev() computes a "running standard deviation" 
// for a sequence of numbers.
//
// Input  : n >= 0         Step within running sequence 
//	    curr_x	   Input value at step n
//	    prev_mean	   Running average for n-1 numbers
//	    prev_std_dev   Standard deviation for n-1 numbers

// Output : return value   Standard deviation for n numbers

   double incremental_std_dev(
      int n, double curr_x, double prev_mean, double prev_std_dev)
   {
      if (n==0)
      {
         return 0;
      }
      else{
         double curr_mean = incremental_mean(n, curr_x, prev_mean);
         double prev_mean_sqr = sqr(prev_std_dev) + sqr(prev_mean);
         double curr_mean_sqr = incremental_mean(n,sqr(curr_x),prev_mean_sqr);
         double curr_var = curr_mean_sqr - sqr(curr_mean);
         double sigma = sqrt(curr_var);
         return sigma;
      }
   }

// ---------------------------------------------------------------------
// Method combined_mean() computes the mean of 2 sets of numbers.

// Input  : a         	   Number of elements in first set
//	    b		   Number of elements in second set
//	    mean_a	   Mean of first set of numbers
//	    mean_b	   Mean of second set of numbers
//
// Output : return value   Mean of combined set of numbers

   double combined_mean(unsigned int a, unsigned int b, 
                        double mean_a, double mean_b)
   {
      double denom = a + b;
      double combined_mean = (a * mean_a + b * mean_b) / denom;
      return combined_mean;
   }

// ---------------------------------------------------------------------
// Method combined_std_dev() computes the standard deviation of 2 sets
// of numbers.

// Input  : a         	   Number of elements in first set
//	    b		   Number of elements in second set
//	    mean_a	   Mean of first set of numbers
//	    mean_b	   Mean of second second of numbers
//	    std_dev_a	   Standard deviation of first set of numbers
//	    std_dev_b	   Standard deviation of second set of numbers

// Output : return value   Mean of combined set of numbers

   double combined_std_dev(unsigned int a, unsigned int b, 
                           double mean_a, double mean_b,
                           double std_dev_a, double std_dev_b)
   {
      double combined_mu = combined_mean(a, b, mean_a, mean_b);
      double mu_sqr_a = sqr(std_dev_a) + sqr(mean_a);
      double mu_sqr_b = sqr(std_dev_b) + sqr(mean_b);
      double combined_mu_sqr = combined_mean(a, b, mu_sqr_a, mu_sqr_b);
  
      double combined_variance = combined_mu_sqr - sqr(combined_mu);
      double combined_std_dev = sqrt(combined_variance);
      return combined_std_dev;
   }

// ---------------------------------------------------------------------
// Method mean_std_dev_for_subset() computes the mean and standard
// deviation for some set B of numbers given the mean and standard
// deviations for set A and combined set A+B.

// Input  : a         	   Number of elements in first set
//	    b		   Number of elements in second set
//	    mean_a	   Mean of first set of numbers
//	    std_dev_a	   Standard deviation of first set of numbers
//	    mean_ab	   Mean of all numbers
//	    std_dev_ab	   Standard deviation of all numbers

// Output:  mean_b	   Mean of second second of numbers
//	    std_dev_b	   Standard deviation of second set of numbers

   void mean_std_dev_for_subset(
      unsigned int a, unsigned int b,
      double mean_a, double std_dev_a,
      double mean_ab, double std_dev_ab,
      double& mean_b, double& std_dev_b)
   {
      double mu_sqr_a = sqr(mean_a) + sqr(std_dev_a);
      double mu_sqr_ab = sqr(mean_ab) + sqr(std_dev_ab);
      double mu_sqr_b = ( (a+b) * mu_sqr_ab - a * mu_sqr_a)/double(b);
      mean_b = ( (a+b) * mean_ab - a * mean_a)/double(b);
      std_dev_b = sqrt( mu_sqr_b - sqr(mean_b));
   }

// ==========================================================================
// N-dimensional statistical methods
// ==========================================================================

// This overloaded version of mean() takes in an STL vector containing
// a set of genvector pointers.  It instantiates and returns a
// genvector containing the mean values for each of the input vectors
// components.

   genvector* mean(const vector<genvector*>& A)
   {
//      cout << "inside mathfunc::mean()" << endl;
//      cout << "A.size() = " << A.size() << endl;
      
      unsigned int mdim=A[0]->get_mdim();
      genvector* mean_ptr=new genvector(mdim);
      mean_ptr->clear_values();

      for (unsigned int d=0; d<mdim; d++)
      {
         for (unsigned int v=0; v<A.size(); v++)
         {
            mean_ptr->put(d,mean_ptr->get(d)+A[v]->get(d));
         } // loop over index v labeling input genvectors
         mean_ptr->put(d,mean_ptr->get(d)/A.size());
      } // loop over index d labeling vector dimensions

      return mean_ptr;
   }

// ---------------------------------------------------------------------
// Method recursive_mean() sequentially takes in a set of genvectors.
// Starting with a zero-valued mean vector, this method returns the
// mean of the first n vectors at the nth recursion step.

   void recursive_mean(
      int n,const genvector* A_ptr,genvector* recur_mean_ptr)
   {
//      cout << "inside mathfunc::recursive_mean()" << endl;

      double coeff1=double(n)/double(n+1);
      double coeff2=1.0/double(n+1);
      *recur_mean_ptr = coeff1 * (*recur_mean_ptr) + coeff2 * (*A_ptr);
   }

   void recursive_mean(
      int n,const descriptor* A_ptr,descriptor* recur_mean_ptr)
   {
//      cout << "inside mathfunc::recursive_mean() #2" << endl;

      double coeff1=double(n)/double(n+1);
      double coeff2=1.0/double(n+1);
      *recur_mean_ptr = coeff1 * (*recur_mean_ptr) + coeff2 * (*A_ptr);
   }

// ---------------------------------------------------------------------
// This overloaded version of recursive_mean() has been
// intentionally optimized to run as fast as possible.

   void recursive_mean(
      int mdim,int n,const genvector* A_ptr,genvector* recur_mean_ptr)
   {
//      cout << "inside mathfunc::recursive_mean() #3" << endl;

      double coeff1=double(n)/double(n+1);
      double coeff2=1.0/double(n+1);
      for (int i=0; i<mdim; i++)
      {
         recur_mean_ptr->put(
            i,coeff1*recur_mean_ptr->get(i)+coeff2*A_ptr->get(i));
      }
   }

   void recursive_mean(
      int mdim,int n,const descriptor* A_ptr,descriptor* recur_mean_ptr)
   {
//      cout << "inside mathfunc::recursive_mean() #4" << endl;

      double coeff1=double(n)/double(n+1);
      double coeff2=1.0/double(n+1);
      for (int i=0; i<mdim; i++)
      {
         recur_mean_ptr->put(
            i,coeff1*recur_mean_ptr->get(i)+coeff2*A_ptr->get(i));
      }
   }

// ---------------------------------------------------------------------
// Method recursive_second_moment() sequentially takes in a set of
// genvectors.  Starting with a zero-valued genmatrix, this
// method returns the second moment of the first n vectors at the nth
// recursion step.

   void recursive_second_moment(
      int n,const genvector* A_ptr,genmatrix* recur_second_moment_ptr)
   {
//      cout << "inside mathfunc::recursive_second_moment() #1a" << endl;

      int mdim=A_ptr->get_mdim();
      genmatrix outerprod(mdim,mdim);
      outerprod=A_ptr->outerproduct(*A_ptr);

      double coeff1=double(n)/double(n+1);
      double coeff2=1.0/double(n+1);
      
      *recur_second_moment_ptr = coeff1 * (*recur_second_moment_ptr) 
         + coeff2 * outerprod;
   }

   void recursive_2nd_moment(
      int n,const descriptor* A_ptr,genmatrix* recur_second_moment_ptr)
   {
//      cout << "inside mathfunc::recursive_second_moment() #1b" << endl;

      unsigned int mdim=A_ptr->get_mdim();
      genmatrix outerprod(mdim,mdim);
      A_ptr->outerproduct(*A_ptr,*recur_second_moment_ptr);

      double coeff1=double(n)/double(n+1);
      double coeff2=1.0/double(n+1);
      
      *recur_second_moment_ptr = coeff1 * (*recur_second_moment_ptr) 
         + coeff2 * outerprod;
   }

// ---------------------------------------------------------------------
// This overloaded version of recursive_second_moment() has been
// intentionally optimized to run as fast as possible.
   
   void recursive_second_moment(
      int mdim,int n,const genvector* A_ptr,
      genmatrix* recur_second_moment_ptr)
   {
//      cout << "inside mathfunc::recursive_second_moment() #2a" << endl;

      double coeff1=double(n)/double(n+1);
      double coeff2=1.0/double(n+1);

      for (int i=0; i<mdim; i++)
      {
         double A_i=A_ptr->get(i);
         for (int j=0; j<mdim; j++)
         {
//            double term1=coeff1*recur_second_moment_ptr->get(i,j);
//            double term2=coeff2*A_i*A_ptr->get(j);
            recur_second_moment_ptr->put(
               i,j,coeff1*recur_second_moment_ptr->get(i,j)+
               coeff2*A_i*A_ptr->get(j));
         } // loop over index j
      } // loop over index i 
   }
   
// ---------------------------------------------------------------------
   void recursive_2nd_moment(
      int mdim,int n,const descriptor* A_ptr,
      genmatrix* recur_second_moment_ptr)
   {
//      cout << "inside mathfunc::recursive_second_moment() #2b" << endl;

      double coeff1=double(n)/double(n+1);
      double coeff2=1.0/double(n+1);

      for (int i=0; i<mdim; i++)
      {
         double A_i=A_ptr->get(i);
         for (int j=0; j<mdim; j++)
         {
//            double term1=coeff1*recur_second_moment_ptr->get(i,j);
//            double term2=coeff2*A_i*A_ptr->get(j);
            recur_second_moment_ptr->put(
               i,j,coeff1*recur_second_moment_ptr->get(i,j)+
               coeff2*A_i*A_ptr->get(j));
         } // loop over index j
      } // loop over index i 
   }

// ---------------------------------------------------------------------
   void recursive_second_moment(
      int n,const descriptor* A_ptr,flann::Matrix<float>* 
      recur_second_moment_ptr)
   {
//      cout << "inside mathfunc::recursive_second_moment() #3" << endl;

      int mdim=A_ptr->get_mdim();

      double coeff1=double(n)/double(n+1);
      double coeff2=1.0/double(n+1);

      for (int i=0; i<mdim; i++)
      {
         double A_i=A_ptr->get(i);
         for (int j=0; j<mdim; j++)
         {
            (*recur_second_moment_ptr)[i][j] = 
               coeff1 * (*recur_second_moment_ptr)[i][j] 
               + coeff2 * A_i * A_ptr->get(j);
         }
      }
   }

// ---------------------------------------------------------------------
// Method covariance_matrix() takes in an STL vector containing
// a set of genvector pointers.  It instantiates and returns the
// covariance matrix for the input vectors.

   genmatrix* covariance_matrix(const vector<genvector*>& A)
   {
//      cout << "inside mathfunc::covariance_matrix()" << endl;
//      cout << "A.size() = " << A.size() << endl;
      
      unsigned int mdim=A[0]->get_mdim();
      genmatrix* covariance_matrix_ptr=new genmatrix(mdim,mdim);
      covariance_matrix_ptr->clear_values();

      genvector* mean_ptr=mathfunc::mean(A);
//      cout << "*mean_ptr = " << *mean_ptr << endl;

// Recall covariance matrix is symmetric.  So only explicitly compute its 
// upper triangular part:

      for (unsigned int i=0; i<mdim; i++)
      {
//         cout << i << " " << flush;
         for (unsigned int j=i; j<mdim; j++)
         {
            for (unsigned int v=0; v<A.size(); v++)
            {
               double factor1=A[v]->get(i)-mean_ptr->get(i);
               double factor2=A[v]->get(j)-mean_ptr->get(j);
               covariance_matrix_ptr->put(
                  i,j,covariance_matrix_ptr->get(i,j)+factor1*factor2);
            } // loop over index v labeling input genvectors
            covariance_matrix_ptr->put(
               i,j,covariance_matrix_ptr->get(i,j)/A.size());

         } // loop over index j labeling covariance matrix column
      } // loop over index i labeling covariance matrix row
      cout << endl;

      for (unsigned int i=0; i<mdim; i++)
      {
         for (unsigned int j=0; j<i; j++)
         {
            covariance_matrix_ptr->put(
               i,j,covariance_matrix_ptr->get(j,i));
         }
      }
      
      delete mean_ptr;
      return covariance_matrix_ptr;
   }

// ---------------------------------------------------------------------
// Method recursive_covariance_matrix() sequentially takes in a set of
// genvectors.  Starting with a zero-valued covariance matrix, this
// method returns the covariance of the first n vectors at the nth
// recursion step.

   void recursive_covariance_matrix(
      int n,const genvector* A_ptr,const genvector* mean_ptr,
      genmatrix* recur_covar_ptr)
   {
//      cout << "inside mathfunc::recursive_covariance_matrix()" << endl;

      genmatrix mean_outerprod( mean_ptr->outerproduct(*mean_ptr) );
      genmatrix outerprod( A_ptr->outerproduct(*A_ptr) );
      double coeff1=double(n)/double(n+1);
      double coeff2=1.0/double(n+1);

      *recur_covar_ptr=coeff1 * (*recur_covar_ptr)
         - coeff2 * mean_outerprod + coeff2 * outerprod;
   }

   void recursive_covariance_matrix(
      int n,const genvector* A_ptr,const genmatrix& mean_outerprod,
      genmatrix* recur_covar_ptr)
   {
      int mdim = A_ptr->get_mdim();
      genmatrix outerprod(mdim, mdim);

      A_ptr->self_outerproduct(outerprod);

      double coeff1 = double(n)/double(n+1);
      double coeff2 = 1 - coeff1;

      *recur_covar_ptr = coeff1 * (*recur_covar_ptr)
         - coeff2 * mean_outerprod + coeff2 * outerprod;
   }

// ==========================================================================
// Incremental N-dimensional statistical methods working on C arrays
// ==========================================================================

// Method incremental_mean_vec

/*
 *
 * Input  : mdim		   	Vector space dimension
 *	    n				iteration counter
 *	    X				Current iteration sample vector
 *	    incremental_mean		Vector mean calculated from n-1 sample vectors
 *
 * Output:  incremental_mean		Vector mean calculated from n sample vectors
 *****/

void incremental_mean_vec(unsigned int mdim, unsigned int n, float *X, 
                          double *incremental_mean)
{
  unsigned int i;
  double coeff1 = (double) n / (double) (n + 1);
  double coeff2 = 1.0 / (double) (n + 1);
  for(i = 0; i < mdim; i++)
  {
    incremental_mean[i] = coeff1 * incremental_mean[i] + coeff2 * X[i];
  }
}

/***** ma_incremental_second_vec_moment ******************************************
 * Description : 	
 *
 * Input  : mdim		   	Vector space dimension
 *	    n				iteration counter
 *	    X				Current iteration sample vector
 *	    incremental_second_moment	<X[i] X[j]> calculated from n-1 sample vectors
 *
 * Output:  incremental_second_momemnt	<X[i] X[j]> calculated from n sample vectors
 *****/

void incremental_second_vec_moment(
   unsigned int mdim, unsigned int n, float *X, 
   double **incremental_second_moment)
{
  unsigned int i, j;
  double coeff1 = (double) n / (double) (n + 1);
  double coeff2 = 1.0 / (double) (n + 1);

  for (i = 0; i < mdim; i++)
  {
    for (j = 0; j < mdim; j++)
    {
      incremental_second_moment[i][j] = 
         coeff1 * incremental_second_moment[i][j] + coeff2 * X[i] * X[j];
    } // loop over index j
  } // loop over index i 
}

/***** incremental_covar_matrix ******************************************
 * Description : 	
 *
 * Input  : mdim		   	Vector space dimension
 *	    mean			<X[i]> for all sample vectors
 *	    second_moment		<X[i] X[j]> for all sample vectors
 *
 * Output:  covar_matrix		<(X[i] - mu) * (X[j]-mu)> for all sample vectors
 *****/

void covar_matrix(unsigned int mdim, double *mean, double **second_moment, 
                  double **covar_matrix)
{
  unsigned int i, j;
  for (i = 0; i < mdim; i++)
  {
    for (j = 0; j < mdim; j++)
    {
      covar_matrix[i][j] = second_moment[i][j] - mean[i] * mean[j];
    } // loop over index j
  } // loop over index i 
}






// ---------------------------------------------------------------------
// Method Mahalanobis_distance() takes in two genvectors X,Y along
// with an inverse covariance matrix for some distribution of
// genvectors.  It returns the distance between the two input
// genvectors normalized by the input covariance matrix.

   double Mahalanobis_distance(
      const genvector* X_ptr,const genvector* Y_ptr,
      const genmatrix* inverse_covariance_matrix_ptr)
   {
      cout << "inside mathfunc::Mahalanobis_distance()" << endl;
      
      double dsqr=
         (*X_ptr - *Y_ptr).dot(
            (*inverse_covariance_matrix_ptr) * 
            ( *X_ptr - *Y_ptr) );

      return sqrt(dsqr);
   }

// ==========================================================================
// Tensor methods
// ==========================================================================

// Method LeviCivita() implements the fully antisymmetric 3D
// Levi-Civita tensor.  Input indices are expected to equal 0, 1 or 2.

   int LeviCivita(int i,int j,int k)
   {
      if (i==j || j==k || k==i) return 0;

      if (i==0 && j==1 && k==2) return 1;
      if (i==1 && j==2 && k==0) return 1;
      if (i==2 && j==0 && k==1) return 1;

      if (i==0 && j==2 && k==1) return -1;
      if (i==1 && j==0 && k==2) return -1;
      if (i==2 && j==1 && k==0) return -1;
      
      cout << "Error in mathfunc::LeviCivita(i,j,k)!" << endl;
      cout << "i = " << i << " j = " << j << " k = " << k << endl;
      exit(-1);
   }

// ==========================================================================
// Trigonometry methods:
// ==========================================================================

   double cosinv(double x)
   {
      if (fabs(x) <= 1)
      {
         double term1=x/sqrt(1-x*x);
         double term2=atan(term1);
         return(PI/2-term2);
      }
      else
      {
         cout << "Call to cosinv(x) with |x| > 1 !! " << endl;
         cout << "x = " << x << endl;
         exit(-1);
      }
   }

// --------------------------------------------------------------------------
   double coshinv(double x)
   {
      if (x >= 1)
      {
         return log(x+sqrt(x*x-1));
      }
      else
      {
         cout << "Call to coshinv(x) with x < 1 !!!" << endl;
         cout << "x = " << x << endl;
         exit(-1);
      }
   }

// ---------------------------------------------------------------------
// Method myatan2 maintains continuity across the theta=-PI,PI branch
// cut.  It returns atan2(y,x) +/- 0,2 PI , whichever is closest to
// some previous angle value.  This arctangent routine is useful for
// calculating derivatives with respect to theta where continuity must
// be preserved across the branch cut.

// In July 03, we discovered some serious problems within this method
// which lead to catastrophic RH results.  So we have intentionally
// dumbed this method down as much as possible in an attempt to make
// it robust!

   double myatan2(double y,double x,double prevangle)
   {
      double angle0=atan2(y,x);
      double angle1=angle0+2*PI;
      double angle2=angle0-2*PI;

      double diff0=fabs((angle0-prevangle));
      double diff1=fabs((angle1-prevangle));
      double diff2=fabs((angle2-prevangle));

      double angle;
      if ((diff1 <= diff0) && (diff1 <= diff2))
      {
//            cout << "mindiff = diff1" << endl;
//            cout << "angle0 = " << angle0*180/PI << endl;
//            cout << "angle1 = " << angle1*180/PI << endl;
//            cout << "angle2 = " << angle2*180/PI << endl;
//            cout << "diff0 = " << diff0*180/PI << endl;
//            cout << "diff1 = " << diff1*180/PI << endl;
//            cout << "diff2 = " << diff2*180/PI << endl;
         angle=angle1;
      }
      else if ((diff2 <= diff0) && (diff2 <= diff1))
      {
//            cout << "mindiff = diff2" << endl;
//            cout << "angle0 = " << angle0*180/PI << endl;
//            cout << "angle1 = " << angle1*180/PI << endl;
//            cout << "angle2 = " << angle2*180/PI << endl;
//            cout << "diff0 = " << diff0*180/PI << endl;
//            cout << "diff1 = " << diff1*180/PI << endl;
//            cout << "diff2 = " << diff2*180/PI << endl;
         angle=angle2;
      }
      else
      {
         angle=angle0;
      }
      return angle;
   }

/*
  double myatan2(double y, double x, double prevangle)
  {
  const double pi=3.14159265358979323846;
  double angle[3],diff[3];
   
  angle[0]=atan2(y,x);
  angle[1]=angle[0]+2*pi;
  angle[2]=angle[0]-2*pi;

  diff[0]=fabs(angle[0]-prevangle);
  diff[1]=fabs(angle[1]-prevangle);
  diff[2]=fabs(angle[2]-prevangle);

  double mindiff=basic_math::min(diff[0],diff[1],diff[2]);
  if (mindiff==diff[0])
  {
  return angle[0];
  }
  else if (mindiff==diff[1])
  {
  cout << "mindiff = diff[1]" << endl;
  cout << "angle0 = " << angle[0]*180/pi << endl;
  cout << "angle1 = " << angle[1]*180/pi << endl;
  cout << "angle2 = " << angle[2]*180/pi << endl;
  cout << "diff0 = " << diff[0]*180/pi << endl;
  cout << "diff1 = " << diff[1]*180/pi << endl;
  cout << "diff2 = " << diff[2]*180/pi << endl;
  return angle[1];
  }
  else
  {
  cout << "mindiff = diff[2]" << endl;
  cout << "angle0 = " << angle[0]*180/pi << endl;
  cout << "angle1 = " << angle[1]*180/pi << endl;
  cout << "angle2 = " << angle[2]*180/pi << endl;
  cout << "diff0 = " << diff[0]*180/pi << endl;
  cout << "diff1 = " << diff[1]*180/pi << endl;
  cout << "diff2 = " << diff[2]*180/pi << endl;
  return angle[2];
  }
  }
*/

// ==========================================================================
// Miscellaneous methods:
// ==========================================================================

   void addarray(double A1[],double A2[],double Asum[],int Nsize)
   {
      for (int i=0; i<Nsize; i++) Asum[i]=A1[i]+A2[i];
   }

// ---------------------------------------------------------------------
// Method approx_ellipse_circumference takes in semi-major and
// semi-minor axes' lengths a and b.  It evaluates and returns a
// closed form approximation for the circumference of the
// corresponding ellipse (http://en.wikipedia.org/wiki/Ellipse):

   double approx_ellipse_circumference(double a,double b)
   {
      double term1=PI*(a+b);
      double numer=3*sqr( (a-b)/(a+b) );
      double denom=10+sqrt(4-numer);
      double term2=1+numer/denom;
      double approx_circumference=term1*term2;
      return approx_circumference;
   }

// ---------------------------------------------------------------------
// Function binary_locate() takes in STL vector data which is
// monotonically increasing or decreasing over a range between
// starting and ending bins startbin and stopbin.  It performs a
// binary search to locate the index i for which the specified value
// x_in is bracketed as data[i] <= x_in < data[i+1] or data[i] >= x >
// data[i+1].  If x < data[startbin] or x > data[stopbin], then
// startbin-1 and stopbin+1 are respectively returned.  This method is
// basically a rewrite of the Numerical Recipes "locate" routine
// (which is badly written and sometimes buggy!)

   int binary_locate(
      const vector<double>& data,int startbin,int stopbin,double x_in)
   {
      int nsize=stopbin-startbin+1;
      vector<double> datacopy;
 
      double maxvaluediff=data[stopbin]-data[startbin];
      for (int i=0; i<nsize; i++)
      {
         if (maxvaluediff >=0)
         {
            datacopy.push_back(data[startbin+i]);
         }
         else
         {
            datacopy.push_back(-data[startbin+i]);
         }
      }

      double x;
      if (maxvaluediff >=0)
      {
         x=x_in;
      }
      else
      {
         x=-x_in;
      }

// Make sure x is not less than datacopy[startbin] or greater than
// datacopy[nsize]:

      int istar;
      if (x < datacopy[0])
      {
         istar=startbin-1;
      }
      else if (x > datacopy[nsize-1])
      {
         istar=stopbin+1;
      }

// Check whether x==datacopy[nsize-1].  If so, immediately return
// stopbin without performing binary search:

      else if (x==datacopy[nsize-1])
      {
         istar=stopbin;
      }
      else
      {
         int lo_bin=0;
         int hi_bin=nsize-1;
         int nsteps=0;
         while(hi_bin-lo_bin > 1)
         {
            int curr_bin=basic_math::round(0.5*(lo_bin+hi_bin));
            if (x >= datacopy[curr_bin])
            {
               lo_bin=curr_bin;
            }
            else
            {
               hi_bin=curr_bin;
            }
            nsteps++;
         }
         istar=startbin+lo_bin;
      }

      return istar;
   }

   int binary_locate(double data[],int startbin,int stopbin,double x_in)
   {
      int nsize=stopbin-startbin+1;
      double datacopy[nsize];
 
      double maxvaluediff=data[stopbin]-data[startbin];
      for (int i=0; i<nsize; i++)
      {
         if (maxvaluediff >=0)
         {
            datacopy[i]=data[startbin+i];
         }
         else
         {
            datacopy[i]=-data[startbin+i];
         }
      }

      double x;
      if (maxvaluediff >=0)
      {
         x=x_in;
      }
      else
      {
         x=-x_in;
      }

// Make sure x is not less than datacopy[startbin] or greater than
// datacopy[nsize]:

      int istar;
      if (x < datacopy[0])
      {
         istar=startbin-1;
//            return(startbin-1);
      }
      else if (x > datacopy[nsize-1])
      {
         istar=stopbin+1;
//            return(stopbin+1);
      }

// Check whether x==datacopy[nsize-1].  If so, immediately return
// stopbin without performing binary search:

      else if (x==datacopy[nsize-1])
      {
         istar=stopbin;
//            return stopbin;
      }
      else
      {
         int lo_bin=0;
         int hi_bin=nsize-1;
         int nsteps=0;
         while(hi_bin-lo_bin > 1)
         {
            int curr_bin=basic_math::round(0.5*(lo_bin+hi_bin));
            if (x >= datacopy[curr_bin])
            {
               lo_bin=curr_bin;
            }
            else
            {
               hi_bin=curr_bin;
            }
            nsteps++;
         }
         istar=startbin+lo_bin;
//            return(startbin+lo_bin);
      }
      return istar;
   }

// ---------------------------------------------------------------------
// Method brute_force_auto_correlation takes in some function f within
// an STL vector.  It first subtracts off the function's mean value.
// It then evaluates the auto-correlation integral 

//   		    Sum_i f_zeromean[i] f_zeromean[i+j]

// for min_shift <= j < max_shift.  It returns integer j for which
// this auto-correlation sum is maximal.  

   int brute_force_auto_correlation(
      int min_shift,int max_shift,const vector<double>& f)
   {
         
// First compute and subtract f_avg from the input f STL vector to
// ensure that we work with a zero-mean function for auto-correlation
// purposes:

      double f_avg=0;
      for (unsigned int i=0; i<f.size(); i++)
      {
         f_avg += f[i];
      }
      f_avg /= double(f.size());

      vector<double> f_zeromean;
      f_zeromean.reserve(f.size());
      for (unsigned int i=0; i<f.size(); i++)
      {
         f_zeromean.push_back(f[i]-f_avg);
      }

// Next perform brute-force autocorrelation of zero-mean f function:

      int start_bin=max_shift;
      int stop_bin=f.size();
      vector<double> product;
      product.reserve(f.size());

      int j_best=-1;
      double max_correlation=NEGATIVEINFINITY;
      for (int j=min_shift; j<max_shift; j++)
      {
         product.clear();
         for (unsigned int i=0; i<f.size()-j; i++)
         {
            product.push_back(f_zeromean[i]*f_zeromean[i+j]);
         }
         double correlation=simpsonsum(product,start_bin,stop_bin);
         if (correlation > max_correlation)
         {
            max_correlation=correlation;
            j_best=j;
         }
      } // loop over index j labeling function shift

      cout << "Max correlation = " << max_correlation 
           << " j_best = " << j_best << endl;
      return j_best;
   }

// ---------------------------------------------------------------------
// Method cum_value takes in STL vector z.  After sorting its entries,
// this method return the entry corresponding to input fraction frac.
// Use of this quick-and-dirty method obviates the need to compute
// genuine probability distributions which can sometimes be corrupted
// by spurious entries within z.

   double cum_value(double frac,const vector<double>& z)
   {
      unsigned int nbins=z.size();
      vector<double> zcopy;
      for (unsigned int i=0; i<nbins; i++)
      {
         zcopy.push_back(z[i]);
      }
      std::sort(zcopy.begin(),zcopy.end());

      int i=basic_math::min((int) nbins-1,basic_math::round(frac*nbins));
      return zcopy[i];
   }

// ---------------------------------------------------------------------
   double dB(double x)
   {
      if (x < 0)
      {
         cout << "Error in mathfunc::dB" << endl;
         cout << " x = " << x << endl;
         exit(-1);
      }
      else if (x==0)
      {
         return(NEGATIVEINFINITY);
      }
      else
      {
         return 10*log10(x);
      }
   }

// ---------------------------------------------------------------------
// Method find_ordering takes in an array A of doubles and returns
// an integer array order whose entries contain the ordering of A.
// The zeroth ordered entry has the minimum value in A, while the
// (nbins-1)th entry has the maximum value in A.

   void find_ordering(int nbins,double A[],int order[])
   {
      vector<double> Asorted(nbins);
      for (int n=0; n<nbins; n++)
      {
         Asorted.push_back(A[n]);
      }
      std::sort(Asorted.begin(),Asorted.end());

      for (int n=0; n<nbins; n++)
      {
         order[n]=mylocate(Asorted,0,nbins-1,A[n]);
      }
   }

// ---------------------------------------------------------------------
// Method generate_skip_sequence takes in 3 integers jbegin, jend and
// jperiod.  If boolean input ascending_sequence==true, it generates a
// sequence of integers in which the separation between members
// progressively grows larger.  The final integer can be at most one
// less than jend.  For example, here is the sequence for jbegin=2,
// jend=111 and jperiod=4:

//  2 3 4 5 6 8 10 12 14 17 20 23 26 30 34 38 42 47 52 57 62 68 74 80
//  86 93 100 107

// The skip interval for the first 4 elements is 1, then it is 2 for
// the next 4 elements, 3 for the subsequent 4, etc.  The skip
// sequence is returned within integer array jsequence[].

// If ascending_sequence==false, the sequence returned for this
// example looks like:

// 110 109 108 107 106 104 102 100 98 95 92 89 86 82 78 74 70 65 60 55
// 50 44 38 32 26 19 12 5

   int generate_skip_sequence(int jbegin,int jend,int jperiod,int jsequence[],
                              bool ascending_sequence)
   {
      int jcount=0;
      int jstep=1;
      int jskip=0;
      int counter=0;
         
      jperiod++;
      for (int j=jbegin; j<jend; j++)
      {
         if ((jcount+1)%jperiod==0)
         {
            jstep++;
            jcount=0;
         }
         if (j-jbegin < jskip)
         {
         }
         else
         {
            jsequence[counter++]=j;
            jcount++;
            jskip += jstep;
         }
      }

      if (!ascending_sequence)
      {
         for (int n=0; n<counter; n++)
         {
            jsequence[n]=jend-(jsequence[n]-jbegin+1);
         }
      }
      return counter;
   }

// ---------------------------------------------------------------------
   void multarray(double A1[],double A2[],double Aprod[],int Nsize)
   {
      for (int i=0; i<Nsize; i++) Aprod[i]=A1[i]*A2[i];
   }


// ---------------------------------------------------------------------
// In late June/early July 2007, we discovered that there is some
// problem within libtdp which prevents us from calling under Mac OS X
// the legitimate isnan() macro defined within math.h So as an ugly
// workaround, we replace the isnan() call with __isnan() in order to
// achieve portability from linux to Mac OS X...  We also wrap this
// ugly call within this ugly special method so that it is at least
// confined to one location within our entire source tree.

   bool my_isnan(double x)
   {
      return __isnan(x);
   }

// ---------------------------------------------------------------------
// Method second_derivs takes in 3x3 array f which represents some
// subset of a function of two variables.  Using formulas 25.3.25 and
// 25.3.26 in "Handbook of Mathematical Functions" by Abramowitz and
// Stegun, it computes the numerical xx, yy and xy second derivatives
// of function f at the array point (xoffset,yoffset).  We
// specifically choose to implement the longer formula for fxx and fyy
// rather than its shorter counterpart 25.3.23 in order to average out
// as much as possible random noise present within the 8 nearest
// elements neighboring some particular pixel point.

   void second_derivs(double xstep,double ystep,double f[3][3],
                      double& fxx,double& fyy,double& fxy)
   {
      int xoffset=1;
      int yoffset=1;

      fxx=(f[xoffset+1][yoffset+0]
           -2*f[xoffset+0][yoffset+0]
           +f[xoffset-1][yoffset+0])/sqr(xstep);

      fyy=(f[xoffset+0][yoffset+1]
           -2*f[xoffset+0][yoffset+0]
           +f[xoffset+0][yoffset-1])/sqr(ystep);
   
      /*
        fxx=(f[xoffset+1][yoffset+1]
        -2*f[xoffset+0][yoffset+1]
        +f[xoffset-1][yoffset+1]
        +f[xoffset+1][yoffset+0]
        -2*f[xoffset+0][yoffset+0]
        +f[xoffset-1][yoffset+0]
        +f[xoffset+1][yoffset-1]
        -2*f[xoffset+0][yoffset-1]
        +f[xoffset-1][yoffset-1])/(3.0*sqr(xstep));

        fyy=(f[xoffset+1][yoffset+1]
        -2*f[xoffset+1][yoffset+0]
        +f[xoffset+1][yoffset-1]
        +f[xoffset+0][yoffset+1]
        -2*f[xoffset+0][yoffset+0]
        +f[xoffset+0][yoffset-1]
        +f[xoffset-1][yoffset+1]
        -2*f[xoffset-1][yoffset+0]
        +f[xoffset-1][yoffset-1])/(3.0*sqr(ystep));
      */
 
      fxy=(f[xoffset+1][yoffset+1]
           -f[xoffset+1][yoffset-1]
           -f[xoffset-1][yoffset+1]
           +f[xoffset-1][yoffset-1])/(4.0*xstep*ystep);
   }
   
// ==========================================================================

   namespace errorfunc
   {
      const int fast_errorfunc_array_size=50000;
      const double x_high=5;
      const double x_low=-x_high;
      const double delta_x=(x_high-x_low)/(fast_errorfunc_array_size-1);
      double fast_error_func_array[fast_errorfunc_array_size];

      void initialize_fast_error_function()
      {
         for (int n=0; n<fast_errorfunc_array_size; n++)
         {
            double x=x_low+n*delta_x;
            fast_error_func_array[n]=error_function(x);
         }
      }
   
      double fast_error_function(double x)
      {
         double fast_erfx;

         if (x > x_high)
         {
            fast_erfx=1;
         }
         else if (x < x_low)
         {
            fast_erfx=-1;
         }
         else
         {
            int n=basic_math::round((x-x_low)/delta_x);
            fast_erfx=fast_error_func_array[n];
         }
         return fast_erfx;
      }
   } // errorfunc sub-namespace

} // mathfunc namespace



