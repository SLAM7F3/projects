// ==========================================================================
// Stand-alone geometry methods
// ==========================================================================
// Last updated on 5/8/13; 5/9/13; 10/18/13; 4/4/14; 8/5/16
// ==========================================================================

#include <iostream>
#include "math/basic_math.h"
#include "geometry/bounding_box.h"
#include "math/constant_vectors.h"
#include "geometry/contour.h"
#include "general/filefuncs.h"
#include "geometry/geometry_funcs.h"
#include "linesegment.h"
#include "datastructures/Linkedlist.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "geometry/parallelepiped.h"
#include "geometry/polygon.h"
#include "math/rotation.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/threematrix.h"
#include "geometry/vertices_handler.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

namespace geometry_func
{

// ==========================================================================
// Moving around threevector methods:
// ==========================================================================

   void translate(threevector& V,const threevector& rvec)
      {
         V += rvec;
      }
   
   void scale(threevector& V,const threevector& scale_origin,
              const threevector& scalefactor)
      {
         threevector dV=V-scale_origin;
         dV=threevector(dV.get(0)*scalefactor.get(0),
                        dV.get(1)*scalefactor.get(1),
                        dV.get(2)*scalefactor.get(2));
         V=scale_origin+dV;
      }

   void rotate(threevector& V,const threevector& rotation_origin,
   	       const rotation& R)
      {
         threevector dV=V-rotation_origin;
         dV=R*dV;
         V=rotation_origin+dV;
      }

// ==========================================================================
// Point_to_line methods:
// ==========================================================================

// Method point_to_line_distance is a streamlined version of
// linesegment::point_to_line_distance() which can be used without
// having to instantiate a linesegment object.

   double point_to_line_distance(
      const threevector& curr_point,
      const threevector& line_basepoint,const threevector& line_e_hat)
      {
         threevector delta(curr_point-line_basepoint);
         double term1=sqr(delta.get(0))+sqr(delta.get(1))+sqr(delta.get(2));
         double term2=sqr(delta.dot(line_e_hat));

         if (term1 > term2)
         {
            return sqrt(term1-term2);
         }
         else
         {
            return 0;
         }
      }
   
// ---------------------------------------------------------------------
// Method line_to_line_squared_distance computes the minimal squared
// distance between two infinite lines which are labeled by their base
// points (v1 and w1) and their direction vectors (e_hat and f_hat).

   double line_to_line_squared_distance(
      const threevector& v1,const threevector& e_hat,
      const threevector& w1,const threevector& f_hat)
      {
//         cout << "inside geometry_func::line_to_line_squared_distance()" 
//		<< endl;
         threevector term1(v1-w1);
         double delta=e_hat.dot(f_hat);

         double sqrd_dist;
         if (sqr(delta) > 0.99999999) // Two input lines are parallel
         {
            sqrd_dist=term1.dot(term1)-sqr(term1.dot(e_hat));
         }
         else
         {
            double prefactor=-1.0/(1-sqr(delta));
            double alpha=prefactor*term1.dot(e_hat-delta*f_hat);
            double beta=prefactor*term1.dot(delta*e_hat-f_hat);
            double C1=term1.dot(term1);
            sqrd_dist=C1+2*alpha*e_hat.dot(term1)-2*beta*f_hat.dot(term1)
               +sqr(alpha)+sqr(beta)-2*alpha*beta*delta;
         }
         
//         cout << "sqrd_dist = " << sqrd_dist << endl;
//         double dist=sqrt(sqrd_dist);
         return sqrd_dist;
      }


// ==========================================================================
// Triangle methods:
// ==========================================================================
   
   double compute_triangle_area(
      const threevector& v1,const threevector& v2,const threevector& v3)
      {
         threevector a=v2-v1;
         threevector b=v3-v1;
         return 0.5*(a.cross(b)).magnitude();
      }

// The COM for a triangle lies at the average of its 3 vertices.  See
// problem #5 on page 43 in section 1.6 of "Computational geometry in
// C" by Joseph O'Rourke.  

   threevector compute_triangle_COM(
      const threevector& v1,const threevector& v2,const threevector& v3)
      {
         double avg_x=(v1.get(0)+v2.get(0)+v3.get(0))/3.0;
         double avg_y=(v1.get(1)+v2.get(1)+v3.get(1))/3.0;
         double avg_z=(v1.get(2)+v2.get(2)+v3.get(2))/3.0;
         return threevector(avg_x,avg_y,avg_z);
      }

// Method SameHalfPlane takes in test points P and Q as well as
// vertices V1 and V2.  The linesegment running from V1 to V2 defines
// a half-plane.  If P and Q both lie within this half-plane, this
// boolean method returns true.

   bool SameHalfPlane(const threevector& P,const threevector& Q,
                      const threevector& V1,const threevector& V2)
      {
/*
         std::cout << "V2-V1 = " << V2-V1 << std::endl;
         std::cout << "P = " << P << " Q = " << Q << std::endl;
         std::cout << "(P-V1).cross(V2-V1) = " 
                   << (P-V1).cross(V2-V1) << std::endl;
         std::cout << "(Q-V1).cross(V2-V1) = " 
                   << (Q-V1).cross(V2-V1) << std::endl;
*/
         return (( (P-V1).cross(V2-V1)).dot(  (Q-V1).cross(V2-V1) ) > 0);
      }

// ---------------------------------------------------------------------
// Method PointInsideTriangle takes in test point P as well as triangle
// vertices V1, V2 and V3.  This method checks whether P lies inside
// the 3 half-planes defined by each of the triangle's sides.  If so,
// this boolean method returns true.  We stole this algorithm from
// http://www.blackpawn.com/texts/pointinpoly/default.html in July
// 2006.

   bool PointInsideTriangle(
      const threevector& P,const threevector& V1,
      const threevector& V2,const threevector& V3)
      {
//         cout << "inside geometry_func::PointInsideTriangle()" << endl;
         
//         cout << "P = " << P
//              << " V1 = " << V1 
//              << " V2 = " << V2
//              << " V3 = " << V3 << endl;
         outputfunc::enter_continue_char();
         if (!SameHalfPlane(P,V1,V2,V3)) return false;
         if (!SameHalfPlane(P,V2,V3,V1)) return false;
         if (!SameHalfPlane(P,V3,V1,V2)) return false;

         return true;
      }

// ---------------------------------------------------------------------
// Method Point_in_Triangle() implements the barycentric technique
// described in
// http://www.blackpawn.com/texts/pointinpoly/default.html.  This
// website claims the barycentric technique is faster than the
// same-side algorithm:
 
   bool Point_in_Triangle(
      const twovector& P,const twovector& A,
      const twovector& B,const twovector& C)
      {
         twovector v0(C - A);
         twovector v1(B - A);
         twovector v2(P - A);

         double dot00 = v0.sqrd_magnitude();
         double dot01 = v0.dot(v1);
         double dot02 = v0.dot(v2);
         double dot11 = v1.sqrd_magnitude();
         double dot12 = v1.dot(v2);

// Compute barycentric coordinates

         double invDenom = 1.0 / (dot00 * dot11 - dot01 * dot01);
         double u = (dot11 * dot02 - dot01 * dot12) * invDenom;
         double v = (dot00 * dot12 - dot01 * dot02) * invDenom;
            
// Check if point is in triangle

         return (u > 0) && (v > 0) && (u + v < 1);
      }

// ---------------------------------------------------------------------
   bool Point_in_Triangle(
      const threevector& P,const threevector& A,
      const threevector& B,const threevector& C)
      {
         threevector v0(C - A);
         threevector v1(B - A);
         threevector v2(P - A);

         double dot00 = v0.sqrd_magnitude();
         double dot01 = v0.dot(v1);
         double dot02 = v0.dot(v2);
         double dot11 = v1.sqrd_magnitude();
         double dot12 = v1.dot(v2);

// Compute barycentric coordinates

         double invDenom = 1.0 / (dot00 * dot11 - dot01 * dot01);
         double u = (dot11 * dot02 - dot01 * dot12) * invDenom;
         double v = (dot00 * dot12 - dot01 * dot02) * invDenom;
            
// Check if point is in triangle

         return (u > 0) && (v > 0) && (u + v < 1);
      }

// ---------------------------------------------------------------------
// Method ray_intersects_Triangle() takes in *triangle_ptr along with
// the basepoint and direction vector for some ray.  If the ray
// pierces the Triangle's interior, this boolean method returns true
// along with the intersection point.  Otherwise, it returns false.

   bool ray_intersects_Triangle(
      polygon* triangle_ptr,const threevector& ray_basept,
      const threevector& r_hat)
   {
      threevector intersection_pt;
      return ray_intersects_Triangle(triangle_ptr,ray_basept,
      r_hat,intersection_pt);
   }

   bool ray_intersects_Triangle(
      polygon* triangle_ptr,const threevector& ray_basept,
      const threevector& r_hat,threevector& intersection_pt)
   {
//      cout << "inside geometry_func:: ray_intersects_Triangle()" << endl;
//      cout << "ray_basept = " << ray_basept << endl;
//      cout << "r_hat = " << r_hat << endl;

      threevector triangle_COM=triangle_ptr->compute_COM();
//      cout << "triangle_COM = " << triangle_COM << endl;
      threevector n_towards_COM=(triangle_COM-ray_basept).unitvector();
//      cout << "n_towards_COM = " << n_towards_COM << endl;
//      double dotproduct=r_hat.dot(n_towards_COM);
//      cout << "angle between r_hat and n_towards_COM = "
//           << acos(dotproduct)*180/PI << endl;

      plane* plane_ptr=triangle_ptr->get_plane_ptr();
//      cout << "*plane_ptr = " << *plane_ptr << endl;
      if (!plane_ptr->ray_intersection(
         ray_basept,r_hat,intersection_pt))
      {
//         cout << "Ray does not intersect Triangle's infinite plane" << endl;
         return false;
      }

//      cout << "triangle = " << *triangle_ptr << endl;
//      cout << "V0 = " 
//           << triangle_ptr->get_vertex(0).get(0) << " , "
//           << triangle_ptr->get_vertex(0).get(1) << " , "
//           << triangle_ptr->get_vertex(0).get(2) << endl;

//      cout << "intersection_pt = " 
//           << intersection_pt.get(0) <<  " , "
//           << intersection_pt.get(1) << " , "
//           << intersection_pt.get(2) << " , "
//           << endl;

//      triangle_ptr->recompute_plane();
//      cout << "triangle plane = " 
//           << *(triangle_ptr->get_plane_ptr()) << endl;
//      outputfunc::enter_continue_char();

//      return PointInsideTriangle(
      return Point_in_Triangle(
         intersection_pt,triangle_ptr->get_vertex(0),
         triangle_ptr->get_vertex(1),triangle_ptr->get_vertex(2));
   }
   
// ==========================================================================
// Contour methods
// ==========================================================================

// Method right_handed_vertex_ordering is based upon Peter Buchak's
// suggestion to consider the sum of a polygon's EXTERIOR angles.  If
// the polygon's vertices are arranged in a right [left]-handed order,
// this sum equals + [-] 2*PI.  This boolean method returns true if
// the contour is right-handed.

