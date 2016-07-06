// ==========================================================================
// Program HULL
// ==========================================================================
// Last updated on 6/15/04
// ==========================================================================

#include <fstream>
#include <iomanip>
#include <iostream>
#include <math.h>
#include <new>
#include <stdio.h>
#include <string>
#include <time.h>
#include "convexhull.h"
#include "drawfuncs.h"
#include "genfuncs.h"
#include "math/myvector.h"
#include "numrec/nrfuncs.h"
#include "general/sysfuncs.h"
#include "datastructures/twoDarray.h"
#include "urbanimage.h"

// Function declarations:

/*
std::pair<int,int> closest_vertices_to_external_point(
   int n_jump,int i,int& n_expensive_computations,
   const myvector& external_point,polygon const *convex_poly_ptr,
   double external_pnt_vertex_dist[]);

double min_dist_to_convex_polygon(
   const myvector& external_point,polygon const *convex_poly_ptr);
*/

using std::cin;
using std::cout;
using std::endl;
using std::ios;
using std::ofstream;
using std::ostream;
using std::pair;
using std::string;


// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{

   std::set_new_handler(sysfunc::out_of_memory);

// ==========================================================================
// Constant definitions
// ==========================================================================

   const int nxbins=501;
   const int nybins=501;
//   const double max_x=7;  // meters
//   const double max_y=7;
   const double max_x=15;  // meters
   const double max_y=15;
//   const double max_x=20;  // meters
//   const double max_y=20;
	
   urbanimage xyzimage(nxbins,nybins);   
   xyzimage.z2Darray_orig_ptr=new twoDarray(nxbins,nybins);
   xyzimage.z2Darray_ptr=new twoDarray(nxbins,nybins);
   
// Initialize image parameters:

   xyzimage.imagedir=sysfunc::get_cplusplusrootdir()+"alirt/images/voronoi/";
   filefunc::dircreate(xyzimage.imagedir);
   xyzimage.classified=false;
   xyzimage.title="Simulated Ladar Image";
   xyzimage.colortablefilename=sysfunc::get_cplusplusrootdir()
      +"alirt/colortables/colortable.image";
   xyzimage.z2Darray_orig_ptr->init_coord_system(max_x,max_y);
   xyzimage.z2Darray_ptr->init_coord_system(max_x,max_y);
   xyzimage.z2Darray_ptr->initialize_values(NEGATIVEINFINITY);
   twoDarray* ztwoDarray_ptr=xyzimage.z2Darray_ptr;
   
//   int nsites=6;
//   int nsites=10;
   int nsites=20;
//   cout << "Enter number of sites:" << endl;
//   cin >> nsites;
   myvector site[nsites];

/*
   string filename="sites.txt";
   ofstream sitestream;
   openfile(filename,sitestream);

   numrec::init_time_based_seed();
   double scale=5;	// meters
   for (int i=0; i<nsites; i++)
   {
      double random_number1=scale*2*(nrfunc::ran1()-0.5);
      double random_number2=scale*2*(nrfunc::ran1()-0.5);
      site[i]=myvector(random_number1,random_number2);
      sitestream << "(" << random_number1 << "," << random_number2 
                 << ")" << endl;
   }
   closefile(filename,sitestream);
*/

/*
   site[0]=myvector(-10.3,9.2);
   site[1]=myvector(0.2,7.3);
   site[2]=myvector(10.11,10.33);
   site[3]=myvector(-9.23,-8.45);
   site[4]=myvector(1.33,-7.83);
   site[5]=myvector(11.011,-11.01);
*/

/*
   site[0]=myvector(-1.45093,2.24683);
   site[1]=myvector(2.88523,2.55114);
   site[2]=myvector(-3.06403,3.21962);
   site[3]=myvector(-1.77054,2.41086);
   site[4]=myvector(-0.176528,0.288212);
   site[5]=myvector(-3.0856,4.13784);
   site[6]=myvector(-3.96284,-3.50267);
   site[7]=myvector(-4.67862,2.70835);
   site[8]=myvector(0.328086,4.41524);
   site[9]=myvector(-3.54043,-4.11237);
*/

   site[0]=myvector(3.89082,7.50446);
   site[1]=myvector(9.86439,8.26385);
   site[2]=myvector(0.933532,-5.61408);
   site[3]=myvector(1.56889,-0.0503132);
   site[4]=myvector(-9.57009,-4.30894);
   site[5]=myvector(-0.299827,8.76095);
   site[6]=myvector(-9.48171,5.20348);
   site[7]=myvector(7.44078,-9.22343);
   site[8]=myvector(3.43998,7.5054);
   site[9]=myvector(0.589812,4.22743);
   site[10]=myvector(0.799702,3.44581);
   site[11]=myvector(5.49851,6.5087);
   site[12]=myvector(1.80091,-2.97681);
   site[13]=myvector(-9.76876,-1.53231);
   site[14]=myvector(0.180447,-4.92939);
   site[15]=myvector(-7.04221,4.43398);
   site[16]=myvector(-2.84123,9.3725);
   site[17]=myvector(-3.60454,-3.50188);
   site[18]=myvector(8.35617,4.35434);
   site[19]=myvector(-8.27695,-5.55224);

