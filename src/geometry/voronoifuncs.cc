// ==========================================================================
// VORONOIFUNCS stand-alone methods
// ==========================================================================
// Last modified on 8/6/06; 12/4/10; 2/17/11
// ==========================================================================

#include <osgUtil/DelaunayTriangulator>
#include "math/basic_math.h"
#include "geometry/convexhull.h"
#include "delaunay/delaunay.h"
#include "image/drawfuncs.h"
#include "datastructures/Linkedlist.h"
#include "datastructures/Mynode.h"
#include "templates/mytemplates.h"
#include "math/threevector.h"
#include "geometry/polygon.h"
#include "geometry/voronoifuncs.h"

#include "general/outputfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

namespace voronoifunc
{

// ---------------------------------------------------------------------
// Method generate_Delaunay_triangle_list calculates Delaunay
// triangulation of the surface defined by all lattice sites within
// input array site[].  It dynamically generates and returns a linked
// list of triangles from this input information.

/*
   Linkedlist<polygon>* generate_Delaunay_triangle_list(
      const vector<threevector>& site)
      {
         int* delaunay_triangle_vertex;
         Linkedlist<polygon>* triangle_list_ptr=
            generate_Delaunay_triangle_list(site,delaunay_triangle_vertex);
         delete [] delaunay_triangle_vertex;
         return triangle_list_ptr;
      }
*/

   vector<polygon> generate_Delaunay_triangles(const vector<threevector>& site)
      {
         int* delaunay_triangle_vertex;
         vector<polygon> triangles=
            generate_Delaunay_triangles(site,delaunay_triangle_vertex);
         delete [] delaunay_triangle_vertex;
         return triangles;
      }
   
/*
   Linkedlist<polygon>* generate_Delaunay_triangle_list(
      const vector<threevector>& site,int*& delaunay_triangle_vertex)
      {
         delaunay::read_delaunay_vertices(site);
         int number_of_delaunay_triangles=
            delaunay::compute_delaunay_triangulation();
         delaunay_triangle_vertex=delaunay::delaunay_triangle_vertices(
            number_of_delaunay_triangles);

         delaunay::delete_circular_lists();
         Linkedlist<polygon>* triangle_list_ptr=
            generate_Delaunay_triangle_list(
               number_of_delaunay_triangles,delaunay_triangle_vertex,site);
         return triangle_list_ptr;
      }
*/

/*
   vector<polygon> generate_Delaunay_triangles(
      const vector<threevector>& site,int*& delaunay_triangle_vertex)
      {
//         cout << "inside voronoifunc::generate_Delaunay_triangles()" << endl;
         
         delaunay::read_delaunay_vertices(site);
         int number_of_delaunay_triangles=
            delaunay::compute_delaunay_triangulation();
//         cout << "number_of_delaunay_triangles = "
//              << number_of_delaunay_triangles << endl;
         
         delaunay_triangle_vertex=delaunay::delaunay_triangle_vertices(
            number_of_delaunay_triangles);
         delaunay::delete_circular_lists();
         return generate_Delaunay_triangles(
               number_of_delaunay_triangles,delaunay_triangle_vertex,site);
      }
*/

// ---------------------------------------------------------------------
// This overloaded version of generate_Delaunay_triangle_list takes in
// integer array delaunay_triangle_vertex[] which contains a
// compactified representation for the Delaunay triangulation of a
// surface.  This method is less user friendly and lower level than
// its counterpart above.

   Linkedlist<polygon>* generate_Delaunay_triangle_list(
      int number_of_delaunay_triangles,int* delaunay_triangle_vertex,
      const vector<threevector>& site)
      {
         vector<threevector> vertex(3);
         Linkedlist<polygon>* triangle_list_ptr=new Linkedlist<polygon>;  
         for (int n=0; n<number_of_delaunay_triangles; n++)
         {
            int i=delaunay_triangle_vertex[3*n+0];
            int j=delaunay_triangle_vertex[3*n+1];
            int k=delaunay_triangle_vertex[3*n+2];
            vertex[0]=site[i];
            vertex[1]=site[j];
            vertex[2]=site[k];
            triangle_list_ptr->append_node(polygon(vertex));

//            cout << "Triangle number = " << n
//                 << " point A: " << i
//                 << " point B: " << j
//                 << " point C: " << k << endl;
//            cout << "n = " << n << " triangle = "
//                 << triangle_list_ptr->get_stop_ptr()->get_data() << endl;
//            cout << "============================================" << endl;
//            outputfunc::newline();
         } // loop over index n labeling Delaunay triangle
         return triangle_list_ptr;
      }

