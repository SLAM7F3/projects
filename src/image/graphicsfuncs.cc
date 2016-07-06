// ==========================================================================
// GRAPHICSFUNCS stand-alone methods
// ==========================================================================
// Last modified on 5/12/15; 6/16/16; 6/17/16; 6/20/16
// ==========================================================================

#include <algorithm>
#include <connexe/iopnm.h>
#include <connexe/connexe.h>
#include <iostream>
#include <string>

#include "math/basic_math.h"
#include "image/binaryimagefuncs.h"
#include "geometry/contour.h"
#include "geometry/convexhull.h"
#include "image/drawfuncs.h"
#include "image/graphicsfuncs.h"
#include "image/imagefuncs.h"
#include "ladar/ladarimage.h"
#include "templates/mytemplates.h"
#include "general/outputfuncs.h"
#include "geometry/polygon.h"
#include "math/prob_distribution.h"
#include "datastructures/Stack.h"
#include "image/TwoDarray.h"

#include "image/drawfuncs.h"
#include "datastructures/Forest.h"
#include "image/pixel_location.h"
#include "image/pixelForest.h"
#include "video/texture_rectangle.h"
#include "datastructures/treenode.h"
#include "datastructures/union_find.h"

using std::cout;
using std::cin;
using std::endl;
using std::flush;
using std::map;
using std::pair;
using std::string;
using std::vector;

namespace graphicsfunc
{

// Method poor_mans_circle_array takes in an (odd) integer which
// specifies the number of pixel rows which the poor man's circle is
// to contain.  It returns a dynamically generated integer array which
// specifies the number of columns as a function of row number
// corresponding to a crude circle centered about some pixel location.

   int* poor_mans_circle_array(unsigned int n)
      {
         if (is_even(n)) n++;

         int* column;
         new_clear_array(column,n);
         
         if (modulo(n,4)==1)
         {
            unsigned int j=(n-1)/4;
            unsigned int m=2*j+1;
            for (unsigned int k=0; k<j; k++)
            {
               column[k]=column[n-1-k]=n+2*(k-j);
            }
            for (unsigned int k=0; k<m; k++)
            {
               column[j+k]=n;
            }
         } // n mod 4 == 1 conditional
         else
         {
            unsigned int j=(n+1)/4;
            unsigned int m=2*j-1;
            for (unsigned int k=0; k<j; k++)
            {
               column[k]=column[n-1-k]=n+2*(k-j);
            }
            for (unsigned int k=0; k<m; k++)
            {
               column[j+k]=n;
            }
         }
         return column;
      }

// ---------------------------------------------------------------------
// Little utility method compute_delta_s_values fills up the
// one-dimensional delta_s array with ds=sqrt(dx**2+dy**2) for the 8
// nearest pixels neighbors.

   vector<double> compute_delta_s_values(double dx,double dy)
      {
//         cout << "inside graphicsfunc::compute_delta_s_values()" << endl;
         vector<double> delta_s;

         for (int i=-1; i<=1; i++)
         {
            double delta_x=abs(i)*dx;
            for (int j=-1; j<=1; j++)
            {
               double delta_y=abs(j)*dy;
               delta_s.push_back(sqrt(sqr(delta_x)+sqr(delta_y)));
//               cout << "delta_s = " << delta_s.back() << endl;
            }
         }
         return delta_s;
      }

// ---------------------------------------------------------------------
// Boolean method point_near_hot_pixels scans a n_window_size x
// n_window_size neighborhood centered upon the pixel corresponding to
// input point curr_pnt.  If the number of neighbor non-zero valued
// pixels within input *zbinary_twoDarray_ptr exceeds input parameter
// min_hot_pixels, this method returns true.

   bool point_near_hot_pixels(
      int min_hot_pixels,double znull,
      const threevector& curr_pnt,twoDarray const *zbinary_twoDarray_ptr,
      int n_window_size)
      {
         const double epsilon=1E-3;

         unsigned int px0,py0;
         int n_nearby_hot_pixels=0;
         if (zbinary_twoDarray_ptr->point_to_pixel(curr_pnt,px0,py0))
         {
            int half_window_size=n_window_size/2;
            for (unsigned int px=px0-half_window_size; 
                 px<=px0+half_window_size; px++)
            {
               for (unsigned int py=py0-half_window_size; 
                    py<=py0+half_window_size; py++)
               {
                  if (zbinary_twoDarray_ptr->pixel_inside_working_region(
                     px,py))
                  {
                     if (zbinary_twoDarray_ptr->get(px,py) > znull+epsilon) 
                     {
                        n_nearby_hot_pixels++;
                        if (n_nearby_hot_pixels > min_hot_pixels)
                        {
                           return true;
                        }
                     }
                  }
               } // loop over py loop
            } // loop over px index
         }
         return false;
      }

// ---------------------------------------------------------------------
// Method height_percentile returns the 95th percentile height of the
// non-zero valued pixels in the vicinity of input location curr_pnt.

   double height_percentile(
      double znull,const threevector& curr_pnt,
      twoDarray const *zbinary_twoDarray_ptr,twoDarray const *ztwoDarray_ptr)
      {
         const double epsilon=1E-3;
         const double frac=0.95;

         unsigned int px0,py0;
         int n_nearby_hot_pixels=0;
         int window_size=3;

// NOTE: This next max window size is a only a reasonable guess.  It
// is possible (though unlikely) that the window size could grow
// larger than 21...6/25/04 at 10:34 am

         int max_window_size=21;
         double pixel_height[sqr(max_window_size)];
         double avg_height=-999;

         if (zbinary_twoDarray_ptr->point_to_pixel(curr_pnt,px0,py0))
         {
            do 
            {
               for (unsigned int px=px0-window_size/2; px<=px0+window_size/2; 
                    px++)
               {
                  for (unsigned int py=py0-window_size/2; 
                       py<=py0+window_size/2; py++)
                  {
                     if (zbinary_twoDarray_ptr->pixel_inside_working_region(
                        px,py))
                     {
                        if (zbinary_twoDarray_ptr->get(px,py) > 
                            znull+epsilon) 
                        {
                           pixel_height[n_nearby_hot_pixels++]=
                              ztwoDarray_ptr->get(px,py);
                        }
                     }
                  } // loop over py loop
               } // loop over px index
               window_size += 2;
            }
            while (n_nearby_hot_pixels==0);
            
            if (n_nearby_hot_pixels==0)
            {
               cout << "Error in graphicsfunc::height_percentile()" << endl;
               cout << "n_nearby_hot_pixels=0" << endl;
               avg_height=-999;
            }
            else if (n_nearby_hot_pixels==1)
            {
               avg_height=pixel_height[0];
            }
            else
            {
               prob_distribution prob_z(n_nearby_hot_pixels,pixel_height,10);
               avg_height=prob_z.find_x_corresponding_to_pcum(frac);
            }
         }
         else
         {
            cout << "Trouble in graphicsfunc::height_percentile()" << endl;
            cout << "Input point not inside image window!" << endl;
         }
         
         return avg_height;
      }
   
// ==========================================================================
// Contour methods
// ==========================================================================

// Method connected_region_convex_hull takes in an image within
// *ztwoDarray_ptr which is assumed to contain a dense set of
// connected pixels.  This method returns a polygon representing the
// convex hull of this pixel set.

   polygon* connected_region_convex_hull(twoDarray const *ztwoDarray_ptr)
      {
         return connected_region_convex_hull(
            ztwoDarray_ptr,0,ztwoDarray_ptr->get_mdim(),
            0,ztwoDarray_ptr->get_ndim());
      }
   