   bool right_handed_vertex_ordering(const vector<threevector>& vertex,
                                     const threevector& normal)
      {
         unsigned int nvertices=vertex.size();
         vector<threevector> uhat(nvertices);
         for (unsigned int i=0; i<nvertices; i++)
         {
            threevector curr_u(vertex[modulo(i+1,nvertices)]-vertex[i]);
            uhat[i]=curr_u.unitvector();
         }

         double thetasum=0;
         for (unsigned int i=0; i<nvertices; i++)
         {
            double theta=mathfunc::angle_between_unitvectors(
               uhat[i],uhat[modulo(i+1,nvertices)]);

//            double cos_theta=uhat[i].dot(uhat[modulo(i+1,nvertices)]);
//            threevector crossprod(uhat[i].cross(uhat[modulo(i+1,nvertices)]));
//            double sin_theta_mag=crossprod.magnitude();
//            double sgn_theta=sgn(crossprod.dot(normal));
//            double sin_theta=sgn_theta*sin_theta_mag;
//            double theta=atan2(sin_theta,cos_theta);
//            cout << "theta = " << theta*180/PI << endl;

            thetasum += theta;
         }
//         cout << "thetasum/(2*PI) = " << thetasum/(2*PI) << endl;

         int ratio=basic_math::round(thetasum/(2*PI));
         bool right_handed_vertex_ordering=false;
         if (ratio==1)
         {
            right_handed_vertex_ordering=true;
         }
         else if (ratio==-1)
         {
            right_handed_vertex_ordering=false;
         }
         else
         {
            cout << "Error in geometry_func::right_handed_vertex_ordering()!"
                 << endl;
            cout << "thetasum = " << thetasum*180/PI << " degs" << endl;
            cout << "thetasum/(2*PI) = " << thetasum/(2*PI) << endl;
            cout << "round(thetasum/(2*PI)) = " << ratio << endl;
//            cout << "nvertices = " << nvertices << endl;
//            for (unsigned int i=0; i<nvertices; i++)
//            {
//               cout << "i = " << i << " vertex[i] = " << vertex[i] 
//                    << " uhat[i] = " << uhat[i] << endl;
//            }
//            outputfunc::enter_continue_char();
         }
         return right_handed_vertex_ordering;
      }

// ---------------------------------------------------------------------
// Method remove_short_contour_edges takes in a charactistic length
// scale min_edge_length and orthogonal contour *c_ptr.  It loops over
// each of the contour's edges and checks whether their lengths exceed
// min_edge_length.  If not, the short contour edge's vertices are
// deleted, and the surviving vertex on either the preceding or
// following edge is moved parallel to the direction of the short
// edge.  A new orthogonal contour is dynamically generated, and this
// method recursively calls itself to search for more short edges on
// the smaller contour.  

   void remove_short_contour_edges(double min_edge_length,contour*& c_ptr)
      {
         Linkedlist<threevector>* vertex_list_ptr=new Linkedlist<threevector>;
         for (Mynode<pair<threevector,bool> >* currnode_ptr=
                 c_ptr->get_vertex_list_ptr()->get_start_ptr();
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            vertex_list_ptr->append_node(currnode_ptr->get_data().first);
         }
         if (remove_short_contour_edges(min_edge_length,vertex_list_ptr))
         {
            delete c_ptr;
            c_ptr=new contour(vertex_list_ptr);
         }
         delete vertex_list_ptr;
      }

// This overloaded boolean version of remove_short_contour_edges
// returns true if any point in the contour vertex linked list is
// altered:
   
   bool remove_short_contour_edges(
      double min_edge_length,Linkedlist<threevector>* vertex_list_ptr)
      {
         bool contour_vertex_list_altered_flag=false;
         unsigned int nvertices=vertex_list_ptr->size();
         if (nvertices < 4)
         {
            cout << "Error in geometry_func::remove_short_contour_edges()"
                 << endl;
            cout << "Current vertex list has " << nvertices << " vertices !!" 
                 << endl;
         }
         else if (nvertices==4)
         {

// Rectangles cannot be reduced!

         }
         else
         {

// First ensure that vertex list does not contain repeated points nor
// adjacent parallel edges:

            simplify_orthogonal_contour(vertex_list_ptr);

// Next instantiate local edge array:

            linesegment edge[nvertices];
            for (unsigned int i=0; i<nvertices; i++)
            {
               edge[i]=linesegment(
                  vertex_list_ptr->get_node(i)->get_data(),
                  vertex_list_ptr->get_node(modulo(i+1,nvertices))->
                  get_data());
            }

            const double TINY=1E-6;
            bool short_edge_found=false;
            for (unsigned int i=0; i<nvertices; i++)
            {
               linesegment curr_edge(edge[i]);
               double curr_edge_length=curr_edge.get_length();
               if (curr_edge.get_length() < min_edge_length)
               {
                  short_edge_found=true;
                  double prev_edge_length=edge[modulo(i-1,nvertices)].
                     get_length();

                  if (prev_edge_length > curr_edge_length)
                  {
                     Mynode<threevector>* node_to_move_ptr=
                        vertex_list_ptr->get_node(modulo(i+2,nvertices));
                     node_to_move_ptr->get_data() -=
                        curr_edge_length*curr_edge.get_ehat();

// Check whether vertex i+2 lies on top of vertex i+3.  If so, delete
// vertex i+2:
   
                     Mynode<threevector>* next_node_ptr=
                        vertex_list_ptr->get_node(modulo(i+3,nvertices));
                     threevector delta(node_to_move_ptr->get_data()-
                                    next_node_ptr->get_data());
                     if (delta.magnitude() < TINY)
                     {
                        vertex_list_ptr->delete_node(modulo(i+2,nvertices));
                     }
                  }
                  else
                  {
                     Mynode<threevector>* node_to_move_ptr=
                        vertex_list_ptr->get_node(modulo(i-1,nvertices));
                     node_to_move_ptr->get_data() +=
                        curr_edge_length*curr_edge.get_ehat();

// Check whether vertex i-1 lies on top of vertex i-2.  If so, delete
// vertex i-1:

                     Mynode<threevector>* prev_node_ptr=
                        vertex_list_ptr->get_node(modulo(i-2,nvertices));
                     threevector delta(node_to_move_ptr->get_data()-
                                       prev_node_ptr->get_data());
                     if (delta.magnitude() < TINY)
                     {
                        vertex_list_ptr->delete_node(modulo(i-1,nvertices));
                     }
                  }

                  vertex_list_ptr->delete_node(modulo(i+1,nvertices));
                  vertex_list_ptr->delete_node(i);
                  simplify_orthogonal_contour(vertex_list_ptr);
                  break;         
               }
            } // loop over index i labeling contour edge
         
            if (short_edge_found) 
            {
               contour_vertex_list_altered_flag=true;
               remove_short_contour_edges(min_edge_length,vertex_list_ptr);
            }
            
         } // nvertices > 4 conditional
         return contour_vertex_list_altered_flag;
      }

// ---------------------------------------------------------------------
// Method simplify_orthogonal_contour takes in a pointer to a linked
// list of orthogonal contour vertices.  It eliminates any repeated
// vertices within the linked list.  This method subsequently loops
// over the contour's edges and computes the dotproduct between
// adjacent edges.  If |dotproduct| is close to unity, the adjacent
// edges' common vertex is deleted from the linked list of vertices.
// This method recursively calls itself until no adjacent parallel
// edges are found.

   void simplify_orthogonal_contour(Linkedlist<threevector>*& vertex_list_ptr)
      {
         unsigned int nvertices=vertex_list_ptr->size();
         
         if (nvertices < 4)
         {
            cout << "Error in geometry_func::simplify_orthogonal_contour()"
                 << endl;
            cout << "Current contour has " << nvertices << " vertices !!" 
                 << endl;
         }
         else if (nvertices==4)
         {

// Rectangles cannot be simplified!         

         }
         else
         {
            
// First check whether vertex list contains any point more than once:

            bool repeated_vertex_found=false;
            for (unsigned int i=0; i<nvertices; i++)
            {
               threevector curr_vertex(
                  vertex_list_ptr->get_node(i)->get_data());
               threevector next_vertex(
                  vertex_list_ptr->get_node(modulo(i+1,nvertices))->
                  get_data());
               if (nearly_equal((curr_vertex-next_vertex).magnitude(),0))
               {
//                  cout << "inside geometry_func::simplify_orthogonal_contour()"
//                       << endl;
//                  cout << "repeated contour vertex found" << endl;
//                  cout << "nvertices = " << nvertices 
//                       << " will be reduced by 1" << endl;
                  repeated_vertex_found=true;
                  vertex_list_ptr->delete_node(modulo(i+1,nvertices));
                  break;
               }
            } // loop over index i labeling contour vertices
            if (repeated_vertex_found) simplify_orthogonal_contour(
               vertex_list_ptr);

// Next check whether any adjacent contour edges are parallel:

            linesegment edge[nvertices];
            for (unsigned int i=0; i<nvertices; i++)
            {
               edge[i]=linesegment(
                  vertex_list_ptr->get_node(i)->get_data(),
                  vertex_list_ptr->get_node(modulo(i+1,nvertices))->
                  get_data());
            }

            bool parallel_edges_found=false;
            for (unsigned int i=0; i<nvertices; i++)
            {
               double dotproduct=edge[i].get_ehat().dot(
                  edge[modulo(i+1,nvertices)].get_ehat());
               if (fabs(dotproduct) > 0.99)
               {
//                  cout << "inside geometry_func::simplify_orthogonal_contour()"
//                       << endl;
//                  cout << "parallel adjacent contour edges found" << endl;
//                  cout << "nvertices = " << nvertices 
//                       << " will be reduced by 1" << endl;
                  parallel_edges_found=true;
                  vertex_list_ptr->delete_node(modulo(i+1,nvertices));
                  break;
               }
            } // loop over index i labeling contour edges

            if (parallel_edges_found) simplify_orthogonal_contour(
               vertex_list_ptr);
         } // nvertices > 4 conditional
      }

// ---------------------------------------------------------------------
// Method eliminate_nearly_degenerate_intersection_point_triples was
// written after we discovered that contour edge intersection points
// generated in the process of constructing orthogonal contour
// approximations sometimes can lie nearly on top of each other.  In
// order to avoid orthogonal contour edge intersections resulting from
// such closely spaced intersection points, we scan through each
// intersection point within the input linked list and see how close
// it lies to its previous and next neighbors within the list.  If all
// 3 points lie within min_distance of their center-of-mass, this
// method deletes the previous and next nodes within the list and it
// replaces the position value of the middle node with the COM.  This
// method keeps recursively calling itself until no 3 nodes lie too
// close to each other.

