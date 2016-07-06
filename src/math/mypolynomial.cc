// Note added at 17:51 on Dec 29, 2003: Excruciatingly ugly hack added
// within mypolynomial::fit_coeffs_using_residuals().  Replace this
// extreme ugliness with STL vector in very near future!!!

// ==========================================================================
// Polynomial class member function definitions
// ==========================================================================
// Last modified on 8/20/11; 3/23/12; 3/31/12; 3/29/14
// ==========================================================================

#include "datastructures/datapoint.h"
#include "math/Genarray.h"
#include "math/genvector.h"
#include "math/mathfuncs.h"
#include "datastructures/Mynode.h"
#include "math/mypolynomial.h"
#include "templates/mytemplates.h"
#include "numrec/nr.h"
#include "numrec/nrutil.h"
#include "math/prob_distribution.h"

using std::cout;
using std::endl;
using std::ios;
using std::ostream;
using std::string;
using std::vector;

const int mypolynomial::max_polyorder=50;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

mypolynomial::mypolynomial(void)
{
   allocate_member_objects();
   initialize_member_objects();
}

// Copy constructor:

mypolynomial::mypolynomial(const mypolynomial& p)
{
   allocate_member_objects();
   initialize_member_objects();
   docopy(p);
}

mypolynomial::mypolynomial(int polyorder)
{
   allocate_member_objects();
   initialize_member_objects();
   order=polyorder;
}

mypolynomial::mypolynomial(int polyorder,double coeff[])
{
   allocate_member_objects();
   initialize_member_objects();
   order=polyorder;

   for (unsigned int i=0; i<=order; i++) a[i]=coeff[i];
}

mypolynomial::mypolynomial(int polyorder,const vector<double>& coeff)
{
   allocate_member_objects();
   initialize_member_objects();
   order=polyorder;

   for (unsigned int i=0; i<=order; i++) a[i]=coeff[i];
}

mypolynomial::~mypolynomial()
{
   delete [] a;
   delete covmatrix_ptr;
   a=NULL;
   covmatrix_ptr=NULL;
}

// ---------------------------------------------------------------------
void mypolynomial::docopy(const mypolynomial & p)
{
   basis=p.basis;
   order=p.order;
   for (unsigned int i=0; i<=order; i++) a[i]=p.a[i];
   *covmatrix_ptr=*(p.covmatrix_ptr);
}
   
// Overload = operator:

mypolynomial& mypolynomial::operator= (const mypolynomial& p)
{
   if (this==&p) return *this;
   docopy(p);
   return *this;
}

// Overload << operator:

ostream& operator<< (ostream& outstream,const mypolynomial& p)
{
   outstream.precision(10);
   outstream.setf(ios::scientific);

   outstream << endl;
   outstream << p.get_basis_str(p.basis)+" polynomial basis" << endl;
   outstream << "Polynomial order = " << p.order << endl;
   if (p.order < 21)
   {
      for (unsigned int i=0; i<=p.order; i++)
      {
         outstream << "i = " << i << " a[i] = " << p.a[i] << endl;
      }
   }
   
//   outstream << "Coefficient covariance matrix = " 
//             << *(p.covmatrix_ptr) << endl;

   outstream << endl;
   return outstream;
}

// ==========================================================================
// Polynomial evaluation methods
// ==========================================================================

// This overloaded version of member function value generates a
// polynomial whose independent variable y is shifted by mu and
// rescaled by 1/sigma relative to x: y = (x-mu)/sigma.

double mypolynomial::value(double x,double mu,double sigma) const
{
   double y=(x-mu)/sigma;
   double p=0;

   for (unsigned int i=0; i<=order; i++)
   {
      p += a[i]*basis_function(y,i);
   }
   return p;
}

// ---------------------------------------------------------------------
// This overloaded version of member function derivative computes the
// current polynomial object's deriv_order derivative after its
// independent variable is shifted by mu and rescaled by 1/sigma:

double mypolynomial::derivative(
   double x,double mu,double sigma,int deriv_order) const
{
   double y=(x-mu)/sigma;
   double poly_derivative=0;

   if (deriv_order==0) 
   {
      return value(y);
   }
   else
   {
      for (unsigned int i=0; i<=order; i++)
      {
         poly_derivative += a[i]*deriv_basis_function(y,deriv_order,i);
      }
      return poly_derivative/mathfunc::real_power(sigma,deriv_order);
   } // deriv_order==0 conditional
}

// ==========================================================================
// Basis function methods
// ==========================================================================

// Member function basis_function returns the value poly_order basis
// function at point x specified by member variable basis:

double mypolynomial::basis_function(double x,int poly_order) const
{
   if (basis==harmonic_osc)
   {
      return basis_harmonic_osc(x,poly_order);
   }
   else if (basis==hermite)
   {
      return basis_hermite(x,poly_order);
   }
   else if (basis==chebyshev)
   {
      return basis_chebyshev(x,poly_order);
   }
   else // default basis = power 
   {
      return basis_power(x,poly_order);
   }
}

// ---------------------------------------------------------------------
// Member function deriv_basis_function returns the value deriv_order
// derivative of poly_order basis function at point x specified by
// member variable basis:

double mypolynomial::deriv_basis_function(
   double x,int deriv_order,int poly_order) const
{
   if (basis==harmonic_osc)
   {
      return deriv_harmonic_osc(x,deriv_order,poly_order);
   }
   else if (basis==hermite)
   {
      return deriv_hermite(x,deriv_order,poly_order);
   }
   else if (basis==chebyshev)
   {
      return deriv_chebyshev(x,deriv_order,poly_order);
   }
   else // default basis=power
   {
      return deriv_power(x,deriv_order,poly_order);
   }
}

// ---------------------------------------------------------------------
// Member function basis_chebyshev recursively calculates the value of
// the j=poly_order basis Chebyshev polynomial T_j(x):

double mypolynomial::basis_chebyshev(double x,int poly_order) const
{
   double T;

   if (poly_order==0)
   {
      T=1;
   }
   else if (poly_order==1)
   {
      T=x;
   }
   else
   {
      T=2*x*basis_chebyshev(x,poly_order-1)-basis_chebyshev(x,poly_order-2);
   }
   return T;
}

// ---------------------------------------------------------------------
// Member function deriv_chebyshev recursively calculates the
// deriv_order-th derivative of T_j(x) where j=poly_order:

