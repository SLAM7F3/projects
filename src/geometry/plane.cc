// =========================================================================
// Plane class member function definitions
// =========================================================================
// Last modified on 3/23/12; 7/10/12; 2/9/13; 4/4/14
// =========================================================================

#include <iostream>
#include "geometry/bounding_box.h"
#include "math/constant_vectors.h"
#include "geometry/geometry_funcs.h"
#include "geometry/linesegment.h"
#include "math/mathfuncs.h"
#include "numrec/nrfuncs.h"
#include "numerical/param_range.h"
#include "geometry/plane.h"
#include "geometry/polygon.h"
#include "math/prob_distribution.h"
#include "templates/mytemplates.h"

using std::cout;
using std::endl;
using std::flush;
using std::ostream;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void plane::allocate_member_objects()
{
}

void plane::initialize_member_objects()
{
   a_min=b_min=n_min=POSITIVEINFINITY;
   a_max=b_max=n_max=NEGATIVEINFINITY;
   xhat=threevector(1,0,0);
   yhat=threevector(0,1,0);
   zhat=threevector(0,0,1);
}		 

// ---------------------------------------------------------------------
plane::plane()
{
   allocate_member_objects();
   initialize_member_objects();
}

plane::plane(const threevector& V1,const threevector& V2,
             const threevector& V3)
{
//   cout << "inside plane constructor() #1" << endl;
//   cout << "V1 = " << V1 << " V2 = " << V2 << " V3 = " << V3 << endl;

   allocate_member_objects();
   initialize_member_objects();

   threevector cross=(V1-V3).cross(V2-V3);
   n_hat=cross.unitvector();
   double pi4=-n_hat.dot(V1);
   pi=fourvector(n_hat,pi4);

//   cout << "pi = " << pi << endl;
//   construct_2D_coord_system();

   origin=V1;

//   cout << "origin = " << get_origin() << endl;
//   cout << "n_hat = " << get_nhat() << endl;

//   cout << "V1 dist from plane = " << signed_distance_from_plane(V1) << endl;
//   cout << "V2 dist from plane = " << signed_distance_from_plane(V2) << endl;
//   cout << "V3 dist from plane = " << signed_distance_from_plane(V3) << endl;
}

// ---------------------------------------------------------------------
// This next constructor builds a plane from the input threevector n
// which is proportional to its normal vector as well as threevector P
// lying on the plane.

plane::plane(const threevector& n,const threevector& P)
{
//   cout << "inside plane constructor, n = " << n << " P = " << P << endl;
   allocate_member_objects();
   initialize_member_objects();

   pi.put(0,n.get(0));
   pi.put(1,n.get(1));
   pi.put(2,n.get(2));
   pi.put(3,-n.dot(P));

//   cout << "pi = " << pi << endl;
   construct_2D_coord_system();
   origin=P;
}

// ---------------------------------------------------------------------
plane::plane(const fourvector& input_pi)
{
   allocate_member_objects();
   initialize_member_objects();
   pi=input_pi;
   construct_2D_coord_system();
}

// ---------------------------------------------------------------------
// This next constructor takes in an STL vector containing 4 or more
// threevectors.  It utilizes a least squares approach to determine
// the best planar fit to the points' locations.

plane::plane(const vector<threevector>& Vworld)
{
//   cout << "inside plane constructor which takes in vector of threevectors"
//        << endl;

   allocate_member_objects();
   initialize_member_objects();

   unsigned int mdim=Vworld.size();
   if (mdim <= 3)
   {
      cout << "Error in plane constructor!" << endl;
      cout << "Number of input threevectors = " << mdim << endl;
      cout << "Need 4 or more inputs for least squares fit" << endl;
      exit(-1);
   }

   genmatrix A(mdim,4);
   for (unsigned int m=0; m<mdim; m++)
   {
      threevector curr_V(Vworld[m]);
      A.put(m,0,curr_V.get(0));
      A.put(m,1,curr_V.get(1));
      A.put(m,2,curr_V.get(2));
      A.put(m,3,1);
   }

   A.homogeneous_soln(pi);

//      cout << "pi = " << pi << endl;
   construct_2D_coord_system();
}

// ---------------------------------------------------------------------
// Copy constructor:

plane::plane(const plane& l)
{
   docopy(l);
}

plane::~plane()
{
}

// ---------------------------------------------------------------------
void plane::docopy(const plane& p)
{
   a_min=p.a_min;
   a_max=p.a_max;
   b_min=p.b_min;
   b_max=p.b_max;
   n_min=p.n_min;
   n_max=p.n_max;
   
   xhat=p.xhat;
   yhat=p.yhat;
   zhat=p.zhat;

   n_hat=p.n_hat;
   a_hat=p.a_hat;
   b_hat=p.b_hat;
   origin=p.origin;

   pi=p.pi;
}

// Overload = operator:

plane& plane::operator= (const plane& p)
{
   if (this==&p) return *this;
   docopy(p);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const plane& p)
{
   outstream << endl;
   outstream << "Pi = " << p.pi << endl;
   outstream << "origin = " << p.origin << endl;
   outstream << "n_hat = " << p.n_hat << endl;
//   outstream << "a_hat = " << p.a_hat << endl;
//   outstream << "b_hat = " << p.b_hat << endl;
   return outstream;
}

