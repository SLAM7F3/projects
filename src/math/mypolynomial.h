// ==========================================================================
// Header file for mypolynomial class
// ==========================================================================
// Last updated on 3/31/12; 1/31/13; 3/29/14; 6/7/14
// ==========================================================================

#ifndef MYPOLYNOMIAL_H
#define MYPOLYNOMIAL_H

#include <iostream>
#include <string>
#include <vector>
#include "color/colorfuncs.h"
#include "math/genmatrix.h"
#include "math/mathfuncs.h"
#include "datastructures/datapoint.h"
#include "datastructures/Linkedlist.h"
typedef Linkedlist<datapoint> linkedlist;
template <class T> class Genarray;
typedef Genarray<double> genarray;

class mypolynomial
{
  public:

   enum Basis
   {
      chebyshev,hermite,harmonic_osc,power
   };

// Initialization, constructor and destructor functions:

   void clear_coeffs();
   mypolynomial(void);
   mypolynomial(const mypolynomial& p);
   mypolynomial(int polyorder);
   mypolynomial(int polyorder,double coeff[]);
   mypolynomial(int polyorder,const std::vector<double>& coeff);
   ~mypolynomial();
   mypolynomial& operator= (const mypolynomial& p);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const mypolynomial& p);
 
// Set and get methods:

   void set_basis(Basis b);
   void set_order(int o);
   void set_coeff(unsigned int coeff_index,double coeff_value);
   void set_coeffs(double coeff_values[]);
   Basis get_basis() const;
   std::string get_basis_str(Basis basis) const;
   unsigned int get_order() const;
   double get_coeff(int coeff_index) const;
   
// Polynomial evaluation methods:

   double value(double x) const;
   double value(double x,double mu,double sigma) const;
   double derivative(double x,int deriv_order) const;
   double derivative(double x,double mu,double sigma,int deriv_order) const;
   double power_value(double x) const;

// Basis function and derivative methods:

   double basis_function(double x,int poly_order) const;
   double deriv_basis_function(double x,int deriv_order,int poly_order) const;
   double basis_power(double x,int poly_order) const;
   double deriv_power(double x,int deriv_order,int poly_order) const;
   double basis_chebyshev(double x,int poly_order) const;
   double deriv_chebyshev(double x,int deriv_order,int poly_order) const;
   double basis_hermite(double x,int poly_order) const;
   double deriv_hermite(double x,int deriv_order,int poly_order) const;
   double basis_harmonic_osc(double y,int poly_order) const;
   double deriv_harmonic_osc(double y,int deriv_order,int poly_order) const;

// Extrema location methods:

   double global_extremum(
      bool find_min,double xlo,double xhi,double mu,double sigma,
      double& x_extremum);
   double global_extremum(
      bool find_min,int deriv_order,
      double xlo,double xhi,double mu,double sigma,double& x_extremum);
   double local_extremum(double x);

// Polynomial root member functions:

   double root(double x);
   genmatrix* construct_companion_matrix() const;

// Least squares fitting to a function:

   bool fit_coeffs(const std::vector<double>& X,const std::vector<double> Y,
                   double& chisq);
   bool fit_coeffs(const std::vector<double>& X,const std::vector<double> Y,
                   const std::vector<double>& S,double& chisq);
   bool fit_coeffs(unsigned int ndata,double x[],double y[]);
   bool fit_coeffs(unsigned int ndata,double x[],double y[],double& chisq);
   bool fit_coeffs(unsigned int ndata,double x[],double y[],double sigma[]);
   bool fit_coeffs(unsigned int ndata,double x[],double y[],double sigma[],
                   double& chisq);
   bool fit_coeffs_with_curvature_penalty(
      unsigned int ndata,double x[],double y[],
      double curvature_penalty_term_weight,double characteristic_length,
      double xstart,double xstop,double dx,double& chisq);

   std::vector<double> compute_residuals(
      const std::vector<double>& X,const std::vector<double> Y);
   void compute_residuals(
      unsigned int ndata,double x[],double y[],double residual[],double logresidual[]);

   bool fit_coeffs_using_residuals(
      const std::vector<double>& X,const std::vector<double> Y,double& chisq);
   bool fit_coeffs_using_residuals(unsigned int ndata,double x[],double y[]);
   bool fit_coeffs_using_residuals(
      unsigned int ndata,double x[],double y[],double& chisq);
   bool fit_coeffs_using_residuals(
      unsigned int ndata,double x[],double y[],linkedlist& fitlist);
   bool fit_coeffs_using_residuals(
      unsigned int ndata,double x[],double y[],linkedlist& fitlist,double& chisq);
   bool fit_coeffs_using_residuals(
      unsigned int ndata,double x[],int row,genarray& y);
   bool fit_coeffs_using_residuals(
      unsigned int ndata,double x[],int row,genarray& y,linkedlist& fitlist);

