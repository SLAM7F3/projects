// ==========================================================================
// ViewFrustum class member function definitions
// ==========================================================================
// Last updated on 6/20/07; 8/21/07; 9/20/07
// ==========================================================================

#include <iostream>
#include <string>
#include <osg/BoundingSphere>
#include "geometry/geometry_funcs.h"
#include "math/genmatrix.h"
#include "general/inputfuncs.h"
#include "osg/ViewFrustum.h"
#include "osg/osgWindow/WindowManager.h"

#include "osg/osgfuncs.h"
#include "general/outputfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void ViewFrustum::allocate_member_objects()
{
}		       

void ViewFrustum::initialize_member_objects()
{
   fixed_params_already_computed=false;
   WindowManager_ptr=NULL;
}		       

ViewFrustum::ViewFrustum(WindowManager* WM_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
   WindowManager_ptr=WM_ptr;
}		       

ViewFrustum::~ViewFrustum()
{	
}

// ==========================================================================
// Frustum parameter evaluation member functions
// ==========================================================================

void ViewFrustum::retrieve_camera_posn_and_orientation()
{
//   cout << "inside ViewFrustum::retrieve_camera_posn_and_orientation()"
//        << endl;

   osg::Vec3 eye,center,up;
   if (WindowManager_ptr != NULL)
   {
      WindowManager_ptr->retrieve_camera_posn_and_direction_vectors(
         eye,center,up);
   }
   else
   {
      cout << "Error in ViewFrustum::retrieve_camera_posn_and_orientation()"
           << endl;
      cout << "WindowManager_ptr = NULL ! " << endl;
      exit(-1);
   }
   
//   cout << "eye = " << endl;
//   osgfunc::print_Vec3(eye);
//   cout << "center = " << endl;
//   osgfunc::print_Vec3(center);
//   cout << "up = " << endl;
//   osgfunc::print_Vec3(up);

   camera_Yhat=threevector(up);
   camera_Zhat=threevector(eye-center).unitvector();
   camera_Xhat=camera_Yhat.cross(camera_Zhat);
   camera_posn=threevector(eye);
//   cout << "Xhat = " << camera_Xhat << endl;
//   cout << "Yhat = " << camera_Yhat << endl;
//   cout << "Zhat = " << camera_Zhat << endl;
//   cout << "posn = " << camera_posn << endl;
}

// --------------------------------------------------------------------------
void ViewFrustum::compute_params_planes_and_vertices()
{
//   cout << "inside ViewFrustum::compute_params_planes_and_vertices()" << endl;

// Retrieve time-independent Projection matrix just once from
// *WindowManager_ptr and copy into member object ProjectionMatrix:

   if (!fixed_params_already_computed)
   {
      if (WindowManager_ptr != NULL)
      {
         ProjectionMatrix=WindowManager_ptr->getProjectionMatrix();
      }
      else
      {
         return;
      }
   }

//   cout << "ProjectionMatrix = " << endl;
//   osgfunc::print_matrix(ProjectionMatrix);
//   outputfunc::enter_continue_char();

   retrieve_camera_posn_and_orientation();
   compute_planes();
   compute_vertices();

   if (!fixed_params_already_computed)
   {
      compute_fixed_params();
      fixed_params_already_computed=true;
   }   

/*
   string label="Enter sphere's center:";
   threevector V=inputfunc::enter_threevector(label);
   double radius;
   cout << "Enter sphere's radius:" << endl;
   cin >> radius;
   cout << "sphere inside frustum = " << sphere_inside(V,radius) << endl;
*/

}

// --------------------------------------------------------------------------
// Member function compute_planes recomputes the left, right, bottom,
// top, near and far planes which define the ViewFrustum using
// information extracted from the Projection and Model View matrices.
// See http://www.lighthouse3d.com/opengl/viewfrustum by Antonio
// Ramires Fernandes.

