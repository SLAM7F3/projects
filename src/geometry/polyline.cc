// ==========================================================================
// Polyline class member function definitions
// ==========================================================================
// Last modified on 2/9/11; 11/7/11; 6/28/12; 4/4/14
// ==========================================================================

#include "math/basic_math.h"
#include "math/constant_vectors.h"
#include "geometry/geometry_funcs.h"
#include "kdtree/kdtreefuncs.h"
#include "geometry/polyline.h"

#include "geometry/linesegment.h"
#include "templates/mytemplates.h"

using std::cout;
using std::endl;
using std::ostream;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void polyline::allocate_member_objects()
{
}		       

void polyline::initialize_member_objects()
{
   total_length=0;
   regular_vertex_spacing=10;	// meters

// For speed purposes, we do NOT dynamically allocate the edge array
// unless we specifically know that we're going to use it.  Instead,
// we simply set the edge array's pointer to NULL:

   edge.clear();

   kdtree_ptr=NULL;
}

void polyline::assign_vertex_array_pointers(int num_of_vertices)
{
   n_vertices=num_of_vertices;

//   if (n_vertices <= 2)
//   {
//      cout << "Error inside polyline constructor!" << endl;
//      cout << "n_vertices = " << n_vertices << endl;
//   }
   if (num_of_vertices < MAX_STATIC_VERTICES)
   {
      vertex=vertex_stack;
      vertex_heap_ptr=NULL;
   }
   else
   {
      vertex_heap_ptr=new threevector[num_of_vertices];
      vertex=vertex_heap_ptr;
   }
}

polyline::polyline(void)
{
   allocate_member_objects();
   initialize_member_objects();
   assign_vertex_array_pointers(0);
}

polyline::polyline(const vector<threevector>& currvertex)
{
   allocate_member_objects();
   initialize_member_objects();
   assign_vertex_array_pointers(currvertex.size());

   for (unsigned int i=0; i<n_vertices; i++)
   {
      vertex[i]=currvertex[i];
   }
   compute_total_length();
}

// This next specialized constructor takes in threevector l=(a,b,c)
// which corresponds to the line aU + bV + c = 0 in homogeneous 3D
// space.  It generates a finite linesegment which hopefully
// reasonably represents the infinite line for most computer vision
// applications.

polyline::polyline(const threevector& l)
{
   allocate_member_objects();
   initialize_member_objects();
   assign_vertex_array_pointers(2);

   double a=l.get(0);
   double b=l.get(1);
   double c=l.get(2);

   threevector endpoint1(-100,(100*a-c)/b,0);
   threevector endpoint2(100,(-100*a-c)/b,0);

   vertex[0]=endpoint1;
   vertex[1]=endpoint2;

   compute_total_length();
}

// Copy constructor:

polyline::polyline(const polyline& p)
{
   allocate_member_objects();
   initialize_member_objects();
   assign_vertex_array_pointers(p.n_vertices);
   docopy(p);
}

polyline::~polyline()
{
   if (vertex_heap_ptr != NULL) delete [] vertex_heap_ptr;
}

// ---------------------------------------------------------------------
void polyline::docopy(const polyline& p)
{
   assign_vertex_array_pointers(p.n_vertices);
   for (unsigned int i=0; i<n_vertices; i++)
   {
      vertex[i]=p.vertex[i];
   }
}

// Overload = operator:

polyline& polyline::operator= (const polyline& p)
{
   if (this==&p) return *this;
   docopy(p);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const polyline& p)
{
   outstream << endl;
   outstream << "n_vertices = " << p.n_vertices << endl;
   outstream << "n_edges = " << p.get_n_edges() << endl;
   outstream << "total_length = " << p.get_total_length() << endl;
   if (p.n_vertices >= 2)
   {
      for (unsigned int i=0; i<p.n_vertices; i++)
      {
         outstream << "Polyline vertex " << i 
                   << " x = " << p.vertex[i].get(0)
                   << " y = " << p.vertex[i].get(1) 
                   << " z = " << p.vertex[i].get(2)
                   << endl;
      }
   }
   else
   {
      outstream << "Polyline is degenerate" << endl;
   }
   return(outstream);
}

