// ==========================================================================
// TRANSFORMER class member function definitions
// ==========================================================================
// Last modified on 9/20/07; 9/21/07; 9/22/07; 6/19/08; 4/5/14
// ==========================================================================

#include <iostream>
#include <vector>

#include "math/fourvector.h"
#include "math/genmatrix.h"
#include "astro_geo/geofuncs.h"
#include "geometry/linesegment.h"
#include "osg/Transformer.h"
#include "osg/osgWindow/WindowManager.h"

#include "general/outputfuncs.h"
#include "osg/osgfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::pair;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void Transformer::allocate_member_objects()
{
}		       

void Transformer::initialize_member_objects()
{
   PV_ptr=new genmatrix(4,4);
   PVinverse_ptr=new genmatrix(4,4);
   WindowManager_ptr=NULL;
}

Transformer::Transformer(WindowManager* WM_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
   WindowManager_ptr=WM_ptr;
}		       

Transformer::~Transformer()
{	
   delete PV_ptr;
   delete PVinverse_ptr;
}

// ==========================================================================
// Transformations between world xyz and screen XYZ coordinate systems
// ==========================================================================

// Member functions world_to_screen_transformation and
// screen_to_world_transformation access the current viewer which
// holds the current orientation and position of the world-space XYZ
// coordinate system relative to the screen's fixed horizontal,
// vertical and normal-towards-eye direction vectors.  These methods
// retrieve the 4x4 viewer and projection matrices which map
// world-space to screen-space coordinates.  The total transformation
// is returned within the output 4x4 matrix PV or PVinverse.

// See written notes "OpenGL mapping from 3D world space to 2D screen
// space" dated 6/29/05.

void Transformer::world_to_screen_transformation()
{
//   cout << "inside Transformer::world_to_screen_transformation() #1" << endl;
//   cout << "this = " << this << endl;

//   osg::CameraNode* cameranode_ptr=SceneView_ptr->getCamera();
//   osg::Matrix PM=cameranode_ptr->getProjectionMatrix();
//   cout << "CameraNode proj matrix = " << endl;
//   osgfunc::print_matrix(PM);

//   double fovy,aspectRatio,zNear,zFar;
//   double left,right,bottom,top;

/*
   if (cameranode_ptr->getProjectionMatrixAsPerspective(
      fovy,aspectRatio,zNear,zFar))
   {
      cout << "cameranode fovy = " << fovy << endl;
      cout << "cameranode aspectRatio = " << aspectRatio << endl;
      cout << "cameranode zNear = " << zNear << endl;
      cout << "cameranode zFar = " << zFar << endl;
   }
*/

/*
   if (cameranode_ptr->getProjectionMatrixAsFrustum(
      left,right,bottom,top,zNear,zFar))
   {
      cout << "cameranode left = " << left << " right = " << right << endl;
      cout << "cameranode bottom = " << bottom << " top = " << top << endl;
      cout << "cameranode zNear = " << zNear << " zFar = " << zFar << endl;
   }
*/

/*
   if (SceneView_ptr->getProjectionMatrixAsPerspective(
      fovy,aspectRatio,zNear,zFar) )
   {
      cout << "fovy = " << fovy << endl;
      cout << "aspectRatio = " << aspectRatio << endl;
      cout << "zNear = " << zNear << endl;
      cout << "zFar = " << zFar << endl;
   }

   if (SceneView_ptr->getProjectionMatrixAsFrustum(
      left,right,bottom,top,zNear,zFar))
   {
      cout << "left = " << left << " right = " << right << endl;
      cout << "bottom = " << bottom << " top = " << top << endl;
      cout << "zNear = " << zNear << " zFar = " << zFar << endl;
   }
*/
 
//   osg::Matrixd ViewMatrix(SceneView_ptr->getViewMatrix());
//   osg::Matrixd ProjMatrix(SceneView_ptr->getProjectionMatrix());
//   cout << "ViewMatrix = " << endl;
//   osgfunc::print_matrix(ViewMatrix);
//   cout << "ProjMatrix = " << endl;
//   osgfunc::print_matrix(ProjMatrix);

// Note: We learned the hard and painful way on 6/29/05 that
// *DIFFERENT values for the near and far clipping plane values are
// stored within the projection matrices for the scene view and
// camera.  n = 1 and f = 1E6 from the camera.  On the other hand, n
// and f vary as we zoom in and out if we use the scene view project
// matrix.  We don't understand why this difference exists...

// l = left, r = right, b = bottom, t = top, n = near clipping plane,
// f = far clipping plane.

// Note2: As of Sept 2007 under OSG1.2, n and f appear to be the same
// whether we extract them from SceneView or from a CameraNode coming
// from a SceneView.  Both vary with zoom level...

   osg::Matrixd matVP;
   if (WindowManager_ptr != NULL)
   {
      matVP = WindowManager_ptr->getViewMatrix() * 
         WindowManager_ptr->getProjectionMatrix();
   }
   else
   {
      cout << "Error in Transformer::world_to_screen_transformation()!"
           << endl;
      cout << "WindowManager_ptr==NULL" << endl;
      cout << "This pointer must be set before this method can be called!"
           << endl;
      exit(-1);
   }
   
//   double l,r,b,t,n,f,xshear,yshear;
//   camera_ptr->getLensParams(l,r,b,t,n,f,xshear,yshear);
//   cout << "l = " << l << " r = " << r
//        << " b = " << b << " t = " << t
//        << " n = " << n << " f = " << f << endl;

/*
   genmatrix R(4,4),Rtrans(4,4);
   for (int i=0; i<4; i++)
   {
      for (int j=0; j<4; j++)
      {
         R.put(i,j,0);
         Rtrans.put(i,j,0);
      }
   }
      
   R.put(0,0,2*n/(r-l));
   R.put(0,2,(r+l)/(r-l));
   R.put(1,1,2*n/(t-b));
   R.put(1,2,(t+b)/(t-b));
   R.put(2,2,-(f+n)/(f-n));
   R.put(2,3,-2*f*n/(f-n));
   R.put(3,2,-1);
   Rtrans=R.transpose();
   cout << "Rtrans = " << Rtrans << endl;
   outputfunc::enter_continue_char();
*/

//         cout << "left = " << l << " right = " << r
//              << " top = " << t << " bottom = " << b << endl;
//         cout << "near = " << n << " far = " << f << endl;
//         cout << "xshear = " << xshear << " yshear = " << yshear << endl;

// On 6/29/05, we learned that OSG follows a row-vector left-multiply
// convention for 4-vectors, whereas OpenGL follows a column-vector
// right-multiply convention.  We take transposes of matView and
// matProj so that we can multiply on the right by column 4-vectors
// rather than on the left by row 4-vectors:

   for (int i=0; i<4; i++)
   {
      for (int j=0; j<4; j++)
      {
         PV_ptr->put(j,i,matVP(i,j));
      } // loop over j index
   } // loop over i index
}
   