// =========================================================================
// Intrinsic plane properties
// =========================================================================

bool plane::point_on_plane(const threevector& V,double tolerance) const
{
//   cout << "inside plane::point_on_plane()" << endl;
   fourvector V4(V.get(0),V.get(1),V.get(2),1);
   double dotproduct=V4.dot(pi);
//   cout << "dotproduct = " << dotproduct << endl;
//   return nearly_equal(dotproduct,0,1E-4);
   return nearly_equal(dotproduct , 0 , tolerance);
}

/*
// ---------------------------------------------------------------------
// Member function signed_distance_from_plane takes in a threevector
// and returns its perpendicular distance to the current plane object.
// If this distance is negative, the point lies on the side of the
// plane anti-parallel to normal direction vector n_hat.

double plane::signed_distance_from_plane(const threevector& V) const
{
//   cout << "inside plane::signed_distance_from_plane()" << endl;
//   cout << "V = " << V << endl;
//   cout << "origin = " << origin << endl;
//   cout << "n_hat = " << n_hat << endl;
    double signed_distance=n_hat.dot(V-origin);
//   cout << "signed_distance = " << signed_distance << endl;
    return signed_distance;
  }
*/

// ---------------------------------------------------------------------
// Member function z_coordinate returns the z value corresponding to
// the input (x,y) pair.

double plane::z_coordinate(double x,double y) const
{
   if (nearly_equal(pi.get(2),0))
   {
      cout << "Error in plane::z_coordinate(), pi_z = 0 !!!" << endl;
      return NEGATIVEINFINITY;
   }
   else
   {
      return -(pi.get(0)*x+pi.get(1)*y+pi.get(3))/pi.get(2);
   }
}

// ---------------------------------------------------------------------
// Member function construct_2D_coord_system generates an orthonormal
// basis {a_hat,b_hat,n_hat} for the current plane object.  It also
// fixes an origin point on the plane.

void plane::construct_2D_coord_system()
{
//   cout << "inside plane::construct_2D_coord_system()" << endl;
   threevector n(pi.get(0),pi.get(1),pi.get(2));
   n_hat=n.unitvector();

   if (nearly_equal(n_hat.get(0),0) && nearly_equal(n_hat.get(1),0)
       && nearly_equal(n_hat.get(2),1))
   {
      a_hat=xhat;
      b_hat=yhat;
   }
   else if (nearly_equal(n_hat.get(0),0) && nearly_equal(n_hat.get(1),0)
            && nearly_equal(n_hat.get(2),-1))
   {
      a_hat=xhat;
      b_hat=-yhat;
   }
   else
   {
      a_hat=n_hat.cross(zhat);
      a_hat=a_hat.unitvector();
      b_hat=n_hat.cross(a_hat);
   }

//   cout << "a_hat = " << a_hat << " b_hat = " << b_hat 
//        << " n_hat = " << n_hat << endl;
//   cout << "a_hat.b_hat = " << a_hat.dot(b_hat) << endl;
//   cout << "b_hat.n_hat = " << b_hat.dot(n_hat) << endl;
//   cout << "n_hat.a_hat = " << n_hat.dot(a_hat) << endl;
//   cout << "a_hat.a_hat = " << a_hat.dot(a_hat) << endl;
//   cout << "b_hat.b_hat = " << b_hat.dot(b_hat) << endl;
//   cout << "n_hat.n_hat = " << n_hat.dot(n_hat) << endl;

   double kappa=-pi.get(3)/n_hat.dot(
      threevector(pi.get(0),pi.get(1),pi.get(2)));

   origin=threevector(kappa*n_hat);
//   cout << "plane's origin = " << origin << endl;
//   cout << "origin on plane = " << point_on_plane(origin) << endl;

//   outputfunc::enter_continue_char();
}

// ---------------------------------------------------------------------
// Member function reset_ahat takes in threevector a_vec which is
// assumed to already lie within the exisiting a_hat - b_hat plane.
// It resets a_hat equal to the unitvector defined by a_vec, and
// modifies b_hat appropriately.  

void plane::reset_ahat(const threevector& a_vec)
{
   a_hat=a_vec.unitvector();
   b_hat=n_hat.cross(a_hat);
}

// ---------------------------------------------------------------------
// Member function compute_extremal_planar_coords takes in STL vector
// Vworld containing a set of XYZ points.  After transforming the
// points' world space coordinates to planar coordinates, this method
// computes a_min, a_max, b_min, b_max, n_min and n_max.

void plane::compute_extremal_planar_coords(const vector<threevector>& Vworld) 
{
   vector<threevector>* Vplanar_ptr=coords_wrt_plane(Vworld);
   for (unsigned int i=0; i<Vplanar_ptr->size(); i++)
   {
      threevector curr_abn( (*Vplanar_ptr)[i] );
      a_min=basic_math::min(a_min,curr_abn.get(0));
      a_max=basic_math::max(a_max,curr_abn.get(0));
      b_min=basic_math::min(b_min,curr_abn.get(1));
      b_max=basic_math::max(b_max,curr_abn.get(1));
      n_min=basic_math::min(n_min,curr_abn.get(2));
      n_max=basic_math::max(n_max,curr_abn.get(3));
   }
   delete Vplanar_ptr;

   cout << "a_min = " << a_min << " a_max = " << a_max << endl;
   cout << "b_min = " << b_min << " b_max = " << b_max << endl;
   cout << "n_min = " << n_min << " n_max = " << n_max << endl;
}