   polygon* connected_region_convex_hull(
      twoDarray const *ztwoDarray_ptr,
      int min_px,int max_px,int min_py,int max_py)
      {
         Linkedlist<std::pair<int,int> >* pixel_list_ptr=
            binaryimagefunc::binary_image_to_list(ztwoDarray_ptr);
         polygon* hull_poly_ptr=convexhull::convex_hull_poly(pixel_list_ptr);
         delete pixel_list_ptr;
         ztwoDarray_ptr->convert_pixel_to_posn_polygon_vertices(
            hull_poly_ptr);
         return hull_poly_ptr;
      }

// ---------------------------------------------------------------------
// Method shrink_wrap_regularized_contour takes in contour c, binary
// tree pixel classification information in *zbinary_twoDarray_ptr.
// The contour is assumed to be initialized to the convex hull of some
// dense cluster of non-null pixels within the binary image.  This
// method loops over the vertices within the contour (which are
// assumed to have already been evenly distributed along the hull).
// Those contour vertices which lie directly along the pixel cluster's
// perimeter are left alone.  But the remaining ones are iteratively
// migrated radially inwards towards the cluster of tree pixels.  The
// iteration terminates when all of the contour pixels lie on the
// cluster's perimeter.  The "shrink wrapped" contour is returned
// within contour c.

// Note added on Monday, April 18, 2005: After playing around with
// chunk33-39, we discovered that contours surrounding very
// complicated tree clusters can intersect themselves during the
// process of shrink wrapping.  We really need to search for contour
// self-intersection points and break apart a single contour into
// multiple components where these points occur.  As of 4/18/05, we do
// not have time to do this.  But we must come back to this issue in
// the future and clean this mess up.

   void shrink_wrap_regularized_contour(
      double znull,contour& c,twoDarray const *zbinary_twoDarray_ptr,
      double delta_r,unsigned int n_max_iters)
      {
         cout << "inside graphicsfunc::shrink_warp_regularized_contour()"
              << endl;
         
         const double edge_length=c.get_edge(0).get_length();
         cout << "edge_length = " << edge_length << endl;
         
         unsigned int iter=0;
         unsigned int nvertices_on_perimeter=0;
         
         do 
         {
            unsigned int nvertices=c.get_nvertices();
            nvertices_on_perimeter=0;
            for (unsigned int i=0; i<nvertices; i++)
            {
               threevector r_hat(c.radial_direction_vector(i));
               threevector e_hat=c.get_edge(i).get_ehat();
               threevector normal_test(r_hat.cross(e_hat));

               double dotproduct=normal_test.dot(c.get_normal());
               if (sgn(dotproduct <= 0))
               {
                  cout << "normal = " << c.get_normal() << endl;
                  cout << "e_hat = " << e_hat << endl;
                  cout << "r_hat = " << r_hat << endl;
                  cout << "e_hat.r_hat = " << e_hat.dot(r_hat) << endl;
                  cout << "normal.r_hat = " << c.get_normal().dot(r_hat) 
                       << endl;
                  cout << "sgn[(r_hatxe_hat).n_hat] = "
                       << sgn(dotproduct) << endl;
               }
             
               bool vertex_on_perimeter=point_near_hot_pixels(
                  1,znull,c.get_vertex(i).first,zbinary_twoDarray_ptr);
               
               if (!vertex_on_perimeter)
               {
                  threevector candidate_vertex=c.get_vertex(i).first-
                     delta_r*r_hat;

// On 11/10/10, we found that the following condition fouls up contour
// fitting to indoor packbot ladar data for the RASR project.  So we
// comment it out...
                  
/*
// Make sure candidate vertex is no closer than edge length to all
// other existing vertices:

                  for (unsigned int j=0; j<nvertices; j++)
                  {
                     if (j != i)
                     {
                        threevector delta(
                           c.get_vertex(j).first-candidate_vertex);
                        if (delta.sqrd_magnitude() < 0.5*edge_length)
                        {
                           cout << "Reseting candidate vertex to orig"
                                << endl;
                           candidate_vertex=c.get_vertex(i).first;
                        }
                     }
                  }
*/

//                  cout << "candidate vertex = " << candidate_vertex << endl;
                  c.set_vertex(i,candidate_vertex);
//                     threevector long_rvector=-5*r_hat;
//                     drawfunc::draw_vector(
//                        long_rvector,c.get_vertex(i).first,colorfunc::white,
//                        zbinary_twoDarray_ptr);
               }
               else
               {
                  nvertices_on_perimeter++;
               }
            } // loop over index i labeling contour vertices
            iter++;

            c.compute_edges();
            c.regularize_vertices(edge_length);

            cout <<  "iter = " << iter 
                 << " n_max_iters = " << n_max_iters
                 << endl;
            cout << "nvertices_on_perim = " << nvertices_on_perimeter << endl;

         } // nvertices_on_perimeter < nvertices while loop
         while (nvertices_on_perimeter < c.get_nvertices() && 
                iter < n_max_iters);

         c.guarantee_right_handed_vertex_ordering();
         c.robust_locate_origin();
      }

// ---------------------------------------------------------------------
// Method dilate_regularized_contour takes in regularized contour c
// along with binary feature information within
// *zbinary_twoDarray_ptr.  The contour is assumed to lie strictly
// inside some zero-valued region within the binary image.  This
// method loops over the vertices within the contour (which are
// assumed to have already been evenly distributed).  Those contour
// vertices which lie directly along the pixel cluster's perimeter are
// left alone.  But the remaining ones are iteratively migrated
// radially outwards towards the zero-valued region's periphery.  The
// iteration terminates when all of the contour pixels lie on the
// hole's perimeter.  The dilated contour is returned within contour
// c.

   int dilate_regularized_contour(
      double znull,contour& c,twoDarray const *zbinary_twoDarray_ptr)
      {
         const unsigned int n_max_iters=1000;
         const double delta_r=0.33;	// meter
         const double edge_length=c.get_edge(0).get_length();

         unsigned int iter=0;
         unsigned int nvertices_on_perimeter=0;
         
         do 
         {
            cout << iter << " " << flush;
            unsigned int nvertices=c.get_nvertices();
            nvertices_on_perimeter=0;
            for (unsigned int i=0; i<nvertices; i++)
            {
               threevector r_hat(c.radial_direction_vector(i));
               bool vertex_on_perimeter=point_near_hot_pixels(
                  1,znull,c.get_vertex(i).first,zbinary_twoDarray_ptr,3);
               if (!vertex_on_perimeter)
               {
                  c.set_vertex(i,c.get_vertex(i).first+delta_r*r_hat);
               }
               else
               {
                  nvertices_on_perimeter++;
               }
            } // loop over index i labeling contour vertices
            iter++;
            c.compute_edges();
            c.regularize_vertices(edge_length);
            c.shrink_inwards(0.05*delta_r);

         } // nvertices_on_perimeter < nvertices while loop
         while (nvertices_on_perimeter < 0.95*c.get_nvertices() && 
                iter < n_max_iters);

         c.robust_locate_origin();
         return iter;
      }

// ---------------------------------------------------------------------
// Method avg_heights_at_contour_vertices computes local 85th
// percentile heights are stored within the z values of each vertex in
// the deformed contour.  This height information can be used for
// elevated cylinder display purposes.

   void avg_heights_at_contour_vertices(
      double znull,contour& c,twoDarray const *zbinary_twoDarray_ptr,
      twoDarray const *ztwoDarray_ptr)
      {
         for (unsigned int i=0; i<c.get_nvertices(); i++)
         {
            double avg_height=height_percentile(
               znull,c.get_vertex(i).first,
               zbinary_twoDarray_ptr,ztwoDarray_ptr);
            c.set_vertex(i,threevector(
               c.get_vertex(i).first.get(0),c.get_vertex(i).first.get(1),
               avg_height),c.get_vertex(i).second);
         } // loop over index i labeling contour vertices
      }

// ---------------------------------------------------------------------
// Method edge_gradient_contour_integral takes in edge label n for
// some contour c along with an input image within *ztwoDarray_ptr.
// It returns the edge gradient line integral for this particular
// edge.  

