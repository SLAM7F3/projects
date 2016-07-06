// ==========================================================================
// Frustum class member function definitions
// ==========================================================================
// Last modified on 7/30/06; 7/31/06; 1/29/12; 4/5/14
// ==========================================================================

#include <vector>
#include "math/basic_math.h"
#include "math/constant_vectors.h"
#include "geometry/frustum.h"
#include "math/rotation.h"

using std::cout;
using std::endl;
using std::vector;

const int frustum::nmax_sidefaces=50;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void frustum::allocate_member_objects()
{
   side_face=new polygon[nmax_sidefaces];
}		       

void frustum::initialize_member_objects()
{
}

frustum::frustum(void)
{
   allocate_member_objects();
}

frustum::frustum(int num_of_sidefaces)
{
   allocate_member_objects();
   n_sidefaces=num_of_sidefaces;
}

frustum::frustum(int num_of_sidefaces,double top_radius,double bottom_radius,
                 double h)
{
   allocate_member_objects();

   n_sidefaces=num_of_sidefaces;
   height=h;

   center=Zero_vector;

   top_face=regular_polygon(n_sidefaces,top_radius);
   top_face.translate(height/2*z_hat);
   top_face.compute_area();

   bottom_face=regular_polygon(n_sidefaces,bottom_radius);
   bottom_face.translate(-height/2*z_hat);
   bottom_face.compute_area();

   vector<threevector> vertex;
   vertex.reserve(4);
   for (unsigned int i=0; i<n_sidefaces-1; i++)
   {
      vertex.clear();
      vertex.push_back(bottom_face.get_vertex(i));
      vertex.push_back(bottom_face.get_vertex(i+1));
      vertex.push_back(top_face.get_vertex(i+1));
      vertex.push_back(top_face.get_vertex(i));
      side_face[i]=polygon(vertex);
      side_face[i].compute_area();
   }
   vertex[0]=bottom_face.get_vertex(n_sidefaces-1);
   vertex[1]=bottom_face.get_vertex(0);
   vertex[2]=top_face.get_vertex(0);
   vertex[3]=top_face.get_vertex(n_sidefaces-1);
   side_face[n_sidefaces-1]=polygon(vertex);
   side_face[n_sidefaces-1].compute_area();
}

// Copy constructor:

frustum::frustum(const frustum& f)
{
   allocate_member_objects();
   docopy(f);
}

frustum::~frustum()
{
   delete [] side_face;
   side_face=NULL;
}

// ---------------------------------------------------------------------
void frustum::docopy(const frustum& f)
{
   n_sidefaces=f.n_sidefaces;
   height=f.height;
   center=f.center;
   top_face=f.top_face;
   bottom_face=f.bottom_face;
   
   for (unsigned int i=0; i<n_sidefaces; i++)
   {
      side_face[i]=f.side_face[i];
   }
}	

// Overload = operator:

frustum& frustum::operator= (const frustum& f)
{
   if (this==&f) return *this;
   docopy(f);
   return *this;
}

// ---------------------------------------------------------------------
// Member function orient_normals_outward multiplies the top, bottom
// and side faces' normal direction vectors by +/- 1 so that they all
// point radially outward from the frustum's center:

void frustum::orient_normals_outward()
{
   threevector diff(top_face.get_vertex(0)-center.get_pnt());
   threevector top_normal(top_face.get_normal());
   top_face.set_normal(sgn(diff.dot(top_normal)) * top_normal);

   diff=bottom_face.get_vertex(0)-center.get_pnt();
   threevector bottom_normal(bottom_face.get_normal());
   bottom_face.set_normal(sgn(diff.dot(bottom_normal)) * bottom_normal);

   for (unsigned int i=0; i<n_sidefaces; i++)
   {
      diff=side_face[i].get_vertex(0)-center.get_pnt();
      threevector side_normal(side_face[i].get_normal());
      side_face[i].set_normal(sgn(diff.dot(side_normal)) * side_normal);
   }
}

// ---------------------------------------------------------------------
void frustum::print_frustum()
{
   cout << "Inside print_frustum within frustum.cc, n_sidefaces = "
        << n_sidefaces << endl;

   cout << "Top face:" << top_face << endl;
   cout << "Bottom face:" << bottom_face << endl;
   for (unsigned int i=0; i<n_sidefaces; i++)
   {
      cout << "Side face " << i << side_face[i] << endl;
   }
}

// ---------------------------------------------------------------------
void frustum::translate(const threevector& rvec)
{
   center.set_pnt(center.get_pnt()+rvec);
   top_face.translate(rvec);
   bottom_face.translate(rvec);
   
   for (unsigned int i=0; i<n_sidefaces; i++)
   {
      side_face[i].translate(rvec);
   }
}

// ---------------------------------------------------------------------
void frustum::scale(const threevector& scale_origin,
                    const threevector& scalefactor)
{
// Note: The following line is NOT generally true!  As of 6/4/01, we
// have not taken the time to figure out the correct generalization of
// the following line which holds only if the frustum's symmetry axis
// is aligned along +/- z_hat:

   height *= scalefactor.get(2);
   
   center.scale(scale_origin,scalefactor);
   top_face.scale(scale_origin,scalefactor);
   bottom_face.scale(scale_origin,scalefactor);
   for (unsigned int i=0; i<n_sidefaces; i++)
   {
      side_face[i].scale(scale_origin,scalefactor);
   }
   orient_normals_outward();
}

// ---------------------------------------------------------------------
void frustum::rotate(const rotation& R)
{
   rotate(Zero_vector,R);
}

// ---------------------------------------------------------------------
void frustum::rotate(const threevector& rotation_origin,const rotation& R)
{
   center.rotate(rotation_origin,R);
   top_face.rotate(rotation_origin,R);
   bottom_face.rotate(rotation_origin,R);
   
   for (unsigned int i=0; i<n_sidefaces; i++)
   {
      side_face[i].rotate(rotation_origin,R);
   }
   orient_normals_outward();
}

// ---------------------------------------------------------------------
void frustum::rotate(const threevector& rotation_origin,
                     double thetax,double thetay,double thetaz)
{
   rotation R(thetax,thetay,thetaz);
   rotate(rotation_origin,R);
}

// ---------------------------------------------------------------------
bool frustum::point_inside_xyprojected_frustum(const threevector& point) const
{
   bool point_inside=false;
   double dotproduct;
   polygon curr_projection;

//   dotproduct=z_hat.dot(top_face.get_normal());
//   if (dotproduct > 0)
   {
      curr_projection=top_face.xy_projection();
      if (curr_projection.point_inside_polygon(point)) point_inside=true;
   }

//   dotproduct=z_hat.dot(bottom_face.get_normal());
//   if (dotproduct > 0)
   {
      curr_projection=bottom_face.xy_projection();
      if (curr_projection.point_inside_polygon(point)) point_inside=true;
   }
   
   for (unsigned int i=0; i<n_sidefaces; i++)
   {
      dotproduct=z_hat.dot(side_face[i].get_normal());
      if (dotproduct > 0)
      {
         curr_projection=side_face[i].xy_projection();
         if (curr_projection.point_inside_polygon(point)) point_inside=true;
      }
   }
   return point_inside;
}