vector<plane>& ViewFrustum::compute_planes()
{
//   cout << "inside ViewFrustum::compute_planes()" << endl;
   osg::Matrix VP;
   
   if (WindowManager_ptr != NULL)
   {
      VP = WindowManager_ptr->getViewMatrix() * 
         WindowManager_ptr->getProjectionMatrix();
   }

   fourvector column[4];
   for (int i=0; i<4; i++)
   {
      for (int j=0; j<4; j++)
      {
         column[i].put(j,VP(j,i));
      }
   }


   frustum_plane.clear();
   frustum_plane.push_back(plane(column[0]+column[3]));	  // left plane
   frustum_plane.push_back(plane(-column[0]+column[3]));  // right plane
   frustum_plane.push_back(plane(column[1]+column[3]));  // bottom plane
   frustum_plane.push_back(plane(-column[1]+column[3]));  // top plane
   frustum_plane.push_back(plane(column[2]+column[3]));  // near plane
   frustum_plane.push_back(plane(-column[2]+column[3]));  // far plane

/*
   cout << "left plane n_hat = " << frustum_plane[0].get_nhat() << endl;
   cout << "left plane origin = " << frustum_plane[0].get_origin() << endl;

   cout << "right plane n_hat = " << frustum_plane[1].get_nhat() << endl;
   cout << "right plane origin = " << frustum_plane[1].get_origin() << endl;

   cout << "bottom plane n_hat = " << frustum_plane[2].get_nhat() << endl;
   cout << "bottom plane origin = " << frustum_plane[2].get_origin() << endl;

   cout << "top plane n_hat = " << frustum_plane[3].get_nhat() << endl;
   cout << "top plane origin = " << frustum_plane[3].get_origin() << endl;

   cout << "near plane n_hat = " << frustum_plane[4].get_nhat() << endl;
   cout << "near plane origin = " << frustum_plane[4].get_origin() << endl;

   cout << "far plane n_hat = " << frustum_plane[5].get_nhat() << endl;
   cout << "far plane origin = " << frustum_plane[5].get_origin() << endl;
*/

   return frustum_plane;
}

// --------------------------------------------------------------------------
// Member function compute_vertices works with the 6 view frustum
// planes generated by member function compute_planes.  It computes
// and returns the 8 vertices which define the view frustum and which
// are located at the intersections of 3 different frustum planes.

vector<threevector>& ViewFrustum::compute_vertices()
{
//   cout << "inside ViewFrustum::compute_vertices()" << endl;
   
   plane* left_ptr(&frustum_plane[0]);
   plane* right_ptr(&frustum_plane[1]);
   plane* bottom_ptr(&frustum_plane[2]);
   plane* top_ptr(&frustum_plane[3]);
   plane* near_ptr(&frustum_plane[4]);
   plane* far_ptr(&frustum_plane[5]);

   threevector ntl=geometry_func::intersection_of_three_planes(
      *near_ptr,*top_ptr,*left_ptr);
   threevector ntr=geometry_func::intersection_of_three_planes(
      *near_ptr,*top_ptr,*right_ptr);
   threevector nbr=geometry_func::intersection_of_three_planes(
      *near_ptr,*bottom_ptr,*right_ptr);
   threevector nbl=geometry_func::intersection_of_three_planes(
      *near_ptr,*bottom_ptr,*left_ptr);

   threevector ftl=geometry_func::intersection_of_three_planes(
      *far_ptr,*top_ptr,*left_ptr);
   threevector ftr=geometry_func::intersection_of_three_planes(
      *far_ptr,*top_ptr,*right_ptr);
   threevector fbr=geometry_func::intersection_of_three_planes(
      *far_ptr,*bottom_ptr,*right_ptr);
   threevector fbl=geometry_func::intersection_of_three_planes(
      *far_ptr,*bottom_ptr,*left_ptr);

/*
   cout << "ntl = " << ntl << endl;
   cout << "ntr = " << ntr << endl;
   cout << "nbl = " << nbl << endl;
   cout << "nbr = " << nbr << endl;
   cout << "ftl = " << ftl << endl;
   cout << "ftr = " << ftr << endl;
   cout << "fbl = " << fbl << endl;
   cout << "fbr = " << fbr << endl;
*/

   ray.clear();

// FAKE FAKE: On Weds, June 20 at 8 am, we changed the ordering of
// rays emanating from eye position so that it agrees with our
// convention for digital photos.  Ordering change does not appear to
// negatively impact our EARTH programs which seem to be the only ones
// (as of June 2007) that use ray information.

   ray.push_back( (fbl-nbl).unitvector() );
   ray.push_back( (fbr-nbr).unitvector() );
   ray.push_back( (ftr-ntr).unitvector() );
   ray.push_back( (ftl-ntl).unitvector() );

//   threevector avg_ray=0.25*(ray[0]+ray[1]+ray[2]+ray[3]);
//   cout << "avg_ray = " << avg_ray 
//        << " -camera_Zhat = " << -camera_Zhat << endl;

//   cout << "ray[0] = " << ray[0] << endl;
//   cout << "ray[1] = " << ray[1] << endl;
//   cout << "ray[2] = " << ray[2] << endl;
//   cout << "ray[3] = " << ray[3] << endl;

   vertex.clear();
   vertex.push_back(ntl);
   vertex.push_back(ntr);
   vertex.push_back(nbl);
   vertex.push_back(nbr);

   vertex.push_back(ftl);
   vertex.push_back(ftr);
   vertex.push_back(fbl);
   vertex.push_back(fbr);
   return vertex;
}