   double edge_gradient_contour_integral(
      int n,double correlation_length,const contour& c,
      twoDarray const *ztwoDarray_ptr)
      {
         const linesegment curr_edge(c.get_edge(n));
         return curr_edge.get_length()*imagefunc::edge_gradient(
            correlation_length,curr_edge,-PI/2.0,ztwoDarray_ptr);
      }
   
// ==========================================================================
// Turtle perimeter finding methods
// ==========================================================================

// Method turtle_boundary implements Papert's turtle algorithm for
// tracing closed boundaries in binary images.  Input twoDarray
// *zbinary_twoDarray_ptr is assumed to contain a connected binary
// region.  The turtle starts at some pixel located immediately
// outside the white region.  It then moves around the boundary
// according to the following rule: If turtle is located on a white
// [black] pixel, turn left [right] and step.  The turtle terminates
// when it returns to its starting location.

   vector<pair<int,int>> new_turtle_boundary(
      twoDarray* zorig_binary_twoDarray_ptr)
      {
//         cout << "inside graphicsfunc::new_turtle_boundary()" << endl;

// First null all pixels located along image border to prevent turtle
// from ever moving outside allowed region:

         twoDarray* zbinary_twoDarray_ptr=new twoDarray(
            zorig_binary_twoDarray_ptr);
         zorig_binary_twoDarray_ptr->copy(zbinary_twoDarray_ptr);

         const unsigned int mdim=zbinary_twoDarray_ptr->get_mdim();
         const unsigned int ndim=zbinary_twoDarray_ptr->get_ndim();
         for (unsigned int px=0; px<mdim; px++)
         {
            zbinary_twoDarray_ptr->put(px,0,0);
            zbinary_twoDarray_ptr->put(px,ndim-1,0);
         }
         for (unsigned int py=0; py<ndim; py++)
         {
            zbinary_twoDarray_ptr->put(0,py,0);
            zbinary_twoDarray_ptr->put(mdim-1,py,0);
         }

// Find some non-zero valued pixel along connected binary region's
// periphery:

         int px,py;
         binaryimagefunc::locate_first_nonzero_pixel(
            zbinary_twoDarray_ptr,px,py);
         twovector curr_pixel_posn(px,py);
//         cout << "curr_pixel_posn = " << curr_pixel_posn << endl;

// Travel northwards until positioned on a black pixel located along
// region's periphery:
         
         twovector heading[4];
         heading[0]=twovector(1,0);	// east
         heading[1]=twovector(0,-1);	// north
         heading[2]=twovector(-1,0);	// west
         heading[3]=twovector(0,1);	// south

         int heading_index=1;	// Initially travel north
         twovector curr_heading(heading[heading_index]);

         do
         {
            curr_pixel_posn += curr_heading;
            px=basic_math::round(curr_pixel_posn.get(0));
            py=basic_math::round(curr_pixel_posn.get(1));
//            cout <<  "px = " << px << " py = " << py << endl;
         }
         while (zbinary_twoDarray_ptr->get(px,py) > 0);

// Instantiate STL vector to hold perimeter pixel positions:

         vector<pair<int,int> > perim_pixel_posns;

// Keep track of boundary points as they're marked within the
// following twoDarray to prevent double counting:

         twoDarray* marked_points_twoDarray_ptr=new twoDarray(
            zorig_binary_twoDarray_ptr);

// Let turtle wander around connected region's boundary until it
// returns to its starting point:

         int counter=0;
         int px_first=px;
         int py_first=py;
         twovector curr_posn;
         while (counter == 0 || !(px==px_first && py==py_first))
         {
            int qsize = 2;
            for(int qy = -qsize; qy <= qsize; qy++)
            {
               for(int qx = -qsize; qx <= qsize; qx++)
               {
                  int curr_Z = zbinary_twoDarray_ptr->get(px+qx,py+qy);
                  cout << curr_Z << "  ";
               }
               cout << endl;
            }

            cout << "curr_heading = " << curr_heading << endl;
            outputfunc::enter_continue_char();
            
            if (zbinary_twoDarray_ptr->get(px,py) > 0)
            {
               if (nearly_equal(marked_points_twoDarray_ptr->get(px,py),0))
               {
                  pair<int,int> P;
                  P.first = px;
                  P.second = py;
                  perim_pixel_posns.push_back(P);
                  marked_points_twoDarray_ptr->put(px,py,1);
               }
            }

//            cout << "i = " << counter << " px = " << px << " py = " << py
//                 << " x = " << curr_posn.get(0) << " y = " << curr_posn.get(1)
//                 << " zbinary = " << zbinary_twoDarray_ptr->get(px,py) 
//                 << endl;
               
//            cout << "heading.x = " << curr_heading.get(0)
//                 << " heading.y = " << curr_heading.get(1) 
//                 << " heading index = " << heading_index << endl << endl;

// At each pixel, turtle turns left [right] when positioned on top of
// a white [black] pixel:

            if (zbinary_twoDarray_ptr->get(px,py) > 0)
            {
               heading_index=modulo(heading_index+1,4);
            }
            else
            {
               heading_index=modulo(heading_index-1,4);
            }
            curr_heading=heading[heading_index];

            curr_pixel_posn += curr_heading;
            px=basic_math::round(curr_pixel_posn.get(0));
            py=basic_math::round(curr_pixel_posn.get(1));

            counter++;
         } // while loop

         delete zbinary_twoDarray_ptr;
         delete marked_points_twoDarray_ptr;
         
         return perim_pixel_posns;
      }

// ---------------------------------------------------------------------
   Linkedlist<threevector>* turtle_boundary(
      twoDarray* zorig_binary_twoDarray_ptr)
      {
         cout << "inside graphicsfunc::turtle_boundary()" << endl;
//         outputfunc::write_banner("Computing turtle boundary:");

//          twoDarray* zperim_twoDarray_ptr=zorig_binary_twoDarray_ptr;

// First null all pixels located along image border to prevent turtle
// from ever moving outside allowed region:

         twoDarray* zbinary_twoDarray_ptr=new twoDarray(
            zorig_binary_twoDarray_ptr);
         zorig_binary_twoDarray_ptr->copy(zbinary_twoDarray_ptr);

         const unsigned int mdim=zbinary_twoDarray_ptr->get_mdim();
         const unsigned int ndim=zbinary_twoDarray_ptr->get_ndim();
         for (unsigned int px=0; px<mdim; px++)
         {
            zbinary_twoDarray_ptr->put(px,0,0);
            zbinary_twoDarray_ptr->put(px,ndim-1,0);
         }
         for (unsigned int py=0; py<ndim; py++)
         {
            zbinary_twoDarray_ptr->put(0,py,0);
            zbinary_twoDarray_ptr->put(mdim-1,py,0);
         }

// Find some non-zero valued pixel along connected binary region's
// periphery:

         int px,py;
         binaryimagefunc::locate_first_nonzero_pixel(
            zbinary_twoDarray_ptr,px,py);
         threevector curr_pixel_posn(px,py);
//         cout << "curr_pixel_posn = " << curr_pixel_posn << endl;

// Travel northwards until positioned on a black pixel located along
// region's periphery:
         
         threevector heading[4];
         heading[0]=threevector(1,0);	// east
         heading[1]=threevector(0,-1);	// north
         heading[2]=threevector(-1,0);	// west
         heading[3]=threevector(0,1);	// south

         int heading_index=1;	// Initially travel north
         threevector curr_heading(heading[heading_index]);

         do
         {
            curr_pixel_posn += curr_heading;
            px=basic_math::round(curr_pixel_posn.get(0));
            py=basic_math::round(curr_pixel_posn.get(1));
//            cout <<  "px = " << px << " py = " << py << endl;
         }
         while (zbinary_twoDarray_ptr->get(px,py) > 0);

// Instantiate linked list to hold perimeter pixel positions:

         Linkedlist<threevector>* perim_posn_list_ptr=
            new Linkedlist<threevector>;
//         cout << "perim_posn_list_ptr = " << perim_posn_list_ptr << endl;

// Keep track of boundary points as they're marked within the
// following twoDarray to prevent double counting:

         twoDarray* marked_points_twoDarray_ptr=new twoDarray(
            zorig_binary_twoDarray_ptr);

// Let turtle wander around connected region's boundary until it
// returns to its starting point:

//         bool found_first_nonzero_pixel=false;
         int counter=0;
         int px_first=px;
         int py_first=py;
         threevector curr_posn;
         while (!(px==px_first && py==py_first))
         {
            zbinary_twoDarray_ptr->pixel_to_point(px,py,curr_posn);
            if (zbinary_twoDarray_ptr->get(px,py) > 0)
            {
/*
               if (!found_first_nonzero_pixel)
               {
                  px_first=px;
                  py_first=py;
                  found_first_nonzero_pixel=true;
//                  threevector first_point;
//                  zbinary_twoDarray_ptr->pixel_to_point(
//                     px_first,py_first,first_point);
//                  zperim_twoDarray_ptr->put(px,py,1);
//                  drawfunc::draw_hugepoint(
//                     first_point,2,5,zperim_twoDarray_ptr);
               }
*/

               if (nearly_equal(marked_points_twoDarray_ptr->get(px,py),0))
               {
                  perim_posn_list_ptr->append_node(threevector(curr_posn));
                  marked_points_twoDarray_ptr->put(px,py,1);
//                  zperim_twoDarray_ptr->put(px,py,2);
               }
            }

//            cout << "i = " << counter << " px = " << px << " py = " << py
//                 << " x = " << curr_posn.get(0) << " y = " << curr_posn.get(1)
//                 << " zbinary = " << zbinary_twoDarray_ptr->get(px,py) 
//                 << endl;
               
//            cout << "heading.x = " << curr_heading.get(0)
//                 << " heading.y = " << curr_heading.get(1) 
//                 << " heading index = " << heading_index << endl << endl;

// At each pixel, turtle turns left [right] when positioned on top of
// a white [black] pixel:

            if (zbinary_twoDarray_ptr->get(px,py) > 0)
            {
               heading_index=modulo(heading_index+1,4);
            }
            else
            {
               heading_index=modulo(heading_index-1,4);
            }
            curr_heading=heading[heading_index];

            curr_pixel_posn += curr_heading;
            px=basic_math::round(curr_pixel_posn.get(0));
            py=basic_math::round(curr_pixel_posn.get(1));

            counter++;
         } // while loop

         delete zbinary_twoDarray_ptr;
         delete marked_points_twoDarray_ptr;
         
         return perim_posn_list_ptr;
      }

// ---------------------------------------------------------------------
// Method convert_turtle_boundary_to_contour downsamples the turtle
// perimeter positions contained within input linked list
// *turtle_list_ptr.  It effectively selects turtle points which are
// separated by input parameter delta_s.  The subsampled perimeter
// points are then used to instantiate a dynamically generated contour
// which is returned by this method.