double mypolynomial::deriv_chebyshev(double x,int deriv_order,int poly_order) const
{
   if (deriv_order==1)
   {
      double Tprime;
      if (poly_order==0)
      {
         Tprime=0;
      }
      else if (poly_order==1)
      {
         Tprime=1;
      }
      else
      {
         Tprime=2*basis_chebyshev(x,poly_order-1)
            +2*x*deriv_chebyshev(x,deriv_order,poly_order-1)
            -deriv_chebyshev(x,deriv_order,poly_order-2);
      }
      return Tprime;
   }
   else if (deriv_order >= 2)
   {
      double Tderiv_order_prime;
      if (poly_order==0 || poly_order==1)
      {
         Tderiv_order_prime=0;
      }
      else
      {
         Tderiv_order_prime=2*deriv_order*deriv_chebyshev(
            x,deriv_order-1,poly_order-1)+
            2*x*deriv_chebyshev(x,deriv_order,poly_order-1)
            -deriv_chebyshev(x,deriv_order,poly_order-2);
      }
      return Tderiv_order_prime;
   }
   else
   {
      return NEGATIVEINFINITY;
   }
}

// ---------------------------------------------------------------------
// Member function basis_hermite recursively calculates the value of
// the j=poly_order basis Hermite polynomial H_j(x):

// In order to speed up processing, we hard-wire in the explicit
// expressions for the first 13 Hermite polynomials:

double mypolynomial::basis_hermite(double x,int poly_order) const
{
   double H,xsqr,xfourth,xsixth,xeighth,xtenth,xtwelveth;

   if (poly_order < 0)
   {
      H=0;
   }
   else
   {
      switch(poly_order)
      {
         case 0:
            H=1;
            break;
         case 1: 
            H=2*x;
            break;
         case 2:
            H=2*(2*x*x-1);
            break;
         case 3:
            H=4*x*(2*x*x-3);
            break;
         case 4:
            xsqr=x*x;
            xfourth=xsqr*xsqr;
            H=4*(4*xfourth-12*xsqr+3);
            break;
         case 5:
            xsqr=x*x;
            xfourth=xsqr*xsqr;
            H=8*x*(4*xfourth-20*xsqr+15);
            break;
         case 6:
            xsqr=x*x;
            xfourth=xsqr*xsqr;
            xsixth=xsqr*xfourth;
            H=8*(8*xsixth-60*xfourth+90*xsqr-15);
            break;
         case 7:
            xsqr=x*x;
            xfourth=xsqr*xsqr;
            xsixth=xsqr*xfourth;
            H=16*x*(8*xsixth-84*xfourth+210*xsqr-105);
            break;
         case 8:
            xsqr=x*x;
            xfourth=xsqr*xsqr;
            xsixth=xsqr*xfourth;
            xeighth=xsqr*xsixth;
            H=16*(16*xeighth-224*xsixth+840*xfourth-840*xsqr+105);
            break;
         case 9:
            xsqr=x*x;
            xfourth=xsqr*xsqr;
            xsixth=xsqr*xfourth;
            xeighth=xsqr*xsixth;
            H=32*x*(16*xeighth-288*xsixth+1512*xfourth-2520*xsqr+945);
            break;
         case 10:
            xsqr=x*x;
            xfourth=xsqr*xsqr;
            xsixth=xsqr*xfourth;
            xeighth=xsqr*xsixth;
            xtenth=xsqr*xeighth;
            H=32*(32*xtenth-720*xeighth+5040*xsixth-12600*xfourth+
                  9450*xsqr-945);
            break;
         case 11:
            xsqr=x*x;
            xfourth=xsqr*xsqr;
            xsixth=xsqr*xfourth;
            xeighth=xsqr*xsixth;
            xtenth=xsqr*xeighth;
            H=64*x*(32*xtenth-880*xeighth+7920*xsixth-27720*xfourth
                    +34650*xsqr-10395);
            break;
         case 12:
            xsqr=x*x;
            xfourth=xsqr*xsqr;
            xsixth=xsqr*xfourth;
            xeighth=xsqr*xsixth;
            xtenth=xsqr*xeighth;
            xtwelveth=xsqr*xtenth;
            H=64*(64*xtwelveth-2112*xtenth+23760*xeighth-110880*xsixth
                  +207900*xfourth-124740*xsqr+10395);
            break;
         default:
            H=2*x*basis_hermite(x,poly_order-1)-
               2*(poly_order-1)*basis_hermite(x,poly_order-2);
            break;
      } // poly_order switch statement
   } // poly_order < 0 conditional
   return H;
}

// ---------------------------------------------------------------------
// Member function deriv_hermite recursively calculates the
// deriv_order-th derivative of H_j(x) where j=poly_order.  

// Note:  H'_j = 2 j H_(j-1)
//	  H''_j = 4 j (j-1) H_(j-2)
//	  H^(m)_j = pow(2,m) m! (j choose m) H_(j-m)

double mypolynomial::deriv_hermite(double x,int deriv_order,int poly_order) 
   const
{
   if (deriv_order==1)
   {
      return 2*poly_order*basis_hermite(x,poly_order-1);
   }
   else if (deriv_order==2)
   {
      return 4*poly_order*(poly_order-1)*basis_hermite(x,poly_order-2);
   }
   else 
   {
      return mathfunc::power_of_two(deriv_order)*
         mathfunc::choose_numerator(poly_order,deriv_order)*
         basis_hermite(x,poly_order-deriv_order);
   }
}

// ---------------------------------------------------------------------
// Member function basis_harmonic_osc recursively calculates the value
// of the j=poly_order basis quantum harmonic oscillator energy
// eigenfunction phi_j(y; mu=0; sigma=1). 

// Note: y = (x-mu)/sigma.

double mypolynomial::basis_harmonic_osc(double y,int poly_order) const
{
//   const double numer=0.751125544; // = (1/PI)**0.25
   double prefactor=0.751125544/
      sqrt(mathfunc::power_of_two(poly_order)*
           mathfunc::factorial(poly_order));
   double gaussian=exp(-0.5*y*y);
   return prefactor*gaussian*basis_hermite(y,poly_order); // = phi
}

