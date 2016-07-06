// ==========================================================================
// Pyramid class member function definitions
// ==========================================================================
// Last updated on 7/13/08; 7/14/08; 7/22/09; 1/19/10; 4/6/14
// ==========================================================================

#include <string>
#include "osg/osgGraphicals/AnimationController.h"
#include "color/colorfuncs.h"
#include "osg/osgGeometry/LineSegmentsGroup.h"
#include "geometry/pyramid.h"
#include "osg/osgGeometry/Pyramid.h"

#include "math/mathfuncs.h"
#include "numrec/nrfuncs.h"
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

void Pyramid::allocate_member_objects()
{
}		       

void Pyramid::initialize_member_objects()
{
   Graphical_name="Pyramid";
   pyramid_ptr=NULL;
}		       

Pyramid::Pyramid(
   Pass* PI_ptr,const threevector& grid_world_origin,pyramid* p_ptr,
   osgText::Font* f_ptr,int id,AnimationController* AC_ptr):
   Polyhedron(PI_ptr,grid_world_origin,p_ptr,f_ptr,id,AC_ptr)
{	
//   cout << "inside Pyramid constructor" << endl;

   initialize_member_objects();
   allocate_member_objects();

   pyramid_ptr=p_ptr;
}		       

Pyramid::~Pyramid()
{
//   cout << "inside Pyramid destructor" << endl;
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const Pyramid& P)
{
   outstream << "inside Pyramid::operator<<" << endl;
//   outstream << static_cast<const Polyhedron&>(P) << endl;
   if (P.get_pyramid_ptr() != NULL)
   {
      outstream << "pyramid_ptr = " << P.get_pyramid_ptr() << endl;
      outstream << "*pyramid_ptr = " << *(P.get_pyramid_ptr()) << endl;
   }
   return(outstream);
}

// ==========================================================================
// Pyramid generation methods
// ==========================================================================

void Pyramid::build_canonical_pyramid(double curr_t,int pass_number)
{
//   cout << "inside Pyramid::build_canonical_pyramid" << endl;
   pyramid canonical_pyramid;
   threevector origin=canonical_pyramid.get_origin();

   int external_edge_counter=0;
   for (unsigned int e=0; e<canonical_pyramid.get_n_edges(); e++)
   {
      if (!(canonical_pyramid.get_internal_edge_flag(e)))
      {
         threevector V1=(canonical_pyramid.get_edge_V1(e).get_posn());
         threevector V2=(canonical_pyramid.get_edge_V2(e).get_posn());

         LineSegment* curr_segment_ptr=get_or_create_LineSegment_ptr(
            external_edge_counter++);

         curr_segment_ptr->set_scale_attitude_posn(
            curr_t,pass_number,V1-origin,V2-origin);
      } // !internal_edge_flag conditional
   } // loop over index e labeling pyramid edges
//   outputfunc::enter_continue_char();

} 

// ---------------------------------------------------------------------
void Pyramid::build_current_pyramid(
   double curr_t,int pass_number,pyramid* curr_pyramid_ptr)
{
//   cout << "inside Pyramid::build_current_pyramid()" << endl;
//   cout << "curr_t = " << curr_t << endl;

   if (curr_pyramid_ptr != NULL)
   {
      pyramid_ptr=curr_pyramid_ptr;
      threevector origin( curr_pyramid_ptr->get_origin() );
//      cout << "curr_pyramid_ptr->origin = " << origin << endl;

      set_UVW_coords(curr_t,pass_number,origin);
//      cout << "*curr_pyramid_ptr = " << *curr_pyramid_ptr << endl;

// On 7/22/09, we discovered the painful and hard way that the scale
// for Pyramids corresponding to OBSFRUSTA which are regenerated after
// the time step is decreased in QTLOS would equal NEGATIVEINFINITY.
// So we explicitly set the scale to unity below to avoid this
// catastrophic behavior!

      threevector scale(1,1,1);
      set_scale(curr_t,pass_number,scale);

// LineSegment coordinates are specified relative to origin:

      int external_edge_counter=0;
      for (unsigned int e=0; e<curr_pyramid_ptr->get_n_edges(); e++)
      {
//         cout << "e = " << e << " internal_edge_flag(e) = "
//              << curr_pyramid_ptr->get_internal_edge_flag(e) << endl;

         if (!(curr_pyramid_ptr->get_internal_edge_flag(e)))
         {
            threevector V1(curr_pyramid_ptr->get_edge_V1(e).get_posn());
            threevector V2(curr_pyramid_ptr->get_edge_V2(e).get_posn());
//            cout << "V1 = " << V1 << " V2 = " << V2 << endl;

            LineSegment* curr_segment_ptr=get_or_create_LineSegment_ptr(
               external_edge_counter++);
            curr_segment_ptr->set_scale_attitude_posn(
               curr_t,pass_number,V1-origin,V2-origin);
         } // !internal_edge_flag conditional
      } // loop over index i labeling pyramid edges

      update_triangle_mesh();
      
   } // curr_pyramid_ptr != NULL conditional
}

