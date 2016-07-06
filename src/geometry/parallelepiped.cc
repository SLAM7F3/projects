// ==========================================================================
// Parallelepiped class member function definitions
// ==========================================================================
// Last modified on 3/17/09; 12/4/10; 1/29/12; 3/6/14; 4/4/14
// ==========================================================================

#include <iostream>
#include "math/basic_math.h"
#include "math/constant_vectors.h"
#include "templates/mytemplates.h"
#include "geometry/parallelepiped.h"
#include "geometry/parallelogram.h"
#include "math/threevector.h"

using std::ostream;
using std::cout;
using std::endl;
using std::vector;

const unsigned int parallelepiped::n_sidefaces=4;

// ==========================================================================
// Initialization, constructor and destructor methods:
// ==========================================================================

void parallelepiped::allocate_member_objects()
{
   side_face=new polygon[n_sidefaces];
}		       

void parallelepiped::initialize_member_objects()
{
}

// ---------------------------------------------------------------------
// Member function generate_top_and_side_faces takes in some constant
// translation vector W and uses it to generate the top and side faces
// from the bottom face which is assumed to already exist:

void parallelepiped::generate_top_and_side_faces(const threevector& W)
{
   vector<threevector> vertex(n_sidefaces);
   vertex[0]=bottom_face.get_vertex(0)+W;
   vertex[1]=bottom_face.get_vertex(3)+W;
   vertex[2]=bottom_face.get_vertex(2)+W;
   vertex[3]=bottom_face.get_vertex(1)+W;
   top_face=polygon(vertex);
   
   center=0.5*(top_face.vertex_average()+bottom_face.vertex_average());

//   cout << "top_face = " << top_face << endl;
//   cout << "bottom_face = " << bottom_face << endl;
//   cout << "center = " << center << endl;

   for (unsigned int i=0; i<n_sidefaces; i++)
   {
      vertex[0]=bottom_face.get_vertex(modulo(4-i,4));
      vertex[1]=bottom_face.get_vertex(3-i);
      vertex[2]=top_face.get_vertex(modulo(i+1,4));
      vertex[3]=top_face.get_vertex(i);
      side_face[i]=polygon(vertex);
   }
}

// ---------------------------------------------------------------------
parallelepiped::parallelepiped(void)
{
   allocate_member_objects();
   initialize_member_objects();
}

// This first constructor takes in an origin as well as vectors U, V
// and W which specify both the directions and sidelengths of the
// parallelepiped.

parallelepiped::parallelepiped(
   const threevector& U,const threevector& V,const threevector& W,
   const threevector& origin)
{
   allocate_member_objects();
   initialize_member_objects();

   vector<threevector> vertex(n_sidefaces);
   vertex[0]=origin;
   vertex[1]=vertex[0]+V;
   vertex[2]=vertex[1]+U;
   vertex[3]=vertex[2]-V;
   bottom_face=polygon(vertex);

   generate_top_and_side_faces(W);
}

parallelepiped::parallelepiped(
   const polygon& bface,const threevector& uhat,double h)
{
   allocate_member_objects();
   initialize_member_objects();

   bottom_face=bface;
   threevector W(h*uhat);
   generate_top_and_side_faces(W);
}

// This next constructor takes in 4 topface [bottomface] threevectors
// which are assumed to obey a right-hand rule ordering.  It forms a
// parallelepiped which represents a "best fit" to these input points:

parallelepiped::parallelepiped(
   const vector<threevector>& T,const vector<threevector>& B)
{
   threevector center=0.125*(T[0]+T[1]+T[2]+T[3] + B[0]+B[1]+B[2]+B[3]);
   threevector w=0.25 * ( (T[3]-T[2]) + (T[0]-T[1]) + (B[1]-B[2])
                          + (B[0]-B[3]) );
   threevector l=0.25 * ( (T[0]-T[3]) + (T[1]-T[2]) + (B[0]-B[1])
                          + (B[3]-B[2]) );
   threevector h=0.25 * ( (T[0]-B[0]) + (T[1]-B[3]) + (T[2]-B[2])
                          + (T[3]-B[1]) );

   cout << "center = " << center << endl;
   cout << "w = " << w << " l = " << l << " h = " << h << endl;
   
   vector<threevector> vertex(n_sidefaces);
   vertex[0]=threevector(center+0.5*(l+w-h));
   vertex[1]=threevector(center+0.5*(l-w-h));
   vertex[2]=threevector(center+0.5*(-l-w-h));
   vertex[3]=threevector(center+0.5*(-l+w-h));
   bottom_face=polygon(vertex);

   allocate_member_objects();
   initialize_member_objects();

   generate_top_and_side_faces(h);
}