   void eliminate_nearly_degenerate_intersection_point_triples(
      double min_distance,Linkedlist<threevector>*& perim_posn_list_ptr)
      {
         if (perim_posn_list_ptr != NULL)
         {
            bool perim_posn_list_altered_flag=false;
            Mynode<threevector>* prevnode_ptr=NULL;
            Mynode<threevector>* nextnode_ptr=NULL;
            for (Mynode<threevector>* currnode_ptr=perim_posn_list_ptr->
                    get_start_ptr(); currnode_ptr != NULL;
                 currnode_ptr=currnode_ptr->get_nextptr())
            {
               if (currnode_ptr==perim_posn_list_ptr->get_start_ptr())
               {
                  prevnode_ptr=perim_posn_list_ptr->get_stop_ptr();
                  nextnode_ptr=currnode_ptr->get_nextptr();
               }
               else if (currnode_ptr==perim_posn_list_ptr->get_stop_ptr())
               {
                  prevnode_ptr=currnode_ptr->get_prevptr();
                  nextnode_ptr=perim_posn_list_ptr->get_start_ptr();
               }
               else
               {
                  prevnode_ptr=currnode_ptr->get_prevptr();
                  nextnode_ptr=currnode_ptr->get_nextptr();
               }
               
               threevector prev_posn(prevnode_ptr->get_data());
               threevector curr_posn(currnode_ptr->get_data());
               threevector next_posn(nextnode_ptr->get_data());
               threevector COM=(prev_posn+curr_posn+next_posn)/3.0;
               double d_prev=(prev_posn-COM).magnitude();
               double d_curr=(curr_posn-COM).magnitude();
               double d_next=(next_posn-COM).magnitude();

               if (d_prev < min_distance && d_curr < min_distance &&
                   d_next < min_distance)
               {
                  currnode_ptr->set_data(COM);
                  perim_posn_list_ptr->delete_node(prevnode_ptr);
                  perim_posn_list_ptr->delete_node(nextnode_ptr);
                  perim_posn_list_altered_flag=true;

//                  cout << "prev_posn = " << prev_posn << endl;
//                  cout << "curr_posn = " << curr_posn << endl;
//                  cout << "next_posn = " << next_posn << endl;
//                  cout << "These 3 nodes will be replaced by their COM = "
//                       << COM << endl;
                  break;
               }
            } // loop over nodes within perim_posn_list

// Recursively call this method if any nodes were removed from the
// input linked list:

            if (perim_posn_list_altered_flag)
            {
               eliminate_nearly_degenerate_intersection_point_triples(
                  min_distance,perim_posn_list_ptr);
            }
         } // perim_posn_list_ptr != NULl conditional
      }

// ---------------------------------------------------------------------
// Method function
// decompose_orthogonal_concave_contour_into_large_rectangles takes in
// a linked list containing a single orthogonal contour.  It first
// computes the largest rectangle (measured in terms of area) that
// fits inside the interior of the contour.  It uses this rectangle to
// define a "cleaving line" corresponding to one side of the
// rectangle.  This method subsequently decomposes the contour into 2
// or more subcontours which have the cleaving line as their
// boundaries.  It recursively calls itself until all of the
// subcontours are convex.  This method returns a new version of the
// subcontour list containing only rectangles which are "maximally
// large" and which fully partition the original orthogonal contour.

   void decompose_orthogonal_concave_contour_into_large_rectangles(
      Linkedlist<contour*>*& subcontour_list_ptr)
      {
         bool contours_subdivided=false;

// Traverse over list of subcontours.  Decompose each concave
// subcontour into simpler subcontours:

         Linkedlist<contour*>* subsubcontour_list_ptr=
            new Linkedlist<contour*>;
         
         for (Mynode<contour*>* currnode_ptr=subcontour_list_ptr->
                 get_start_ptr(); currnode_ptr != NULL; 
              currnode_ptr=currnode_ptr->get_nextptr())
         {
            contour* curr_contour_ptr=currnode_ptr->get_data();

//            cout << "subcontour_list_ptr->size() = "
//                 << subcontour_list_ptr->size() << endl;
//            cout << "curr_contour = " << *curr_contour_ptr << endl;
//            cout << "curr_contour is convex = " 
//                 << curr_contour_ptr->is_convex() << endl;
            
            if (curr_contour_ptr->is_convex())
            {
               subsubcontour_list_ptr->append_node(new contour(
                  *curr_contour_ptr));
            }
            else
            {
               subsubcontour_list_ptr->concatenate_wo_duplication(
                  curr_contour_ptr->
                  decompose_orthogonal_concave_contour_into_subcontours());
               contours_subdivided=true;
            }
         } // loop over subcontours within linked list
         delete subcontour_list_ptr;
         subcontour_list_ptr=subsubcontour_list_ptr;

// Recursively call this method again if any contour has been subdivided:

         if (contours_subdivided)
            decompose_orthogonal_concave_contour_into_large_rectangles(
               subcontour_list_ptr); 
      }

// ---------------------------------------------------------------------
// Method abutting_contours takes in two contours *c1_ptr and *c2_ptr.
// It performs a brute-force search over all pairs of edges on the two
// contours to see if any two overlap.  If so, the two contours abut
// each other.  This method returns the maximum overlap (measured from
// 0 to 1) as well as the integer indices of the two edges on the two
// contours which maximally overlap.

   double abutting_contours(contour const *c1_ptr,contour const *c2_ptr,
                            int& abutting_edge1,int& abutting_edge2)
      {
         abutting_edge1=abutting_edge2=-1;
         double max_edge_overlap=NEGATIVEINFINITY;
         for (unsigned int i=0; i<c1_ptr->get_nvertices(); i++)
         {
            linesegment edge1(c1_ptr->get_edge(i));
            for (unsigned int j=0; j<c2_ptr->get_nvertices(); j++)
            {
               linesegment edge2(c2_ptr->get_edge(j));
               double curr_edge_overlap=edge1.segment_overlap(edge2);
               if (curr_edge_overlap > max_edge_overlap)
               {
                  max_edge_overlap=curr_edge_overlap;
                  abutting_edge1=i;
                  abutting_edge2=j;
               }
            } // loop over index j labeling *c2_ptr edges
         } // loop over index i labeling *c1_ptr edges
         return max_edge_overlap;
      }

// ---------------------------------------------------------------------
// Method rectangle_contours_bounding_box takes in two rectangular
// contours *c1_ptr and *c2_ptr.  It also takes in integer indices
// which label the edges on these two contours that abut each other.
// This method dynamically generates and returns a rectangular
// bounding box around the pair of abutting rectangles.

   contour* rectangle_contours_bounding_box(
      contour const *c1_ptr,contour const *c2_ptr,
      int abutting_edge1,int abutting_edge2)
      {
         linesegment edge1(c1_ptr->get_edge(modulo(abutting_edge1+2,4)));
         linesegment edge2(c2_ptr->get_edge(modulo(abutting_edge2+2,4)));
//         cout << "edge1 = " << edge1 << endl;
//         cout << "edge2 = " << edge2 << endl;

         threevector e_hat(edge1.get_ehat());
         threevector f_hat(z_hat.cross(e_hat));
//         cout << "e_hat = " << e_hat << " f_hat = " << f_hat << endl;

         double e11=edge1.get_v1().dot(e_hat);
         double e12=edge1.get_v2().dot(e_hat);
         double e21=edge2.get_v1().dot(e_hat);
         double e22=edge2.get_v2().dot(e_hat);
         double min_e=basic_math::min(e11,e12,e21,e22);
         double max_e=basic_math::max(e11,e12,e21,e22);
//         cout << "min_e = " << min_e << " max_e = " << max_e << endl;

         double f11=edge1.get_v1().dot(f_hat);
         double f12=edge1.get_v2().dot(f_hat);
         double f21=edge2.get_v1().dot(f_hat);
         double f22=edge2.get_v2().dot(f_hat);
         double min_f=basic_math::min(f11,f12,f21,f22);
         double max_f=basic_math::max(f11,f12,f21,f22);
//         cout << "min_f = " << min_f << " max_f = " << max_f << endl;

         vector<threevector> vertex(4);
         vertex[0]=min_e*e_hat+min_f*f_hat;
         vertex[1]=max_e*e_hat+min_f*f_hat;
         vertex[2]=max_e*e_hat+max_f*f_hat;
         vertex[3]=min_e*e_hat+max_f*f_hat;
         
         contour* union_contour_ptr=new contour(vertex);
//         cout << "*union_contour_ptr = " << *union_contour_ptr << endl;
         return union_contour_ptr;
      }

// ---------------------------------------------------------------------
// Method consoliate_rectangle_subcontours takes in a linked list of
// subcontour pointers.  It computes abutting fractions for each pair
// of subcontours within the list.  If two contours abut, it next
// checks the ratio of their abutting edges' lengths.  If this ratio
// is large, this method replaces the two subcontours within the
// linked list with their common rectangular bounding box.  This
// method recursively calls itself until no further subcontour
// consolidation needs to be performed.

   void consolidate_rectangle_subcontours(
      Linkedlist<contour*>*& subcontour_list_ptr)
      {
         bool subcontours_consolidated=false;
         for (unsigned int i=0; i<subcontour_list_ptr->size() && 
                 !subcontours_consolidated; i++)
         {
            Mynode<contour*>* node1_ptr=subcontour_list_ptr->get_node(i);
            contour* subcontour1_ptr=node1_ptr->get_data();
            for (unsigned int j=i+1; j<subcontour_list_ptr->size() &&
                    !subcontours_consolidated; j++)
            {
               Mynode<contour*>* node2_ptr=subcontour_list_ptr->get_node(j);
               contour* subcontour2_ptr=node2_ptr->get_data();

               int abutting_edge1,abutting_edge2;
               double overlap=abutting_contours(
                  subcontour1_ptr,subcontour2_ptr,
                  abutting_edge1,abutting_edge2);
//               cout << "Abutting edge overlap frac = " << overlap << endl;

               const double min_abutting_edge_overlap_frac=0.9;
               if (overlap > min_abutting_edge_overlap_frac)
               {

// Next check ratio of abutting edges' lengths:

                  linesegment edge1(subcontour1_ptr->get_edge(
                     abutting_edge1));
                  linesegment edge2(subcontour2_ptr->get_edge(
                     abutting_edge2));
                  double abutting_edge_length_ratio=
                     edge2.get_length()/edge1.get_length();
                  if (edge1.get_length() <= edge2.get_length())
                  {
                     abutting_edge_length_ratio=
                        edge1.get_length()/edge2.get_length();
                  }

                  const double min_abutting_edge_length_ratio=0.80;
//                  cout << "abutting_edge_length_ratio = " 
//                       << abutting_edge_length_ratio << endl;
                  if (abutting_edge_length_ratio > 
                      min_abutting_edge_length_ratio)
                  {
//                     cout << "i = " << i << " subcontour = " 
//                          << *subcontour1_ptr << endl;
//                     cout << "j = " << j << " subcontour = " 
//                          << *subcontour2_ptr << endl;
//                     cout << "abutting_edge1 = " << edge1 << endl;
//                     cout << "abutting_edge2 = " << edge2 << endl;
                     subcontour_list_ptr->delete_node(node2_ptr);
                     subcontour_list_ptr->delete_node(node1_ptr);
                     subcontour_list_ptr->append_node(
                        rectangle_contours_bounding_box(
                           subcontour1_ptr,subcontour2_ptr,
                           abutting_edge1,abutting_edge2));
                     subcontours_consolidated=true;
                  }
               }
            } // loop over index j
         } // loop over index i

// Recursively call this method again if any subcontours have been
// consolidated:

         if (subcontours_consolidated)
            consolidate_rectangle_subcontours(subcontour_list_ptr);
      }

// ---------------------------------------------------------------------
// Method delete_small_rectangle_subcontours takes in a linked list of
// subcontour pointers.  