   vector<polygon> generate_Delaunay_triangles(
      int number_of_delaunay_triangles,int* delaunay_triangle_vertex,
      const vector<threevector>& site)
      {
         vector<threevector> vertex(3);
         vector<polygon> triangles;
         for (int n=0; n<number_of_delaunay_triangles; n++)
         {
            int i=delaunay_triangle_vertex[3*n+0];
            int j=delaunay_triangle_vertex[3*n+1];
            int k=delaunay_triangle_vertex[3*n+2];
            vertex[0]=site[i];
            vertex[1]=site[j];
            vertex[2]=site[k];
            triangles.push_back(polygon(vertex));

//            cout << "Triangle number = " << n
//                 << " point A: " << i
//                 << " point B: " << j
//                 << " point C: " << k << endl;
//            cout << "n = " << n << " triangle = "
//                 << triangle_list_ptr->get_stop_ptr()->get_data() << endl;
//            cout << "============================================" << endl;
//            outputfunc::newline();
         } // loop over index n labeling Delaunay triangle
         return triangles;
      }

// ---------------------------------------------------------------------
// Method generate_OSG_Delaunay_triangle_list calculates Delaunay
// triangulation of the surface defined by all lattice sites within
// input STL vector vertices.  It dynamically generates and returns a
// linked list of triangles from this input information.

   Linkedlist<polygon>* generate_OSG_Delaunay_triangle_list(
      const vector<threevector>& vertices)
      {
         int nsites=vertices.size();

         osg::Vec3Array* vertices_ptr=new osg::Vec3Array;
         vertices_ptr->reserve(nsites);
         for (int n=0; n<nsites; n++)
         {
            threevector curr_vertex(vertices[n]);
            vertices_ptr->push_back(
               osg::Vec3(curr_vertex.get(0),curr_vertex.get(1),
                         curr_vertex.get(2)));
         }

         osg::ref_ptr<osgUtil::DelaunayTriangulator> triangulator = new
            osgUtil::DelaunayTriangulator(vertices_ptr);
         triangulator->triangulate();
         osg::DrawElementsUInt* DEUI_ptr=triangulator->getTriangles();

         Linkedlist<polygon>* Delaunay_list_ptr=new Linkedlist<polygon>;
         vector<threevector> triangle_vertices;
         for (unsigned int t=0; t<DEUI_ptr->size()/3; t++)
         {
            for (unsigned int s=0; s<3; s++)
            {
               int i=t*3+s;
               int index=(*DEUI_ptr)[i];
               triangle_vertices.push_back(vertices[index]);
            }
            polygon curr_triangle(triangle_vertices);
            Delaunay_list_ptr->append_node(curr_triangle);
            cout << "curr_triangle = " << curr_triangle << endl;
            triangle_vertices.clear();            
         }

         cout << "Delaunay_list_ptr->size() = "
              << Delaunay_list_ptr->size() << endl;
         
         return Delaunay_list_ptr;
      }

// ---------------------------------------------------------------------
// Method infinite_voronoi_vertices takes in site position information
// within input array site[].  It first generates the sites' convex
// hull and reorders the sites so that the first "n_infinite_points"
// within this array correspond to the convex hull polygon's vertices.
// This method subsequently computes the midpoints along each edge of
// the convex hull as well as the radially outward direction vector at
// those midpoints.  "Infinite" vectors located away from the
// midpoints by input distance parameter infinite_distance_scale along
// the radial directions are calculated and stored within the
// voronoi_list linked list.  This method also returns the number of
// "infinite" Voronoi vertices.

