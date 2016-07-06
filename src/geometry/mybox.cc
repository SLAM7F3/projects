// ==========================================================================
// Mybox class member function definitions
// ==========================================================================
// Last modified on 8/6/08; 12/4/10; 3/6/14; 4/4/14
// ==========================================================================

#include <iostream>
#include "math/basic_math.h"
#include "math/constant_vectors.h"
#include "geometry/mybox.h"
#include "geometry/parallelogram.h"
#include "math/threevector.h"

using std::cout;
using std::endl;
using std::ostream;
using std::pair;
using std::vector;

// Constructor functions:

mybox::mybox(void)
{
}

mybox::mybox(double w,double l,double h)
{
   center=Zero_vector;
   reset_dimensions(w,l,h);
   
// On 3/1/02, we carefully reordered the vertices on the top, bottom
// and side faces of the box object so that these faces' normals obey
// a right hand rule and point outward from the center of the box

//   top_face.normal=z_hat;
//   bottom_face.normal=-z_hat;
//   side_face[0].normal=y_hat;
//   side_face[1].normal=-x_hat;
//   side_face[2].normal=-y_hat;
//   side_face[3].normal=x_hat;
}

mybox::mybox(
   double xlo,double xhi,double ylo,double yhi,double zlo,double zhi)
{
   center=0.5*threevector(xlo+xhi,ylo+yhi,zlo+zhi);
   reset_dimensions(xhi-xlo,yhi-ylo,zhi-zlo);
}

void mybox::reset_dimension_fractions(
   double w_frac,double l_frac,double h_frac)
{
   reset_dimensions(w_frac*width,l_frac*length,h_frac*height);
}

void mybox::reset_dimensions(double w,double l,double h)
{
   width=w;
   length=l;
   height=h;

   vector<threevector> vertex(4);
   vertex[0]=threevector(width/2,length/2,0);
   vertex[1]=threevector(-width/2,length/2,0);
   vertex[2]=threevector(-width/2,-length/2,0);
   vertex[3]=threevector(width/2,-length/2,0);
   top_face=polygon(vertex);
   top_face.absolute_position(center.get_pnt()+height/2*z_hat);

   vertex[0]=threevector(width/2,length/2,0);
   vertex[3]=threevector(-width/2,length/2,0);
   vertex[2]=threevector(-width/2,-length/2,0);
   vertex[1]=threevector(width/2,-length/2,0);
   bottom_face=polygon(vertex);
   bottom_face.absolute_position(center.get_pnt()-height/2*z_hat);

   for (unsigned int i=0; i<n_sidefaces; i++)
   {
      vertex[0]=bottom_face.get_vertex(modulo(4-i,4));
      vertex[1]=bottom_face.get_vertex(3-i);
      vertex[2]=top_face.get_vertex(modulo(i+1,4));
      vertex[3]=top_face.get_vertex(i);
      side_face[i]=polygon(vertex);
   }
}

mybox::mybox(const polygon& bface,const threevector& uhat,double h):
   parallelepiped(bface,uhat,h)
{
}

// Copy constructor:

mybox::mybox(const mybox& b):
   parallelepiped(b)
{
   docopy(b);
}

mybox::~mybox()
{
}

// ---------------------------------------------------------------------
void mybox::docopy(const mybox& b)
{
}	

// Overload = operator:

mybox mybox::operator= (const mybox& b)
{
   if (this==&b) return *this;
   parallelepiped::operator=(b);
   docopy(b);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,mybox& b)
{
   outstream << (parallelepiped&)b << endl;
   outstream << endl;
   
   return(outstream);
}

// =====================================================================

bool mybox::point_inside_xyprojected_mybox(const threevector& point)
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

   if (!point_inside)
   {
//   dotproduct=z_hat.dot(bottom_face.get_normal());
//   if (dotproduct > 0)
      {
         curr_projection=bottom_face.xy_projection();
         if (curr_projection.point_inside_polygon(point)) point_inside=true;
      }
   }
   
   if (!point_inside)
   {
      for (unsigned int i=0; i<n_sidefaces; i++)
      {
         dotproduct=z_hat.dot(side_face[i].get_normal());
         if (dotproduct > 0)
         {
            curr_projection=side_face[i].xy_projection();
            if (curr_projection.point_inside_polygon(point)) 
               point_inside=true;
         }
      }
   }
   
   return point_inside;
}