// =====================================================================
// Polyline manipulation member functions
// =====================================================================

// Member function reset_vertex_altitudes() resets all of the current
// polyline vertices' z-values to input parameter
// constant_vertices_altitude.

void polyline::reset_vertex_altitudes(double constant_vertices_altitude)
{   
//   cout << "inside polyline::reset_vertex_altitudes()" << endl;
   
   for (unsigned int v=0; v<get_n_vertices(); v++)
   {
      vertex[v].put(2,constant_vertices_altitude);
   }
   edge.clear();
   initialize_edge_segments();
}

// =====================================================================
// Intrinsic polyline properties
// =====================================================================

// Member function compute_total_length sets member variable
// total_length equal to the sum of the current polygon's edge
// lengths.

double polyline::compute_total_length()
{
   if (nearly_equal(total_length,0))
   {
      initialize_edge_segments();
      for (unsigned int i=0; i<edge.size(); i++) total_length += edge[i].get_length();
   }
   return total_length;
}

void polyline::compute_first_edge_2D_line_coeffs()
{
//   cout << "inside polyline::compute_first_edge_2D_line_coeffs()" << endl;
   
   initialize_edge_segments();
   first_edge_2D_line_coeffs=edge[0].compute_2D_line_coeffs();
   for (unsigned int i=0; i<first_edge_2D_line_coeffs.size(); i++)
   {
//      cout << "i = " << i << " coeff = " << first_edge_2D_line_coeffs[i]
//           << endl;
   }
}

threevector polyline::compute_vertices_COM() const
{
//   cout << "inside polyline::compute_vertices_COM()" << endl;

   threevector COM=Zero_vector;
   for (unsigned int i=0; i<get_n_vertices(); i++)
   {
      COM += get_vertex(i);
   }
   COM /= get_n_vertices();
   return COM;
}

// =====================================================================
// Distance to point member functions
// =====================================================================

// Member function min_distance_to_point performs a brute-force search
// over each edge linesegment for the polyline point which lies
// closest to the input threevector currpoint.  It returns the results
// within output closest_point_on_polyline along with the closest
// edge's direction vector within e_hat.

double polyline::min_distance_to_point(const threevector& currpoint) 
{
   threevector closest_point_on_polyline,e_hat;
   return min_distance_to_point(currpoint,closest_point_on_polyline,e_hat);
}

double polyline::min_distance_to_point(
   const threevector& currpoint,threevector& closest_point_on_polyline) 
{
   threevector e_hat;
   return min_distance_to_point(currpoint,closest_point_on_polyline,
                                e_hat);
}

double polyline::min_distance_to_point(
   const threevector& currpoint,threevector& closest_point_on_polyline,
   threevector& e_hat) 
{
//   cout << "inside polyline::min_dist_to_point()" << endl;
   initialize_edge_segments();

   double min_dist=POSITIVEINFINITY;
   threevector closest_point_on_segment;
   for (unsigned int l=0; l<edge.size(); l++)
   {
      double curr_min_dist=edge[l].point_to_line_segment_distance(
         currpoint,closest_point_on_segment);
      if (curr_min_dist < min_dist)
      {
         min_dist=curr_min_dist;
         closest_point_on_polyline=closest_point_on_segment;
         e_hat=edge[l].get_ehat();
      }
   } // loop over index l labeling edges within polyline object
   return min_dist;
}

// ---------------------------------------------------------------------
// Member function min_sqrd_distance_to_point takes in threevectors V1
// and V2 which are assumed to define an edge segment along the
// current PolyLine.  This method returns the squared impact parameter
// distance of input threevector currpoint to the edge segment.  We
// wrote this little method to be as efficient as possible.

double polyline::min_sqrd_distance_to_point(
   const threevector& currpoint,const threevector& V1,
   const threevector& V2)
{
   threevector e_hat( (V2-V1).unitvector() );
   threevector Delta(currpoint-V1);
   return Delta.sqrd_magnitude()-sqr(Delta.dot(e_hat));
}

