// ==========================================================================
// Rectangle class member function definitions
// ==========================================================================
// Last updated on 6/15/08; 3/31/11; 7/10/11
// ==========================================================================

#include <iostream>
#include <osg/Geode>
#include <osg/LineWidth>
#include <osg/Node>
//#include <osg/PolygonStipple>
#include <osgDB/ReadFile>
#include <osg/Texture2D>
#include "osg/osgGeometry/Rectangle.h"
#include "math/threevector.h"
#include "math/twovector.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void Rectangle::allocate_member_objects()
{
   Stateset_map_ptr=new STATESET_MAP;
}		       

void Rectangle::initialize_member_objects()
{
   Graphical_name="Rectangle";

   canonical_length=0.05;
   canonical_width=0.05;

//   canonical_length=0.5;
//   canonical_width=0.25;

//   canonical_length=10;
//   canonical_width=5;

   double alpha=0.1;
//   double alpha=0.2;
//   double alpha=0.5;
   set_permanent_color(colorfunc::blue,alpha);
   set_selected_color(colorfunc::red,alpha);
}		       

Rectangle::Rectangle(int id,int ndims):
   Geometrical(ndims,id)
{	
   allocate_member_objects();
   initialize_member_objects();
}		       

Rectangle::~Rectangle()
{
//   cout << "inside Rectangle destructor" << endl;
   delete Stateset_map_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const Rectangle& rectangle)
{
   outstream << "inside Rectangle::operator<<" << endl;
   outstream << "bbox = " << rectangle.get_bbox() << endl;
   outstream << static_cast<const Geometrical&>(rectangle) << endl;
   
   return outstream;
}

// ==========================================================================
// Drawing methods
// ==========================================================================

// Member function generate_drawable_geode instantiates an osg::Geode
// containing an OpenGL QUAD drawable.

osg::Geode* Rectangle::generate_drawable_geode()
{
   geode_refptr = new osg::Geode();
   geode_refptr->addDrawable(generate_drawable_geom());
   return geode_refptr.get();
}

// ---------------------------------------------------------------------
// Member function generate_drawable_geom instantiates an OSG Geometry
// which which contains an OpenGL QUAD drawable.

osg::Geometry* Rectangle::generate_drawable_geom()
{
   geom_refptr=new osg::Geometry;
   const int n_vertices=4;
   vertices_refptr = new osg::Vec3Array(n_vertices);
   geom_refptr->setVertexArray(vertices_refptr.get());

   color_array_refptr = new osg::Vec4Array(n_vertices);
   geom_refptr->setColorArray(color_array_refptr.get());
   geom_refptr->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
   geom_refptr->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,n_vertices));

/*
// On 9/19/05, we found that adding a polygon stipple to *geom_refptr
// yields translucent rectangles.  This is useful for viewing video
// underneath measuring rectangles...

   osg::StateSet* stateSet = new osg::StateSet();
   geom_refptr->setStateSet(stateSet);
   osg::PolygonStipple* polygonStipple = new osg::PolygonStipple;
   stateSet->setAttributeAndModes(
      polygonStipple,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);
*/

// On 9/19/05, we found that the following StateSet commands enable
// alpha blending for rectangles appearing on top of video imagery...

   if (get_ndims()==2)
   {
      stateset_refptr=new osg::StateSet;
      stateset_refptr->setMode(GL_BLEND,osg::StateAttribute::ON);
      geom_refptr->setStateSet(stateset_refptr.get());
   }
   
   return geom_refptr.get();
}

// ---------------------------------------------------------------------
// Member function generate_texture_and_stateset creates a Texture2D
// object from an image read in from a file.  It then instantiates a
// StateSet for the Rectangle and assigns the texture to that
// StateSet.  The StateSet returned by this method should be assigned
// to the geode holding the Rectangle's geometry.

osg::StateSet* Rectangle::generate_texture_and_stateset(string image_filename)
{

// First check whether an osg::StateSet corresponding to the input
// image_filename has previously been instantiated and stored within
// *Stateset_map_ptr:

/*

   STATESET_MAP::iterator iter=Stateset_map_ptr->find(image_filename);
   if (iter != Stateset_map_ptr->end())
   {
      cout << "stateset corresponding to image_filename = " << image_filename
           << " already exists!" << endl;
      return iter->second;
   }
*/

// Since the mapping from vertices to texture coordinates is 1:1, we
// don't need to use an index array to map vertices to texture
// coordinates. We can do it directly with the 'setTexCoordArray'
// method of the Geometry class.  This method takes a variable that is
// an array of two dimensional vectors (osg::Vec2). This variable
// needs to have the same number of elements as our Geometry has
// vertices. Each array element defines the texture coordinate for the
// corresponding vertex in the vertex array.

// Recall that we need to "flip" the image wrt its vertical axis.  So
// we map the upper left corner to texture coords (0,1), the upper
// right corner to texture coords (1,1), the lower right corner to
// texture coords (1,0) and the lower left corner to texture coords
// (0,0):

   osg::Vec2Array* texcoords = new osg::Vec2Array(4);
   texcoords->at(0).set(0,1);
   texcoords->at(1).set(1,1);
   texcoords->at(2).set(1,0);
   texcoords->at(3).set(0,0);
   geom_refptr->setTexCoordArray(0,texcoords);
   geom_refptr->setColorBinding(osg::Geometry::BIND_OFF);

   face_texture_refptr=new osg::Texture2D;

// Protect from being optimized away as static state:

   face_texture_refptr->setDataVariance(osg::Object::DYNAMIC); 

// Load image from input file.  Then assign image to texture:

   osg::Image* face_image_ptr=osgDB::readImageFile(image_filename);
   if (face_image_ptr==NULL)
   {
      cout << "Couldn't find texture image" << endl;
      return NULL;
   }
   else
   {
      face_texture_refptr->setImage(face_image_ptr);

// Instantiate new StateSet and assign its texture unit 0 to the
// texture just created:

      stateset_refptr=new osg::StateSet;
      stateset_refptr->setTextureAttributeAndModes
         (0,face_texture_refptr.get(),osg::StateAttribute::ON);

      (*Stateset_map_ptr)[image_filename]=stateset_refptr.get();

      return stateset_refptr.get();
   } // face_image_ptr == NULL conditional
}

