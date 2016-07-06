// Note added on Thursday, Sept 3, 2009 at 3:48 pm:

// Need to create new method inside Geometrical class which initializes
// text_refptr vector members to NULL:

//   n_text_messages=1;
//   for (int m=0; m<get_n_text_messages(); m++)
//   {
//      text_refptr.push_back(static_cast<osgText::Text*>(NULL));
//   }

// Polyhedron::reset_edges_width() looks totally redundant relative to
// LineSegmentsGroup::set_width() !!!

// ==========================================================================
// Polyhedron class member function definitions
// ==========================================================================
// Last updated on 1/22/12; 1/23/12; 8/4/13; 4/6/14
// ==========================================================================

#include <string>
#include <osg/Depth>
#include <osg/Geode>
#include "osg/osgGraphicals/AnimationController.h"
#include "color/colorfuncs.h"
#include "math/constant_vectors.h"
#include "osg/osgGeometry/LineSegmentsGroup.h"
#include "math/mathfuncs.h"
#include "geometry/polygon.h"
#include "geometry/polyhedron.h"
#include "osg/osgGeometry/Polyhedron.h"

#include "templates/mytemplates.h"
#include "osg/osgfuncs.h"
#include "general/outputfuncs.h"


using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void Polyhedron::allocate_member_objects()
{
}		       

void Polyhedron::initialize_member_objects()
{
   Graphical_name="Polyhedron";
   dynamically_instantiated_polyhedron_flag=false;
   selected_vertex_ID=selected_edge_ID=selected_face_ID=-1;
   polyhedron_ptr=NULL;
   Cylinder_ptr=NULL;

   set_selected_color(colorfunc::red);
   set_permanent_color(colorfunc::white);

   selected_vertex_ID=selected_edge_ID=selected_face_ID=-1;

   n_text_messages=1;
   for (unsigned int m=0; m<get_n_text_messages(); m++)
   {
      text_refptr.push_back(static_cast<osgText::Text*>(NULL));
   }
}		       

Polyhedron::Polyhedron(
   Pass* PI_ptr,const threevector& grid_world_origin,osgText::Font* f_ptr,
   int id,AnimationController* AC_ptr):
   Geometrical(3,id,AC_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
   this->grid_world_origin=grid_world_origin;
   font_refptr=f_ptr;

   LineSegmentsGroup_ptr=new LineSegmentsGroup(
      3,PI_ptr,AnimationController_ptr);
   initialize_linesegments();

   PointsGroup_ptr=new osgGeometry::PointsGroup(
      3,PI_ptr,AnimationController_ptr);
}		       

Polyhedron::Polyhedron(
   Pass* PI_ptr,const threevector& grid_world_origin,
   polyhedron* p_ptr,osgText::Font* f_ptr,int id,AnimationController* AC_ptr):
   Geometrical(3,id,AC_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
   this->grid_world_origin=grid_world_origin;
   font_refptr=f_ptr;
   if (p_ptr != NULL) 
   {
      polyhedron_ptr=new polyhedron(*p_ptr);
      dynamically_instantiated_polyhedron_flag=true;
   }
   
   LineSegmentsGroup_ptr=new LineSegmentsGroup(
      3,PI_ptr,AnimationController_ptr);
   initialize_linesegments();

   PointsGroup_ptr=new osgGeometry::PointsGroup(
      3,PI_ptr,AnimationController_ptr);
   generate_vertex_Points();
}		       

Polyhedron::~Polyhedron()
{
//   cout << "inside Polyhedron destructor" << endl;
   delete LineSegmentsGroup_ptr;
   delete PointsGroup_ptr;

   if (dynamically_instantiated_polyhedron_flag)
   {
      delete polyhedron_ptr;
   }
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const Polyhedron& P)
{
   outstream << "inside Polyhedron::operator<<" << endl;
   outstream << static_cast<const Geometrical&>(P) << endl;
   if (P.get_polyhedron_ptr() != NULL)
   {
      outstream << "polyhedron_ptr = " << P.get_polyhedron_ptr() << endl;
      outstream << "*polyhedron_ptr = " << *(P.get_polyhedron_ptr()) << endl;
   }
   return(outstream);
}

// ==========================================================================
// Set & get methods
// ==========================================================================