double polyline::min_sqrd_XY_distance_to_point(
   const threevector& currpoint,const threevector& V1,
   const threevector& V2)
{
   twovector e_hat( (twovector(V2-V1)).unitvector() );
   twovector Delta(currpoint-V1);
   return Delta.sqrd_magnitude()-sqr(Delta.dot(e_hat));
}

// ---------------------------------------------------------------------
// Member function min_sqrd_XY_distance_to_point takes in external
// threevector currpoint along with an approximate desired range to
// the currently polyline.  It first performs a KDtree search for
// regularly spaced polyline vertices to the input external
// threevector.  This method subsequently performs a brute-force
// search only over the small number of returned nodes for the minimal
// squared distance to the external threevector.

double polyline::min_sqrd_XY_distance_to_point(
   const threevector& currpoint,double approx_range_to_polyline)
{
//   cout << "inside polyline::min_sqrd_XY_distance_to_point()" << endl;

   unsigned int n_closest_nodes=3;
   fourvector xyz_posn(currpoint);
   vector<fourvector> closest_nodes;
   
   if (kdtree_ptr==NULL)
   {
      cout << "Error in polyline::min_sqrd_distance_to_point()"
           << endl;
      cout << "kdtree_ptr = NULL ! " << endl;
      exit(-1);
   }

   int n_max_search_iters=2;
   int __K=2;
   kdtreefunc::find_closest_nodes(
      kdtree_ptr,xyz_posn,approx_range_to_polyline,
      n_closest_nodes,closest_nodes,n_max_search_iters,__K);

   double closest_sqrd_dist_to_point=POSITIVEINFINITY;
   for (unsigned int i=0; i<closest_nodes.size(); i++)
   {
      cout << "i = " << i
           << " closest_node = " << closest_nodes[i] << endl;

      threevector V1(closest_nodes[i]);
      threevector V2(sampled_edge_points[closest_nodes[i].get(3)+1]);
      closest_sqrd_dist_to_point=basic_math::min(
         closest_sqrd_dist_to_point,
         min_sqrd_XY_distance_to_point(currpoint,V1,V2));
      cout << "V1 = " << V1 << endl;
      cout << "V2 = " << V2 << endl;
      cout << "curr_point = " << currpoint << endl;
   }   
   cout << "sqrt(closest_sqrd_dist_to_point) = "
        << sqrt(closest_sqrd_dist_to_point) << endl;
   
   return closest_sqrd_dist_to_point;
}

// ---------------------------------------------------------------------
// Member function min_sqrd_distance_to_ray takes in the basepoint and
// direction vector for an input ray which we assume is generally
// aligned with the Z-axis (i.e. for point or line picking within the
// XY plane, the LOS from the camera basically points along -Zhat).
// This method first performs a KDtree search for the polyline
// vertices closest to the ray's grid intercept position.
// Approximating each edge associated with these vertices as an
// infinite line, we next compute the 3D impact parameter for the
// input ray with the edges.  The squared length of the smallest 3D
// impact parameter is returned by this method.

double polyline::min_sqrd_distance_to_ray(
   const threevector& grid_intercept_posn,
   const threevector& ray_basepoint,const threevector& ray_ehat,
   double approx_range_to_polyline)
{
//   cout << "inside polyline::min_sqrd_distance_to_ray()" << endl;
   
   if (kdtree_ptr==NULL)
   {
      generate_sampled_edge_points_kdtree();
//      cout << "Error in polyline::min_sqrd_distance_to_ray()"
//          << endl;
//      cout << "kdtree_ptr = NULL ! " << endl;
//      exit(-1);
   }
   fourvector xyz_posn(grid_intercept_posn);
   unsigned int n_closest_nodes=5;
   vector<fourvector> closest_nodes;
   int n_max_search_iters=5;
   //   int n_max_search_iters=10;
   int __K=2;
   kdtreefunc::find_closest_nodes(
      kdtree_ptr,xyz_posn,approx_range_to_polyline,
      n_closest_nodes,closest_nodes,n_max_search_iters,__K);
//   cout << "closest_nodes.size() = " << closest_nodes.size() << endl;

   double closest_sqrd_dist_to_point=POSITIVEINFINITY;
   for (unsigned int i=0; i<closest_nodes.size(); i++)
   {
//      cout << "i = " << i
//           << " closest_node = " << closest_nodes[i] << endl;

      threevector V1(closest_nodes[i]);
      threevector V2(sampled_edge_points[closest_nodes[i].get(3)+1]);
      threevector e_hat((V1-V2).unitvector());
      closest_sqrd_dist_to_point=basic_math::min(
         closest_sqrd_dist_to_point,
         geometry_func::line_to_line_squared_distance(
            V1,e_hat,ray_basepoint,ray_ehat));
//      cout << "V1 = " << V1 << endl;
//      cout << "V2 = " << V2 << endl;
//      cout << "ray_basepoint = " << ray_basepoint << endl;
   }   
//   cout << "sqrt(closest_sqrd_dist_to_point) = "
//        << sqrt(closest_sqrd_dist_to_point) << endl;
   
   return closest_sqrd_dist_to_point;
}

