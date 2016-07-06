// ==========================================================================
// Deformable contour class member function definitions
// ==========================================================================
// Last modified on 1/29/12; 10/22/13; 3/6/14; 4/4/14
// ==========================================================================

#include <iostream>
#include <string>
#include "math/constants.h"
#include "geometry/contour.h"
#include "geometry/face.h"
#include "filter/filterfuncs.h"
#include "geometry/geometry_funcs.h"
#include "datastructures/Linkedlist.h"
#include "math/mathfuncs.h"
#include "templates/mytemplates.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "math/rotation.h"
#include "geometry/triangulate_funcs.h"
#include "geometry/triangulater.h"
#include "geometry/vertex.h"
#include "geometry/voronoifuncs.h"

using std::cout;
using std::endl;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void contour::allocate_member_objects()
{
   vertex_list_ptr=new Linkedlist<pair<threevector,bool> >;
}		       

void contour::initialize_member_objects()
{
   nvertices=0;
   perimeter=area=0;
   origin=Zero_vector;
   normal=z_hat;
   triangles_group_ptr=NULL;

   edge.clear();
}

contour::contour(void)
{
   allocate_member_objects();
   initialize_member_objects();
}

contour::contour(int num_of_vertices)
{
   allocate_member_objects();
   initialize_member_objects();
   nvertices=num_of_vertices;
   initialize_edge_segments(false);
}

contour::contour(int num_of_vertices,const threevector currvertex[])
{
   allocate_member_objects();
   initialize_member_objects();
   nvertices=num_of_vertices;
   pair<threevector,bool> p;
   for (unsigned int i=0; i<nvertices; i++)
   {
      p.first=currvertex[i];
      p.second=true;	// Assume vertex is convex by default
      vertex_list_ptr->append_node(p);
   }
   finish_construction();
}

// This next contour constructor takes in STL vector vertices
// containing vertices which may or may not all lie in a plane.  If
// they don't, we assume that the XY components correspond to the
// contour's footprint.  We generate the footprint contour in the XY
// plane and then set its heights equal to the z values within
// vertices.

contour::contour(
   const vector<threevector>& vertices,bool vertices_lie_in_plane)
{
   allocate_member_objects();
   initialize_member_objects();
   nvertices=vertices.size();
   pair<threevector,bool> p;

   for (unsigned int i=0; i<nvertices; i++)
   {
      p.first=vertices[i];
      if (!vertices_lie_in_plane)
      {
         p.first.put(2,0);
      }
      p.second=true;	// Assume vertex is convex by default
      vertex_list_ptr->append_node(p);
   }

   finish_construction();

   if (!vertices_lie_in_plane)
   {
      int i=0;
      for (Mynode<pair<threevector,bool> >* currnode_ptr=vertex_list_ptr->
              get_start_ptr(); currnode_ptr != NULL; 
           currnode_ptr=currnode_ptr->get_nextptr())
      {
         p=currnode_ptr->get_data();
         p.first.put(2,vertices[i++].get(2));
         currnode_ptr->set_data(p);
      }
   } // vertices do NOT lie in plane conditional
}

contour::contour(Linkedlist<threevector> const *perim_posn_list_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   nvertices=perim_posn_list_ptr->size();
   pair<threevector,bool> p;
   for (Mynode<threevector> const *currnode_ptr=perim_posn_list_ptr->
           get_start_ptr(); currnode_ptr != NULL; 
        currnode_ptr=currnode_ptr->get_nextptr())
   {
      p.first=currnode_ptr->get_data();
      p.second=true;	// Assume vertex is convex by default
      vertex_list_ptr->append_node(p);
   }
   finish_construction();
}

contour::contour(Linkedlist<pair<threevector,bool> > const *perim_list_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   nvertices=perim_list_ptr->size();
   pair<threevector,bool> p;
   for (Mynode<pair<threevector,bool> > const *currnode_ptr=perim_list_ptr->
           get_start_ptr(); currnode_ptr != NULL; 
        currnode_ptr=currnode_ptr->get_nextptr())
   {
      p.first=currnode_ptr->get_data().first;
      p.second=true;	// Assume vertex is convex by default
      vertex_list_ptr->append_node(p);
   }
   finish_construction();
}

contour::contour(polygon const *poly_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   nvertices=poly_ptr->get_nvertices();
   pair<threevector,bool> p;
   for (unsigned int i=0; i<nvertices; i++)
   {
      p.first=poly_ptr->get_vertex(i);
      p.second=true;	// Assume vertex is convex by default
      vertex_list_ptr->append_node(p);
   }
   finish_construction();
}

// Next constructor builds contour from input polyline which is
// assumed to NOT close upon itself

contour::contour(const polyline& curr_polyline)
{
   allocate_member_objects();
   initialize_member_objects();
   nvertices=curr_polyline.get_n_vertices();

   pair<threevector,bool> p;
   for (unsigned int i=0; i<nvertices; i++)
   {
      p.first=curr_polyline.get_vertex(i);
      p.second=true;	// Assume vertex is convex by default
      vertex_list_ptr->append_node(p);
   }
   finish_construction();
}

contour::contour(const face& f)
{
   allocate_member_objects();
   initialize_member_objects();
   nvertices=f.get_n_vertices();
   pair<threevector,bool> p;
   for (unsigned int i=0; i<nvertices; i++)
   {
      p.first=f.get_vertex_from_chain(i).get_posn();
      p.second=true;	// Assume vertex is convex by default
      vertex_list_ptr->append_node(p);
   }
   finish_construction();
}

void contour::finish_construction()
{
//   cout << "inside contour::finish_construction()" << endl;
   guarantee_right_handed_vertex_ordering();
   initialize_edge_segments();
   identify_concave_vertices();
   robust_locate_origin();
}

// ---------------------------------------------------------------------
// Method guarantee_right_handed_vertex_ordering checks the sums of
// the input vertices' exterior angles.  If they sum to -2*PI, this
// method rearranges their cyclic order.  The original left-handed
// linked list vertices is destroyed and replaced with its
// right-handed counterpart.

void contour::guarantee_right_handed_vertex_ordering()
{
   if (!right_handed_vertex_ordering())
   {
//      cout << "initial ordering is LH!" << endl;
      vector<threevector> vertex(nvertices);
      for (unsigned int i=0; i<nvertices; i++)
      {
         vertex[i]=get_vertex(i).first;
      }
      delete vertex_list_ptr;

      vertex_list_ptr=new Linkedlist<pair<threevector,bool> >;      
      pair<threevector,bool> p;
      for (unsigned int i=0; i<nvertices; i++)
      {
         p.first=vertex[modulo(nvertices-i,nvertices)];
         p.second=true;	// Assume vertex is convex by default
         vertex_list_ptr->append_node(p);
      }
//      cout << "Final RH ordering = "
//           << right_handed_vertex_ordering() << endl;
   }
}

// Copy constructor:

contour::contour(const contour& p)
{
   allocate_member_objects();
   initialize_member_objects();
   docopy(p);
   initialize_edge_segments();
   identify_concave_vertices();
}

contour::~contour()
{
   delete vertex_list_ptr;
   delete triangles_group_ptr;
}

// ---------------------------------------------------------------------
void contour::docopy(const contour& c)
{
//   cout << "inside contour::docopy()" << endl;
   nvertices=c.nvertices;
   perimeter=c.perimeter;
   area=c.area;
   origin=c.origin;
   normal=c.normal;
   vertex_avg=c.vertex_avg;

//  Explicitly copy contents of *(c.vertex_list_ptr) onto
//  *vertex_list_ptr:

   vertex_list_ptr->purge_all_nodes();

   for (Mynode < std::pair < threevector,bool > >* currnode_ptr=
           c.vertex_list_ptr->get_start_ptr(); currnode_ptr != NULL;
        currnode_ptr=currnode_ptr->get_nextptr())
   {
      pair<threevector,bool> p=currnode_ptr->get_data();
      vertex_list_ptr->append_node(p);
   }

   edge.clear();
   for (unsigned int e=0; e<c.edge.size(); e++)
   {
      edge.push_back(c.edge[e]);
   }
}

// Overload = operator:

contour& contour::operator= (const contour& c)
{
   if (this==&c) return *this;
   docopy(c);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,contour& c)
{
   outstream << endl;
   outstream << "# contour vertices = " << c.nvertices << endl;
   for (unsigned int i=0; i<c.nvertices; i++)
   {
//      outstream << "i = " << i << endl;
//      outstream << " vertex = " << c.get_vertex(i).first
//                << endl;
//      outstream << "Edge = " << c.get_edge(i) << endl;
//      outstream << "Edge.get_ehat() = " << c.get_edge(i).get_ehat() << endl;
//      outstream << "edge.get_length() = " 
// 		  << c.get_edge(i).get_length() << endl;
      threevector vertex(c.get_vertex(i).first);

//      vertex=vertex-c.vertex_average();
      
//      outstream << vertex.get(0) << " , " << vertex.get(1) << endl;
      outstream << "vertex[" << i << "]=threevector(" << vertex.get(0) << ","
                << vertex.get(1) << "," << vertex.get(2) << ");" << endl;
   }

   for (unsigned int i=0; i<c.nvertices; i++)
   {
      if (!c.get_vertex(i).second)
      {
         outstream << "Vertex " << i << " is CONCAVE" << endl;
      }
   }

   if (c.vertex_list_ptr != NULL)
   {
//      outstream << "Contour vertex list = " << *(c.vertex_list_ptr) << endl;
   }
   return(outstream);
}

// ---------------------------------------------------------------------
std::ostream& contour::write_to_textstream(std::ostream& textstream)
{
   textstream << nvertices << std::endl;
   for (unsigned int n=0; n<nvertices; n++)
   {
      threevector curr_vertex(get_vertex(n).first);
      textstream << curr_vertex.get(0) << " "
                 << curr_vertex.get(1) << " " 
                 << curr_vertex.get(2) << endl;
   }
   return textstream;
}

// ---------------------------------------------------------------------
void contour::read_from_text_lines(
   unsigned int& i,vector<string>& line,bool vertices_lie_in_plane)
{
   unsigned int n_vertices;
   std::istringstream input_string_stream(line[i++]);
   input_string_stream >> n_vertices;

   vector<threevector> Vertices;
   for (unsigned int n=0; n<n_vertices; n++)
   {
      vector<double> V=stringfunc::string_to_numbers(line[i++]);
      threevector curr_vertex(V[0],V[1],V[2]);
      Vertices.push_back(curr_vertex);
   }
   *this=contour(Vertices,vertices_lie_in_plane);
}

// =====================================================================
// Intrisic contour properties methods:
// =====================================================================

bool contour::right_handed_vertex_ordering() const
{
   vector<threevector> vertex;
   for (unsigned int i=0; i<nvertices; i++)
   {
      vertex.push_back(get_vertex(i).first);
   }
   return geometry_func::right_handed_vertex_ordering(vertex,normal);
}