// ---------------------------------------------------------------------
// Member function parity_flip_2D_coord_system negates the directions
// of the a_hat and n_hat axes.  This parity flip still retains right
// handedness for the a_hat, b_hat and n_hat coordinate system.

void plane::parity_flip_2D_coord_system()
{
   a_hat *= -1;
   n_hat *= -1;

   pi.put(0,-pi.get(0));
   pi.put(1,-pi.get(1));
   pi.put(2,-pi.get(2));
   pi.put(3,-pi.get(3));
}

// ---------------------------------------------------------------------
// Member function planar_coords takes in a threevector V.  If it lies
// on the current plane object, this method returns its alpha and beta
// coordinates relative to the plane's origin.

bool plane::planar_coords(const threevector& V,twovector& Vplanar) const
{
   if (point_on_plane(V))
   {
      threevector delta(V-origin);
      double a=delta.dot(a_hat);
      double b=delta.dot(b_hat);
      Vplanar=twovector(a,b);
      return true;
   }
   return false;
}

vector<twovector> plane::planar_coords(const vector<threevector>& V3) const
{
   twovector curr_V2;
   vector<twovector> V2;
   for (unsigned int i=0; i<V3.size(); i++)
   {
      if (planar_coords(V3[i],curr_V2))
      {
         V2.push_back(curr_V2);
      }
   }
   return V2;
}

// =========================================================================
// Intersection methods
// =========================================================================

// Member function infinite_line_intersection() computes the
// intersection of an infinite line specified by its basepoint and
// direction vector with the current infinite plane object.  This
// boolean method returns false if the line's direction vector r_hat
// is orthogonal to the plane's normal vector.

bool plane::infinite_line_intersection(
   const threevector& ray_basept,const threevector& r_hat,
   threevector& intersection_pt) const
{
//   cout << "inside plane::infinite_line_intersection()" << endl;
//   cout << "ray_basept = " << ray_basept << endl;
//   cout << "r_hat = " << r_hat << endl;
//   cout << "n_hat = " << n_hat << endl;
//   cout << "signed distance of basept from plane = "
//        << signed_distance_from_plane(ray_basept) << endl;

   if (nearly_equal(fabs(n_hat.dot(r_hat)),0))
   {
      return false;
   }

   double numer=-signed_distance_from_plane(ray_basept);
   double denom=n_hat.dot(r_hat);
//   double lambda=numer/denom;
//   intersection_pt=ray_basept+lambda*r_hat;
   intersection_pt=ray_basept+numer/denom*r_hat;

//   cout << "ray_basept = " << ray_basept << endl;
//   cout << "r_hat = " << r_hat << endl;
//   cout << "numer = " << numer << " denom = " << denom
//        << " lambda = " << lambda << endl;
//   cout << "intersection_pt = " << intersection_pt << endl;
//   cout << "point_on_plane(intersection_pt) = "
//        << point_on_plane(intersection_pt) << endl;

   return true;
}

// ---------------------------------------------------------------------
// Method ray_intersection() returns true if the semi-infinite ray
// defined by its base point and direction vector r_hat intersects the
// current infinite plane.  Otherwise, if the ray is oriented away
// from the plane or is orthogonal to its normal vector, this boolean
// method returns false.

bool plane::ray_intersection(
   const threevector& ray_basept,const threevector& r_hat,
   threevector& intersection_pt) const
{
   if (!infinite_line_intersection(ray_basept,r_hat,intersection_pt))
      return false;

//   cout << "ray_basepoint = " << ray_basept << endl;
//   cout << "r_hat = " << r_hat << endl;
//   cout << "intersection_pt = " << intersection_pt << endl;

/*   
   threevector test_dir((intersection_pt-ray_basept).unitvector());
//   double dotproduct=test_dir.dot(r_hat);
//   cout << "dotproduct = " << dotproduct << endl;
//   return nearly_equal(dotproduct,1);

   return nearly_equal(test_dir.dot(r_hat),1);
*/

   return ((intersection_pt-ray_basept).dot(r_hat) > 0);
}

// ---------------------------------------------------------------------
// This overloaded version of ray_intersection simply returns a
// boolean flag.  

bool plane::ray_intersection(
   const threevector& ray_basept,const threevector& r_hat) const
{
   threevector intersection_pt;
   return ray_intersection(ray_basept,r_hat,intersection_pt);
}

// ---------------------------------------------------------------------
// Boolean member function linesegment_intersection() returns true
// [false] if the input line segment does [not] intersect the current
// plane.  If the segment does intersect, the intersection point's
// location on the plane is also returned.  

bool plane::linesegment_intersection(
   const linesegment& l,threevector& intersection_pnt)
{
   if (infinite_line_intersection(
      l.get_v1(),l.get_ehat(),intersection_pnt))
   {
      if (l.point_on_segment(intersection_pnt)) return true;
   }
   return false;
}

// ---------------------------------------------------------------------
// Boolean member function polygon_intersection_query() returns true
// [false] if the input polygon does [not] intersect the current
// plane.

