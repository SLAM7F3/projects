// Note added on 1/19/04: Test someday what happens if we do NOT call
// compute_normal() as a default constructor function.  Most often, we
// do not need any normal info.  So calling this expensive function is
// wasteful!

// ==========================================================================
// Polygon class member function definitions
// ==========================================================================
// Last modified on 4/16/12; 3/5/14; 3/6/14; 4/4/14
// ==========================================================================

#include <unistd.h>	// Needed for sleep command
#include "math/basic_math.h"
#include "math/constant_vectors.h"
#include "geometry/contour.h"
#include "geometry/geometry_funcs.h"
#include "templates/mytemplates.h"
#include "geometry/plane.h"
#include "geometry/polygon.h"
#include "geometry/polyline.h"
#include "math/rotation.h"

#include "general/outputfuncs.h"

using std::cout;
using std::endl;
using std::ostream;
using std::pair;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void polygon::allocate_member_objects()
{
}		       

void polygon::initialize_member_objects()
{
   occluded_flag=false;
   ID=-1;
   area=perimeter=0;
   origin=normal=vertex_avg=Zero_vector;
   triangles_group_ptr=NULL;
   plane_ptr=NULL;

// For speed purposes, we do NOT dynamically allocate the edge array
// unless we specifically know that we're going to use it.  Instead,
// we simply set the edge array's pointer to NULL:

   edge=NULL;
}

void polygon::assign_vertex_array_pointers(int num_of_vertices)
{
   nvertices=num_of_vertices;
   if (nvertices <= 2)
   {
//      cout << "Error inside polygon::assign_vertex_array_points()!" << endl;
//      cout << "nvertices = " << nvertices << endl;
//      exit(-1);
   }
   if (num_of_vertices < MAX_STATIC_VERTICES)
   {
      vertex_ptr=vertex_stack;
      vertex_heap_ptr=NULL;
   }
   else
   {
      vertex_heap_ptr=new threevector[num_of_vertices];
      vertex_ptr=vertex_heap_ptr;
   }
}

polygon::polygon(void)
{
   allocate_member_objects();
   initialize_member_objects();
   assign_vertex_array_pointers(0);
}

polygon::polygon(int num_of_vertices)
{
   allocate_member_objects();
   initialize_member_objects();
   assign_vertex_array_pointers(num_of_vertices);
}

polygon::polygon(const vector<twovector>& currvertex)
{
   allocate_member_objects();
   initialize_member_objects();
   assign_vertex_array_pointers(currvertex.size());
   for (unsigned int i=0; i<nvertices; i++)
   {
      set_vertex(i,threevector(currvertex[i]));
   }
   compute_area();
   locate_origin();
   compute_normal();
}

polygon::polygon(const threevector& interior_pnt,
                 const vector<threevector>& currvertex)
{
   allocate_member_objects();
   initialize_member_objects();
   assign_vertex_array_pointers(currvertex.size());

   vector<int> vertexlabel(nvertices);
   vector<double> theta(nvertices);
   vector<threevector> relative_vertex(nvertices);

   for (unsigned int i=0; i<nvertices; i++)
   {
      relative_vertex[i]=currvertex[i]-interior_pnt;
      vertexlabel[i]=i;
      theta[i]=atan2(relative_vertex[i].get(1),relative_vertex[i].get(0));
   }
   templatefunc::Quicksort(theta,vertexlabel);
   for (unsigned int i=0; i<nvertices; i++)
   {
      set_vertex(i,currvertex[vertexlabel[i]]);
   }
   compute_area();
   origin=interior_pnt;
   compute_normal();
}

polygon::polygon(const vector<threevector>& currvertex)
{
   allocate_member_objects();
   initialize_member_objects();
   assign_vertex_array_pointers(currvertex.size());

// Reject any input vertices which are nearly equal:

   set_vertex(0,currvertex[0]);
   threevector prev_vertex=get_vertex(0);
   const double TINY=1E-7;
   for (unsigned int v=1; v<nvertices; v++)
   {
      if (
         !currvertex[v].nearly_equal(prev_vertex,TINY) &&
         !currvertex[v].nearly_equal(currvertex[0],TINY) )
      {
         set_vertex(v,threevector(currvertex[v]));
         prev_vertex=get_vertex(v);
      }
   }

   if (get_nvertices() < 2)
   {
      cout << "Error in polygon constructor!" << endl;
      cout << "Number of non-overlapping input vertices = "
           << get_nvertices() << endl;
      outputfunc::enter_continue_char();
      return;
   }

//   for (unsigned int i=0; i<nvertices; i++)
//   {
//      set_vertex(i,threevector(currvertex[i]));
//   }
   compute_area();
   locate_origin();
   compute_normal();
}

// Specialized "bounding box" polygon constructor:

polygon::polygon(double min_x,double min_y,double max_x,double max_y)
{
//   cout << "inside polygon bbox constructor" << endl;
   allocate_member_objects();
   initialize_member_objects();

   assign_vertex_array_pointers(4);
   set_vertex(0,threevector(min_x,min_y,0));
   set_vertex(1,threevector(max_x,min_y,0));
   set_vertex(2,threevector(max_x,max_y,0));
   set_vertex(3,threevector(min_x,max_y,0));

   origin=vertex_average();
   normal=z_hat;
   area=(max_x-min_x)*(max_y-min_y);
   perimeter=2.0*( (max_x-min_x) + (max_y-min_y) );
}

polygon::polygon(contour const *c_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   assign_vertex_array_pointers(c_ptr->get_nvertices());
   for (unsigned int i=0; i<nvertices; i++)
   {
      set_vertex(i,c_ptr->get_vertex(i).first);
   }
   locate_origin();
   compute_normal();
}

// This overloaded constructor takes in a polyline which may close
// onto itself to form a loop.  If so, the polyline's final vertex
// equals its zeroth vertex and should NOT be incorporated into the
// constructed polygon...

polygon::polygon(const polyline& p)
{
//   cout << "inside polygon(polyline) constructor" << endl;
   
   allocate_member_objects();
   initialize_member_objects();
   int n_vertices=p.get_n_vertices();
//   cout << "p.get_vertex(0) = " << p.get_vertex(0) << endl;
//   cout << "p.get_vertex(nvertices-2) = " << p.get_vertex(n_vertices-2) 
//        << endl;
//   cout << "p.get_vertex(nvertices-1) = " << p.get_vertex(n_vertices-1) 
//        << endl;

   if (p.get_vertex(0).nearly_equal(p.get_vertex(n_vertices-1)))
   {
      n_vertices--;
   }
   assign_vertex_array_pointers(n_vertices);
   for (unsigned int i=0; i<nvertices; i++)
   {
      set_vertex(i,p.get_vertex(i));
   }
   compute_area();
   locate_origin();
   compute_normal();
}

// Copy constructor:

polygon::polygon(const polygon& p)
{
   allocate_member_objects();
   initialize_member_objects();
   assign_vertex_array_pointers(p.nvertices);
   docopy(p);
}

polygon::~polygon()
{
   if (vertex_heap_ptr != NULL) delete [] vertex_heap_ptr;
   delete [] edge;
   delete plane_ptr;
}

// ---------------------------------------------------------------------
void polygon::docopy(const polygon& p)
{
   assign_vertex_array_pointers(p.nvertices);

   nvertices=p.nvertices;
   area=p.area;
   perimeter=p.perimeter;
   origin=p.origin;
   normal=p.normal;
   vertex_avg=p.vertex_avg;
   for (unsigned int i=0; i<nvertices; i++)
   {
      set_vertex(i,p.get_vertex(i));
   }
}

// Overload = operator:

polygon& polygon::operator= (const polygon& p)
{
   if (this==&p) return *this;
   docopy(p);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const polygon& p)
{
   outstream << endl;
   outstream << "nvertices = " << p.nvertices << endl;
   outstream << "area = " << p.area << endl;
   if (p.nvertices >= 3)
   {
      for (unsigned int i=0; i<p.nvertices; i++)
      {
         int i_prev=modulo(i-1,p.nvertices);
         threevector vertex(p.get_vertex(i));
         threevector prev_vertex(p.get_vertex(i_prev));
//         outstream << "vertex[" << i << "]=threevector(" << vertex.get(0) << ","
//                   << vertex.get(1) << ");" << endl;
         outstream << "vertex[" << i << "]=threevector(" 
                   << vertex.get(0) << "," << vertex.get(1) << ","
                   << vertex.get(2) << ");" << endl;

//         outstream << "Polygon vertex " << i 
//                   << " x = " << p.get_vertex(i).get(0)
//                   << " y = " << p.get_vertex(i).get(1) 
//                   << " z = " << p.get_vertex(i).get(2)
//                   << endl;
//         outstream << "edge = " << p.edge[i] << endl;
//         outstream << "edge.get_ehat() = " << p.edge[i].get_ehat() << endl;
//         outstream << "edge.get_length() = " << p.edge[i].get_length() << endl;
//         cout << "edge_length = " << (vertex-prev_vertex).magnitude()
//              << endl;

      }
      outstream << "Polygon origin = " << p.origin << endl;
      outstream << "Polygon normal vector = " << p.normal << endl;
   }
   else
   {
      outstream << "Polygon is degenerate" << endl;
   }
   return(outstream);
}

// =====================================================================
// Set & get member functions
// =====================================================================

plane* polygon::get_plane_ptr()
{
   if (plane_ptr==NULL) recompute_plane();
   return plane_ptr;
}

// =====================================================================
// Intrinsic polygon properties:
// =====================================================================

// Member function compute_normal() calculates a polygon's unit normal
// vector up to an overall sign.  It forms cross products between
// adjacent polygon edge direction vectors.  If the polygon's vertices
// are ordered according to the right hand rule, this method will
// return a normal vector with the proper sign.