   void delete_small_rectangle_subcontours(
      contour* contour_ptr,Linkedlist<contour*>*& subcontour_list_ptr)
      {
         const double min_subcontour_area_ratio=0.01;
         double total_contour_area=contour_ptr->
            compute_orthogonal_contour_area();

         Mynode<contour*>* currnode_ptr=subcontour_list_ptr->get_start_ptr();
         while (currnode_ptr != NULL)
         {
            Mynode<contour*>* nextnode_ptr=currnode_ptr->get_nextptr();
            double subcontour_area=currnode_ptr->get_data()->
               compute_orthogonal_contour_area();
//            cout << "subcontour_area = " << subcontour_area << endl;
//            cout << "subcontour_area/total_contour_area = " 
//                 << subcontour_area/total_contour_area << endl;

            if (subcontour_area/total_contour_area < 
                min_subcontour_area_ratio)
            {
               subcontour_list_ptr->delete_node(currnode_ptr);
            }
            currnode_ptr=nextnode_ptr;
         } // while loop
      }

// ==========================================================================
// Contour decomposition methods
// ==========================================================================

// Method decompose_contour_into_subcontours() takes in a set of
// 3D contour vertices.  It systematically cleaves off subcontours
// which have constant height values.  This method returns an STL
// vector of polylines which represent a decomposition of the original
// contour into a set of subcontours with constant heights.

   vector<polyline> decompose_contour_into_subcontours(
      const vector<threevector>& contour_corners)
   {
//      cout << "inside geometry_func::decompose_contour_into_subcontours()" 
//           << endl;

      vector<polyline> cleaved_subcontours;

// First create copy of contour_corners within polyline_corners:

      vector<threevector> polyline_corners;
      for (unsigned int v=0; v<contour_corners.size(); v++)
      {
         polyline_corners.push_back(contour_corners[v]);
      }

      vector<threevector> reduced_contour_corners;

// Number of remaining polyline corners should never go below 3!

      while (polyline_corners.size() > 2)
      {
         polyline curr_polyline=
            cleave_subcontour(polyline_corners,reduced_contour_corners);
         
//         cout << "curr subcontour n_vertices = "
//              << curr_polyline.get_n_vertices() << endl;

// Perform sanity check on current subcontour's area.  Only add
// subcontour to output STL vector if its area is not ridiculously
// small:

         contour curr_contour=contour(curr_polyline);
         double contour_area=curr_contour.compute_area();
//         cout << "contour area = " << contour_area << endl;

         const double min_contour_area=1;	// square meter

         if (contour_area > min_contour_area)
         {
            cleaved_subcontours.push_back(curr_polyline);
         }

         polyline_corners.clear();
         for (unsigned int v=0; v<reduced_contour_corners.size(); v++)
         {
            polyline_corners.push_back(reduced_contour_corners[v]);
         }

//         cout << "cleaved_subcontour " 
//              << cleaved_subcontours.back() << endl;
      }
      return cleaved_subcontours;
   }
   
// ---------------------------------------------------------------------
// Method cleave_subcontour() takes in a set of contour corners.
// Starting with the zeroth corner, it searches backwards and forwards
// for locations where the contour undergoes a height discontinuity.
// This method cleaves off all contour corners within a contiguous
// string with the zeroth which have the same height value.  It also
// attempts to force the cleaved contour edge to align with the
// symmetry direction defined by the subcontour's non-cleaved edges
// modulo 90 degrees.

   polyline cleave_subcontour(
      const vector<threevector>& contour_corners,
      vector<threevector>& reduced_contour_corners)
   {
//      cout << "inside geometry_func::cleave_subcontour()" << endl;
      
      double Z=contour_corners.front().get(2);

      int n_corners=contour_corners.size();
      reduced_contour_corners.clear();

      unsigned int i_start=0;
      bool stop_search_flag=false;
      while (!stop_search_flag)
      {
         i_start--;
         int j=modulo(i_start,n_corners);

         double curr_Z=contour_corners[j].get(2);
         if (!nearly_equal(curr_Z,Z))
         {
            stop_search_flag=true;
            i_start++;
         }
         if (j==0)
         {
            stop_search_flag=true;
         }
      } // while loop
//      cout << "i_start = " << i_start << endl;
      
      if (modulo(i_start,n_corners)==0)
      {
         return polyline(contour_corners);
      }

      unsigned int i_stop=0;
      stop_search_flag=false;
      while (!stop_search_flag)
      {
         i_stop++;
         i_stop=modulo(i_stop,n_corners);

         double curr_Z=contour_corners[i_stop].get(2);
         if (!nearly_equal(curr_Z,Z))
         {
            stop_search_flag=true;
            i_stop--;
         }
      } // while loop
//      cout << "i_stop = " << i_stop << endl;
      
      vector<threevector> subcontour_corners;
      for (unsigned int i=i_start; i<=i_stop; i++)
      {
         int j=modulo(i,n_corners);
         subcontour_corners.push_back(contour_corners[j]);
      }

// Adjust i_start and i_stop subcontour corners so that added edge
// aligns well with edges between i_start and i_stop modulo 90 degs:
      
      contour subcontour(subcontour_corners);

      double avg_theta=subcontour.average_edge_orientation();
      threevector w_hat(cos(avg_theta),sin(avg_theta));

      int n_subcontour_edges=subcontour.get_nedges();
      linesegment last_edge=subcontour.get_edge(n_subcontour_edges-1);
      threevector f_hat=last_edge.max_direction_overlap(w_hat,PI/2);

// Compute angle between last edge's XY direction vector e_hat and
// f_hat. If they disagree by just a few degrees (modulo 90 degs),
// adjust former so that it matches latter:

      double delta_theta=mathfunc::absolute_angle_between_unitvectors(
         last_edge.get_ehat(),f_hat);
      const double max_delta_theta=7*PI/180;
      if (delta_theta < max_delta_theta)
      {
         last_edge.rotate_about_midpoint(f_hat);
      }

      subcontour_corners.clear();
      subcontour_corners.push_back(last_edge.get_v2());
      for (unsigned int i=i_start+1; i<=i_stop-1; i++)
      {
         int j=modulo(i,n_corners);
         subcontour_corners.push_back(contour_corners[j]);
      }
      subcontour_corners.push_back(last_edge.get_v1());

// Adjust i_stop+1 and i_start-1 corners for reduced contour so that
// their edge aligns with that between i_start and i_stop corners:

      for (unsigned int i=i_stop+1; i<i_start+n_corners; i++)
      {
         int j=modulo(i,n_corners);
         reduced_contour_corners.push_back(contour_corners[j]);
      }
      reduced_contour_corners.front().put(0,last_edge.get_v1().get(0));
      reduced_contour_corners.front().put(1,last_edge.get_v1().get(1));

      reduced_contour_corners.back().put(0,last_edge.get_v2().get(0));
      reduced_contour_corners.back().put(1,last_edge.get_v2().get(1));

//      cout << "contour_corners.size() = " << contour_corners.size() << endl;
//      cout << "subcontour_corners.size() = " << subcontour_corners.size()
//           << endl;
//      cout << "reduced_contour_corners.size() = " 
//           << reduced_contour_corners.size() << endl;

      return polyline(subcontour_corners);
   }

// ==========================================================================
// Writing and reading geometrical objects to I/O files:
// ==========================================================================
   
// Method function write_contour_info_to_file

   void write_contour_info_to_file(
      contour const *c_ptr,string contour_filename)
      {
         ofstream binary_outstream;
         filefunc::open_binaryfile(contour_filename,binary_outstream);

         for (Mynode<pair<threevector,bool> > const *currnode_ptr=
                 c_ptr->get_vertex_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; 
              currnode_ptr=currnode_ptr->get_nextptr())
         {
            threevector curr_vertex(currnode_ptr->get_data().first);
            double x=curr_vertex.get(0);
            double y=curr_vertex.get(1);
            double z=curr_vertex.get(2);
            filefunc::writeobject(binary_outstream,x);
            filefunc::writeobject(binary_outstream,y);
            filefunc::writeobject(binary_outstream,z);
         }
         binary_outstream.close();  
      }

// ---------------------------------------------------------------------
   int nvertices_in_contour_file(string contour_filename)
      {
         long long nbytes=0;
         ifstream binary_instream;
         filefunc::open_binaryfile(contour_filename,binary_instream,nbytes);
         binary_instream.close();
         int ndoubles=nbytes/sizeof(double);
         int nvertices=ndoubles/3;
         outputfunc::newline();
         cout << "Number of vertices within input binary contour file = " 
              << nvertices << endl;
         return nvertices;
      }
   
// ---------------------------------------------------------------------
   contour* read_contour_info_from_file(string contour_filename)
      {
         unsigned int nvertices=nvertices_in_contour_file(contour_filename);

         ifstream binary_instream;
         filefunc::open_binaryfile(contour_filename,binary_instream);

         double x,y,z;
         vector<threevector> curr_vertex;
         for (unsigned int i=0; i<nvertices; i++)
         {
            filefunc::readobject(binary_instream,x);
            filefunc::readobject(binary_instream,y);
            filefunc::readobject(binary_instream,z);
            curr_vertex.push_back(threevector(x,y,z));
         }
         binary_instream.close();  

         contour* c_ptr=new contour(curr_vertex);
         return c_ptr;
      }
   
// ==========================================================================
// Planar methods
// ==========================================================================

// Method fit_line_to_multiple_planes takes in an STL vector which
// should contain at least 4 planes (in the future, we'll relax this
// to 2).  Following section 2.2.2 in "Multiple view geometry in
// computer vision" by Hartley and Zisserman, we form the rank-2 mdim
// x 4 matrix Wstar whose rows contain the pi fourvectors for each
// plane.  After performing a singular value decomposition on Wstar,
// we extract the two eigen fourvectors corresponding to the two
// smallest singular values.  The best fit line to the multiple planes
// is given by the linear combination of these two fourvectors.  This
// method instantiates and returns a linesegment object which is
// oriented along the infinite best fit line.
   