   contour* convert_turtle_boundary_to_contour(
      double delta_s,Linkedlist<threevector> const *turtle_list_ptr)
      {
//         outputfunc::write_banner("Converting turtle boundary to contour:");
         Linkedlist<threevector>* perim_posn_list_ptr=
            new Linkedlist<threevector>;
         perim_posn_list_ptr->append_node(
            turtle_list_ptr->get_start_ptr()->get_data());
         
         for (const Mynode<threevector>* currnode_ptr=
                 turtle_list_ptr->get_start_ptr(); currnode_ptr != NULL; 
              currnode_ptr=currnode_ptr->get_nextptr())
         {
            threevector curr_point(currnode_ptr->get_data());
            if ((curr_point-perim_posn_list_ptr->get_stop_ptr()->get_data()).
                magnitude() > delta_s)
            {
               perim_posn_list_ptr->append_node(curr_point);
            }
         } // loop over nodes in turtle linked list
         contour* turtle_contour_ptr=new contour(perim_posn_list_ptr);
         delete perim_posn_list_ptr;
         return turtle_contour_ptr;
      }
   
// ---------------------------------------------------------------------
// Method contour_for_enclosed_region takes in a binary image within
// *ztwoDarray_ptr which we assume essentially contains a lattice
// superposed on empty background.  The lattice's border value equals
// z_boundary, while the empty background's value equals z_null.  This
// method also takes in some origin point which is assume to lie
// inside some enclosed cell within the lattice.  

// This method first fills in the lattice cell region via a recursive
// "paint program" flooding procedure.  It then uses the turtle
// algorithm to extract the filled region's perimeter.  Finally, it
// downsamples the turtle perimeter to obtain a closed contour
// surrounding the region whose edge length equals input parameter
// delta_s.

   contour* contour_surrounding_enclosed_region(
      double z_null,double z_boundary,double delta_s,
      const threevector& origin,twoDarray const *ztwoDarray_ptr)
      {
         int npixels_filled;
         double z_fill=0.5*z_boundary;
         twoDarray* fmask_twoDarray_ptr=mask_boundaryFill(
            z_fill,z_null,origin,ztwoDarray_ptr,npixels_filled);
         Linkedlist<threevector>* turtle_list_ptr=
            turtle_boundary(fmask_twoDarray_ptr);
         delete fmask_twoDarray_ptr;
         
//         cout << "turtle list = " << *turtle_list_ptr << endl;

         contour* c_ptr=convert_turtle_boundary_to_contour(
            delta_s,turtle_list_ptr);
         delete turtle_list_ptr;
         c_ptr->regularize_vertices(delta_s);         
         return c_ptr;
      }

// ---------------------------------------------------------------------
// Method turtle_erode_binary_region takes in binary image
// *zbinary_twoDarray_ptr along with the number of iterations n_iters
// to be performed and the pixel size nsize of the mask window.  For
// each iteration, it computes the turtle boundary of the non-zero
// valued region within *zbinary_twoDarray_ptr.  It subsequently
// performs an erosion operation for those pixels within the turtle
// boundary.  In order to minimize execution time, this method skips
// over nsize/2 turtle boundary pixels whenever it performs an
// nsize x nsize nulling operation.  

   void turtle_erode_binary_region(
      unsigned int n_iters,int nsize,twoDarray* zbinary_twoDarray_ptr,
      double znull)
      {
         outputfunc::write_banner("Turtle eroding binary region:");
         
         for (unsigned int iter=0; iter<n_iters; iter++)
         {
            Linkedlist<threevector>* boundary_list_ptr=turtle_boundary(
               zbinary_twoDarray_ptr);
            int counter=0;
            for (Mynode<threevector>* currnode_ptr=boundary_list_ptr->
                    get_start_ptr(); currnode_ptr != NULL; 
                 currnode_ptr=currnode_ptr->get_nextptr())
            {
               if (counter==0)
               {
                  threevector curr_posn=currnode_ptr->get_data();
                  unsigned int px,py;
                  if (zbinary_twoDarray_ptr->point_to_pixel(curr_posn,px,py))
                  {
                     for (int qx=int(px-nsize/2); qx<= int(px+nsize/2); qx++)
                     {
                        for (int qy=int(py-nsize/2); qy<= int(py+nsize/2); 
                             qy++)
                        {
                           if (zbinary_twoDarray_ptr->
                               pixel_inside_working_region(qx,qy))
                           {
                              zbinary_twoDarray_ptr->put(qx,qy,znull);
                           }
                        } // loop over qy index
                     } // loop over qx index
                     counter=nsize/2;
                  } // (px,py) inside working region conditional
               }
               else
               {
                  counter--;
               } // counter==0 conditional
            } // loop over nodes in turtle boundary list
            delete boundary_list_ptr;
         } // loop over iter loop
      }

// ==========================================================================
// Scan filling methods
// ==========================================================================

// Method mask_boundaryFill returns a dynamically generated twoDarray
// containing just the filled region with all other pixel values set
// equal to znull.