// ==========================================================================
// Multi-Polyline intersection methods:
// ==========================================================================

// Member function intersection_points_with_another_polyline performs
// a brute-force search over each edge segment within the current
// polyline to see whether it intersects any edge segment of input
// polyline p.  If so, the intersection point is stored within the STL
// vector which is returned by this method.

vector<threevector> polyline::intersection_points_with_another_polyline(
   polyline& p,double max_dotproduct,double endpoint_distance_threshold)
{
//   cout << "inside polyline::intersection_points_with_another_polyline()"
//        << endl;
   initialize_edge_segments();
   p.initialize_edge_segments();
   
   bool intersection_pnt_on_this_segment,intersection_pnt_on_that_segment;
   threevector curr_intersection_pnt;
   vector<threevector> intersection_points;

   for (unsigned int i=0; i<get_n_edges(); i++)
   {
      linesegment this_segment(get_edge(i));
//      cout << "i = " << i << " this_segment = " << this_segment << endl;
      for (unsigned int j=0; j<p.get_n_edges(); j++)
      {
         linesegment that_segment(p.get_edge(j));
//         cout << "j = " << j << " that_segment = " << that_segment << endl;
         this_segment.point_of_intersection(
            that_segment,curr_intersection_pnt,
            intersection_pnt_on_this_segment,
            intersection_pnt_on_that_segment,endpoint_distance_threshold);
         if (intersection_pnt_on_this_segment &&
             intersection_pnt_on_that_segment)
         {

// Compute dotproduct between candidate intersecting linesegments:

            double dotproduct=
               this_segment.get_ehat().dot(that_segment.get_ehat());
//            cout << "dotproduct = " << dotproduct
//                 << " max_dotproduct = " << max_dotproduct << endl;
            if (fabs(dotproduct) < max_dotproduct)
            {
               intersection_points.push_back(curr_intersection_pnt);
            }
            
         }
      } // loop over index j labeling polyline p's edge segments
   } // loop over index i labeling current polyline's edge segments
   
//   cout << "intesection points = " << endl;
//   templatefunc::printVector(intersection_points);
//   cout << "At end of polyline::intersection_points_with_another_polyline()"
//        << endl;
   return intersection_points;
}

// =====================================================================
// Polyline edge traversing methods:
// =====================================================================

// For speed purposes, we do NOT include the following edge
// initialization within the overall initialize_member_objects()
// member function:

void polyline::initialize_edge_segments()
{
   if (edge.size()==0)
   {
      
// Unlike closed polygons, number of edges for open polylines =
// n_vertices - 1 !

      for (unsigned int i=0; i<n_vertices-1; i++) 
      {
         edge.push_back(linesegment(vertex[i],vertex[i+1]));
      }
   } // edge.size==0 conditional
}

// ---------------------------------------------------------------------
// Member function edge_number takes in fraction 0 <= frac <= 1 and
// returns the number of the edge on which the point whose length from
// vertex[0] equals frac*total_length.  This method returns -1 if it
// encounters an error condition.