   polygon* infinite_voronoi_vertices(
      double infinite_distance_scale,
      vector<threevector>& site,vector<int>& site_order,
      Linkedlist<delaunay::VoronoiStructure>& voronoi_list)
      {
         polygon* convexhull_ptr=convexhull::convex_hull_poly(
            site,site_order);

         int n_infinite_points=convexhull_ptr->get_nvertices();

         convexhull_ptr->initialize_edge_segments();
         for (unsigned int i=0; i<convexhull_ptr->get_nvertices(); i++)
         {
            linesegment curr_edge(convexhull_ptr->get_edge(i));
            threevector r_hat(curr_edge.get_ehat().cross(
               convexhull_ptr->get_normal()));

            int index1=i+1;
            int index2=i+2;
            if (index2 > n_infinite_points) index2 -= n_infinite_points;
            delaunay::VoronoiStructure curr_voronoi_vertex;
            curr_voronoi_vertex.label[0]=0;
            curr_voronoi_vertex.label[1]=index1;
            curr_voronoi_vertex.label[2]=index2;
            curr_voronoi_vertex.posn=curr_edge.get_midpoint()+
               infinite_distance_scale*r_hat;
//            cout << "infinite_voronoi_vertex i = " << i << endl;
//            cout << "label0 = " << 0 << " label1 = " << index1
//                 << " label2 = " << index2 << endl;
            voronoi_list.append_node(curr_voronoi_vertex);
         }
         return convexhull_ptr;
      }

// ---------------------------------------------------------------------
// Method compute_finite_voronoi_vertices takes in integer array
// delaunay_triangle_vertex[] which contains a compactified
// representation for the Delaunay triangulation of a surface.  The
// first triangle's vertices are loaded into the first 3 elements of
// delaunay_triangle_vertex[].  The 2nd triangle's vertices are
// located into the next 3 elements, etc.  This method computes the
// finite Voronoi vertices associated with each of these Delaunay
// triangles.  (The vertex positions are located at the centers of the
// circles circumscribing the Delaunay triangles.)  Finite Vornoi
// vertex information is appended to the voronoi_list linked list.

   void compute_finite_voronoi_vertices(
      int number_of_delaunay_triangles,int* delaunay_triangle_vertex,
      threevector site[],Linkedlist<delaunay::VoronoiStructure>& voronoi_list)
      {
         for (int i=0; i<number_of_delaunay_triangles; i++)
         {
            delaunay::VoronoiStructure curr_voronoi_vertex;
            curr_voronoi_vertex.label[0]=delaunay_triangle_vertex[3*i+0]+1;
            curr_voronoi_vertex.label[1]=delaunay_triangle_vertex[3*i+1]+1;
            curr_voronoi_vertex.label[2]=delaunay_triangle_vertex[3*i+2]+1;
            circumcircle_center_of_delaunay_triangle(
               site,curr_voronoi_vertex);
            voronoi_list.append_node(curr_voronoi_vertex);

//            cout << "Triangle number = " << i
//                 << " point A: " 
//                 << voronoi_list.get_stop_ptr()->get_data().label[0]
//                 << " point B: " 
//                 << voronoi_list.get_stop_ptr()->get_data().label[1]
//                 << " point C: " 
//                 << voronoi_list.get_stop_ptr()->get_data().label[2]
//                 << endl;
//            cout << "Voronoi vertex posn = " 
//                 << voronoi_list.get_stop_ptr()->get_data().posn << endl;
//            cout << "================================================" << endl;
//            outputfunc::newline();
         } // loop over index i labeling Delaunay triangle
      }

