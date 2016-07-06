// ==========================================================================
// Ellipse class member function definitions
// ==========================================================================
// Last modified on 2/19/07
// ==========================================================================

#include "geometry/ellipse.h"

using std::cout;
using std::endl;
using std::ostream;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void ellipse::allocate_member_objects()
{
}		       

void ellipse::initialize_member_objects()
{
   a=b=1;
   theta=0;
   XY_center=threevector(0,0);
}

ellipse::ellipse(void)
{
   allocate_member_objects();
   initialize_member_objects();
}

ellipse::ellipse(double a,double b,double theta)
{
   allocate_member_objects();
   initialize_member_objects();

   this->a=a;
   this->b=b;
   this->theta=theta;
}

ellipse::ellipse(const threevector& XY_center,double a,double b,double theta)
{
   allocate_member_objects();
   initialize_member_objects();

   this->XY_center=XY_center;
   this->a=a;
   this->b=b;
   this->theta=theta;
}

// Copy constructor:

ellipse::ellipse(const ellipse& e)
{
   allocate_member_objects();
   initialize_member_objects();
   docopy(e);
}

ellipse::~ellipse()
{
}

// ---------------------------------------------------------------------
void ellipse::docopy(const ellipse& e)
{
}

// Overload = operator:

ellipse& ellipse::operator= (const ellipse& e)
{
   if (this==&e) return *this;
   docopy(e);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const ellipse& e)
{
   outstream << endl;
   return(outstream);
}

// =====================================================================
// Ellipse vertex generation member functions
// =====================================================================

// Member function generate_vertices computes n_vertices vertex
// locations along the ellipse's perimeter which are equally spaced in
// azimuthal angle phi.  

vector<threevector>& ellipse::generate_vertices(
   int n_vertices,double phase_offset)
{
   const double cos_theta=cos(theta);
   const double sin_theta=sin(theta);

   double phi_start=0;
   double phi_stop=2*PI;
   double d_phi=(phi_stop-phi_start)/(n_vertices);
   for (int n=0; n<n_vertices; n++)
   {
      double phi=phi_start+n*d_phi;
      double u=a*cos(phi+phase_offset);
      double v=b*sin(phi+phase_offset);
      double x=cos_theta*u-sin_theta*v;
      double y=sin_theta*u+cos_theta*v;
      vertex.push_back(threevector(x,y)+XY_center);
   } // loop over index n labeling vertices along ellipse

   return vertex;
}