// ---------------------------------------------------------------------
// Member function deriv_harmonic_osc calculates the value of 
// d/dy [ phi_n(y; mu=0; sigma=1] or d^2/dy^2 [ phi_n(y; mu=0; sigma=1) ]:

// Note: y = (x-mu)/sigma.

double mypolynomial::deriv_harmonic_osc(
   double y,int deriv_order,int poly_order) const
{
//   const double numer=0.751125544; // = (1/PI)**0.25
   double prefactor=0.751125544/
      sqrt(mathfunc::power_of_two(poly_order)*
           mathfunc::factorial(poly_order));
   double gaussian=exp(-0.5*y*y);

   if (deriv_order==1)
   {
      return -y*basis_harmonic_osc(y,poly_order)
         +prefactor*gaussian*deriv_hermite(y,deriv_order,poly_order); 
						// = phi_prime
   }
   else if (deriv_order==2)
   {
      double term1=-(basis_harmonic_osc(y,poly_order)+
                     y*deriv_harmonic_osc(y,1,poly_order));
      double term2=deriv_hermite(y,2,poly_order)-
         y*deriv_hermite(y,1,poly_order);
      return term1+prefactor*gaussian*term2; // = phi_primeprime
   }
   else
   {
      cout << "Error inside  mypolynomial::deriv_harmonic_osc()" << endl;
      cout << "deriv_order = " << deriv_order << endl;
      cout << "As of Oct 02, only the 1st and 2nd derivatives" << endl;
      cout << " of harmonic oscillator functions have been explicitly" 
           << endl;
      cout << " worked out..." << endl;
      return NEGATIVEINFINITY;
   }
}

// ==========================================================================
// Extrema location methods
// ==========================================================================

// Member function global_extremum performs an iterative search for
// the current polynomial object's maximum or minimum value within the
// interval set by input parameters xlo and xhi.  The current
// polynomial's independent parameter is shifted by mu and rescaled by
// 1/sigma prior to its being evaluated: y = (x-mu)/sigma.  The
// location of the global extremum is returned within x_extremum,
// while the value of the polynomial at the extremum point is returned
// by this member function.  If boolean flag find_min==true, the
// extremum corresponds to a global minimum.  Otherwise, the extremum
// corresponds to a global maximum.

double mypolynomial::global_extremum(
   bool find_min,double xlo,double xhi,double mu,double sigma,
   double& x_extremum)
{
   return global_extremum(find_min,0,xlo,xhi,mu,sigma,x_extremum);
}

double mypolynomial::global_extremum(
   bool find_min,int deriv_order,double xlo,double xhi,double mu,double sigma,
   double& x_extremum)
{
   const unsigned int nbins=1000;
   const unsigned int niters=10;

   double x,y,dx,xmax=0;
   double p,pmax=NEGATIVEINFINITY;

   double xstart=xlo;
   double xstop=xhi;

   for (unsigned int n=0; n<niters; n++)
   {
      dx=(xstop-xstart)/(nbins-1);
      for (unsigned int i=0; i<nbins; i++)
      {
         x=xlo+i*dx;
         y=(x-mu)/sigma;

         p=derivative(y,deriv_order);
         if (find_min) p *= -1;
         
         if (p > pmax)
         {
            pmax=p;
            xmax=x;
         }
      } // loop over index i labeling x location
      xstart=xmax-100*dx;
      xstop=xmax+100*dx;
   } // loop over refinement iteration index n 
   
   x_extremum=xmax;
   if (find_min)
   {
      return -pmax;
   }
   else
   {
      return pmax;
   }
}

// ---------------------------------------------------------------------
// Member function local_extremum uses Newton's method to locate either a
// maximum or minimum point within the vicinity of point x of a
// polynomial object whose order+1 coefficients are stored in array a:

double mypolynomial::local_extremum(double x)
{
   const int max_iters=10;
   const double min_frac_change=0.001;
   
   double frac_change=POSITIVEINFINITY;
   double xnew;
   int iter=0;
   do
   {
      double p1=derivative(x,1);
      double p2=derivative(x,2);
      xnew=x-p1/p2;
      iter++;
      if (x != 0) frac_change=fabs((xnew-x)/x);
      x=xnew;
   }
   while (frac_change > min_frac_change && iter < max_iters);
   return xnew;
}

// ==========================================================================
// Polynomial root member functions
// ==========================================================================

// Member function root uses Newton's method to locate a root within
// the vicinity of point x of a polynomial whose order+1 coefficients
// are stored within array a:

double mypolynomial::root(double x)
{
   const int max_iters=10;
   const double min_frac_change=0.001;
   
   double frac_change=POSITIVEINFINITY;
   double xnew;
   int iter=0;
   do
   {
      double p0=value(x);
      double p1=derivative(x,1);
      xnew=x-p0/p1;
      iter++;
      if (x != 0) frac_change=fabs((xnew-x)/x);
      x=xnew;
   }
   while (frac_change > min_frac_change && iter < max_iters);
   return xnew;
}

// ---------------------------------------------------------------------
// Member function construct companion_matrix() generates a matrix
// whose characteristic polynomial equals *this.  The eigenvalues of
// this companion matrix equal the roots of *this.

genmatrix* mypolynomial::construct_companion_matrix() const
{
//   cout << "inside mypolynomial::construct_companion_matrix()" << endl;
//   cout << "order = " << order << endl;
   
   genmatrix* companion_matrix_ptr=new genmatrix(order,order);
   companion_matrix_ptr->clear_values();
   for (unsigned int i=0; i<order; i++)
   {
      double renorm_coeff=get_coeff(i)/get_coeff(order);
//      cout << "i = " << i << " renorm_coeff = " << renorm_coeff << endl;
      companion_matrix_ptr->put(i,order-1,-renorm_coeff);
   }
   for (unsigned int i=1; i< order; i++)
   {
      companion_matrix_ptr->put(i,i-1,1);
   }
   
//   cout << "companion_matrix = " << *companion_matrix_ptr << endl;
//   outputfunc::enter_continue_char();
   return companion_matrix_ptr;
}

// ==========================================================================
// Least squares fitting to a function
// ==========================================================================

bool mypolynomial::fit_coeffs(const vector<double>& X,const vector<double> Y,
                              double& chisq)
{
   const unsigned int ndata=X.size();
   double x[ndata],y[ndata],sigma[ndata];
   for (unsigned int i=0; i<ndata; i++) 
   {
      x[i]=X[i];
      y[i]=Y[i];
      sigma[i]=1;
   }
   return fit_coeffs(ndata,x,y,sigma,chisq);
}

