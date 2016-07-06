// =========================================================================
// Edge class member function definitions
// =========================================================================
// Last modified on 1/8/12; 1/27/12; 1/29/12
// =========================================================================

#include <iostream>
#include "geometry/edge.h"
#include "templates/mytemplates.h"
#include "math/rotation.h"

using std::cout;
using std::endl;
using std::ostream;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void edge::initialize_member_objects()
{
   ID=-1;
   internal_edge_flag=false;
   vertices_handler_ptr=NULL;
}		 

void edge::allocate_member_objects()
{
   bool edge_flag=true;
   vertices_handler_ptr=new vertices_handler(edge_flag);
}

// ---------------------------------------------------------------------
edge::edge(int id)
{
//   cout << "inside edge(int id) constructor, this(edge) = " << this << endl;
   initialize_member_objects();
   allocate_member_objects();
   ID=id;
}

edge::edge(const vertex& V1,const vertex& V2,int id)
{
//   cout << "inside edge(V1,V2) constructor, this(edge) = " << this << endl;
   initialize_member_objects();
   allocate_member_objects();

   if (V1.get_ID()==V2.get_ID())
   {
      cout << "Error in edge constructor!" << endl;
      cout << "V1 = " << V1 << " V2 = " << V2 <<endl;
      exit(-1);
   }

   vertices_handler_ptr->clear_all_containers();
//   vertices_handler_ptr->clear_vertex_chain();
//   vertices_handler_ptr->purge_vertex_table();
//   vertices_handler_ptr->clear_vertex_map();
   vertices_handler_ptr->update_vertex_in_table_chain_and_map(V1);
   vertices_handler_ptr->update_vertex_in_table_chain_and_map(V2);

   ID=id;
}

// ---------------------------------------------------------------------
// Copy constructor:

edge::edge(const edge& e)
{
//   cout << "inside edge copy constructor, this(edge) = " << this << endl;
   initialize_member_objects();
   docopy(e);
}

edge::~edge()
{
   delete vertices_handler_ptr;
   vertices_handler_ptr=NULL;
}

// ---------------------------------------------------------------------
void edge::docopy(const edge& e)
{
//   cout << "inside edge::docopy()" << endl;
//   cout << "this = " << this << endl;
   ID=e.ID;
   internal_edge_flag=e.internal_edge_flag;

   delete vertices_handler_ptr;
   vertices_handler_ptr=new vertices_handler(*e.vertices_handler_ptr);
}

// Overload = operator:

edge& edge::operator= (const edge& e)
{
//   cout << "inside edge::operator=" << endl;
//   cout << "this(edge) = " << this << endl;
   if (this==&e) return *this;
   docopy(e);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const edge& e)
{
   outstream << endl;
   outstream << "Edge ID = " << e.ID << endl;
   outstream << "internal_edge_flag = " << e.internal_edge_flag << endl;
   outstream << "V1: " << e.get_V1();
   outstream << "V2: " << e.get_V2();
   
   return(outstream);
}

// =========================================================================
// Set and get member functions
// =========================================================================

// Member function get_frac_posn() returns the threevector
// corresponding to the input fraction between the start and stop edge
// points.

threevector edge::get_frac_posn(double frac) const
{
   threevector start_posn=get_V1().get_posn();
   threevector stop_posn=get_V2().get_posn();
   threevector frac_posn=start_posn+frac*(stop_posn-start_posn);
   return frac_posn;
}

// =========================================================================
// Vertex map member functions
// =========================================================================

void edge::update_vertex_map()
{
   vertices_handler_ptr->clear_vertex_map();
   vertices_handler_ptr->update_vertex_map(get_V1());
   vertices_handler_ptr->update_vertex_map(get_V2());
}

// ---------------------------------------------------------------------
void edge::reset_vertex_posn(
   const threevector& old_posn,const threevector& new_posn)
{
   vertices_handler_ptr->reset_vertex_posn(old_posn,new_posn);
   update_vertex_map();
}

// =========================================================================
// Above Z-plane member functions
// =========================================================================

// Member function swap_vertices

void edge::swap_vertices()
{
//   cout << "inside edge::swap_vertices()" << endl;
   vertex v1=get_V1();
   vertex v2=get_V2();

   set_V1(v2);
   set_V2(v1);
//   templatefunc::swap(V1,V2);
}

// ---------------------------------------------------------------------
// Member function nearly_equal_posn compares the locations of the
// current edge's vertices with those of input edge e.  If they are
// nearly equal, this boolean method returns true.

bool edge::nearly_equal_posn(const edge& e)
{
//   cout << "inside edge::nearly_equal_posn()" << endl;
   threevector p1(e.get_V1().get_posn());
   threevector p2(e.get_V2().get_posn());

   return( 
      (get_V1().get_posn().nearly_equal(p1) && 
       get_V2().get_posn().nearly_equal(p2)) ||
      (get_V1().get_posn().nearly_equal(p2) && 
       get_V2().get_posn().nearly_equal(p1)) );
}

// =========================================================================
// Above Z-plane member functions
// =========================================================================

// Member function intersection_with_Zplane returns false if the
// current edge object lies completely above or below z=Z.  Otherwise,
// it computes the edge's intersection point with the z-plane and
// returns true.

bool edge::intersection_with_Zplane(double z,threevector& intersection_posn)
{
   if (above_Zplane(z) || below_Zplane(z))
   {
      return false;
   }
   else
   {
      double alpha=(z-get_V1().get_posn().get(2))/
         (get_V2().get_posn().get(2)-get_V1().get_posn().get(2));
      intersection_posn=get_V1().get_posn()+alpha*(
         get_V2().get_posn()-get_V1().get_posn());
      return true;
   }
}

// =========================================================================
// Moving around edge member functions
// =========================================================================

void edge::translate(const threevector& rvec)
{
   get_V1().translate(rvec);
   get_V2().translate(rvec);
   update_vertex_map();
}

void edge::absolute_position(const threevector& rvec)
{
   threevector delta=rvec-get_V1().get_posn();
   translate(delta);

//   threevector relative_v=get_V2().get_posn()-get_V1().get_posn();
//   get_V1().absolute_position(rvec);
//   get_V2().set_posn(get_V1().get_posn()+relative_v);
}

void edge::scale(
   const threevector& scale_origin,const threevector& scalefactor)
{
   get_V1().scale(scale_origin,scalefactor);
   get_V2().scale(scale_origin,scalefactor);
   update_vertex_map();
}

void edge::rotate(const rotation& R)
{
   get_V1().rotate(R);
   get_V2().rotate(R);
   update_vertex_map();
}

void edge::rotate(const threevector& rotation_origin,const rotation& R)
{
   get_V1().rotate(rotation_origin,R);
   get_V2().rotate(rotation_origin,R);
   update_vertex_map();
}

void edge::rotate(const threevector& rotation_origin,
                  double thetax,double thetay,double thetaz)
{
   get_V1().rotate(rotation_origin,thetax,thetay,thetaz);
   get_V2().rotate(rotation_origin,thetax,thetay,thetaz);
   update_vertex_map();
}