// ---------------------------------------------------------------------
// Member function point_inside determines whether a twovector assumed
// to lie within the same plane as the current contour object is
// actually contained inside the contour.  This method adds up the
// angles made between consecutive rays joining the point and the
// contour's vertices.  The sign of each angular contribution to the
// sum is given by the right hand rule relative to the contour's
// current normal vector.  If the sum (approximately) equals 2*PI, the
// point lies within the contour.  If the sum (approximately) equals
// -2*PI, the point lies within the contour, and the normal vector's
// sign needs to be changed so that it is consistent with right hand
// rule ordering of the contour vertices.  Finally, the angle sum
// (approximately) equals 0 if the point lies outside the contour.

bool contour::point_inside(const threevector& point,bool print_flag) const
{
   bool point_inside=true;
   vector<threevector> uhat(nvertices);
   for (unsigned int i=0; i<nvertices; i++)
   {
      threevector curru(get_vertex(i).first-point);
      if (curru.magnitude()==0)
      {
         point_inside=false; 		// point lies on contour vertex
         cout << "point lies on contour vertex" << endl;
         return point_inside;
      }
      else
      {
         uhat[i]=curru.unitvector();
      }
   }

   double thetasum=0;
   for (unsigned int i=0; i<nvertices; i++)
   {
/*
      double thetamag=fabs(acos(uhat[i].dot(uhat[modulo(i+1,nvertices)])));
      double cos_theta=(uhat[i].dot(uhat[modulo(i+1,nvertices)]));
      threevector crossprod(uhat[i].cross(uhat[modulo(i+1,nvertices)]));
      double mag_sin_theta=crossprod.magnitude();
      double theta_sgn=sgn(crossprod.dot(normal));
      double sin_theta=theta_sgn*mag_sin_theta;
      double delta_theta=theta_sgn*thetamag;
      double delta_theta_new=atan2(sin_theta,cos_theta);

      if (!nearly_equal(delta_theta,delta_theta_new))
      {
         cout << "Trouble in contour::point_inside()" << endl;
         cout << "delta_theta = " << delta_theta*180/PI << endl;
         cout << "delta_theta_new = " << delta_theta_new*180/PI << endl;
      }
*/

      double delta_theta_new=mathfunc::angle_between_unitvectors(
         uhat[i],uhat[modulo(i+1,nvertices)]);
      thetasum += delta_theta_new;

      if (print_flag) 
      {
         cout << "i = " << i 
              << " delta_theta_new = " << delta_theta_new*180/PI
              << " thetasum/2*PI = " << thetasum/(2*PI) << endl;
      }
   } // loop over index i labeling contour vertices

// Note: On 6/23/04, we empirically found that thetasum = 0.9977 for a
// point which actually lay inside one particular contour.  But since
// this method should only be used to find a trial origin point, we
// continue to require that | thetasum | > 0.9999*2*PI for interior
// points...

   if (print_flag)
   {
      cout << "thetasum/2*PI = " << thetasum/(2*PI) << endl;
   }

   if (thetasum >= 0.99*2*PI)
   {
      point_inside=true;
   }
   else if (thetasum <= -0.001)
   {
      cout << "Trouble in contour::point_inside() !!!" << endl;
      cout << "thetasum/(2*PI) = " << thetasum/(2*PI) << endl;
      point_inside=true;
   }
   else if (thetasum > 0.01 && thetasum < 0.99*2*PI)
   {
      cout << "Trouble in contour::point_inside() !!!" << endl;
      cout << "thetasum/(2*PI) = " << thetasum/(2*PI) << endl;
      point_inside=false;
   }
   else
   {
      point_inside=false;
   }

   return point_inside;
}

// ---------------------------------------------------------------------
// Member function point_within_polygon() implements W. Randolph
// Franklin's PNPOLY (Point Inclusion in Polygon) test.  See
// http://www.ecse.rpi.edu/Homepages/wrf/Research/Short_Notes/pnpoly.html

// As of 10/22/11, very preliminary testing of this (fast!) method
// looks encouraging...

bool contour::point_within_contour(const threevector& point)
{
   return point_within_contour(twovector(point));
}

bool contour::point_within_contour(const twovector& point)
{
   double x_point=point.get(0);
   double y_point=point.get(1);

   bool point_inside=false;
   for (unsigned int i=0, j=nvertices-1; i<nvertices; j=i++)
   {
      double x_i = get_vertex(i).first.get(0);
      double y_i = get_vertex(i).first.get(1);
      double x_j = get_vertex(j).first.get(0);
      double y_j = get_vertex(j).first.get(1);

      if ( ( (y_i > y_point) != (y_j > y_point) ) &&
      (x_point < (x_j-x_i) * (y_point-y_i)/(y_j-y_i)+x_i) )
      {
         point_inside=!point_inside;
      }
   }

   return point_inside;
}

// ---------------------------------------------------------------------
double contour::min_edge_length() const
{
   double min_edge_length=POSITIVEINFINITY;
   for (unsigned int i=0; i<nvertices; i++)
   {
      min_edge_length=basic_math::min(min_edge_length,edge[i].get_length());
   }
   return min_edge_length;
}

// ---------------------------------------------------------------------
// Member function vertex_average returns the average of all the
// contour's vertices:

threevector& contour::vertex_average()
{
   vertex_avg=Zero_vector;
   for (unsigned int i=0; i<nvertices; i++)
   {
      vertex_avg += get_vertex(i).first;
   }
   vertex_avg /= double(nvertices);
   return vertex_avg;
}

// ---------------------------------------------------------------------
// Member function locate_extremal_xy_points locates the
// minimum/maximum x and y points on the current contour.

void contour::locate_extremal_xy_points(
   double& min_x,double& min_y,double& max_x,double& max_y) const
{
   min_x=min_y=POSITIVEINFINITY;
   max_x=max_y=NEGATIVEINFINITY;
   for (unsigned int i=0; i<nvertices; i++)
   {
      min_x=basic_math::min(get_vertex(i).first.get(0),min_x);
      min_y=basic_math::min(get_vertex(i).first.get(1),min_y);
      max_x=basic_math::max(get_vertex(i).first.get(0),max_x);
      max_y=basic_math::max(get_vertex(i).first.get(1),max_y);
   }
}

// ---------------------------------------------------------------------
// Method identify_concave_vertices loops over all vertices within
// linkedlist *vertex_list_ptr.  It computes the overlap of
// ehat[n-1]xehat[n] with z_hat for the nth vertex.  If the overlap is
// negative, the nth vertex is concave and the 2nd boolean member of
// each node in *vertex_list_ptr is set equal to false.  Otherwise,
// the nth vertex is concave and the 2nd boolean member is set equal
// to true.

void contour::identify_concave_vertices()
{
   for (unsigned int i=0; i<nvertices; i++)
   {
      threevector curr_ehat(edge[i].get_ehat());
      threevector prev_ehat(edge[modulo(i-1,nvertices)].get_ehat());
      double dotproduct=z_hat.dot(prev_ehat.cross(curr_ehat));

      if (dotproduct < 0)
      {
         set_vertex(i,edge[i].get_v1(),false);
      }
      else
      {
         set_vertex(i,edge[i].get_v1(),true);
      }
   } // loop over contour vertices
}

// ---------------------------------------------------------------------
// Boolean method is_convex returns false if the contour contains any
// concave vertex.

bool contour::is_convex() const
{
   bool contour_is_convex=true;
   for (unsigned int i=0; i<nvertices; i++)
   {
      if (!get_vertex(i).second) contour_is_convex=false;
   } // loop over contour vertices
   return contour_is_convex;
}

// ---------------------------------------------------------------------
// Member function compute_area() implements the very clever
// algorithm of D.R. Finley for finding areas of arbitrary 2D
// polygons.  This algorithm works for concave, convex and even
// self-intersecting polygons!  The polygon's vertices are assumed to be
// ordered according to the right hand rule.  If not, the area
// returned is negative in sign.  See
// http://alienryderflex.com/polygon_area/ .

double contour::compute_area() 
{
//   cout << "inside contour::compute_area()" << endl;

   area=0;
   for (unsigned int i=0; i<get_nvertices(); i++)
   {
      int j=modulo(i+1,get_nvertices());

      threevector curr_vertex=get_vertex(i).first;
      threevector next_vertex=get_vertex(j).first;
      area += 0.5*(curr_vertex.get(0)+next_vertex.get(0)) * 
         (next_vertex.get(1)-curr_vertex.get(1));
   }
//   cout << "area = " << area << endl;
   
   return area;
}

// =====================================================================
// Contour edge methods:
// =====================================================================

// For speed purposes, we do NOT include the following edge
// initialization within the overall initialize_member_objects()
// member function:

void contour::initialize_edge_segments(bool compute_edges_flag)
{
   if (edge.size()==0)
   {
      if (compute_edges_flag) compute_edges();
   }
}

// ---------------------------------------------------------------------
void contour::compute_edges()
{
//   cout << "inside contour::compute_edges()" << endl;
   edge.clear();
   for (unsigned int i=0; i<nvertices; i++)
   {
      threevector vertex(get_vertex(i).first);
      threevector next_vertex(get_vertex(modulo(i+1,nvertices)).first);
      edge.push_back(linesegment(vertex,next_vertex));
   }
   compute_perimeter();
}

// ---------------------------------------------------------------------
double contour::compute_perimeter()
{
   initialize_edge_segments();
   perimeter=0;
   for (unsigned int i=0; i<nvertices; i++)
   {
      perimeter += edge[i].get_length();
   }
   return perimeter;
}

// ---------------------------------------------------------------------
// Member function edge_number takes in fraction 0 <= frac <= 1 and
// returns the number of the edge on which the point whose length from
// vertex[0] equals frac*perimeter:

unsigned int contour::edge_number(double frac) const
{
   bool segment_found=false;
   unsigned int segment_number=0;
   double running_length=0;
   do
   {
      if (frac*perimeter >= running_length &&
          frac*perimeter < running_length + edge[segment_number].get_length())
      {
         segment_found=true;
      }
      else
      {
         running_length += edge[segment_number].get_length();
         segment_number++;
      }
   }
   while(!segment_found && segment_number < nvertices);
   return segment_number;
}

// ---------------------------------------------------------------------
// Member function edge_point takes in fraction 0 <= frac <= 1 and
// returns the corresponding point along the contour's perimeter whose
// length from vertex[0] equals frac*perimeter:

void contour::edge_point(double frac,threevector& curr_point) const
{
   unsigned int segment_number=edge_number(frac);
   double running_length=0;
   for (unsigned int i=0; i<segment_number; i++)
   {
      running_length += edge[i].get_length();
   }

// Next compute residual fractional length of desired point on its
// edge segment:

   double curr_edge_frac=(frac*perimeter-running_length)/
      edge[segment_number].get_length();
   curr_point=edge[segment_number].get_v1()+
      curr_edge_frac*edge[segment_number].get_length()*
      edge[segment_number].get_ehat();
}

threevector contour::edge_point(double frac) const
{
   unsigned int segment_number=edge_number(frac);
   double running_length=0;
   for (unsigned int i=0; i<segment_number; i++)
   {
      running_length += edge[i].get_length();
   }

// Next compute residual fractional length of desired point on its
// edge segment:

   double curr_edge_frac=(frac*perimeter-running_length)/
      edge[segment_number].get_length();
   threevector curr_point=edge[segment_number].get_v1()+
      curr_edge_frac*edge[segment_number].get_length()*
      edge[segment_number].get_ehat();
   return curr_point;
}

// ---------------------------------------------------------------------
// Member function find_edge_containing_point() loops over every edge
// within the contour and tests whether the input point lies along
// it.  If so, this method returns the edge index.  Otherwise, it
// returns -1.

int contour::find_edge_containing_point(const threevector& currpoint) const
{
   for (unsigned int l=0; l<edge.size(); l++)
   {
      if (edge[l].point_on_segment(currpoint))
      {
         return l;
      }
   } // loop over index l labeling polyline edges
   return -1;
}

// ---------------------------------------------------------------------
// Member function frac_distance_along_contour() returns -1 if the
// input currpoint does not lie along the contour.  Otherwise, it
// returns currpoint's fractional distance starting from the zeroth
// vertex.

double contour::frac_distance_along_contour(const threevector& currpoint) 
{
//   cout << "inside contour::frac_distance_along_contour()" << endl;

   int l_pnt=find_edge_containing_point(currpoint);
   if (l_pnt < 0) return -1;
   
   compute_perimeter();

   double distance_to_point=0;
   for (int l=0; l<l_pnt; l++)
   {
      distance_to_point += edge[l].get_length();
   }
   
   double f_final_edge=edge[l_pnt].frac_distance_along_segment(currpoint);
   distance_to_point += f_final_edge*edge[l_pnt].get_length();
   
   double frac=distance_to_point/get_perimeter();
//   cout << "frac = " << frac << endl;
   return frac;
}

// ---------------------------------------------------------------------
// Member function regularize_vertices uses the existing vertices on
// the current contour object to compute a new set of vertices which
// are evenly spaced apart by input length separation delta_s
// (measured in meters).  The original set of vertices contained
// within linked list vertex_list_ptr is destroyed, and the new
// regularized set takes its place.  The original set of edges
// contained within the edge STL vector is also destroyed and replaced
// by new edges for the regularized vertices.

void contour::regularize_vertices(double delta_s)
{
   unsigned int n_sides=basic_math::max(
      3,basic_math::round(perimeter/delta_s));
   delta_s=perimeter/n_sides;

   vector<threevector> new_vertex(n_sides);
   for (unsigned int n=0; n<n_sides; n++)
   {
      double frac=n*delta_s/perimeter;
      edge_point(frac,new_vertex[n]);
   }

   delete vertex_list_ptr;

   edge.clear();
   
   vertex_list_ptr=new Linkedlist<pair<threevector,bool> >;
   nvertices=n_sides;
   pair<threevector,bool> p;
   for (unsigned int i=0; i<nvertices; i++)
   {
      p.first=new_vertex[i];
      p.second=true;	// Assume vertex is convex by default
      vertex_list_ptr->append_node(p);
   }
//   initialize_edge_segments();
   compute_edges();
}

// ---------------------------------------------------------------------
// Member function average_edge_orientation() loops over all edges
// within *this.  It computes the angle between each edge's direction
// vector and +X_hat modulo 90 degrees.  This method returns the
// average of the canonical edge oriention angles.

double contour::average_edge_orientation() const
{
//   cout << "inside contour::average_edge_orientation()" << endl;
   
   double avg_canonical_theta=0;
   for (unsigned int i=0; i<get_nvertices(); i++)
   {
      linesegment curr_edge(get_edge(i));
      threevector curr_ehat=curr_edge.get_ehat();
//      double curr_theta=mathfunc::angle_between_unitvectors(curr_ehat,x_hat);
      double curr_theta=atan2(curr_ehat.get(1),curr_ehat.get(0));
      
      double canonical_theta=basic_math::phase_to_canonical_interval(
         curr_theta,0,PI/2);
      avg_canonical_theta += canonical_theta;
   } // loop over index i labeling contour edges
   avg_canonical_theta /= get_nvertices();
//   cout << "avg canonical theta = " << avg_canonical_theta*90/PI << endl;
   return avg_canonical_theta;
}

// ---------------------------------------------------------------------
// Member function median_edge_orientation() loops over all edges
// within *this.  It computes the angle between each edge's direction
// vector and +X_hat modulo 90 degrees.  This method returns the
// average of the canonical edge oriention angles.

double contour::median_edge_orientation() const
{
   vector<double> edge_orientation;
   for (unsigned int i=0; i<get_nvertices(); i++)
   {
      linesegment curr_edge(get_edge(i));
//      double curr_theta=mathfunc::angle_between_unitvectors(
//         curr_edge.get_ehat(),x_hat)
      threevector curr_ehat=curr_edge.get_ehat();
      double curr_theta=atan2(curr_ehat.get(1),curr_ehat.get(0));
      double canonical_theta=basic_math::phase_to_canonical_interval(
         curr_theta,0,PI/2);
      edge_orientation.push_back(canonical_theta);
   } // loop over index i labeling contour edges
   return mathfunc::median_value(edge_orientation);
}

// ---------------------------------------------------------------------
// Member function consolidate_parallel_edges searches over all edges
// of the current contour object and searches for any two adjacent
// edges which are parallel.  If such a pair is found, this method
// instantiates a new edge with the same direction vector as each each
// of the two parallel edges.  It also sets the new edge's length
// based upon the head of the first member and the tail of the second.
// The two parallel adjacent edges are replaced by their averaged
// replacement.  This member function returns an STL vector containing
// pointers to dynamically generated edge linesegments which are
// guaranteed to not contain any adjacent parallel members.

vector<linesegment*>* contour::consolidate_parallel_edges(
   double edge_angle_deviation)
{
//   cout << "inside contour::consolidate_parallel_edges()" << endl;

   Linkedlist<linesegment*>* segment_list_ptr=new Linkedlist<linesegment*>;
   
// First fill linked list with existing contour edges:

   for (unsigned int i=0; i<get_nedges(); i++)
   {
      linesegment* edge_ptr=new linesegment(edge[i]);
      segment_list_ptr->append_node(edge_ptr);
   }

// Loop over all edges in linked list.  Search for any two adjacent
// edges which are parallel.  If such a pair is found, instantiate a
// new edge with the same direction vector as each pair member and
// with a length set by the head of the first member to the tail of
// the second.  Replace the pair of parallel edges with their new
// averaged replacement in the link list.  

   bool contour_changed_flag=false;
   bool edges_changed_flag=false;
   int n_iters=0;
   do
   {
      edges_changed_flag=false;
      int posn=0;
      for (Mynode<linesegment*>* currnode_ptr=segment_list_ptr->
              get_start_ptr(); currnode_ptr != NULL; 
           currnode_ptr=currnode_ptr->get_nextptr())
      {
         Mynode<linesegment*>* nextnode_ptr=currnode_ptr->get_nextptr();
         if (nextnode_ptr==NULL) nextnode_ptr=segment_list_ptr->
                                    get_start_ptr();
         linesegment* curr_edge_ptr=currnode_ptr->get_data();
         linesegment* next_edge_ptr=nextnode_ptr->get_data();

         double absolute_angle=mathfunc::absolute_angle_between_unitvectors(
            curr_edge_ptr->get_ehat(),next_edge_ptr->get_ehat());

         if (nearly_equal(absolute_angle,edge_angle_deviation) ||
             absolute_angle < edge_angle_deviation)
         {
            linesegment slanted_edge(curr_edge_ptr->get_v1(),
                                     next_edge_ptr->get_v2());
            double proj_length=slanted_edge.get_length()*
               (slanted_edge.get_ehat().dot(curr_edge_ptr->get_ehat()));
            threevector midpoint(slanted_edge.get_midpoint());
            linesegment* new_edge_ptr=
               new linesegment(
                  midpoint-0.5*proj_length*curr_edge_ptr->get_ehat(),
                  midpoint+0.5*proj_length*curr_edge_ptr->get_ehat());
            segment_list_ptr->create_and_insert_node(
               posn-1,posn,new_edge_ptr);

            delete curr_edge_ptr;
            delete next_edge_ptr;
            segment_list_ptr->delete_node(currnode_ptr);
            segment_list_ptr->delete_node(nextnode_ptr);

            edges_changed_flag=true;
            contour_changed_flag=true;
            n_iters++;
            break;

         } // curr and next edges are nearly parallel conditional
         posn++;
      } // loop over nodes in *segment_list_ptr
   } 
   while (edges_changed_flag);
//   cout << "n_iters = " << n_iters << endl;

   if (contour_changed_flag)
   {
//      string banner=
//         "*** Contour changed in contour::consolidate_parallel_edges() ***";
//      outputfunc::write_banner(banner);
   }
   
// Fill STL vector with edge line segments in *segment_list_ptr:
   
   vector<linesegment*>* perturbed_edge_ptr=segment_list_ptr->
      convert_list_to_vector();
   delete segment_list_ptr;

//   cout << "perturbed_edge_ptr->size() = "
//        << perturbed_edge_ptr->size() << endl;
   return perturbed_edge_ptr;
}

// ---------------------------------------------------------------------
// Member function generate_consolidated_contour() consolidates edges
// whose direction angles differ by less than input threshold opening
// angle edge_angle_deviation.  It then forms a new set of vertices
// from the consolidated edges.  This method returns a contour from
// the reduced number of vertices.

contour contour::generate_consolidated_contour(double edge_angle_deviation)
{
//   cout << "inside contour::generate_consolidated_contour()" << endl;
//   cout << "get_nvertices() = " << get_nvertices() << endl;

   vector<threevector> vertices;
   vector<linesegment*>* consolidated_edges_ptr=
      consolidate_parallel_edges(edge_angle_deviation);

   for (unsigned int e=0; e<consolidated_edges_ptr->size(); e++)
   {
      linesegment* curr_edge_ptr=consolidated_edges_ptr->at(e);
      vertices.push_back( curr_edge_ptr->get_v1() );
      delete curr_edge_ptr;
   }
   delete consolidated_edges_ptr;

//   cout << "n_consolidated vertices = " << vertices.size() << endl;

// Form new contour with parallel edges consolidated:

   contour consolidated_contour(vertices);
   consolidated_contour.compute_edges();

//   cout << "At end of contour::generate_consolidated_contour()" << endl;
//   cout << "consolidated.contour.get_nvertices() = "
//        << consolidated_contour.get_nvertices() << endl;
//   cout << "consolidated.contour.get_nedges() = "
//        << consolidated_contour.get_nedges() << endl;
   
   return consolidated_contour;
}