void Polyhedron::set_Cylinder_ptr(Cylinder* Cylinder_ptr)
{
   this->Cylinder_ptr=Cylinder_ptr;
}

Cylinder* Polyhedron::get_Cylinder_ptr()
{
   return Cylinder_ptr;
}

const Cylinder* Polyhedron::get_Cylinder_ptr() const
{
   return Cylinder_ptr;
}

// ==========================================================================
// Polyhedron generation methods
// ==========================================================================

// Member function generate_vertex_Points() fills member
// *PointsGroup_ptr with vertices from *polyhedron_ptr.

void Polyhedron::generate_vertex_Points()
{
//   cout << "inside Polyhedron::generate_vertex_Points()" << endl;

   if (polyhedron_ptr==NULL) return;
   
   PointsGroup_ptr->destroy_all_Points();

   threevector origin=polyhedron_ptr->get_origin();
//   cout << "n_vertices = " << polyhedron_ptr->get_n_vertices() << endl;
   for (unsigned int v=0; v<polyhedron_ptr->get_n_vertices(); v++)
   {
      threevector vertex_posn=polyhedron_ptr->get_vertex(v).get_posn()-origin;
//      cout << "v = " << v << " vertex_posn = " << vertex_posn << endl;
      PointsGroup_ptr->generate_new_Point(vertex_posn);
   } // loop over index v labeling polyhedron vertices

// As of 1/23/12, we mask the Polyhedron's vertices by default:

   PointsGroup_ptr->set_OSGgroup_nodemask(0);
}

// ---------------------------------------------------------------------
// Member function initialize_linesegments() 

void Polyhedron::initialize_linesegments()
{
//    cout << "inside Polyhedron::initialize_linesegments()" << endl;

   if (polyhedron_ptr != NULL)
   {
      LineSegmentsGroup_ptr->generate_canonical_segments(
         polyhedron_ptr->get_n_external_edges());
//      cout << "n_external_edges = " << polyhedron_ptr->get_n_external_edges()
//           << endl;
   }
}

// ---------------------------------------------------------------------
// Member function get_or_create_LineSegment_ptr takes in ID integer
// n.  If n is larger than the number of LineSegments already within
// LineSegmentsGroup, this method instantiates new canonical
// LineSegments and appends them to LineSegmentsGroup.  It then
// returns the LineSegment pointer requested by the input integer
// argument.

LineSegment* Polyhedron::get_or_create_LineSegment_ptr(unsigned int n)
{
   if (n >= LineSegmentsGroup_ptr->get_n_Graphicals())
   {
//      cout << "inside Polyhedron::get_or_create_LineSegment_ptr(), n = " 
//           << n << endl;
      unsigned int n_start=LineSegmentsGroup_ptr->get_n_Graphicals();
//      int n_stop=n+1;
      LineSegmentsGroup_ptr->generate_canonical_segments(n+1-n_start);
//      cout << "Created " << n+1-n_start 
//           << " new canonical LineSegments" << endl;
   }

   return LineSegmentsGroup_ptr->get_LineSegment_ptr(n);
}

// ---------------------------------------------------------------------
// Member function build_current_polyhedron()

void Polyhedron::build_current_polyhedron(double curr_t,int pass_number)
{
//   cout << "inside Polyhedron::build_current_polyhedron" << endl;
//   cout << "polyhedron_ptr = " << polyhedron_ptr << endl;

   if (polyhedron_ptr != NULL)
   {
      threevector origin=polyhedron_ptr->get_origin();
      set_UVW_coords(curr_t,pass_number,origin);
//      cout << "origin = " << origin << endl;
//      cout << "*polyhedron_ptr = " << *polyhedron_ptr << endl;

// LineSegment coordinates are specified relative to origin:

      int external_edge_counter=0;
      for (unsigned int i=0; i<polyhedron_ptr->get_n_edges(); i++)
      {
//         cout << "edge index i = " << i << " internal edge flag = " 
//              << polyhedron_ptr->get_internal_edge_flag(i) << endl;
         
         if (!(polyhedron_ptr->get_internal_edge_flag(i)))
         {
            threevector V1=polyhedron_ptr->get_edge_V1(i).get_posn();
            threevector V2=polyhedron_ptr->get_edge_V2(i).get_posn();

            LineSegment* curr_segment_ptr=get_or_create_LineSegment_ptr(
               external_edge_counter++);

            curr_segment_ptr->set_scale_attitude_posn(
               curr_t,pass_number,V1-origin,V2-origin);

         } // !internal_edge_flag conditional
      } // loop over index i labeling polyhedron edges

      update_triangle_mesh();

   } // polyhedron_ptr != NULL conditional
}