threevector& polygon::compute_normal() 
{
   normal=Zero_vector;
   for (unsigned int i=0; i<get_nvertices(); i++)
   {
      int j=modulo(i+1,get_nvertices());
      int k=modulo(i+2,get_nvertices());
      threevector first_relv(get_vertex(j)-get_vertex(i));
      threevector second_relv(get_vertex(k)-get_vertex(j));
      threevector nvec(first_relv.cross(second_relv));

      const double TINY=1E-8;
      if (nearly_equal(nvec.magnitude(),0,TINY)) continue;
      
      normal=nvec.unitvector();
      break;
      
   } // loop over index i labeling vertices

   if (nearly_equal(normal.magnitude(),0))
   {
      cout << "Error in polygon::compute_normal()" << endl;
      cout << "*this = " << *this << endl;
//      outputfunc::enter_continue_char();
   }
   
   return normal;
}

// ---------------------------------------------------------------------
// Member function locate_origin finds some point lying inside the
// current polygon object which can serve as an origin for normal
// vector, polygon area and interior/exterior computations.  We simply
// construct bisector linesegments that join vertices of the current
// polygon object to midpoints of opposing sides.  The midpoints of
// the bisector segments are tested to determine whether they lie
// inside the polygon.  The first one which does is taken to be the origin.