bool plane::polygon_intersection_query(const polygon& poly)
{
//   cout << "inside plane::polygon_intersection_query()" << endl;
   
   bool poly_intersects_plane_flag=false;
   int prev_sgn=0;
   for (unsigned int v=0; v<poly.get_nvertices(); v++)
   {
      threevector curr_vertex=poly.get_vertex(v);
//      cout << "v = " << v << " curr_vertex = " << curr_vertex << endl;
      if (point_on_plane(curr_vertex)) continue;

      double curr_distance=signed_distance_from_plane(curr_vertex);
      int curr_sgn=sgn(curr_distance);

      if (v > 0)
      {
         if (curr_sgn != prev_sgn && prev_sgn != 0)
         {
            poly_intersects_plane_flag=true;
            break;
         }
      }
      prev_sgn=curr_sgn;
   } // loop over index v labeling polygon vertices

//   cout << "At end of polygon_intersection_query(), poly_intersects_plan_flag="
//        << poly_intersects_plane_flag << endl;
   
   return poly_intersects_plane_flag;
}

// ---------------------------------------------------------------------
// Member function polygon_side_query() takes in a polygon which is
// assumed to fully lie on either the positive or negative side of the
// current plane object.  It returns +1 or -1 to indicate the plane's
// side on which the polygon lies.  If the polygon lies within the
// plane, this method returns 0.

int plane::polygon_side_query(const polygon& poly) const
{
//   cout << "inside plane::polygon_side_query()" << endl;

   for (unsigned int v=0; v<poly.get_nvertices(); v++)
   {
      if (point_on_plane(poly.get_vertex(v))) continue;
      double signed_distance=signed_distance_from_plane(
         poly.get_vertex(v));
      return sgn(signed_distance);
   } // loop over index v labeling polygon vertices

//   cout << "Polygon lies inside plane!" << endl;
   return 0;
}

// ---------------------------------------------------------------------
// Member function decompose_intersecting_triangle() takes in
// a triangle which is assumed to nontrivially intersect the current
// plane object.  It returns the directed linesegment intersection
// between the plane and the input triangle.  It also computes and
// returns the decomposition of the input triangle into a triangle
// that lies on one side of the plane and a triangle/quadrilateral
// that lies on the other side of the plane. Finally, this method
// returns the sgn relative to the plane of the decomposed, smaller
// triangle's vertex which lies outside the plane.

int plane::decompose_intersecting_triangle(
   const polygon& triangle,linesegment& intersection_segment,
   polygon& intersection_triangle,polygon& intersection_tri_or_quad)
{
//   cout << "======================================================" << endl;
//   cout << "inside plane::decompose_intersecting_triangle()" << endl;
//   cout << "triangle = " << triangle << endl;
 
   linesegment l01(triangle.get_vertex(0),triangle.get_vertex(1));
   linesegment l12(triangle.get_vertex(1),triangle.get_vertex(2));
   linesegment l20(triangle.get_vertex(2),triangle.get_vertex(0));
   vector<linesegment> segments;
   segments.push_back(l01);
   segments.push_back(l12);
   segments.push_back(l20);

//   cout << "l01 = " << l01 << endl;
//   cout << "l12 = " << l12 << endl;
//   cout << "l20 = " << l20 << endl;

   threevector curr_intersection_pnt;
   vector<bool> intersection_flags;
   vector<threevector> intersection_points;
   for (unsigned int s=0; s<3; s++)
   {
      intersection_flags.push_back(linesegment_intersection(
         segments[s],curr_intersection_pnt));
      intersection_points.push_back(curr_intersection_pnt);
//      cout << "s = " << s
//           << " intersection flag = " << intersection_flags.back() 
//           << " intersection_pt = " << curr_intersection_pnt << endl;
   }

   int start_index,stop_index;
   for (unsigned int s=0; s<3; s++)
   {
      start_index=s;
      stop_index=modulo(s+1,3);
      if (intersection_flags[start_index] && intersection_flags[stop_index])
      {
         break;
      }
   }

//   cout << "start_index = " << start_index
//        << " stop_index = " << stop_index << endl;

   threevector V1=intersection_points[start_index];
   threevector V2=intersection_points[stop_index];
   intersection_segment=linesegment(V1,V2);

   threevector V3=triangle.get_vertex(modulo(stop_index+1,3));
   threevector V4=triangle.get_vertex(modulo(stop_index+2,3));

   threevector V5=triangle.get_vertex(modulo(stop_index,3));

   vector<threevector> quadrilateral_vertices;
   quadrilateral_vertices.push_back(V1);
   quadrilateral_vertices.push_back(V2);
   if (!V2.nearly_equal(V3))
   {
      quadrilateral_vertices.push_back(V3);
   }
   if (!V1.nearly_equal(V4))
   {
      quadrilateral_vertices.push_back(V4);
   }
   intersection_tri_or_quad=polygon(quadrilateral_vertices);
//   cout << "intersection_tri_or_quad = "
//        << intersection_tri_or_quad << endl;

   vector<threevector> triangle_vertices;
   triangle_vertices.push_back(V1);
   triangle_vertices.push_back(V5);
   triangle_vertices.push_back(V2);
   intersection_triangle=polygon(triangle_vertices);
//   cout << "intersection_triangle = " 
//        << intersection_triangle << endl;

   int intersection_triangle_sgn=sgn(signed_distance_from_plane(V5));
   return intersection_triangle_sgn;
}