// ==========================================================================
// Drawing member functions
// ==========================================================================

// Polyhedron::reset_edges_width() looks totally redundant relative to
// LineSegmentsGroup::set_width() !!!

// Member function reset_edges_width loops over every LineSegment
// within member *LineSegmentsGroup_ptr.  It resets their line widths
// to the input value.

void Polyhedron::reset_edges_width(double width)
{
//   cout << "inside Polyhedron::reset_edges_width(), width = " << width
//        << endl;

   for (unsigned int i=0; i<LineSegmentsGroup_ptr->get_n_Graphicals(); i++)
   {
      LineSegment* curr_Segment_ptr=
         LineSegmentsGroup_ptr->get_LineSegment_ptr(i);
      curr_Segment_ptr->get_LineWidth_ptr()->setWidth(width);
   } // loop over index i labeling LineSegments within Polyhedron object
}

// ---------------------------------------------------------------------
// Member function generate_drawable_geode dynamically generates a
// geode.  It adds an OSG triangle mesh for the polyhedron to this geode.

osg::Geode* Polyhedron::generate_drawable_geode(
   bool text_screen_axis_alignment_flag)
{
//   cout << "inside Polyhedron::generate_drawable_geode()" << endl;
//   cout << "polyhedron_ptr = " << polyhedron_ptr << endl;

   if (polyhedron_ptr==NULL) return NULL;
//   cout << "*polyhedron_ptr = " << *polyhedron_ptr << endl;

   geode_refptr=new osg::Geode;

   mesh_refptr=new osg::TriangleMesh;
   shape_refptr=new osg::ShapeDrawable(mesh_refptr.get());
   generate_triangle_mesh();

   osg::StateSet* stateset_ptr=new osg::StateSet;
   stateset_ptr->setMode(GL_BLEND,osg::StateAttribute::ON);
   stateset_ptr->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );

// Helpful hint from Ross on 12/17/07:

//   int val=stateset_ptr->getMode(GL_DEPTH_WRITEMASK);
//   cout << "orig value = " << val << endl;

   stateset_ptr->setMode(GL_DEPTH_WRITEMASK,osg::StateAttribute::OFF);

   stateset_ptr->setAttributeAndModes(
      new osg::Depth(osg::Depth::LESS,0.0, 1.0, false),
      osg::StateAttribute::ON);

// val=stateset_ptr->getMode(GL_DEPTH_WRITEMASK); 
//   cout << "new value = " << val << endl;

   shape_refptr->setStateSet(stateset_ptr);

   geode_refptr->addDrawable(shape_refptr.get());
   return geode_refptr.get();
}

// ---------------------------------------------------------------------
// Member function generate_triangle_mesh fills an OSG triangle mesh
// with vertex chain information for each face of the current
// polyhedron object. 

void Polyhedron::generate_triangle_mesh()
{
//   cout << "inside Polyhedron::generate_triangle_mesh()" << endl;

   if (!mesh_refptr.valid()) return;

//   cout << "*polyhedron_ptr = " << *polyhedron_ptr << endl;
//   cout << "polyhedron_ptr->get_origin() = " 
//        << polyhedron_ptr->get_origin() << endl;

   if (polyhedron_ptr==NULL) return;

   int n_vertices=polyhedron_ptr->get_n_vertices();
   osg::Vec3Array* vertices = new osg::Vec3Array(n_vertices);

   int n_faces=polyhedron_ptr->get_n_faces();
   osg::UIntArray* indices = new osg::UIntArray(3*n_faces);

//   cout << "n_vertices = " << n_vertices
//        <<  " n_faces = " << n_faces << endl;

   mesh_refptr->setVertices(vertices);
   mesh_refptr->setIndices(indices);

//   cout << "vertices->size() = " << vertices->size() << endl;
//   cout << "indices->size() = " << indices->size() << endl;
}