// ---------------------------------------------------------------------
// Member function set_canonical_local_vertices instantiates 4
// Rectangle corners whose positions are specified relative to their
// average location.  We later use a PositionAttitudeTransform to
// relocate, rotate and rescale the vertices below to generate an
// arbitrary Rectangle.

void Rectangle::set_canonical_local_vertices()
{
   (*vertices_refptr)[0]=osg::Vec3f(
      -0.5*canonical_length,0,-0.5*canonical_width);
   (*vertices_refptr)[1]=osg::Vec3f(
      0.5*canonical_length,0,-0.5*canonical_width);
   (*vertices_refptr)[2]=osg::Vec3f(
      0.5*canonical_length,0,0.5*canonical_width);
   (*vertices_refptr)[3]=osg::Vec3f(
      -0.5*canonical_length,0,0.5*canonical_width);
}

// ---------------------------------------------------------------------
// Member function reset_bbox() updates the min and max X and Y
// locations for the 2D Rectangle.  As of 7/10/11, this method assumes
// ndims==2.

void Rectangle::reset_bbox(double curr_t,int pass_number)
{
//   cout << "inside Rectangle::reset_bbox()" << endl;
//   cout << "t = " << curr_t << " pass_number = " << pass_number << endl;

   double xcenter=NEGATIVEINFINITY;
   double ycenter=NEGATIVEINFINITY;
   threevector Rectangle_posn;
   if (get_UVW_coords(curr_t,pass_number,Rectangle_posn))
   {
//      cout << "Rectangle_posn = " << Rectangle_posn << endl;
      xcenter=Rectangle_posn.get(0);
      ycenter=Rectangle_posn.get(1);
   }
//   cout << "Xcenter = " << xcenter << " Ycenter = " << ycenter << endl;

   double length=0;
   double width=0;
   threevector Rectangle_scale(1,1,1);
   if (get_scale(curr_t,pass_number,Rectangle_scale))
   {
//      cout << "Rectangle_scale = " << Rectangle_scale << endl;
      length=canonical_length*Rectangle_scale.get(0);
      width=canonical_width*Rectangle_scale.get(2);
   }
//   cout << "length = " << length << " width = " << width << endl;

   double xmin=xcenter-0.5*length;
   double xmax=xcenter+0.5*length;
   double ymin=ycenter-0.5*width;
   double ymax=ycenter+0.5*width;
 
   bbox.set_xy_bounds(xmin,xmax,ymin,ymax);
//   cout << "Rectangle bbox = " << bbox << endl;
}

// ---------------------------------------------------------------------
void Rectangle::set_world_vertices(
   const twovector& V0,const twovector& V1,const twovector& V2,
   const twovector& V3)
{
//   cout << "inside Rectangle::set_world_vertices() #1" << endl;
//   cout << "V0 = " << V0 << " V1 = " << V1 
//        << " V2 = " << V2 << " V3 = " << V3 << endl;
   
   (*vertices_refptr)[0]=osg::Vec3f(V0.get(0),0,V0.get(1));
   (*vertices_refptr)[1]=osg::Vec3f(V1.get(0),0,V1.get(1));
   (*vertices_refptr)[2]=osg::Vec3f(V2.get(0),0,V2.get(1));
   (*vertices_refptr)[3]=osg::Vec3f(V3.get(0),0,V3.get(1));
}

void Rectangle::set_world_vertices(
   const threevector& V0,const threevector& V1,const threevector& V2,
   const threevector& V3)
{
   (*vertices_refptr)[0]=osg::Vec3f(V0.get(0),V0.get(1),V0.get(2));
   (*vertices_refptr)[1]=osg::Vec3f(V1.get(0),V1.get(1),V1.get(2));
   (*vertices_refptr)[2]=osg::Vec3f(V2.get(0),V2.get(1),V2.get(2));
   (*vertices_refptr)[3]=osg::Vec3f(V3.get(0),V3.get(1),V3.get(2));
}

// ---------------------------------------------------------------------
// Member function compute_screenspace_vertex_posns retrieves the
// cumulative PositionAttitudeTransform matrix which maps the
// Rectangle's canonical local vertices to UVW screenspace
// coordinates.  It then multiples the canonical local set of
// Rectangle vertices by this matrix and returns the vertices' current
// screen space coordinates in output array screenspace_vertex[].

void Rectangle::compute_screenspace_vertex_posns(
   threevector screenspace_vertex[])
{
   osg::Matrix local_to_world_matrix;
   get_PAT_ptr()->computeLocalToWorldMatrix(local_to_world_matrix,NULL);

   for (int n=0; n<get_n_vertices(); n++)
   {
      osg::Vec3f V=local_to_world_matrix.preMult( (*vertices_refptr)[n] );
      if (get_ndims()==2)
      {
         screenspace_vertex[n]=threevector(V.x(),V.z(),V.y());
      }
      else if (get_ndims()==3)
      {
         screenspace_vertex[n]=threevector(V.x(),V.y(),V.z());
      }
//      cout << "n = " << n << " screenspace vertex = " << screenspace_vertex[n]
//           << endl;
   }      
}