int polyline::edge_number(double frac)
{
//  cout << "inside polyline::edge_number()" << endl;
//  cout << "frac = " << frac << endl;
//  cout << "get_n_edges() = " << get_n_edges() << endl;

  if (frac < 0 || frac > 1.0)
   {
      cout << "Error in polyline::edge_number!" << endl;
      cout << "frac = " << frac << endl;
      outputfunc::enter_continue_char();
      return -1;
   }
  //  if (get_n_edges()==0)
  //  {
  //    cout << "Error in polyline::edge_number()!" << endl;
  //    cout << "n_edges = 0 !!!" << endl;
  //    return -1;
  //  }
  
   compute_total_length();
    
   bool segment_found=false;
   unsigned int segment_number=0;
   double running_length=0;
   do
   {
//      cout << "segment_number = " << segment_number << endl;
//      cout << "frac*total_length = " << frac*total_length
//           << " running_length = " << running_length << endl;
     
      if (frac*total_length >= running_length &&
          frac*total_length <= running_length + 
          edge[segment_number].get_length())
      {
         segment_found=true;
      }
      else
      {
         running_length += edge[segment_number].get_length();
         segment_number++;
      }
   }
   while(!segment_found && segment_number < edge.size());

// Perform sanity check on segment_number.  Do not allow its value to
// exceed get_n_edges()-1!

   segment_number=basic_math::min(get_n_edges()-1,segment_number);
//   cout << "Final segment_number = " << segment_number << endl;

   return segment_number;
}

// ---------------------------------------------------------------------
// Member function edge_point takes in fraction 0 <= frac <= 1 and
// returns the corresponding point along the polyline's total_length whose
// length from vertex[0] equals frac*total_length:

threevector polyline::edge_point(double frac) 
{
//   cout << "inside polyline::edge_point(), frac = " << frac << endl;

   initialize_edge_segments();

   unsigned int segment_number=edge_number(frac);
//   cout << "segment_number = " << segment_number << endl;
   double running_length=0;
   for (unsigned int i=0; i<segment_number; i++) 
   {
      running_length += edge[i].get_length();
   }

// Next compute residual fractional length of desired point on its
// edge segment:

   double curr_edge_frac=(frac*total_length-running_length)/
      edge[segment_number].get_length();
   threevector curr_point=edge[segment_number].get_v1()+
      curr_edge_frac*edge[segment_number].get_length()*edge[segment_number].
      get_ehat();

//   cout << "curr_point = " << curr_point << endl;
   return curr_point;
}

// ---------------------------------------------------------------------
// Member function edge_point_at_distance_along_polyline() takes in
// arclength s and computes its corresponding fractional distance
// frac.  After clamping frac to lie within the interval [0,1], this
// method returns the point along the polyline corresponding to s.

threevector polyline::edge_point_at_distance_along_polyline(double s)
{
//   cout << "inside polyline::edge_point_at_distance_along_polyline(), s = "
//        << s << endl;
   compute_total_length();
//   cout << "total_length = " << total_length << endl;
   double frac=s/total_length;
   frac=basic_math::max(frac,0.000001);
   frac=basic_math::min(frac,0.999999);
//   cout << "frac = " << frac << endl;
   return edge_point(frac);
}

// ---------------------------------------------------------------------
// Member function edge_direction returns the polyline's unit
// direction vector at the point along the polyline where length =
// frac*total_length

threevector polyline::edge_direction(double frac) 
{
//   cout << "inside polyline::edge_direction, frac = " << frac << endl;
//   const double delta=0.001;
   const double delta=0.0001;

   threevector next,prev;
   if (frac < delta)
   {
      next=edge_point(frac+delta);
      prev=edge_point(frac);
   }
   else if (frac > 1-delta)
   {
      next=edge_point(frac);
      prev=edge_point(frac-delta);
   }
   else
   {
      next=edge_point(frac+delta);
      prev=edge_point(frac-delta);
   }
//   cout << "next = " << next << endl;
//   cout << "prev = " << prev << endl;
//   cout << "(next-prev).unitvector() = " << (next-prev).unitvector()
//        << endl;
   return (next-prev).unitvector();
}

// ---------------------------------------------------------------------
// Member function find_edge_containing_point loops over every edge
// within the polyline and tests whether the input point lies along
// it.  If so, this method returns the edge index.  Otherwise, it
// returns -1.