// ---------------------------------------------------------------------
void Polyhedron::update_triangle_mesh()
{
//   cout << "inside Polyhedron::update_triangle_mesh()" << endl;
//   cout << "polyhedron_ptr = " << polyhedron_ptr << endl;
//   cout << "*polyhedron_ptr = " << *polyhedron_ptr << endl;
//   cout << "polyhedron_ptr->get_origin() = " 
//        << polyhedron_ptr->get_origin() << endl;
   
   if ( polyhedron_ptr==NULL || !mesh_refptr.valid() ) return;

   unsigned int n_vertices=polyhedron_ptr->get_n_vertices();
   for (unsigned int v=0; v<n_vertices; v++)
   {
      threevector rel_vertex=
         polyhedron_ptr->get_vertex(v).get_posn()-
         polyhedron_ptr->get_origin();
      mesh_refptr->getVertices()->at(v).set(
         rel_vertex.get(0),rel_vertex.get(1),rel_vertex.get(2));
   }
//   cout << "polyhedron.origin = " << polyhedron_ptr->get_origin() << endl;

   unsigned int n_faces=polyhedron_ptr->get_n_faces();
   int vertex_counter=0;

   for (unsigned int f=0; f<n_faces; f++)
   {
      face* curr_face_ptr=polyhedron_ptr->get_face_ptr(f);
      osg::UIntArray* indices = static_cast<osg::UIntArray*>(
         mesh_refptr->getIndices());
      for (unsigned int i=0; i<curr_face_ptr->get_n_vertices(); i++)
      {
         vertex curr_vertex=curr_face_ptr->get_vertex_from_chain(i);
         (*indices)[vertex_counter]=curr_vertex.get_ID();

//         threevector posn(curr_vertex.get_posn());
//         cout << "i = " << i 
//              << " vertex_counter = " << vertex_counter
//              << " face f=" << f
//              << " vertex ID = " << curr_vertex.get_ID()
//              << " x=" << posn.get(0) << " y=" << posn.get(1) 
//              << " z=" << posn.get(2) << endl;
         vertex_counter++;
      }
   }

   dirtyDisplay();
}

// ---------------------------------------------------------------------
// Member function set_color sets the colors of the current polyhedron
// edges and volume.  Internal edges given the same color as the
// volume in order to hide them.

void Polyhedron::set_color(const osg::Vec4& edge_color)
{
   set_color(edge_color,osg::Vec4(0,0,0,0));
}

void Polyhedron::set_color(const osg::Vec4& edge_color,
                           const osg::Vec4& volume_color)
{
//   cout << "inside Polyhedron::set_color()" << endl;

   if (!shape_refptr.valid())
   {
      cout << "Error in Polyhedron::set_color()" << endl;
      cout << "shape_refptr.valid()==0 !!!" << endl;
      outputfunc::enter_continue_char();
      return;
   }
   
   shape_refptr->setColor(volume_color); // colors polyhedron's volume
   set_edge_color(edge_color);
}

// ---------------------------------------------------------------------
// Member function set_edge_color sets the color of all members of
// *LineSegmentsGroup_ptr which contains only external polyhedron edges.

void Polyhedron::set_edge_color(const osg::Vec4& external_edge_color)
{
//   cout << "inside Polyhedron::set_edge_color()" << endl;
//   cout << "External_edge_color = " << endl;
//   osgfunc::print_Vec4(external_edge_color);
   
   for (unsigned int i=0; i<LineSegmentsGroup_ptr->get_n_Graphicals(); i++)
   {
      LineSegment* curr_segment_ptr=LineSegmentsGroup_ptr->
         get_LineSegment_ptr(i);

      curr_segment_ptr->set_permanent_color(external_edge_color);      
      curr_segment_ptr->set_curr_color(external_edge_color);      
   }
   LineSegmentsGroup_ptr->reset_colors();
}

// ---------------------------------------------------------------------
// Member function reset_volume_alpha() holds fixed the RGB value of
// the Polyhedron's volume coloring but resets its alpha value.