// ---------------------------------------------------------------------
// This overloaded version of member function
// decompose_intersecting_triangle() takes in
// a triangle which is assumed to nontrivially intersect the current
// plane object.  It also takes in integer plane_side= +1 [-1] which
// indicates the side of the plane parallel [anti-parallel] to the
// plane's normal direction vector.  It returns either a triangle or
// quadrilateral which represents the part of the input triangle lying
// on the specified side of the current plane.

polygon plane::decompose_intersecting_triangle(
   const polygon& triangle,int plane_side)
{
   linesegment intersection_segment;
   polygon intersection_triangle,intersection_tri_or_quad;
   int intersection_triangle_sgn=decompose_intersecting_triangle(
      triangle,intersection_segment,intersection_triangle,
      intersection_tri_or_quad);

   if (plane_side==intersection_triangle_sgn)
   {
      return intersection_triangle;
   }
   else
   {
      return intersection_tri_or_quad;
   }
}

// ---------------------------------------------------------------------
// Member function polygon_above_or_below_plane() is a high-level
// method which takes in a polygon and integer plane_side_sgn
// indicating the plane side parallel or antiparallel to the plane's
// normal direction vector.  It returns a pointer to the polygon which
// resides completely on the specified side of the plane.  The
// returned polygon may equal the input one, some partial
// piece of the input polygon, or NULL.

polygon* plane::polygon_above_or_below_plane(
   polygon& poly,int plane_side_sgn)
{
   if (polygon_intersection_query(poly))
   {
      return polygon_cleaved_by_plane(poly,plane_side_sgn);
   }
   else
   {
      int side_sgn=polygon_side_query(poly);
      if (side_sgn==plane_side_sgn)
      {
         return &poly;
      }
      else
      {
         return NULL;
      }
   }
}

// ---------------------------------------------------------------------
// Member function polygon_cleaved_by_plane() takes in polygon poly
// which is assumed to nontrivially intersect the current plane
// object.  The input polygon is triangulated, and each of its
// triangles are decomposed into pieces lying on the side of the plane
// specified by input integer plane_side_sgn.  The parts of the
// polygon triangles lying on the desired plane are slightly inflated.
// Their union is subsequently calculated via General Polygon Clipper
// (GPC) algorithms.  Nearly parallel edges within the union
// polygon are coalesced.  Finally, the consolidated polygon is turned
// into a dynamically instantiated polygon whose pointer is returned
// by this method.

polygon* plane::polygon_cleaved_by_plane(polygon& poly,int plane_side_sgn)
{
//   cout << "inside plane::polygon_cleaved_by_plane()" << endl;

//   cout << "polygon.n_hat = " << poly.get_normal() << endl;
   poly.generate_interior_triangles();

   triangles_group* triangles_group_ptr=poly.get_triangles_group_ptr();
   vector<polygon> polys_cleaved_by_plane;
   for (unsigned int t=0; t<triangles_group_ptr->get_n_triangles(); t++)
   {
      triangle* triangle_ptr=triangles_group_ptr->get_triangle_ptr(t);
      polygon* triangle_poly_ptr=triangle_ptr->get_triangle_poly_ptr();
//      cout << "t = " << t 
//           << " *triangle_poly_ptr = " << *triangle_poly_ptr << endl;

      bool intersection_flag=
         polygon_intersection_query(*triangle_poly_ptr);
//      cout << "intersection flag = " << intersection_flag << endl << endl;

      if (intersection_flag)
      {
         polys_cleaved_by_plane.push_back(
            decompose_intersecting_triangle(
               *triangle_poly_ptr,plane_side_sgn));
      }
      else
      {
         if (plane_side_sgn==polygon_side_query(*triangle_poly_ptr))
         {
            polys_cleaved_by_plane.push_back(*triangle_poly_ptr);
         }
      }
   } // loop over index t labeling triangles within *triangles_group_ptr

// Slightly inflate polygons above or below plane prior to forming
// their union:

   double tolerance=1E-5;
   const double scale_factor=1+tolerance;

   threevector n_hat;
   threevector super_COM=Zero_vector;
   vector<threevector> poly_COMs;
   for (unsigned int p=0; p<polys_cleaved_by_plane.size(); p++)
   {
      poly_COMs.push_back(polys_cleaved_by_plane[p].compute_COM());
      super_COM += poly_COMs.back();
      polys_cleaved_by_plane[p].scale(
         poly_COMs.back(),scale_factor);
      n_hat=polys_cleaved_by_plane[p].get_normal();
//      cout << "p = " << p
//           <<  " n_hat = " << n_hat << endl;
   }
   super_COM /= poly_COMs.size();
//   cout << "super_COM = " << super_COM << endl;

// Before performing union operation, we first translate all polys so
// that their super_COM-->(0,0,0).  Then rotate each polygon about
// (0,0,0) so that they each lie within Z=0 XY plane:

   rotation R;
   R=R.rotation_taking_u_to_v(n_hat,z_hat);
//   cout << "R = " << R << endl;
   for (unsigned int p=0; p<polys_cleaved_by_plane.size(); p++)
   {
      polys_cleaved_by_plane[p].translate(-super_COM);      
      polys_cleaved_by_plane[p].rotate(Zero_vector,R);      
   }

   vector<polygon> polys_cleaved_by_plane_union=
      geometry_func::polygon_union(polys_cleaved_by_plane);
   
   polygon candidate_poly_cleaved_by_plane=
      polys_cleaved_by_plane_union.front();
   vector<threevector> unique_union_vertices;
   for (unsigned int v=0; v<candidate_poly_cleaved_by_plane.get_nvertices(); v++)
   {
      threevector curr_vertex=candidate_poly_cleaved_by_plane.
         get_vertex(v);
      if (v==0)
      {
         unique_union_vertices.push_back(curr_vertex);
      }
      else
      {
         if (!curr_vertex.nearly_equal(
            unique_union_vertices.back(),10*tolerance) &&
         !curr_vertex.nearly_equal(
            unique_union_vertices.front(),10*tolerance))
         {
            unique_union_vertices.push_back(curr_vertex);
         }
      }
   } // loop over index v
   
   polygon poly_cleaved_by_plane(unique_union_vertices);
   poly_cleaved_by_plane.eliminate_all_parallel_edges();

// After performing union operation, rotate poly_cleaved_by_plane back
// from Z=0 XY plane to original plane.  Then translate by super_COM:
   
   poly_cleaved_by_plane.rotate(Zero_vector,R.transpose());      
   poly_cleaved_by_plane.translate(super_COM);

   polygon* consolidated_poly_ptr=new polygon(poly_cleaved_by_plane);
//   cout << "poly_cleaved_by_plane = "  << *consolidated_poly_ptr << endl;
   return consolidated_poly_ptr;
}