   linesegment fit_line_to_multiple_planes(const vector<plane>& P)
      {
         unsigned int mdim=P.size();
         unsigned int ndim=4;

         if (mdim < ndim)
         {
            cout << "Error in geometry_func::fit_line_multiple_planes()"
                 << endl;
            cout << "Currently need to input at least four planes"
                 << endl;
            exit(-1);
         }

         genmatrix Wstar(mdim,ndim);
         for (unsigned int m=0; m<mdim; m++)
         {
            fourvector curr_pi(P[m].get_pi());
            Wstar.put_row(m,curr_pi);
         }
         
         genmatrix U(mdim,ndim),W(ndim,ndim),V(ndim,ndim);
         Wstar.sorted_singular_value_decomposition(U,W,V);
//         Wstar.singular_value_decomposition(U,W,V);

//         cout << "Wstar = " << Wstar << endl;
//         cout << "U = " << U << endl;
//         cout << "W = " << W << endl;
//         cout << "V = " << V << endl;
//         cout << "U * W * Vtrans - Wstar = " 
//              << U*W*V.transpose() -Wstar << endl;

         fourvector alpha,beta;
         V.get_column(2,alpha);
         V.get_column(3,beta);
         
         const double TINY=1.0E-6;
         if (fabs(alpha.get(3)) < TINY) alpha.put(3,1);
         threevector v1(alpha.get(0)/alpha.get(3),alpha.get(1)/alpha.get(3),
                        alpha.get(2)/alpha.get(3));
            
         if (fabs(beta.get(3)) < TINY) beta.put(3,1);
         threevector v2(beta.get(0)/beta.get(3),beta.get(1)/beta.get(3),
                        beta.get(2)/beta.get(3));
         
         return linesegment(v1,v2);
      }

// ---------------------------------------------------------------------
// Method intersection_of_three_planes solves for homogeneous
// fourvector X satisfying pi_0.X = pi_1.X = pi_2.X = 0.  It uses the
// brute-force evaluation of 3x3 submatrix determinants to determine
// X.  This method returns the inhomogeneous 3-vector counterpart to
// X.  See section page 48 in 2.2.1 of "Multiple View Geometry in
// Computer Vision" by Hartley and Zisserman.

   threevector intersection_of_three_planes(
      const plane& P0,const plane& P1,const plane& P2)
      {
         genmatrix M(3,4),Mtrans(4,3);
         M.put_row(0,P0.get_pi());
         M.put_row(1,P1.get_pi());
         M.put_row(2,P2.get_pi());
         Mtrans=M.transpose();

         double det;
         threevector row[3];
         fourvector alpha;
         genmatrix N(3,3);

         Mtrans.get_row(1,row[0]);
         Mtrans.get_row(2,row[1]);
         Mtrans.get_row(3,row[2]);
         N.put_row(0,row[0]);
         N.put_row(1,row[1]);
         N.put_row(2,row[2]);
         N.determinant(det);
         alpha.put(0,det);

         Mtrans.get_row(0,row[0]);
         Mtrans.get_row(2,row[1]);
         Mtrans.get_row(3,row[2]);
         N.put_row(0,row[0]);
         N.put_row(1,row[1]);
         N.put_row(2,row[2]);
         N.determinant(det);
         alpha.put(1,-det);

         Mtrans.get_row(0,row[0]);
         Mtrans.get_row(1,row[1]);
         Mtrans.get_row(3,row[2]);
         N.put_row(0,row[0]);
         N.put_row(1,row[1]);
         N.put_row(2,row[2]);
         N.determinant(det);
         alpha.put(2,det);

         Mtrans.get_row(0,row[0]);
         Mtrans.get_row(1,row[1]);
         Mtrans.get_row(2,row[2]);
         N.put_row(0,row[0]);
         N.put_row(1,row[1]);
         N.put_row(2,row[2]);
         N.determinant(det);
         alpha.put(3,-det);

         const double TINY=1.0E-6;
         if (fabs(alpha.get(3)) < TINY) alpha.put(3,1);
         threevector intersection(
            alpha.get(0)/alpha.get(3),alpha.get(1)/alpha.get(3),
            alpha.get(2)/alpha.get(3));

//         cout << "M*alpha = " << M*alpha << endl;

         return intersection;
      }

// ==========================================================================
// 2D line methods
// ==========================================================================

// Method closest_point_to_origin() takes in a 2D line represented by
// homogeneous threevector l.  It returns the threevector
// corresponding to the point on the line that lies closest to the
// origin.  This formula comes from Algorithm 12.1 in Hartley &
// Zisserman, Multiple view geometry in computer vision, 2nd edition.

   threevector closest_point_to_origin(const threevector& l)
   {
      double lambda=l.get(0);
      double mu=l.get(1);
      double nu=l.get(2);
      threevector closest_point(-lambda*nu,-mu*nu,sqr(lambda)+sqr(mu));
      return closest_point;
   }

// ==========================================================================
// 3D line methods
// ==========================================================================

/*
// Method threeD_line_thru_two_points takes in 3-vectors r1 and r2.
// It returns coefficients (alpha,beta,gamma,delta) within an output
// fourvector which satisfy alpha x + beta y + gamma z + delta = 0.

   fourvector threeD_line_thru_two_points(
      const threevector& r1,const threevector& r2)
   {
      double x1=r1.get(0);
      double x2=r2.get(0);
      double y1=r1.get(1);
      double y2=r2.get(1);
      double z1=r1.get(2);
      double z2=r2.get(2);
         
      double dx=x2-x1;
      double dy=y2-y1;
      double dz=z2-z1;

      double det_xy=x1*y2-y1*x2;
      double det_yz=y1*z2-z1*y2;
      double det_zx=z1*x2-x1*z2;

      double alpha=dy-dz;
      double beta=dz-dx;
      double gamma=dx-dy;
      double delta=-(det_xy+det_yz+det_zx);

      double result1=alpha*x1+beta*y1+gamma*z1+delta;
      double result2=alpha*x2+beta*y2+gamma*z2+delta;
      
      cout << "result1 = " << result1
           << " result2 = " << result2 << endl;

      fourvector line_coeffs(alpha,beta,gamma,delta);
      cout << "line coeffs = " << line_coeffs << endl;
      return line_coeffs;
   }
*/

// ---------------------------------------------------------------------
// Method multi_line_intersection_point() implements the least-squares
// multi-line intersection algorithm described in 

// 	 http://en.wikipedia.org/wiki/Line-line_intersection

// If it successfully computes the 3D point located closest to all the
// input 3D lines, this boolean method returns true.

   bool multi_line_intersection_point(
      const vector<linesegment>& lines,threevector& intersection_point)
      {
//         cout << "inside geometry_func::multi_line_intersection_point()"
//              << endl;

         unsigned int n_lines=lines.size();
//         cout << "n_lines = " << n_lines << endl;

         threevector numer(Zero_vector);
         genmatrix denom(3,3),denom_inverse(3,3);
         denom.clear_values();
         for (unsigned int i=0; i<n_lines; i++)
         {
            threevector V1=lines[i].get_v1();
            threevector ehat=lines[i].get_ehat();
            genmatrix EEtrans(3,3);
            EEtrans=ehat.outerproduct(ehat);
            genmatrix curr_term(3,3);
            curr_term.identity();
            curr_term -= EEtrans;
            denom += curr_term;
            numer += curr_term*V1;
         }

// On 2/8/13, we discovered the hard and painful way that the 3x3
// denom matrix can have nearly zero determinant.  So we explicitly
// evaluate the determinant and return false if it's too small:
         
         double min_abs_determinant=1E-6;
         if (fabs(denom.determinant()) < min_abs_determinant)
         {
            return false;
         }
         
         bool inverse_flag=denom.inverse(denom_inverse);
         if (!inverse_flag)
         {
            cout << "Error in geometry_func::multi_line_intersection_point()"
                 << endl;
            cout << "Could not compute inverse of denominator!" << endl;
            return false;
         }

         intersection_point=denom_inverse*numer;

/*
         if (mathfunc::my_isnan(intersection_point.get(0)) ||
         mathfunc::my_isnan(intersection_point.get(1)) ||
         mathfunc::my_isnan(intersection_point.get(2)))
         {
            cout << "Trouble in geometry_func::multi_line_intersection_point()"
                 << endl;
            cout << "denom = " << denom << endl;
            cout << "numer = " << numer << endl;
            cout << "denom.determinant() = "
                 << denom.determinant() << endl;
            cout << "intersection_point = " << intersection_point
                 << endl;
            outputfunc::enter_continue_char();
            return false;
         }
*/

         return true;
      }

// ---------------------------------------------------------------------
// This overloaded version multi_line_intersection_point() computes
// the least-squares fit for the 3D intersection point corresponding
// to multiple nearly-intersecting lines.  It also propagates errors
// in the lines' direction vectors' polar and azimuthal angles to
// yield an "error ellipsoid" for the intersection point.