// --------------------------------------------------------------------------
// Member function compute_fixed_params computes the locations of the
// near and far clipping planes as well as the horizontal and vertical
// fields-of-view.  This method should only be called once.

void ViewFrustum::compute_fixed_params()
{
//   cout << "inside VF::compute_fixed_params()" << endl;
   
   threevector ntl(vertex.front());
   threevector fbr(vertex.back());

   n=-(camera_Zhat.dot(ntl-camera_posn));
   f=-(camera_Zhat.dot(fbr-camera_posn));
   double t=camera_Yhat.dot(ntl-camera_posn);
   double l=-(camera_Xhat.dot(ntl-camera_posn));
   aspect=t/l;		// = frustum's width / height ratio
   tan_half_FOV_up=t/n;
   tan_half_FOV_right=l/n;
   cos_half_FOV_up=n/sqrt(t*t+n*n);
   cos_half_FOV_right=n/sqrt(l*l+n*n);
   double half_FOV_up=atan2(t,n);
   double half_FOV_right=atan2(l,n);

   FOV_horiz=2*half_FOV_right;
   FOV_vert=2*half_FOV_up;

//   cout << "l = " << l << " t = " << t << endl;
//   cout << "n = " << n << " f = " << f << endl;
//   cout << "aspect = " << aspect << endl;
//   cout << "tan_half_FOV_up = " << tan_half_FOV_up
//        << " tan_half_FOV_right = " << tan_half_FOV_right << endl;
//   cout << " FOV_horiz = " << FOV_horiz*180/PI << " degs" << endl;
//   cout << "FOV_vert = " << FOV_vert*180/PI << " degs" << endl;
//   outputfunc::enter_continue_char();
}

// ==========================================================================
// Ray tracing member functions
// ==========================================================================

// Member function get_ray takes in fractions -1 <= fu, fv <= 1.  It
// returns the direction vector which emanates from the virtual
// camera's position and passes through the (U,V) image plane point
// corresponding to these input fractions.  This direction vector is a
// linear combination of the ViewFrustum's 4 corner rays.

threevector ViewFrustum::get_ray(double fu,double fv)
{
   return (0.25* ( (1-fu)*(1-fv)*ray[0] + (1+fu)*(1-fv)*ray[1] +
                   (1+fu)*(1+fv)*ray[2] + (1-fu)*(1+fv)*ray[3] ) ).
      unitvector();
}

// ==========================================================================
// Culling member functions
// ==========================================================================

bool ViewFrustum::point_inside(const threevector& V)
{
   threevector diff(V-camera_posn);
   double d=-(camera_Zhat.dot(diff));
   if (d < n || d > f) return false;

   double r=camera_Xhat.dot(diff);
   double r_extremal=d*tan_half_FOV_right;
   if (r < -r_extremal || r > r_extremal) return false;

   double u=camera_Yhat.dot(diff);
   double u_extremal=d*tan_half_FOV_up;
   if (u < -u_extremal || u > u_extremal) return false;
   
   return true;
}

// --------------------------------------------------------------------------
// Member function sphere_inside tests whether an input bounding
// sphere lies completely outside the ViewFrustum.  If so, this
// boolean method returns true.  If it lies partially or completely
// inside the frustum, this method returns false.  See
// http://www.lighthouse3d.com/opengl/viewfrustum by Antonio Ramires
// Fernandes.