// =========================================================================
// World <--> planar coordinate system transforatmion methods
// =========================================================================

threevector plane::coords_wrt_plane(const threevector& Vworld) const
{
//   cout << "inside plane::coords_wrt_plane, Vworld = " << Vworld << endl;
//   cout << "origin =  "<< origin << endl;
//   cout << "a_hat = " << a_hat 
//        << " b_hat = " << b_hat
//        << " n_hat = " << n_hat << endl;
   threevector delta(Vworld-origin);
   double a=delta.dot(a_hat);
   double b=delta.dot(b_hat);
   double n=delta.dot(n_hat);
   return threevector(a,b,n);
}

vector<threevector>* plane::coords_wrt_plane(
   const vector<threevector>& Vworld) const
{
   vector<threevector>* V_ptr=new vector<threevector>;
   for (unsigned int i=0; i<Vworld.size(); i++)
   {
      V_ptr->push_back(coords_wrt_plane(Vworld[i]));
   }
   return V_ptr;
}

vector<threevector>* plane::coords_wrt_plane(
   const vector<fourvector>& Vworld) const
{
   vector<threevector>* V_ptr=new vector<threevector>;
   for (unsigned int i=0; i<Vworld.size(); i++)
   {
      V_ptr->push_back(coords_wrt_plane(threevector(Vworld[i])));
   }
   return V_ptr;
}

// ---------------------------------------------------------------------
// Member function world_coords takes in a twovector Vplanar.  It
// converts the input vectors (alpha,beta) coordinates to (X,Y,Z)
// world coordinates.

threevector plane::world_coords(const twovector& Vplanar)
{
   double a=Vplanar.get(0);
   double b=Vplanar.get(1);
   return origin+a*a_hat+b*b_hat;
}

threevector plane::world_coords(const threevector& Vplanar)
{
   double a=Vplanar.get(0);
   double b=Vplanar.get(1);
   double n=Vplanar.get(2);
   return origin+a*a_hat+b*b_hat+n*n_hat;
}

// =========================================================================
// RANSAC construction member functions
// =========================================================================

// Member function renormalize_input_points() takes in a set of
// threevectors within V_input.  It computes the median location of
// the input threevectors which is defined to be the points origin.
// This method next computes the points' residuals relative to the
// origin.  The residuals are rescaled by the medians of their
// magnitudes relative to the origin.  The renormalized residuals are
// returned by this method.

vector<threevector> plane::renormalize_input_points(
   const vector<threevector>& V_input)
{
//   cout << "inside plane::renormalize_input_points()" << endl;

// First compute median of input points.  Take this median location as
// a new origin:

   vector<double> X,Y,Z;
   for (unsigned int n=0; n<V_input.size(); n++)
   {
      threevector curr_V(V_input[n]);
      X.push_back(curr_V.get(0));
      Y.push_back(curr_V.get(1));
      Z.push_back(curr_V.get(2));
   } // loop over index n labeling input points
   double X_median=mathfunc::median_value(X);
   double Y_median=mathfunc::median_value(Y);
   double Z_median=mathfunc::median_value(Z);
   threevector origin(X_median,Y_median,Z_median);

// Compute residuals relative to origin. Then compute the median of
// the residuals' absolute values:

   X.clear();
   Y.clear();
   Z.clear();
   vector<threevector> V_residual;   
   for (unsigned int n=0; n<V_input.size(); n++)
   {
      V_residual.push_back(V_input[n]-origin);
      X.push_back(fabs(V_residual[n].get(0)));
      Y.push_back(fabs(V_residual[n].get(1)));
      Z.push_back(fabs(V_residual[n].get(2)));
   }
   
   double sigma_X=mathfunc::median_value(X);
   double sigma_Y=mathfunc::median_value(Y);
   double sigma_Z=mathfunc::median_value(Z);

// Rescale residuals by sigma values:

   for (unsigned int n=0; n<V_input.size(); n++)
   {
      threevector curr_V=V_residual[n];
      V_residual[n]=threevector(curr_V.get(0)/sigma_X,
                                curr_V.get(1)/sigma_Y,
                                curr_V.get(2)/sigma_Z);
   }
   return V_residual;
}