void Polyhedron::reset_volume_alpha(double alpha)
{
//    cout << "inside Polyhedron::reset_volume_alpha()" << endl;

   if (!shape_refptr.valid())
   {
      cout << "Error in Polyhedron::reset_volume_alpha()" << endl;
      cout << "shape_refptr.valid()==0 !!!" << endl;
      outputfunc::enter_continue_char();
      return;
   }

   osg::Vec4 curr_Color=shape_refptr->getColor();
   osg::Vec4 new_Color(curr_Color.r(),curr_Color.g(),curr_Color.b(),alpha);
   shape_refptr->setColor(new_Color); 
}

// ==========================================================================
// Text member functions
// ==========================================================================

// Member function generate_text

osgText::Text* Polyhedron::generate_text(
   int i,const threevector& text_posn,colorfunc::Color text_color,
   bool text_screen_axis_alignment_flag)
{
//   cout << "inside Polyhedron::generate_text, alignment_flag = "
//        << text_screen_axis_alignment_flag << endl;

   text_refptr[i]=new osgText::Text;
   geode_refptr->addDrawable(text_refptr[i].get());
   Geometrical::initialize_text(i);

//   float char_size=0.7*size[get_ndims()];
//   float char_size=0.85*size[get_ndims()];
//   float char_size=size[get_ndims()];
   float char_size=2*size[get_ndims()];
   text_refptr[i]->setCharacterSize(char_size);
   text_refptr[i]->setAlignment(osgText::Text::CENTER_CENTER);

// If text_screen_axis_alignment_flag==true, cylinder text is always
// oriented horizontally wrt the screen independent of the camera's
// attitude:

   if (text_screen_axis_alignment_flag)
   {
      text_refptr[i]->setAxisAlignment(osgText::Text::SCREEN);
   }
   else
   {
      text_refptr[i]->setAxisAlignment(osgText::Text::XY_PLANE);
   }

// Set maximum width of text box to force long text strings to wrap
// around into several smaller, more readable lines:

//   text_refptr[i]->setMaximumWidth(10*char_size);
//   text_refptr[i]->setMaximumWidth(20*char_size);
//   text_refptr[i]->setMaximumWidth(25*char_size);

   osg::Vec3 text_position(
      text_posn.get(0),text_posn.get(1),text_posn.get(2));
   text_refptr[i]->setPosition(text_position);
   text_refptr[i]->setColor(colorfunc::get_OSG_color(text_color));

   return text_refptr[0].get();
}

// ==========================================================================
// Manipulation member functions
// ==========================================================================

// Member function scale_rotate_and_then_translate()
   
void Polyhedron::scale_rotate_and_then_translate(
   double curr_t,int pass_number,
   double theta,double phi,const threevector& scale,
   const threevector& trans)
{
   cout << "inside Polyhedron::scale_rotate_and_then_translate()" << endl;
//   cout << "Scale = " << scale << endl;
//   cout << "theta = " << theta*180/PI << endl;
//   cout << "phi = " << phi*180/PI << endl;
//   cout << "trans = " << trans << endl;

   rotation F,Ry,Rz,R,RF;

// Rotate polyhedron cone about y-axis by angle theta:

   double cos_theta=cos(theta);
   double sin_theta=sin(theta);

   Ry.put(0,0,cos_theta);
   Ry.put(0,2,sin_theta);
   Ry.put(1,1,1);
   Ry.put(2,0,-sin_theta);
   Ry.put(2,2,cos_theta);

// Rotate polyhedron about z-axis by angle phi.  Cone's symmetry
// axis is then oriented along (sin_theta cos_phi) x_hat + (sin_theta
// sin_phi) y_hat + cos_theta z_hat:

   double cos_phi=cos(phi);
   double sin_phi=sin(phi);

   Rz.put(0,0,cos_phi);
   Rz.put(0,1,-sin_phi);
   Rz.put(1,0,sin_phi);
   Rz.put(1,1,cos_phi);
   Rz.put(2,2,1);

   R=Rz*Ry;

//   cout << "Ry = " << Ry << endl;
//   cout << "Rz = " << Rz << endl;
//   cout << "R = " << R << endl;
   
   if (!R.rotation_sanity_check())
   {
      cout << "Error in Polyhedron::scale_rotate_and_then_translate_cone()" << endl;
      cout << "R is not a proper rotation!" << endl;
      exit(-1);
   }

   const threevector origin(0,0,0);
   Graphical::scale_rotate_and_then_translate(
      curr_t,pass_number,origin,R,scale,trans);
}