// ---------------------------------------------------------------------
// Member function align_edges_with_sym_dirs takes in two direction
// vectors w_hat and l_hat.  Looping over all edges within the current
// contour object, this method computes the overlap between each
// segment's direction vector and w_hat, l_hat, -w_hat & -l_hat.  It
// subsequently rotates each edge about its midpoint so that it's
// aligned with the maximum overlap direction vector.

void contour::align_edges_with_sym_dirs(
   const threevector& w_hat,const threevector& l_hat,int i_first,
   bool debug_flag)
{
   int i=modulo(i_first,nvertices);
   threevector f_hat(edge[i].max_direction_overlap(w_hat,l_hat));
   threevector fhat_first(f_hat);
   edge[i].rotate_about_midpoint(f_hat);

   threevector fhat_prev(f_hat);
   for (unsigned int j=i_first+1; j<i_first+nvertices; j++)
   {
      int i=modulo(j,nvertices);
      threevector candidate_f_hat=edge[i].max_direction_overlap(w_hat,l_hat);

// Do not permit adjacent edges to change direction vectors by 180
// degrees!

      if (candidate_f_hat.dot(fhat_prev) > -0.5)
      {
         f_hat=candidate_f_hat;
      }
      else
      {
         
// Turn by 90 degrees to avoid anti-parallel adjacent orthogonal
// contour edges.  If current contour object is left-handed, turn by
// -90 degrees rather than +90 degrees.

         f_hat=z_hat.cross(fhat_prev);
         if (!right_handed_vertex_ordering()) f_hat=-f_hat;

         if (debug_flag)
         {
            cout << "fhat_prev = " << fhat_prev << endl;
            cout << "candidate_f_hat = " << candidate_f_hat << endl;
            cout << "Setting f_hat = z_hat x fhat_prev = " << f_hat << endl;
            cout << " rather than to candidate_f_hat" << endl;
         }
      }
      edge[i].rotate_about_midpoint(f_hat);
      fhat_prev=f_hat;
   } // loop over index j labeling vertices modulo nvertices

// Check whether edges i_first and ifirst+nvertices-1 are
// anti-parallel.  If so, call this method again after incrementing
// i_first:

   if (f_hat.dot(fhat_first) < -0.5 && debug_flag)
   {
      cout << "Trouble in contour::align_edges_with_sym_dirs()!" << endl;
      cout << "Edges i_first = " << i_first << endl;
      cout << " & i_first+nvertices-1 = " << i_first+nvertices-1
           << " are anti-parallel" << endl;
      cout << "f_hat(i_first) = " << fhat_first << endl;
      cout << "f_hat(i_first+nvertices-1) = " << f_hat << endl;
      align_edges_with_sym_dirs(w_hat,l_hat,i_first+1,debug_flag);
   }
}

// ---------------------------------------------------------------------
// Member function align_edges_with_sym_dirs() takes in starting
// symmetry direction vector w_hat along with angle delta_theta.  It
// rotates each edge about its midpoint so that it's aligned with the
// maximum overlap direction vector defined by w_hat,
// w_hat+delta_theta, w_hat+2*delta_theta,etc...

void contour::align_edges_with_sym_dirs(
   const threevector& w_hat,double delta_theta)
{
   cout << "inside contour::align_edges_with_sym_dirs() #1" << endl;

   for (unsigned int i=0; i<get_nedges(); i++)
   {
      threevector f_hat(edge[i].max_direction_overlap(w_hat,delta_theta));
//      cout << "i = " << i << " f_hat = " << f_hat << endl;
//      cout << "before rot: v1 = " << edge[i].get_v1() << endl;
//      cout << "before rot: v2 = " << edge[i].get_v2() << endl;
//      cout << "before: e_hat = " << edge[i].get_ehat() << endl;      

      edge[i].rotate_about_midpoint(f_hat);
//      cout << "after rot: v1 = " << edge[i].get_v1() << endl;
//      cout << "after rot: v2 = " << edge[i].get_v2() << endl;
//      cout << "e_hat = " << edge[i].get_ehat() << endl;
   } // loop over index i labeling contour edges
}



// ---------------------------------------------------------------------
// Member function align_edges_with_sym_dirs()

contour contour::align_edges_with_sym_dirs(
   double avg_canonical_edge_orientation,double delta_theta,
   double edge_angle_deviation,double ds_min)
{
   cout << "inside contour::align_edges_with_sym_dirs() #2" << endl;
//   cout << "get_nedges() = " << get_nedges() << endl;

   threevector w_hat(cos(avg_canonical_edge_orientation),
                     sin(avg_canonical_edge_orientation));
   align_edges_with_sym_dirs(w_hat,delta_theta);

   vector<threevector> vertices;
   for (unsigned int e=0; e<get_nedges(); e++)
   {
      linesegment curr_edge=get_edge(e);
      linesegment next_edge=get_edge((modulo(e+1,get_nedges())));
      vertices.push_back(curr_edge.get_v1());
      threevector v2=curr_edge.get_v2();
      threevector next_v1=next_edge.get_v1();
      if (! v2.nearly_equal(next_v1) ) vertices.push_back(v2);
   }
//   cout << "vertices.size() = " << vertices.size() << endl;

/*
   if (!is_even(vertices.size()))
   {
      cout << "Error in contour::align_edges_with_sym_dirs()" << endl;
      cout << "vertices.size() = " << vertices.size() << " isn't even!"
           << endl;
      exit(-1);
   }
*/

   vector<threevector> realigned_vertices;

/*
   for (unsigned int v=0; v<vertices.size(); v++)
   {
      realigned_vertices.push_back(vertices[v]);
   }
*/


   realigned_vertices.push_back(vertices[0]);
   for (unsigned int v=0; v<vertices.size(); v++)
   {
      threevector v1=vertices[v];
      threevector v2=vertices[modulo(v+1,vertices.size())];
      linesegment l(v1,v2);
      threevector v1prime=vertices[modulo(v+2,vertices.size())];
      linesegment l_connector(v2,v1prime);
      threevector v2prime=vertices[modulo(v+3,vertices.size())];
      linesegment lprime(v1prime,v2prime);

      double ratio=l.get_length()/l_connector.get_length();
      double ratioprime=lprime.get_length()/l_connector.get_length();
      const double min_ratio=10;
//      const double min_ratio=20;

      double theta=mathfunc::absolute_angle_between_unitvectors(
         l.get_ehat(),lprime.get_ehat());
//      cout << "ratio = " << ratio << " ratioprime = " << ratioprime
//           << " theta = " << theta*180/PI << endl;
      if (ratio > min_ratio && ratioprime > min_ratio)
      {
         if (theta < edge_angle_deviation)
         {
            threevector r=v2prime-v1;
            threevector e_hat=l.get_ehat();
            threevector rperp=r-(r.dot(e_hat))*e_hat;
            threevector rperp_hat=rperp.unitvector();
            double dperp=r.dot(rperp_hat);
            threevector vnew_1=v1+0.5*dperp*rperp_hat;
            threevector vnew_2=v1-0.5*dperp*rperp_hat;
         
//         threevector vnew_2=v2prime-dperp*rperp_hat;
//         realigned_vertices.push_back(vnew_1);
         
            realigned_vertices[realigned_vertices.size()-1]=vnew_1;
            realigned_vertices.push_back(vnew_2);
         }
         else
         {
            threevector midpoint=0.5*(v2+v1prime);
            realigned_vertices.push_back(midpoint);
            vertices[modulo(v+2,vertices.size())]=midpoint;
         } 
      }
      else
      {
//         realigned_vertices.push_back(v1);
      } // ratio && ratioprime > min_ratio conditional
         
   } // loop over index v labeling vertices
//   cout << "n_realigned_vertices = " << realigned_vertices.size() << endl;

// Aggregate any realigned vertices which lie very close to each other:

   vector<threevector> aggregated_vertices;
   aggregated_vertices.push_back(realigned_vertices[0]);
   for (unsigned int v=0; v<realigned_vertices.size()-1; v++)
   {
      threevector curr_vertex=aggregated_vertices.back();
      threevector next_vertex=
         realigned_vertices[modulo(v+1,realigned_vertices.size())];
      double separation=(next_vertex-curr_vertex).magnitude();
//      cout << "v = " << v << " ds_min = " << ds_min 
//           << " separation = " << separation << endl;
      if (separation > ds_min)
      {
         aggregated_vertices.push_back(next_vertex);
      }
   }
//   cout << "n_aggregated_vertices = " << aggregated_vertices.size() << endl;

// Form new contour with parallel edges consolidated:

//   contour realigned_contour(realigned_vertices);
   contour realigned_contour(aggregated_vertices);
   realigned_contour.compute_edges();

//   cout << "At end of contour::align_edges_with_sym_dir()" << endl;
//   cout << "realigned_contour.get_nvertices() = "
//        << realigned_contour.get_nvertices() << endl;
//   cout << "religned_contour.get_nedges() = "
//        << realigned_contour.get_nedges() << endl;

   *this=realigned_contour;
   return realigned_contour;
}

// ---------------------------------------------------------------------
// Member function locate_contour_corners takes in direction vectors
// w_hat and l_hat.  We assume that the contour's edge segments have
// already been aligned with w_hat and l_hat via a call to member
// function align_edges_with_sym_dirs().  This method loops over the
// contour's edges and looks for changes in e_hat.w_hat & e_hat.l_hat.
// Such changes mark the corner points which are returned within a
// dynamically generated linked list of edge integer labels.