// ---------------------------------------------------------------------
void mybox::locate_extremal_xy_points(double& min_x,double& min_y,
                                      double& max_x,double& max_y)
{
   min_x=min_y=POSITIVEINFINITY;
   max_x=max_y=NEGATIVEINFINITY;

   double currmin_x,currmin_y,currmax_x,currmax_y;
   top_face.locate_extremal_xy_points(
      currmin_x,currmin_y,currmax_x,currmax_y);
   min_x=basic_math::min(min_x,currmin_x);
   min_y=basic_math::min(min_y,currmin_y);
   max_x=basic_math::max(max_x,currmax_x);
   max_y=basic_math::max(max_y,currmax_y);

   bottom_face.locate_extremal_xy_points(
      currmin_x,currmin_y,currmax_x,currmax_y);
   min_x=basic_math::min(min_x,currmin_x);
   min_y=basic_math::min(min_y,currmin_y);
   max_x=basic_math::max(max_x,currmax_x);
   max_y=basic_math::max(max_y,currmax_y);

   for (unsigned int i=0; i<n_sidefaces; i++)
   {
      side_face[i].locate_extremal_xy_points(
         currmin_x,currmin_y,currmax_x,currmax_y);
      min_x=basic_math::min(min_x,currmin_x);
      min_y=basic_math::min(min_y,currmin_y);
      max_x=basic_math::max(max_x,currmax_x);
      max_y=basic_math::max(max_y,currmax_y);
   }
}

// ---------------------------------------------------------------------
void mybox::locate_extremal_xyz_corners(
   threevector& min_corner,threevector& max_corner)
{
   double min_x,min_y,min_z,max_x,max_y,max_z;
   min_x=min_y=min_z=POSITIVEINFINITY;
   max_x=max_y=max_z=NEGATIVEINFINITY;

   double currmin_x,currmin_y,currmin_z,currmax_x,currmax_y,currmax_z;
   top_face.locate_extremal_xyz_points(
      currmin_x,currmin_y,currmin_z,currmax_x,currmax_y,currmax_z);
   min_x=basic_math::min(min_x,currmin_x);
   min_y=basic_math::min(min_y,currmin_y);
   min_z=basic_math::min(min_z,currmin_z);
   max_x=basic_math::max(max_x,currmax_x);
   max_y=basic_math::max(max_y,currmax_y);
   max_z=basic_math::max(max_z,currmax_z);

   bottom_face.locate_extremal_xyz_points(
      currmin_x,currmin_y,currmin_z,currmax_x,currmax_y,currmax_z);
   min_x=basic_math::min(min_x,currmin_x);
   min_y=basic_math::min(min_y,currmin_y);
   min_z=basic_math::min(min_z,currmin_z);
   max_x=basic_math::max(max_x,currmax_x);
   max_y=basic_math::max(max_y,currmax_y);
   max_z=basic_math::max(max_z,currmax_z);

   for (unsigned int i=0; i<n_sidefaces; i++)
   {
      side_face[i].locate_extremal_xyz_points(
         currmin_x,currmin_y,currmin_z,currmax_x,currmax_y,currmax_z);
      min_x=basic_math::min(min_x,currmin_x);
      min_y=basic_math::min(min_y,currmin_y);
      min_z=basic_math::min(min_z,currmin_z);
      max_x=basic_math::max(max_x,currmax_x);
      max_y=basic_math::max(max_y,currmax_y);
      max_z=basic_math::max(max_z,currmax_z);
   }
   min_corner=threevector(min_x,min_y,min_z);
   max_corner=threevector(max_x,max_y,max_z);
}

// ---------------------------------------------------------------------
// Member function calculate_illuminated_box_projected_area determines
// which faces of the current box object are illuminated by a light
// source whose propagation direction is set by unit vector uhat.  It
// then computes the projected area of these illuminated faces within
// the plane defined by viewing direction vector nhat.

double mybox::calculate_illuminated_box_projected_area(
   const threevector& uhat,const threevector& nhat)
{
   const double TINY_NEGATIVE=-0.00001;

// First determine which faces of the box are illuminated by the light
// source whose propagation direction is set by unit vector uhat.
// Recall that a face's normal vector must basically be ANTI-ALIGNED
// (rather than aligned) with uhat if it is to be illuminated at all:

   double projected_area=0;
   if (uhat.dot(top_face.get_normal()) < TINY_NEGATIVE)
   {
      projected_area += top_face.projected_area(nhat);
   }
   else if (uhat.dot(bottom_face.get_normal()) < TINY_NEGATIVE)
   {
      projected_area += bottom_face.projected_area(nhat);
   }
   for (unsigned int i=0; i<n_sidefaces; i++)
   {
       if (uhat.dot(side_face[i].get_normal()) < TINY_NEGATIVE)
       {
          projected_area += side_face[i].projected_area(nhat);
       }
   }
   return projected_area;
}

// ---------------------------------------------------------------------
// Member function retrieve_illuminated_faces determines which faces
// of the box are illuminated by the light source whose propagation
// direction is set by unit vector uhat.  Recall that a face's normal
// vector must basically be ANTI-ALIGNED (rather than aligned) with
// uhat if it is to be illuminated at all:

vector<pair<int,polygon> > mybox::retrieve_illuminated_faces(
   const threevector& rhat)
{
   vector<pair<int,polygon> > lit_face;
   threevector uhat(rhat.get(0),rhat.get(1),rhat.get(2));

   const double TINY_NEGATIVE=-0.00001;

   if (uhat.dot(top_face.get_normal()) < TINY_NEGATIVE)
   {
      lit_face.push_back(pair<int,polygon>(4,top_face));
   }
   else if (uhat.dot(bottom_face.get_normal()) < TINY_NEGATIVE)
   {
      lit_face.push_back(pair<int,polygon>(-1,bottom_face));
   }
   for (unsigned int i=0; i<n_sidefaces; i++)
   {
       if (uhat.dot(side_face[i].get_normal()) < TINY_NEGATIVE)
       {
          lit_face.push_back(pair<int,polygon>(i,side_face[i]));
       }
   }
   return lit_face;
}

void mybox::retrieve_illuminated_faces(
   int& nlit_faces,const threevector& uhat,polygon lit_face[])
{
   const double TINY_NEGATIVE=-0.00001;

   if (uhat.dot(top_face.get_normal()) < TINY_NEGATIVE)
   {
      lit_face[nlit_faces++]=top_face;
   }
   else if (uhat.dot(bottom_face.get_normal()) < TINY_NEGATIVE)
   {
      lit_face[nlit_faces++]=bottom_face;
   }
   for (unsigned int i=0; i<n_sidefaces; i++)
   {
       if (uhat.dot(side_face[i].get_normal()) < TINY_NEGATIVE)
       {
          lit_face[nlit_faces++]=side_face[i];
       }
   }
}

// ---------------------------------------------------------------------
// Member function calculate_box_shadow_regions determines which faces
// of the current box object are in shadow wrt a light source whose
// propagation direction is set by unit vector uhat.  It returns 3D
// shadow regions within the parallelepiped array shadow_volume as well
// as the number of such regions.

int mybox::calculate_box_shadow_regions(
   const threevector& uhat,parallelepiped shadow_volume[])
{
   const double shadow_region_height=20;	// meters
   const double TINY_NEGATIVE=-0.00001;

   int nregion=0;

// First determine which faces of the box are illuminated by the light
// source whose propagation direction is set by unit vector uhat.
// Recall that a face's normal vector must basically be ANTI-ALIGNED
// (rather than aligned) with uhat if it is to be illuminated at all:

   if (uhat.dot(top_face.get_normal()) < TINY_NEGATIVE)
   {
      shadow_volume[nregion]=parallelepiped(
         top_face,uhat,shadow_region_height);
      shadow_volume[nregion].calculate_symmetry_vectors_and_lengths();
      shadow_volume[nregion].calculate_metric_and_its_inverse();
      nregion++;
   }
   else if (uhat.dot(bottom_face.get_normal()) < TINY_NEGATIVE)
   {
      shadow_volume[nregion]=parallelepiped(
         bottom_face,uhat,shadow_region_height);
      shadow_volume[nregion].calculate_symmetry_vectors_and_lengths();
      shadow_volume[nregion].calculate_metric_and_its_inverse();
      nregion++;
   }
   for (unsigned int i=0; i<n_sidefaces; i++)
   {
       if (uhat.dot(side_face[i].get_normal()) < TINY_NEGATIVE)
       {
          shadow_volume[nregion]=parallelepiped(
             side_face[i],uhat,shadow_region_height);
          shadow_volume[nregion].calculate_symmetry_vectors_and_lengths();
          shadow_volume[nregion].calculate_metric_and_its_inverse();
          nregion++;
       }
   }
   return nregion;
}

// ---------------------------------------------------------------------
// Member function print_box_corners

void mybox::print_box_corners(std::ostream& outstream)
{
   outstream << top_face.get_vertex(0).get(0) << endl;
   outstream << top_face.get_vertex(0).get(1) << endl;
   outstream << top_face.get_vertex(0).get(2) << endl;
   outstream << bottom_face.get_vertex(2).get(0) << endl;
   outstream << bottom_face.get_vertex(2).get(1) << endl;
   outstream << bottom_face.get_vertex(2).get(2) << endl;
}

// ==========================================================================
// Face selection methods
// ==========================================================================

// Member function face_intercepted_by_ray

int mybox::face_intercepted_by_ray(const pair<threevector,threevector>& p)
{
   threevector basepoint(p.first);
   threevector r_hat(p.second);
   vector<pair<int,polygon> > illum_faces=retrieve_illuminated_faces(r_hat);

   for (unsigned int i=0; i<illum_faces.size(); i++)
   {
      int face_ID=illum_faces[i].first;
      polygon curr_face=illum_faces[i].second;
      
      threevector proj_pnt_in_face_plane;
      if (curr_face.ray_projected_into_poly_plane(
         basepoint,r_hat,curr_face.vertex_average(),proj_pnt_in_face_plane))
      {
         if (curr_face.point_inside_polygon(proj_pnt_in_face_plane))
         {
//            cout << "ray intercepts face #" << face_ID << endl;
            return face_ID;
         }
      } 
   } // loop over index i labeling "illuminated" rectangular faces
//   cout << "Ray does not intercept current mybox" << endl;
   return -2;
}