// ---------------------------------------------------------------------
bool mypolynomial::fit_coeffs(const vector<double>& X,const vector<double> Y,
                              const vector<double>& S,double& chisq)
{
   const unsigned int ndata=X.size();
   double x[ndata],y[ndata],sigma[ndata];
   for (unsigned int i=0; i<ndata; i++) 
   {
      x[i]=X[i];
      y[i]=Y[i];
      sigma[i]=S[i];
   }
   return fit_coeffs(ndata,x,y,sigma,chisq);
}

// ---------------------------------------------------------------------
bool mypolynomial::fit_coeffs(
   unsigned int ndata,double x[],double y[],double& chisq)
{
   double sigma[ndata];
   for (unsigned int i=0; i<ndata; i++) sigma[i]=1;
   return fit_coeffs(ndata,x,y,sigma,chisq);
}


// ---------------------------------------------------------------------
bool mypolynomial::fit_coeffs(
   unsigned int ndata,double x[],double y[],double sigma[],double& chisq)
{
   bool fit_successfully_calculated=true;
   
   if (ndata <= order)
   {
      fit_successfully_calculated=false;
   }
   else
   {
      const int ma=order+1;

      int *ia;
      double *acoeff;
      double **covar;
      ia=numrec::ivector(1,ma);
      acoeff=numrec::vector(1,ma);
      covar=numrec::matrix(1,ma,1,ma);

      for (int i=1; i<=ma; i++)
      {
         ia[i]=1;
      }

      if (basis==harmonic_osc)
      {
         numrec::lfit(x-1,y-1,sigma-1,ndata,acoeff,ia,ma,covar,&chisq,
                      &numrec::numrec_harmonic_osc);
      }
      else if (basis==hermite)
      {
         numrec::lfit(x-1,y-1,sigma-1,ndata,acoeff,ia,ma,covar,
                      &chisq,&numrec::numrec_hermite);
      }
      else if (basis==power)
      {
         numrec::lfit(x-1,y-1,sigma-1,ndata,acoeff,ia,ma,covar,
                      &chisq,&numrec::numrec_poly);
      }
      else if (basis==chebyshev)
      {
         numrec::lfit(x-1,y-1,sigma-1,ndata,acoeff,ia,ma,covar,
                      &chisq,&numrec::numrec_cheby);
      }
   
      for (unsigned int i=0; i<=order; i++)
      {
         a[i]=acoeff[i+1];
//      cout << "i = " << i << " a[i] = " << a[i] << endl;
      }
//   cout << "chisq = " << chisq << endl;

      numrec::free_ivector(ia,1,ma);
      numrec::free_vector(acoeff,1,ma);
      numrec::free_matrix(covar,1,ma,1,ma);
   } // ndata <= order conditional
   return fit_successfully_calculated;
}

// ---------------------------------------------------------------------
bool mypolynomial::fit_coeffs_with_curvature_penalty(
   unsigned int ndata,double x[],double y[],
   double curvature_penalty_term_weight,double characteristic_length,
   double xstart,double xstop,double dx,double& chisq)
{
   double yprime[ndata],inverse_sigma[ndata],inverse_sigma_prime[ndata];
   for (unsigned int i=0; i<ndata; i++)
   {
      yprime[i]=inverse_sigma_prime[i]=0;
      inverse_sigma[i]=1;
   }
   return fit_coeffs(ndata,x,y,yprime,inverse_sigma,inverse_sigma_prime,
                     curvature_penalty_term_weight,characteristic_length,
                     xstart,xstop,dx,chisq);
}

// ---------------------------------------------------------------------
// Member function compute_residuals returns an array containing
// absolute differences between raw data point values and their
// polynomial fitted values.  It also returns another array containing
// the logarithm of these absolute differences.

vector<double> mypolynomial::compute_residuals(
   const vector<double>& X,const vector<double> Y)
{
   vector<double> residual;
   for (unsigned int i=0; i<X.size(); i++)
   {
      residual.push_back(Y[i]-value(X[i]));
   }
   return residual;
}

void mypolynomial::compute_residuals(
   unsigned int ndata,double x[],double y[],double residual[],
   double logresidual[])
{
   for (unsigned int i=0; i<ndata; i++)
   {
      residual[i]=fabs(y[i]-value(x[i]));
      logresidual[i]=log10(residual[i]);
   }
}

// ---------------------------------------------------------------------
bool mypolynomial::fit_coeffs_using_residuals(
   unsigned int ndata,double x[],double y[],linkedlist& fitlist,double& chisq)
{
   bool fit_successfully_calculated=fit_coeffs(ndata,x,y);
   if (fit_successfully_calculated)
   {
      double residual[ndata];
      double logresidual[ndata];
      compute_residuals(ndata,x,y,residual,logresidual);

      prob_distribution prob(ndata,logresidual,2*ndata);
//   prob.cumulativefilenamestr="residual.meta";
//   prob.xlabel="Residual";
//     prob.writeprobdists();
//   double bottom_third=prob.find_x_corresponding_to_pcum(0.333);
//   double top_third=prob.find_x_corresponding_to_pcum(0.667);

      double x_40=prob.find_x_corresponding_to_pcum(0.4);
      double x_70=prob.find_x_corresponding_to_pcum(0.7);
      double x_90=prob.find_x_corresponding_to_pcum(0.9);

      double sigma[ndata];
      for (unsigned int i=0; i<ndata; i++)
      {
         if (logresidual[i] <= x_40)
         {
            sigma[i]=0.5;
         }
         else if (logresidual[i] > x_40 && logresidual[i] <= x_70)
         {
            sigma[i]=1.0;
         }
         else if (logresidual[i] > x_70 && logresidual[i] <= x_90)
         {
            sigma[i]=2.0;
         }
         else
         {
            sigma[i]=4.0;
         }
      }
      fit_successfully_calculated=fit_coeffs(ndata,x,y,sigma,chisq);

// Save residual values into 2nd function values inside fitlist's
// nodes:

      if (fitlist.size() > 0)
      {
         for (unsigned int i=0; i<ndata; i++)
         {

// EXTREME UGLY HACK:  
// REPLACE THE NEXT FEW LINES WITH STL VECTOR IMPLEMENTATION !!!

            mynode* currnode_ptr=fitlist.get_node(i);

            double tmp_func0=currnode_ptr->get_data().get_func(0);
            delete currnode_ptr->get_data().get_func();
            currnode_ptr->get_data().set_n_depend_vars(2);
            currnode_ptr->get_data().set_func(
               new double[fitlist.get_node(i)->get_data().
                         get_n_depend_vars()]);
            currnode_ptr->get_data().set_func(0,tmp_func0);
            currnode_ptr->get_data().set_func(1,sigma[i]);
         }
      }
   } // fit_successfully_calculated conditional
   return fit_successfully_calculated;
}