   twoDarray* mask_boundaryFill(
      double zfill,double znull,const threevector& origin,
      twoDarray const *ztwoDarray_ptr,int& npixels_filled)
      {
         twoDarray* zmask_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
         ztwoDarray_ptr->copy(zmask_twoDarray_ptr); 

         mask_boundaryFill(
            zfill,znull,origin,ztwoDarray_ptr,
            zmask_twoDarray_ptr,npixels_filled);
         return zmask_twoDarray_ptr;
      }

   void mask_boundaryFill(
      double zfill,double znull,const threevector& origin,
      twoDarray const *ztwoDarray_ptr,twoDarray* zmask_twoDarray_ptr,
      int& npixels_filled)
      {
         unsigned int px,py;
         ztwoDarray_ptr->point_to_pixel(origin,px,py);
         npixels_filled=basicfill(px,py,zfill,zmask_twoDarray_ptr);
         imagefunc::particular_cutoff_threshold(
            zfill,zmask_twoDarray_ptr,znull);
      }

// ---------------------------------------------------------------------
// NOTE ADDED ON 7/2/2012: WE STRONGLY BELIEVE THAT THIS TINT FILL
// METHOD RETURNED INVALID RESULTS FOR A CONNECTED COMPONENT
// CONSISTING OF ONE PIXEL LOCATED ABOVE ANOTHER.  SO THIS METHOD IS
// DEPRECATED.  USE THE MORE GENERAL LABEL_CONNECTED_COMPONENTS()
// METHOD BELOW INSTEAD!

// The "scanline oriented" algorithm implemented in method basicfill
// is given in detailed pseudocode in "Tint fill" by Alvy Ray Smith,
// ACM O-8791-004 (1979) 276.  It tries to fill along a scanline
// before it changes pixel coordinate py.  Basicfill fills all pixels
// 4-connected to the seedpoint on the first scanline.  It then looks
// at the scanlines above and below for points 4-connected to the same
// scanline segment just filled (and of the same color as the
// seedpoint pixel before it was filled).  A number of these points
// sufficient to guarantee connectivity is maintained on a stack by
// basicfill.  When the scans are finished, a point is popped from the
// stack and becomes a new seedpoint.  Only one point per scanline
// segment to be filled need be pushed.

   int basicfill(int seed_x,int seed_y,double zfill,
                 twoDarray* ztwoDarray_ptr)
   {
      int min_px,max_px,min_py,max_py;
      return basicfill(
         seed_x,seed_y,zfill,ztwoDarray_ptr,
         min_px,max_px,min_py,max_py);
   }

// This overloaded version of basicfill returns the pixel bounding box
// surrounding the 4-connected region:
   
   int basicfill(
      int seed_x,int seed_y,double zfill,twoDarray* ztwoDarray_ptr,
      int& min_px,int& max_px,int& min_py,int& max_py)
      {
//         cout << "inside graphicsfunc::basicfill()" << endl;
         
         int px,py,lx,rx; // lx & rx are the left and right x pixel 
			  //  coordinates of a scanline segment

         min_px=POSITIVEINFINITY;
         max_px=NEGATIVEINFINITY;
         min_py=POSITIVEINFINITY;
         max_py=NEGATIVEINFINITY;
         
         int npixels_filled=0;

         px=seed_x;
         py=seed_y;
         double new_value=zfill;
         double old_value=ztwoDarray_ptr->get(px,py);
//         cout << "new_value = " << new_value << " old_value = " << old_value
//              << endl;
         if (nearly_equal(old_value,new_value)) return 0;
         
         Stack<pair<int,int> > pixel_stack(1000);
         pixel_stack.push(pair<int,int>(px,py));
         pair<int,int> pixel_pair;
         while (!pixel_stack.isEmpty())
         {
            pixel_stack.pop(pixel_pair);
            int qx=pixel_pair.first;
            int qy=pixel_pair.second;
            
            if (!nearly_equal(ztwoDarray_ptr->get(qx,qy),new_value))
            {
               int prev_npixels_filled=npixels_filled;
               fill_line(qx,qy,old_value,new_value,ztwoDarray_ptr,lx,rx,
                         npixels_filled);
               if (npixels_filled > prev_npixels_filled)
               {
                  min_px=basic_math::min(min_px,lx);
                  max_px=basic_math::max(max_px,rx);
                  min_py=basic_math::min(min_py,qy);
                  max_py=basic_math::max(max_py,qy);
               }

               scanhi(qy,lx,rx,old_value,ztwoDarray_ptr,pixel_stack);
               scanlo(qy,lx,rx,old_value,ztwoDarray_ptr,pixel_stack);
            }
         } // pixel_stack not empty while loop
//         cout << "min_px = " << min_px << " max_px = " << max_px << endl;
//         cout << "min_py = " << min_py << " max_py = " << max_py << endl;

         return npixels_filled;
      }

   void fill_line(
      unsigned int qx,unsigned int qy,
      const double old_value,const double new_value,
      twoDarray* ztwoDarray_ptr,int& lx,int& rx,
      int& npixels_filled)
      {
         rx=fill_right(qx,qy,old_value,new_value,ztwoDarray_ptr,
                       npixels_filled);
         lx=fill_left(qx,qy,old_value,new_value,ztwoDarray_ptr,
                      npixels_filled);
      }
   
   int fill_right(
      unsigned int qx,unsigned int qy,
      const double old_value,const double new_value,
      twoDarray* ztwoDarray_ptr,int& npixels_filled)
      {
         unsigned int px=qx;
         unsigned int py=qy;
         while (px < ztwoDarray_ptr->get_mdim() && 
         nearly_equal(ztwoDarray_ptr->get(px,py),old_value))
         {
            ztwoDarray_ptr->put(px,py,new_value);
            npixels_filled++;
            px++;
         }
         return px-1;
      }
   
   int fill_left(
      unsigned int qx,unsigned int qy,
      const double old_value,const double new_value,
      twoDarray* ztwoDarray_ptr,int& npixels_filled)
      {
         unsigned int px=qx-1;
         unsigned int py=qy;
         while (px >= 0 && nearly_equal(ztwoDarray_ptr->get(px,py),old_value))
         {
            ztwoDarray_ptr->put(px,py,new_value);
            npixels_filled++;
            px--;
         }
         return px+1;
      }
   
// The "shadow" of a scanline segment is the set of pixels just under
// (or just above) the pixels in the segment.  A scanline segment of
// length n pixels has two shadows (except for one that lies on the
// very top or bottom rows of the image) each of length n.  Methods
// scanlo (or scanhi) stacks only one point from each scanline segment
// which is 4-connected to the scanline segment just filled with
// method fill_line.  This point is the leftmost point in each
// scanline segment, or subset of a scanline segment, in the shadow.

