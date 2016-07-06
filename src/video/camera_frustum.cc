// ==========================================================================
// CAMERA_FRUSTUM class member function definitions
// ==========================================================================
// Last modified on 2/28/12; 2/29/12; 3/1/12
// ==========================================================================

#include <iostream>
#include "geometry/bounding_box.h"
#include "video/camera_frustum.h"
#include "math/constant_vectors.h"
#include "geometry/face.h"
#include "geometry/geometry_funcs.h"
#include "geometry/linesegment.h"
#include "geometry/plane.h"
#include "geometry/polygon.h"

using std::cout;
using std::cin;
using std::endl;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void camera_frustum::allocate_member_objects()
{
}		       

void camera_frustum::initialize_member_objects()
{
//   cout << "inside camera_frustum::init_member_objects()" << endl;
}

camera_frustum::camera_frustum(
      const threevector& apex,
      const threevector& Uhat,const threevector& Vhat,
      const threevector& What,const vector<threevector>& world_rays)
{
   allocate_member_objects();
   initialize_member_objects();
   
   world_posn=apex;
   this->Uhat=Uhat;
   this->Vhat=Vhat;
   this->What=What;
   
   for (unsigned int c=0; c<world_rays.size(); c++)
   {
      UV_corner_world_ray.push_back(world_rays[c]);
   }

   compute_frusta_planes();
}		       

// Copy constructor:

camera_frustum::camera_frustum(const camera_frustum& c)
{
//   cout << "inside camera_frustum copy constructor, this(camera_frustum) = " << this << endl;
   allocate_member_objects();
   initialize_member_objects();
   docopy(c);
}

camera_frustum::~camera_frustum()
{
//   cout << "inside camera_frustum destructor" << endl;
//   cout << "this = " << this << endl;

}

// Overload = operator:

camera_frustum& camera_frustum::operator= (const camera_frustum& c)
{
//   cout << "inside camera_frustum::operator=" << endl;
   if (this==&c) return *this;
   docopy(c);
//   cout << "*this = " << *this << endl;
   return *this;
}