   bool multi_line_intersection_point(
      const vector<linesegment>& lines,
      double sigma_theta,double sigma_phi,
      threevector& intersection_point,threevector& sigma_intersection_point)
      {
//         cout << "inside geometry_func::multi_line_intersection_point() #2"
//              << endl;

         unsigned int n_lines=lines.size();
//         cout << "n_lines = " << n_lines << endl;

         threematrix EEtrans,M,dM_dalpha,dM_dbeta;
         threematrix D;
         D.clear_values();

         vector<threematrix> dD_dalpha,dD_dbeta;
         threevector numer(0,0,0);
         
         for (unsigned int i=0; i<n_lines; i++)
         {
            threevector V1=lines[i].get_v1();
            threevector ehat=lines[i].get_ehat();

            double beta,alpha;
            mathfunc::decompose_direction_vector(ehat,beta,alpha);
//            cout << "alpha = " << alpha*180/PI
//                 << " beta = " << beta*180/PI << endl;

            double cos_alpha=cos(alpha);
            double sin_alpha=sin(alpha);
            double cos_beta=cos(beta);
            double sin_beta=sin(beta);

            threevector dehat_dalpha(
               -sin_alpha*cos_beta , -sin_alpha*sin_beta , cos_alpha);
            threevector dehat_dbeta(
               -cos_alpha*sin_beta , cos_alpha*cos_beta , 0);

//            cout << "dehat_dalpha = " << dehat_dalpha << endl;
//            cout << "dehat_dbeta = " << dehat_dbeta << endl;

            EEtrans=ehat.outerproduct(ehat);
            M.identity();
            M -= EEtrans;

            dM_dalpha = dehat_dalpha.outerproduct(-ehat);
            dM_dalpha += ehat.outerproduct(-dehat_dalpha);

            dM_dbeta = dehat_dbeta.outerproduct(-ehat);
            dM_dbeta += ehat.outerproduct(-dehat_dbeta);

            D += M;
            dD_dalpha.push_back(dM_dalpha);
            dD_dbeta.push_back(dM_dbeta);

            numer += M*V1;
         } // loop over index i labeling intersecting lines

//          outputfunc::enter_continue_char();

// On 2/8/13, we discovered the hard and painful way that the 3x3
// denom matrix can have nearly zero determinant.  So we explicitly
// evaluate the determinant and return false if it's too small:
         
         double min_abs_determinant=1E-6;
         if (fabs(D.determinant()) < min_abs_determinant)
         {
            return false;
         }
         
         threematrix Dinv;
         bool inverse_flag=D.inverse(Dinv);
         if (!inverse_flag)
         {
            cout << "Error in geometry_func::multi_line_intersection_point_error()"
                 << endl;
            cout << "Could not compute Dinv!" << endl;
            return false;
         }

         vector<threematrix> dDinv_dalpha,dDinv_dbeta;
         for (unsigned int i=0; i<n_lines; i++)
         {
            threematrix curr_dDinv_dalpha=(-Dinv) * dD_dalpha[i] * Dinv;
            threematrix curr_dDinv_dbeta=(-Dinv) * dD_dbeta[i] * Dinv;
            dDinv_dalpha.push_back(curr_dDinv_dalpha);
            dDinv_dbeta.push_back(curr_dDinv_dbeta);
         }

         intersection_point=Dinv*numer;

         double sigma_alpha=sigma_theta;
         double sigma_beta=sigma_phi;
//         double sigma_alpha=0*PI/180;
//         double sigma_alpha=1*PI/180;
//         double sigma_beta= 1*PI/180;
         threevector sqr_sigma_x(0,0,0);
         for (unsigned int i=0; i<n_lines; i++)
         {
            threevector V1=lines[i].get_v1();
            threevector curr_dx_dalpha=
               dDinv_dalpha[i]*numer + 
               Dinv * dD_dalpha[i]*V1;
            threevector curr_dx_dbeta=
               dDinv_dbeta[i]*numer + 
               Dinv * dD_dbeta[i]*V1;

//            cout << "i = " << i 
//                 << " curr_dx_dalpha = " << curr_dx_dalpha
//                 << " curr_dx_dbeta = " << curr_dx_dbeta
//                 << endl;
            
            sqr_sigma_x += threevector (
               sqr(curr_dx_dalpha.get(0) * sigma_alpha) , 
               sqr(curr_dx_dalpha.get(1) * sigma_alpha) , 
               sqr(curr_dx_dalpha.get(2) * sigma_alpha) );
            sqr_sigma_x += threevector (
               sqr(curr_dx_dbeta.get(0) * sigma_beta) , 
               sqr(curr_dx_dbeta.get(1) * sigma_beta) , 
               sqr(curr_dx_dbeta.get(2) * sigma_beta) );
         }
     
         sigma_intersection_point=threevector(
            sqrt(sqr_sigma_x.get(0)),
            sqrt(sqr_sigma_x.get(1)),
            sqrt(sqr_sigma_x.get(2)));

         return true;
      }

// ==========================================================================
// General Polygon Clipper (GPC) methods
// ==========================================================================
   
   gpc_vertex* generate_GPC_vertex(const threevector& V)
   {
      gpc_vertex* gpc_vertex_ptr=new gpc_vertex();
      gpc_vertex_ptr->x=V.get(0);
      gpc_vertex_ptr->y=V.get(1);
      return gpc_vertex_ptr;
   }

   void destroy_GPC_vertex(gpc_vertex* gpc_vertex_ptr)
   {
      delete gpc_vertex_ptr;
   }

   gpc_vertex_list* generate_GPC_vertex_list(
      const vector<threevector>& vertices)
   {
      gpc_vertex_list* gpc_vertex_list_ptr=new gpc_vertex_list();

      gpc_vertex_list_ptr->num_vertices=vertices.size();
      gpc_vertex_list_ptr->vertex=new gpc_vertex[vertices.size()];

      for (unsigned int n=0; n<vertices.size(); n++)
      {
         gpc_vertex* gpc_vertex_ptr=
            geometry_func::generate_GPC_vertex(vertices[n]);
         gpc_vertex_list_ptr->vertex[n]=*gpc_vertex_ptr;
      }
   
      return gpc_vertex_list_ptr;
   }

   void destroy_GPC_vertex_list(gpc_vertex_list* gpc_vertex_list_ptr)
   {
      for (int n=0; n<gpc_vertex_list_ptr->num_vertices; n++)
      {
         gpc_vertex* curr_gps_vertex_ptr=&(gpc_vertex_list_ptr->vertex[n]);
         geometry_func::destroy_GPC_vertex(curr_gps_vertex_ptr);
      }
      delete gpc_vertex_list_ptr;
   }

   void destroy_GPC_polygon(gpc_polygon* gpc_polygon_ptr)
   {
      delete [] gpc_polygon_ptr->hole;
      geometry_func::destroy_GPC_vertex_list(gpc_polygon_ptr->contour);
      delete gpc_polygon_ptr;
   }

// ---------------------------------------------------------------------
   void print_GPC_vertex(gpc_vertex* gpc_vertex_ptr)
   {
      cout << "x = " << gpc_vertex_ptr->x
           << " y = " << gpc_vertex_ptr->y << endl;
   }

   void print_GPC_vertex_list(gpc_vertex_list* gpc_vertex_list_ptr)
   {
      unsigned int n_vertices=gpc_vertex_list_ptr->num_vertices;
      cout << "num_vertices = " << n_vertices << endl;
      for (unsigned int v=0; v<n_vertices; v++)
      {
         cout << "v = " << v << endl;
         geometry_func::print_GPC_vertex(&(gpc_vertex_list_ptr->vertex[v]));
      } // loop over index v labeling GPC vertices
   }
         
   void print_GPC_polygon(gpc_polygon* gpc_polygon_ptr)
   {
      cout << "num_contours = " << gpc_polygon_ptr->num_contours << endl;
      for (int c=0; c<gpc_polygon_ptr->num_contours; c++)
      {
         cout << "c = " << c
              << " hole[c] = " << gpc_polygon_ptr->hole[c] 
              << endl;
         geometry_func::print_GPC_vertex_list(&(gpc_polygon_ptr->contour[c]));
      } // loop over index c labeling GPC polygon contours
   }

// ---------------------------------------------------------------------   
// Method convert_GPC_to_polygons()

   vector<polygon> convert_GPC_to_polygons(gpc_polygon* gpc_polygon_ptr)
   {
//      cout << "inside geometry_func::convert_GPC_to_polygons()" << endl;
      
      vector<polygon> converted_polygons;
      
      unsigned int n_polys=gpc_polygon_ptr->num_contours; 
      for (unsigned int p=0; p<n_polys; p++)
      {
//         int curr_hole=gpc_polygon_ptr->hole[p];
//         cout << "p = " << p << " curr_hole = " << curr_hole << endl;
         gpc_vertex_list* gpc_vertex_list_ptr=&(gpc_polygon_ptr->contour[p]);

         unsigned int n_vertices=gpc_vertex_list_ptr->num_vertices;
         vector<twovector> vertices,reversed_vertices;

         twovector prev_vertex,prev_reversed_vertex;
         for (unsigned int v=0; v<n_vertices; v++)
         {
            gpc_vertex* gpc_vertex_ptr=&(gpc_vertex_list_ptr->vertex[v]);
            gpc_vertex* reversed_gpc_vertex_ptr=&(gpc_vertex_list_ptr->vertex[
               n_vertices-1-v]);
            twovector curr_vertex(gpc_vertex_ptr->x,gpc_vertex_ptr->y);
            twovector reversed_vertex(
               reversed_gpc_vertex_ptr->x,reversed_gpc_vertex_ptr->y);

            if (v==0)
            {
               vertices.push_back(curr_vertex);
               reversed_vertices.push_back(reversed_vertex);
            }
            if (v > 0 && !curr_vertex.nearly_equal(prev_vertex))
            {
               vertices.push_back(curr_vertex);
            }
            if (v > 0 && !reversed_vertex.nearly_equal(prev_reversed_vertex))
            {
               reversed_vertices.push_back(reversed_vertex);
            }

            prev_vertex=curr_vertex;
            prev_reversed_vertex=reversed_vertex;
         } // loop over index v labeling vertices

         polygon curr_poly(vertices);
         polygon reversed_poly(reversed_vertices);
         if (curr_poly.get_normal().dot(z_hat) < 0)
         {
            converted_polygons.push_back(reversed_poly);            
         }
         else
         {
            converted_polygons.push_back(curr_poly);
         }

//         cout << "p = " << p
//              <<  " converted polygon = " << converted_polygons.back()
//              << endl;
      } // loop over index p labeling polygons

      return converted_polygons;
   }
   
// ---------------------------------------------------------------------
// Method polygon_union() takes in polygons poly1 and poly2.  It calls
// the General Polygon Clipper method which generates the union from
// two input polygons.  The union result (which may contain one or
// two polygons) is returned within an output STL vector.

   vector<polygon> polygon_union(polygon& poly1,polygon& poly2)
   {
      gpc_polygon* gpc_poly1_ptr=poly1.generate_GPC_polygon();
      gpc_polygon* gpc_poly2_ptr=poly2.generate_GPC_polygon();

      gpc_polygon* gpc_polyunion_ptr=new gpc_polygon();
      gpc_polygon_clip(
         GPC_UNION,gpc_poly1_ptr,gpc_poly2_ptr,gpc_polyunion_ptr);

//      cout << "GPC union polygon = " << endl;
//      geometry_func::print_GPC_polygon(gpc_polyunion_ptr);

      gpc_free_polygon(gpc_poly1_ptr);
      gpc_free_polygon(gpc_poly2_ptr);
  
      vector<polygon> union_polygons=convert_GPC_to_polygons(
         gpc_polyunion_ptr);
      
      gpc_free_polygon(gpc_polyunion_ptr);

      return union_polygons;
   }

// ---------------------------------------------------------------------
// This overloaded version of method polygon_union() 

