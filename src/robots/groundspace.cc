// ==========================================================================
// Groundspace class member function definitions
// ==========================================================================
// Last modified on 2/14/08; 2/27/08
// ==========================================================================

#include <iostream>
#include <vector>
#include "math/constant_vectors.h"
#include "robots/groundspace.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/Polygon.h"

using std::cout;
using std::endl;
using std::ostream;
using std::vector;

// --------------------------------<-------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void groundspace::allocate_member_objects()
{
}		       

void groundspace::initialize_member_objects()
{
   origin=Zero_vector;
   max_robot_dist_from_origin=100; // default value in meters
}

groundspace::groundspace()
{
   allocate_member_objects();
   initialize_member_objects();
}

// Copy constructor:

groundspace::groundspace(const groundspace& g)
{
   allocate_member_objects();
   initialize_member_objects();
   docopy(g);
}

groundspace::~groundspace()
{
}

// ---------------------------------------------------------------------
void groundspace::docopy(const groundspace& g)
{
//   cout << "inside groundspace::docopy()" << endl;

}

// Overload = operator:

groundspace& groundspace::operator= (const groundspace& g)
{
   if (this==&g) return *this;
   docopy(g);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const groundspace& g)
{
   outstream << endl;

//   cout << "inside groundspace::operator<<" << endl;
   return(outstream);
}

// =====================================================================

// Member function potential_energy

double groundspace::potential_energy(double r,const threevector& x)
{
//   cout << "inside groundspace::potential_energy()" << endl;
//   cout << "x = " << x << endl;

// Penalize robots which move too far away from ground space origin:

   double E_origin,alpha_origin;
   if (r > max_robot_dist_from_origin)
   {
      alpha_origin=1;
      E_origin=alpha_origin*r;
   }
   else
   {
//      alpha_origin=0;
      alpha_origin=1E-5;
      E_origin=alpha_origin*r;
   }

// Heavily penalize robots which stray into Keep-out zones:

   double E_KOZ=0;

   for (unsigned int k=0; k<KOZ_PolygonsGroup_ptr->get_n_Graphicals(); k++)
   {
      osgGeometry::Polygon* curr_Polygon_ptr=
         KOZ_PolygonsGroup_ptr->get_Polygon_ptr(k);

      threevector relative_x=x-curr_Polygon_ptr->get_reference_origin();

//      cout << "k = " << k 
//           << " ref origin = " << curr_Polygon_ptr->get_reference_origin()
//           << " rel x = " << relative_x.get(0)
//           << " rel y = " << relative_x.get(1) 
//           << endl;
      
      polygon* relative_poly_ptr=curr_Polygon_ptr->get_relative_poly_ptr();

//      cout << " rel poly = " << *relative_poly_ptr << endl;
      
      if (relative_poly_ptr->point_inside_polygon(relative_x))
      {
//         cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
//         cout << "Robot inside KOZ k = " << k << endl;
//         cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
         double alpha_KOZ=10;
         E_KOZ=alpha_KOZ*(
            relative_x-relative_poly_ptr->vertex_average()).magnitude();
      }
   } // loop over index k labeling KOZ Polygons

//   cout << "E0 = " << E0 << " E_KOZ = " << E_KOZ << endl;
   double Etotal=E_origin+E_KOZ;
   return Etotal;
}