void Transformer::screen_to_world_transformation()
{
   world_to_screen_transformation();
   PV_ptr->inverse(*PVinverse_ptr);
}
   
// ---------------------------------------------------------------------
threevector Transformer::world_to_screen_transformation(const threevector& q)
{
//   cout << "inside Transformer::world_to_screen_transformation() #2" << endl;
   fourvector Q(q,1);

   world_to_screen_transformation();
//   cout << "*PV_ptr = " << *PV_ptr << endl;
//   cout << "q = " << q << endl;

   fourvector Qprime=(*PV_ptr)*Q;

// Renormalize homogeneous 4-vector's X, Y and Z coordinates so that
// they lie within the interval [-1,1]:

   for (int i=0; i<4; i++)
   {
      Qprime.put(i,Qprime.get(i)/Qprime.get(3));
   }

   threevector qprime(Qprime.get(0),Qprime.get(1),Qprime.get(2));
//   cout << "qprime = " << qprime << endl;
   return qprime;
}
   
// ---------------------------------------------------------------------
threevector Transformer::screen_to_world_transformation(const threevector& q)
{
   fourvector Q(q,1);

   screen_to_world_transformation();
   fourvector Qprime=(*PVinverse_ptr)*Q;

// Renormalize homogeneous 4-vector's x, y and z coordinates so that
// they lie within the interval [-1,1]:

   for (int i=0; i<4; i++)
   {
      Qprime.put(i,Qprime.get(i)/Qprime.get(3));
   }

   threevector qprime(Qprime.get(0),Qprime.get(1),Qprime.get(2));
   return qprime;
}
   
// --------------------------------------------------------------------------
// Member function compute_ray_into_screen takes in screen coordinates
// X and Y.  It computes the world-space ray which runs through X and
// Y along the normal direction into the screen.  It returns a unit
// vector pointing into the screen.  

// Important note added on 6/19/08: Worldspace basepoint should be set
// by CustomManipulator's eye position in world space rather than by
// the output of this method!

threevector Transformer::compute_ray_into_screen(double X,double Y)
{   
//   cout << "inside Transformer::compute_ray_into_screen()" << endl;
//   cout << "X = " << X << " Y = " << Y << endl;
   
   threevector screenspace_posn(X,Y,0);
   threevector screenspace_posn2(X,Y,1);

   threevector worldspace_basepoint=screen_to_world_transformation(
      screenspace_posn);
   threevector worldspace_point2=screen_to_world_transformation(
      screenspace_posn2);
   threevector r_hat((worldspace_point2-worldspace_basepoint).unitvector());

//   cout << "worldspace_basepoint = " << worldspace_basepoint << endl;
//   cout << "worldspace_point2 = " << worldspace_point2 << endl;
//   cout << "r_hat = " << r_hat << endl;

   return r_hat;
}