// Simultaneous least squares fitting to a function and its first derivative:

   bool fit_coeffs(
      unsigned int ndata,double x[],double y[],double yprime[],
      double inverse_sigma[],double inverse_sigma_prime[],double& chisq);
   bool fit_coeffs(
      unsigned int ndata,double x[],double y[],double yprime[],
      double inverse_sigma[],double inverse_sigma_prime[],
      double curvature_penalty_term_weight,double characteristic_length,
      double xstart,double xstop,double dx,double& chisq);
   bool fit_coeffs( 
      unsigned int ndata,double x[],double y[],double yprime[], 
      double inverse_sigma[],double inverse_sigma_prime[], 
      double curvature_penalty_term_weight,
      double extremal_value_penalty_term_weight,
      double yceiling,double yfloor,double ydot_ceiling,double ydot_floor,
      double characteristic_length,double xstart,double xstop,double dx,
      double& chisq);
   bool fit_coeffs(
      unsigned int ndata,double x[],int row,genarray& y,genarray& yprime,
      double inverse_sigma[],double inverse_sigma_prime[],double& chisq);
   bool fit_coeffs(
      unsigned int ndata,double x[],int row,genarray& y,genarray& yprime,
      double inverse_sigma[],double inverse_sigma_prime[],
      double curvature_penalty_term_weight,double characteristic_length,
      double xstart,double xstop,double dx,double& chisq);
   bool fit_coeffs(
      unsigned int ndata,double x[],int row,genarray& y,genarray& yprime,
      double inverse_sigma[],double inverse_sigma_prime[],
      double curvature_penalty_term_weight,
      double extremal_value_penalty_term_weight,
      double yceiling,double yfloor,double ydot_ceiling,double ydot_floor,
      double characteristic_length,double xstart,double xstop,double dx,
      double& chisq);

// Ushomirsky's generalizations to simultaneous least squares fitting
// methods which incorporate off-diagonal contributions to the
// covariance matrix:

   bool fit_coeffs(
      unsigned int ndata,double x[],double y[],double yprime[],
      double inverse_covar[],double inverse_covar_prime[],
      double inverse_covar_cross[],double curvature_penalty_term_weight,
      double characteristic_length,double xstart,double xstop,double dx,
      double& chisq);
   bool fit_coeffs( 
      unsigned int ndata,double x[],double y[],double yprime[], 
      double inverse_covar[],double inverse_covar_prime[],
      double inverse_covar_cross[],double curvature_penalty_term_weight,
      double extremal_value_penalty_term_weight,
      double yceiling,double yfloor,double yprime_ceiling,double yprime_floor,
      double characteristic_length,double xstart,double xstop,double dx,
      double& chisq);
   bool fit_coeffs(
      unsigned int ndata,double x[],int row,genarray& y,genarray& yprime,
      double inverse_covar[],double inverse_covar_prime[], 
      double inverse_covar_cross[],double& chisq);
   bool fit_coeffs(
      unsigned int ndata,double x[],int row,genarray& y,genarray& yprime,
      double inverse_covar[],double inverse_covar_prime[],
      double inverse_covar_cross[],double curvature_penalty_term_weight,
      double characteristic_length,double xstart,double xstop,double dx,
      double& chisq);
   bool fit_coeffs(
      unsigned int ndata,double x[],int row,genarray& y,genarray& yprime,
      double inverse_covar[],double inverse_covar_prime[], 
      double inverse_covar_cross[],double curvature_penalty_term_weight,
      double extremal_value_penalty_term_weight,
      double yceiling,double yfloor,double ydot_ceiling,double ydot_floor,
      double characteristic_length,double xstart,double xstop,double dx,
      double& chisq);