Linkedlist<int>* contour::locate_contour_corners(
   const threevector& w_hat,const threevector& l_hat)
{
   Linkedlist<int>* corner_list_ptr=new Linkedlist<int>;

// Find first corner point:

   double wdot_prev=edge[0].get_ehat().dot(w_hat);
   double ldot_prev=edge[0].get_ehat().dot(l_hat);
   
   unsigned int i=0;
   while(i < nvertices)
   {
      double wdot=edge[i].get_ehat().dot(w_hat);
      double ldot=edge[i].get_ehat().dot(l_hat);
      if (nearly_equal(wdot,wdot_prev) && nearly_equal(ldot,ldot_prev))
      {
         i++;
      }
      else
      {
         wdot_prev=wdot;
         ldot_prev=ldot;
         corner_list_ptr->append_node(i);
//         cout << "First corner found at start of edge i = " << i << endl;
//         cout << "wdot = " << wdot << " ldot = " << ldot << endl;
//         cout << "edge[i] = " << edge[i] << endl;
         break;
      }
   }

// Look for subsequent changes in dotproduct overlap of e_hat with
// w_hat and l_hat to identify remaining corner points:

   bool error_flag=false;
   for (unsigned int j=0; j<nvertices; j++)
   {
      double wdot=edge[i].get_ehat().dot(w_hat);
      double ldot=edge[i].get_ehat().dot(l_hat);

// Adjacent contour edges should be oriented at right angles.  If they
// are not, something has gone terribly wrong!

      if (wdot*wdot_prev < -0.9 || ldot*ldot_prev < -0.9)
      {
//         cout << "*********************************************" << endl;
//         cout << "Major error in contour::locate_contour_corners()" << endl;
//         cout << "i = " << i << " j = " << j << endl;
//         cout << "edge[i] = " << edge[i] << endl;
//         cout << "wdot = " << wdot << " wdot_prev = " << wdot_prev << endl;
//         cout << "ldot = " << ldot << " ldot_prev = " << ldot_prev << endl;
//         cout << "*********************************************" << endl;
         error_flag=true;
      }
      
      if (!(nearly_equal(wdot,wdot_prev) && nearly_equal(ldot,ldot_prev)))
      {
         wdot_prev=wdot;
         ldot_prev=ldot;
         corner_list_ptr->append_node(i);
//         cout << "Next corner found at start of edge = " << i << endl;
      }
      i=modulo(i+1,nvertices);
   } // loop over index j

// Number of corners must be even!

   int n_corners=corner_list_ptr->size();
   if (is_odd(n_corners))
   {
//      cout << "*********************************************" << endl;
//      cout << "Major error in contour::locate_contour_corners()" << endl;
//      cout << "corner_list_ptr->size() = " << n_corners << endl;
//      cout << "*********************************************" << endl;
      error_flag=true;
   }

   if (error_flag)
   {
//      cout << "Contour = " << *this << endl;
//      cout << "corner_list = " << *corner_list_ptr << endl;
   }

   return corner_list_ptr;
}

// =====================================================================
// Contour triangulation methods:
// =====================================================================

// Member function locate_origin finds some point lying inside the
// current polygon object which can serve as an origin for normal
// vector, polygon area and interior/exterior computations.  We simply
// construct bisector linesegments that join vertices of the current
// polygon object to midpoints of opposing sides.  The midpoints of
// the bisector segments are tested to determine whether they lie
// inside the polygon.  The first one which does is taken to be the origin.

threevector& contour::locate_origin()
{
   bool origin_located=false;

   if (nvertices==3)
   {
      origin=vertex_average();
      origin_located=true;
   }
   else
   {

// As of Nov 04, we try to "statically" allocate the following side
// array rather than dynamically allocate member edge array.  Perhaps
// we are just fooling ourselves given that the following array must
// in fact be allocated at run-time rather than at compile-time...

      linesegment side[nvertices];
      for (unsigned int i=0; i<nvertices; i++)
      {     
         side[i]=linesegment(get_vertex(i).first,
                             get_vertex(modulo(i+1,nvertices)).first);
      }

      unsigned int i=0;	// index i labels side of current polygon object
      do
      {
         linesegment currside(side[i]);
         unsigned int j=0;
         do
         {
            if (i != j)
            {
               threevector midpnt(side[j].get_midpoint());
               linesegment bisector=linesegment(get_vertex(i).first,midpnt);
               threevector trial_origin(bisector.get_midpoint());
               if (point_inside(trial_origin))
               {
                  origin=trial_origin;
                  origin_located=true;
               }
            }
            j++;
         }
         while (j < nvertices && origin_located==false);
      
         i++;
      }
      while (i < nvertices && origin_located==false);
   } // nvertices==3 conditional
   return origin;
}

// ---------------------------------------------------------------------
// Method robust_locate_origin() searches for convex vertices within
// *this.  When it finds one, this method forms the triangle from the
// convex vertex, the preceding edge's v1 vertex and the subsequent
// edge's v2 vertex.  It explicitly tests whether the triangle's
// center lies within *this.  If so, this method sets the contour's
// origin to the triangle's COM.  Otherwise, it continues on to the
// next convex vertex in the contour.

threevector& contour::robust_locate_origin()
{
//   cout << "inside contour::robust_locate_origin()" << endl;
   unsigned int vertex_counter=0;
   
   while (vertex_counter < get_nvertices()-1)
   {
      pair<threevector,bool> curr_vertex=get_vertex(vertex_counter);
      if (curr_vertex.second) 
      {
         linesegment before_edge ( get_edge(
            modulo(vertex_counter-1,get_nvertices())) );
         linesegment after_edge ( get_edge(vertex_counter) ) ;
//         cout << "vertex_counter = " << vertex_counter << endl;
//         cout << "z_hat.dot(before_edge x after_edge) = "
//              << z_hat.dot(before_edge.get_ehat().cross(
//                              after_edge.get_ehat())) << endl;
         threevector avg_triangle_vertex(before_edge.get_v1());
         avg_triangle_vertex += before_edge.get_v2();
         avg_triangle_vertex += after_edge.get_v2();
         avg_triangle_vertex /= 3.0;

         if (point_inside(avg_triangle_vertex)) 
         {
            origin=avg_triangle_vertex;
            break;
         }
      }
      vertex_counter++;
   } // while loop over vertex counter

//   cout << "origin = " << origin << endl;
//   cout << "point_inside(origin) = " << point_inside(origin) << endl;
//   outputfunc::enter_continue_char();
   
   return origin;
}

// ---------------------------------------------------------------------
// Member function generate_Delaunay_triangles() uses the Delaunay
// triangulation built into the 2011 TrianglesGroup class rather than
// much older and deprecated Delaunay triangulation routines from
// "Computational Geometry in C" book by O'Rourke. 

triangles_group* contour::generate_Delaunay_triangles()
{
//   cout << "inside contour::generate_Delaunay_triangles()" << endl;
   
   delete triangles_group_ptr;
   triangles_group_ptr=new triangles_group;
   
   for (unsigned int n=0; n<get_nvertices(); n++)
   {
      pair<threevector,bool> p=get_vertex(n);
      vertex curr_Vertex(p.first,n);
      triangles_group_ptr->update_triangle_vertices(curr_Vertex);
   } // loop over index n labeling contour vertices
   
   triangles_group_ptr->delaunay_triangulate_vertices();

   for (unsigned int t=0; t<triangles_group_ptr->get_n_triangles(); t++)
   {
//      cout << "Triangle " << t << endl << endl;
      cout << *(triangles_group_ptr->get_triangle_ptr(t)) << endl;
   }
   return triangles_group_ptr;
}

// ---------------------------------------------------------------------
// Member function generate_interior_triangles() utilizes John
// Ratcliff's static triangulator class in order to triangulate a 2D
// contour living in the XY plane which has no holes.

triangles_group* contour::generate_interior_triangles()
{
//   cout << "inside contour::generate_interior_triangles()" << endl;
   
   delete triangles_group_ptr;
   triangles_group_ptr=new triangles_group;

   vector<double> vertex_Z_values;
   
   threevector prev_posn(NEGATIVEINFINITY,NEGATIVEINFINITY,NEGATIVEINFINITY);
   for (unsigned int n=0; n<get_nvertices(); n++)
   {
      pair<threevector,bool> p=get_vertex(n);
      vertex curr_Vertex(p.first,n);
//      cout << "n = " << n 
//           << " n_vertices = " << get_nvertices() 
//           << " curr_Vertex = " << curr_Vertex << endl;
//      cout << "prev_posn = " << prev_posn << endl;

      if (prev_posn.nearly_equal(curr_Vertex.get_posn()))
      {
         threevector curr_posn=curr_Vertex.get_posn();

// On 10/22/13, we discovered the hard and painful way that
// curr_Vertex can sometimes numerically equal prev_Vertex.  If so, we
// randomly perturb the 3 coordinates of curr_Vertex in order to avoid
// overwriting the previous vertex within the vertices handler member
// of *triangles_group_ptr:

         double TINY=1E-4;
         for (unsigned int i=0; i<3; i++)
         {
            curr_posn.put(i,curr_posn.get(i)+nrfunc::ran1()*TINY);
         }
         curr_Vertex.set_posn(curr_posn);
         set_vertex(n,curr_posn,p.second);
      }
      prev_posn=curr_Vertex.get_posn();

      triangles_group_ptr->update_triangle_vertices(curr_Vertex);
      vertex_Z_values.push_back(curr_Vertex.get_posn().get(2));
   } // loop over index n labeling contour vertices

   double median_Z_value=mathfunc::median_value(vertex_Z_values);
   triangles_group_ptr->inner_triangulate_vertices(median_Z_value);

//   for (unsigned int t=0; t<triangles_group_ptr->get_n_triangles(); t++)
//   {
//      cout << "Triangle " << t << endl << endl;
//      cout << *(triangles_group_ptr->get_triangle_ptr(t)) << endl;
//   }

   return triangles_group_ptr;
}

// =====================================================================
// Distance to boundary methods:
// =====================================================================

// Member function ray_pierces_me takes with the current contour
// object which is assumed to contain nontrivial local z information.
// It first computes the xy projection of the current contour.  It
// next loops over the contour vertices and forms rectangular side
// faces which link together the contour object and its xy projection.
// This method tests whether the ray whose basepoint and direction
// vector are passed as inputs pierces any of the side faces.  If so,
// this boolean method returns true.

bool contour::ray_pierces_me(
   double tan_elevation,const threevector& ray_basepoint,
   const threevector& ray_hat,int vertex_skip)
{
   static vector<threevector> v(4);
   for (unsigned int i=0; i<nvertices; i += vertex_skip)
   {
      int next_i=modulo(i+vertex_skip,nvertices);
      v[0]=get_vertex(next_i).first;
      v[1]=get_vertex(i).first;
      v[2]=threevector(v[1].get(0),v[1].get(1));
      v[3]=threevector(v[0].get(0),v[0].get(1));
      polygon side_face(v);

      double height=v[0].get(2);
      double sqrd_basepoint_sideface_dist=
         (ray_basepoint-side_face.vertex_average()).sqrd_magnitude();
      double max_shadow_length_on_ground=height/tan_elevation;
      if (9*sqr(max_shadow_length_on_ground) > sqrd_basepoint_sideface_dist)
      {
         threevector pnt_in_plane;
         if (side_face.ray_projected_into_poly_plane(
            ray_basepoint,ray_hat,side_face.vertex_average(),pnt_in_plane))
         {
            if (side_face.point_inside_polygon(pnt_in_plane))
            {
               return true;
            }
         }
      } // max_shadow_length_on_ground > basepoint_sideface_dist conditional
   } // loop over index i labeling side faces
   
   return false;
}

