// ==========================================================================
// Header file for Triangle class
// ==========================================================================
// Last updated on 7/13/06; 8/27/06; 1/21/07; 10/21/07; 1/16/12
// ==========================================================================

#ifndef OSGTRIANGLE_H
#define OSGTRIANGLE_H

#include <iostream>
#include <osg/Array>
#include "osg/osgGeometry/Geometrical.h"
#include "color/colorfuncs.h"
#include "geometry/geometry_funcs.h"
#include "math/threevector.h"

// class osg::Geode;
// class osg::Geometry;
class plane;
class twovector;

class Triangle : public Geometrical
{

  public:
    
// Initialization, constructor and destructor functions:

   Triangle(int id,int ndims=2);
   virtual ~Triangle();
   friend std::ostream& operator<< (
      std::ostream& outstream,const Triangle& t);

// Set & get methods

   void set_vertex_ID(int n_vertex,int id);
   int get_vertex_ID(int n_vertex) const;

// Drawing methods:

   osg::Geode* generate_drawable_geode();
   void set_geom_vertices(
      const twovector& V0,const twovector& V1,const twovector& V2);
   void set_geom_vertices(
      const threevector& V0,const threevector& V1,const threevector& V2);
   void set_geom_vertices(const std::vector<threevector>& V);

// Mathematical methods:

   void update_plane();
   bool projection(const threevector& P,threevector& P_proj) const;
   bool z_projection(double x,double y,threevector& P_proj) const;

  protected:

  private:

   std::vector<int> vertex_ID;
   std::vector<threevector> curr_V;  // local copy for this class alone
   plane* plane_ptr;

   void allocate_member_objects(); 
   void initialize_member_objects();
   void docopy(const Triangle& t);
   void set_geom_vertices();
   osg::Geometry* generate_drawable_geom();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void Triangle::set_vertex_ID(int n_vertex,int id) 
{
   vertex_ID.at(n_vertex)=id;
}

inline int Triangle::get_vertex_ID(int n_vertex) const
{
   return vertex_ID.at(n_vertex);
}

// ---------------------------------------------------------------------
// Boolean member function projection takes in threevector P and
// projects it into the triangle's infinite plane.  It returns true if
// the projection of P lies inside the triangle.

inline bool Triangle::projection(const threevector& P,threevector& P_proj) 
   const
{
   P_proj=plane_ptr->projection_into_plane(P);
   return geometry_func::PointInsideTriangle(
      P_proj,curr_V[0],curr_V[1],curr_V[2]);
}

// Boolean member function z_projection is a minor variant of
// projection.  It extrudes the input (x,y) into the triangle's
// infinite plane.  It returns true if the extruded point lies inside
// the triangle.

inline bool Triangle::z_projection(double x,double y,threevector& P_proj) 
   const
{
   P_proj=threevector(x,y,plane_ptr->z_coordinate(x,y));
   return geometry_func::PointInsideTriangle(
      P_proj,curr_V[0],curr_V[1],curr_V[2]);
}


#endif // OSGTriangle.h