bool mypolynomial::fit_coeffs_using_residuals(
   unsigned int ndata,double x[],int row,genarray& y,linkedlist& fitlist)
{
   double curr_row[y.get_ndim()];
   y.get_row(row,curr_row);
   return fit_coeffs_using_residuals(ndata,x,curr_row,fitlist);
}

// ==========================================================================
// Simultaneous least squares fitting to a function and its first derivative
// ==========================================================================

// These next three overloaded versions of member function fit_coeffs
// minimize a chisq function which contains 2 independent terms that
// contain zeroth and first derivative information.  The polynomial is
// fitted so that its zeroth and first derivatives match as best as
// possible the ndata (x,y) points along with the ndata (x,yprime)
// points (which are assumed to be INDEPENDENT of (x,y)) contained in
// input arrays x[], y[] and yprime[].  Error information for each y
// and yprime point is passed into this method via arrays
// inverse_sigma[] and inverse_sigma_prime[] (i.e. their elements
// should contain 1/sigma_i and 1/sigma_prime_i !).  The chisq
// function also contains a 3rd term proportional to the polynomial's
// squared second derivative integrated from xstart <= x <= xstop.
// Inclusion of this "wiggly" term into the chisq function helps
// reduce excessive twisting and turning of the best polynomial fit.
// The relative strength of the "wiggly" term compared to the zeroth
// and first derivative terms in chisq is set by input weight p, and
// the characteristic length scale over which acceptable wiggles may
// occur is passed as input parameter characteristic_length.

// This method solves the matrix equation M A = V for vector A which
// contains the coefficients of a Chebyshev, Hermite, quantum harmonic
// oscillator energy eigenfunction or ordinary polynomial.  It also
// saves the coefficient covariance matrix Minverse into member
// genmatrix *covamatrix_ptr.

// See notes "Least square fitting to both function & derivative
// information (10/7/02)" and "Least squares fitting to both function
// and derivative info with curvature penalty (10/23/03)" for further
// discussion.

bool mypolynomial::fit_coeffs( 
   unsigned int ndata,double x[],double y[],double yprime[], 
   double inverse_sigma[],double inverse_sigma_prime[], 
   double curvature_penalty_term_weight,double characteristic_length,
   double xstart,double xstop,double dx,double& chisq) 
{ 
   double inverse_sigma_cross[ndata];
   for (unsigned int n=0; n<ndata; n++)
   {
      inverse_sigma_cross[n]=0;
   }
   return fit_coeffs(
      ndata,x,y,yprime,inverse_sigma,inverse_sigma_prime,inverse_sigma_cross,
      curvature_penalty_term_weight,characteristic_length,
      xstart,xstop,dx,chisq);
}

// This final overloaded version of fit_coeffs yields precisely the
// same polynomial coefficients as its immediate predecessor.
// However, it returns a chisq value which includes an error function
// penalty term that grows large if the polynomial zeroth or first
// derivatives stray outside the intervals yfloor <= y <= yceiling and
// yprime_floor <= yprime <= yprime_ceiling.  The relative strength of
// this penalty term compared to the zeroth and first derivative
// contributions to chisq along with the wiggly term is controlled by
// input weight q.  Since the penalty term is most definitely
// non-linear in the polynomial coefficients, we do not attempt to
// directly use it to influence the fit.  However, this method can be
// called for different values of some external parameters (such as
// gaussian mean mu and width sigma).  We then simply pick the values
// of those parameters for which this method yields the smallest value
// of chisq.

bool mypolynomial::fit_coeffs( 
   unsigned int ndata,double x[],double y[],double yprime[], 
   double inverse_sigma[],double inverse_sigma_prime[],
   double curvature_penalty_term_weight,
   double extremal_value_penalty_term_weight,
   double yceiling,double yfloor,double yprime_ceiling,double yprime_floor,
   double characteristic_length,double xstart,double xstop,double dx,
   double& chisq) 
{ 
   double inverse_sigma_cross[ndata];
   for (unsigned int n=0; n<ndata; n++)
   {
      inverse_sigma_cross[n]=0;
   }
   
   return fit_coeffs(
      ndata,x,y,yprime,inverse_sigma,inverse_sigma_prime,inverse_sigma_cross,
      curvature_penalty_term_weight,extremal_value_penalty_term_weight,
      yceiling,yfloor,yprime_ceiling,yprime_floor,
      characteristic_length,xstart,xstop,dx,chisq);
}

// ---------------------------------------------------------------------
// These overloaded versions of member function fit_coeffs take in
// genarrays y and yprime along with a particular row number
// associated with those arrays.  They return a polynomial fit to the
// data and derivative information stored in those rows.  These
// methods were created primarily for use with motion_profile objects.

bool mypolynomial::fit_coeffs(
   unsigned int ndata,double x[],int row,genarray& y,genarray& yprime,
   double inverse_sigma[],double inverse_sigma_prime[],double& chisq)
{
   double inverse_sigma_cross[ndata];
   for (unsigned int n=0; n<ndata; n++)
   {
      inverse_sigma_cross[n]=0;
   }
   return fit_coeffs(
      ndata,x,row,y,yprime,inverse_sigma,inverse_sigma_prime,
      inverse_sigma_cross,chisq);
}