// ---------------------------------------------------------------------
// Member function compute_rectangle_sideface works with the current
// contour object which is assumed to have nontrivial local height
// content.  It returns the rectangle formed from the contour vertices
// labeled by indices i and i+vertex_skip as well as their projections
// within the xy-plane.

polygon contour::compute_rectangle_sideface(int i,int vertex_skip)
{
   vector<threevector> v(4);

   int next_i=modulo(i+vertex_skip,nvertices);
   v[0]=get_vertex(next_i).first;
   v[1]=get_vertex(i).first;
   v[2]=threevector(v[1].get(0),v[1].get(1));
   v[3]=threevector(v[0].get(0),v[0].get(1));
   return polygon(v);
}

/*
// ---------------------------------------------------------------------
// Member function compute_vertices_voronoi_diagram returns a
// dynamically generated linked list of Voronoi polygons corresponding
// to each of the vertices within the current contour object.

Linkedlist<polygon>* contour::compute_vertices_voronoi_diagram() const
{
   vector<threevector> site(nvertices);
   for (unsigned int i=0; i<nvertices; i++)
   {
      site[i]=get_vertex(i).first;

// In June 04, we learned (the hard way) that our Voronoi diagram
// generation fail spectacularly if the z-values for the Voronoi sites
// are non-zero.  So to avoid such catastrophic failures, we null the
// 3rd components of each site:

      site[i].put(2,0);
   }
   Linkedlist<polygon>* voronoi_poly_list_ptr=
      voronoifunc::generate_voronoi_regions(site);

   return voronoi_poly_list_ptr;
}
*/

// =====================================================================
// Orthogonal contour methods:
// =====================================================================

// Member function
// decompose_orthogonal_concave_contour_into_subcontours extracts the
// largest rectangle (measured in terms of area) that lies within the
// current contour object which is assumed to be orthogonal.  It uses
// this interior rectangle to identify an infinite "cleaving" line
// (which partially overlaps at least one contour edge and totally
// overlaps one of the rectangle's sides).  This member function next
// computes starting vertices located on the cleaving line which act
// as subcontour seeds.  It subsequently generates lists of vertices
// which trace out right-handed subcontours.  The subcontours (which
// are not necessarily) convex are returned by this member function
// within a dynamically generated linked list.

Linkedlist<contour*>* 
contour::decompose_orthogonal_concave_contour_into_subcontours() const
{
   linesegment cleaving_line;
   Linkedlist<int>* starting_right_vertex_indices_list_ptr=NULL;
   Linkedlist<int>* starting_left_vertex_indices_list_ptr=NULL;
   return decompose_orthogonal_concave_contour_into_subcontours(
      cleaving_line,starting_right_vertex_indices_list_ptr,
      starting_left_vertex_indices_list_ptr);
}

Linkedlist<contour*>* 
contour::decompose_orthogonal_concave_contour_into_subcontours(
   linesegment& cleaving_line,
   Linkedlist<int>*& starting_right_vertex_indices_list_ptr,
   Linkedlist<int>*& starting_left_vertex_indices_list_ptr) const
{
   contour* largest_interior_rectangle_ptr=
      extract_largest_interior_rectangle();

//   cout << "largest_rectangle = " << *largest_interior_rectangle_ptr << endl;

   cleaving_line=orthogonal_concave_contour_cleaving_line(
      largest_interior_rectangle_ptr);
   delete largest_interior_rectangle_ptr;

//   cout << "cleaving_line = " << cleaving_line << endl;

// Locate starting vertices on the cleaving line for subcontours:

   Linkedlist<pair<int,threevector> >* starting_vertices_list_ptr=
      starting_vertices_on_cleaving_line(cleaving_line);

// Generate right-handed subcontour corresponding to each starting
// vertex on cleaving line:

   Linkedlist<contour*>* subcontour_list_ptr=new Linkedlist<contour*>;
   for (Mynode<pair<int,threevector> >* vertex_node_ptr=
           starting_vertices_list_ptr->get_start_ptr(); 
        vertex_node_ptr != NULL;
        vertex_node_ptr=vertex_node_ptr->get_nextptr())
   {
      Linkedlist<threevector>* vertex_list_ptr=
         subcontour_vertices_to_right_of_cleaving_line(
            vertex_node_ptr,cleaving_line);

//      cout << "starting vertex lies on edge = "
//           << vertex_node_ptr->get_data().first << endl;
//      cout << "vertex_node_ptr->get_data().second = "
//           << vertex_node_ptr->get_data().second << endl;
//      cout << "Right sub contour vertices = " << *vertex_list_ptr << endl;
//      cout << "*********************************" << endl;

// For reasons we don't understand, we have empirically observed that
// subcontours sometimes only have 3 vertices (of which 2 are actually
// the same).  We discard such bogus subcontours.  In other cases,
// orthogonal subcontours have 5 or some other odd number of vertices.
// We execute the following call in order to fix up such mal-formed
// subcontours...

      geometry_func::simplify_orthogonal_contour(vertex_list_ptr);
      int n_nodes=vertex_list_ptr->size();

//      cout << "n_nodes = " << n_nodes << endl;
      if (is_even(n_nodes) && n_nodes >= 4)
      {
         contour* subcontour_ptr=new contour(vertex_list_ptr);
         subcontour_list_ptr->append_node(subcontour_ptr);
//         cout << "*subcontour_list_ptr = " << *subcontour_list_ptr << endl;
      }
      delete vertex_list_ptr;
   } // loop over starting right vertices on cleaving line
   return subcontour_list_ptr;
}

// ---------------------------------------------------------------------
// Member function extract_largest_interior_rectangle scans over all
// vertices of an orthogonal contour.  It first searches for contour
// edges whose endpoints are both convex vertices.  This method
// subsequently finds these edges' anti-parallel partners which can
// legitimately be used to construct rectangles fully enclosed by the
// orthogonal contour.  This method returns the enclosed rectangle
// with the largest area.

contour* contour::extract_largest_interior_rectangle() const
{
   polygon* largest_rectangle_ptr=new polygon(4);

   int edge_index=0;
   for (Mynode<pair<threevector,bool> > const *currnode_ptr=vertex_list_ptr->
           get_start_ptr(); currnode_ptr != NULL; 
        currnode_ptr=currnode_ptr->get_nextptr())
   {

// First extract pairs of adjacent convex vertices:

      Mynode<pair<threevector,bool> > const *nextnode_ptr;
      if (currnode_ptr==vertex_list_ptr->get_stop_ptr())
      {
         nextnode_ptr=vertex_list_ptr->get_start_ptr();
      }
      else
      {
         nextnode_ptr=currnode_ptr->get_nextptr();
      }
      pair<threevector,bool> p1=currnode_ptr->get_data();
      pair<threevector,bool> p2=nextnode_ptr->get_data();

      if (p1.second==true && p2.second==true)
      {
//         cout << "Vertices " << p1.first << " and " << p2.first << endl;
//         cout << " are adjacent convex corners" << endl;
         linesegment curr_edge(edge[edge_index]);
         double inter_edge_distance=-1;
//         int partner_edge_index=
	   anti_parallel_partner_edge(edge_index,inter_edge_distance);
//         cout << "current edge index = " << edge_index << endl;
//         cout << "partner edge index = " << partner_edge_index << endl;
//         cout << "inter_edge_distance = " << inter_edge_distance << endl;
//         cout << endl;
         threevector r_hat(radial_direction_vector(edge_index));
         vector<threevector> vertex(4);
         vertex[1]=curr_edge.get_v1();
         vertex[2]=curr_edge.get_v2();
         vertex[0]=vertex[1]-inter_edge_distance*r_hat;
         vertex[3]=vertex[2]-inter_edge_distance*r_hat;
         polygon rectangle(vertex);

//         cout << "Current rectangle = " << rectangle << endl;
         if (rectangle.compute_area() > largest_rectangle_ptr->get_area())
         {
            *largest_rectangle_ptr=rectangle;
         }
         
      } //  p1.second==true && p2.second==true conditional
      edge_index++;
   } // loop over nodes within vertex_list

// Convert largest rectangle from polygon to contour:

   contour* largest_rectangle_contour_ptr=new contour(largest_rectangle_ptr);
   delete largest_rectangle_ptr;
//   cout << "largest rectangle contour = " 
//        << *largest_rectangle_contour_ptr << endl;
   return largest_rectangle_contour_ptr;
}

// ---------------------------------------------------------------------
// Member function anti_parallel_partner_edge takes in the integer
// label for some orthogonal contour edge.  It scans over all other
// contour edges which are anti-parallel to the input edge.  Of these,
// it rejects those which have no overlap in the contour's local
// inward radial direction with the input edge.  This method
// subsequently computes the infinite parallel line distances between
// the input edge and the surviving candidates.  It returns the index
// of the anti-parallel edge which lies closest to the input edge.  

int contour::anti_parallel_partner_edge(
   int edge_index,double& min_inter_edge_distance) const
{
   linesegment curr_edge(edge[edge_index]);

// Search for all edges anti-aligned with current edge which represent
// candidate opposite edge partners.  Fill linked list with these
// candidate partner edges' indices:

   Linkedlist<int>* candidate_edge_indices_list_ptr=new Linkedlist<int>;
   for (unsigned int j=edge_index; j<edge_index+nvertices; j++)
   {
      int i=modulo(j,nvertices);
      linesegment candidate_edge(edge[i]);
      double dotproduct(curr_edge.get_ehat().dot(candidate_edge.get_ehat()));
      if (nearly_equal(dotproduct,-1))
      {
//         cout << "Candidate opposite edge = " << candidate_edge << endl;
         candidate_edge_indices_list_ptr->append_node(i);
      }
   } // loop over index j labeling candidate opposite edges to current edge

//   cout << "candidate edge indices = " << endl;
//   cout << *candidate_edge_indices_list_ptr << endl;

   int partner_edge_index=-1;	// index labeling partner to curr_edge
   min_inter_edge_distance=POSITIVEINFINITY;

// Compute ehat coordinates of candidate partner edges.  Remove any
// edge from the candidate partner linked list whose ehat coordinates
// do not at least partially overlap with those of the current edge:

   double E1=curr_edge.get_v1().dot(curr_edge.get_ehat());
   double E2=curr_edge.get_v2().dot(curr_edge.get_ehat());
//   cout << "E1 = " << E1 << " E2 = " << E2 << endl;
   Mynode<int>* curredge_ptr=candidate_edge_indices_list_ptr->get_start_ptr();
   while (curredge_ptr != NULL)
   {
      Mynode<int>* nextedge_ptr=curredge_ptr->get_nextptr();
      int i=curredge_ptr->get_data();
//      cout << "Candidate partner edge " << i << endl;

      double eps1=edge[i].get_v1().dot(curr_edge.get_ehat());
      double eps2=edge[i].get_v2().dot(curr_edge.get_ehat());
//      cout << "eps1 = " << eps1 << " eps2 = " << eps2 << endl;
      double min_eps=basic_math::min(eps1,eps2);
      double max_eps=basic_math::max(eps1,eps2);
      if ((E1 <= min_eps && E2 <= min_eps) || 
          (E1 >= max_eps && E2 >= max_eps))
      {
         candidate_edge_indices_list_ptr->delete_node(curredge_ptr);
      }
      else
      {

// Compute infinite parallel line distance between curr_edge and
// candidate partner edge.  Record index for partner whose distance to
// curr_edge is smallest:

         double inter_edge_distance=curr_edge.point_to_line_distance(
            edge[i].get_v1());
//         cout << "inter edge distance = " << inter_edge_distance << endl;
         if (inter_edge_distance < min_inter_edge_distance)
         {
            min_inter_edge_distance=inter_edge_distance;
            partner_edge_index=i;
         }
      }
      curredge_ptr=nextedge_ptr;
   } // while currnode_ptr != NULL loop

//   cout << "After computing ehat coords, candidate_edge_indices_list = "
//        << *candidate_edge_indices_list_ptr << endl;
//   cout << "Partner edge index = " << partner_edge_index << endl;

   return partner_edge_index;
}