void camera_frustum::docopy(const camera_frustum& c)
{
//   cout << "inside camera_frustum::docopy()" << endl;

}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,camera_frustum& c)
{
   outstream << "inside camera_frustum::operator<<" << endl;
   
   outstream.precision(10);
   outstream << "World posn = " 
             << c.world_posn.get(0) << " "
             << c.world_posn.get(1) << " " 
             << c.world_posn.get(2) << endl;
   outstream << "Uhat = " 
             << c.Uhat.get(0) << " "
             << c.Uhat.get(1) << " " 
             << c.Uhat.get(2) << endl;
   outstream << "Vhat = " 
             << c.Vhat.get(0) << " "
             << c.Vhat.get(1) << " " 
             << c.Vhat.get(2) << endl;
   outstream << "What = " 
             << c.What.get(0) << " "
             << c.What.get(1) << " " 
             << c.What.get(2) << endl;
   return outstream;
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

// ==========================================================================
// View frustum culling member funtions
// ==========================================================================

// Member function compute_frusta_planes() fills member STL vector
// frusta_plane_ptrs within planes constructed from the camera_frustum's world
// position and its UV corner world rays.

void camera_frustum::compute_frusta_planes()
{
//   cout << "inside camera_frustum::compute_frusta_planes()" << endl;

   threevector V0=world_posn;
   for (int c=0; c<4; c++)
   {
      threevector curr_ray=UV_corner_world_ray[c];
      threevector next_ray=UV_corner_world_ray[modulo(c+1,4)];
      threevector V1=V0+curr_ray;
      threevector V2=V0+next_ray;
      plane* plane_ptr=new plane(V0,V1,V2);
      frusta_plane_ptrs.push_back(plane_ptr);

//      cout << "***************   c = " << c 
//            << " plane n_hat = " << plane_ptr->get_nhat() << endl;
//           << "curr_ray = " << curr_ray
//           << " next_ray = " << next_ray
//           << " plane = " << *plane_ptr << endl;
   } // loop over index c labeling UV imageplane corners

//   outputfunc::enter_continue_char();
}

// ---------------------------------------------------------------------
// Member function PointInsideFrustum() takes in 3D point XYZ and computes
// its signed distance to the 4 side planes of the camera_frustum's view
// frustum.  Recall that the side planes' normal direction vectors
// point away from the frustum.  So if any signed distance is
// positive, XYZ must lie outside the camera_frustum's frustum.  

bool camera_frustum::PointInsideFrustum(
   const threevector& XYZ,double tolerance)
{
//   cout << "inside camera_frustum::PointInsideFrustum()" << endl;

//   threevector UVW;
//   project_XYZ_to_UV_coordinates(XYZ,UVW);
//   cout << "UVW = " << UVW << endl;

   bool point_inside_frustum_flag=true;
   for (int c=0; c<4; c++)
   {
      plane* plane_ptr=frusta_plane_ptrs[c];
      double signed_distance=plane_ptr->signed_distance_from_plane(XYZ);
//      cout << "c = " << c << " signed_distance = " << signed_distance << endl;
      if (signed_distance > tolerance)
      {
         point_inside_frustum_flag=false;
         break;
      }
   } // loop over index c labeling frustum side planes
   
//   cout << "Point in frustum flag = " << point_in_frustum_flag << endl;
   return point_inside_frustum_flag;
}

// ---------------------------------------------------------------------
// Member function PointOutsideFrustum() 

bool camera_frustum::PointOutsideFrustum(
   const threevector& XYZ,double tolerance)
{
   return !PointInsideFrustum(XYZ,tolerance);
}

// ---------------------------------------------------------------------
// Member function PointOnFrustum() 

bool camera_frustum::PointOnFrustum(const threevector& XYZ)
{
//   cout << "inside camera_frustum::PointOnFrustum()" << endl;

   double tolerance=1E-4;
   bool outside_frustum_flag=PointOutsideFrustum(XYZ,tolerance);
   bool inside_frustum_flag=PointInsideFrustum(XYZ,-tolerance);
//   cout << "outside_frustum_flag = " << outside_frustum_flag << endl;
//   cout << "inside_frustum_flag = " << inside_frustum_flag << endl;

   bool on_frustum_flag=true;
   if (outside_frustum_flag==true) on_frustum_flag=false;
   if (inside_frustum_flag==true) on_frustum_flag=false;

//   cout << "on_frustum_flag = " << on_frustum_flag << endl;
   return on_frustum_flag;
}

// ---------------------------------------------------------------------
// Member function SphereInsideFrustum() takes in a sphere's center point
// XYZ and radius r.  If the sphere's center lies less than r from any
// of its side frusta planes, the sphere must lie at least partially
// within the camera_frustum's view frustum.  See "View frustum culling" web
// document by Mark Morley.
// http://www.racer.nl/reference/vfc_markmorley.htm

bool camera_frustum::SphereInsideFrustum(const threevector& XYZ,double radius)
{
//   cout << "inside camera_frustum::SphereInsideFrustum()" << endl;
//   cout << "sphere_center: X = " << XYZ.get(0) << " Y = " << XYZ.get(1)
//        << " Z = " << XYZ.get(2) << " R = " << radius << endl;

   bool sphere_in_frustum_flag=true;
   for (int c=0; c<4; c++)
   {
      plane* plane_ptr=frusta_plane_ptrs[c];
      double signed_distance=plane_ptr->signed_distance_from_plane(XYZ);
//      cout << "c = " << c << " signed_distance = " << signed_distance 
//           << " radius = " << radius << endl;

      if (signed_distance > radius)
      {
         sphere_in_frustum_flag=false;
         break;
      }
   } // loop over index c labeling frustum side planes
   
//   cout << "Sphere in frustum flag = " << sphere_in_frustum_flag << endl;
   return sphere_in_frustum_flag;
}

// ---------------------------------------------------------------------
// Member function BoxInsideFrustum() implements the algorithm for culling
// Axis Aligned Bounding Boxes presented in figure 4 of "Organized
// View Frustum Culling Algorithms for Bounding Boxes" by U. Assarsson
// and T. Moller.  Only one of the output boolean flags is true.  

void camera_frustum::BoxInsideFrustum(
   const bounding_box* bbox_ptr,
   bool& inside_flag,bool& outside_flag,bool& intersects_flag)
{
//   cout << "inside camera_frustum::BoxInsideFrustum()" << endl;

   outside_flag=false;
   intersects_flag=false;
   inside_flag=false;
   for (int c=0; c<4; c++)
   {
      plane* plane_ptr=frusta_plane_ptrs[c];

      threevector p_vertex,n_vertex;
      plane_ptr->compute_bbox_pn_vertices(bbox_ptr,p_vertex,n_vertex);
      double n_distance=plane_ptr->signed_distance_from_plane(n_vertex);
      double p_distance=plane_ptr->signed_distance_from_plane(p_vertex);
      
//      cout << "c = " << c 
//           << " n dist = " << n_distance
//           << " p dist = " << p_distance << endl;

      if (n_distance > 0)
      {
         outside_flag=true;
         intersects_flag=false;
//         cout << "outside_flag = true " << endl;
         return;
      }
      if (p_distance > 0)
      {
         intersects_flag=true;
//         cout << "intersects_flag = true" << endl;
      }
   } // loop over index c labeling frustum side planes   
      
   if (intersects_flag)
   {
   }
   else
   {
      inside_flag=true;
   }
}

// ==========================================================================
// Intersection member funtions
// ==========================================================================

// Member function RayFrustumIntersection() takes in a basepoint and
// direction vector for an (infinite, not semi-infinite!) ray.  An
// infinite line intersects the camera frustum in either 0, 1 or 2
// points.  The intersection points are returned in an STL vector.

vector<threevector> camera_frustum::RayFrustumIntersection(
   const threevector& ray_basepoint,const threevector& r_hat)
{
//   cout << "inside camera_frustum::RayFrustumIntersection()" << endl;

   vector<threevector> intersection_points;
   for (unsigned int p=0; p<frusta_plane_ptrs.size(); p++)
   {
      threevector candidate_intersection_point;
      if (frusta_plane_ptrs.at(p)->ray_intersection(
         ray_basepoint,r_hat,candidate_intersection_point))
      {
         if (!PointOutsideFrustum(candidate_intersection_point))
         {
            intersection_points.push_back(candidate_intersection_point);
         }
      }

   } // loop over index p labeling frusta planes
   return intersection_points;
}

// ---------------------------------------------------------------------
// Member function SegmentFrustumIntersection() takes in a finite
// linesegment l.  If l lies completely outside the camera frustum,
// this method returns NULL.  Otherwise, it returns a pointer to a
// dynamically instantiated clipped version of l which lies inside the
// camera frustum.

linesegment* camera_frustum::SegmentFrustumIntersection(const linesegment& l)
{
//   cout << "inside camera_frustum::SegmentFrustumIntersection()" << endl;

   linesegment* output_linesegment_ptr=NULL;

   threevector basepoint=l.get_v1();
   threevector e_hat=l.get_ehat();

   vector<threevector> intersection_points=RayFrustumIntersection(
      basepoint,e_hat);
//   cout << "intersection_points.size() = " << intersection_points.size()
//        << endl;

   if (intersection_points.size()==0) return output_linesegment_ptr;

   if (intersection_points.size()==2)
   {
      vector<double> intersection_pt_fracs;
      intersection_pt_fracs.push_back(
         l.frac_distance_along_infinite_line(intersection_points[0]));
      intersection_pt_fracs.push_back(
         l.frac_distance_along_infinite_line(intersection_points[1]));
      templatefunc::Quicksort(intersection_pt_fracs,intersection_points);
    
      double v1_frac=basic_math::max(0.0 , intersection_pt_fracs[0] );
      double v2_frac=basic_math::min(1.0 , intersection_pt_fracs[1] );
      if (v1_frac > v2_frac) return output_linesegment_ptr;

//      cout << "intersection_pt_fracs[0] = "
//           << intersection_pt_fracs[0] << endl;
//      cout << "intersection_pt_fracs[1] = "
//           << intersection_pt_fracs[1] << endl;
//      cout << "v1_frac = " << v1_frac
//           << " v2_frac = " << v2_frac << endl;

      threevector clipped_v1=l.get_v1();
      threevector clipped_v2=l.get_v2();
   
      if (!nearly_equal(v1_frac,0.0))
      {
         clipped_v1=intersection_points[0];
      }

      if (!nearly_equal(v2_frac,1.0))
      {
         clipped_v2=intersection_points[1];
      }
      
      output_linesegment_ptr=new linesegment(clipped_v1,clipped_v2);
      return output_linesegment_ptr;
   }  // intersection_points.size()==2 conditional

   if (intersection_points.size()==1)
   {

// Instantiate new version of input linesegment whose direction vector
// is generally aligned with frustum's:

      linesegment lnew;
      double dotproduct=-What.dot(e_hat);
      if (dotproduct > 0)
      {
         lnew=linesegment(l.get_v1(),l.get_v2());
      }
      else
      {
         lnew=linesegment(l.get_v2(),l.get_v1());
      }
      
      double intersection_pt_frac=
         lnew.frac_distance_along_infinite_line(intersection_points[0]);

      double v1_frac=basic_math::max(0.0 , intersection_pt_frac );
      double v2_frac=basic_math::max(1.0 , intersection_pt_frac );

      if (nearly_equal(v1_frac,v2_frac))
      {
         // v1,v2 both lie outside frustum
         return output_linesegment_ptr;
      }
      
      threevector clipped_v1=lnew.get_v1();
      if (!nearly_equal(v1_frac,0.0))
      {
         clipped_v1=intersection_points[0];
      }
      threevector clipped_v2=lnew.get_v2();

      output_linesegment_ptr=new linesegment(clipped_v1,clipped_v2);
      return output_linesegment_ptr;
   }  // intersection_points.size()==1 conditional

   return output_linesegment_ptr;
}

// ---------------------------------------------------------------------
// Member function PolygonFrustumIntersection() takes in a polygon
// poly.  If poly lies completely outside the camera frustum this
// method returns NULL.  Otherwise, it returns a pointer to a
// dynamically instantiated clipped version of poly which lies inside
// the camera frustum.

polygon* camera_frustum::PolygonFrustumIntersection(polygon& poly)
{
//   cout << "inside camera_frustum::PolygonFrustumIntersection()" << endl;

   const int plane_side_sgn=-1; // frusta planes are directed outwards
   vector<polygon*> cleaved_polygon_ptrs;
   for (unsigned int f=0; f<frusta_plane_ptrs.size(); f++)
   {
//      cout << "frustum plane f index = " << f << endl;
//      cout << "curr frustum plane = " << *(frusta_plane_ptrs[f])
//           << endl;

      polygon* cleaved_polygon_ptr=
         frusta_plane_ptrs[f]->polygon_above_or_below_plane(
            poly,plane_side_sgn);
//      cout << "cleaved_polygon_ptr = " << cleaved_polygon_ptr << endl;

      if (cleaved_polygon_ptr != NULL)
      {
         cleaved_polygon_ptrs.push_back(cleaved_polygon_ptr);
//         cout << "Cleaved polygon = " << *cleaved_polygon_ptr << endl;
      }
      else
      {

// Input polygon lies outside at least one of the frusta planes. So it
// has no intersection with the camera frustum:

//         cout << "cleaved polygon_ptr = NULL" << endl;
         return NULL;
      }
      
   } // loop over index f labeling frusta planes
//   cout << "cleaved_polygon_ptrs.size() = " << cleaved_polygon_ptrs.size()
//        << endl;

   threevector n_hat;
   threevector COM=Zero_vector;
   for (unsigned int p=0; p<cleaved_polygon_ptrs.size(); p++)
   {
      COM += cleaved_polygon_ptrs[p]->compute_COM();
      n_hat=cleaved_polygon_ptrs[p]->get_normal();
   }
   COM /= cleaved_polygon_ptrs.size();

//   cout << "COM = " << COM << endl;
//   cout << "n_hat = " << n_hat << endl;

// Before performing intersection operation, we first translate all
// polys so that their COM-->(0,0,0).  Then rotate each polygon
// about (0,0,0) so that they each lie within Z=0 XY plane:

   rotation R;
   R=R.rotation_taking_u_to_v(n_hat,z_hat);
//   cout << "R = " << R << endl;

   vector<polygon> cleaved_polys;
   for (unsigned int p=0; p<cleaved_polygon_ptrs.size(); p++)
   {
      polygon curr_cleaved_poly=*(cleaved_polygon_ptrs[p]);
      curr_cleaved_poly.translate(-COM);      
      curr_cleaved_poly.rotate(Zero_vector,R);      
      cleaved_polys.push_back(curr_cleaved_poly);
   }

   polygon* intersection_poly_ptr=
      geometry_func::polygon_intersection(cleaved_polys);

   if (intersection_poly_ptr==NULL) return NULL;

// After performing intersection operation, rotate
// poly_cleaved_by_plane back from Z=0 XY plane to original plane.
// Then translate by COM:

   intersection_poly_ptr->rotate(Zero_vector,R.transpose());
   intersection_poly_ptr->translate(COM);
   
   return intersection_poly_ptr;
}

// ---------------------------------------------------------------------
// Member function FaceFrustumIntersection() takes in a face.  If the face
// lies completely outside the camera frustum this method returns
// NULL.  Otherwise, it returns a pointer to a dynamically
// instantiated clipped version of the face's polygon which lies
// inside the camera frustum.

polygon* camera_frustum::FaceFrustumIntersection(face& f)
{
//   cout << "inside camera_frustum::FaceFrustumIntersection()" << endl;
   
   polygon poly=f.get_polygon();
   return PolygonFrustumIntersection(poly);
}