bool mypolynomial::fit_coeffs(
   unsigned int ndata,double x[],int row,genarray& y,genarray& yprime,
   double inverse_sigma[],double inverse_sigma_prime[],
   double curvature_penalty_term_weight,double characteristic_length,
   double xstart,double xstop,double dx,double& chisq)
{
   double inverse_sigma_cross[ndata];
   for (unsigned int n=0; n<ndata; n++)
   {
      inverse_sigma_cross[n]=0;
   }
   return fit_coeffs(
      ndata,x,row,y,yprime,inverse_sigma,inverse_sigma_prime,
      inverse_sigma_cross,curvature_penalty_term_weight,characteristic_length,
      xstart,xstop,dx,chisq);
}

bool mypolynomial::fit_coeffs(
   unsigned int ndata,double x[],int row,genarray& y,genarray& yprime,
   double inverse_sigma[],double inverse_sigma_prime[],
   double curvature_penalty_term_weight,
   double extremal_value_penalty_term_weight,
   double yceiling,double yfloor,double yprime_ceiling,double yprime_floor,
   double characteristic_length,double xstart,double xstop,double dx,
   double& chisq)
{
   double inverse_sigma_cross[ndata];
   for (unsigned int n=0; n<ndata; n++)
   {
      inverse_sigma_cross[n]=0;
   }
   return fit_coeffs(
      ndata,x,row,y,yprime,inverse_sigma,inverse_sigma_prime,
      inverse_sigma_cross,
      curvature_penalty_term_weight,extremal_value_penalty_term_weight,
      yceiling,yfloor,yprime_ceiling,yprime_floor,
      characteristic_length,xstart,xstop,dx,chisq);
}

// ==========================================================================
// Ushomirsky's generalizations to simultaneous least squares fitting
// methods which incorporate off-diagonal contributions to the
// covariance matrix.  These contributions multiply rather than divide
// the residuals.  They also are not squared anymore.
// ==========================================================================

// In Feb 03, PLC made a concerted effort to streamline these
// following methods so that they run as fast as possible...

bool mypolynomial::fit_coeffs( 
   unsigned int ndata,double x[],double y[],double yprime[], 
   double inverse_covar[],double inverse_covar_prime[],
   double inverse_covar_cross[],double curvature_penalty_term_weight,
   double characteristic_length,double xstart,double xstop,double dx,
   double& chisq) 
{ 
   genmatrix basis(ndata,order+1);
   genmatrix deriv(ndata,order+1);
   for (unsigned int i=0; i<ndata; i++) 
   { 
      for (unsigned int k=0; k<=order; k++) 
      { 
         basis.put(i,k,basis_function(x[i],k)); 
         deriv.put(i,k,deriv_basis_function(x[i],1,k));
      } // loop over index k 
   } // loop over index i labeling image number 
   
   genvector V(order+1); 
   for (unsigned int k=0; k<=order; k++) 
   { 
      for (unsigned int i=0; i<ndata; i++) 
      { 
         double Vterm1=y[i]*basis.get(i,k)*inverse_covar[i]; 
         double Vterm2=yprime[i]*deriv.get(i,k)*inverse_covar_prime[i]; 
	 double Vterm3=(y[i]*deriv.get(i,k)+yprime[i]*basis.get(i,k))
            *inverse_covar_cross[i];
         V.increment(k,Vterm1+Vterm2+Vterm3); 
      } // loop over index i labeling image number 
   } // loop over index k 

// Matrix M is symmetric.  So we only need to explicitly compute its
// upper triangle elements:

   genmatrix M(order+1,order+1); 
   for (unsigned int i=0; i<ndata; i++) 
   { 
      for (unsigned int k=0; k<=order; k++) 
      { 
         for (unsigned int j=k; j<=order; j++) 
         { 
            double Mterm1=basis.get(i,k)*basis.get(i,j)*inverse_covar[i]; 
            double Mterm2=deriv.get(i,k)*deriv.get(i,j)
               *inverse_covar_prime[i]; 
	    double Mterm3=(basis.get(i,k)*deriv.get(i,j)
                           +basis.get(i,j)*deriv.get(i,k))
               *inverse_covar_cross[i];
            M.increment(k,j,Mterm1+Mterm2+Mterm3); 
         } // loop over index j 
      } // loop over index k 
   } // loop over index i labeling image number 

// Include the following integral term into matrix M whose purpose is
// to minimize wiggles in polynomial fits to noisy data:

   unsigned int nbins=basic_math::round((xstop-xstart)/dx)+1; 
   genmatrix double_deriv(nbins,order+1);
   for (unsigned int i=0; i<nbins; i++) 
   { 
      double currx=xstart+i*dx; 
      for (unsigned int k=0; k<=order; k++) 
      { 
         double_deriv.put(i,k,deriv_basis_function(currx,2,k));
      } // loop over index k 
   } // loop over index i labeling bin number 

   double prefactor=curvature_penalty_term_weight
      *sqr(sqr(characteristic_length))/(xstop-xstart); 
   double func[nbins]; 
   for (unsigned int k=0; k<=order; k++) 
   { 
      for (unsigned int j=k; j<=order; j++) 
      { 
         for (unsigned int i=0; i<nbins; i++) 
         { 
            func[i]=double_deriv.get(i,k)*double_deriv.get(i,j);
         } 
         double integral=prefactor*mathfunc::simpsonsum(func,0,nbins-1)*dx; 
         M.increment(k,j,integral);          
      } // loop over index j 
   } // loop over index k 

// Copy upper triangle elements of symmetric matrix M onto its lower
// triangle:

   for (unsigned int k=1; k<=order; k++)
   {
      for (unsigned int j=0; j<k; j++)
      {
         M.put(k,j,M.get(j,k));
      }
   }

// Recall that Minverse = polynomial's coefficient covariance matrix.
// See section 15.4 in Numerical Recipes...

   M.pseudo_inverse(0.01,*covmatrix_ptr);

   genvector A(order+1); 
   A=*covmatrix_ptr*V; 
   for (unsigned int k=0; k<=order; k++) a[k]=A.get(k); 
   
// Compute 0th and 1st derivative along with curvature term
// contributions to chisq function.  Also keep track of the cross term
// contribution:
   
   double chisq_0=0; 
   double chisq_1=0; 
   double chisq_cross=0;
   for (unsigned int i=0; i<ndata; i++) 
   { 
      chisq_0 += sqr(value(x[i])-y[i])*inverse_covar[i]; 
      chisq_1 += sqr(derivative(x[i],1)-yprime[i])*inverse_covar_prime[i]; 
      chisq_cross += 2.0*inverse_covar_cross[i]*(y[i]-value(x[i]))*
         (yprime[i]-derivative(x[i],1));
   }
   double func2[nbins];
   for (unsigned int i=0; i<nbins; i++)
   {
      double currx=xstart+i*dx; 
      func2[i]=sqr(derivative(currx,2));
   } 
   double chisq_2=prefactor*mathfunc::simpsonsum(func2,0,nbins-1)*dx; 

//      cout << "chisq_0 = " << chisq_0 << endl; 
//      cout << "chisq_1 = " << chisq_1 << endl; 
//      cout << "chisq_cross = " << chisq_cross << endl;
//      cout << "chisq_2 = " << chisq_2 << endl; 
//      cout << "chisq/NDOF = " << 
//         (chisq_0+chisq_1+chisq_cross)/(2*ndata-order-1) << endl; 
   chisq=chisq_0+chisq_1+chisq_cross+chisq_2;    

   return true;
} 