   vector<polygon> polygon_union(polygon& new_poly,vector<polygon>& old_polys)
   {
//      cout << "inside geometry_func::polygon_union()" << endl;
//      cout << "old_polys.size() = " << old_polys.size() << endl;

      vector<polygon> total_union_polygons;
      for (unsigned int p=0; p<old_polys.size(); p++)
      {
         total_union_polygons.push_back(old_polys[p]);
      }
      total_union_polygons.push_back(new_poly);

      if (total_union_polygons.size()==1) return total_union_polygons;
      
      bool change_flag=false;
      int iter=0;
      while (iter==0 || change_flag)
      {
//         cout << "iter = " << iter
//              << " total_union_polygons.size() = "
//              << total_union_polygons.size() << endl;
         
         change_flag=false;
         for (unsigned int p=0; p<total_union_polygons.size() && !change_flag; p++)
         {
            for (unsigned int q=p+1; q<total_union_polygons.size() && !change_flag; q++)
            {
               vector<polygon> intersection_polygons=
                  geometry_func::polygon_intersection(
                     total_union_polygons[p],total_union_polygons[q]);

               if (intersection_polygons.size() > 0)
               {
                  change_flag=true;

                  vector<polygon> union_polygons=polygon_union(
                     total_union_polygons[p],total_union_polygons[q]);
                  if (union_polygons.size() > 1)
                  {
                     cout << "Error !!! union_polygons.size() = "
                          << union_polygons.size() << endl;
                     cout << "total_union_polygons[p].nvertices = "
                          << total_union_polygons[p].get_nvertices() << endl;
                     cout << "total_union_polygons[q].nvertices = "
                          << total_union_polygons[q].get_nvertices() << endl;
                     union_polygons.clear();
                     if (total_union_polygons[p].get_nvertices() > 
                     total_union_polygons[q].get_nvertices())
                     {
                        union_polygons.push_back(total_union_polygons[p]);
                     }
                     else
                     {
                        union_polygons.push_back(total_union_polygons[q]);
                     }
//                     outputfunc::enter_continue_char();
                  }

                  int max_nvertices=-1;
                  vector<polygon> reduced_total_union_polygons;
                  for (unsigned int t=0; t<total_union_polygons.size(); t++)
                  {
                     if (t==p || t==q) continue;
                     reduced_total_union_polygons.push_back(
                        total_union_polygons[t]);
                     max_nvertices=basic_math::max(
		       max_nvertices,
                       int(reduced_total_union_polygons.back().get_nvertices()));
                  }
                  reduced_total_union_polygons.push_back(union_polygons[0]);
                  total_union_polygons.clear();
                  for (unsigned int t=0; t<reduced_total_union_polygons.size(); t++)
                  {
                     if (max_nvertices <= 12 ||
                     (max_nvertices > 12 && reduced_total_union_polygons[t].
                     get_nvertices() > 8))
                     {
                        total_union_polygons.push_back(
                           reduced_total_union_polygons[t]);
//                        cout << "t = " << t
//                             << " total_union_polygons[t].nvertices = "
//                             << total_union_polygons[t].get_nvertices()
//                             << endl;
                     }
                     
                  } // loop over index t labeling reduced total union polys
               }

            } // loop over index q
         } // loop over index p labeling total_union_polygons

         iter++;

      } // while loop
      
//      cout << "At end of geometry_func::polygon_union()" << endl;
//      cout << "total_union_polygons.size() = "
//           << total_union_polygons.size() << endl;

      return total_union_polygons;
   }

// ---------------------------------------------------------------------
// This overloaded version of polygon_union() is a high-level method
// which takes in an STL vector of polygons that are assumed to all
// reside within the same plane.  It returns their union within the
// output STL vector.

   vector<polygon> polygon_union(vector<polygon>& input_polys)
   {
//      cout << "inside geometry_func::polygon_union()" << endl;

      vector<polygon> total_union_polygons;
      for (unsigned int p=0; p<input_polys.size(); p++)
      {
         total_union_polygons=polygon_union(
            input_polys[p],total_union_polygons);
      }
      return total_union_polygons;
   }

// ---------------------------------------------------------------------
   vector<polygon> polygon_intersection(polygon& poly1,polygon& poly2)
   {
      gpc_polygon* gpc_poly1_ptr=poly1.generate_GPC_polygon();
      gpc_polygon* gpc_poly2_ptr=poly2.generate_GPC_polygon();

      gpc_polygon* gpc_polyintersection_ptr=new gpc_polygon();
      gpc_polygon_clip(
         GPC_INT,gpc_poly1_ptr,gpc_poly2_ptr,gpc_polyintersection_ptr);

//      cout << "GPC union polygon = " << endl;
//      geometry_func::print_GPC_polygon(gpc_polyunion_ptr);

      gpc_free_polygon(gpc_poly1_ptr);
      gpc_free_polygon(gpc_poly2_ptr);
  
      vector<polygon> intersection_polygons=convert_GPC_to_polygons(
         gpc_polyintersection_ptr);
      
      gpc_free_polygon(gpc_polyintersection_ptr);

      return intersection_polygons;
   }

// ---------------------------------------------------------------------
// Method polygon_intersection() is a high-level method which takes in
// a set of polygons assumed to lie in the XY plane.  It returns their
// intersection within a single dynamically-generated output polygon.

   polygon* polygon_intersection(vector<polygon>& polys)
   {
//      cout << "inside geometry_func::polygon_intersection()" << endl;
//      cout << "polys.size() = " << polys.size() << endl;

      if (polys.size()==0) 
      {
         return NULL;
      }
      
      if (polys.size()==1) 
      {
         return new polygon(polys[0]);
      }

      polygon* intersection_polygon_ptr=new polygon(polys[0]);
      for (unsigned int p=1; p<polys.size(); p++)
      {
         vector<polygon> output_polys=polygon_intersection(
            *intersection_polygon_ptr,polys[p]);

         delete intersection_polygon_ptr;
         if (output_polys.size()==0)
         {
            return NULL;
         }
         intersection_polygon_ptr=new polygon(output_polys.front());
      }
      return intersection_polygon_ptr;
   }

// ---------------------------------------------------------------------   
   polygon* polygon_intersection(const vector<polygon*>& poly_ptrs)
   {
      vector<polygon> polys;
      for (unsigned int p=0; p<poly_ptrs.size(); p++)
      {
         polys.push_back(*poly_ptrs[p]);
      }
      return polygon_intersection(polys);
   }
   
// ---------------------------------------------------------------------
// Method polygon_difference() returns poly1-poly2 - i.e. the part of
// polygon #1 which does not overlap with polygon #2.  The returned
// result may be the null set.

   vector<polygon> polygon_difference(polygon& poly1,polygon& poly2)
   {
      gpc_polygon* gpc_poly1_ptr=poly1.generate_GPC_polygon();
      gpc_polygon* gpc_poly2_ptr=poly2.generate_GPC_polygon();

      gpc_polygon* gpc_polydifference_ptr=new gpc_polygon();
      gpc_polygon_clip(
         GPC_DIFF,gpc_poly1_ptr,gpc_poly2_ptr,gpc_polydifference_ptr);

//      cout << "GPC union polygon = " << endl;
//      geometry_func::print_GPC_polygon(gpc_polyunion_ptr);

      gpc_free_polygon(gpc_poly1_ptr);
      gpc_free_polygon(gpc_poly2_ptr);
  
      vector<polygon> difference_polygons=convert_GPC_to_polygons(
         gpc_polydifference_ptr);
      
      gpc_free_polygon(gpc_polydifference_ptr);

      return difference_polygons;
   }
   
// ---------------------------------------------------------------------
// This overloaded version of polygon_difference() returns sum_p
// (poly_p - poly2).  

   vector<polygon> polygon_difference(
      vector<polygon>& polys,polygon& poly2)
   {
      vector<polygon> poly_differences;
      for (unsigned int p=0; p<polys.size(); p++)
      {
         vector<polygon> curr_differences=polygon_difference(polys[p],poly2);
         for (unsigned int c=0; c<curr_differences.size(); c++)
         {
            poly_differences.push_back(curr_differences[c]);
         } // loop over index c
      } // loop over index p
      return poly_differences;
   }
   
// ---------------------------------------------------------------------
// This overloaded version of polygon_difference() returns sum_p poly_p 
// - sum_q poly_q.

   vector<polygon> polygon_difference(
      vector<polygon>& polys,vector<polygon>& qolys)
   {
      vector<polygon> poly_copies,poly_differences;

      for (unsigned int p=0; p<polys.size(); p++)
      {
         poly_copies.push_back(polys[p]);
      }

      for (unsigned int q=0; q<qolys.size(); q++)
      {
         vector<polygon> curr_differences=polygon_difference(
            poly_copies,qolys[q]);
         poly_copies.clear();
         for (unsigned int c=0; c<curr_differences.size(); c++)
         {
            poly_copies.push_back(curr_differences[c]);
         }
      } // loop over index q
      return poly_copies;
   }

// ==========================================================================
// Bounding methods
// ==========================================================================

// Method compute_bounding_sphere() takes in a set of threevectors.  It
// returns the center and radius for a bounding sphere which encloses
// all the input points.

   void compute_bounding_sphere(
      const std::vector<threevector>& V,
      threevector& sphere_center,double& sphere_radius)
   {
      sphere_center=Zero_vector;

      unsigned int n_points=V.size();
      for (unsigned int i=0; i<n_points; i++)
      {
         sphere_center += V[i];
      }
      sphere_center /= n_points;
      
      sphere_radius=0;
      for (unsigned int i=0; i<n_points; i++)
      {
         double curr_radius=(V[i]-sphere_center).magnitude();
         sphere_radius=basic_math::max(sphere_radius,curr_radius);
      }
   }
   
// ---------------------------------------------------------------------
// This overloaded version of compute_bounding_sphere() takes in
// *vertices_handler_ptr and returns the bounding sphere surrounding
// all its vertex members.

   void compute_bounding_sphere(
      vertices_handler* vertices_handler_ptr,
      threevector& sphere_center,double& sphere_radius)
   {
      unsigned int n_points=vertices_handler_ptr->get_n_vertices();
      
      vector<threevector> V;
      for (unsigned int n=0; n<n_points; n++)
      {
         V.push_back(vertices_handler_ptr->get_vertex(n).get_posn());
      } // loop over index n
      compute_bounding_sphere(V,sphere_center,sphere_radius);
   }
   
// ---------------------------------------------------------------------
// Method compute_bounding_box() takes in *vertices_handler_ptr &
// returns the bounding box surrounding all its vertex members.
   