// ---------------------------------------------------------------------
// Member function orthogonal_concave_contour_cleaving_line compares
// each side of the input interior rectangle *rectange_ptr with each
// edge of the current contour object which is assumed to be
// orthogonal.  It first searches for those contour edges which are
// aligned with the interior rectangle edges.  This method
// subsequently checks whether the rectangle and contour edges lie
// along the same infinite line.  Finally, this method associates one
// particular contour edge with each edge of the interior rectangle by
// requiring two of their endpoints be coincident. 

// This method returns an interior rectangle edge which does NOT fully
// overlap with its associated contour edge.  Such a rectangle edge
// represents a cleaving line along with the contour can be decomposed
// in order to break it apart into maximally large rectangles.

linesegment contour::orthogonal_concave_contour_cleaving_line(
   contour* rectangle_ptr) const
{
   const double SMALL=1E-3;
   for (unsigned int i=0; i<4; i++)
   {
      linesegment rect_edge(rectangle_ptr->get_edge(i));
//      cout << "i = " << i << " rectangle edge = " << rect_edge << endl;

      for (unsigned int j=0; j<nvertices; j++)
      {
         linesegment contour_edge(edge[j]);

// Check whether current contour edge is aligned with rectangle edge:

         if (fabs(rect_edge.get_ehat().dot(contour_edge.get_ehat())) > 0.99)
         {

// Next check whether current contour edge and rectangle edge lie
// along the same infinite line:

            double dist_between_rect_and_contour_edges=rect_edge.
               point_to_line_distance(contour_edge.get_v1());
            if (dist_between_rect_and_contour_edges < SMALL)
            {

// Finally check absolute minimal distance between contour and
// rectangle edges' endpoints.  At least one should equal zero:

               double r1c1=(rect_edge.get_v1()-contour_edge.get_v1()).
                  magnitude();
               double r1c2=(rect_edge.get_v1()-contour_edge.get_v2()).
                  magnitude();
               double r2c1=(rect_edge.get_v2()-contour_edge.get_v1()).
                  magnitude();
               double r2c2=(rect_edge.get_v2()-contour_edge.get_v2()).
                  magnitude();
               bool contour_edge_overlap_with_rect_edge=
                  rect_edge.total_segment_overlap(contour_edge);
               
               if (nearly_equal(r1c1,0,SMALL) || 
                   nearly_equal(r1c2,0,SMALL) ||
                   nearly_equal(r2c1,0,SMALL) || 
                   nearly_equal(r2c2,0,SMALL) ||
                   contour_edge_overlap_with_rect_edge)
               {
//                  cout << "j = " << j << " contour edge = " << contour_edge
//                       << endl;
                  bool rect_edge_overlap_with_contour_edge=
                     contour_edge.total_segment_overlap(rect_edge);

                  if (!rect_edge_overlap_with_contour_edge) return rect_edge;
               } // contour & rectangle edges' endpoints proximity conditional
            } // parallel distance between rect & contour edges conditional
         } // rect_edge & contour_edge alignment conditional
      } // loop over index j labeling orthogonal contour edges
   } // loop over index i labeling rectangle sides

   cout << "Error in contour::orthogonal_concave_contour_cleaving_line()"
        << endl;
   cout << "No cleaving line found!" << endl;

   exit(-1);
}

// ---------------------------------------------------------------------
// Member function starting_vertices_on_cleaving_line takes in
// linesegment cleaving_line corresponding to some side of an interior
// rectangle for the current contour which is assumed to be
// orthogonal.  This method returns a dynamically generated linked
// list of pairs of contour edge indices along with position vectors
// for subcontour starting vertices that lie on the infinite extension
// of of the cleaving line segment.

Linkedlist<pair<int,threevector> >* contour::starting_vertices_on_cleaving_line(
   const linesegment& cleaving_line) const
{
   const double TINY=1E-6;
   Linkedlist<pair<int,threevector> >* starting_vertices_list_ptr=
      new Linkedlist<pair<int,threevector> >;

   for (unsigned int i=0; i<nvertices; i++)
   {

// First check whether current contour vertex lies on infinite
// cleaving line:

      threevector curr_vertex(get_vertex(i).first);
      threevector next_vertex(get_vertex(modulo(i+1,nvertices)).first);
      if (cleaving_line.point_to_line_distance(curr_vertex) < TINY && 
          cleaving_line.point_to_line_distance(next_vertex) > TINY)
      {
         pair<int,threevector> p;
         p.first=i;
         p.second=get_vertex(i).first;
         starting_vertices_list_ptr->append_node(p);
      }
      else
      {

// Next check whether cleaving line intersects current contour edge
// somewhere in its middle:

         threevector intersection_point;
         linesegment curr_edge(edge[i]);
         if (curr_edge.segment_intersects_infinite_line(
            cleaving_line,intersection_point) &&
             cleaving_line.point_to_line_distance(curr_edge.get_v1()) > TINY 
             &&
             cleaving_line.point_to_line_distance(curr_edge.get_v2()) > TINY)
         {
            pair<int,threevector> p;
            p.first=i;
            p.second=intersection_point;
            starting_vertices_list_ptr->append_node(p);
         }
      }
   } // loop over index i labeling contour vertices & edges
   return starting_vertices_list_ptr;
}

// ---------------------------------------------------------------------
// Member functions subcontour_vertices_to_right_of_cleaving_line
// dynamically generates a linked list of subcontour position
// vertices.  The subcontour starts at input pair member
// vertex_node_ptr->get_data().second, and it continues around the
// current orthogonal contour according to the right-hand rule until
// the "infinite" cleaving line is encountered.

Linkedlist<threevector>* contour::subcontour_vertices_to_right_of_cleaving_line(
   Mynode<pair<int,threevector> >* vertex_node_ptr,
   const linesegment& cleaving_line) const
{
   
// Add starting vertex to head of new linked list of vertices:

   int i=vertex_node_ptr->get_data().first;
   Linkedlist<threevector>* vertex_list_ptr=new Linkedlist<threevector>;   
   vertex_list_ptr->append_node(vertex_node_ptr->get_data().second);

// Orthogonal contour's next vertex also definitely lies on
// subcontour:

   i=modulo(i+1,nvertices);
   vertex_list_ptr->append_node(get_vertex(i).first);

   bool cleaving_line_encountered=false;
   const double TINY=1E-6;
   const double max_separation=0.001;	// meter
   threevector intersection_point;
   while (!cleaving_line_encountered)
   {
      linesegment curr_edge(edge[i]);

// Check whether current edge terminates on infinite cleaving line:

      if (fabs(curr_edge.get_ehat().dot(cleaving_line.get_ehat())) < TINY &&
          cleaving_line.point_to_line_distance(curr_edge.get_v2()) < 
          max_separation)
      {
         cleaving_line_encountered=true;
         vertex_list_ptr->append_node(curr_edge.get_v2());
      }

// Check whether current edge runs over infinite cleaving line:

      else if (curr_edge.segment_intersects_infinite_line(
         cleaving_line,intersection_point))
      {
         cleaving_line_encountered=true;
         vertex_list_ptr->append_node(intersection_point);
      }
      else
      {
         vertex_list_ptr->append_node(curr_edge.get_v2());
      }
      i=modulo(i+1,nvertices);
   } // while !cleaving_line_encountered loop

   return vertex_list_ptr;
}

// ---------------------------------------------------------------------
// Member function orthogonal_contour_bbox returns a dynamically
// generated rectangular convex hull that wraps around the current
// contour which is assumed to be orthogonal.

contour* contour::orthogonal_contour_bbox() const
{
   threevector w_hat(edge[0].get_ehat());
   threevector l_hat(edge[1].get_ehat());
   double max_w=NEGATIVEINFINITY;
   double max_l=NEGATIVEINFINITY;
   double min_w=POSITIVEINFINITY;
   double min_l=POSITIVEINFINITY;

   threevector origin(get_vertex(0).first);
   for (unsigned int i=0; i<nvertices; i++)
   {
      threevector curr_vertex(get_vertex(i).first);
      double w((curr_vertex-origin).dot(w_hat));
      double l((curr_vertex-origin).dot(l_hat));
      max_w=basic_math::max(max_w,w);
      max_l=basic_math::max(max_l,l);
      min_w=basic_math::min(min_w,w);
      min_l=basic_math::min(min_l,l);
   }
   vector<threevector> vertex(4);
   vertex[0]=origin+max_w*w_hat+max_l*l_hat;
   vertex[1]=origin+min_w*w_hat+max_l*l_hat;
   vertex[2]=origin+min_w*w_hat+min_l*l_hat;
   vertex[3]=origin+max_w*w_hat+min_l*l_hat;
   contour* bbox_contour_ptr=new contour(vertex);
   bbox_contour_ptr->compute_orthogonal_contour_area();
   return bbox_contour_ptr;
}

// ---------------------------------------------------------------------
// Member function cleave_orthogonal_concave_contour first checks
// whether the current contour object is convex.  If not, it cleaves
// the contour into two dynamically generated orthogonal sub-contours
// at some concave vertex.  If either of the sub-contours is convex,
// it is added to the STL vector of convex orthogonal sub-contours
// which is returned by this member function.  If a sub-contour is not
// convex, this method is recursively called again.