// Copy constructor:

parallelepiped::parallelepiped(const parallelepiped& p)
{
   allocate_member_objects();
   docopy(p);
}

parallelepiped::~parallelepiped()
{
   delete [] side_face;
}

// ---------------------------------------------------------------------
void parallelepiped::docopy(const parallelepiped& b)
{
   width=b.width;
   length=b.length;
   height=b.height;
   what=b.what;
   lhat=b.lhat;
   hhat=b.hhat;
   center=b.center;
   top_face=b.top_face;
   bottom_face=b.bottom_face;
   
   for (unsigned int i=0; i<n_sidefaces; i++)
   {
      side_face[i]=b.side_face[i];
   }
}	

// Overload = operator:

parallelepiped parallelepiped::operator= (const parallelepiped& b)
{
   if (this==&b) return *this;
   docopy(b);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,parallelepiped& p)
{
   outstream << endl;
   outstream << "top face = " << p.top_face << endl;
   outstream << "bottom face = " << p.bottom_face << endl;
//   outstream << "center = " << p.center << endl;
   for (unsigned int i=0; i<p.n_sidefaces; i++)
   {
//      outstream << "side face " << i << ": " << p.side_face[i] << endl;
   }

   p.calculate_symmetry_vectors_and_lengths();
   cout << "width = " << p.width << " length = " << p.length
        << " height = " << p.height << endl;
   cout << "what = " << p.what << endl;
   cout << "lhat = " << p.lhat << endl;
   cout << "hhat = " << p.hhat << endl;

   cout << "angle(what,lhat) = " << acos(p.what.dot(p.lhat))*180/PI << endl;
   cout << "angle(lhat,hhat) = " << acos(p.lhat.dot(p.hhat))*180/PI << endl;
   cout << "angle(hhat,what) = " << acos(p.hhat.dot(p.what))*180/PI << endl;
   
   return(outstream);
}

// =====================================================================
// Parallelepiped properties member functions
// =====================================================================

// Member function calculate_symmetry_vectors_and_lengths sets the
// values of the unit vector member variables which point along the
// parallelepiped's width, length and height directions.  It also
// computes the parallelepiped's width, length and height.

void parallelepiped::calculate_symmetry_vectors_and_lengths()
{
   threevector wvec(top_face.get_vertex(3)-top_face.get_vertex(2));
   threevector lvec(top_face.get_vertex(0)-top_face.get_vertex(3));
   threevector hvec(top_face.get_vertex(0)-bottom_face.get_vertex(0));
   
   width=wvec.magnitude();
   length=lvec.magnitude();
   height=hvec.magnitude();

   half_width=0.5*width;
   half_length=0.5*length;
   half_height=0.5*height;

   what=wvec.unitvector();
   lhat=lvec.unitvector();
   hhat=hvec.unitvector();
}

// ---------------------------------------------------------------------
// Member function calculate_metric_and_its_inverse computes metric g
// for the current parallelepiped object and its matrix inverse:

void parallelepiped::calculate_metric_and_its_inverse()
{
   double a=what.dot(lhat);
   double b=what.dot(hhat);
   double c=lhat.dot(hhat);

   g.put(0,0,what.dot(what));
   g.put(0,1,a);
   g.put(0,2,b);
   g.put(1,0,g.get(0,1));
   g.put(1,1,lhat.dot(lhat));
   g.put(1,2,c);
   g.put(2,0,g.get(0,2));
   g.put(2,1,g.get(1,2));
   g.put(2,2,hhat.dot(hhat));

   double detg=1-a*a-b*b-c*c+2*a*b*c;

   ginv.put(0,0,(1-c*c)/detg);
   ginv.put(0,1,(b*c-a)/detg);
   ginv.put(0,2,(a*c-b)/detg);
   ginv.put(1,0,ginv.get(0,1));
   ginv.put(1,1,(1-b*b)/detg);
   ginv.put(1,2,(a*b-c)/detg);
   ginv.put(2,0,ginv.get(0,2));
   ginv.put(2,1,ginv.get(1,2));
   ginv.put(2,2,(1-a*a)/detg);
//   cout << "g = " << g << endl;
//   cout << "detg = " << detg << endl;
//   cout << "ginv = " << ginv << endl;
}

