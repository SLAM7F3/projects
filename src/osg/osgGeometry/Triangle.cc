// ==========================================================================
// Triangle class member function definitions
// ==========================================================================
// Last updated on 1/3/07; 1/4/07; 1/21/07; 6/15/08
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osg/Geode>
#include <osg/LineWidth>
#include <osg/Node>
#include "geometry/plane.h"
#include "osg/osgGeometry/Triangle.h"
#include "math/twovector.h"

using std::cout;
using std::endl;
using std::ostream;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void Triangle::allocate_member_objects()
{
}		       

void Triangle::initialize_member_objects()
{
   Graphical_name="Triangle";
   for (int i=0; i<3; i++)
   {
      vertex_ID.push_back(-1);
      curr_V.push_back(threevector(0,0,0));
   }
   plane_ptr=new plane();
}		       

Triangle::Triangle(int id,int ndims):
   Geometrical(ndims,id)
{	
   allocate_member_objects();
   initialize_member_objects();
}		       

Triangle::~Triangle()
{
   delete plane_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const Triangle& t)
{
   outstream << "inside Triangle::operator<<" << endl;
   outstream << "V0 = ( " 
             << t.vertices_refptr->at(0).x() << " , " 
             << t.vertices_refptr->at(0).y() << " , " 
             << t.vertices_refptr->at(0).z() << " )" << endl;
   outstream << "V1 = ( " 
             << t.vertices_refptr->at(1).x() << " , " 
             << t.vertices_refptr->at(1).y() << " , " 
             << t.vertices_refptr->at(1).z() << " )" << endl;
   outstream << "V2 = ( " 
             << t.vertices_refptr->at(2).x() << " , " 
             << t.vertices_refptr->at(2).y() << " , " 
             << t.vertices_refptr->at(2).z() << " )" << endl;
   outstream << static_cast<const Geometrical&>(t) << endl;
   return(outstream);
}

// ==========================================================================
// Drawing methods
// ==========================================================================

// Member function generate_drawable_geode instantiates an osg::Geode
// containing an OpenGL TRIANGLE drawable.

osg::Geode* Triangle::generate_drawable_geode()
{
   geode_refptr = new osg::Geode();
   geode_refptr->addDrawable(generate_drawable_geom());
   return geode_refptr.get();
}

// ---------------------------------------------------------------------
// Member function generate_drawable_geom instantiates Graphical
// member object *geom_ptr which contains an OpenGL TRIANGLE drawable.

osg::Geometry* Triangle::generate_drawable_geom()
{
//   Graphical::generate_drawable();

   osg::Geometry* geom_ptr=new osg::Geometry;

   const int n_vertices=3;
   vertices_refptr = new osg::Vec3Array(n_vertices);
   geom_ptr->setVertexArray(vertices_refptr.get());

   color_array_refptr = new osg::Vec4Array(n_vertices);
   geom_ptr->setColorArray(color_array_refptr.get());
   geom_ptr->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

   geom_ptr->addPrimitiveSet(
      new osg::DrawArrays(GL_TRIANGLES,0,n_vertices));

// Following StateSet commands enable alpha blending for triangles
// appearing on top of video imagery...

   osg::StateSet* stateset_ptr=new osg::StateSet;
   stateset_ptr->setMode(GL_BLEND,osg::StateAttribute::ON);
   geom_ptr->setStateSet(stateset_ptr);
   return geom_ptr;
}

// ---------------------------------------------------------------------
void Triangle::set_geom_vertices(
   const twovector& V0,const twovector& V1,const twovector& V2)
{
   vertices_refptr->at(0)=osg::Vec3f(V0.get(0),0,V0.get(1));
   vertices_refptr->at(1)=osg::Vec3f(V1.get(0),0,V1.get(1));
   vertices_refptr->at(2)=osg::Vec3f(V2.get(0),0,V2.get(1));
   set_geom_vertices();
}

void Triangle::set_geom_vertices(
   const threevector& V0,const threevector& V1,const threevector& V2)
{
   vertices_refptr->at(0)=osg::Vec3f(V0.get(0),V0.get(1),V0.get(2));
   vertices_refptr->at(1)=osg::Vec3f(V1.get(0),V1.get(1),V1.get(2));
   vertices_refptr->at(2)=osg::Vec3f(V2.get(0),V2.get(1),V2.get(2));
   set_geom_vertices();
}

void Triangle::set_geom_vertices(const vector<threevector>& V)
{
   if (V.size()==3)
   {
      vertices_refptr->at(0)=osg::Vec3f(V[0].get(0),V[0].get(1),V[0].get(2));
      vertices_refptr->at(1)=osg::Vec3f(V[1].get(0),V[1].get(1),V[1].get(2));
      vertices_refptr->at(2)=osg::Vec3f(V[2].get(0),V[2].get(1),V[2].get(2));
      set_geom_vertices();
   }
}

void Triangle::set_geom_vertices()
{
   curr_V.clear();
   for (int i=0; i<3; i++)
   {
      curr_V.push_back(threevector(vertices_refptr->at(i)));
   }
}

// ==========================================================================
// Mathematical methods
// ==========================================================================

void Triangle::update_plane()
{
   *plane_ptr=plane(threevector(vertices_refptr->at(0)),
                    threevector(vertices_refptr->at(1)),
                    threevector(vertices_refptr->at(2)));
}