// Polynomial uncertainty estimation methods:

   double sigma2_value(double x) const;
   double sigma2_value(double x,double mu,double sigma) const;
   double sigma2_derivative(double x,int deriv_order) const;
   double sigma2_derivative(double x,double mu,double sigma,int deriv_order)
      const;

// Linked list generation methods:

   void generate_polynomial_list(
      unsigned int nbins,double xstart,double xstop,colorfunc::Color color,
      linkedlist& polylist);
   void generate_polynomial_list(
      unsigned int nbins,double xstart,double xstop,double mu,double sigma,
      colorfunc::Color color,linkedlist& polylist);
   void generate_polyderiv_list(
      unsigned int nbins,int deriv_order,double xstart,double xstop,
      colorfunc::Color color,linkedlist& polylist);
   void generate_polyderiv_list(
      unsigned int nbins,int deriv_order,double xstart,double xstop,
      double mu,double sigma,colorfunc::Color color,linkedlist& polylist,
      double error_multiple=1);

// Overloading elementary arithmetic operations:

   void operator+= (const mypolynomial& p);
   void operator-= (const mypolynomial& p);
   void operator*= (double c);
   void operator/= (double c);
   
   friend mypolynomial operator+ (
      const mypolynomial& p,const mypolynomial& q);
   friend mypolynomial operator- (const mypolynomial& p);
   friend mypolynomial operator- (
      const mypolynomial& p,const mypolynomial& q);
   friend mypolynomial operator* (double c,const mypolynomial& p);
   friend mypolynomial operator* (const mypolynomial& p,double c);
   friend mypolynomial operator/ (const mypolynomial& p,double c);

  private: 

   static const int max_polyorder;

   Basis basis;
   unsigned int order;
   double *a;			// Polynomial coefficient array
   genmatrix* covmatrix_ptr;	// Polynomial coefficient covariance matrix

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const mypolynomial& p);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void mypolynomial::allocate_member_objects()
{
   new_clear_array(a,max_polyorder);
   covmatrix_ptr=new genmatrix(max_polyorder,max_polyorder);
}		       

inline void mypolynomial::initialize_member_objects()
{
   order=0;
   basis=power;
}

inline void mypolynomial::clear_coeffs()
{
   for (unsigned int i=0; i<=order; i++) a[i]=0;
}

// Set and get member functions:

inline void mypolynomial::set_basis(Basis b)
{
   if (b==chebyshev || b==hermite || b==harmonic_osc || b==power) 
   {
      basis=b;
   }
   else
   {
      std::cout << "Error inside mypolynomial::set_basis()!" << std::endl;
      std::cout << "basis = " << get_basis_str(b)
                << " does not correspond to a legitimate basis choice" 
                << std::endl;
      std::cout << "Setting basis = power" << std::endl;
      basis=power;
   }
}

inline void mypolynomial::set_order(int o)
{
   if (o >= 0)
   {
      order=o;
   }
}

inline void mypolynomial::set_coeff(
   unsigned int coeff_index,double coeff_value)
{
   if (coeff_index <= order)
   {
      a[coeff_index]=coeff_value;
   }
}

inline void mypolynomial::set_coeffs(double coeff_values[])
{
   for (unsigned int i=0; i<= order; i++)
   {
      a[i]=coeff_values[i];
   }
}

inline mypolynomial::Basis mypolynomial::get_basis() const
{
   return basis;
}

inline std::string mypolynomial::get_basis_str(Basis basis) const
{
   if (basis==chebyshev)
   {
      return "chebyshev";
   }
   else if (basis==hermite)
   {
      return "hermite";
   }
   else if (basis==harmonic_osc)
   {
      return "harmonic_osc";
   }
   else if (basis==power)
   {
      return "power";
   }
   else
   {
      return "power";
   }
}

inline unsigned int mypolynomial::get_order() const
{
   return order;
}

inline double mypolynomial::get_coeff(int coeff_index) const
{
   return a[coeff_index];
}