// --------------------------------------------------------------------------
// Member function compute_screen_ray_intercept_with_zplane takes in
// screen coordinates X and Y along with world-space coordinate
// Zplane.  Recall -1 <= X,Y <= 1.  It first computes the world-space
// line which runs through X and Y along the normal direction into the
// screen.  This method subsequently solves for the world-space
// location where the line intercepts the z=Z world-plane.  The
// threevector world-space intercept location is returned by this
// member function.  If the input screen ray has zero z-component,
// this boolean method returns false.

bool Transformer::compute_screen_ray_intercept_with_zplane(
   double X,double Y,double Zplane,threevector& intercept_posn)
{   
//   cout << "inside Transformer::compute_screen_ray_intercept_with_zplane()" 
//        << endl;
//   cout << "X = " << X << " Y = " << Y << endl;

   bool ray_intercepts_zplane_flag=true;

   threevector screenspace_posn1(X,Y,0);
   threevector screenspace_posn2(X,Y,-1);
   threevector worldspace_posn1=screen_to_world_transformation(
      screenspace_posn1);
   threevector worldspace_posn2=screen_to_world_transformation(
      screenspace_posn2);

   double delta_z=worldspace_posn2.get(2)-worldspace_posn1.get(2);
   if (nearly_equal(delta_z,0))
   {
      ray_intercepts_zplane_flag=false;
   }
   else
   {
      double lambda_star=(Zplane-worldspace_posn1.get(2))/delta_z;
      intercept_posn=worldspace_posn1+lambda_star*(
         worldspace_posn2-worldspace_posn1);
   }
   
//   cout << "world-space intercept_posn = " << intercept_posn << endl;

   return ray_intercepts_zplane_flag;
}

// Member function compute_screen_ray_forward_position extrudes the
// world-space position corresponding to input screen space coords
// (X,Y) forward into the screen by input distance depth_range.

threevector Transformer::compute_screen_ray_forward_position(
   double X,double Y,double depth_range)
{   
   threevector screenspace_posn1(X,Y,0);
   threevector screenspace_posn2(X,Y,-1);
   threevector worldspace_posn1=screen_to_world_transformation(
      screenspace_posn1);
   threevector worldspace_posn2=screen_to_world_transformation(
      screenspace_posn2);

   return worldspace_posn1-depth_range*(worldspace_posn2-worldspace_posn1);
}

/*
// ---------------------------------------------------------------------
// Member function find_world_point_nearest_to_ray first constructs an
// STL vector containing geometries which are pierced by the input
// linesegment ray.  It subsequently loops over every vertex within
// those geometries lying close to the input ray.  It returns the
// absolute location of the vertex which lies closest to the ray.

threevector Transformer::find_world_point_nearest_to_ray(linesegment& ray)
{
   cout << "inside Transformer::find_world_point_nearest_to_ray()" << endl;
//   cout << "ray = " << ray << endl;

   double max_sphere_to_ray_frac_dist=1.5;
   if (DataGraphsGroup_ptr==NULL)
   {
      cout << "Error in Transformer::find_world_point_nearest_to_ray()!" 
           << endl;
      cout << "DataGraphsGroup_ptr = " << DataGraphsGroup_ptr << endl;
      exit(-1);
   }
   vector<pair<osg::Geometry*,osg::Matrix> > geoms_along_ray=
      DataGraphsGroup_ptr->geometries_along_ray(
         ray,max_sphere_to_ray_frac_dist);
//   cout << "geoms_along_ray.size() = " << geoms_along_ray.size() << endl;

   double min_separation=POSITIVEINFINITY;
   threevector closest_worldspace_point(
      NEGATIVEINFINITY,NEGATIVEINFINITY,NEGATIVEINFINITY);

   for (unsigned int g=0; g<geoms_along_ray.size(); g++)
   {
      osg::Geometry* curr_Geometry_ptr(geoms_along_ray[g].first);
      osg::Matrix MatrixTransform(geoms_along_ray[g].second);
      osg::Vec3Array* rel_vertices_ptr=dynamic_cast<osg::Vec3Array*>(
         curr_Geometry_ptr->getVertexArray());

// We must transform every vertex within the current Geometry by
// MatrixTransform:

      for (unsigned int i=0; i<rel_vertices_ptr->size(); i++)
      {
         threevector abs_vertex(rel_vertices_ptr->at(i)*MatrixTransform);
         double curr_separation=ray.point_to_line_distance(abs_vertex);

         if (curr_separation < min_separation)
         {
            min_separation=curr_separation;
            closest_worldspace_point=abs_vertex;
         }
      } // loop over index i labeling vertices within *curr_Geometry_ptr
   } // loop over index g labeling geodes in *DataGraphsGroup_ptr

//   cout << "Closest point = " << closest_worldspace_point << endl;
   return closest_worldspace_point;
}
*/