// =====================================================================
// Intersection member functions
// =====================================================================

// Member function point_location determines whether the input point
// lies inside, outside or on the current parallelepiped object.
// Input parameter delta_half_frac defines the small tolerance
// fraction within which points are declared to lie on the
// parallelepiped's surface.

void parallelepiped::point_location(
   const threevector& point,bool& point_inside,bool& point_outside,
   bool& point_on,double delta_half_frac)
{
   double w,l,h;
   calculate_wlh_coords(point,w,h,l);

   half_width=(0.5-delta_half_frac)*width;
   half_length=(0.5-delta_half_frac)*length;
   half_height=(0.5-delta_half_frac)*height;
   point_inside=(w > -half_width && w < half_width &&
                 l > -half_length && l < half_length &&
                 h > -half_height && h < half_height);

   half_width=(0.5+delta_half_frac)*width;
   half_length=(0.5+delta_half_frac)*length;
   half_height=(0.5+delta_half_frac)*height;
   point_outside=(w < -half_width || w > half_width ||
                  l < -half_length || l > half_length ||
                  h < -half_height || h > half_height) ;
   point_on=(!point_inside && !point_outside);
}

// ---------------------------------------------------------------------
// Member function linesegment_intersection returns the approximate
// fractional line segment lengths fstart and fstop between which the
// segment lies inside the current parallelepiped object:

void parallelepiped::linesegment_intersection(
   const linesegment& l,double& fstart,double& fstop)
{
//   cout << "inside parallelepiped::linesegment_intersection()" << endl;
//   cout << "fstart = " << fstart << " fstop = " << fstop << endl;
   
   const double SMALL_POSITIVE=0.001;
   const double dl=0.33;	// meters
   const int n_segment_bins=basic_math::round(l.get_length()/dl)+1;
   
   bool point_inside,point_outside,point_on;
   threevector dv(dl*l.get_ehat());
   
   int n=0;
   bool found_initial_pnt_inside=false;
   while(n<n_segment_bins && !found_initial_pnt_inside)
   {
      threevector v=l.get_v1()+n*dv;
      point_location(v,point_inside,point_outside,point_on);
      if (point_inside || point_on)
      {
         fstart=basic_math::min(fstart,double(n)/double(n_segment_bins-1));
         found_initial_pnt_inside=true;
      }
      n++;
   }

   if (found_initial_pnt_inside)
   {
      bool found_final_pnt_inside=false;
      while(n<n_segment_bins && !found_final_pnt_inside)
      {
         threevector v=l.get_v1()+n*dv;
         point_location(v,point_inside,point_outside,point_on);
         if (point_outside)
         {
            fstop=basic_math::max(fstop,double(n-1)/double(n_segment_bins-1));
            found_final_pnt_inside=true;
         }
         n++;
      }

// Line segment ends inside box:

      if (!found_final_pnt_inside) fstop=1;
   }

// If both difference between fstart and fstop is extremely small, we
// reset the values of fstart and fstop to 1 and 0, respectively, to
// indicate that linesegment l effective does NOT pierce the current
// parallelepiped object:

   if (fabs(fstart-fstop) < SMALL_POSITIVE) 
   {
      fstart=1;
      fstop=0;
   }

//   cout << "At end of parallelepiped::linesegment_intersection()" << endl;
//   cout << "fstart = " << fstart << endl;
//   cout << "fstop = " << fstop << endl;
}

// ---------------------------------------------------------------------
// Member function polygon_edges_intersection returns arrays
// fstart and fstop of fractional line segment lengths between which
// the polygon edges lie inside the current box object:

void parallelepiped::polygon_edges_intersection(
   const polygon& poly,double fstart[],double fstop[])
{
//   const double SMALL_POSITIVE=0.001;
   
   for (unsigned int n=0; n<poly.get_nvertices(); n++)
   {
      linesegment l(poly.get_vertex(n),poly.get_vertex(
         modulo(n+1,poly.get_nvertices())));
      linesegment_intersection(l,fstart[n],fstop[n]);
//      if (fstart[n] < SMALL_POSITIVE && fstop[n] < SMALL_POSITIVE)
//      {
//         cout << "Problem in parallelepiped::polygon_edges_intersection() !"
//              << endl;
//         cout << "n = " << n << " fstart = " << fstart[n]
//              << " fstop = " << fstop[n] << endl;
//      }
   }
}

