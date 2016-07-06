// ==========================================================================
// piecewise_linear_vector class member function definitions
// ==========================================================================
// Last modified on 5/14/08; 6/20/08; 4/5/14
// ==========================================================================

#include <iostream>
#include "math/mathfuncs.h"
#include "filter/piecewise_linear_vector.h"

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

void piecewise_linear_vector::allocate_member_objects()
{
}

void piecewise_linear_vector::initialize_member_objects()
{
}

piecewise_linear_vector::piecewise_linear_vector(
   const vector<double>& T_input,const vector<threevector>& R_input)
{
   allocate_member_objects();
   initialize_member_objects();
   for (unsigned int i=0; i<T_input.size(); i++)
   {
      T.push_back(T_input[i]);
      R.push_back(R_input[i]);
   }
   n_vertices=T.size();
}

piecewise_linear_vector::piecewise_linear_vector(
   const vector<double>& T_input,const vector<rpy>& R_input)
{
   allocate_member_objects();
   initialize_member_objects();
   for (unsigned int i=0; i<T_input.size(); i++)
   {
      T.push_back(T_input[i]);
      threevector curr_R(
         R_input[i].get_roll(),R_input[i].get_pitch(),R_input[i].get_yaw());
      R.push_back(curr_R);
   }
   n_vertices=T.size();
}

// Copy constructor:

piecewise_linear_vector::piecewise_linear_vector(
   const piecewise_linear_vector& p)
{
   allocate_member_objects();
   initialize_member_objects();
   docopy(p);
}

piecewise_linear_vector::~piecewise_linear_vector()
{
}

// ---------------------------------------------------------------------
void piecewise_linear_vector::docopy(const piecewise_linear_vector & p)
{
}
   
// Overload = operator:

piecewise_linear_vector& piecewise_linear_vector::operator= (
   const piecewise_linear_vector& p)
{
   if (this==&p) return *this;
   docopy(p);
   return *this;
}

// Overload << operator:

ostream& operator<< (ostream& outstream,const piecewise_linear_vector& p)
{
   outstream.precision(10);
   outstream.setf(ios::scientific);

   for (unsigned int v=0; v<p.n_vertices; v++)
   {
      cout << "v = " << v 
           << " t = " << p.T[v]
           << " x = " << p.R[v].get(0)
           << " y = " << p.R[v].get(1)
           << " z = " << p.R[v].get(2)
           << endl;
   }

   outstream << endl;
   return outstream;
}

// ==========================================================================
// Polynomial evaluation methods
// ==========================================================================

int piecewise_linear_vector::get_segment_number(double t) const
{
   return mathfunc::binary_locate(T,0,n_vertices-1,t);
}

int piecewise_linear_vector::closest_vertex_ID(double t) const
{
//   cout << "inside piecewise_linear_vector::closest_vertex_ID()" << endl;
   int n_lo=get_segment_number(t);
   int n_hi=n_lo+1;
   if (n_lo < 0)
   {
      n_lo=n_hi=0;
   }
   else if (n_lo >= int(n_vertices-1))
   {
      n_lo=n_hi=n_vertices-1;
   }
   double dt_lo=fabs(t-T[n_lo]);
   double dt_hi=fabs(T[n_hi]-t);
//   cout << "n_lo = " << n_lo << " n_hi = " << n_hi << endl;
//   cout << "dt_lo = " << dt_lo << " dt_hi = " << dt_hi << endl;

   if (dt_lo < dt_hi)
   {
      return n_lo;
   }
   else
   {
      return n_hi;
   }
}

// ---------------------------------------------------------------------
threevector piecewise_linear_vector::value(double t) const
{
//   cout << "inside piecewise_linear_vector::value(), t = " << t << endl;
   int n=get_segment_number(t);
   if (n < 0)
   {
      return R[0];
   }
   else if (n >= int(n_vertices-1))
   {
      return R[n_vertices-1];
   }
   else
   {
      double frac=(t-T[n])/(T[n+1]-T[n]);
      return ( R[n] + frac*(R[n+1]-R[n]) );
   }
}