   void scanhi(
      unsigned int qy,unsigned int lx,unsigned int rx,
      const double old_value,twoDarray* ztwoDarray_ptr,
      Stack<pair<int,int> >& pixel_stack)
      {
//         cout << "inside graphicsfunc::scanhi(), qy = " << qy
//              << " old_value = " << old_value 
//              << " lx = " << lx << " rx = " << rx 
//              << endl;
         
         if (qy+1 > ztwoDarray_ptr->get_ndim()) return;
         unsigned int px=lx;
         unsigned int py=qy+1;

         while (px < rx)
         {
            while (!nearly_equal(ztwoDarray_ptr->get(px,py),old_value) &&
                   px < rx) px++;
            if (px > rx) break;
            pixel_stack.push(pair<int,int>(px,py));            
            while (nearly_equal(ztwoDarray_ptr->get(px,py),old_value) &&
                   px < rx) px++;
         }
      }
   
   void scanlo(
      unsigned int qy,unsigned int lx,unsigned int rx,
      const double old_value,twoDarray* ztwoDarray_ptr,
      Stack<pair<int,int> >& pixel_stack)
      {
         if (qy-1 < 0) return;
         unsigned int px=lx;
         unsigned int py=qy-1;
         while (px < rx)
         {
            while (!nearly_equal(ztwoDarray_ptr->get(px,py),old_value) &&
                   px < rx) px++;
            if (px > rx) break;
            pixel_stack.push(pair<int,int>(px,py));            
            while (nearly_equal(ztwoDarray_ptr->get(px,py),old_value) &&
                   px < rx) px++;
         }
      }

// ==========================================================================
// Connected components labeling methods
// ==========================================================================

// Method label_connected_components() takes in binary-thresholded
// array *ptwoDarray_ptr which is assumed to be integer valued.  It
// also takes in the number of neighbors (4,8) which define
// connectivity in the binary input image.

// This method calls the connected components extraction routines of
// Gregoir Malandain.  This method returns within
// *cc_labels_twoDarray_ptr integer labels corresponding to the
// non-null valued connected components within *ptwoDarray_ptr.  It
// also fills hash_map labels_map with CC IDs, CC labels and number of
// pixels per CC.

   void label_connected_components(
      int n_neighbors, int label_offset, const twoDarray* ptwoDarray_ptr,
      int curr_class_ID, LABELS_MAP& labels_map, 
      twoDarray* cc_labels_twoDarray_ptr)
      {
//         cout << "inside graphicsfunc::label_connected_components()" << endl;

         unsigned int xdim=ptwoDarray_ptr->get_xdim();
         unsigned int ydim=ptwoDarray_ptr->get_ydim();
//         cout << "xdim = " << xdim << " ydim = " << ydim << endl;

// Fill linear array input_buffer with unsigned chars
// corresponding to contents of *ptwoDarray_ptr:

         unsigned char* input_buffer=new unsigned char[xdim*ydim];   

         int counter=0;
         for (unsigned int py=0; py<ydim; py++)
         {
            for (unsigned int px=0; px<xdim; px++)
            {
//               cout << "px = " << px <<" py = " << py
//                    << " p(px,py) = " << ptwoDarray_ptr->get(px,py) << endl;
               unsigned char curr_uchar=
                  stringfunc::ascii_integer_to_unsigned_char(
                  ptwoDarray_ptr->get(px,py));

               input_buffer[counter++]=curr_uchar;
               
            } // loop over px index
//            cout << endl;
         } // loop over py index

         int bufferDims[3] = {xdim,ydim,sizeof(unsigned char)};
//         cout << "bufferDims[0] = " << bufferDims[0] << endl;
//         cout << "bufferDims[1] = " << bufferDims[1] << endl;
//         cout << "bufferDims[2] = " << bufferDims[2] << endl;

         Connexe_SetConnectivity( n_neighbors );

         int size = 1;
         Connexe_SetMinimumSizeOfComponents( size );

         int number = -1;
         Connexe_SetMaximumNumberOfComponents( number );

//         Connexe_verbose();

// In order to handle more than 255 connected components, we need to
// work with an output_buffer of at least 2-byte unsigned shorts
// rather than 1-byte unsigned chars:

//         unsigned char* output_buffer=new unsigned char[xdim*ydim];   
         unsigned short* output_buffer=new unsigned short[xdim*ydim];   
//         float* output_buffer=new float[xdim*ydim];   
//         double* output_buffer=new double[xdim*ydim];   

         int n_ccs = CountConnectedComponents( 
            input_buffer, UCHAR,
            output_buffer, USHORT,
//            output_buffer, FLOAT,
//            output_buffer, DOUBLE,
            bufferDims );

         if (n_ccs < 0)
         {
            cout << "Failure in graphicsfunc::label_connected_components()" 
                 << endl;
            cout << "Number of connected components = " << n_ccs << endl;
            exit( 1 );
         }
         delete [] input_buffer;

// Transfer connected component labels from linear array output_buffer
// into output *cc_labels_twoDarray_ptr:

         counter=0;
         int max_label=-1;
         for (unsigned int py=0; py<ydim; py++)
         {
            for (unsigned int px=0; px<xdim; px++)
            {
//               int curr_label=stringfunc::unsigned_char_to_ascii_integer(
//                  output_buffer[counter++]);
               int curr_label=output_buffer[counter++];

               if (curr_label > 0)
               {
                  curr_label += label_offset;
                  cc_labels_twoDarray_ptr->put(px,py,curr_label);

                  LABELS_MAP::iterator label_iter=labels_map.find(curr_label);
                  if(label_iter == labels_map.end())
                  {
                     pair<int,int> P;
                     P.first = curr_class_ID;
                     P.second = 1;
                     labels_map[curr_label] = P;
                  }
                  else
                  {
                     label_iter->second.first = curr_class_ID;
                     label_iter->second.second = label_iter->second.second+1;
                  }
               }
               max_label=basic_math::max(max_label,curr_label);

            } // loop over px index
         } // loop over py index

         delete [] output_buffer;
      }

// ---------------------------------------------------------------------
// Method Label_Connected_Components() follows Algorithm 1 within
// "Building the component tree in quasi-linear time" by Najman and
// Couprie (2006).  It uses the non-tree implementation of the
// Union-Find algorithms built into the union_find class.  The number
// of connected components is returned by this method.