   void compute_finite_voronoi_vertices(
      int number_of_delaunay_triangles,int* delaunay_triangle_vertex,
      vector<threevector>& site,
      Linkedlist<delaunay::VoronoiStructure>& voronoi_list)
      {
         for (int i=0; i<number_of_delaunay_triangles; i++)
         {
            delaunay::VoronoiStructure curr_voronoi_vertex;
            curr_voronoi_vertex.label[0]=delaunay_triangle_vertex[3*i+0]+1;
            curr_voronoi_vertex.label[1]=delaunay_triangle_vertex[3*i+1]+1;
            curr_voronoi_vertex.label[2]=delaunay_triangle_vertex[3*i+2]+1;
            circumcircle_center_of_delaunay_triangle(
               site,curr_voronoi_vertex);
            voronoi_list.append_node(curr_voronoi_vertex);

//            cout << "Triangle number = " << i
//                 << " point A: " 
//                 << voronoi_list.get_stop_ptr()->get_data().label[0]
//                 << " point B: " 
//                 << voronoi_list.get_stop_ptr()->get_data().label[1]
//                 << " point C: " 
//                 << voronoi_list.get_stop_ptr()->get_data().label[2]
//                 << endl;
//            cout << "Voronoi vertex posn = " 
//                 << voronoi_list.get_stop_ptr()->get_data().posn << endl;
//            cout << "================================================" << endl;
//            outputfunc::newline();
         } // loop over index i labeling Delaunay triangle
      }

// ---------------------------------------------------------------------
// Method circumcircle_center_of_delaunay_triangle takes in array
// site[] which contains the integer coordinates for all the input
// points that have been Delaunay triangulated.  It also takes in
// integer i,j,k which label one particular Delaunay triangle.  This
// method returns the integer coordinates of the circle which passes
// through these 3 vertices.

   void circumcircle_center_of_delaunay_triangle(
      const threevector site[],delaunay::VoronoiStructure& voronoi_vertex)
      {

// Recall Voronoi vertex label zero is reserved to indicate a point at
// infinity.  All finite voronoi vertices have labels which start at
// 1.  So we need to subtract off 1 from the vertex labels in order to
// match onto the site numbers which start at index 0:

         int i=voronoi_vertex.label[0]-1;
         int j=voronoi_vertex.label[1]-1;
         int k=voronoi_vertex.label[2]-1;

         double a=site[j].get(0)-site[i].get(0);
         double b=site[j].get(1)-site[i].get(1);
         double c=site[k].get(0)-site[i].get(0);
         double d=site[k].get(1)-site[i].get(1);
         double e=a*(site[i].get(0)+site[j].get(0))
            +b*(site[i].get(1)+site[j].get(1));
         double f=c*(site[i].get(0)+site[k].get(0))
            +d*(site[i].get(1)+site[k].get(1));
         double g=2*(a*(site[k].get(1)-site[j].get(1))
                  -b*(site[k].get(0)-site[j].get(0)));

         const double TINY=1E-6;
         if (fabs(g) < TINY)
         {
            cout << 
               "Trouble in voronoifunc::circumcircle_center_of_delaunay_triangle"
                 << endl;
            cout << "g = " << g << endl;
         }
         double p0=(d*e-b*f)/g;
         double p1=(a*f-c*e)/g;
         voronoi_vertex.posn=threevector(p0,p1);
      }

   void circumcircle_center_of_delaunay_triangle(
      const vector<threevector>& site,
      delaunay::VoronoiStructure& voronoi_vertex)
      {

// Recall Voronoi vertex label zero is reserved to indicate a point at
// infinity.  All finite voronoi vertices have labels which start at
// 1.  So we need to subtract off 1 from the vertex labels in order to
// match onto the site numbers which start at index 0:

         int i=voronoi_vertex.label[0]-1;
         int j=voronoi_vertex.label[1]-1;
         int k=voronoi_vertex.label[2]-1;

         double a=site[j].get(0)-site[i].get(0);
         double b=site[j].get(1)-site[i].get(1);
         double c=site[k].get(0)-site[i].get(0);
         double d=site[k].get(1)-site[i].get(1);
         double e=a*(site[i].get(0)+site[j].get(0))
            +b*(site[i].get(1)+site[j].get(1));
         double f=c*(site[i].get(0)+site[k].get(0))
            +d*(site[i].get(1)+site[k].get(1));
         double g=2*(a*(site[k].get(1)-site[j].get(1))
                  -b*(site[k].get(0)-site[j].get(0)));

         const double TINY=1E-6;
         if (fabs(g) < TINY)
         {
            cout << 
               "Trouble in voronoifunc::circumcircle_center_of_delaunay_triangle"
                 << endl;
            cout << "g = " << g << endl;
         }
         double p0=(d*e-b*f)/g;
         double p1=(a*f-c*e)/g;
         voronoi_vertex.posn=threevector(p0,p1);
      }

// ---------------------------------------------------------------------
// Method report_voronoi_vertices outputs the labels and position for
// each Voronoi vertex within input linked list voronoi_vertex_list:

   void report_voronoi_vertices(
      Linkedlist<delaunay::VoronoiStructure>& voronoi_list)
      {
         Mynode<delaunay::VoronoiStructure>* currnode_ptr=
            voronoi_list.get_start_ptr();

         int n_voronoi_vertex_number=1;
         while (currnode_ptr != NULL)
         {
            cout << "Voronoi vertex number = " 
                 << n_voronoi_vertex_number << endl;
            delaunay::VoronoiStructure curr_voronoi_vertex=
               currnode_ptr->get_data();
            cout << "vertex.label[0] = " << curr_voronoi_vertex.label[0] 
                 << endl;
            cout << "vertex.label[1] = " << curr_voronoi_vertex.label[1] 
                 << endl;
            cout << "vertex.label[2] = " << curr_voronoi_vertex.label[2] 
                 << endl;
            cout << "vertex.posn = " << curr_voronoi_vertex.posn << endl;
            cout << endl;
            currnode_ptr=currnode_ptr->get_nextptr();
            n_voronoi_vertex_number++;
         } // loop over nodes within Voronoi vertex linked list
      }

/*
// ---------------------------------------------------------------------
// Method generate_voronoi_regions returns a linked list of polygons
// which correspond to the Voronoi regions for each site within input
// STL vector site.  The origin of each polygon is set equal to its
// associated lattice site.  The ordering of the polygons in the
// returned list corresponds to the ordering of the sites in the input
// site STL vector.

   Linkedlist<polygon>* generate_voronoi_regions(
      const vector<threevector>& site)
      {

// Instantiate linked list to hold Voronoi vertices:

         Linkedlist<delaunay::VoronoiStructure> voronoi_list;

// Original site ordering within input site[] array is altered by
// subsequent processing in this method.  So we first make a copy of
// this input array.  We also hang onto two arrays which keep track of
// the original and eventual altered site ordering:

         int nsites=site.size();
         vector<int> site_order(nsites),site_orig_order(nsites);
         vector<threevector> site_copy(nsites);
         for (int i=0; i<nsites; i++)
         {
            site_order[i]=site_orig_order[i]=i;
            site_copy[i]=site[i];
         }

// First compute convex hull of all site points.  Use this hull to
// find Voronoi vertices located off at "infinity".  Mark such
// infinite voronoi vertices with label 0.  Save convex hull for
// sanity check on Voronoi region results at the end of this method.

         double infinite_distance_scale=2000;	// meters
         polygon* convexhull_ptr=infinite_voronoi_vertices(
            infinite_distance_scale,site_copy,site_order,voronoi_list);

// Resort site_order array while making corresponding alterations on
// site_orig_order.  The latter array then effectively contains
// inverse transformation information which will allow us at the end
// of this method to convert Voronoi regions back to the original
// sites' order:

         templatefunc::Quicksort(site_order,site_orig_order);
         
// Next compute Delauney triangulation of surface defined by all
// lattice sites:

         delaunay::read_delaunay_vertices(site_copy);
         int number_of_delaunay_triangles=
            delaunay::compute_delaunay_triangulation();
//         cout << "Number of Delaunay triangles = " 
//              << number_of_delaunay_triangles << endl;

         int* delaunay_triangle_vertex=delaunay::delaunay_triangle_vertices(
            number_of_delaunay_triangles);
         delaunay::delete_circular_lists();

// Compute positions of finite Voronoi vertices which are given by the
// centers of circles that circumscribe each Delaunay triangle:

   
         compute_finite_voronoi_vertices(
            number_of_delaunay_triangles,delaunay_triangle_vertex,site_copy,
            voronoi_list);
         delete [] delaunay_triangle_vertex;

// Write out Voronoi vertices with their labels:
   
//         report_voronoi_vertices(voronoi_list);

// Compute Voronoi regions for reordered site locations.  Then
// generate new linked polygon list in which nodes appear in the
// original site order:

         Linkedlist<polygon>* reordered_region_poly_list_ptr=
            generate_voronoi_regions_list(nsites,voronoi_list);

         Linkedlist<polygon>* orig_order_region_poly_list_ptr=
            new Linkedlist<polygon>;

         for (int i=0; i<nsites; i++)
         {
            polygon* curr_poly_ptr=new polygon(
               reordered_region_poly_list_ptr->
               get_node(site_orig_order[i])->get_data());

// In April 2004, we discovered a subtle problem in determining
// Voronoi regions that include at least one Voronoi vertex at
// infinity.  The site associated with an infinite Voronoi region does
// not necessarily lie along the convex hull of all the sites.  For
// example, suppose sites 2, 15 and 3 all reside within neighboring
// infinite Voronoi regions.  If sites 2 and 3 lie on the convex hull
// but site 15 lies inside the hull, then an infinite Voronoi vertex
// with Voronoi region labels 023 will be generated by
// voronoifunc::infinite_voronoi_vertices().  But this same method
// will NOT generate any Voronoi vertex with region labels 0,15,? .
// In this case, *curr_poly_ptr for site 15 currently contains only
// FINITE Voronoi vertices.  But it is missing the infinite vertex
// 023.

// To fix this problem, we explicitly check whether the ith site
// actually resides inside *curr_poly_ptr.  If not, we search for the
// edge on the convex hull which lies closest to site[i].  The number
// for this convex hull edge labels the node within voronoi_list
// containing the missing infinite vertex.  We instantiate a new
// polygon containing the vertices within *curr_poly_ptr as well as
// the missed infinite vertex.  We replace *curr_poly_ptr with this
// enlarged polygon.

            if (!curr_poly_ptr->point_inside_polygon(site[i]))
            {
               cout << "Problem in voronoifunc::generate_voronoi_regions()"
                    << endl;
               cout << "i = " << i << " site[i] = " << site[i] << endl;
               cout << "voronoi_region poly = " << *curr_poly_ptr << endl;
               int closest_convex_hull_edge_number=
                  convexhull_ptr->closest_polygon_edge_to_point(site[i]);
               cout << "closest_convex_hull_edge_number = " 
                    << closest_convex_hull_edge_number << endl;
               Mynode<delaunay::VoronoiStructure>* infinite_voronoi_node_ptr=
                  voronoi_list.get_node(closest_convex_hull_edge_number);
               
               delaunay::VoronoiStructure curr_voronoi_vertex=
                  infinite_voronoi_node_ptr->get_data();

               int n_new_vertices=curr_poly_ptr->get_nvertices()+1;
               vector<threevector> new_vertex(n_new_vertices);
               for (unsigned int j=0; j<curr_poly_ptr->get_nvertices(); j++)
               {
                  new_vertex[j]=curr_poly_ptr->get_vertex(j);
               }
               new_vertex[n_new_vertices-1]=curr_voronoi_vertex.posn;
               polygon* new_voronoi_poly_ptr=new polygon(site[i],new_vertex);
               delete curr_poly_ptr;
               curr_poly_ptr=new_voronoi_poly_ptr;
            } // !curr_poly_ptr->point_inside_poly conditional

            curr_poly_ptr->set_origin(site[i]);
            orig_order_region_poly_list_ptr->append_node(*curr_poly_ptr);
            delete curr_poly_ptr;

         } // loop over index i labeling sites
         delete reordered_region_poly_list_ptr;

// Delete convex hull polygon now that we're done with it...

         delete convexhull_ptr;

         return orig_order_region_poly_list_ptr;
      }
*/

// ---------------------------------------------------------------------
// Method generate_voronoi_regions_list scans through the input
// Voronoi vertex linked list.  It subsequently dynamically generates
// polygons for each Voronoi region whose vertices are those within
// the linked list which share a common label.  This method is more
// low level than the preceding generate_voronoi_regions which is
// intended to be more user friendly.