bool ViewFrustum::sphere_inside(const osg::BoundingSphere& sphere)
{
   double radius=sphere.radius();
   threevector center(sphere.center().x(),sphere.center().y(),
                      sphere.center().z());
   
   if (sphere_inside(center,radius)==-1)
   {
      return false;
   }
   else
   {
      return true;
   }
}

int ViewFrustum::sphere_inside(
   const threevector& sphere_center, double radius)
{
   const int OUTSIDE=-1;
   const int INTERSECT=0;
   const int INSIDE=1;
   int result = INSIDE;

   threevector diff(sphere_center-camera_posn);

   double d = -(camera_Zhat.dot(diff));
//   double d = camera_dir.dot(diff);
//   cout << "d = " << d << " radius = " << radius << endl;
   if (d < n-radius || d > f+radius)
   {
//      cout << "n-radius = " << n-radius << endl;
      return OUTSIDE;
   }
   else
   {
//      cout << "sphere lies between near and far planes" << endl;
   }
   if (d > f-radius || d < n+radius) result=INTERSECT;

   double u=camera_Yhat.dot(diff);
   double u_extremal=d*tan_half_FOV_up;
//   cout << "u = " << u << " u_extremal = " << u_extremal << endl;

   double radius_over_cos_half_FOV_up=radius/cos_half_FOV_up;
   if (u > u_extremal + radius_over_cos_half_FOV_up || 
       u < -u_extremal - radius_over_cos_half_FOV_up)
   {
//      cout << "radius_over_cos_half_FOV_up = "
//           << radius_over_cos_half_FOV_up << endl;
      return OUTSIDE;
   }
   
   if (u > u_extremal - radius_over_cos_half_FOV_up || 
       u < -u_extremal + radius_over_cos_half_FOV_up) result = INTERSECT;

//   cout << "diff = " << diff << endl;

   double r=camera_Xhat.dot(diff);
   double radius_over_cos_half_FOV_right=radius/cos_half_FOV_right;
   double r_extremal=d*tan_half_FOV_right;

   if (r > r_extremal + radius_over_cos_half_FOV_right ||
       r < -r_extremal - radius_over_cos_half_FOV_right) 
   {
//      cout << "r = " << r << endl;
//      cout << "radius_over_cos_half_FOV_right = "
//           << radius_over_cos_half_FOV_right << endl;
//      cout << " r_extremal = " << r_extremal << endl;
//      cout << "sum = " << radius_over_cos_half_FOV_right + r_extremal
//           << endl;
      return OUTSIDE;
   }
   
   if (r > r_extremal - radius_over_cos_half_FOV_right ||
       r < -r_extremal + radius_over_cos_half_FOV_right) result=INTERSECT;

   return result;
}

// --------------------------------------------------------------------------
// Member function reset_viewmatrix() attempts to alter in real time
// the pointing and/or position of the virtual camera.  But for
// reasons we don't understand as of 1/29/08, calling this method
// currently has no effect upon the virtual camera.

void ViewFrustum::reset_viewmatrix(
   const threevector& Uhat,const threevector& Vhat,
   const threevector& camera_posn)
{
//   cout << "inside ViewFrustum::reset_viewmatrix()" << endl;
   
   threevector What(Uhat.cross(Vhat));

   osg::Matrix M,Minv;
   M.set(Uhat.get(0),Uhat.get(1),Uhat.get(2),0,
         Vhat.get(0),Vhat.get(1),Vhat.get(2),0,
         What.get(0),What.get(1),What.get(2),0,
         camera_posn.get(0),camera_posn.get(1),camera_posn.get(2),1);
   Minv=M.inverse(M);

//   cout << "Minv =  " << endl;
//   osgfunc::print_matrix(Minv);

   WindowManager_ptr->setViewMatrix(Minv);

   compute_params_planes_and_vertices();

//   cout << "WM.getViewMatrix() = " << endl;
//   osg::Matrix Mcurr=WindowManager_ptr->getViewMatrix();
//   osgfunc::print_matrix(Mcurr);
}