   int Label_Connected_Components(
      int n_neighbors,int label_offset,double z_null,
      const twoDarray* pbinary_twoDarray_ptr,
      twoDarray* cc_labels_twoDarray_ptr)
      {
//         cout << "inside graphicsfunc::Label_Connected_Components()" << endl;
//         cout << "label_offset = " << label_offset << endl;

         union_find* union_find_ptr=new union_find();

         unsigned int xdim=pbinary_twoDarray_ptr->get_xdim();
         unsigned int ydim=pbinary_twoDarray_ptr->get_ydim();
         for (unsigned int px=0; px<xdim; px++)
         {
            for (unsigned int py=0; py<ydim; py++)
            {
               double curr_z=pbinary_twoDarray_ptr->get(px,py);
               if (curr_z <= z_null) continue;
               int node_ID=get_pixel_ID(px,py,xdim);
               union_find_ptr->MakeSet(node_ID);
            }
         }
//         cout << "n_nodes = " << union_find_ptr->get_n_nodes() << endl;

         unsigned int px,py;
         int node_ID=union_find_ptr->reset_curr_node_iterator();
         vector<int> neighbor_IDs;
         while (node_ID >= 0)
         {
            int root_ID=union_find_ptr->Find(node_ID);
            get_pixel_px_py(node_ID,xdim,px,py);

            if (n_neighbors==4)
            {
               compute_four_neighbor_IDs(px,py,xdim,ydim,neighbor_IDs);
            }
            else if (n_neighbors==8)
            {
               compute_eight_neighbor_IDs(px,py,xdim,ydim,neighbor_IDs);
            }

            for (unsigned int n=0; n<neighbor_IDs.size(); n++)
            {
               int neighbor_root_ID=union_find_ptr->Find(neighbor_IDs[n]);
               if (neighbor_root_ID==-1) continue;

               if (root_ID != neighbor_root_ID)
               {
                  union_find_ptr->Link(neighbor_root_ID,root_ID);
               }
            } // loop over index n labeling 4-neighbors for current node

            node_ID=union_find_ptr->increment_curr_node_iterator();
         } // node_ID while loop

// Transfer connected component labels into *cc_labels_twoDarray_ptr:

         int curr_cc_label=label_offset;
         int next_cc_label=label_offset;

         typedef map<int,int> LABELS_MAP;
         LABELS_MAP* labels_map_ptr=new LABELS_MAP;

         cc_labels_twoDarray_ptr->clear_values();
         for (unsigned int px=0; px<xdim; px++)
         {
            for (unsigned int py=0; py<ydim; py++)
            {
               double curr_z=pbinary_twoDarray_ptr->get(px,py);
               if (curr_z <= z_null) continue;

               int node_ID=get_pixel_ID(px,py,xdim);
               int curr_cc_ID=union_find_ptr->Find(node_ID);

               LABELS_MAP::iterator label_iter=labels_map_ptr->find(
                  curr_cc_ID);
               if (label_iter==labels_map_ptr->end())
               {
                  next_cc_label++;
                  (*labels_map_ptr)[curr_cc_ID]=next_cc_label;
                  curr_cc_label=next_cc_label;
               }
               else
               {
                  curr_cc_label=label_iter->second;
               }
               cc_labels_twoDarray_ptr->put(px,py,curr_cc_label);
               
            } // loop over py index
         } // loop over px index
	       
         delete union_find_ptr;

         unsigned int n_components=labels_map_ptr->size();
         delete labels_map_ptr;

         return n_components;
      }

// ---------------------------------------------------------------------
   void lexicographical_four_neighbor_pixels(
      unsigned int px,unsigned int py,unsigned int xdim,unsigned int ydim,
      vector<pair<unsigned int,unsigned int> >& four_neighbor_pixels)
   {
      four_neighbor_pixels.clear();

      if (py >= 1)
      {
         pair<unsigned int,unsigned int> P(px,py-1);
         four_neighbor_pixels.push_back(P);
      }
      if (px >= 1)
      {
         pair<unsigned int,unsigned int> P(px-1,py);
         four_neighbor_pixels.push_back(P);
      }
   }

// ---------------------------------------------------------------------
   void compute_four_neighbor_pixels(
      unsigned int px,unsigned int py,unsigned int xdim,unsigned int ydim,
      vector<pair<unsigned int,unsigned int> >& four_neighbor_pixels)
   {
      four_neighbor_pixels.clear();

      if (py >= 1)
      {
         pair<int,int> P(px,py-1);
         four_neighbor_pixels.push_back(P);
      }
      if (py <= ydim-2)
      {
         pair<int,int> P(px,py+1);
         four_neighbor_pixels.push_back(P);
      }
      if (px >= 1)
      {
         pair<int,int> P(px-1,py);
         four_neighbor_pixels.push_back(P);
      }
      if (px <= xdim-2)
      {
         pair<int,int> P(px+1,py);
         four_neighbor_pixels.push_back(P);
      }
   }

// ---------------------------------------------------------------------
   void compute_four_neighbor_IDs(
      unsigned int px,unsigned int py,unsigned int xdim,unsigned int ydim,
      vector<int>& four_neighbor_IDs)
   {
      four_neighbor_IDs.clear();

      if (py >= 1)
      {
         four_neighbor_IDs.push_back(get_pixel_ID(px,py-1,xdim));
      }
      if (py <= ydim-2)
      {
         four_neighbor_IDs.push_back(get_pixel_ID(px,py+1,xdim));
      }
      if (px >= 1)
      {
         four_neighbor_IDs.push_back(get_pixel_ID(px-1,py,xdim));
      }
      if (px <= xdim-2)
      {
         four_neighbor_IDs.push_back(get_pixel_ID(px+1,py,xdim));
      }
   }

// ---------------------------------------------------------------------
   void lexicographical_eight_neighbor_pixels(
      unsigned int px,unsigned int py,unsigned int xdim,unsigned int ydim,
      vector<pair<unsigned int,unsigned int> >& eight_neighbor_pixels)
   {
      eight_neighbor_pixels.clear();

      if (py >= 1)
      {
         if (px >= 1) 
         {
            pair<unsigned int,unsigned int> P(px-1,py-1);
            eight_neighbor_pixels.push_back(P);
         }
         pair<unsigned int,unsigned int> P(px,py-1);
         eight_neighbor_pixels.push_back(P);
         if (px <= xdim-2)
         {
            pair<unsigned int,unsigned int> P(px+1,py-1);
            eight_neighbor_pixels.push_back(P);
         }
      }
      if (px >= 1)
      {
         pair<unsigned int,unsigned int> P(px-1,py);
         eight_neighbor_pixels.push_back(P);
      }
   }

// ---------------------------------------------------------------------
   void compute_eight_neighbor_pixels(
      unsigned int px,unsigned int py,unsigned int xdim,unsigned int ydim,
      vector<pair<unsigned int,unsigned int> >& eight_neighbor_pixels)
   {
      eight_neighbor_pixels.clear();

      if (py >= 1)
      {
         if (px >= 1) 
         {
            pair<unsigned int,unsigned int> P(px-1,py-1);
            eight_neighbor_pixels.push_back(P);
         }
         pair<unsigned int,unsigned int> P(px,py-1);
         eight_neighbor_pixels.push_back(P);
         if (px <= xdim-2)
         {
            pair<unsigned int,unsigned int> P(px+1,py-1);
            eight_neighbor_pixels.push_back(P);
         }
      }
      if (px >= 1)
      {
         pair<unsigned int,unsigned int> P(px-1,py);
         eight_neighbor_pixels.push_back(P);
      }
      if (px <= xdim-2)
      {
         pair<unsigned int,unsigned int> P(px+1,py);
         eight_neighbor_pixels.push_back(P);
      }

      if (py <= ydim-2)
      {
         if (px >= 1)
         {
            pair<unsigned int,unsigned int> P(px-1,py+1);
            eight_neighbor_pixels.push_back(P);
         }
         pair<unsigned int,unsigned int> P(px,py+1);
         eight_neighbor_pixels.push_back(P);
         if (px <= xdim-2)
         {
            pair<unsigned int,unsigned int> P(px+1,py+1);
            eight_neighbor_pixels.push_back(P);
         }
      }
   }

// ---------------------------------------------------------------------
   void compute_eight_neighbor_IDs(
      int px,int py,int xdim,int ydim,vector<int>& eight_neighbor_IDs)
   {
      eight_neighbor_IDs.clear();

      if (py >= 1)
      {
         if (px >= 1)
            eight_neighbor_IDs.push_back(get_pixel_ID(px-1,py-1,xdim));
         eight_neighbor_IDs.push_back(get_pixel_ID(px,py-1,xdim));
         if (px <= xdim-2)
            eight_neighbor_IDs.push_back(get_pixel_ID(px+1,py-1,xdim));
      }

      if (px >= 1)
         eight_neighbor_IDs.push_back(get_pixel_ID(px-1,py,xdim));
      if (px <= xdim-2)
         eight_neighbor_IDs.push_back(get_pixel_ID(px+1,py,xdim));

      if (py <= ydim-2)
      {
         if (px >= 1)
            eight_neighbor_IDs.push_back(get_pixel_ID(px-1,py+1,xdim));
         eight_neighbor_IDs.push_back(get_pixel_ID(px,py+1,xdim));
         if (px <= xdim-2)
            eight_neighbor_IDs.push_back(get_pixel_ID(px+1,py+1,xdim));
      }
   }

// ---------------------------------------------------------------------
// Method run_length_encode takes in a binary thresholded twoDarray.
// It encodes each run of 1-pixels by its row and the columns of its
// starting and ending pixels.  A dummy permutation label holding the
// component label of each run is also initialized to zero.  The RLE
// binary image is returned in a dynamically generated linked list.
// This method follows the RLE conventions spelled out in section
// 2.3.6 of "Computer and Robot Vision" by Haralick and Shapiro (TA
// 1632.H37 vol 1, 1992).  It is needed for binary image connected
// components labeling.  