void contour::cleave_orthogonal_concave_contour(
   vector<contour*>& convex_contour) const
{
   if (!is_convex())
   {
      contour *c1_ptr,*c2_ptr;
      cleave_orthogonal_concave_contour(c1_ptr,c2_ptr);
      if (c1_ptr->is_convex())
      {
         c1_ptr->compute_orthogonal_convex_contour_area();
         convex_contour.push_back(c1_ptr);
      }
      else
      {
         c1_ptr->cleave_orthogonal_concave_contour(convex_contour);
      }

      if (c2_ptr->is_convex())
      {
         c2_ptr->compute_orthogonal_convex_contour_area();
         convex_contour.push_back(c2_ptr);
      }
      else
      {
         c2_ptr->cleave_orthogonal_concave_contour(convex_contour);
      }
   }
   else
   {
      contour* c_ptr=new contour(*this);
      convex_contour.push_back(c_ptr);
   } // !is_convex conditional
}

// ---------------------------------------------------------------------
// This overloaded version of member function
// cleave_orthogonal_concave_contour chops apart the current
// orthogonal contour into two sub-contours which are also orthogonal.
// The current contour is assumed to be concave.  This method first
// searches for the location of a concave vertex.  It subsequently
// forms a sub-contour which includes a new vertex that would have
// existed had the contour turned right rather than left at the
// concave point.  It forms a 2nd sub-contour starting at the new
// vertex and including the remaining vertices traversed according to
// the right hand rule.  Both sub-contours are dynamically generated
// and returned by this method.

void contour::cleave_orthogonal_concave_contour(
   contour*& c1_ptr,contour*& c2_ptr) const
{

// Start search for convex subcontours at a concave vertex:

   int i=0;
   while (get_vertex(i).second)
   {
      i=modulo(i+1,nvertices);
   }
   threevector concave_vertex(get_vertex(i).first);
   threevector convex_dir(-edge[i].get_ehat());
   
// Find new vertex that would exist if contour had turned right rather
// than left at concave vertex i:

   const double TINY=1E-6;
   int j_edge_max=-1;
   int j_edge_min=-1;
   double min_distance=POSITIVEINFINITY;
   double max_distance=NEGATIVEINFINITY;
   threevector candidate_intersection_point,new_max_intersection_point,
      new_min_intersection_point;
   for (unsigned int j=0; j<nvertices; j++)
   {
      if (edge[j].direction_vector_intersects_segment(
         concave_vertex,convex_dir,candidate_intersection_point))
      {
         double curr_distance=edge[j].point_to_line_segment_distance(
            concave_vertex);
         if (curr_distance > max_distance)
         {
            j_edge_max=j;
            max_distance=curr_distance;
            new_max_intersection_point=candidate_intersection_point;
         }
         if (curr_distance < min_distance && curr_distance > TINY)
         {
            j_edge_min=j;
            min_distance=curr_distance;
            new_min_intersection_point=candidate_intersection_point;
         }
      }
   } // loop over index j labeling contour vertices

// Generate linked list of vertices starting at
// new_max_intersection_point and continuing around contour according
// to right-hand rule until concave vertex i is encountered:

   Linkedlist<threevector>* vertex1_list_ptr=new Linkedlist<threevector>;   
   vertex1_list_ptr->append_node(new_max_intersection_point);

   int j=j_edge_max;
   while (j != i)
   {
      j=modulo(j+1,nvertices);
      vertex1_list_ptr->append_node(get_vertex(j).first);
   }
 
// Generate linked list of vertices starting at
// new_min_intersection_point and continuing around contour according
// to left-hand rule until vertex i+1 is encountered:

   Linkedlist<threevector>* vertex2_list_ptr=new Linkedlist<threevector>;   
   vertex2_list_ptr->append_node(new_min_intersection_point);
   
   if ( (new_min_intersection_point-get_vertex(j_edge_min).first).magnitude()
        < TINY)
   {
      j=j_edge_min;
   }
   else
   {
      j=j_edge_min+1;
   }

   while (j != modulo(i+1,nvertices))
   {
      j=modulo(j-1,nvertices);
      vertex2_list_ptr->append_node(get_vertex(j).first);
   }

   c1_ptr=new contour(vertex1_list_ptr);
   c2_ptr=new contour(vertex2_list_ptr);
   delete vertex1_list_ptr;
   delete vertex2_list_ptr;

//   cout << "contour c1 = " << *c1_ptr << endl;
//   cout << "contour c2 = " << *c2_ptr << endl;
}

// ---------------------------------------------------------------------
// Member function compute_orthogonal_contour_area returns the area of
// either a concave or convex orthogonal contour.

double contour::compute_orthogonal_contour_area() 
{
   if (is_convex())
   {
      compute_orthogonal_convex_contour_area();
   }
   else
   {
      compute_orthogonal_concave_contour_area();
   }
   return area;
}

// ---------------------------------------------------------------------
// Member function compute_orthogonal_convex_contour_area returns the
// area of the rectangle defined by edge[0] and edge[1]

void contour::compute_orthogonal_convex_contour_area() 
{
   area=edge[0].get_length()*edge[1].get_length();
}

// ---------------------------------------------------------------------
// Member function compute_orthogonal_concave_contour_area breaks
// apart an arbitrary concave orthogonal contour into dynamically
// generated rectangles.  It sets the contour's total area equal to
// the sum of the rectangles' areas.

void contour::compute_orthogonal_concave_contour_area()
{
   vector<contour*> convex_subcontour;
   cleave_orthogonal_concave_contour(convex_subcontour);
   area=0;
   for (unsigned int i=0; i<convex_subcontour.size(); i++)
   {
      area += convex_subcontour[i]->get_area();

// Delete dynamically generated rectangle now that we're done with it:

      delete convex_subcontour[i];
   }
}

// ---------------------------------------------------------------------
// Member function orthogonal_concave_contour_COM breaks apart an
// arbitrary concave orthogonal contour into dynamically generated
// rectangles.  It sets the contour's center-of-mass equal to the
// rectangles' centers-of-mass weighted by their areas.

threevector contour::orthogonal_concave_contour_COM() const
{
   vector<contour*> convex_subcontour;
   cleave_orthogonal_concave_contour(convex_subcontour);
   
   double total_area=0;
   threevector COM(Zero_vector);
   for (unsigned int i=0; i<convex_subcontour.size(); i++)
   {
      double current_area=convex_subcontour[i]->
         compute_orthogonal_contour_area();
      total_area += current_area;
      COM += current_area*convex_subcontour[i]->vertex_average();

// Delete dynamically generated rectangle now that we're done with it:

      delete convex_subcontour[i];
   }
   COM /= total_area;
   return COM;
}

// =====================================================================
// Contour manipulation methods:
// =====================================================================

void contour::translate(const threevector& rvec)
{
   for (unsigned int i=0; i<nvertices; i++) 
   {
      set_vertex(i,get_vertex(i).first+rvec,get_vertex(i).second);
   }
   origin=origin+rvec;

   if (edge.size() > 0) compute_edges();
}

// ---------------------------------------------------------------------
void contour::scale(double scalefactor)
{
   if (scalefactor >= 0)
   {
      vertex_average();
      for (unsigned int i=0; i<nvertices; i++) 
      {
         threevector dv(get_vertex(i).first-vertex_avg);
         dv *= scalefactor;
         set_vertex(i,vertex_avg+dv);
      }
      threevector dv(origin-vertex_avg);
      dv *= scalefactor;
      origin=vertex_avg+dv;
      if (edge.size() > 0) compute_edges();
   }
}

// ---------------------------------------------------------------------
void contour::rotate(const threevector& rotation_origin,
                     double thetax,double thetay,double thetaz)
{
   rotation R(thetax,thetay,thetaz);
   rotate(rotation_origin,R);
}

// ---------------------------------------------------------------------
void contour::rotate(const threevector& rotation_origin,const rotation& R)
{
// First rotate polygon's vertices:

   for (unsigned int i=0; i<nvertices; i++)
   {
      threevector dv(get_vertex(i).first-rotation_origin);
      bool convex_vertex_flag(get_vertex(i).second);
      dv=R*dv;
      set_vertex(i,rotation_origin+dv,convex_vertex_flag);
   }

// Next rotate contour's origin:

   threevector dv(origin-rotation_origin);
   dv=R*dv;
   origin=rotation_origin+dv;

   if (edge.size() > 0) compute_edges();
}

// ---------------------------------------------------------------------
// Member function xy_projection projects the contour's vertices and
// edges into the xy plane.
   
void contour::xy_projection()
{
   for (unsigned int i=0; i<nvertices; i++) 
   {
      threevector curr_vertex(get_vertex(i).first);
      bool convex_vertex_flag(get_vertex(i).second);
      set_vertex(i,threevector(curr_vertex.get(0),curr_vertex.get(1)),
                 convex_vertex_flag);
   }
   origin=threevector(origin.get(0),origin.get(1));

   if (edge.size() > 0) compute_edges();
}

// ---------------------------------------------------------------------
// Member function max_z_value loops over all vertices within the
// current contour object.  It returns the vertices' maximum z value.

double contour::max_z_value() const
{
   double max_z=NEGATIVEINFINITY;
   for (unsigned int i=0; i<nvertices; i++)
   {
      max_z=basic_math::max(max_z,get_vertex(i).first.get(2));
   }
   return max_z;
}

// ---------------------------------------------------------------------
// Member function filter_z_values generates a gaussian filter with a
// width set by the input correlation distance.  It performs a brute
// force convolution of this gaussian with the z values associated
// with each contour vertex.  This method replaces the raw z values
// with their filtered counterparts.

void contour::filter_z_values(double correlation_dist)
{
   double h[nvertices];
   double raw_z[nvertices],filtered_z[nvertices];
   for (unsigned int i=0; i<nvertices; i++)
   {
      raw_z[i]=get_vertex(i).first.get(2);
   }
   
   filterfunc::gaussian_filter(nvertices,edge[0].get_length(),
                               correlation_dist,h);
   filterfunc::brute_force_filter(
      nvertices,nvertices,raw_z,h,filtered_z,true);

   for (unsigned int i=0; i<nvertices; i++)
   {
      threevector curr_vertex(get_vertex(i).first);
      bool convex_vertex_flag(get_vertex(i).second);
      set_vertex(i,threevector(
         curr_vertex.get(0),curr_vertex.get(1),filtered_z[i]),
                 convex_vertex_flag);
   }
}

// ---------------------------------------------------------------------
// Member function shrink_inwards displaces every edge within the
// contour by delta_r in its radially inwards direction.

void contour::shrink_inwards(double delta_r)
{
   for (unsigned int i=0; i<nvertices; i++)
   {
      threevector r_hat(radial_direction_vector(i));
      int next_i=modulo(i+1,nvertices);
      set_vertex(i,get_vertex(i).first-delta_r*r_hat,get_vertex(i).second);
      set_vertex(next_i,get_vertex(next_i).first-delta_r*r_hat,
                 get_vertex(next_i).second);
   }
   compute_edges();
}

