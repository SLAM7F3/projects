// ==========================================================================
// Polygon class member function definitions
// ==========================================================================
// Last updated on 6/15/08; 9/24/09; 6/28/12; 4/6/14
// ==========================================================================

#include <iostream>
#include <osg/Geode>
#include <osg/Node>
#include <osg/PolygonStipple>
#include "passes/Pass.h"
#include "osg/osgGeometry/PointsGroup.h"
#include "osg/osgGeometry/Polygon.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;

namespace osgGeometry
{

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

   void Polygon::allocate_member_objects()
      {
      }		       

   void Polygon::initialize_member_objects()
      {
         Graphical_name="Polygon";
         PointsGroup_ptr=NULL;
         PolyLine_ptr=NULL;
      }		       

   Polygon::Polygon(
      const int p_ndims,Pass* PI_ptr,threevector* GO_ptr,
      const threevector& reference_origin,const polygon& p,int id):
      Geometrical(p_ndims,id)
      {	
//         cout << "inside Polygon constructor" << endl;
         
         allocate_member_objects();
         initialize_member_objects();

         this->reference_origin=reference_origin;
         relative_poly_ptr=new polygon(p);

//         cout << "reference_origin = " << reference_origin << endl;
//         cout << "*relative_poly_ptr = " << *relative_poly_ptr
//              << endl;

         PointsGroup_ptr=new osgGeometry::PointsGroup(p_ndims,PI_ptr,GO_ptr);
      }		       

   Polygon::~Polygon()
      {
         delete relative_poly_ptr;
      }

// ---------------------------------------------------------------------
// Overload << operator

   ostream& operator<< (ostream& outstream,const Polygon& p)
      {
         outstream << "inside Polygon::operator<<" << endl;
         outstream << static_cast<const Geometrical&>(p) << endl;
         if (p.relative_poly_ptr != NULL)
         {
            outstream << "*relative_poly_ptr = " << *(p.relative_poly_ptr) 
                      << endl;
         }
         return(outstream);
      }

// ==========================================================================
// Drawing methods
// ==========================================================================

// Member function generate_drawable_geode instantiates an osg::Geode
// containing an OpenGL QUAD drawable.

   osg::Geode* Polygon::generate_drawable_geode()
      {
         geode_refptr = new osg::Geode();
         geode_refptr->addDrawable(generate_drawable_geom());
         set_curr_color(colorfunc::red);
//   set_permanent_color(colorfunc::blue);
         set_permanent_color(colorfunc::red);
         return geode_refptr.get();
      }

// ---------------------------------------------------------------------
// Member function generate_drawable_geom instantiates an OSG Geometry
// which which contains an OpenGL Polygon drawable.

   osg::Geometry* Polygon::generate_drawable_geom()
      {
//         cout << "inside Polygon::generate_drawable_geom()" << endl;
         
         geom_refptr=new osg::Geometry;

         int n_vertices=relative_poly_ptr->get_nvertices();
         vertices_refptr = new osg::Vec3Array(n_vertices);
         geom_refptr->setVertexArray(vertices_refptr.get());

         color_array_refptr = new osg::Vec4Array(n_vertices);
         geom_refptr->setColorArray(color_array_refptr.get());
         geom_refptr->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

         geom_refptr->addPrimitiveSet(new osg::DrawArrays(
            osg::PrimitiveSet::POLYGON,0,n_vertices));

         if (get_ndims()==2) enable_alpha_blending();
         return geom_refptr.get();
      }

// ---------------------------------------------------------------------
// Member function enable_alpha_blending()

   void Polygon::enable_alpha_blending()
      {
//         cout << "inside Polygon::enable_alpha_blending()" << endl;
         
         if (!stateset_refptr.valid()) 
         {
            stateset_refptr = new osg::StateSet;
            geom_refptr->setStateSet(stateset_refptr.get());
         }
         stateset_refptr->setMode(GL_BLEND,osg::StateAttribute::ON);
         stateset_refptr->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );

/*
// On 9/19/05, we found that adding a polygon stipple to *geom_refptr
// yields translucent polygons.  This is useful for viewing video
// underneath measuring polygons...

  osg::PolygonStipple* polygonStipple = new osg::PolygonStipple;
  stateset_refptr->setAttributeAndModes(
  polygonStipple,osg::StateAttribute::OVERRIDE|
  osg::StateAttribute::ON);
*/

// On 9/19/05, we found that the following StateSet commands enable
// alpha blending for polygons appearing on top of video imagery...

//         stateset_refptr->setMode(GL_BLEND,osg::StateAttribute::ON);


      }

// ---------------------------------------------------------------------
// Member function set_relative_vertices fills *vertices_refptr with
// relative vertex information with respect to reference_origin.  To
// avoid roundoff problems, we simply need to incorporate the global
// reference_origin translation within the Polygon's PAT.

   void Polygon::set_relative_vertices()
      {
         for (unsigned int n=0; n<relative_poly_ptr->get_nvertices(); n++)
         {
            threevector rel_V=relative_poly_ptr->get_vertex(n);

            if (get_ndims()==2)
            {
               vertices_refptr->at(n)=
                  osg::Vec3f(rel_V.get(0),0,rel_V.get(1));
            }
            else if (get_ndims()==3)
            {
               vertices_refptr->at(n)=
                  osg::Vec3f(rel_V.get(0),rel_V.get(1),rel_V.get(2));
            }
         
         } // loop over index n labeling polygon vertices
      }

} // osgGeometry namespace