int polyline::find_edge_containing_point(const threevector& currpoint) const
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
// Member function frac_distance_along_polyline returns -1 if the
// input currpoint does not lie along the polyline.  Otherwise, it
// returns currpoint's fractional distance starting from the zeroth
// vertex.

double polyline::frac_distance_along_polyline(const threevector& currpoint) 
{
//   cout << "inside polyline::frac_distance_along_polyline()" << endl;

   int l_pnt=find_edge_containing_point(currpoint);
   if (l_pnt < 0) return -1;
   
   compute_total_length();

   double distance_to_point=0;
   for (int l=0; l<l_pnt; l++)
   {
      distance_to_point += edge[l].get_length();
   }
   
   double f_final_edge=edge[l_pnt].frac_distance_along_segment(currpoint);
   distance_to_point += f_final_edge*edge[l_pnt].get_length();
   
   double frac=distance_to_point/total_length;
//   cout << "frac = " << frac << endl;
   return frac;
}

// ---------------------------------------------------------------------
// Member function distance_along_polyline returns a negative value if
// the input currpoint does not lie along the polyline.  Otherwise, it
// returns currpoint's distance along the polyline starting from the
// zeroth vertex.

double polyline::distance_along_polyline(const threevector& currpoint) 
{
//   cout << "inside polyline::distance_along_polyline()" << endl;
//   cout << "total_length = " << total_length << endl;
   return frac_distance_along_polyline(currpoint)*total_length;
}

// ---------------------------------------------------------------------
// Member function vertices_in_frac_interval generates an STL vector
// of threevectors containing the currently polyline's vertices whose
// fractional distances lie within [f_start,f_stop].

vector<threevector> polyline::vertices_in_frac_interval(
   double f_start,double f_stop)
{
   vector<threevector> vertices_in_interval;
   for (unsigned int v=0; v<get_n_vertices(); v++)
   {
      threevector curr_vertex=get_vertex(v);
      double curr_frac=frac_distance_along_polyline(curr_vertex);
      if (curr_frac >= f_start && curr_frac <= f_stop)
      {
         vertices_in_interval.push_back(curr_vertex);
      }
   }
   return vertices_in_interval;
}

// ==========================================================================
// Edge point resampling member functions
// ==========================================================================

// Member function compute_regularly_spaced_edge_points() periodically
// samples the polyline and returns points along the polyline within
// output STL vector V.  It starts at frac=0 and increments frac until
// it exceeds unity.

void polyline::compute_regularly_spaced_edge_points(
   double ds,vector<threevector>& V)
{
//   cout << "inside polyline::compute_regularly_spaced_edge_points()" << endl;
//   cout << "ds = " << ds << endl;

   double frac=ds/total_length;
   unsigned int n_samples=basic_math::mytruncate(total_length/ds);
//   cout << "total_length = " << total_length << endl;
//   cout << "total_length/ds = " << total_length/ds << endl;
//   cout << "n_samples = " << n_samples << endl;
   V.clear();
   for (unsigned int n=0; n<=n_samples; n++)
   {
      double curr_frac=n*frac;
      V.push_back(edge_point(curr_frac));
//      cout << "n = " << n << " curr_frac = " << curr_frac << endl;
//      cout << "V.back() = " << V.back() << endl;
   }
}

// This overloaded version of compute_regularly_spaced_edge_points()
// takes in boolean parameter include_original_vertices_flag.  If this
// flag ==true, the polyline's "corner" vertices are included within
// output V.  This flag should be set to true when PolyLine picking
// purposes where rounded corners are undesirable.

void polyline::compute_regularly_spaced_edge_points(
   double ds,vector<threevector>& V,bool include_original_vertices_flag)
{
   unsigned int n_samples=basic_math::round(total_length/ds)+1;
   regular_vertex_spacing=ds;
   compute_regularly_spaced_edge_points(
      n_samples,V,include_original_vertices_flag);
}