// ---------------------------------------------------------------------
// This final overloaded version of fit_coeffs yields precisely the
// same polynomial coefficients as its immediate predecessor.
// However, it returns a chisq value which includes an error function
// penalty term that grows large if the polynomial zeroth or first
// derivatives stray outside the intervals yfloor <= y <= yceiling and
// yprime_floor <= yprime <= yprime_ceiling.  The relative strength of
// this penalty term compared to the zeroth and first derivative
// contributions to chisq along with the wiggly term is controlled by
// input weight extremal_value_penalty_term_weight.  Since the penalty
// term is most definitely non-linear in the polynomial coefficients,
// we do not attempt to directly use it to influence the fit.
// However, this method can be called for different values of some
// external parameters (such as gaussian mean mu and width sigma).  We
// then simply pick the values of those parameters for which this
// method yields the smallest value of chisq.

bool mypolynomial::fit_coeffs( 
   unsigned int ndata,double x[],double y[],double yprime[], 
   double inverse_covar[],double inverse_covar_prime[],
   double inverse_covar_cross[],double curvature_penalty_term_weight,
   double extremal_value_penalty_term_weight,
   double yceiling,double yfloor,double yprime_ceiling,double yprime_floor,
   double characteristic_length,double xstart,double xstop,double dx,
   double& chisq) 
{ 
   double chisq_012;
   bool fit_successfully_calculated=fit_coeffs( 
      ndata,x,y,yprime,inverse_covar,inverse_covar_prime,inverse_covar_cross,
      curvature_penalty_term_weight,characteristic_length,xstart,xstop,dx,
      chisq_012);

// Add extremal value penalty term to chisq function:

   if (fit_successfully_calculated)
   {
      
      const double one_over_sqrt_two=0.707106781;
      unsigned int nbins=basic_math::round((xstop-xstart)/dx)+1; 
      double prefactor3=extremal_value_penalty_term_weight/(xstop-xstart); 
      double func3[nbins];
      for (unsigned int i=0; i<nbins; i++)
      {
         double currx=xstart+i*dx; 
         double curr_value=value(currx);
         double curr_deriv=derivative(currx,1);

// Experiment: Replace expensive calls to error_function with calls to
// faster poor_man_erf.  As of Feb 03, this modification seems to have
// negligible impact upon the quality of the final RHmotion fitting
// results.  So we retain this change.

         double term1=0.5*(1+mathfunc::poor_man_erf(
            one_over_sqrt_two*(curr_value-yceiling)));
         double term2=0.5*(1+mathfunc::poor_man_erf(
            one_over_sqrt_two*(yfloor-curr_value)));
         double term3=0.5*(1+mathfunc::poor_man_erf(
            one_over_sqrt_two*(curr_deriv-yprime_ceiling)));
         double term4=0.5*(1+mathfunc::poor_man_erf(
            one_over_sqrt_two*(yprime_floor-curr_deriv)));
         func3[i]=term1+term2+term3+term4;
      } 
      double chisq_3=prefactor3*mathfunc::simpsonsum(func3,0,nbins-1)*dx;
      chisq=chisq_012+chisq_3;
   }
   return fit_successfully_calculated;
} 

// ---------------------------------------------------------------------
// These overloaded versions of member function fit_coeffs take in
// genarrays y and yprime along with a particular row number
// associated with those arrays.  They return a polynomial fit to the
// data and derivative information stored in those rows.  These
// methods were created primarily for use with motion_profile objects.

bool mypolynomial::fit_coeffs(
   unsigned int ndata,double x[],int row,genarray& y,genarray& yprime,
   double inverse_covar[],double inverse_covar_prime[], 
   double inverse_covar_cross[],double curvature_penalty_term_weight,
   double characteristic_length,double xstart,double xstop,double dx,
   double& chisq)
{
   bool fit_successfully_calculated=false;
   chisq=POSITIVEINFINITY;
   if (y.get_ndim() != yprime.get_ndim())
   {
      cout << "Warning inside mypolynomial::fit_coeffs()!" << endl;
      cout << "Genarray y.ndim = " << y.get_ndim() 
           << " does not equal " << endl;
      cout << "genarray yprime.ndim" << endl;
   }
   else
   {
      double curr_data_row[y.get_ndim()];
      double curr_deriv_row[yprime.get_ndim()];
      y.get_row(row,curr_data_row);
      yprime.get_row(row,curr_deriv_row);
      fit_successfully_calculated=fit_coeffs(
         ndata,x,curr_data_row,curr_deriv_row,inverse_covar,
         inverse_covar_prime,inverse_covar_cross,
         curvature_penalty_term_weight,characteristic_length,xstart,xstop,dx,
         chisq);
   }
   return fit_successfully_calculated;
}