// ---------------------------------------------------------------------
// Member function ray_pierces_me takes in the basepoint and direction
// vector for some ray.  It checks whether the ray pierces the
// parallelepiped's top or side faces.  If so, this boolean method
// returns true.

bool parallelepiped::ray_pierces_me(
   double tan_elevation,const threevector& ray_basepoint,
   const threevector& ray_hat)
{

// First test whether ray pierces parallelepiped's top face:
   
   threevector pnt_in_plane;

/*   
   if (top_face.ray_projected_into_poly_plane(
      ray_basepoint,ray_hat,top_face.vertex_average(),pnt_in_plane))
   {
      if (top_face.point_inside_polygon(pnt_in_plane))
      {
         return true;
      }
   }
*/
 
// Test whether ray pierces any of the parallelepiped's side faces:

   for (unsigned int i=0; i<n_sidefaces; i++)
   {
      double sqrd_basepoint_sideface_dist=
         (ray_basepoint-side_face[i].get_vertex_avg()).sqrd_magnitude();
      double max_shadow_length_on_ground=height/tan_elevation;
      if (9*sqr(max_shadow_length_on_ground) > sqrd_basepoint_sideface_dist)
      {
         if (side_face[i].ray_projected_into_poly_plane(
            ray_basepoint,ray_hat,side_face[i].vertex_average(),pnt_in_plane))
         {
            if (side_face[i].point_inside_polygon(pnt_in_plane))
            {
               return true;
            }
         }
      } // max_shadow_length_on_ground > basepoint_sideface_dist conditional
   } // loop over index i labeling side faces
   
   return false;
}

// ---------------------------------------------------------------------
// Member function wrap_bbox_around_segment() takes in threevectors V1
// and V2 which we generally assume have different Z heights.  It
// projects the ray running from V1 to V2 into the XY plane.  This
// method constructs parallelepiped *this which fits snugly around the
// ray.  

// We wrote this little utility in Mar 2009 for ray tracing purposes.

void parallelepiped::wrap_bbox_around_segment(
   const threevector& V1,const threevector& V2,double width)
{
   threevector delta=V2-V1;
   delta.put(2,0);
   threevector e_hat=delta.unitvector();
   threevector f_hat=z_hat.cross(e_hat);

   threevector origin=V1-0.5*width*f_hat;

   threevector U=delta.magnitude()*e_hat;
   threevector V=width*f_hat;
   threevector W=(V2-V1).get(2)*z_hat;

   *this=parallelepiped(U,V,W,origin);
   cout << "wrapping parallelepiped = " << *this << endl;
}

// ==========================================================================
// Projection methods
// ==========================================================================

// Member function XYZ_vertex_positions loops over the corners of the
// top and bottom faces of the current parallelepiped object.  It
// fills an STL vector with these vertices' 3D positions:

void parallelepiped::XYZ_vertex_positions(vector<threevector>& vertex_posn)
{
   for (unsigned int n=0; n<top_face.get_nvertices(); n++)
   {
      vertex_posn.push_back(top_face.get_vertex(n));
   } // loop over index n labeling cube vertices
   for (unsigned int n=0; n<bottom_face.get_nvertices(); n++)
   {
      vertex_posn.push_back(bottom_face.get_vertex(n));
   } // loop over index n labeling cube vertices

//   templatefunc::printVector(proj_vertices);
}

// ---------------------------------------------------------------------
// Member function XY_vertex_projections loops over the corners of the
// top and bottom faces of the current parallelepiped object.  It
// fills an STL vector with these vertices' projections into the XY
// plane.

void parallelepiped::XY_vertex_projections(
   vector<twovector>& proj_vertices,double xscale_factor)
{
   for (unsigned int n=0; n<top_face.get_nvertices(); n++)
   {
      twovector curr_vertex=top_face.get_vertex(n);
      curr_vertex.put(0,curr_vertex.get(0)*xscale_factor);
      proj_vertices.push_back(curr_vertex);
   } // loop over index n labeling cube vertices
   for (unsigned int n=0; n<bottom_face.get_nvertices(); n++)
   {
      twovector curr_vertex=bottom_face.get_vertex(n);
      curr_vertex.put(0,curr_vertex.get(0)*xscale_factor);
      proj_vertices.push_back(curr_vertex);
   } // loop over index n labeling cube vertices
//   templatefunc::printVector(proj_vertices);
}