   Linkedlist<polygon>* generate_voronoi_regions_list(
      int nsites,Linkedlist<delaunay::VoronoiStructure>& voronoi_list)
      {
         vector<threevector> voronoi_region_vertex;
         Linkedlist<polygon>* region_poly_list_ptr=new Linkedlist<polygon>;

         for (int n=1; n<=nsites; n++)
         {
            Mynode<delaunay::VoronoiStructure>* currnode_ptr=
               voronoi_list.get_start_ptr();

            while (currnode_ptr != NULL)
            {
               delaunay::VoronoiStructure curr_voronoi_vertex=
                  currnode_ptr->get_data();
               if (curr_voronoi_vertex.label[0]==n ||
                   curr_voronoi_vertex.label[1]==n ||
                   curr_voronoi_vertex.label[2]==n)
               {
                  voronoi_region_vertex.push_back(curr_voronoi_vertex.posn);
//                  cout << "vertex.label[0] = " 
//                       << curr_voronoi_vertex.label[0] << endl;
//                  cout << "vertex.label[1] = " 
//                       << curr_voronoi_vertex.label[1] << endl;
//                  cout << "vertex.label[2] = " 
//                       << curr_voronoi_vertex.label[2] << endl;
//                  cout << "vertex.posn = " 
//                       << curr_voronoi_vertex.posn << endl;
               }
               currnode_ptr=currnode_ptr->get_nextptr();
            } // loop over nodes within Voronoi vertex linked list

            threevector origin;
            for (unsigned int i=0; i<voronoi_region_vertex.size(); i++)
            {
               origin += voronoi_region_vertex[i];
            }
            origin /= double(voronoi_region_vertex.size());

            polygon voronoi_region(origin,voronoi_region_vertex);
            voronoi_region_vertex.clear();

            region_poly_list_ptr->append_node(voronoi_region);
         } // loop over index n labeling Voronoi region 

         return region_poly_list_ptr;
      }

// ---------------------------------------------------------------------
// Boolean method direct_voronoi_region_neighbors takes in two Voronoi
// region polygons whose origins are assumed to correspond to lattice
// sites.  This method instantiates a linesegment between the two
// sites.  It then performs a brute force search to see whether a
// point along the line segment lies outside of both Voronoi regions.
// If so, this boolean method declares the two regions to NOT be
// immediate neighbors and it returns false.

   bool immediate_voronoi_region_neighbors(
      const threevector& point1,const threevector& point2,
      polygon& region1,polygon& region2,
      double dx,double dy)
      {
//         cout << "inside voronoifunc::immediate_voronoi_region_neighbors()"
//              << endl;
         linesegment l(point1,point2);
//         cout << "point1 = " << point1 << " point2 = " << point2 << endl;
         
         double ds=basic_math::min(dx,dy);
         int nsteps=basic_math::round(l.get_length()/ds);

         bool segment_pierces_more_than_two_voronoi_regions=false;
         for (int n=0; n<nsteps; n++)
         {
            threevector curr_pnt(l.get_v1()+n*ds*l.get_ehat());
            if (region1.point_outside_polygon(curr_pnt,dx,dy) &&
                region2.point_outside_polygon(curr_pnt,dx,dy))
            {
               segment_pierces_more_than_two_voronoi_regions=true;
//               cout << "curr_pnt which lies outside both voronoi regions = "
//                    << curr_pnt << endl;
               break;
            }
         } // loop over index n 
         return !segment_pierces_more_than_two_voronoi_regions;
      }

   
} // voronoifunc namespace