// ---------------------------------------------------------------------
// Member function compute_candidate_plane() takes in a set of
// threevectors and randomly selects 3 points.  If all 3 threevectors
// are distinct, this method set *this equal to the plane which passes
// through them and returns true.  Otherwise, this boolean method
// returns false.  

bool plane::compute_candidate_plane(const vector<threevector>& V)
{
//   cout << "inside plane::compute_candidate_plane()" << endl;
   int i1=basic_math::mytruncate(V.size()*0.99*nrfunc::ran1());

   int i2,i3;
   do
   {
      i2=basic_math::mytruncate(V.size()*0.99*nrfunc::ran1());
   }
   while (i2 == i1);
   
   do
   {
      i3=basic_math::mytruncate(V.size()*0.99*nrfunc::ran1());
   }
   while (i3 == i1 || i3 == i2);

   threevector V1=V[i1];
   threevector V2=V[i2];
   threevector V3=V[i3];

//   cout << "i1 = " << i1 << " i2 = " << i2 << " i3 = " << i3 << endl;
//   cout << "V1 = " << V1 << " V2 = " << V2 << " V3 = " << V3 << endl;
   
   if (V1.nearly_equal(V2) || V2.nearly_equal(V3) || V3.nearly_equal(V1))
   {
      return false;
   }
   
   *this=plane(V1,V2,V3);

//   cout << "*this = " << *this << endl;
   return true;
}

// ---------------------------------------------------------------------
// Member function identify_inlier_indices() loops over all
// threevectors within input STL vector V.

vector<int> plane::identify_inliers_indices(
   double max_delta,const vector<threevector>& V)
{
//   cout << "inside plane::identify_inliers()" << endl;

   vector<int> inlier_indices;
   for (unsigned int i=0; i<V.size(); i++)
   {
      if (fabs(signed_distance_from_plane(V[i])) < max_delta)
      {
         inlier_indices.push_back(i);
      }
   } // loop over index i labeling threevectors
   return inlier_indices;
}
   
// ---------------------------------------------------------------------
// Member function RANSAC_fit_to_points()

void plane::RANSAC_fit_to_points(
   double max_delta,const vector<threevector>& V_input,
   unsigned int max_n_iters)
{
   cout << "inside plane::RANSAC_fit_to_points()" << endl;
   vector<threevector> V_renormalized=renormalize_input_points(V_input);

   unsigned int max_n_inliers=0;
   unsigned int n_iters=0;

   vector<int> inlier_indices;
   plane candidate_plane;

   while (n_iters < max_n_iters)
   {
      if (n_iters%100==0) cout << n_iters/100 << " " << flush;

      candidate_plane.compute_candidate_plane(V_renormalized);
      vector<int> candidate_inlier_indices=
         candidate_plane.identify_inliers_indices(
         max_delta,V_renormalized);

      if (candidate_inlier_indices.size() > max_n_inliers)
      {
         max_n_inliers=candidate_inlier_indices.size();
//         cout << "max_n_inliers = " << max_n_inliers << endl;
         
         inlier_indices.clear();
         for (unsigned int i=0; i<max_n_inliers; i++)
         {
            inlier_indices.push_back(candidate_inlier_indices[i]);
         }
      }
      n_iters++;
   }
   cout << endl;

   vector<threevector> inlier_V;
   for (unsigned int i=0; i<inlier_indices.size(); i++)
   {
      inlier_V.push_back(V_input[inlier_indices[i]]);
   }
   *this=plane(inlier_V);
}

// =========================================================================
// Ground plane estimation member functions
// =========================================================================

// Member function ground_plane_origin() takes in normal vector n_hat
// for some candidate ground plane.  It computes the n coordinate for
// each point within STL vector V_input.  The threevector with the
// minimal n coordinate value is returned as a best guess for a ground
// plane origin.

threevector plane::find_ground_plane_origin(
   const threevector& n_hat,const vector<threevector>& V_input)
{
   cout << "inside plane::find_ground_plane()" << endl;
   
   threevector origin=Zero_vector;

   unsigned int n_points=V_input.size();
   double n_min=POSITIVEINFINITY;
   for (unsigned int i=0; i<n_points; i++)
   {
      threevector curr_V(V_input[i]);
      double n=curr_V.dot(n_hat);
      if (n < n_min)
      {
         n_min=n;
         origin=curr_V;
      }
   } // loop over index i labeling input points

   return origin;
}

// ---------------------------------------------------------------------
// Member function estimate_ground_plane() performs a brute-force search
// over normal direction vector angles theta and phi.  For each
// candidate ground plane formed from guesses for its origin
// and its normal, this method evaluates the signed distance of all
// threevectors witin V_input to the plane.  The parameters for the
// plane for which this distnace score function is minimized are
// returned.