// Member function value returns the value of the polynomial whose
// coefficients are contained within the order+1 dimension array a[]
// at the point x:

inline double mypolynomial::value(double x) const
{
   return value(x,0,1);
}

// ---------------------------------------------------------------------
// Member function derivative returns the value of the deriv_order
// derivative of ordinary polynomials whose coefficients are contained
// within the order+1 dimension array a[] at the point x.  It can also
// compute the 1st and 2nd derivatives of polynomials with are
// Chebyshev, Hermite and quantum harmonic oscillator basis functions.
// If deriv_order==0, the value of the polynomial evaluated at x is
// returned by this method.

inline double mypolynomial::derivative(double x,int deriv_order) const
{
   return derivative(x,0,1,deriv_order);
}

// ---------------------------------------------------------------------
// Member function power_value is a specialized method intended to
// evaluate power basis polynomials as quickly as possible.

inline double mypolynomial::power_value(double x) const
{
   double p=0;
   for (unsigned int i=0; i<order; i++)
   {
      p += a[order-i];
      p *= x;
   }
   p += a[0];
   return p;
}

// ---------------------------------------------------------------------
// Member function basis_power returns the value of ordinary
// polynomial basis function x**poly_order:

inline double mypolynomial::basis_power(double x,int poly_order) const
{
   return mathfunc::real_power(x,poly_order);
}

// ---------------------------------------------------------------------
// Member function deriv_power returns the deriv_order-th derivative
// of ordinary polynomial basis function x**poly_order:

inline double mypolynomial::deriv_power(
   double x,int deriv_order,int poly_order) const
{
   return mathfunc::choose_numerator(poly_order,deriv_order)*
      mathfunc::real_power(x,poly_order-deriv_order);
}

// ---------------------------------------------------------------------
// Member function fit_coeffs sets the weights of all the input data
// points to unity and then calls the next version of fit_coeffs:

inline bool mypolynomial::fit_coeffs(unsigned int ndata,double x[],double y[])
{
   double chisq;
   return fit_coeffs(ndata,x,y,chisq);
}

// ---------------------------------------------------------------------
// Member function fit_coeffs computes the coefficients of a
// Chebyshev, Hermite, quantum harmonic oscillator energy
// eigenfunction or ordinary polynomial which fits ndata (x,y) points
// contained in arrays x and y with errors stored in array sigma.  It
// uses the Numerical Recipes least squares fitting routine.  As of
// 11/28/01, we suspect that this lfit routine is perhaps more
// reliable than the Numerical Recipes singular value decomposition
// subroutine!  Recall that the number of input data points must
// exceed the order of the polynomial in order for the fit to be
// meaningful.  This boolean method returns false if this last
// condition is not satisfied.

inline bool mypolynomial::fit_coeffs(
   unsigned int ndata,double x[],double y[],double sigma[])
{
   double chisq;
   return fit_coeffs(ndata,x,y,sigma,chisq);
}

// ---------------------------------------------------------------------
// This variant of member function fit_coeffs initially sets the
// weights of all the input data points to unity and then computes an
// initial set of polynomial coefficients.  It next calculates the
// residuals between the original data point values and the initial
// polynomial fit.  This method then sets sigma equal to 1/2 for those
// points whose residuals lie within the lowest third.  Similarly, it
// sets sigma = 2 for those points whose residuals lie within the top
// third.  Fit_coeffs_using_residuals then fits the polynomial
// coefficient values once again with these modified sigma values.

inline bool mypolynomial::fit_coeffs_using_residuals(
   unsigned int ndata,double x[],double y[])
{
   double chisq;
   linkedlist fitlist;
   return fit_coeffs_using_residuals(ndata,x,y,fitlist,chisq);
}

inline bool mypolynomial::fit_coeffs_using_residuals(
   unsigned int ndata,double x[],double y[],double& chisq)
{
   linkedlist fitlist;
   return fit_coeffs_using_residuals(ndata,x,y,fitlist,chisq);
}

inline bool mypolynomial::fit_coeffs_using_residuals(
   unsigned int ndata,double x[],double y[],linkedlist& fitlist)
{
   double chisq;
   return fit_coeffs_using_residuals(ndata,x,y,fitlist,chisq);
}