bool mypolynomial::fit_coeffs(
   unsigned int ndata,double x[],int row,genarray& y,genarray& yprime,
   double inverse_covar[],double inverse_covar_prime[], 
   double inverse_covar_cross[],double curvature_penalty_term_weight,
   double extremal_value_penalty_term_weight,
   double yceiling,double yfloor,double yprime_ceiling,double yprime_floor,
   double characteristic_length,double xstart,double xstop,double dx,
   double& chisq)
{
   bool fit_successfully_calculated=false;
   chisq=POSITIVEINFINITY;
   if (y.get_ndim() != yprime.get_ndim())
   {
      cout << "Warning inside mypolynomial::fit_coeffs()!" << endl;
      cout << "Genarray y.ndim = " << y.get_ndim() 
           << " does not equal " << endl;
      cout << "genarray yprime.ndim" << endl;
   }
   else
   {
      double curr_data_row[y.get_ndim()];
      double curr_deriv_row[yprime.get_ndim()];
      y.get_row(row,curr_data_row);
      yprime.get_row(row,curr_deriv_row);
      fit_successfully_calculated=fit_coeffs(
         ndata,x,curr_data_row,curr_deriv_row,
         inverse_covar,inverse_covar_prime,inverse_covar_cross,
         curvature_penalty_term_weight,extremal_value_penalty_term_weight,
         yceiling,yfloor,yprime_ceiling,yprime_floor,
         characteristic_length,xstart,xstop,dx,chisq);
   }
   return fit_successfully_calculated;
}

// ==========================================================================
// Polynomial uncertainty estimation methods
// ==========================================================================

// Member function sigma2_value returns the squared uncertainty in the
// current polynomial object's value at point x using the covariance
// matrix for the polynomial's coefficients.  Recall

//    sigma_poly**2(x) = sum_{i,j} dpoly/da_i(x) covar_ij dpoly/da_j(x)

// See chapter 9 in "Lectures on probabililty and statistics" by
// G.P. Yost for discussion.

double mypolynomial::sigma2_value(double x,double mu,double sigma) const
{
   double y=(x-mu)/sigma;
   double sigma_sqr=0;

   for (unsigned int i=0; i<=order; i++)
     for (unsigned int j=0; j<=order; j++)
        sigma_sqr += 
           covmatrix_ptr->get(i,j)*basis_function(y,i)*basis_function(y,j);
   return sigma_sqr;
}

// ---------------------------------------------------------------------
// Member function sigma2_derivative returns the squared uncertainty
// in the deriv_order derivative of the current polynomial object
// point x given the covariance matrix for the polynomial's
// coefficients:

double mypolynomial::sigma2_derivative(double x,int deriv_order) const
{
   return sigma2_derivative(x,0,1,deriv_order);
}

double mypolynomial::sigma2_derivative(
   double x,double mu,double sigma,int deriv_order) const
{
   double y=(x-mu)/sigma;
   double sigma_derivative_sqr=0;

   if (deriv_order==0) 
   {
      return sigma2_value(y);
   }
   else
   {
      for (unsigned int i=0; i<=order; i++)
         for (unsigned int j=0; j<=order; j++)
           sigma_derivative_sqr += covmatrix_ptr->get(i,j)
	    *deriv_basis_function(y,deriv_order,i)
	    *deriv_basis_function(y,deriv_order,j);
      return sigma_derivative_sqr/mathfunc::real_power(sigma,2*deriv_order);
   } 
}

// ==========================================================================
// Linked list generation methods
// ==========================================================================

// This overloaded version of member function generate_polyderiv_list
// generates the current polynomial object's deriv_order derivative
// after its independent variable is shifted by mu and rescaled by
// 1/sigma.  It also takes in parameter error_multiple whose
// default value equals 1.  The error bounds returned within the
// func[1] and func[2] members of each node are multipled by
// error_multiple.  This allows one to specify that "2 sigma", "3
// sigma", etc errors be returned rather than just "1 sigma"
// uncertainties.

// Note: y = (x-mu)/sigma.

void mypolynomial::generate_polyderiv_list(
   unsigned int nbins,int deriv_order,double xstart,double xstop,
   double mu,double sigma,colorfunc::Color color,linkedlist& polylist,
   double error_multiple)
{
   const int n_depend_vars=3;
   double func[n_depend_vars];
   double dx=(xstop-xstart)/(nbins-1);
   for (unsigned int i=0; i<nbins; i++)
   {
      double x=xstart+i*dx;
      double y=(x-mu)/sigma;
      func[0]=derivative(y,deriv_order)/
         mathfunc::real_power(sigma,deriv_order);
      func[1]=func[0]+error_multiple*
         sqrt(sigma2_derivative(x,mu,sigma,deriv_order));
      func[2]=func[0]-error_multiple*
         sqrt(sigma2_derivative(x,mu,sigma,deriv_order));
      polylist.append_node(datapoint(n_depend_vars,x,func));
      polylist.get_node(i)->get_data().set_color(color);
   }
}

// ==========================================================================
// Overloading elementary arithmetic operations:
// ==========================================================================

void mypolynomial::operator/= (double c)
{
   if (c != 0)
   {
      for (unsigned int i=0; i<=order; i++) a[i] /= c;
   }
   else
   {
      cout << "Division by zero error in mypolynomial::operator/= " << endl;
   }
}

mypolynomial operator+ (const mypolynomial& p,const mypolynomial& q)
{
   unsigned int order=p.get_order();
   if (q.get_order()==order)
   {
      double a[order+1];
      for (unsigned int i=0; i<=order; i++)
      {
         a[i]=p.get_coeff(i)+q.get_coeff(i);
      }
      return mypolynomial(order,a);
   }
   else
   {
      cout << "Error in operator+ within mypolynomial class!" << endl;
      cout << "Polynomial p order = " << p.get_order() << endl;
      cout << "Polynomial q order = " << q.get_order() << endl;
      exit(-1);
   }
}

mypolynomial operator- (const mypolynomial& p)
{
   unsigned int order=p.get_order();
   double a[order+1];
   for (unsigned int i=0; i<=order; i++)
   {
      a[i]=-p.get_coeff(i);
   }
   return mypolynomial(order,a);
}

mypolynomial operator* (double c,const mypolynomial& p)
{
   unsigned int order=p.get_order();
   double a[order+1];
   for (unsigned int i=0; i<=order; i++)
   {
      a[i]=c*p.get_coeff(i);
   }
   return mypolynomial(order,a);
}





// ---------------------------------------------------------------------
bool mypolynomial::fit_coeffs_using_residuals(
   const std::vector<double>& X,const std::vector<double> Y,double& chisq)
{
   bool fit_successfully_calculated=fit_coeffs(X,Y,chisq);
   if (!fit_successfully_calculated) return false;
   vector<double> residual=compute_residuals(X,Y);
   fit_successfully_calculated=fit_coeffs(X,Y,residual,chisq);
   return fit_successfully_calculated;
}