// Generate convex hull poly:
   
   int site_order[nsites];
   for (int n=0; n<nsites; n++)
   {
      site_order[n]=n;
   }
   polygon* hull_ptr=convexhull::convex_hull_poly(nsites,site,site_order);
   drawfunc::draw_polygon(
      *hull_ptr,colorfunc::red,ztwoDarray_ptr);

// Draw original site locations:
  
//   double radius=0.4;
   double radius=0.2;
   for (int i=0; i<nsites; i++)
   {
      drawfunc::draw_hugepoint(site[i],radius,1000,ztwoDarray_ptr);
   }

//   xyzimage.writeimage(
//      "convexhull",ztwoDarray_ptr,false,ladarimage::p_data);

   int n_vertices=hull_ptr->get_nvertices();
   while(true)
   {
      outputfunc::newline();
      double x_point,y_point;
      cout << "Enter x:" << endl;
      cin >> x_point;
      cout << "Enter y:" << endl;
      cin >> y_point;
      myvector external_point(x_point,y_point);
      for (int n=0; n<n_vertices; n++)
      {
         myvector delta=external_point-hull_ptr->vertex[n];
         cout << "Vertex n = " << n << " distance from obs to vertex = "
              << delta.magnitude() << endl;
      }

/*
      int n_expensive_computations=0;
      double ext_pnt_vertex_sqrd_dist[n_vertices];
      for (int n=0; n<n_vertices; n++)
      {
         ext_pnt_vertex_sqrd_dist[n]=-1;
      }
      int i=0;
      pair<int,int> p=hull_ptr->closest_vertices_to_external_point(
         n_vertices,i,n_expensive_computations,external_point,
         ext_pnt_vertex_sqrd_dist);

      cout << "closest vertex index = " << p.first << endl;
      cout << "closest vertex = " << hull_ptr->vertex[p.first] << endl;
      cout << "2nd closest vertex index = " << p.second << endl;
      cout << "2nd closest vertex = " << hull_ptr->vertex[p.second] << endl;
      cout << "Number of expensive computations performed = "
           << n_expensive_computations << endl;
*/

      double min_dist=hull_ptr->min_dist_to_convex_polygon(external_point);
      cout << "Minimum distance = " << min_dist << endl;

   } // while(true) loop
}


/*
// ==========================================================================
// Method min_dist_to_convex_polygon recursively determines the two
// vertices on convex polygon *convex_poly_ptr which lie closest to
// the input external_point.  It subsequently computes and returns the
// distance between the polygon edge defined by those two vertices and
// the external point.

double min_dist_to_convex_polygon(
   const myvector& external_point,polygon const *convex_poly_ptr)
{
   int n_vertices=convex_poly_ptr->get_nvertices();
   int n_expensive_computations=0;
   double ext_pnt_vertex_sqrd_dist[n_vertices];
   for (int n=0; n<n_vertices; n++)
   {
      ext_pnt_vertex_sqrd_dist[n]=-1;
   }
   pair<int,int> p=closest_vertices_to_external_point(
      n_vertices,0,n_expensive_computations,
      external_point,convex_poly_ptr,ext_pnt_vertex_sqrd_dist);
   linesegment poly_edge(convex_poly_ptr->vertex[p.first],
                         convex_poly_ptr->vertex[p.second]);
   return poly_edge.point_to_line_segment_distance(external_point);
}

// ==========================================================================
// Method closest_vertices_to_external_point recursively computes the
// closest vertices on a CONVEX polygon to some observation point
// which is assumed to lie outside the polygon.  The distance metric
// is a periodic function of polygon vertex number, and it generally
// exhibits a single minimum.  So we can employ a recursive binary
// search to locate this minimum in log(n_vertices) time.

std::pair<int,int> closest_vertices_to_external_point(
   int n_jump,int i,int& n_expensive_computations,
   const myvector& external_point,polygon const *convex_poly_ptr,
   double ext_pnt_vertex_sqrd_dist[])
{
   int n_vertices=convex_poly_ptr->get_nvertices();
   int i_prev=modulo(i-1,n_vertices);
   int i_next=modulo(i+1,n_vertices);

// Compute distances from observation point to i_prev, i and i_next
// vertices on convex polygon if they have not already been
// calculated:

   for (int j=-1; j<=1; j++)
   {
      int k=modulo(i+j,n_vertices);
      if (ext_pnt_vertex_sqrd_dist[k]==-1)
      {
         ext_pnt_vertex_sqrd_dist[k]=
            (external_point-convex_poly_ptr->vertex[k]).sqrd_magnitude();
         n_expensive_computations++;
      }
   }

   std::pair<int,int> p;
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
      int n_next_jump=max(1,n_jump/2);
      int next_i=modulo(i+sgn*n_next_jump,n_vertices);
      
      p=closest_vertices_to_external_point(
         n_next_jump,next_i,n_expensive_computations,
         external_point,convex_poly_ptr,ext_pnt_vertex_sqrd_dist);
   }
   return p;
}
      
*/