   vector<int> run_length_encode(
      int cc_label,unsigned int px_start,unsigned int px_stop,
      unsigned int py_start,unsigned int py_stop,
      double threshold,const twoDarray* cc_twoDarray_ptr)
      {
         vector<int> V;

         int xdim=cc_twoDarray_ptr->get_xdim();

         for (unsigned int py=py_start; py<=py_stop; py++)
         {
            bool running=false;
            int start_ID=-1;
            int stop_ID=-1;
            for (unsigned int px=px_start; px<=px_stop; px++)
            {
               if (int(cc_twoDarray_ptr->get(px,py))==cc_label)
               {
                  if (!running)
                  {
                     running=true;
                     start_ID=get_pixel_ID(px,py,xdim);
                  }
               }
               else
               {
                  if (running)
                  {
                     stop_ID=get_pixel_ID(px-1,py,xdim);
                     running=false;
                     V.push_back(start_ID);
                     V.push_back(stop_ID);
                  }
               }
            } // loop over px index

// Terminate any runs which reach end of binary image's current row:

            if (running)
            {
               stop_ID=get_pixel_ID(px_stop,py,xdim);
               V.push_back(start_ID);
               V.push_back(stop_ID);
            }
         } // loop over py index 
         return V;
      }

// ---------------------------------------------------------------------
// Method twopass_cc_labeling() implements the "two-pass" algorithm
// presented in the connected components labeling wikipage.

   int twopass_cc_labeling(
      int n_neighbors,int label_offset,double z_null,
      const twoDarray* pbinary_twoDarray_ptr,
      twoDarray* cc_labels_twoDarray_ptr)
      {
         cout << "inside graphicsfunc::twopass_cc_labeling()" << endl;
//         cout << "label_offset = " << label_offset << endl;
//         cout << "n_neighbors = " << n_neighbors << endl;

         unsigned int xdim=pbinary_twoDarray_ptr->get_xdim();
         unsigned int ydim=pbinary_twoDarray_ptr->get_ydim();

/*
// Print initial binary image:

         cout << "pbinary_twoDarray = " << endl;
         for (unsigned int py=0; py<ydim; py++)
         {
            for (unsigned int px=0; px<xdim; px++)
            {
               double curr_z=pbinary_twoDarray_ptr->get(px,py);
               cout << curr_z << "  " << flush;
            }
            cout << endl;
         }
         cout << endl;
*/

         union_find* union_find_ptr=new union_find();	// = "linked"
         cc_labels_twoDarray_ptr->initialize_values(z_null);
//         cout << "z_null = " << z_null << endl;

// First pass: Record connected component equivalence pixel relations
// within *union_find_ptr:

         int cc_label=1;
         vector<pair<unsigned int,unsigned int> > 
            neighbor_pixels,samevalue_neighbor_pixels;
         for (unsigned int py=0; py<ydim; py++)
         {
            for (unsigned int px=0; px<xdim; px++)
            {
               double curr_z=pbinary_twoDarray_ptr->get(px,py);
               
               if (curr_z <= z_null) continue;
//               cout << "px = " << px << " py = " << py
//                    << " curr_z = " << curr_z << endl;

               neighbor_pixels.clear();
               if (n_neighbors==4)
               {
                  lexicographical_four_neighbor_pixels(
                     px,py,xdim,ydim,neighbor_pixels);
               }
               else if (n_neighbors==8)
               {
                  lexicographical_eight_neighbor_pixels(
                     px,py,xdim,ydim,neighbor_pixels);
               }
//               cout << "neighbor_pixels.size() = " << neighbor_pixels.size()
//                    << endl;

               samevalue_neighbor_pixels.clear();
               for (unsigned int n=0; n<neighbor_pixels.size(); n++)
               {
                  unsigned int qx=neighbor_pixels[n].first;
                  unsigned int qy=neighbor_pixels[n].second;
                  if (pbinary_twoDarray_ptr->get(qx,qy) == curr_z) 
                  {
                     samevalue_neighbor_pixels.push_back(neighbor_pixels[n]);
                  }
               }
//               cout << "samevalue_neighbor_pixels.size() = " 
//                    << samevalue_neighbor_pixels.size()
//                    << endl;

               if (samevalue_neighbor_pixels.size()==0)
               {
                  union_find_ptr->MakeSet(cc_label);
                  cc_labels_twoDarray_ptr->put(px,py,cc_label);
                  cc_label++;
               }
               else
               {
                  vector<double> neighbor_labels;
                  for (unsigned int n=0; n<samevalue_neighbor_pixels.size(); 
                       n++)
                  {
                     unsigned int qx=samevalue_neighbor_pixels[n].first;
                     unsigned int qy=samevalue_neighbor_pixels[n].second;
                     neighbor_labels.push_back(
                        cc_labels_twoDarray_ptr->get(qx,qy));
                  } // loop over index n labeling neighboring pixels
                  std::sort(neighbor_labels.begin(),neighbor_labels.end());

                  int min_label=neighbor_labels.front();
                  cc_labels_twoDarray_ptr->put(px,py,min_label);

                  for (unsigned int l=0; l<neighbor_labels.size(); l++)
                  {
                     union_find_ptr->Link(neighbor_labels[l],min_label);
//                     union_find_ptr->Link(neighbor_labels[l],min_node_ID);
                  }
               } // samevalue_neighbor_pixels.size() conditional
//               cout << "cc_label = " << cc_label << endl;

            } // loop over px index
         } // loop over py index

// Second pass: Relabel each pixel with a connected components ID that
// ranges from 1 to n_connected_components:

         typedef map<int,int> LABELS_MAP;
// independent int = root label
// dependent int = cc counter 

         LABELS_MAP* labels_map_ptr=new LABELS_MAP;
         LABELS_MAP::iterator iter;

         int cc_counter=0;
         for (unsigned int py=0; py<ydim; py++)
         {
            for (unsigned int px=0; px<xdim; px++)
            {
               double curr_z=pbinary_twoDarray_ptr->get(px,py);
               if (curr_z <= z_null) continue;
               int curr_label=cc_labels_twoDarray_ptr->get(px,py);
               int root_label=union_find_ptr->Find(curr_label);
               iter=labels_map_ptr->find(root_label);

               if (iter==labels_map_ptr->end())
               {
                  cc_counter++;
                  cc_label=cc_counter;
                  (*labels_map_ptr)[root_label]=cc_label;
               }
               else
               {
                  cc_label=iter->second;
               }
//               cc_labels_twoDarray_ptr->put(px,py,root_label);
               cc_labels_twoDarray_ptr->put(px,py,cc_label);
            } // loop over px index
         } // loop over py index
         delete labels_map_ptr;

         int n_components=cc_counter;
         delete union_find_ptr;

/*
// Print final connected component labels:

         cout << "cc_labels_twoDarray = " << endl;
         for (unsigned int py=0; py<ydim; py++)
         {
            for (unsigned int px=0; px<xdim; px++)
            {
               cout << cc_labels_twoDarray_ptr->get(px,py) << "  " << flush;
            }
            cout << endl;
         }
*/

         return n_components;
      }

   
} // graphicsfunc namespac