// ---------------------------------------------------------------------
// This overloaded version of member function
// fit_coeffs_using_residuals takes in a genarray y along with a
// particular row number within that array.  It returns a polynomial
// fit to the data stored within that one particular row:

inline bool mypolynomial::fit_coeffs_using_residuals(
   unsigned int ndata,double x[],int row,genarray& y)
{
   linkedlist fitlist;
   return fit_coeffs_using_residuals(ndata,x,row,y,fitlist);
}

// ---------------------------------------------------------------------
inline bool mypolynomial::fit_coeffs(
   unsigned int ndata,double x[],double y[],double yprime[],
   double inverse_sigma[],double inverse_sigma_prime[],double& chisq)
{
   return fit_coeffs(ndata,x,y,yprime,inverse_sigma,inverse_sigma_prime,
                     0,0,0,1,1,chisq);
}

inline bool mypolynomial::fit_coeffs(
   unsigned int ndata,double x[],int row,genarray& y,genarray& yprime,
   double inverse_covar[],double inverse_covar_prime[],
   double inverse_covar_cross[],double& chisq)
{
   return fit_coeffs(ndata,x,row,y,yprime,inverse_covar,
                     inverse_covar_prime,inverse_covar_cross,0,0,0,1,1,chisq);
}

// ---------------------------------------------------------------------
inline double mypolynomial::sigma2_value(double x) const
{
   return sigma2_value(x,0,1);
}

// ---------------------------------------------------------------------
// Member function generate_polynomial_list returns a linkedlist
// containing the current polynomial object's values between x=xstart
// and x=xstop in the func[0] member of each node.  Positive and
// negative polynomial error bounds based upon coefficient covariance
// matrix information are also saved within the func[1] and func[2]
// members of each node.  However, this uncertainty information is
// only meaningful if the *covmatrix_ptr has previously been correctly
// calculated!

inline void mypolynomial::generate_polynomial_list(
   unsigned int nbins,double xstart,double xstop,colorfunc::Color color,
   linkedlist& polylist)
{
   generate_polynomial_list(nbins,xstart,xstop,0,1,color,polylist);
}

// This overloaded version of member function generate_polynomial_list
// generates a polynomial whose independent variable y is shifted by
// mu and rescaled by 1/sigma relative to x:  y = (x-mu)/sigma.  

inline void mypolynomial::generate_polynomial_list(
   unsigned int nbins,double xstart,double xstop,double mu,double sigma,
   colorfunc::Color color,linkedlist& polylist)
{
   generate_polyderiv_list(nbins,0,xstart,xstop,mu,sigma,color,polylist);
}

// ---------------------------------------------------------------------
// Member function generate_polyderiv_list returns a linkedlist
// containing the current polynomial object's deriv_order derivative
// values between x=xstart and x=xstop in the func[0] member of each
// node.  Positive and negative error bounds based upon coefficient
// covariance matrix information are also saved into the func[1] and
// func[2] members of each node.  However, this uncertainty
// information is only meaningful if the *covmatrix_ptr has previously
// been correctly calculated!

inline void mypolynomial::generate_polyderiv_list(
   unsigned int nbins,int deriv_order,double xstart,double xstop,
   colorfunc::Color color,linkedlist& polylist)
{
   generate_polyderiv_list(nbins,deriv_order,xstart,xstop,0,1,color,polylist);
}

// ---------------------------------------------------------------------
inline void mypolynomial::operator+= (const mypolynomial& p)
{
   for (unsigned int i=0; i<=order; i++) a[i] += p.get_coeff(i);
}

inline void mypolynomial::operator-= (const mypolynomial& p)
{
   for (unsigned int i=0; i<=order; i++) a[i] -= p.get_coeff(i);
}

inline void mypolynomial::operator*= (double c)
{
   for (unsigned int i=0; i<=order; i++) a[i] *= c;
}

inline mypolynomial operator- (const mypolynomial& p,const mypolynomial& q)
{
   return p+(-q);
}

inline mypolynomial operator* (const mypolynomial& p,double c)
{
   return c*p;
}

inline mypolynomial operator/ (const mypolynomial& p,double c)
{
   return (1.0/c)*p;
}

# endif // math/mypolynomial.h