// ---------------------------------------------------------------------
// Member function box_approx aligns a box with the longest sides of
// the current parallelepiped object.  It projects the face of the
// parallelepiped corresponding to the two shortest sides into the
// plane orthogonal to the longest side direction vector.  We next
// approximate the parallelogram projection by a rectangle.  Finally,
// we orthogonally extrude the rectangle approximation to the smallest
// face for an altitude equal to along the parallelepiped's longest
// side.

parallelepiped parallelepiped::box_approx(polygon& bfp)
{
   calculate_symmetry_vectors_and_lengths();

   double altitude;
   vector<threevector> vertex(4);
   parallelogram baseface,baseface_proj;

   if (width >= height && width >= length)
   {
      vertex[0]=top_face.get_vertex(0);
      vertex[1]=top_face.get_vertex(3);
      vertex[2]=bottom_face.get_vertex(1);
      vertex[3]=bottom_face.get_vertex(0);
      baseface=parallelogram(vertex);
      baseface_proj=baseface.planar_projection(what);
      altitude=width;
   }
   else if (length >= width && length >= height)
   {
      vertex[0]=top_face.get_vertex(1);
      vertex[1]=top_face.get_vertex(0);
      vertex[2]=bottom_face.get_vertex(0);
      vertex[3]=bottom_face.get_vertex(3);
      baseface=parallelogram(vertex);
      baseface_proj=baseface.planar_projection(lhat);
      altitude=length;
   }
   else
   {
      vertex[0]=top_face.get_vertex(0);
      vertex[1]=top_face.get_vertex(1);
      vertex[2]=top_face.get_vertex(2);
      vertex[3]=top_face.get_vertex(3);
      baseface=parallelogram(vertex);
      baseface_proj=baseface.planar_projection(hhat);
      altitude=height;
   }
   
//   cout << "baseface = " << baseface << endl;
//   cout << "baseface_proj = " << baseface_proj << endl;
//   cout << "normal = " << baseface_proj.get_normal() << endl;
   parallelogram base_rect=baseface_proj.rectangle_approx();

//   bfp=baseface_proj;
   bfp=base_rect;
   parallelepiped box(base_rect,-base_rect.get_normal(),altitude);
//   cout << "box = " << box << endl;
   return box;
}

// =====================================================================
// Rigid parallelepiped manipulation methods
// =====================================================================

void parallelepiped::absolute_position(const threevector& rvec)
{
   translate(rvec-center.get_pnt());
}

void parallelepiped::translate(const threevector& rvec)
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
void parallelepiped::scale(
   const threevector& scale_origin,const threevector& scalefactor)
{
   center.scale(scale_origin,scalefactor);
   top_face.scale(scale_origin,scalefactor);
   bottom_face.scale(scale_origin,scalefactor);
   for (unsigned int i=0; i<n_sidefaces; i++)
   {
      side_face[i].scale(scale_origin,scalefactor);
   }

// After scaling current parallelepiped object, its width, length and
// height generally change.  So we need to recompute these quantities:

   calculate_symmetry_vectors_and_lengths();
}

// ---------------------------------------------------------------------
void parallelepiped::rotate(const rotation& R)
{
   threevector rotation_origin(Zero_vector);
   rotate(rotation_origin,R);
}

// ---------------------------------------------------------------------
void parallelepiped::rotate(
   const threevector& rotation_origin,const rotation& R)
{
   center.rotate(rotation_origin,R);
   top_face.rotate(rotation_origin,R);
   bottom_face.rotate(rotation_origin,R);
   
   for (unsigned int i=0; i<n_sidefaces; i++)
   {
      side_face[i].rotate(rotation_origin,R);
   }
}

// ---------------------------------------------------------------------
void parallelepiped::rotate(const threevector& rotation_origin,
                            double thetax,double thetay,double thetaz)
{
   rotation R(thetax,thetay,thetaz);
   rotate(rotation_origin,R);
}