// ---------------------------------------------------------------------
// Member function store_zplane_vertices saves the apex and vertex
// chain for the input *curr_pyramid_ptr's within the current Pyramid
// graphical's time-dependent vertices.  We wrote this method in order
// to be able to reconstruct time-varying zplane faces for animation
// purposes.

void Pyramid::store_apex_and_zplane_vertices(
   double curr_t,int pass_number,pyramid* curr_pyramid_ptr)
{
//   cout << "inside Pyramid::store_apex_and_zplane_vertices()" << endl;
   threevector apex(curr_pyramid_ptr->get_apex().get_posn());
//   cout << "apex = " << apex << endl;

   vector<threevector> V;
   V.push_back(apex);
   face* zplane_face_ptr=curr_pyramid_ptr->get_zplane_face_ptr();
   if (zplane_face_ptr != NULL) 
   {
      for (unsigned int v=0; v<zplane_face_ptr->get_n_vertices(); v++)
      {
         vertex curr_vertex=zplane_face_ptr->get_vertex_from_chain(v);
         V.push_back(curr_vertex.get_posn());
//         cout << "v = " << v << " zplane face vertex posn = "
//              << V.back() << endl;
      }
   } // zplane_face_ptr != NULL conditional
   
   set_vertices(curr_t,pass_number,V);
}

// ==========================================================================
// Drawing member functions
// ==========================================================================

// Member function set_color sets the colors of the current pyramid
// edges and volume.  Internal edges given the same color as the
// volume in order to hide them.

void Pyramid::set_color(
   pyramid* curr_pyramid_ptr,const osg::Vec4& side_edge_color,
   const osg::Vec4& base_edge_color)
{
   set_color(curr_pyramid_ptr,side_edge_color,side_edge_color,
             base_edge_color,osg::Vec4(0,0,0,0));
}

