// ==========================================================================
// Piecewise_linear class member function definitions
// ==========================================================================
// Last modified on 9/12/07; 6/20/08; 5/14/13; 4/4/14
// ==========================================================================

#include <iostream>
#include "filter/filterfuncs.h"
#include "math/mathfuncs.h"
#include "filter/piecewise_linear.h"

#include "templates/mytemplates.h"

using std::cin;
using std::cout;
using std::endl;
using std::ios;
using std::ostream;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void piecewise_linear::allocate_member_objects()
{
}

void piecewise_linear::initialize_member_objects()
{
}

piecewise_linear::piecewise_linear(
   const vector<double>& Xinput,const vector<double>& Yinput)
{
   allocate_member_objects();
   initialize_member_objects();
   for (unsigned int v=0; v<Xinput.size(); v++)
   {
      X.push_back(Xinput[v]);
      Y.push_back(Yinput[v]);
   }
   n_vertices=X.size();
}

piecewise_linear::piecewise_linear(const vector<twovector>& V)
{
   allocate_member_objects();
   initialize_member_objects();
   for (unsigned int v=0; v<V.size(); v++)
   {
      X.push_back(V[v].get(0));
      Y.push_back(V[v].get(1));
   }
   n_vertices=X.size();
}

// Copy constructor:

piecewise_linear::piecewise_linear(const piecewise_linear& p)
{
   allocate_member_objects();
   initialize_member_objects();
   docopy(p);
}

piecewise_linear::~piecewise_linear()
{
}

// ---------------------------------------------------------------------
void piecewise_linear::docopy(const piecewise_linear & p)
{
}
   
// Overload = operator:

piecewise_linear& piecewise_linear::operator= (const piecewise_linear& p)
{
   if (this==&p) return *this;
   docopy(p);
   return *this;
}

// Overload << operator:

ostream& operator<< (ostream& outstream,const piecewise_linear& p)
{
   outstream.precision(10);
   outstream.setf(ios::scientific);

   for (unsigned int v=0; v<p.n_vertices; v++)
   {
      cout << "v = " << v << " x = " << p.X[v]
           << " y = " << p.Y[v] << endl;
   }

   outstream << endl;
   return outstream;
}

// ==========================================================================
// Polynomial evaluation methods
// ==========================================================================

unsigned int piecewise_linear::get_segment_number(double x) const
{
   return mathfunc::binary_locate(X,0,n_vertices-1,x);
}

unsigned int piecewise_linear::closest_vertex_ID(double x) const
{
//   cout << "inside piecewise_linear::closest_vertex_ID()" << endl;
   unsigned int n_lo=get_segment_number(x);
   unsigned int n_hi=n_lo+1;
   if (n_lo < 0)
   {
      n_lo=n_hi=0;
   }
   else if (n_lo >= n_vertices-1)
   {
      n_lo=n_hi=n_vertices-1;
   }
   double dx_lo=fabs(x-X[n_lo]);
   double dx_hi=fabs(X[n_hi]-x);
//   cout << "n_lo = " << n_lo << " n_hi = " << n_hi << endl;
//   cout << "dx_lo = " << dx_lo << " dx_hi = " << dx_hi << endl;

   if (dx_lo < dx_hi)
   {
      return n_lo;
   }
   else
   {
      return n_hi;
   }
}

// ---------------------------------------------------------------------
double piecewise_linear::value(double x) const
{
//   cout << "inside piecewise_linear::value()" << endl;
   
   unsigned int n=get_segment_number(x);
//   cout << "x = " << " segment number n = " << n << endl;
   
   if (n < 0)
   {
      return Y[0];
   }
   else if (n >= n_vertices-1)
   {
      return Y[n_vertices-1];
   }
   else
   {
      double frac=(x-X[n])/(X[n+1]-X[n]);
      return ( Y[n] + frac*(Y[n+1]-Y[n]) );
   }
}

// ==========================================================================
// Filtering methods
// ==========================================================================

// Member function filter_values performs a brute-force convolution of
// the piecewise linear function values with a gaussian kernel whose
// width is set by input parameter sigma.  

void piecewise_linear::filter_values(double dx,double sigma)
{
   double x_start=X[0];
   double x_stop=X[n_vertices-1];
   unsigned int nbins=(x_stop-x_start)/dx+1;

   vector<double> Y_raw;
   X_filtered.clear();
   Y_filtered.clear();

   X_filtered.reserve(nbins);
   Y_raw.reserve(nbins);
   Y_filtered.reserve(nbins);

// Compute regularized X and raw Y values:

   for (unsigned int n=0; n<nbins; n++)
   {
      X_filtered.push_back(x_start+n*dx);
      Y_raw.push_back(value(X_filtered.back()));
   }

// Compute gaussian filter values:

   vector<double> h;
   h.reserve(nbins);
   filterfunc::gaussian_filter(dx,sigma,h);
   filterfunc::brute_force_filter(Y_raw,h,Y_filtered,false);
   
// Replace filtered values at beginning and end of piecewise-linear
// function with regularized values so that output values are forced
// to match input values:

   unsigned int n_linear_bins=2*sigma/dx;
   for (unsigned int n=0; n<n_linear_bins; n++)
   {
      Y_filtered[n]=Y_raw[n];
      Y_filtered[nbins-1-n]=Y_raw[nbins-1-n];
   }

//   cout << "Filtered values:" << endl;
//   for (unsigned int n=0; n<nbins; n++)
//   {
//      cout << X_filtered[n] << "     " << Y_filtered[n] << endl;
//   }

}

