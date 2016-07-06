// =========================================================================
// Triangle class member function definitions 
// =========================================================================
// Last modified on 1/26/12; 2/29/12; 10/18/13
// =========================================================================

#include "geometry/bounding_box.h"
#include "geometry/geometry_funcs.h"
#include "geometry/polygon.h"
#include "geometry/triangle.h"

using std::cout;
using std::endl;
using std::ostream;
using std::ofstream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void triangle::allocate_member_objects()
{
}

void triangle::initialize_member_objects()
{
   area=-1;
   triangle_poly_ptr=NULL;
}		 

// ---------------------------------------------------------------------
triangle::triangle(const vector<vertex>& Vertices,int triangle_ID)
{
   allocate_member_objects();
   initialize_member_objects();

   this->ID=triangle_ID;
   this->Vertices.clear();
   for (int v=0; v<3; v++)
   {
      this->Vertices.push_back(Vertices[v]);
      int next_v=modulo(v+1,3);
      Edges.push_back(edge(Vertices[v],Vertices[next_v]));
   }
}

// ---------------------------------------------------------------------
// Copy constructor:

triangle::triangle(const triangle& t)
{
   docopy(t);
}

triangle::~triangle()
{
//   cout << "inside triangle destructor" << endl;
   delete triangle_poly_ptr;
}

// ---------------------------------------------------------------------
void triangle::docopy(const triangle& t)
{
   ID=t.get_ID();

   Vertices.clear();
   for (int v=0; v<3; v++)
   {
      Vertices.push_back(t.Vertices[v]);
   }
   area=compute_area();
}

// Overload = operator:

triangle& triangle::operator= (const triangle& t)
{
   if (this==&t) return *this;
   docopy(t);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,triangle& t)
{
   outstream << endl;
   for (unsigned int v=0; v<t.Vertices.size(); v++)
   {
      vertex curr_vertex(t.get_vertex(v));
      threevector posn=curr_vertex.get_posn();
      cout << "Vertex ID = " << curr_vertex.get_ID()
           << " posn = " 
           << posn.get(0) << " , "
           << posn.get(1) << " , "
           << posn.get(2) << endl;
   }
   for (unsigned int e=0; e<t.Edges.size(); e++)
   {
      edge curr_edge(t.get_edge(e));
      cout << "e = " << e
           << " edge length = " << curr_edge.get_length()
           << endl;
   }
   
   outstream << "area = " << t.get_area() << endl;

   return outstream;
}

double triangle::compute_area() 
{
   area=geometry_func::compute_triangle_area(
      Vertices[0].get_posn(),Vertices[1].get_posn(),
      Vertices[2].get_posn());
   return area;
}

// =========================================================================
// Triangle properties member functions
// =========================================================================


bool triangle::vertices_inside_bbox(const bounding_box& bbox)
{
   bool all_vertices_inside_bbox=true;
   for (int v=0; v<3; v++)
   {
      threevector curr_posn=get_vertex(v).get_posn();
//      cout << "v = " << v << " X = " << curr_posn.get(0)
//           << " Y = " << curr_posn.get(1) << endl;
      if (!bbox.point_inside(curr_posn.get(0),curr_posn.get(1)))
      {
         all_vertices_inside_bbox=false;
         break;
      }
   } // loop over index v labeling triangle vertices
//   cout << "all_vertices_inside_bbox = " << all_vertices_inside_bbox << endl;

   return all_vertices_inside_bbox;
}

// =========================================================================
// Triangle manipulation member functions
// =========================================================================

// Member function reset_all_Z_values() reset the triangle's 3
// vertices to have the input Z value.

void triangle::reset_all_Z_values(double z)
{
   for (int v=0; v<3; v++)
   {
      threevector curr_posn=get_vertex(v).get_posn();
      curr_posn.put(2,z);
      get_vertex(v).set_posn(curr_posn);
   }
}

// ---------------------------------------------------------------------
// Member function generate_polygon() dynamically instantiates a
// polygon object based upon the triangle's 3 vertices:

polygon* triangle::generate_polygon()
{
   vector<threevector> triangle_vertices;
   for (int v=0; v<3; v++)
   {
      triangle_vertices.push_back(get_vertex(v).get_posn());
   }

   triangle_poly_ptr=new polygon(triangle_vertices);
   return triangle_poly_ptr;
}

// ---------------------------------------------------------------------
void triangle::delete_triangle_poly_ptr()
{
   delete triangle_poly_ptr;
   triangle_poly_ptr=NULL;
}