void Pyramid::set_color(
   pyramid* curr_pyramid_ptr,
   const osg::Vec4& side_edge_color,
   const osg::Vec4& zplane_edge_color,
   const osg::Vec4& base_edge_color,
   const osg::Vec4& volume_color)
{
//   cout << "inside Pyramid::set_color()" << endl;
//   cout << "*curr_pyramid_ptr = " << *curr_pyramid_ptr << endl;

   if (!shape_refptr.valid())
   {
      cout << "Error in Pyramid::set_color()" << endl;
      cout << "shape_refptr.valid()==0 !!!" << endl;
      outputfunc::enter_continue_char();
      return;
   }
   
//   cout << "volume color = " << endl;
//   osgfunc::print_Vec4(volume_color);

   shape_refptr->setColor(volume_color); // colors pyramid's volume

// Next color pyramid edges.  Any edge which contains the apex
// represents a "side" edge.  Any edge belonging to the z-plane face
// represents a "z-plane" edge.  All others represent "base" edges.

   int external_edge_counter=0;
   for (unsigned int e=0; e<curr_pyramid_ptr->get_n_edges(); e++)
   {
      edge* curr_edge_ptr=curr_pyramid_ptr->get_edge_ptr(e);
      if (curr_edge_ptr != NULL && !curr_edge_ptr->get_internal_edge_flag())
      {
         face* zplane_face_ptr=curr_pyramid_ptr->get_zplane_face_ptr();
         if (zplane_face_ptr != NULL && zplane_face_ptr->
             find_edge_in_chain_given_edge(*curr_edge_ptr) != NULL)
         {
            set_edge_color(external_edge_counter,zplane_edge_color);
         }
         else
         {
            threevector p1=curr_edge_ptr->get_V1().get_posn();
            threevector p2=curr_edge_ptr->get_V2().get_posn();
            if (p1.nearly_equal(curr_pyramid_ptr->get_apex().get_posn()) ||
                p2.nearly_equal(curr_pyramid_ptr->get_apex().get_posn()))
            {
               set_edge_color(external_edge_counter,side_edge_color);
            }
            else
            {
               set_edge_color(external_edge_counter,base_edge_color);
            }
         } // curr_edge_ptr != NULL && find_edge_in_chain_given_edge 
	   //   conditional
      } // !internal_edge_flag conditional
   } // loop over index e labeling pyramid edges
}

// ---------------------------------------------------------------------
void Pyramid::set_edge_color(
   int& external_edge_counter,const osg::Vec4& external_edge_color)
{
//   cout << "inside Pyramid::set_edge_color()" << endl;

   LineSegment* curr_segment_ptr=get_or_create_LineSegment_ptr(
      external_edge_counter);

   curr_segment_ptr->set_permanent_color(external_edge_color);      
   curr_segment_ptr->set_curr_color(external_edge_color);      
   LineSegmentsGroup_ptr->reset_colors();
      
   external_edge_counter++;
}

// ---------------------------------------------------------------------
void Pyramid::set_edge_widths(
   pyramid* pyramid_ptr,double side_edge_width,
   double base_edge_width,double zplane_edge_width)
{
//   cout << "inside Pyramid::set_edge_widths()" << endl;
//   cout << "side_edge_width = " << side_edge_width << endl;
//   cout << "base_edge_width = " << base_edge_width << endl;
//   cout << "zplane_edge_width = " << zplane_edge_width << endl;

   int external_edge_counter=0;
   for (unsigned int e=0; e<pyramid_ptr->get_n_edges(); e++)
   {
      if (!(pyramid_ptr->get_internal_edge_flag(e)))
      {
         LineSegment* curr_segment_ptr=get_or_create_LineSegment_ptr(
            external_edge_counter++);

         curr_segment_ptr->get_LineWidth_ptr()->setWidth(side_edge_width);
         edge* edge_ptr=pyramid_ptr->get_edge_ptr(e);
         face* base_ptr=pyramid_ptr->get_base_ptr();
         face* zplane_face_ptr=pyramid_ptr->get_zplane_face_ptr();
         if (base_ptr != NULL && base_ptr->find_edge_in_chain_given_edge(
            *edge_ptr) != NULL)
         {
            curr_segment_ptr->get_LineWidth_ptr()->setWidth(base_edge_width);
         }
         else if (zplane_face_ptr != NULL && zplane_face_ptr->
                  find_edge_in_chain_given_edge(*edge_ptr) != NULL)
         {
            curr_segment_ptr->get_LineWidth_ptr()->setWidth(
               zplane_edge_width);
         }
      } // !internal_edge_flag conditional
   } // loop over index e labeling pyramid edges
} 