bool polygon::locate_origin()
{
   bool origin_located=false;
   
   if (nvertices==3)
   {
      origin=vertex_average();
      origin_located=true;
   }
   else if (nvertices > 3)
   {

// As of Nov 04, we try to "statically" allocate the following side
// array rather than dynamically allocate member edge array.  Perhaps
// we are just fooling ourselves given that the following array must
// in fact be allocated at run-time rather than at compile-time...

      linesegment side[nvertices];
      for (unsigned int i=0; i<nvertices; i++)
      {     
         side[i]=linesegment(get_vertex(i),get_vertex(modulo(i+1,nvertices)));
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
               linesegment bisector=linesegment(get_vertex(i),midpnt);
               threevector trial_origin(bisector.get_midpoint());
               if (point_inside_polygon(trial_origin))
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
   return origin_located;   
}

// ---------------------------------------------------------------------
// Member function compute_area() implements the very clever
// algorithm of D.R. Finley for finding areas of arbitrary 2D
// polygons.  This algorithm works for concave, convex and even
// self-intersecting polygons!  

// If the polygon's vertices are ordered according to the right hand
// rule, the calculated area is positive.  If not, we reorder the
// polygon's vertices so that they obey the right-handed rule.

// See // http://alienryderflex.com/polygon_area/ .  See also
// http://paulbourke.net/geometry/polyarea/ . 

double polygon::compute_area() 
{
//   cout << "inside polygon::compute_area()" << endl;
//   cout << "n_vertices = " << get_nvertices() << endl;

   area=0;
   for (unsigned int i=0; i<get_nvertices(); i++)
   {
      int j=modulo(i+1,get_nvertices());
      threevector curr_vertex=get_vertex(i);
      threevector next_vertex=get_vertex(j);
      area += 0.5*(curr_vertex.get(0)+next_vertex.get(0)) * 
         (next_vertex.get(1)-curr_vertex.get(1));
   }
//   cout << "area = " << area << endl;
   
   guarantee_right_handed_vertex_ordering();

   return area;
}

void polygon::guarantee_right_handed_vertex_ordering()
{
   if (area < 0)
   {
      vector<threevector> tmp_vertices;
      for (unsigned int i=0; i<get_nvertices(); i++)
      {
         tmp_vertices.push_back(get_vertex(get_nvertices()-1-i));
      }
      for (unsigned int i=0; i<get_nvertices(); i++)
      {
         set_vertex(i,tmp_vertices[i]);
      }
      area=-area;
   }
}

// ---------------------------------------------------------------------
// Member function compute_2D_COM() implements the centroid result
// reported in
// http://paulbourke.net/geometry/polyarea/

// FAKE FAKE:  TUES JAN 31, 2012 AT 11 AM'

// THIS NEXT METHOD IS A HACK!!!  WE NEED TO GENERALIZE THE
// COMPUTE_2D_COM() MEMBER FUNCTION TO HANDLE THE 3D CASE

threevector polygon::compute_COM()
{
//   cout << "inside polygon::compute_COM()" << endl;

   threevector COM=Zero_vector;
   for (unsigned int i=0; i<get_nvertices(); i++)
   {
      COM += get_vertex(i);
   }
   COM /= get_nvertices();
//   cout << "COM = " << COM << endl;
   return COM;
}

threevector polygon::compute_2D_COM()
{
   cout << "inside polygon::compute_2D_COM()" << endl;
   
   double cx=0;
   double cy=0;
   for (unsigned int i=0; i<get_nvertices(); i++)
   {
      threevector curr_vertex=get_vertex(i);
      threevector next_vertex=get_vertex(modulo(i+1,get_nvertices()));
      double curr_x=curr_vertex.get(0);
      double curr_y=curr_vertex.get(1);
      double next_x=next_vertex.get(0);
      double next_y=next_vertex.get(1);
      
      cx += (curr_x+next_x)*(curr_x*next_y - next_x*curr_y);
      cy += (curr_y+next_y)*(curr_x*next_y - next_x*curr_y);
   }
   threevector COM(cx,cy);
   COM /= (6*get_area());
//   cout << "COM = " << COM << endl;
   return COM;
}

// ---------------------------------------------------------------------
// Member function projected_area returns the positive semi-definite
// projected area of the current polygon object onto the plane
// orthogonal to the input uhat direction vector:

double polygon::projected_area(const threevector& uhat) 
{
   if (area==0) compute_area();
   double projarea=area*fabs(uhat.dot(normal));
   return projarea;
}

// ---------------------------------------------------------------------
// Member function compute_perimeter sets member variable perimeter
// equal to the sum of the current polygon's edge lengths.

double polygon::compute_perimeter()
{
   if (edge==NULL) initialize_edge_segments();
   perimeter=0;
   for (unsigned int i=0; i<nvertices; i++) perimeter += edge[i].get_length();
   return perimeter;
}

// ---------------------------------------------------------------------
// Member function vertex_average computes the average of the
// polygon's n vertices.  This average provides an approximate
// location for the polygon's "center".

threevector& polygon::vertex_average() 
{
   vertex_avg=Zero_vector;
   
   for (unsigned int i=0; i<nvertices; i++) vertex_avg += get_vertex(i);
   vertex_avg /= double(nvertices);
   return vertex_avg;
}

// ---------------------------------------------------------------------
// Member function natural_coordinate_system computes direction
// vectors alpha_hat and beta_hat which are orthogonal to the current
// polygon's normal.  alpha_hat, beta_hat and normal constitute an
// ortho-normal triad which define a natural coordinate system upon
// the current polygon.  This method assembles the components of these
// 3 direction vectors (wrt to the x_hat, y_hat and z_hat) coordinate
// system within rotation matrix R.  Rtranspose provides the passive
// transformation which maps coordinates with the XYZ system onto
// coordinates within the alpha-beta-normal system.  This method
// returns Rtranspose.

rotation polygon::natural_coordinate_system() const
{
   threevector uhat[3];
   uhat[0]=x_hat;
   uhat[1]=y_hat;
   uhat[2]=z_hat;

   double dotproduct[3];
   for (unsigned int i=0; i<3; i++)
   {
      dotproduct[i]=fabs(uhat[i].dot(normal));
   }
   Quicksort(dotproduct,uhat,3);
   threevector ehat(uhat[0]);

   threevector alpha_hat=(normal.cross(ehat)).unitvector();
   threevector beta_hat=(normal.cross(alpha_hat)).unitvector();
   
   rotation R;
   R.put_column(0,alpha_hat);
   R.put_column(1,beta_hat);
   R.put_column(2,normal);
   return R.transpose();
}

// ---------------------------------------------------------------------
// Member function generate_interior_points_list takes in a sampling
// distance ds (measured in meters) as well as rotation matrix Rtrans
// which maps XYZ points into the current polygon's natural coordinate
// system.  This method forms a bounding box around the polygon within
// alpha-beta space.  It performs a brute force scan over points
// inside the bounding box and determines whether they reside inside
// the alpha-beta plane version of the polygon.  Interior points are
// transformed back to XYZ space and are saved within a dynamically
// generated STL vector.  A pointer to this STL vector is returned by
// this method.

vector<pair<threevector,bool> >* polygon::generate_interior_points_list(
   double ds_frac,const rotation& Rtrans,double max_dist_to_poly_edge,
   bool retain_pnts_far_from_edges,double extremal_overstep)
{
   const double TINY=1E-10;
   threevector average_vertex(vertex_average());
   vector<threevector> abn_vertex(nvertices);
   for (unsigned int i=0; i<nvertices; i++)
   {
      abn_vertex[i]=Rtrans*(get_vertex(i)-average_vertex);
      if (fabs(abn_vertex[i].get(0)) < TINY) abn_vertex[i].put(0,0);
      if (fabs(abn_vertex[i].get(1)) < TINY) abn_vertex[i].put(1,0);
      if (fabs(abn_vertex[i].get(2)) < TINY) abn_vertex[i].put(2,0);
   }

// Instantiate a counterpart to the current polygon which resides
// completely within 2D alpha-beta plane:

   polygon ab_poly(abn_vertex);

   double min_alpha,max_alpha,min_beta,max_beta;
   ab_poly.locate_extremal_xy_points(min_alpha,min_beta,max_alpha,max_beta);

// Traverse over bounding box in alpha-beta plane and generate list of
// (alpha,beta) pairs that reside inside ab_poly's interior.  Then
// after transforming these (alpha,beta) pairs back to XYZ space, save
// them within an STL vector:

//   const double dalpha=ds;
//   const double dbeta=ds;
   double dalpha=ds_frac*(max_alpha-min_alpha);
   double dbeta=ds_frac*(max_beta-min_beta);

   double delta=basic_math::min(dalpha,dbeta);
   dalpha=delta;
   dbeta=delta;

   min_alpha -= extremal_overstep*dalpha;
   max_alpha += extremal_overstep*dalpha;
   min_beta -= extremal_overstep*dbeta;
   max_beta += extremal_overstep*dbeta;
   
   rotation R(Rtrans.transpose());
   vector<pair<threevector,bool> >* interior_points_ptr=new vector<
      pair<threevector,bool> >;
   for (double alpha=min_alpha; alpha < max_alpha; alpha += dalpha)
   {
      for (double beta=min_beta; beta < max_beta; beta += dbeta)
      {
         threevector curr_ab_pnt(alpha,beta);
         if (ab_poly.point_inside_polygon(curr_ab_pnt))
         {
//            cout << "pnt_dist_to_polygon = " << ab_poly.point_dist_to_polygon(
//               curr_ab_pnt) << endl;
            
            bool pnt_near_edge=ab_poly.point_dist_to_polygon(curr_ab_pnt)
               < max_dist_to_poly_edge;
            if (pnt_near_edge ||
                (!pnt_near_edge && retain_pnts_far_from_edges))
            {
               threevector curr_point(
                  R*threevector(alpha,beta)+average_vertex);
               interior_points_ptr->push_back(pair<threevector,bool>(
                  curr_point,pnt_near_edge));
            }
         } // curr_ab_pnt inside ab_poly conditional
      } // beta loop
   } // alpha loop

   return interior_points_ptr;
}

// =====================================================================
// Polygon edge traversing methods:
// =====================================================================

// For speed purposes, we do NOT include the following edge
// initialization within the overall initialize_member_objects()
// member function:

void polygon::initialize_edge_segments()
{
   if (edge==NULL)
   {
      edge=new linesegment[nvertices];
      compute_edges();
   }
}

void polygon::compute_edges()
{
//   cout << "inside polygon::compute_edges(), nvertices = "
//        << nvertices << endl;
   for (unsigned int i=0; i<nvertices; i++) 
   {
      edge[i]=linesegment(get_vertex(i),get_vertex(modulo(i+1,nvertices)));
   }
}

// ---------------------------------------------------------------------
// Member function edge_number takes in fraction 0 <= frac <= 1 and
// returns the number of the edge on which the point whose length from
// get_vertex(0) equals frac*perimeter:

int polygon::edge_number(double frac)
{
   compute_perimeter();

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
// returns the corresponding point along the polygon's perimeter whose
// length from get_vertex(0) equals frac*perimeter:

void polygon::edge_point(double frac,threevector& curr_point) 
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
      curr_edge_frac*edge[segment_number].get_length()*edge[segment_number].
      get_ehat();
}

// ---------------------------------------------------------------------
// Member function radial_direction_vector returns within output
// threevector r_hat a direction unit vector which points in the
// outwardly radial direction at the point along the polygon's
// perimeter which is fraction frac*perimeter from starting point
// get_vertex(0).

void polygon::radial_direction_vector(double frac,threevector& r_hat)
{
   int segment_number=edge_number(frac);
   r_hat=edge[segment_number].get_ehat().cross(normal);
}

// =====================================================================
// Determining point, linesegment and polygon locations relative to
// one another:
// =====================================================================

// Locate minimum/maximum x and y points on current polygon object:

void polygon::locate_extremal_xy_points(
   double& min_x,double& min_y,double& max_x,double& max_y) const
{
   min_x=min_y=100.0*POSITIVEINFINITY;
   max_x=max_y=100.0*NEGATIVEINFINITY;
   for (unsigned int i=0; i<nvertices; i++)
   {
      min_x=basic_math::min(get_vertex(i).get(0),min_x);
      min_y=basic_math::min(get_vertex(i).get(1),min_y);
      max_x=basic_math::max(get_vertex(i).get(0),max_x);
      max_y=basic_math::max(get_vertex(i).get(1),max_y);
   }
}

void polygon::locate_extremal_xyz_points(
   double& min_x,double& min_y,double& min_z,
   double& max_x,double& max_y,double& max_z) const
{
   min_x=min_y=min_z=POSITIVEINFINITY;
   max_x=max_y=max_z=NEGATIVEINFINITY;
   for (unsigned int i=0; i<nvertices; i++)
   {
      min_x=basic_math::min(get_vertex(i).get(0),min_x);
      min_y=basic_math::min(get_vertex(i).get(1),min_y);
      min_z=basic_math::min(get_vertex(i).get(2),min_z);
      max_x=basic_math::max(get_vertex(i).get(0),max_x);
      max_y=basic_math::max(get_vertex(i).get(1),max_y);
      max_z=basic_math::max(get_vertex(i).get(2),max_z);
   }
}

// This next method generalizes locate_extremal_xy_points().  It takes
// in direction vectors alpha_hat and beta_hat and projects the
// current polygon's vertices onto the alpha_hat and beta_hat axes.
// This method returns the polygon's extremal alpha and beta
// coordinates.

void polygon::locate_extremal_ab_points(
   const threevector& alpha_hat,const threevector& beta_hat,
   double& min_alpha,double& min_beta,
   double& max_alpha,double& max_beta) const
{
   min_alpha=min_beta=POSITIVEINFINITY;
   max_alpha=max_beta=NEGATIVEINFINITY;

   for (unsigned int i=0; i<nvertices; i++)
   {
      min_alpha=basic_math::min(get_vertex(i).dot(alpha_hat),min_alpha);
      min_beta=basic_math::min(get_vertex(i).dot(beta_hat),min_beta);
      max_alpha=basic_math::max(get_vertex(i).dot(alpha_hat),max_alpha);
      max_beta=basic_math::max(get_vertex(i).dot(beta_hat),max_beta);
   }
}

// ==========================================================================
// Point location relative to polygon
// ==========================================================================

// Member function point_within_polygon() implements W. Randolph
// Franklin's PNPOLY (Point Inclusion in Polygon) test.  See
// http://www.ecse.rpi.edu/Homepages/wrf/Research/Short_Notes/pnpoly.html

// As of 10/22/11, very preliminary testing of this (fast!) method
// looks encouraging...

bool polygon::point_inside_polygon(const threevector& point)
{
   return point_inside_polygon(twovector(point));
}

bool polygon::point_inside_polygon(const twovector& point)
{
//   cout << "inside polygon::point_inside_polygon(), x = "
//	<< point.get(0) << " y = " << point.get(1) << endl;
  
   double x_point=point.get(0);
   double y_point=point.get(1);

   bool point_inside=false;
   for (unsigned int i=0, j=nvertices-1; i<nvertices; j=i++)
   {
      double x_i=get_vertex(i).get(0);
      double y_i=get_vertex(i).get(1);
      double x_j=get_vertex(j).get(0);
      double y_j=get_vertex(j).get(1);
      
      if ( ( (y_i > y_point) != (y_j > y_point) ) &&
      (x_point < (x_j-x_i) * (y_point-y_i)/(y_j-y_i)+x_i) )
      {
         point_inside=!point_inside;
      }
   }

//   cout << "point_inside = " << point_inside << endl;
//   outputfunc::enter_continue_char();
   
   return point_inside;
}

// ---------------------------------------------------------------------
// Member function point_on_perimeter determines whether a 2-point
// lies exactly on the border of a polygon. 

bool polygon::point_on_perimeter(const threevector& point) const
{
   bool pnt_on_border=false;

// First check whether point coincides with any of the current polygon
// object's vertices:

   const double TINY=1E-8;
   for (unsigned int i=0; i<nvertices; i++)
   {
      if (nearly_equal((point-get_vertex(i)).magnitude(),0,TINY)) 
         pnt_on_border=true;
   }

// Next check whether point lies between any two of the current
// polygon object's vertices:

   if (!pnt_on_border)
   {
     for (unsigned int i=0; i<nvertices; i++)
     {
        linesegment currside(get_vertex(i),get_vertex(modulo(i+1,nvertices)));
        if (currside.point_on_segment(point)) pnt_on_border=true;
     } 
   }
   return pnt_on_border;
}

// ---------------------------------------------------------------------
// Member function frac_distance_along_polygon returns -1 if the input
// point does not lie on the polygon's perimeter.  Otherwise, it
// returns the point's fractional distance from get_vertex(0).  We assume
// that the polygon's edge line segments have been initialized prior
// to this method's being called.

double polygon::frac_distance_along_polygon(const threevector& point) 
{
   if (!point_on_perimeter(point))
   {
      return -1;
   }
   else
   {
      double running_distance=0;
      for (unsigned int i=0; i<nvertices; i++)
      {
         double edge_frac=edge[i].frac_distance_along_segment(point);
         if (edge_frac < 0)
         {
            running_distance += edge[i].get_length();
         }
         else
         {
            running_distance += edge_frac*edge[i].get_length();
            break;
         }
      }
      return running_distance/compute_perimeter();
   }
}

// ---------------------------------------------------------------------
// Member function point_on_perimeter determines whether a 2-point
// lies on the border of a polygon.  It checks whether the 8 points
// displaced away from the current point by +/-/0 dx and dy all lie
// inside or outside the polygon.  If not, the current point must lie
// on the polygon's perimeter.  This approach should be robust against
// pixelization problems:

bool polygon::point_on_perimeter(
   const threevector& point,double dx,double dy) 
{
   bool is_inside[8],on_border;
   threevector surrounding_point[8];

   surrounding_point[0]=point+threevector(-dx,dy,0);
   surrounding_point[1]=point+threevector(0,dy,0);
   surrounding_point[2]=point+threevector(dx,dy,0);
   surrounding_point[3]=point+threevector(-dx,0,0);
   surrounding_point[4]=point+threevector(dx,0,0);
   surrounding_point[5]=point+threevector(-dx,-dy,0);
   surrounding_point[6]=point+threevector(0,-dy,0);
   surrounding_point[7]=point+threevector(dx,-dy,0);

   for (unsigned int i=0; i<8; i++)
   {
      is_inside[i]=point_inside_polygon(surrounding_point[i]);
   }
   
// Check whether point is located completely inside polygon:
   
   if (is_inside[0]==true && is_inside[1]==true &&
       is_inside[2]==true && is_inside[3]==true &&
       is_inside[4]==true && is_inside[5]==true &&
       is_inside[6]==true && is_inside[7]==true)
   {
      on_border=false;
   }

// Check whether point is located completely outside polygon:

    else if (is_inside[0]==false && is_inside[1]==false &&
             is_inside[2]==false && is_inside[3]==false &&
             is_inside[4]==false && is_inside[5]==false &&
             is_inside[6]==false && is_inside[7]==false)
   {
      on_border=false;
   }
   else
   {
      on_border=true;
   }
   return on_border;
}

// ---------------------------------------------------------------------
// Boolean member function point_outside_polygon returns true only if
// the input point lies strictly outside the current polygon object:

bool polygon::point_outside_polygon(
   const threevector& point,double dx,double dy) 
{
   return (!point_inside_polygon(point) && !point_on_perimeter(point,dx,dy));
}

// ---------------------------------------------------------------------
// Member function point_dist_to_polygon calculates the minimal
// distance of a point to all the sides of the current polygon object.
// It first forms rays joining together some user specified point to
// each of the polygon vertices.  It next computes the cross product
// of ray i and i+1.  The value of the ith cross product magnitude
// divided by the ith polygonal side length yields the perpendicular
// distance between the point and the ith side.  We add in quadrature
// to this transverse distance a longitudinal distance in order to
// determine a total distance of the point from the ith side.  The
// minimum of all these total distances for all sides of the polyon is
// returned in min_dist_to_side.

double polygon::point_dist_to_polygon(const threevector& point) 
{
   bool point_on_perimeter=false;
   double min_dist_to_side=POSITIVEINFINITY;
   threevector ray[nvertices];
   
   if (edge==NULL) initialize_edge_segments();
   for (unsigned int i=0; i<nvertices; i++)
   {
      ray[i]=get_vertex(i)-point;
      if (ray[i].magnitude()==0) 
      {
         point_on_perimeter=true;
         min_dist_to_side=0;
      }
   }
   
   if (!point_on_perimeter)
   {
      for (unsigned int i=0; i<nvertices; i++)
      {
         threevector crossproduct(ray[i].cross(ray[modulo(i+1,nvertices)]));
         double h=crossproduct.magnitude()/edge[i].get_length();
         double dotproduct1=fabs(edge[i].get_ehat().dot(ray[i]));
         double dotproduct2=
            fabs(edge[i].get_ehat().dot(ray[modulo(i+1,nvertices)]));
         double d=0.5*(dotproduct1+dotproduct2-edge[i].get_length());
         double dist_to_currside=sqrt(sqr(h)+sqr(d));
         min_dist_to_side=basic_math::min(min_dist_to_side,dist_to_currside);
      }
   }
   return min_dist_to_side;
}

// ---------------------------------------------------------------------
// Member function closest_polygon_edge_to_point returns the number
// for particular polygon edge which lies closest to the input point.

int polygon::closest_polygon_edge_to_point(const threevector& point) 
{
   if (edge==NULL) initialize_edge_segments();

   int closest_edge=-1;
   double min_distance=POSITIVEINFINITY;
   
   for (unsigned int i=0; i<nvertices; i++) 
   {
      double curr_distance=edge[i].point_to_line_segment_distance(point);
      if (curr_distance < min_distance)
      {
         min_distance=curr_distance;
         closest_edge=i;
      }
   } // loop over index i labeling edges
   return closest_edge;
}

// ---------------------------------------------------------------------
// Member function closest_polygon_perimeter_point returns the point
// along the polygon's perimeter which lies closest to the input
// external point.  The external point is assumed to lie in the plane
// of the polygon.

pair<double,threevector> polygon::closest_polygon_perimeter_point(
   const threevector& ext_point) 
{
   if (edge==NULL) initialize_edge_segments();

   double min_distance=POSITIVEINFINITY;
   threevector closest_point,closest_point_on_segment;
   for (unsigned int i=0; i<nvertices; i++) 
   {
      double curr_distance=edge[i].point_to_line_segment_distance(
         ext_point,closest_point_on_segment);
      if (curr_distance < min_distance)
      {
         min_distance=curr_distance;
         closest_point=closest_point_on_segment;
      }
   } // loop over index i labeling edges
   return pair<double,threevector>(min_distance,closest_point);
}

// ---------------------------------------------------------------------
// Method min_dist_to_convex_polygon recursively determines the two
// vertices on convex polygon *convex_poly_ptr which lie closest to
// the input external_point.  It subsequently computes and returns the
// distance between the polygon edge defined by those two vertices and
// the external point.

double polygon::min_dist_to_convex_polygon(
   const threevector& external_point) const
{
   int n_expensive_computations=0;
   double ext_pnt_vertex_sqrd_dist[nvertices];
   for (unsigned int n=0; n<nvertices; n++)
   {
      ext_pnt_vertex_sqrd_dist[n]=-1;
   }
   pair<int,int> p=closest_vertices_to_external_point(
      nvertices,0,n_expensive_computations,
      external_point,ext_pnt_vertex_sqrd_dist);
   linesegment poly_edge(get_vertex(p.first),get_vertex(p.second));
   return poly_edge.point_to_line_segment_distance(external_point);
}

// ---------------------------------------------------------------------
// Method closest_vertices_to_external_point recursively computes the
// closest vertices on a CONVEX polygon to some observation point
// which is assumed to lie outside the polygon.  The distance metric
// is a periodic function of polygon vertex number, and it generally
// exhibits a single minimum.  So we can employ a recursive binary
// search to locate this minimum in log(nvertices) time.

pair<int,int> polygon::closest_vertices_to_external_point(
   int n_jump,int i,int& n_expensive_computations,
   const threevector& external_point,double ext_pnt_vertex_sqrd_dist[]) const
{
   int i_prev=modulo(i-1,nvertices);
   int i_next=modulo(i+1,nvertices);

// Compute distances from observation point to i_prev, i and i_next
// vertices on convex polygon if they have not already been
// calculated:

   for (int j=-1; j<=1; j++)
   {
      int k=modulo(i+j,nvertices);
      if (ext_pnt_vertex_sqrd_dist[k]==-1)
      {
         ext_pnt_vertex_sqrd_dist[k]=
            (external_point-get_vertex(k)).sqrd_magnitude();
         n_expensive_computations++;
      }
   }

   pair<int,int> p;
   if ((ext_pnt_vertex_sqrd_dist[i] <= ext_pnt_vertex_sqrd_dist[i_prev]) &&
       (ext_pnt_vertex_sqrd_dist[i] <= ext_pnt_vertex_sqrd_dist[i_next]))
   {
      if (ext_pnt_vertex_sqrd_dist[i_prev] < ext_pnt_vertex_sqrd_dist[i_next])
      {
         p.first=i;
         p.second=i_prev;
      }
      else
      {
         p.first=i;
         p.second=i_next;
      }
   }
   else
   {
      int sgn=1;
      if (ext_pnt_vertex_sqrd_dist[i_prev] < ext_pnt_vertex_sqrd_dist[i])
      {
         sgn=-1;
      }
      int n_next_jump=basic_math::max(1,n_jump/2);
      int next_i=modulo(i+sgn*n_next_jump,nvertices);
      
      p=closest_vertices_to_external_point(
         n_next_jump,next_i,n_expensive_computations,
         external_point,ext_pnt_vertex_sqrd_dist);
   }
   return p;
}

// ---------------------------------------------------------------------
// Member function closest_polygon_point first determines if the
// projection of the input point into the plane of the current polygon
// object lands inside the polygon.  If it does, this method returns
// the projected point as the closest point within the polygon to the
// external point.  Otherwise, this method locates the closest point
// along the polygon's perimeter.  It also returns the distance
// between the closest point and the external point.

Triple<bool,double,threevector> polygon::closest_polygon_point(
   const threevector& ext_point) 
{
   threevector R(ext_point-get_vertex(0));
   threevector Rperp=(R.dot(normal))*normal;
   threevector R_parallel(R-Rperp);
   threevector proj_point=get_vertex(0)+R_parallel;

   bool proj_point_inside=point_inside_polygon(proj_point);
   double distance_to_ext_point;
   threevector closest_point;
   if (proj_point_inside)
   {
      closest_point=proj_point;
   }
   else
   {
      pair<double,threevector> closest_perim_pnt(
         closest_polygon_perimeter_point(ext_point));
      closest_point=closest_perim_pnt.second;
   } // proj_point_inside conditional
   distance_to_ext_point=(ext_point-closest_point).magnitude();

   return Triple<bool,double,threevector>(
      proj_point_inside,distance_to_ext_point,closest_point);
}

// ==========================================================================
// Line segment location relative to polygon
// ==========================================================================

// Member function linesegment_inside_polygon sets boolean variables
// l_inside_poly and l_outside_poly to true if linesegment l lies
// completely within or outside the current polygon object:

void polygon::linesegment_inside_polygon(
   const linesegment& l,bool& l_inside_poly,bool& l_outside_poly)
{
   unsigned int n_intersected_sides;
   int intersected_side[MAX_STATIC_VERTICES];
   threevector intersection_point[MAX_STATIC_VERTICES];
   
   l_inside_poly=false;
   l_outside_poly=true;

   if ((point_inside_polygon(l.get_v1()) || point_on_perimeter(l.get_v1())) 
       && 
       (point_inside_polygon(l.get_v2()) || point_on_perimeter(l.get_v2())))
   {
      l_inside_poly=true;
      l_outside_poly=false;
   }
   if (linesegment_intersection_with_polygon(
      l,n_intersected_sides,intersected_side,intersection_point))
   {
      l_outside_poly=false;
   }
}

// ---------------------------------------------------------------------
// Member function polygon_inside_polygon set boolean variables
// p_inside_poly and p_outside_poly to true if polygon p lies entirely
// inside our outside of the current polygon object.  If p intersects
// with the current polygon object, both boolean variables are set
// equal to false:

void polygon::polygon_inside_polygon(
   const polygon& p,bool& p_inside_poly,bool& p_outside_poly) 
{
   bool segment_inside,segment_outside;
   linesegment currside;

   p_inside_poly=p_outside_poly=true;   
   for (unsigned int i=0; i<p.nvertices; i++)
   {
//          cout << "i = " << i << endl;
      currside=linesegment(
         p.get_vertex(modulo(i,p.nvertices)),
         p.get_vertex(modulo(i+1,p.nvertices)));
//      cout << "currside = " << currside << endl;
      linesegment_inside_polygon(currside,segment_inside,segment_outside);

      if (!segment_inside)
      {
         p_inside_poly=false;
      }
      if (!segment_outside)
      {
         p_outside_poly=false;
      }
   }
}

// ---------------------------------------------------------------------
// Member function partial_overlap returns false if polygon p lies
// entirely inside or outside of the current polygon object or if the
// current polygon object lies entire inside or outside of p.  It
// otherwise returns true if p partially overlaps with the current
// polygon object:

bool polygon::partial_overlap(polygon& p) 
{
   bool p_inside_poly,p_outside_poly;
   bool poly_inside_p,poly_outside_p;

   polygon_inside_polygon(p,p_inside_poly,p_outside_poly);
   p.polygon_inside_polygon(*this,poly_inside_p,poly_outside_p);

   if (p_inside_poly || poly_inside_p || p_outside_poly || poly_outside_p)
   {
      return false;
   }
   else
   {
      return true;
   }
}

// ---------------------------------------------------------------------
// Member function linesegment_intersection_with_polygon returns false
// if input linesegment l does not intersect any of the edges of the
// current polygon object.  If it does intersect, the number of
// intersection points along with the locations of the intersection
// points are respectively returned in n_intersected_sides and
// intersection_point[].  The polygon sides, labeled by their starting
// vertices, which are intersected by linesegment l are returned in
// intersected_side[].  The values within the arrays are ordered so
// that the first intersection point is closest to l.v1, while the
// last one is furthest from l.v1:

bool polygon::linesegment_intersection_with_polygon(
   const linesegment& l,unsigned int& n_intersection_pnts,
   int intersected_poly_side[],threevector intersection_pnt[]) const
{
   const double TINY=1E-10;
   bool intersection_pnt_on_curr_poly_side,intersection_pnt_on_l;
   bool linesegment_intersects_poly=false;

   threevector curr_intersection_pnt;
   threevector tmp_intersection_pnt[MAX_STATIC_VERTICES];

   n_intersection_pnts=0;
   for (unsigned int i=0; i<nvertices; i++)
   {
      linesegment curr_poly_side(
         get_vertex(i),get_vertex(modulo(i+1,nvertices)));
      curr_poly_side.point_of_intersection(
         l,curr_intersection_pnt,intersection_pnt_on_curr_poly_side,
         intersection_pnt_on_l);

      if (intersection_pnt_on_curr_poly_side &&
          intersection_pnt_on_l)
      {

// Make sure current intersection point doesn't coincide with existing 
// intersection points:

         bool point_already_included=false;
         for (unsigned int j=0; j<n_intersection_pnts; j++)
         {
            threevector deltavec(intersection_pnt[j]-curr_intersection_pnt);
            if (deltavec.magnitude() < TINY) 
            {
               point_already_included=true;
            }
         }
            
         if (!point_already_included)
         {
            linesegment_intersects_poly=true;
            intersected_poly_side[n_intersection_pnts]=i;
            intersection_pnt[n_intersection_pnts]=curr_intersection_pnt;
            n_intersection_pnts++;
         }
      }
   } // for loop index i

// Sort intersection points so that first is closest to l.v1 and last
// is furthest from l.v1:

   double dist_to_v1[MAX_STATIC_VERTICES];
   double index[MAX_STATIC_VERTICES];
   for (unsigned int i=0; i<n_intersection_pnts; i++)
   {
      threevector deltavec(l.get_v1()-intersection_pnt[i]);
      dist_to_v1[i]=deltavec.magnitude();
      index[i]=i;
   }

   if (n_intersection_pnts > 1)
   {
      Quicksort(dist_to_v1-1,index-1,int(n_intersection_pnts));
   }
   
   int tmp_intersected_poly_side[MAX_STATIC_VERTICES];
   for (unsigned int i=0; i<n_intersection_pnts; i++)
   {
      tmp_intersected_poly_side[i]=intersected_poly_side[
         basic_math::round(index[i])];
      tmp_intersection_pnt[i]=intersection_pnt[basic_math::round(index[i])];
   }
   for (unsigned int i=0; i<n_intersection_pnts; i++)
   {
      intersected_poly_side[i]=tmp_intersected_poly_side[i];
      intersection_pnt[i]=tmp_intersection_pnt[i];
   }
   return linesegment_intersects_poly;
}

// ---------------------------------------------------------------------
// Member function locate_polygon_intersection_edges determines, via
// brute force, what portions of the polygon object lie inside and
// outside of the polygon q's current side.  We have split off the
// following lines from the polygon::polygon_intersection subroutine.

void polygon::locate_polygon_intersection_edges(
   int n_intersected_poly_sides,int& j,polygon& q,
   int intersected_poly_side[],threevector intersection_point[],
   threevector intersection_poly_vertex[]) 
{
   bool prev_polyvertex_inside_q,next_polyvertex_inside_q;
   bool polyvertex_inside_q;
   int k,l=0;

   while (l<n_intersected_poly_sides)
   {
//      cout << "l = " << l << endl;
      k=intersected_poly_side[l];
//      cout << "k = " << k << endl;
      
      if (test_intersection_polygon_vertex(
         j,intersection_point[l++],intersection_poly_vertex))
      {
//         cout << "checkpt 0: j = " << j-1 << endl;
//         cout << "l = " << l << endl;
      }

      if (l<n_intersected_poly_sides)
      {
         k=intersected_poly_side[l];
         if (test_intersection_polygon_vertex(j,intersection_point[l++],
                                              intersection_poly_vertex))
         {
//            cout << "checkpt 1: j = " << j-1 << endl;
//            cout << "l = " << l << endl;
         }
      }

      if (l<n_intersected_poly_sides)
      {
         prev_polyvertex_inside_q=
            q.point_inside_polygon(get_vertex(modulo(k-1,nvertices)));
         polyvertex_inside_q=
            q.point_inside_polygon(get_vertex(modulo(k,nvertices)));
         next_polyvertex_inside_q=
            q.point_inside_polygon(get_vertex(modulo(k+1,nvertices)));
  
/*
         cout << "k = " << k << endl;
         cout << "vertex[modulo(k-1,nvertices)] = " 
              << get_vertex(modulo(k-1,nvertices)) << endl;
         cout << "vertex[modulo(k,nvertices)] = " 
              << get_vertex(modulo(k,nvertices)) << endl;
         cout << "vertex[modulo(k+1,nvertices)] = " 
              << get_vertex(modulo(k+1,nvertices)) << endl;
         cout << "prev_polyvertex_inside_q = "
              << prev_polyvertex_inside_q << endl;
         cout << "polyvertex_inside_q = "
              << polyvertex_inside_q << endl;
         cout << "next_polyvertex_inside_q = "
              << next_polyvertex_inside_q << endl;
*/

         if (polyvertex_inside_q && next_polyvertex_inside_q)
         {
            while(q.point_inside_polygon(get_vertex(modulo(k,nvertices))))
            {
               if (test_intersection_polygon_vertex(
                  j,get_vertex(modulo(k,nvertices)),
                  intersection_poly_vertex))
               {
//                  cout << "checkpt 2: j = " << j-1 << endl;
               }
               k++;
            }
         }
         else if (polyvertex_inside_q && prev_polyvertex_inside_q)
         {
            while(q.point_inside_polygon(get_vertex(modulo(k,nvertices))))
            {
               if (test_intersection_polygon_vertex(
                  j,get_vertex(modulo(k,nvertices)),
                  intersection_poly_vertex))
               {
//                  cout << "checkpt 3: j = " << j-1 << endl;
               }
               k--;
            }
         }
         else if (!polyvertex_inside_q && next_polyvertex_inside_q)
         {
            while(q.point_inside_polygon(get_vertex(modulo(k+1,nvertices))))
            {
               if (test_intersection_polygon_vertex(
                  j,get_vertex(modulo(k+1,nvertices)),
                  intersection_poly_vertex))
               {
//                  cout << "checkpt 4: j = " << j-1 << endl;
               }
               k++;
            }
         }
         else if (!polyvertex_inside_q && prev_polyvertex_inside_q)
         {
            while(q.point_inside_polygon(get_vertex(modulo(k-1,nvertices))))
            {
               if (test_intersection_polygon_vertex(
                  j,get_vertex(modulo(k-1,nvertices)),
                  intersection_poly_vertex))
               {
//                  cout << "checkpt 5: j = " << j-1 << endl;
               }
               k--;
            }
         }
      }
      
   }	// l while loop
}

// ---------------------------------------------------------------------
// Member function test_intersection_polygon_vertex makes sure that
// the input candidate intersection polygon vertex differs from the
// previous intersection polygon vertex:

bool polygon::test_intersection_polygon_vertex(
   int& j,const threevector& candidate_intersection_polygon_vertex,
   threevector* intersection_poly_vertex) const
{
   const double TINY=1E-10;
   bool intersection_polygon_vertex_OK=false;
   
   if (j==0)
   {
      intersection_polygon_vertex_OK=true;
   }
   else
   {
      threevector deltavec(candidate_intersection_polygon_vertex-
         intersection_poly_vertex[j-1]);
      if (deltavec.magnitude() > TINY)
      {
         intersection_polygon_vertex_OK=true;
      }
   }

   if (intersection_polygon_vertex_OK)
   {
      intersection_poly_vertex[j++]=candidate_intersection_polygon_vertex;
//      cout << "j = " << j-1 << " intersection_poly_vertex = "
//           << intersection_poly_vertex[j-1] << endl;
   }
   
   return intersection_polygon_vertex_OK;
}

// ==========================================================================
// Polygon plane member functions
// ==========================================================================

plane* polygon::recompute_plane()
{
   delete plane_ptr;
   plane_ptr=new plane(get_normal(),get_vertex(0));
   return plane_ptr;
}

// ---------------------------------------------------------------------
plane* polygon::set_plane(const fourvector& input_pi)
{
   delete plane_ptr;
   plane_ptr=new plane(input_pi);
   return plane_ptr;
}

// ---------------------------------------------------------------------
bool polygon::lies_on_plane(const plane& input_plane)
{
   bool lies_on_plane_flag=true;
   for (unsigned int i=0; i<nvertices; i++)
   {
      if (!input_plane.point_on_plane(get_vertex(i)))
      {
         lies_on_plane_flag=false;
         break;
      }
   }
   return lies_on_plane_flag;
}

// ==========================================================================
// Polygon projection member functions
// ==========================================================================

// Member function xy_projection returns a polygon whose vertices have
// had their z components set to zero:

polygon polygon::xy_projection() const
{
//   threevector xy_vertex[nvertices];

   vector<threevector> xy_vertices;
   for (unsigned int i=0; i<nvertices; i++)
   {
      xy_vertices.push_back(
         threevector(get_vertex(i).get(0),get_vertex(i).get(1),0));
   }
   return polygon(xy_vertices);
}

// ---------------------------------------------------------------------
// Member function planar_projection returns the projection of the
// current polygon object within input plane p.

polygon polygon::planar_projection(const threevector& nhat) 
{
   threevector P(vertex_average());
   plane p(nhat,P);
   cout << "inside polygon::planar_proj, plane p = " << p << endl;
   return planar_projection(p);
}

polygon polygon::planar_projection(const plane& p) const
{
   vector<threevector> proj_vertex;
   for (unsigned int i=0; i<nvertices; i++)
   {
      proj_vertex.push_back(
         p.projection_into_plane(threevector(get_vertex(i))));
   } // loop over index i labeling polygon vertices
   return polygon(proj_vertex);
}

// ---------------------------------------------------------------------
// Member function generate_bounding_box() returns a pointer to a
// dynamically generated rectangle in the xy-plane which minimally
// surrounds the xy projection of the current polygon object.

polygon* polygon::generate_bounding_box() const
{
   polygon poly_xy=xy_projection();

   double min_x,max_x,min_y,max_y;
   poly_xy.locate_extremal_xy_points(min_x,min_y,max_x,max_y);
 
   polygon* bbox_ptr=new polygon(min_x,min_y,max_x,max_y);
   return bbox_ptr;
}

// ---------------------------------------------------------------------
// Member function zvalue_for_xypoint_in_poly takes in a point whose
// xy projection is assumed to lie inside the current polygon object's
// projection within the xy plane.  It returns the z value for the
// projection of this point onto the polygon in the z direction.  

double polygon::zvalue_for_xypoint_in_poly(const threevector& pnt) const
{
   double z,numer,denom;
   
   if (normal.get(2)==0)
   {
      z=origin.get(2);
   }
   else
   {
      numer=normal.dot(origin)-normal.get(0)*pnt.get(0)
         -normal.get(1)*pnt.get(1);
      denom=normal.get(2);
      z=numer/denom;
   }
   return z;
}

// ---------------------------------------------------------------------
// Boolean member function point_behind_polygon returns false if the
// xy projection of the input point is NOT contained within the xy
// projection of the current polygon object.  It further returns true
// if the z value of the input point which is contained within the xy
// projection of the current polygon object is less than the z value
// of the corresponding point on the polygon.  Otherwise, this
// subroutine returns false if the point lies "in front of" the
// polygon in the z direction:

bool polygon::point_behind_polygon(const threevector& point) 
{
   const double TINY=1E-10;
   bool pnt_behind_poly;
   
   threevector point_xy(point.get(0),point.get(1),0);
   polygon poly_xy(xy_projection());
   if (poly_xy.point_inside_polygon(point_xy))
   {
      if (point.get(2) < zvalue_for_xypoint_in_poly(point_xy)-TINY)
      {
         pnt_behind_poly=true;
      }
      else
      {
         pnt_behind_poly=false;
      }
   }
   else
   {
      pnt_behind_poly=false;
   }
   return pnt_behind_poly;
}

// ---------------------------------------------------------------------
// Member function ray_projected_into_poly_plane takes in a basepoint
// and direction vector for some ray.  It also takes in some point
// which is assumed to lie inside the polygon.  If the ray's direction
// vector is orthogonal to the polygon's normal vector, this boolean
// method return false.  Otherwise, it computes the point within the
// polygon's plane that lies along the ray's path from its basepoint
// and returns the result within output threevector
// projected_point_in_plane.

bool polygon::ray_projected_into_poly_plane(
   const threevector& ray_basepoint,const threevector& ray_hat,
   const threevector& pnt_inside_poly,threevector& projected_point_in_plane)
{
   const double TINY=1E-8;
   double denom=ray_hat.dot(normal);

   if (fabs(denom) < TINY)
   {
      return false;
   }
   else
   {
      double numer=(ray_basepoint-pnt_inside_poly).dot(normal);
      projected_point_in_plane=ray_basepoint-(numer/denom)*ray_hat;
      return true;
   }
}

// ---------------------------------------------------------------------
// Member function projection_into_xy_plane_along_ray takes in
// direction vector ray_hat.  If ray_hat is orthogonal to z_hat, this
// boolean method returns false.  Otherwise, it projects the current
// polygon's vertices along ray_hat into the xy-plane.  The projection
// is returned within output polygon xy_poly_projection.

bool polygon::projection_into_xy_plane_along_ray(
   const threevector& ray_hat,polygon& xy_poly_projection)
{
   const double TINY=1E-8;
   if (ray_hat.get(2) < TINY) 
   {
      return false;
   }
   else
   {
      vector<threevector> v(3);
      v[0]=x_hat;
      v[1]=y_hat;
      v[2]=-x_hat;
      polygon xy_triangle(v);
      
      vector<threevector> projected_vertex_in_xy_plane(nvertices);
      for (unsigned int i=0; i<nvertices; i++)
      {
         xy_triangle.ray_projected_into_poly_plane(
            get_vertex(i),ray_hat,Zero_vector,projected_vertex_in_xy_plane[i]);
      }
      xy_poly_projection=polygon(projected_vertex_in_xy_plane);
      return true;
   }
}

// ---------------------------------------------------------------------
// Member function polygon_difference implements one particular choice
// for a positive semi-definite measure of the difference between two
// polygons possessing the same number of vertices.  It calculates the
// sum of the squared distances between each of the vertices of the
// input polygon and that of the current polygon object.  The square
// root of this sum of squares is taken as the measure of the
// difference between these two polygons.

double polygon::polygon_difference(polygon& poly) const
{
   double sqrd_difference=0;
   for (unsigned int i=0; i<nvertices; i++)
   {
      threevector delta(poly.get_vertex(i)-get_vertex(i));
      sqrd_difference += sqr(delta.magnitude());
   }
   return sqrt(sqrd_difference);
}

// =====================================================================
// Moving around polygons
// =====================================================================

void polygon::translate(const threevector& rvec)
{
   for (unsigned int i=0; i<nvertices; i++)
   {
      set_vertex(i,get_vertex(i)+rvec);
   }
   origin=origin+rvec;
   if (edge != NULL) compute_edges();
}

void polygon::translate(const twovector& rvec)
{
   translate(threevector(rvec));
}

void polygon::absolute_position(const threevector& rvec)
{
   vertex_average();
   for (unsigned int i=0; i<nvertices; i++) 
      set_vertex(i,get_vertex(i)+rvec-vertex_avg);
   origin += rvec-vertex_avg;
   if (edge != NULL) compute_edges();
}

// ---------------------------------------------------------------------
// Member function extend moves each vertex radially outwards from the
// specified extension_origin by some fixed input length:

void polygon::extend(const threevector& extension_origin,double length)
{
   for (unsigned int i=0; i<nvertices; i++)
   {
      threevector dv(get_vertex(i)-extension_origin);
      set_vertex(i,get_vertex(i)+length*dv.unitvector());
   }
   if (edge != NULL) compute_edges();
}

// ---------------------------------------------------------------------
// Member function scale first computes a central origin location for
// a polygon from the average of all its vertices.  It then scales the
// distances of each of the polygon's vertices from this origin by the
// input scalefactor:

void polygon::scale(double scalefactor)
{
   scale(vertex_average(),scalefactor);
}

// ---------------------------------------------------------------------
void polygon::scale(const threevector& scale_origin,double scalefactor)
{
   if (scalefactor >= 0)
   {
      for (unsigned int i=0; i<nvertices; i++)
      {
         threevector dv(get_vertex(i)-scale_origin);
         dv *= scalefactor;
         set_vertex(i,scale_origin+dv);
      }
      threevector dv(origin-scale_origin);
      dv *= scalefactor;
      origin=scale_origin+dv;

// As of 3/2/12, we don't believe it should be necessary to recompute
// normal when polygon is scaled...

//      compute_normal();
      if (edge != NULL) compute_edges();
   }
   else
   {
      cout << "Error inside polygon::scale()!" << endl;
      cout << "scalefactor = " << scalefactor << " < 0 !" << endl;
   }
}

// ---------------------------------------------------------------------
void polygon::scale(const threevector& scale_origin,
                    const threevector& scalefactor)
{
   for (unsigned int i=0; i<nvertices; i++)
   {
      threevector dv(get_vertex(i)-scale_origin);
      dv.scale(scalefactor);
      set_vertex(i,scale_origin+dv);
   }
   threevector dv(origin-scale_origin);
   dv.scale(scalefactor);
   origin=scale_origin+dv;

// As of 3/2/12, we don't believe it should be necessary to recompute
// normal when polygon is scaled...

//   compute_normal();
   if (edge != NULL) compute_edges();
}

// ---------------------------------------------------------------------
void polygon::rotate(const rotation& R)
{
   rotate(Zero_vector,R);
}

// ---------------------------------------------------------------------
// This overloaded version of member function rotate performs a right
// handed rotation of the current point object through angle alpha
// about the axis_direction direction vector:

void polygon::rotate(
   const threevector& rotation_origin,const threevector& axis_direction,
   double alpha)
{
   rotation R(axis_direction,alpha);
   rotate(rotation_origin,R);
}

// ---------------------------------------------------------------------
void polygon::rotate(const threevector& rotation_origin,
                     double thetax,double thetay,double thetaz)
{
   rotation R(thetax,thetay,thetaz);
   rotate(rotation_origin,R);
}

// ---------------------------------------------------------------------
void polygon::rotate(const threevector& rotation_origin,const rotation& R)
{
// First rotate polygon's vertices:

   for (unsigned int i=0; i<nvertices; i++)
   {
      threevector dv(get_vertex(i)-rotation_origin);
      dv=R*dv;
      set_vertex(i,rotation_origin+dv);
   }

// Next rotate polygon's origin:

   threevector dv(origin-rotation_origin);
   dv=R*dv;
   origin=rotation_origin+dv;

   compute_normal();
   recompute_plane();
   if (edge != NULL) compute_edges();
}

// =====================================================================
// Polygon vertex perturbation methods
// =====================================================================

// Member function delta_rotation computes the change in the current
// polygon vertices' positions induced by a rotation about the
// axis_direction direction vector through infinitesimal angle dtheta:

void polygon::delta_rotation(
   const threevector& rotation_origin,const threevector& axis_direction,
   double dtheta,vector<threevector>& dvertex)
{
   for (unsigned int i=0; i<nvertices; i++)
   {
      dvertex[i]=mypoint(get_vertex(i)).delta_rotation(
         rotation_origin,axis_direction,dtheta);
   }
}

// ---------------------------------------------------------------------
// Member function delta_rotation_xy_proj computes the change in the
// current polygon vertices' x and y coordinates induced by a rotation
// about the axis_direction direction vector through infinitesimal
// angle dtheta:

void polygon::delta_rotation_xy_proj(
   const threevector& rotation_origin,const threevector& axis_direction,
   double dtheta,vector<threevector>& dvertex_xy)
{
   vector<threevector> dvertex(nvertices);
   delta_rotation(rotation_origin,axis_direction,dtheta,dvertex);

   for (unsigned int i=0; i<nvertices; i++)
   {
      dvertex_xy[i]=dvertex[i].xy_projection();
   }
}

// ---------------------------------------------------------------------
// Member function delta_rotation_max_xy_proj computes the change in
// the current polygon vertices' x and y coordinates induced by a
// rotation about the axis_direction direction vector through
// infinitesimal angle dtheta:

void polygon::delta_rotation_max_xy_proj(
   const threevector& rotation_origin,const threevector& axis_direction,
   double dtheta,threevector& dvertex_max_xy)
{
   vector<threevector> dvertex_xy(nvertices);
   delta_rotation_xy_proj(rotation_origin,axis_direction,dtheta,dvertex_xy);
   compute_dvertex_max_xy(dvertex_xy,dvertex_max_xy);
}

// ---------------------------------------------------------------------
// Member function frac_xycell_change_induced_by_rotation computes the
// maximum polygon vertex displacement vector in the xy plane induced
// by a rotation about the axis_direction vector by angle dtheta.  It
// then computes this vertex displacement in terms of fractional x and
// y resolution cell lengths deltax and deltay.  This method returns
// the total resolution cell fractional change in output parameter
// frac_change.

void polygon::frac_xycell_change_induced_by_rotation(
   const threevector& rotation_origin,const threevector& axis_direction,
   double dtheta,double deltax,double deltay,double& frac_change)
{
   threevector dvertex_max_xy;
   delta_rotation_max_xy_proj(
      rotation_origin,axis_direction,dtheta,dvertex_max_xy);
   compute_frac_xycell_change(dvertex_max_xy,deltax,deltay,frac_change);
}

// ---------------------------------------------------------------------
// Member function rot_to_induce_one_xycell_change computes the
// rotation angle dtheta needed to induce a maximum one resolution
// cell projected vertex displacement in the xy plane of the current
// polygon object.  This method employs a bisection search approach.

double polygon::rot_to_induce_one_xycell_change(
   const threevector& rotation_origin,const threevector& axis_direction,
   double deltax,double deltay)
{
   double dtheta_mid,dtheta[2];
   dtheta[0]=0;
   dtheta[1]=30*PI/180;

   int ntrials_max=10;
   int ntrials=0;
   double g_mid,frac_change,min_frac_change=1.0;
   while (ntrials < ntrials_max)
   {
      dtheta_mid=0.5*(dtheta[0]+dtheta[1]);
      frac_xycell_change_induced_by_rotation(
         rotation_origin,axis_direction,dtheta_mid,deltax,deltay,frac_change);
      g_mid=frac_change-min_frac_change;
      if (g_mid < 0)
      {
         dtheta[0]=dtheta_mid;
      }
      else
      {
         dtheta[1]=dtheta_mid;
      }
//      cout << "n = " << ntrials << " g = " << g_mid 
//           << " dtheta[0] = " << dtheta[0]*180/PI
//           << " dtheta[1] = " << dtheta[1]*180/PI << endl;
      ntrials++;
   }
//   cout << "dtheta_mid = " << dtheta_mid*180/PI 
//        <<  " g_mid = " << g_mid << endl;
   return dtheta_mid;
}

// ---------------------------------------------------------------------
// Member function delta_scale computes the change in the current
// polygon vertices' positions induced by the input scaling vector
// scalefactor wrt to the origin point scale_origin:

void polygon::delta_scale(
   const threevector& scale_origin,const threevector& scalefactor,
   vector<threevector>& dvertex)
{
   for (unsigned int i=0; i<nvertices; i++)
   {
      dvertex[i]=mypoint(get_vertex(i)).delta_scale(scale_origin,scalefactor);
   }
}

// ---------------------------------------------------------------------
// Member function delta_scale_xy_proj computes the change in the
// current polygon vertices' x and y coordinates induced by the input
// scaling vector scalefactor wrt to the origin point scale_origin:

void polygon::delta_scale_xy_proj(
   const threevector& scale_origin,const threevector& scalefactor,
   vector<threevector>& dvertex_xy)
{
   vector<threevector> dvertex(nvertices);
   delta_scale(scale_origin,scalefactor,dvertex);

   for (unsigned int i=0; i<nvertices; i++)
   {
      dvertex_xy[i]=dvertex[i].xy_projection();
   }
}

// ---------------------------------------------------------------------
// Member function delta_scale_max_xy_proj computes the maximal change
// among the current polygon vertices' x and y coordinates induced by
// the input scaling vector scalefactor wrt to the origin point
// scale_origin:

void polygon::delta_scale_max_xy_proj(
   const threevector& scale_origin,const threevector& scalefactor,
   threevector& dvertex_max_xy)
{
   vector<threevector> dvertex_xy(nvertices);
   delta_scale_xy_proj(scale_origin,scalefactor,dvertex_xy);
   compute_dvertex_max_xy(dvertex_xy,dvertex_max_xy);
}

// ---------------------------------------------------------------------
void polygon::frac_xycell_change_induced_by_scaling(
   const threevector& scale_origin,const threevector& scalefactor,
   double deltax,double deltay,double& frac_change)
{
   threevector dvertex_max_xy;
   delta_scale_max_xy_proj(scale_origin,scalefactor,dvertex_max_xy);
   compute_frac_xycell_change(dvertex_max_xy,deltax,deltay,frac_change);
}

// ---------------------------------------------------------------------
// Member function scaling_to_induce_one_xycell_change returns the
// logarithm of the x scaling factor dlogs needed to induce a maximum
// one resolution cell projected vertex displacement in the xy plane
// of the current polygon object.  This method employs a bisection
// search approach.

double polygon::scaling_to_induce_one_xycell_change(
   const threevector& scale_origin,double deltax,double deltay)
{
   double dlogs[2];
   dlogs[0]=0;
   dlogs[1]=0.2;

   int ntrials_max=10;
   int ntrials=0;
   double dlogs_mid,g_mid,frac_change,min_frac_change=1.0;
   while (ntrials < ntrials_max)
   {
      dlogs_mid=0.5*(dlogs[0]+dlogs[1]);
      double curr_s=pow(10,dlogs_mid);
      threevector scalefactor(curr_s,1,1);
      frac_xycell_change_induced_by_scaling(
         scale_origin,scalefactor,deltax,deltay,frac_change);
      g_mid=frac_change-min_frac_change;
      if (g_mid < 0)
      {
         dlogs[0]=dlogs_mid;
      }
      else
      {
         dlogs[1]=dlogs_mid;
      }
//      cout << "n = " << ntrials << " g = " << g_mid 
//           << " dlogs[0] = " << dlogs[0]
//           << " dlogs[1] = " << dlogs[1] << endl;
      ntrials++;
   }
//   cout << "dlogs_mid = " << dlogs_mid <<  " g_mid = " << g_mid << endl;
   return dlogs_mid;
}

// ---------------------------------------------------------------------
// Utility member function compute_dvertex_max_xy takes in an array of
// vertex perturbations after they have been projected into the xy
// plane.  This method simply scans through the array and returns that
// vertex perturbation whose magnitude is maximal.

void polygon::compute_dvertex_max_xy(
   const vector<threevector>& dvertex_xy,threevector& dvertex_max_xy)
{
   double max_delta_mag=0;
   for (unsigned int i=0; i<nvertices; i++)
   {
      double curr_delta_mag=dvertex_xy[i].magnitude();
      if (curr_delta_mag > max_delta_mag)
      {
         max_delta_mag=curr_delta_mag;
         dvertex_max_xy=dvertex_xy[i];
      }
   }
//   cout << "max changed vertex " << max_changed_vertex 
//        << " dvertex_max_xy = " << dvertex_max_xy << endl;
}

// ---------------------------------------------------------------------
// Utility member function compute_frac_xycell_change takes in the
// maximum polygon vertex displacement vector in the xy plane induced
// by either a rotation or scaling.  It then computes this vertex
// displacement in terms of fractional x and y resolution cell lengths
// deltax and deltay.  This method returns the total resolution cell
// fractional change in output parameter frac_change.

void polygon::compute_frac_xycell_change(
   const threevector& dvertex_max_xy,double deltax,double deltay,
   double& frac_change)
{
   double fx=dvertex_max_xy.get(0)/deltax;
   double fy=dvertex_max_xy.get(1)/deltay;
//   double fx=dvertex_max_xy.get(0)/deltax;
//   double fy=dvertex_max_xy.get(1)/deltay;
   frac_change=sqrt(sqr(fx)+sqr(fy));
}

// ---------------------------------------------------------------------
// Member function generate_polygon_point_cloud() scans over every
// polygon within input STL vector polygon_face.  It computes interior
// XYZ points for each polygon within the STL vector.  This method
// stores this spatial geometry information within the output STL
// vector interior_points.

vector<threevector> polygon::generate_polygon_point_cloud(double ds_frac)
{
//   cout << "inside polygon::generate_polygon_point_cloud()" << endl;
//   cout << "curr poly = " << *this << endl;
   
   rotation Rtrans(natural_coordinate_system());
//   cout << "Rtrans = " << Rtrans << endl;
   
   double max_dist_to_poly_edge=0;
   const double extremal_overstep=-0.01;
   vector<pair<threevector,bool> >* interior_xyz_ptr=
      generate_interior_points_list(
         ds_frac,Rtrans,max_dist_to_poly_edge,true,extremal_overstep);

   vector<threevector> interior_points;
   for (unsigned int j=0; j<interior_xyz_ptr->size(); j++)
   {
      interior_points.push_back((*interior_xyz_ptr)[j].first);
   }
   delete interior_xyz_ptr;

   return interior_points;
}

// ==========================================================================
// General Polygon Clipper (GPC) member functions
// ==========================================================================

gpc_vertex_list* polygon::generate_GPC_vertex_list_from_vertices()
{
   vector<threevector> vertices;
   for (unsigned int n=0; n<nvertices; n++)
   {
      vertices.push_back(get_vertex(n));
   }
   gpc_vertex_list* gpc_vertex_list_ptr=
      geometry_func::generate_GPC_vertex_list(vertices);

   return gpc_vertex_list_ptr;
}

gpc_polygon* polygon::generate_GPC_polygon()
{
   gpc_polygon* gpc_polygon_ptr=new gpc_polygon();

   unsigned int n_contours=1;
   gpc_polygon_ptr->num_contours=n_contours;

   int* hole_ptr=new int[n_contours];
   for (unsigned int c=0; c<n_contours; c++)
   {
      hole_ptr[c]=0;
   }
   gpc_polygon_ptr->hole=hole_ptr;

   gpc_polygon_ptr->contour=generate_GPC_vertex_list_from_vertices();

   return gpc_polygon_ptr;
}

// ==========================================================================
// Polygon triangulation member functions
// ==========================================================================

// Member function generate_Delaunay_triangles() uses the Delaunay
// triangulation built into the 2011 TrianglesGroup class rather than
// much older and deprecated Delaunay triangulation routines from
// "Computational Geometry in C" book by O'Rourke. 

triangles_group* polygon::generate_Delaunay_triangles()
{
   cout << "inside polygon::generate_Delaunay_triangles()" << endl;
   
   delete triangles_group_ptr;
   triangles_group_ptr=new triangles_group;
   
   for (unsigned int n=0; n<get_nvertices(); n++)
   {
      vertex curr_Vertex(get_vertex(n),n);
      triangles_group_ptr->update_triangle_vertices(curr_Vertex);
   } // loop over index n labeling polygon vertices
   
   triangles_group_ptr->delaunay_triangulate_vertices();

   for (unsigned int t=0; t<triangles_group_ptr->get_n_triangles(); t++)
   {
//      cout << "Triangle " << t << endl << endl;
      cout << *(triangles_group_ptr->get_triangle_ptr(t)) << endl;
   }
   return triangles_group_ptr;
}

// ---------------------------------------------------------------------
// Member function generate_interior_XY_triangles() utilizes John
// Ratcliff's static triangulator class in order to triangulate a 2D
// polygon living in the XY plane which has no holes.

triangles_group* polygon::generate_interior_XY_triangles()
{
//   cout << "inside polygon::generate_interior_XY_triangles()" << endl;
   
   delete triangles_group_ptr;
   triangles_group_ptr=new triangles_group;

   vector<double> vertex_Z_values;
   
   for (unsigned int n=0; n<get_nvertices(); n++)
   {
      vertex curr_Vertex(get_vertex(n),n);
//      cout << "n = " << n << " curr_Vertex = " << curr_Vertex << endl;

      triangles_group_ptr->update_triangle_vertices(curr_Vertex);
      vertex_Z_values.push_back(curr_Vertex.get_posn().get(2));
   } // loop over index n labeling polygon vertices
   double median_Z_value=mathfunc::median_value(vertex_Z_values);
   
   triangles_group_ptr->inner_triangulate_vertices(median_Z_value);

//   for (unsigned int t=0; t<triangles_group_ptr->get_n_triangles(); t++)
//   {
//      cout << "Triangle " << t << endl << endl;
//      cout << *(triangles_group_ptr->get_triangle_ptr(t)) << endl;
//   }

   return triangles_group_ptr;
}

// ---------------------------------------------------------------------
// Member function generate_interior_triangles() first rotates the
// currently polygon so that it lies within the XY plane.  It then
// decomposes the rotated polygon into a set of XY triangles.  The
// polygon and triangles are then rotated back to their original plane.

triangles_group* polygon::generate_interior_triangles()
{
//   cout << "inside polygon::generate_interior_triangles()" << endl;
   
   threevector COM=compute_COM();

   rotation R;
   R=R.rotation_taking_u_to_v(get_normal(),z_hat);
//   cout << "R = " << R << endl;
   rotate(COM,R);

   generate_interior_XY_triangles();
   rotate(COM,R.transpose());

   for (unsigned int t=0; t<triangles_group_ptr->get_n_triangles(); t++)
   {
      triangle* triangle_ptr=triangles_group_ptr->get_triangle_ptr(t);
      polygon* triangle_poly_ptr=triangle_ptr->generate_polygon();
      triangle_poly_ptr->rotate(COM,R.transpose());
      
      for (unsigned int v=0; v<triangle_poly_ptr->get_nvertices(); v++)
      {
         triangle_ptr->set_vertex_posn(v,triangle_poly_ptr->get_vertex(v));
      }
              
//      cout << "Triangle " << t << endl << endl;
//      cout << *(triangles_group_ptr->get_triangle_ptr(t)) << endl;
   }

   return triangles_group_ptr;
}

// ---------------------------------------------------------------------
// Member function consolidate_parallel_edges()

void polygon::eliminate_all_parallel_edges()
{
   unsigned int n_iters=get_nvertices();
   unsigned int n_consolidations=0;
   for (unsigned int iter=0; iter<n_iters; iter++)
   {
//      cout << "iter = " << iter << endl;
      bool edges_consolidated_flag=consolidate_parallel_edges();
      if (!edges_consolidated_flag) break;
      n_consolidations++;
   }

   if (n_consolidations > 0)
   {
      cout << "inside polygon::eliminate_all_parallel_edges()" << endl;
      cout << "Original poly had " << n_iters << " vertices" << endl;
      cout << "Consolidated polygon has " << get_nvertices() << " vertices"
           << endl;
      outputfunc::enter_continue_char();
   }
}

// ---------------------------------------------------------------------
bool polygon::consolidate_parallel_edges()
{
   bool edges_consolidated_flag=false;
   int v_unnecessary=-1;
   const double TINY=1E-7;
   for (unsigned int v=0; v<get_nvertices(); v++)
   {
      threevector curr_vertex(get_vertex(v));
      threevector next_vertex(get_vertex(modulo(v+1,get_nvertices())));
      threevector nextnext_vertex(get_vertex(modulo(v+2,get_nvertices())));

      threevector e_hat=(next_vertex-curr_vertex).unitvector();
      threevector f_hat=(nextnext_vertex-next_vertex).unitvector();
      if (e_hat.nearly_equal(f_hat,TINY))
      {
         v_unnecessary=modulo(v+1,get_nvertices());
         edges_consolidated_flag=true;
         break;
      }
   } // loop over index v labeling original vertices

   if (edges_consolidated_flag)
   {
      vector<threevector> V;
      for (int v=0; v<int(get_nvertices()); v++)
      {
         if (v==v_unnecessary) continue;
         V.push_back(get_vertex(v));
         *this=polygon(V);
      }
   }
   return edges_consolidated_flag;
}