void polyline::compute_regularly_spaced_edge_points(
   unsigned int n_samples,vector<threevector>& V,
   bool include_original_vertices_flag)
{
//   cout << "inside polyline::compute_regularly_spaced_edge_points()" << endl;
//   cout << "n_samples = " << n_samples << endl;
//   cout << "include_original_vertices_flag = " 
//        << include_original_vertices_flag << endl;

   V.clear();
   
   double frac=1.0/double(n_samples);
   double curr_frac;
   int prev_edge_number=0;
   for (unsigned int n=0; n<n_samples; n++)
   {
      curr_frac=n*frac;

      if (include_original_vertices_flag)
      {
         int curr_edge_number=edge_number(curr_frac);
         if (curr_edge_number > prev_edge_number)
         {
            V.push_back(edge[curr_edge_number].get_v1());

//            cout << "inside polyline::compute_regularly_spaced_edge_points()"
//                 << endl;
//            cout << "curr_edge.V1 = " << V.back() << endl;
//            cout << "prev_edge.V2 = " << edge[prev_edge_number].get_v2()
//                 << endl;
         }
         prev_edge_number=curr_edge_number;
      } // include_original_vertices_flag conditional
      
      V.push_back(edge_point(curr_frac));
//      cout << "n = " << n << " curr_frac = " << curr_frac << endl;
//      cout << "V.back() = " << V.back() << endl;
   }
}

// ---------------------------------------------------------------------
// Member function generate_sampled_edge_points_kdtree takes in
// polyline spacing parameter ds.  It fills member STL vector
// sampled_edge_points with fourvectors containing XYZ triples along
// the polyline along with integer indices labeling the triples.  It
// then instantiates a KDtree for the fourvectors which can later be
// used in fast searches for sampled edge points close to some
// external point.

void polyline::generate_sampled_edge_points_kdtree()
{
//   cout << "inside polyline::generate_sampled_edge_points_kdtree()" 
//        << endl;
//   cout << "regular_vertex_spacing = " << regular_vertex_spacing << endl;
   generate_sampled_edge_points_kdtree(regular_vertex_spacing);
}

void polyline::generate_sampled_edge_points_kdtree(double ds)
{
//   cout << "inside polyline::generate_sampled_edge_points_kdtree()"
//        << endl;
//   cout << "ds = " << ds << endl;

   vector<threevector> edge_points;
   bool include_original_vertices_flag=true;
   compute_regularly_spaced_edge_points(
      ds,edge_points,include_original_vertices_flag);

   sampled_edge_points.clear();
   for (unsigned int i=0; i<edge_points.size(); i++)
   {
      sampled_edge_points.push_back(fourvector(edge_points[i],i));
   }
   
   delete kdtree_ptr;
   kdtree_ptr=kdtreefunc::generate_3D_kdtree(sampled_edge_points);
//   cout << "*kdtree_ptr = " << *kdtree_ptr << endl;

// Append final vertex to sampled_edge_points STL vector member to
// simplify polyline edge segment searching purposes:

   sampled_edge_points.push_back(
      fourvector(vertex[n_vertices-1],sampled_edge_points.size()));
}

// ---------------------------------------------------------------------
// Member function smooth_raw_vertices takes in correlation distance
// ds along with a set of raw vertices.  This method performs a brute
// force convolution of the input vertices with a set of (somewhat
// arbitrary) weights. The smoothed set of vertices are returned by
// this method.