// ---------------------------------------------------------------------
void Pyramid::set_edge_masks(
   double t,int pass_number,pyramid* curr_pyramid_ptr,bool side_edge_mask,
   bool zplane_edge_mask,bool base_edge_mask)
{
//   cout << "inside Pyramid::set_edge_masks()" << endl;

// Any edge which contains the apex represents a "side" edge.  Any
// edge belonging to the z-plane face represents a "z-plane" edge.
// All others represent "base" edges.

   int external_edge_counter=0;
   for (unsigned int e=0; e<curr_pyramid_ptr->get_n_edges(); e++)
   {
      edge* curr_edge_ptr=curr_pyramid_ptr->get_edge_ptr(e);
      if (curr_edge_ptr != NULL && !curr_edge_ptr->get_internal_edge_flag())
      {
         face* zplane_face_ptr=curr_pyramid_ptr->get_zplane_face_ptr();
         if (zplane_face_ptr != NULL && zplane_face_ptr->
             find_edge_in_chain_given_edge(*curr_edge_ptr) != NULL)
         {
            set_edge_mask(t,pass_number,
                          external_edge_counter,zplane_edge_mask);
         }
         else
         {
            threevector p1=curr_edge_ptr->get_V1().get_posn();
            threevector p2=curr_edge_ptr->get_V2().get_posn();
            if (p1.nearly_equal(curr_pyramid_ptr->get_apex().get_posn()) ||
                p2.nearly_equal(curr_pyramid_ptr->get_apex().get_posn()))
            {
               set_edge_mask(t,pass_number,
                             external_edge_counter,side_edge_mask);
            }
            else
            {
               set_edge_mask(t,pass_number,
                             external_edge_counter,base_edge_mask);
            }
         } // curr_edge_ptr != NULL && find_edge_in_chain_given_edge 
	   //   conditional
      } // !internal_edge_flag conditional
   } // loop over index e labeling pyramid edges
}

// ---------------------------------------------------------------------
void Pyramid::set_edge_mask(
   double t,int pass_number,int& external_edge_counter,bool edge_mask)
{
//   cout << "inside Pyramid::set_edge_mask(), external_edge_counter = "
//        << external_edge_counter << endl;

   LineSegment* curr_segment_ptr=get_or_create_LineSegment_ptr(
      external_edge_counter);
   curr_segment_ptr->set_mask(t,pass_number,edge_mask);
   external_edge_counter++;
}

// ==========================================================================
// Animation methods
// ==========================================================================

// Member function update_square_pyramid_triangle_mask is meant to be
// called from within update_display() methods.  For reasons we don't
// understand as of 12/17/2007, the relabeling of vertex, edge and
// face indices for OBSFRUSTA pyramids appears to be seriously messed
// up. So we hardwire here a correct set of triangle mesh indices for
// a square pyramid.  This method should disappear once vertex
// relabeling is fixed...

// As of 7/20/08, this next ugly method is unfortunately still needed
// in order for the animated Constant Hawk Observation Frustum to be
// correctly colored within the Bluegrass demo...

void Pyramid::update_square_pyramid_triangle_mesh(
   const vector<threevector>& rel_vertices)
{
//   cout << "inside Pyramid::update_square_pyramid_triangle_mesh(rel_corner)" 
//        << endl;

   for (unsigned int v=0; v<rel_vertices.size(); v++)
   {
      threevector rel_vertex=rel_vertices[v];
//      cout << "v = " << v << " rel_vertex = " << rel_vertex << endl;
      mesh_refptr->getVertices()->at(v).set(
         rel_vertex.get(0),rel_vertex.get(1),rel_vertex.get(2));
   }

   osg::UIntArray* indices = static_cast<osg::UIntArray*>(
      mesh_refptr->getIndices());

   (*indices)[0]=0;
   (*indices)[1]=1;
   (*indices)[2]=2;

   (*indices)[3]=0;
   (*indices)[4]=2;
   (*indices)[5]=3;

   (*indices)[6]=0;
   (*indices)[7]=3;
   (*indices)[8]=4;

   (*indices)[9]=0;
   (*indices)[10]=4;
   (*indices)[11]=1;

   (*indices)[12]=2;
   (*indices)[13]=1;
   (*indices)[14]=4;

   (*indices)[15]=3;
   (*indices)[16]=2;
   (*indices)[17]=4;

   dirtyDisplay();
}