   void compute_bounding_box(
      vertices_handler* vertices_handler_ptr,bounding_box& bbox)
   {
      unsigned int n_points=vertices_handler_ptr->get_n_vertices();
      
      vector<threevector> V;
      for (unsigned int n=0; n<n_points; n++)
      {
         V.push_back(vertices_handler_ptr->get_vertex(n).get_posn());
      } // loop over index n

      bbox.recompute_bounding_box(V);
   }

// ---------------------------------------------------------------------
// Method write_bboxes_to_file() exports a set of bounding box pixel
// coordinates and string attributes to an output text file.

void write_bboxes_to_file(ofstream& outstream, 
                          vector<bounding_box>& curr_image_bboxes)
{
   for(unsigned int b = 0; b < curr_image_bboxes.size(); b++)
   {
      bounding_box curr_bbox = curr_image_bboxes[b];
      
      outstream << curr_bbox.get_ID() << "  "
                << curr_bbox.get_label() << "   "
                << curr_bbox.get_xmin() << "  "
                << curr_bbox.get_xmax() << "  "
                << curr_bbox.get_ymin() << "  "
                << curr_bbox.get_ymax() << "  ";
      
      string attr_key, attr_value;
      for(curr_bbox.get_attributes_map_iter() = 
             curr_bbox.get_attributes_map().begin(); 
          curr_bbox.get_attributes_map_iter() != 
             curr_bbox.get_attributes_map().end();
          curr_bbox.get_attributes_map_iter()++)
      {
         attr_key = curr_bbox.get_attributes_map_iter()->first;
         attr_value = curr_bbox.get_attributes_map_iter()->second;
         outstream << attr_key << "  "  << attr_value << "  ";
      }

      outstream << endl;
   } // loop over index b labeling bboxes for current image
}

// ==========================================================================
// Convex hull methods
// ==========================================================================

// Method compute_convex_hull() takes in a set of twovectors within
// STL vector V.  It calls the qhull program and returns the
// convex hull of the input points within a dynamically instantiated
// polygon.

   polygon* compute_convex_hull(const vector<twovector>& V)
   {
//       cout << "inside geometry_func::compute_convex_hull()" << endl;

      string input_filename="/tmp/qhull_input.dat";
      ofstream input_stream;
      filefunc::openfile(input_filename,input_stream);
      input_stream << "2  # d_dims" << endl;
      int n_points=V.size();
      input_stream << n_points << " # n_points" << endl;

      for (unsigned int v=0; v<V.size(); v++)
      {
         input_stream << V[v].get(0) << " " << V[v].get(1) << endl;
      }
      filefunc::closefile(input_filename,input_stream);

      string output_filename="/tmp/qhull_output.dat";
      string unix_cmd="qhull p TI "+input_filename+" TO "+
         output_filename;
      sysfunc::unix_command(unix_cmd);

      filefunc::ReadInfile(output_filename);
      twovector COM(0,0);
      vector<threevector> hull_vertices;
      for (unsigned int l=2; l<filefunc::text_line.size(); l++)
      {
         vector<double> columns=stringfunc::string_to_numbers(
            filefunc::text_line[l]);
         hull_vertices.push_back(threevector(columns[0],columns[1]));
//         cout << "hull_vertex = " << hull_vertices.back() << endl;
         COM += hull_vertices.back();
      } // loop over index l 
      COM /= hull_vertices.size();
//      cout << "COM = " << COM << endl;

      polygon* convexhull_ptr=new polygon(COM,hull_vertices);
//      cout << "convexhull = " << *convexhull_ptr << endl;
      return convexhull_ptr;
   }

// ==========================================================================
// Homogeneous 2-vector methods
// ==========================================================================

// Method homogeneous_line_coords() returns coeffs (a,b,c) for the
// homogeneous line which passes through 2D points UV1 and UV2.

   threevector homogeneous_line_coords(
      const twovector& UV1,const twovector& UV2)
   {
//      cout << "inside geometry_func::homogeneous_line_coords()" << endl;
      
      threevector UVW1(UV1,1);
      threevector UVW2(UV2,1);
      threevector l=UVW1.cross(UVW2);
      return l;
   }

// ---------------------------------------------------------------------
// Method homogeneous_lines_intersection() takes in coeffs (a,b,c) and
// (a',b',c') for two homogeneous 2D lines.  It returns the Euclidean
// point U,V that represents the lines' intersection.

   twovector homogeneous_lines_intersection(
      const threevector& l1,const threevector& l2)
   {
      threevector homogeneous_intersection_point=l1.cross(l2);
      twovector UV=homogeneous_intersection_point;
      UV /= homogeneous_intersection_point.get(2);
      return UV;
   }

// ==========================================================================
// Ellipse methods
// ==========================================================================

// Method point_on_ellipse() takes in quadratic parameters a,b,c for
// an ellipse along with its center coordinates.  If the input point
// lies along the ellipse, this boolean method returns true.

   bool point_on_ellipse(
      double a,double b,double c,double xcenter,double ycenter,
      const twovector& point)
   {
      twovector translated_point=point-twovector(xcenter,ycenter);

      genmatrix M(2,2);
      M.put(0,0,a);
      M.put(0,1,b);
      M.put(1,0,b);
      M.put(1,1,c);
//      cout << "M = " << M << endl;

      return (nearly_equal(translated_point.dot(M*translated_point),1));
   }

   bool point_inside_ellipse(
      double a,double b,double c,double xcenter,double ycenter,
      const twovector& point)
   {
      twovector translated_point=point-twovector(xcenter,ycenter);

      genmatrix M(2,2);
      M.put(0,0,a);
      M.put(0,1,b);
      M.put(1,0,b);
      M.put(1,1,c);
//      cout << "M = " << M << endl;

      return ((translated_point.dot(M*translated_point),1) < 1);
   }

// ---------------------------------------------------------------------
// Method quadratic_form_to_ellipse_params() takes in coefficients
// a,b,c which appear in the quadratic part of the general equation
// for an ellipse:

// 			 a X**2 + 2 b X Y + c Y**2 = 1

// If the input coefficients do NOT define an ellipse, this boolean
// method returns fals.e Otherwise, it returns the ellipse's major and
// minor axis lengths along with the angle phi which the major axis
// makes relative to +x_hat.

   bool quadratic_form_to_ellipse_params(
      double a,double b,double c,
      double& major_axis,double& minor_axis,double& major_axis_phi)
   {
      genmatrix M(2,2);

      M.put(0,0,a);
      M.put(0,1,b);
      M.put(1,0,b);
      M.put(1,1,c);
//      cout << "M = " << M << endl;

// Compute eigenvalues of matrix M:

      double sqrt_term=sqrt(sqr(a-c)+4*b*b);
      double lambda_plus=0.5*(a+c+sqrt_term);
      double lambda_minus=0.5*(a+c-sqrt_term);
//      cout << "lambda_plus = " << lambda_plus << endl;
//      cout << "lambda_minus = " << lambda_minus << endl;
//      cout << endl;

      if (lambda_plus <= 0 || lambda_minus <= 0) 
      {
         return false;
      }

/*
// Diagonal matrix Lambda holds ellipse eigenvalues lambda_plus and
// lambda_minus:

      genmatrix Lambda(2,2);
      Lambda.clear_values();
      Lambda.put(0,0,lambda_plus);
      Lambda.put(1,1,lambda_minus);
      cout << "Lambda = " << Lambda << endl;
*/
   
// Compute ellipse symmetry direction vectors eplus and eminus:

      double xi=(a-c)+sqrt_term;
      twovector eplus(xi,2*b);
      twovector eminus(2*b,-xi);
      eplus=eplus.unitvector();
      eminus=eminus.unitvector();

/*
// Rotation matrix R holds eigenvectors eplus and eminus.  Note:
// R=Rtrans and R*Rtrans=1:

      genmatrix R(2,2);
      R.put_column(0,eplus);
      R.put_column(1,eminus);
      cout << "R = " << R << endl;
      cout << "R*Rtrans = " << R*R.transpose() << endl;

// Note: M = Rtrans * Lambda * R:

      cout << "Rtrans*Lambda*R = " << R.transpose()*Lambda*R << endl;
      cout << "M - Rtrans*Lambda*R = " << M-R.transpose()*Lambda*R << endl;
*/

// Compute lengths of major and minor axes:

      double sigma_plus=1/sqrt(lambda_plus);
      double sigma_minus=1/sqrt(lambda_minus);
//   cout << "sigma_plus = " << sigma_plus << endl;
//   cout << "sigma_minus = " << sigma_minus << endl;
//   cout << endl;

      double phi;
      if (lambda_plus > lambda_minus)
      {
         phi=atan2(eplus.get(1),eplus.get(0));
         minor_axis=sigma_plus;
         major_axis=sigma_minus;
      }
      else
      {
         phi=atan2(eplus.get(0),eplus.get(1));
         minor_axis=sigma_minus;
         major_axis=sigma_plus;
      }
//      cout << "minor_axis = " << minor_axis << endl;
//      cout << "major_axis = " << major_axis << endl;
//      cout << endl;

      major_axis_phi=phi+PI/2;

/*
      twovector minor_point(minor_axis*cos(phi),minor_axis*sin(phi));
      twovector major_point(major_axis*cos(phi+PI/2),major_axis*sin(phi+PI/2));

      cout << "major_axis_phi = " << major_axis_phi*180/PI << endl;
      cout << "major_point = " << major_point << endl;
      cout << "major_point.mag = " << major_point.magnitude() << endl;
      cout << "minor_point = " << minor_point << endl;
      cout << "minor_point.mag = " << minor_point.magnitude() << endl;
      cout << endl;
   
      cout << "major_point.transpose()*M*major_point = "
           << major_point.dot(M*major_point) << endl;
      cout << "minor_point.transpose()*M*minor_point = "
           << minor_point.dot(M*minor_point) << endl;
      cout << endl;
*/

      return true;
   }
   
// ---------------------------------------------------------------------
// Method ellipse_points() takes in quadratic parameters a,b,c for
// an ellipse along with its center coordinates.  It fills and returns 
// an STL vector containing the specified number of points lying along
// the ellipse.

   vector<twovector> ellipse_points(
      int n_points,double a,double b,double c,double xcenter,double ycenter)
   {
      vector<twovector> ellipse_points;

      double major_axis,minor_axis,major_axis_phi;
      if (geometry_func::quadratic_form_to_ellipse_params(
             a,b,c,major_axis,minor_axis,major_axis_phi))
      {
         unsigned int n_theta_bins=n_points;
         double theta_start=0;
         double theta_stop=360;
         double dtheta=(theta_stop-theta_start)/(n_theta_bins);

         for (unsigned int t=0; t<n_theta_bins; t++)
         {
            double theta=theta_start+t*dtheta;
            theta *= PI/180;

            double r=major_axis*minor_axis/
               sqrt( sqr(minor_axis*cos(theta)) + sqr(major_axis*sin(theta)) );
            twovector ellipse_point=
               twovector(
                  xcenter+r*cos(theta+major_axis_phi),
                  ycenter+r*sin(theta+major_axis_phi));
            ellipse_points.push_back(ellipse_point);
         }
      } // input params correspond to genuine ellipse conditional
      
      return ellipse_points;
   }


   

} // geometry_func namespace