vector<threevector> polyline::smooth_raw_vertices(
   double ds,const vector<threevector>& raw_vertices)
{
//   cout << "inside polyline::smooth_raw_vertices()" << endl;
//   cout << "ds = " << ds << endl;

   vector<threevector> smoothed_vertices;
   for (unsigned int v=0; v<raw_vertices.size(); v++)
   {
      threevector raw_vertex=raw_vertices[v];
      double s=distance_along_polyline(raw_vertex);

      vector<threevector> sampled_points;
      sampled_points.push_back(edge_point_at_distance_along_polyline(
         s-1.5*ds));
      sampled_points.push_back(edge_point_at_distance_along_polyline(
         s-1*ds));
      sampled_points.push_back(edge_point_at_distance_along_polyline(
         s-0.5*ds));
      sampled_points.push_back(edge_point_at_distance_along_polyline(
         s));
      sampled_points.push_back(edge_point_at_distance_along_polyline(
         s+0.5*ds));
      sampled_points.push_back(edge_point_at_distance_along_polyline(
         s+1*ds));
      sampled_points.push_back(edge_point_at_distance_along_polyline(
         s+1.5*ds));

      vector<double> weight;
      weight.push_back(0.025);
      weight.push_back(0.10);
      weight.push_back(0.20);
      weight.push_back(0.35);
      weight.push_back(0.20);
      weight.push_back(0.10);
      weight.push_back(0.025);
      
      threevector smoothed_vertex;
      for (unsigned int s=0; s<weight.size(); s++)
      {
         smoothed_vertex += weight[s]*sampled_points[s];
      }
//      cout << "raw_vertex = " << raw_vertex
//           << " smoothed_vertex = " << smoothed_vertex << endl;
      
      smoothed_vertices.push_back(smoothed_vertex);
   } // loop over index v labeling raw polyline vertices

   return smoothed_vertices;
}

// ---------------------------------------------------------------------
// This overloaded version of smooth_raw_vertices performs a
// brute-force low-pass filtering of the current polyline's vertices.
// It replaces the current raw set of vertices with the smoothed
// counterparts.  This method then recomputes the total length and
// edge content of the current polyline.

void polyline::smooth_raw_vertices(double ds)
{
//   cout << "inside polyline::smooth_raw_vertices(ds)" << endl;
   vector<threevector> raw_vertices;
   for (unsigned int v=0; v<n_vertices; v++)
   {
      raw_vertices.push_back(get_vertex(v));
   }
   vector<threevector> smoothed_vertices=smooth_raw_vertices(ds,raw_vertices);

   for (unsigned int v=0; v<n_vertices; v++)
   {
      vertex[v]=smoothed_vertices[v];
   }
   edge.clear();
   total_length=0;
//   cout << "Before call to compute_total_length()" << endl;
   compute_total_length();	// calls initialize_edge_segments()
//   cout << "total_length = " << total_length << endl;
//   cout << "at end of polyline::smooth_raw_vertices(ds)" << endl;
}

// ---------------------------------------------------------------------
// Member function compute_irregularly_spaced_edge_fracs()

vector<double> polyline::compute_irregularly_spaced_edge_fracs(
   double ds,double ds_fine)
{
//   cout << "inside polyline::compute_irregularly_spaced_edge_points()" 
//	  << endl;

   compute_total_length();
   unsigned int n_steps=total_length/ds_fine+1;
   vector<double> frac;
   vector<threevector> e_hat;
   for (unsigned int n=0; n<n_steps; n++)
   {
      double curr_frac=double(n)/double(n_steps);
      frac.push_back(curr_frac);
      e_hat.push_back(edge_direction(curr_frac));
   } // loop over index n labeling fine steps

   vector<double> irregularly_sampled_fracs;
   irregularly_sampled_fracs.push_back(frac[0]);

   int n_fine_steps_per_big_step=ds/ds_fine;

   const double threshold_dotproduct=cos(30*PI/180);
   for (unsigned int n=0; n<n_steps-1; n++)
   {
      double last_frac_added=irregularly_sampled_fracs.back();
      
      double dotproduct=e_hat[n+1].dot(e_hat[n]);
      if (dotproduct < threshold_dotproduct)
      {
         if (!nearly_equal(frac[n+1],last_frac_added))
         {
            irregularly_sampled_fracs.push_back(frac[n+1]);
         }
      }
      last_frac_added=irregularly_sampled_fracs.back();

      if ((n+1)%n_fine_steps_per_big_step==0)
      {
         if (!nearly_equal(frac[n+1],last_frac_added))
         {
            irregularly_sampled_fracs.push_back(frac[n+1]);
         }
      }
   } // loop over index n labeling fine steps

   return irregularly_sampled_fracs;

/*
   for (unsigned int f=0; f<irregularly_sampled_fracs.size(); f++)
   {
      V.push_back(edge_point(irregularly_sampled_fracs[f]));
   } // loop over index f labeling irregularly sampled fractions
*/

}