void plane::estimate_ground_plane(const vector<threevector>& V_input)
{
   cout << "inside plane::estimate_ground_plane()" << endl;
   
   unsigned int n_points=V_input.size();

// FAKE FAKE:  Thurs, Dec 9 at 1 pm
// Hardwire origin for ground plane for Sailplane PLY dataset...

//   threevector origin=find_ground_plane_origin(z_hat,V_input);

   threevector origin(-2.37972281777,-0.448571067573,-3.16115999222);
   threevector orig_origin=origin;


   double theta_start=1*PI/180;
   double theta_stop=89*PI/180;
   int n_theta_bins=10;

   double phi_start=0;
   double phi_stop=360*PI/180;
   int n_phi_bins=10;

   double min_distance=POSITIVEINFINITY;

   param_range theta(theta_start,theta_stop,n_theta_bins);
   param_range phi(phi_start,phi_stop,n_phi_bins);
   unsigned int n_iters=5;

   for (unsigned int iter=0; iter<n_iters; iter++)
   {
      outputfunc::newline();
      cout << "Iteration = " << iter << endl;

      while (theta.prepare_next_value())
      {
         double curr_theta=theta.get_value();
         while (phi.prepare_next_value())
         {
            double curr_phi=phi.get_value();

            cout << "iter = " << iter
                 << " theta = " << curr_theta*180/PI 
                 << " phi = " << curr_phi*180/PI << endl;

            n_hat=threevector(
               sin(curr_theta)*cos(curr_phi),sin(curr_theta)*sin(curr_phi),
               cos(curr_theta));
            *this=plane(n_hat,origin);
            origin=orig_origin;

//         cout << "*this = " << *this << endl;
   
            double avg_distance=0;
            for (unsigned int n=0; n<n_points; n++)
            {
               threevector curr_V(V_input[n]);
               double curr_distance=signed_distance_from_plane(curr_V);
               
               if (curr_distance > 0) 
               {
                  avg_distance += curr_distance;
               }
               else
               {
                  avg_distance += 2*fabs(curr_distance);
               }

            } // loop over index n labeling input points
            avg_distance /= n_points;

            cout << "Average distance = " << avg_distance << endl;
            if (avg_distance < min_distance)
            {
               min_distance=avg_distance;
               theta.set_best_value();
               phi.set_best_value();
               cout << "******************************************" << endl;
               cout << "Min avgerage distance = " << min_distance
                    << " best_theta = " << theta.get_best_value()*180/PI
                    << " best_phi = " << phi.get_best_value()*180/PI << endl;
               cout << "origin = " << origin << endl;
            }

         } // phi while loop 
      } // theta while loop 

// Recompute new estimate for ground plane origin using new estimate
// for plane's normal direction vector:

      double best_theta=theta.get_best_value();
      double best_phi=phi.get_best_value();

      n_hat=threevector(
         sin(best_theta)*cos(best_phi),sin(best_theta)*sin(best_phi),
         cos(best_theta));

      origin=find_ground_plane_origin(n_hat,V_input);
      orig_origin=origin;

// Refine search for ground plane's normal in next iteration:

      double frac=0.5;
      theta.shrink_search_interval(
         theta.get_best_value(),theta_start,theta_stop,frac);
      phi.shrink_search_interval(
         phi.get_best_value(),phi_start,phi_stop,frac);

   } // loop over iter index

   cout << "========================================================" << endl;
   cout << "min_distance = " << min_distance
        << " best_theta = " << theta.get_best_value()*180/PI
        << " best_phi = " << phi.get_best_value()*180/PI << endl;
   cout << "ground plane origin = " << orig_origin << endl;
   cout << "========================================================" << endl;

   *this=plane(n_hat,origin);
}

// ---------------------------------------------------------------------
// Member function compute_bbox_pn_vertices() calculates the positive
// and negative far points of a bounding box corresponding to the
// current plane and its normal direction vector.  The p_vertex has a
// greater signed distance from the plane than the n_vertex.

void plane::compute_bbox_pn_vertices(
   const bounding_box* bbox_ptr,threevector& p_vertex,threevector& n_vertex) 
           const
{
   vector<linesegment> diagonals=bbox_ptr->get_bbox_diagonals();
   
// Find bbox diagonal which is most closely aligned with plane's
// normal:

   vector<int> diagonal_indices;
   vector<double> abs_dotproducts;
   for (unsigned int d=0; d<diagonals.size(); d++)
   {
      diagonal_indices.push_back(d);
      double dotproduct=n_hat.dot(diagonals[d].get_ehat());
      abs_dotproducts.push_back(fabs(dotproduct));
   } // loop over index d labeling bbox diagonals

   templatefunc::Quicksort(abs_dotproducts,diagonal_indices);
   linesegment fitted_diagonal=diagonals[diagonal_indices.back()];

   if (signed_distance_from_plane(fitted_diagonal.get_v1()) > 
   signed_distance_from_plane(fitted_diagonal.get_v2()) )
   {
      p_vertex=fitted_diagonal.get_v1();
      n_vertex=fitted_diagonal.get_v2();
   }
   else
   {
      n_vertex=fitted_diagonal.get_v1();
      p_vertex=fitted_diagonal.get_v2();
   }
}

