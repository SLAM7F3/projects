// Note added on 3/12/14: Logic for double_line_thickness and
// triple_line_thickness in draw_line() looks wrong!  Should probably
// replace

/*

            if (double_line_thickness && px < mdim-1)
            {
               ztwoDarray_ptr->put(n_eff+ndim,intensity_value);
            }
            else if (triple_line_thickness && px > 0 && px < mdim-1)
            {
               ztwoDarray_ptr->put(n_eff-ndim,intensity_value);
               ztwoDarray_ptr->put(n_eff+ndim,intensity_value);
            }
*/
 
// with 

/*
            if (double_line_thickness && px < mdim-1)
            {
               ztwoDarray_ptr->put(n_eff-1,intensity_value);
            }
            else if (triple_line_thickness && px > 0 && px < mdim-1)
            {
               ztwoDarray_ptr->put(n_eff-1,intensity_value);
               ztwoDarray_ptr->put(n_eff+1,intensity_value);
            }
*/



// ==========================================================================
// DRAWFUNCS stand-alone methods
// ==========================================================================
// Last modified on 6/5/12; 3/6/14; 3/12/14; 4/5/14
// ==========================================================================

#include <iostream>
#include <set>
#include <vector>

#include "math/constant_vectors.h"
#include "geometry/contour.h"
#include "datastructures/dataarray.h"
#include "image/drawfuncs.h"
#include "geometry/frustum.h"
#include "geometry/linesegment.h"
#include "math/mathfuncs.h"
#include "image/myimage.h"
#include "templates/mytemplates.h"
#include "geometry/parallelepiped.h"
#include "geometry/parallelogram.h"
#include "geometry/polygon.h"
#include "math/prob_distribution.h"
#include "math/rotation.h"
#include "geometry/triangles_group.h"
#include "geometry/triangulate_funcs.h"
#include "image/TwoDarray.h"
#include "geometry/vertex.h"

#include "numrec/nrfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

namespace drawfunc
{


// ==========================================================================
// Discrete geometrical object drawing methods (e.g. lines, points, polygons):
// ==========================================================================

// Method draw_line draws the projection of the input linesegment l in
// the x-y plane.  This method implements the "midpoint line
// algorithm" described in section 3.2.2 of "Computer graphics:
// principles and practice", 2nd edition by Foley, van Dam, Feiner and
// Hughes.  All z (3rd spatial dimension) information within l is
// ignored:

   void draw_line(
      const linesegment& l,colorfunc::Color color,twoDarray* ztwoDarray_ptr)
      {
         bool double_line_thickness=(ztwoDarray_ptr->get_mdim() > 400);
         double intensity_value=colorfunc::color_to_value(color);
         draw_line(l,intensity_value,ztwoDarray_ptr,
                   double_line_thickness,false);
      }

// We have intentionally tried to optimize the following method for
// execution speed:
   
   void draw_line(
      const linesegment& l,double intensity_value,twoDarray* ztwoDarray_ptr,
      bool double_line_thickness,bool triple_line_thickness)
      {
         const unsigned int mdim=ztwoDarray_ptr->get_mdim();
         const unsigned int ndim=ztwoDarray_ptr->get_ndim();
         
         unsigned int px_start,py_start,px_stop,py_stop;
         ztwoDarray_ptr->point_to_pixel(
            l.get_v1().get(0),l.get_v1().get(1),px_start,py_start);
         ztwoDarray_ptr->point_to_pixel(
            l.get_v2().get(0),l.get_v2().get(1),px_stop,py_stop);

         unsigned int px=px_start;
         unsigned int py=py_start;
         if (ztwoDarray_ptr->pixel_inside_working_region(px,py))
         {
            int n_eff=px*ndim+py;
            ztwoDarray_ptr->put(n_eff,intensity_value);
            if (double_line_thickness && px < mdim-1)
            {
               ztwoDarray_ptr->put(n_eff+ndim,intensity_value);
            }
            else if (triple_line_thickness && px > 0 && px < mdim-1)
            {
               ztwoDarray_ptr->put(n_eff-ndim,intensity_value);
               ztwoDarray_ptr->put(n_eff+ndim,intensity_value);
            }
         }

         int dx=px_stop-px_start;
         int dy=-(py_stop-py_start);

         if (abs(dx) > abs(dy))
         {
            int d=2*abs(dy)-abs(dx);
            int increE=2*abs(dy);
            int increNE=2*(abs(dy)-abs(dx));
            while (sgn(dx)*px < sgn(dx)*px_stop)
            {
               px += sgn(dx);
               if (d <= 0)
               {
                  d += increE;
               }
               else
               {
                  d += increNE;
                  py -= sgn(dy);
               }
               if (ztwoDarray_ptr->pixel_inside_working_region(px,py))
               {
                  int n_eff=px*ndim+py;
                  ztwoDarray_ptr->put(n_eff,intensity_value);
                  if (double_line_thickness && px < mdim-1)
                  {
                     ztwoDarray_ptr->put(n_eff+ndim,intensity_value);
                  }
                  else if (triple_line_thickness && px > 0 && px < mdim-1)
                  {
                     ztwoDarray_ptr->put(n_eff-ndim,intensity_value);
                     ztwoDarray_ptr->put(n_eff+ndim,intensity_value);
                  }
               }
            } // while loop over px
         }
         else
         {
            int d=abs(dy)-2*abs(dx);
            int increN=-2*abs(dx);
            int increNE=2*(abs(dy)-abs(dx));
            while (sgn(dy)*py > sgn(dy)*py_stop)
            {
               py -= sgn(dy);
               if (d >= 0)
               {
                  d += increN;
               }
               else
               {
                  d += increNE;
		  px += sgn(dx);
               }
               if (ztwoDarray_ptr->pixel_inside_working_region(px,py))
               {
                  int n_eff=px*ndim+py;
                  ztwoDarray_ptr->put(n_eff,intensity_value);
                  if (double_line_thickness && px < mdim-1)
                  {
                     ztwoDarray_ptr->put(n_eff+ndim,intensity_value);
                  }
                  else if (triple_line_thickness && px > 0 && px < mdim-1)
                  {
                     ztwoDarray_ptr->put(n_eff-ndim,intensity_value);
                     ztwoDarray_ptr->put(n_eff+ndim,intensity_value);
                  }
               }
            } // while loop over py
         } // abs(dx) > abs(dy) conditional
      }

// ---------------------------------------------------------------------
   void draw_thick_line(
      const linesegment& l,colorfunc::Color color,
      double point_radius,twoDarray* ztwoDarray_ptr)
      {
         unsigned int px,py;
         double ds=point_radius;
         unsigned int npoints=basic_math::round(l.get_length()/ds);

         for (unsigned int i=0; i<=npoints; i++)
         {
            threevector currv(l.get_v1()+i*ds*l.get_ehat());
            if (ztwoDarray_ptr->point_to_pixel(
               currv.get(0),currv.get(1),px,py))
            {
               draw_hugepoint(currv,point_radius,color,ztwoDarray_ptr);
            }
         }
      }

   void draw_thick_line(
      const linesegment& l,double intensity,
      double point_radius,twoDarray* ztwoDarray_ptr)
      {
         unsigned int px,py;
         double ds=point_radius;
         unsigned int npoints=basic_math::round(l.get_length()/ds);

         for (unsigned int i=0; i<=npoints; i++)
         {
            threevector currv(l.get_v1()+i*ds*l.get_ehat());
            if (ztwoDarray_ptr->point_to_pixel(
               currv.get(0),currv.get(1),px,py))
            {
               draw_hugepoint(currv,point_radius,intensity,ztwoDarray_ptr);
            }
         }
      }

// ---------------------------------------------------------------------
//   void draw_dashedline(
//      const linesegment& l,double intensity_value,twoDarray* ztwoDarray_ptr)
//      {
//         const int dashlength=5; // increase to make dashes longer
//         const int dash_separation=4;  // increase to get more space between dashes
   
//         bool high_resolution=false;
//         int i,npoints,px,py,counter=0;
//         double ds;
//         threevector currv;
   
//         ds=basic_math::min(fabs(ztwoDarray_ptr->get_deltax()),fabs(ztwoDarray_ptr->get_deltay()));
//         npoints=basic_math::round(l.get_length()/ds);

//         counter=0;
//         for (i=0; i<=npoints; i++)
//         {
//            currv=l.get_v1()+i*ds*l.get_ehat();
//            if (ztwoDarray_ptr->point_to_pixel(currv.get(0),currv.get(1),px,py))
//            {
//               if ((counter/dashlength)%4==0) ztwoDarray_ptr->put(
//                  px,py,intensity_value);
//               counter++;
//            }
//         }
//      }

// ---------------------------------------------------------------------
// Method draw_vector calls draw_line and then adds an arrow head at
// the tip of the line segment.  The vector's basepoint location is
// explicitly passed as an input argument.

   void draw_vector(
      threevector& v,const threevector& basepoint,colorfunc::Color color,
      twoDarray* ztwoDarray_ptr)
      {
         const double theta=5*PI/6;
         const double costheta=cos(theta);
         const double sintheta=sin(theta);
   
         double s=0.1*v.magnitude();
         rotation R;

         R.identity();
         R.put(0,0,costheta);
         R.put(0,1,-sintheta);
         R.put(1,0,sintheta);
         R.put(1,1,costheta);
   
         linesegment l(basepoint,basepoint+v);
         threevector v3(l.get_v2()+s*(R*l.get_ehat()));
         threevector v4(l.get_v2()+s*(R.transpose()*l.get_ehat()));
         linesegment l3(l.get_v2(),v3);
         linesegment l4(l.get_v2(),v4);

         draw_line(l,color,ztwoDarray_ptr);
         draw_line(l3,color,ztwoDarray_ptr);
         draw_line(l4,color,ztwoDarray_ptr);
      }

   void draw_vector(
      const linesegment& l,colorfunc::Color color,twoDarray* ztwoDarray_ptr)
      {
         threevector v(l.get_v2()-l.get_v1());
         threevector basepoint(l.get_v1());
         draw_vector(v,basepoint,color,ztwoDarray_ptr);
      }

// ---------------------------------------------------------------------
// Method draw_axes draws a set of coordinate axes rotated about the
// origin through input angle theta (measured in radians) onto output
// twoDarray *ztwoDarray_ptr:

   void draw_axes(colorfunc::Color color,twoDarray* ztwoDarray_ptr,
                  const threevector& origin,double theta)
      {
         threevector zeropoint(Zero_vector);
         threevector v1(10*ztwoDarray_ptr->get_xlo(),0);
         threevector v2(10*ztwoDarray_ptr->get_xhi(),0);
         linesegment l1(v1,v2);
         l1.rotate(zeropoint,0,0,theta);
         l1.translate(origin-l1.get_midpoint());
//   draw_vector(l1,intensity_value,ztwoDarray_ptr);
         draw_line(l1,color,ztwoDarray_ptr);

         threevector v3(0,10*ztwoDarray_ptr->get_ylo());
         threevector v4(0,10*ztwoDarray_ptr->get_yhi());
         linesegment l2(v3,v4);
         l2.rotate(zeropoint,0,0,theta);
         l2.translate(origin-l2.get_midpoint());
//   draw_vector(l2,intensity_value,ztwoDarray_ptr);
         draw_line(l2,color,ztwoDarray_ptr);
      }

// ---------------------------------------------------------------------
   void draw_hugepoint(
      const threevector& v,double radius,double intensity_value,
      twoDarray* ztwoDarray_ptr)
      {
         const int nsides=20;
         regular_polygon p(nsides,radius);
         p.translate(threevector(v.get(0),v.get(1),0));
         color_polygon_interior(p,intensity_value,ztwoDarray_ptr);
      }

   void draw_hugepoint(
      const threevector& v,double radius,colorfunc::Color color,
      twoDarray* ztwoDarray_ptr)
      {
         draw_hugepoint(v,radius,colorfunc::color_to_value(color),
                        ztwoDarray_ptr);
      }

// This overloaded version of method draw_hugepoint draws a solid dot
// in the background "underneath" other colored pixels:

   void draw_hugepoint(
      const threevector& v,double radius,colorfunc::Color foreground_color,
      colorfunc::Color background_color,twoDarray* ztwoDarray_ptr)
      {
         const int nsides=20;
         regular_polygon p(nsides,radius);
         p.translate(v.xy_projection());
         color_polygon_interior(
            p,colorfunc::color_to_value(foreground_color),
            colorfunc::color_to_value(background_color),ztwoDarray_ptr);
      }

   void draw_hugepoint(
      const threevector& v,double a,double b,
      colorfunc::Color foreground_color,
      colorfunc::Color background_color,twoDarray* ztwoDarray_ptr)
      {
//         cout << "inside drawfunc::draw_hugepoint()" << endl;
//         cout << "a = " << a << " b = " << b << endl;
//         cout << "foreground_color = " 
//              << colorfunc::get_colorstr(foreground_color) << endl;
//         cout << "foreground color value = "
//              << colorfunc::color_to_value(foreground_color) << endl;
//         cout << "background_color = " 
//              << colorfunc::get_colorstr(background_color) << endl;
//         cout << "background color value = "
//              << colorfunc::color_to_value(background_color) << endl;

         const int nsides=20;
         regular_polygon p(nsides,a,b);
         p.translate(v.xy_projection());
         color_polygon_interior(
            p,colorfunc::color_to_value(foreground_color),
            colorfunc::color_to_value(background_color),ztwoDarray_ptr);
      }

// ---------------------------------------------------------------------
// Method draw_polygon connects together a polygon's vertices with
// lines:

   void draw_polygon(const polygon& p,colorfunc::Color color,
                     twoDarray* ztwoDarray_ptr)
      {
         if (p.get_nvertices() >= 3)
         {
            for (unsigned int i=0; i<p.get_nvertices(); i++)
            {
               draw_line(linesegment(
                  p.get_vertex(i),p.get_vertex(
                     modulo(i+1,p.get_nvertices()))),color,ztwoDarray_ptr);
            }
         }
      }

   void draw_polygon(const polygon& p,double intensity,
                     twoDarray* ztwoDarray_ptr)
      {
         if (p.get_nvertices() >= 3)
         {
            for (unsigned int i=0; i<p.get_nvertices(); i++)
            {
               draw_line(linesegment(
                  p.get_vertex(i),p.get_vertex(
                     modulo(i+1,p.get_nvertices()))),
                         intensity,ztwoDarray_ptr);
            }
         }
      }

   void draw_thick_polygon(
      const polygon& p,colorfunc::Color color,double point_radius,
      twoDarray* ztwoDarray_ptr)
      {
         if (p.get_nvertices() >= 3)
         {
            for (unsigned int i=0; i<p.get_nvertices(); i++)
            {
               draw_thick_line(linesegment(
                  p.get_vertex(i),p.get_vertex(
                     modulo(i+1,p.get_nvertices()))),
                               color,point_radius,ztwoDarray_ptr);
            }
         }
      }

   void draw_thick_polygon(
      const polygon& p,double intensity,double point_radius,
      twoDarray* ztwoDarray_ptr)
      {
         if (p.get_nvertices() >= 3)
         {
            for (unsigned int i=0; i<p.get_nvertices(); i++)
            {
               draw_thick_line(linesegment(
                  p.get_vertex(i),p.get_vertex(modulo(
                     i+1,p.get_nvertices()))),
                               intensity,point_radius,ztwoDarray_ptr);
            }
         }
      }

// ---------------------------------------------------------------------
// Method draw_contour displays takes in a deformable contour c.  It
// draws the line segments for this contour's edges if boolean input
// flag display_edges==true.  It also draws the contour's vertices as
// points with radius value set by input parameter vertex_radius.

   void draw_contour(
      const contour& c,double edge_intensity_value,
      double vertex_intensity_value,double vertex_radius,
      twoDarray* ztwoDarray_ptr,bool display_edges,bool display_vertices)
      {
         const Linkedlist<pair<threevector,bool> >* vertex_list_ptr=
            c.get_vertex_list_ptr();
         if (vertex_list_ptr != NULL)
         {

// Draw origin:
//            draw_hugepoint(
//               c.get_origin(),5*vertex_radius,5*vertex_intensity_value,
//		 ztwoDarray_ptr);

            if (display_edges)
            {
               for (unsigned int i=0; i<c.get_nvertices(); i++)
               {
                  draw_line(c.get_edge(i),edge_intensity_value,ztwoDarray_ptr,
                           false,false);
               } // loop over index i
            }

            if (display_vertices)
            {
               for (const Mynode<pair<threevector,bool> >* currnode_ptr=
                       vertex_list_ptr->get_start_ptr(); currnode_ptr != NULL;
                    currnode_ptr=currnode_ptr->get_nextptr())
               {
                  double vertex_intensity(vertex_intensity_value);
                  if (!currnode_ptr->get_data().second)
                  {
                     vertex_intensity=10;
                  }
                  draw_hugepoint(
                     currnode_ptr->get_data().first,vertex_radius,
                     vertex_intensity,ztwoDarray_ptr);
               }
            }
         } // vertex_list_ptr != NULL conditional
      }

   void draw_thick_contour(
      const contour& c,double edge_intensity_value,double point_radius,
      twoDarray* ztwoDarray_ptr)
      {
         for (unsigned int i=0; i<c.get_nvertices(); i++)
         {
            draw_thick_line(c.get_edge(i),edge_intensity_value,
                            point_radius,ztwoDarray_ptr);
         }
      }

// ---------------------------------------------------------------------
// Method draw_bbox draws the rectangular bounding box defined by
// input parameters min/max x/y:

   void draw_bbox(
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      colorfunc::Color color,twoDarray* ztwoDarray_ptr)
      {
         cout << "inside drawfunc::draw_bbox()" << endl;
         polygon bbox(minimum_x,minimum_y,maximum_x,maximum_y);
         draw_polygon(bbox,color,ztwoDarray_ptr);
      }

// ---------------------------------------------------------------------
// Method draw_parallelepiped draws the edges of a parallelepiped.  In
// order to make the parallelepiped look more 3D, edges which are
// hidden in the z-direction are drawn in a different color than
// visible edges.

   void draw_parallelepiped(
      const parallelepiped& p,colorfunc::Color color,
      colorfunc::Color hidden_color,twoDarray* ztwoDarray_ptr)
      {
         const unsigned int nfaces=6;

         polygon face[nfaces];
         for (unsigned int f=0; f<nfaces-2; f++)
         {
            face[f]=p.get_sideface(f);
         }
         face[nfaces-2]=p.get_topface();
         face[nfaces-1]=p.get_bottomface();

         for (unsigned int f=0; f<nfaces; f++)
         {
            unsigned int nvertices=face[f].get_nvertices();
            for (unsigned int v=0; v<nvertices; v++)
            {
               linesegment curredge(
                  face[f].get_vertex(v),face[f].get_vertex(
                     modulo(v+1,nvertices)));
               threevector midpnt(curredge.get_v1()+
                               0.5*curredge.get_length()*curredge.get_ehat());
               bool hidden_edge=false;
               for (unsigned int fp=0; fp<nfaces; fp++)
               {
                  if (fp != f)
                  {
                     if (face[fp].point_behind_polygon(midpnt))
                     {
                        hidden_edge=true;
                     }
                  }
               } // loop over index fp labeling other faces 
               if (hidden_edge)
               {
                  draw_line(curredge,hidden_color,ztwoDarray_ptr);
               }
               else
               {
                  draw_line(curredge,color,ztwoDarray_ptr);
               }
            } // loop over index v labeling vertex v1 on current edge
         } // loop over index f labeling current face
      }

/*
   void draw_parallelepiped(
      const parallelepiped& p,colorfunc::Color color,
      twoDarray* ztwoDarray_ptr)
      {
         draw_parallelepiped(p,color,color,ztwoDarray_ptr);
      }
*/

   void draw_parallelepiped(
      const parallelepiped& p,colorfunc::Color color,
      twoDarray* ztwoDarray_ptr,double xscale_factor)
      {
         parallelepiped p_scaled(p);
         threevector scalefactor(xscale_factor,1,1);
         p_scaled.scale(p.get_center().get_pnt(),scalefactor);
         draw_parallelepiped(p_scaled,color,color,ztwoDarray_ptr);
      }

// ---------------------------------------------------------------------
// Method draw_frustum draws the outline of a frustum object.  It
// computes the dot product between each of the frustum's polygon
// sides and the unit vector z_hat=(0,0,1).  If the dot product < 0,
// the polygon is not drawn.  On the other hand, if the dot product >=
// 0, the polygon is drawn at a grey level proportional to the dot
// product value.  The resulting output looks quasi 3-dimensional.

   void draw_frustum(frustum& f,colorfunc::Color color,
                     twoDarray* ztwoDarray_ptr)
      {
         const double SMALLPOS=0.01;

         double dotproduct;
   
         dotproduct=z_hat.dot(f.get_top_face().get_normal());
//   if (dotproduct >= SMALLPOS)
         {
            draw_polygon(f.get_top_face(),color,ztwoDarray_ptr);
         }

         dotproduct=z_hat.dot(f.get_bottom_face().get_normal());
//   if (dotproduct >= SMALLPOS)
         {
            draw_polygon(f.get_bottom_face(),color,ztwoDarray_ptr);
         }

         for (unsigned int i=0; i<f.get_n_sidefaces(); i++)
         {
            dotproduct=z_hat.dot(f.get_sideface(i).get_normal());
            if (dotproduct >= SMALLPOS)
            {
               draw_polygon(f.get_sideface(i),color,ztwoDarray_ptr);
            }
         }
      }

// ==========================================================================
// Continuous region drawing methods (e.g. shading polygons, coloring
// polygon interiors):
// ==========================================================================

// Method color_triangle_interior implements a fast pixel
// rasterization algorithm which fills in triangles of arbitrary size.
// Geometrical triangles which extend beyond the horizontal and
// vertical borders defined within the input *ztwoDarray_ptr are
// clipped by this method.

// Our algorithm is similar in spirit to the more general polygon
// rasterization algorithm described in chapter 3 of "Computer
// graphics: principles and practice", 2nd edition by Foley, van Dam,
// Feiner and Hughes.

   void color_triangle_interior(
      const polygon& t,double intensity_value,twoDarray* ztwoDarray_ptr,
      bool accumulate_flag)
      {
         const unsigned int nsides=3;

         unsigned int px_lo,px_hi,py_lo,py_hi;
         unsigned int px[nsides],py[nsides];
         double x,y_01,y_12,y_20,ylo,yhi;
         double vx[nsides],vy[nsides];

// Sort vertices by their x components:

         for (unsigned int n=0; n<nsides; n++)
         {
            vx[n]=t.get_vertex(n).get(0);
            vy[n]=t.get_vertex(n).get(1);
         }
         Quicksort(vx,vy,nsides);

// Convert vertex coordinates from 2D space to pixel space:

         for (unsigned int n=0; n<nsides; n++)
         {
            ztwoDarray_ptr->x_to_px(vx[n],px[n]);
            ztwoDarray_ptr->y_to_py(vy[n],py[n]);
         }

// Calculate slopes for triangle's sides:

         double m[nsides];
         m[0]=(vy[1]-vy[0])/(vx[1]-vx[0]);
         m[1]=(vy[2]-vy[1])/(vx[2]-vx[1]);
         m[2]=(vy[0]-vy[2])/(vx[0]-vx[2]);

// Horizontally scan across first section of triangle.  For each px
// pixel value, compute corresponding py_lo and py_hi locations on the
// triangle's two sides.  Then fill in vertical line connecting
// (px,py_hi) to (px,py_lo).

         px_lo=basic_math::max(px[0],Unsigned_Zero);
         px_hi=basic_math::min(px[1],ztwoDarray_ptr->get_mdim());

         for (unsigned int currpx=px_lo; currpx < px_hi; currpx++)
         {
            ztwoDarray_ptr->px_to_x(currpx,x);
            y_01=mathfunc::linefit(x,vx[0],vy[0],m[0]);
            y_20=mathfunc::linefit(x,vx[2],vy[2],m[2]);
            ylo=basic_math::min(y_01,y_20);
            yhi=basic_math::max(y_01,y_20);
            ztwoDarray_ptr->y_to_py(ylo,py_hi);
            ztwoDarray_ptr->y_to_py(yhi,py_lo);
            py_lo=basic_math::max(Unsigned_Zero,py_lo);
            py_hi=basic_math::min(py_hi,ztwoDarray_ptr->get_ndim());

            if (accumulate_flag)
            {
               for (unsigned int py=py_lo; py<py_hi; py++)
               {
                  ztwoDarray_ptr->put(
                     currpx,py,ztwoDarray_ptr->get(currpx,py)+intensity_value);
               }
            }
            else
            {
               for (unsigned int py=py_lo; py<py_hi; py++)
               {
                  ztwoDarray_ptr->put(currpx,py,intensity_value);
               }
            }
         } // currpx loop

// Horizontally scan across 2nd section of triangle.  For each px
// pixel value, compute corresponding py_lo and py_hi locations on the
// triangle's two sides.  Then fill in vertical line connecting
// (px,py_hi) to (px,py_lo).

         px_lo=basic_math::max(px[1],Unsigned_Zero);
         px_hi=basic_math::min(px[2],ztwoDarray_ptr->get_mdim());
         for (unsigned int currpx=px_lo; currpx < px_hi; currpx++)
         {
            ztwoDarray_ptr->px_to_x(currpx,x);
            y_12=mathfunc::linefit(x,vx[1],vy[1],m[1]);
            y_20=mathfunc::linefit(x,vx[2],vy[2],m[2]);
            ylo=basic_math::min(y_12,y_20);
            yhi=basic_math::max(y_12,y_20);
            ztwoDarray_ptr->y_to_py(ylo,py_hi);
            ztwoDarray_ptr->y_to_py(yhi,py_lo);
            py_lo=basic_math::max(Unsigned_Zero,py_lo);
            py_hi=basic_math::min(py_hi,ztwoDarray_ptr->get_ndim());

            if (accumulate_flag)
            {
               for (unsigned int py=py_lo; py<py_hi; py++)
               {
                  ztwoDarray_ptr->put(
                     currpx,py,ztwoDarray_ptr->get(currpx,py)+intensity_value);
               }
            }
            else
            {
               for (unsigned int py=py_lo; py<py_hi; py++)
               {
                  ztwoDarray_ptr->put(currpx,py,intensity_value);
               }
            }

         } // currpx loop
      }

// ---------------------------------------------------------------------
// Method color_convex_quadrilateral_interior breaks apart a convex
// quadrilateral into two triangles.  (The input quadrilateral's
// vertices are assumed to be ordered in either a clockwise or
// counter-clockwise fashion.)  It then calls the fast pixel
// rasterization algorithm color_triangle_interior twice in order to
// fill the quadrilateral.

   void color_convex_quadrilateral_interior(
      const polygon& q,double intensity_value,twoDarray* ztwoDarray_ptr,
      bool accumulate_flag)
      {
         vector<threevector> tri1_vertex(3);
         tri1_vertex[0]=q.get_vertex(0);
         tri1_vertex[1]=q.get_vertex(1);
         tri1_vertex[2]=q.get_vertex(2);
         polygon triangle_1(tri1_vertex);

         vector<threevector> tri2_vertex(3);
         tri2_vertex[0]=q.get_vertex(2);
         tri2_vertex[1]=q.get_vertex(3);
         tri2_vertex[2]=q.get_vertex(0);
         polygon triangle_2(tri2_vertex);

         color_triangle_interior(
            triangle_1,intensity_value,ztwoDarray_ptr,accumulate_flag);
         color_triangle_interior(
            triangle_2,intensity_value,ztwoDarray_ptr,accumulate_flag);
      }

// ---------------------------------------------------------------------
// Method color_regular_hexagon_interior breaks apart a regular
// hexagon into two convex quadrilaterals.  It then calls the fast
// pixel rasteriation algorithm color_convex_quadrilateral_interior
// twice in order to fill the hexagon.

   void color_regular_hexagon_interior(
      const polygon& hexagon,double intensity_value,twoDarray* ztwoDarray_ptr,
      bool accumulate_flag)
      {
         vector<threevector> quad1_vertex(4);
         quad1_vertex[0]=hexagon.get_vertex(0);
         quad1_vertex[1]=hexagon.get_vertex(1);
         quad1_vertex[2]=hexagon.get_vertex(2);
         quad1_vertex[3]=hexagon.get_vertex(3);
         polygon quadrilateral_1(quad1_vertex);

         vector<threevector> quad2_vertex(4);
         quad2_vertex[0]=hexagon.get_vertex(3);
         quad2_vertex[1]=hexagon.get_vertex(4);
         quad2_vertex[2]=hexagon.get_vertex(5);
         quad2_vertex[3]=hexagon.get_vertex(0);
         polygon quadrilateral_2(quad2_vertex);

         color_convex_quadrilateral_interior(
            quadrilateral_1,intensity_value,ztwoDarray_ptr,accumulate_flag);
         color_convex_quadrilateral_interior(
            quadrilateral_2,intensity_value+10,ztwoDarray_ptr,accumulate_flag);
      }

// ---------------------------------------------------------------------
// Methods color_triangle_exterior and
// color_convex_quadrilateral_exterior generate a twoDarray containing
// a mask of the input polygon.  They then scane through the entire
// input twoDarray *ztwoDarray_ptr and set those pixels whose
// locations do not overlap with a nonzero mask intensity_value equal
// to the input specified intensity intensity_value.

   void color_triangle_exterior(
      const polygon& t,double intensity_value,twoDarray* ztwoDarray_ptr)
      {
         twoDarray* zmask_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
         zmask_twoDarray_ptr->clear_values();
         color_triangle_interior(t,60,zmask_twoDarray_ptr);

         for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
            {
               if (zmask_twoDarray_ptr->get(px,py)==0)
               {
                  ztwoDarray_ptr->put(px,py,intensity_value);
               }
            }
         }
         delete zmask_twoDarray_ptr;
      }

   void color_convex_quadrilateral_exterior(
      const polygon& q,double intensity_value,twoDarray* ztwoDarray_ptr)
      {
         twoDarray* zmask_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
         zmask_twoDarray_ptr->clear_values();
         color_convex_quadrilateral_interior(q,60,zmask_twoDarray_ptr);

         for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
            {
               if (zmask_twoDarray_ptr->get(px,py)==0)
               {
                  ztwoDarray_ptr->put(px,py,intensity_value);
               }
            }
         }
         delete zmask_twoDarray_ptr;
      }

// ---------------------------------------------------------------------
// Method fill_halfplane generates a 2D box which passes through input
// twovector basepoint.  The side of the box which goes through the
// basepoint is also orthogonal to input direction vector nhat.  This
// method then fills set all entries within input twoDarray
// *ztwoDarray_ptr which lie within the box equal to input intensity
// intensity_value.

   void fill_halfplane(const threevector& basepoint,const threevector& nhat,
                       double intensity_value,twoDarray* ztwoDarray_ptr)
      {
         const double box_length=40;	// meters

         threevector uhat(-nhat.get(1),nhat.get(0),0);
         
         vector<threevector> vertex(4);
         vertex[0]=basepoint+box_length*uhat;
         vertex[1]=basepoint+box_length*(uhat-nhat);
         vertex[2]=basepoint-box_length*(uhat+nhat);
         vertex[3]=basepoint-box_length*uhat;
         polygon quad(vertex);
         color_convex_quadrilateral_interior(
            quad,intensity_value,ztwoDarray_ptr);
      }

// ---------------------------------------------------------------------
// Method color_parallelogram_interior sets the color of pixels
// located within the interior of input parallelogram p equal to some
// specified intensity_value:

   void color_parallelogram_interior(
      const parallelogram& p,double intensity_value,twoDarray* ztwoDarray_ptr)
      {
         unsigned int min_px,min_py,max_px,max_py;
         ztwoDarray_ptr->locate_extremal_xy_pixels(
            p,min_px,min_py,max_px,max_py);
         threevector currpoint;
         for (unsigned int px=basic_math::max(Unsigned_Zero,min_px-1); 
              px<basic_math::min(ztwoDarray_ptr->get_mdim(),max_px+1); px++)
         {
            for (unsigned int py=basic_math::max(Unsigned_Zero,min_py-1); 
                 py<basic_math::min(ztwoDarray_ptr->get_ndim(),max_py+1); py++)
            {
               ztwoDarray_ptr->pixel_to_point(px,py,currpoint);
               if (p.point_inside(currpoint))
               {
                  ztwoDarray_ptr->put(px,py,intensity_value);
               }
            }
         }
      }

// ---------------------------------------------------------------------
// Method color_polygon_interior sets the color of pixels located
// within the interior of the xy projection of input polygon p equal
// to some specified intensity_value:

   void color_polygon_interior(
      const polygon& p,colorfunc::Color color,twoDarray* ztwoDarray_ptr)
      {
         double intensity_value=colorfunc::color_to_value(color);
         color_polygon_interior(p,intensity_value,ztwoDarray_ptr);
      }

   void color_polygon_interior(
      const polygon& p,double intensity_value,twoDarray* ztwoDarray_ptr)
      {
//         cout << "inside drawfunc::color_polygon_interior()" << endl;
         unsigned int px,py,min_px,min_py,max_px,max_py;
         polygon p_xyproj=p.xy_projection();

//         cout << "p_xyproj = " << p_xyproj << endl;

         if (p.get_nvertices() >= 3)
         {
            ztwoDarray_ptr->locate_extremal_xy_pixels(
               p_xyproj,min_px,min_py,max_px,max_py);

            threevector currpoint;
            for (px=basic_math::max(Unsigned_Zero,min_px-1); 
                 px<basic_math::min(ztwoDarray_ptr->get_mdim(),max_px+1); px++)
            {
               for (py=basic_math::max(Unsigned_Zero,min_py-1); 
                    py<basic_math::min(ztwoDarray_ptr->get_ndim(),max_py+1); 
                    py++)
               {
                  ztwoDarray_ptr->pixel_to_point(px,py,currpoint);
                  if (p_xyproj.point_inside_polygon(currpoint))
                  {
                     ztwoDarray_ptr->put(px,py,intensity_value);
                  }
               }
            }
         } // p.get_nvertices() >= 3 conditional
      }

// ---------------------------------------------------------------------
// This overloaded version of method color_polygon_interior sets the
// color of pixels located within the interior of the xy projection of
// input polygon p equal to some specified foreground intensity_value,
// provided that these pixels intensity intensity_values originally
// equaled some specified background intensity_value.  This last
// restriction essentially sends the polygon to the background and
// prevents it from overwriting other colored pixels.

   void color_polygon_interior(
      const polygon& p,double foreground_intensity,
      double background_intensity,twoDarray* ztwoDarray_ptr)
      {
         unsigned int px,py,min_px,min_py,max_px,max_py;
         polygon p_xyproj=p.xy_projection();

         if (p.get_nvertices() >= 3)
         {
            ztwoDarray_ptr->locate_extremal_xy_pixels(
               p_xyproj,min_px,min_py,max_px,max_py);

            threevector currpoint;
            for (px=basic_math::max(Unsigned_Zero,min_px-1); 
                 px<basic_math::min(ztwoDarray_ptr->get_mdim(),max_px+1); px++)
            {
               for (py=basic_math::max(Unsigned_Zero,min_py-1); 
                    py<basic_math::min(ztwoDarray_ptr->get_ndim(),
                    max_py+1); py++)
               {
                  ztwoDarray_ptr->pixel_to_point(px,py,currpoint);
                  if (p_xyproj.point_inside_polygon(currpoint) &&
                      ztwoDarray_ptr->get(px,py)==background_intensity)
                  {
                     ztwoDarray_ptr->put(px,py,foreground_intensity);
                  }
               } // py loop
            } // px loop
         } // p.get_nvertices() >= 3 conditional
      }

// ---------------------------------------------------------------------
// Method color_contour_interior triangulates input contour c.  It
// then colors each triangle's interior according to input
// intensity_value.

/*
   void color_contour_interior(
      const contour& c,double intensity_value,twoDarray* ftwoDarray_ptr)
      {
         cout << "inside drawfunc::color_contour_interior()" << endl;
         
         triangles_group* triangles_group_ptr=new triangles_group();

         for (unsigned int v=0; v<c.get_nvertices(); v++)
         {
            threevector curr_vertex(c.get_vertex(v).first);
            int curr_vertex_ID=triangles_group_ptr->get_n_vertices();
            vertex curr_Vertex(c.get_vertex(v).first,curr_vertex_ID);
            triangles_group_ptr->update_triangle_vertices(curr_Vertex);
         }

         triangles_group_ptr->delaunay_triangulate_vertices();
         cout << "Number of Delaunay triangles = "
              << triangles_group_ptr->get_n_triangles() << endl;
         
// Loop over all triangles and draw them within* ftwoDarray_ptr:

         for (unsigned int t=0; t<triangles_group_ptr->get_n_triangles(); t++)
         {
            triangle* triangle_ptr=triangles_group_ptr->get_triangle_ptr(t);
            polygon* poly_ptr=triangle_ptr->generate_polygon();

            cout << "t = " << t << " *poly_ptr = " << *poly_ptr << endl;

            double random_intensity_value=nrfunc::ran1();

            drawfunc::color_triangle_interior(
               *poly_ptr,random_intensity_value,ftwoDarray_ptr);
            delete poly_ptr;
         }
         
         delete triangles_group_ptr;
      }

*/

// As of 1/26/12, this next method should be reworked to use
// contour::generate_interior_triangles
   
   void color_contour_interior(
      const contour& c,double intensity_value,twoDarray* ftwoDarray_ptr)
      {
/*
         Linkedlist<polygon*>* triangle_list_ptr=c.generate_triangle_list();

         for (Mynode<polygon*>* currnode_ptr=triangle_list_ptr->
                 get_start_ptr();
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {

            double random_intensity_value=0.5+0.5*nrfunc::ran1();

            drawfunc::color_triangle_interior(
               *(currnode_ptr->get_data()),random_intensity_value,
               ftwoDarray_ptr);
//            drawfunc::draw_polygon(*(currnode_ptr->get_data()),
//                                   colorfunc::white,ftwoDarray_ptr);
         }

         triangulate_func::delete_triangle_list(triangle_list_ptr);
*/

      }

// ---------------------------------------------------------------------
// Method color_contour_interior_shell takes in a contour c.  After
// computing a scaled version of c whose radius is twice smaller, this
// method forms all the quadrilaterals between corresponding vertices
// on the outer and inner contours.  It then calls the quadrilateral
// coloring method to fill in the "interior shell" defined between the
// inner and outer contours.

   void color_contour_interior_shell(
      double scale_factor,const contour& c,double intensity_value,
      twoDarray* ztwoDarray_ptr)
      {
         unsigned int nvertices=c.get_nvertices();
         vector<threevector> vertex(4);
         contour c_scale(c);
         c_scale.scale(scale_factor);
         for (unsigned int i=0; i<nvertices; i++)
         {
            vertex[0]=c.get_vertex(i).first;
            vertex[1]=c.get_vertex(modulo(i+1,nvertices)).first;
            vertex[2]=c_scale.get_vertex(modulo(i+1,nvertices)).first;
            vertex[3]=c_scale.get_vertex(i).first;

            if (is_even(i))
            {
               for (unsigned int j=0; j<4; j++)
               {
                  draw_hugepoint(vertex[j],0.5,0.1*double(modulo(i,7)),
                                 ztwoDarray_ptr);
               }
            }
            
//            color_convex_quadrilateral_interior(
//               polygon(vertex),intensity_value,ztwoDarray_ptr);
         } // loop over index i labeling contour vertices
      }

// ---------------------------------------------------------------------
// Method shade_polygon adjusts polygon p's color based upon the
// dotproduct between the polygon's normal and the viewing direction
// defined by input unit vector uhat.

   void shade_polygon(
      polygon& p,double max_intensity_value,double min_intensity_value,
      const threevector& uhat,twoDarray* ztwoDarray_ptr)
      {
         const double SMALLNEG=-0.001;
         double dotproduct,intensity_value;
   
         if (p.get_nvertices() >= 3)
         {

//  On 6/26/01, we experimented with returning shaded polygons on a dB
//  rather than linear scale.  Chi-square agreement between
//  thresholded data and renormalized model images is worse for dB
//  shading than for linear shading.  So as of 6/26/01, we stick with
//  linear shading.  We reverified on 6/27/01 that linear shading
//  yields composited models which look to the eye much closer to
//  composited images than do their dB shaded counterparts.

            dotproduct=uhat.dot(p.get_normal());

// Recall that in order for a surface to be visible, its normal vector
// must basically be ANTI-aligned rather than aligned with the
// illumination direction vector!  Hence the dotproduct between the
// two should be negative rather than positive.

            if (dotproduct < SMALLNEG)
            {
//      intensity_value=max_intensity_value+dB(fabs(dotproduct));
               intensity_value=min_intensity_value+
                  (max_intensity_value-min_intensity_value)*fabs(dotproduct);
               color_polygon_interior(p,intensity_value,ztwoDarray_ptr);
            }
         }
      }

// ---------------------------------------------------------------------
// Method color_bbox_interior sets the color of pixels located within
// the interior of input rectangular bounding box bbox equal to some
// specified intensity_value:

   void color_bbox_interior(
      const polygon& bbox,double intensity_value,twoDarray* ztwoDarray_ptr)
      {
         double min_x=bbox.get_vertex(0).get(0);
         double min_y=bbox.get_vertex(0).get(1);
         double max_x=bbox.get_vertex(2).get(0);
         double max_y=bbox.get_vertex(2).get(1);
         color_bbox_interior(
            min_x,min_y,max_x,max_y,intensity_value,ztwoDarray_ptr);
      }

   void color_bbox_interior(
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      double intensity_value,twoDarray* ztwoDarray_ptr)
      {
         unsigned int min_px,min_py,max_px,max_py;
         if (ztwoDarray_ptr->bbox_corners_to_pixels(
            minimum_x,minimum_y,maximum_x,maximum_y,
            min_px,min_py,max_px,max_py))
         {
            color_bbox_interior(
               min_px,min_py,max_px,max_py,intensity_value,ztwoDarray_ptr);
         }
      }

   void color_bbox_interior(
      unsigned int min_px,unsigned int min_py,
      unsigned int max_px,unsigned int max_py,
      double intensity_value,twoDarray* ztwoDarray_ptr)
      {
         for (unsigned int px=basic_math::max(Unsigned_Zero,min_px); 
              px<basic_math::min(ztwoDarray_ptr->get_mdim(),max_px); px++)
         {
            for (unsigned int py=basic_math::max(Unsigned_Zero,min_py); 
                 py<basic_math::min(ztwoDarray_ptr->get_ndim(),
                                    max_py); py++)
            {
               ztwoDarray_ptr->put(px,py,intensity_value);
            }
         }
      }

// ---------------------------------------------------------------------
// Method color_bbox_exterior sets the color of pixels located outside
// the input rectangular bounding box bbox equal to some specified
// intensity_value:

   void color_bbox_exterior(
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      double intensity_value,twoDarray* ztwoDarray_ptr)
      {
         unsigned int px,py,min_px,min_py,max_px,max_py;
         if (ztwoDarray_ptr->bbox_corners_to_pixels(
            minimum_x,minimum_y,maximum_x,maximum_y,
            min_px,min_py,max_px,max_py))
         {
            for (px=0; px<min_px; px++)
            {
               for (py=0; py<ztwoDarray_ptr->get_ndim(); py++)
               {
                  ztwoDarray_ptr->put(px,py,intensity_value);
               }
            }
            for (px=max_px; px<ztwoDarray_ptr->get_mdim(); px++)
            {
               for (py=0; py<ztwoDarray_ptr->get_ndim(); py++)
               {
                  ztwoDarray_ptr->put(px,py,intensity_value);
               }
            }
            for (py=0; py<min_py; py++)
            {
               for (px=0; px<ztwoDarray_ptr->get_mdim(); px++)
               {
                  ztwoDarray_ptr->put(px,py,intensity_value);
               }
            }
            for (py=max_py; py<ztwoDarray_ptr->get_ndim(); py++)
            {
               for (px=0; px<ztwoDarray_ptr->get_mdim(); px++)
               {
                  ztwoDarray_ptr->put(px,py,intensity_value);
               }
            }
         } // bbox_corners_to_pixels conditional
      }

   void color_bbox_exterior(
      const polygon& bbox,double intensity_value,twoDarray* ztwoDarray_ptr)
      {
         double min_x=bbox.get_vertex(0).get(0);
         double min_y=bbox.get_vertex(0).get(1);
         double max_x=bbox.get_vertex(2).get(0);
         double max_y=bbox.get_vertex(2).get(1);
         color_bbox_exterior(
            min_x,min_y,max_x,max_y,intensity_value,ztwoDarray_ptr);
      }

// ---------------------------------------------------------------------
// Method color_polygon_exterior sets the color of pixels located
// outside the xy projection of input polygon p equal to some
// specified intensity_value.  This method is useful for nulling all
// pixels located outside some bounding polygon.

   void color_polygon_exterior(
      const polygon& p,double intensity_value,twoDarray* ztwoDarray_ptr)
      {
         unsigned int px,py,min_px,min_py,max_px,max_py;
         polygon p_xyproj=p.xy_projection();

         if (p.get_nvertices() >= 3)
         {

// First, quickly color all pixels located outside bounding box which
// encloses polygon p:

            ztwoDarray_ptr->locate_extremal_xy_pixels(
               p_xyproj,min_px,min_py,max_px,max_py);

//      cout << "min_px = " << min_px << " max_px = " << max_px << endl;
//      cout << "min_py = " << min_py << " max_py = " << max_py << endl;
//      cout << "intensity_value = " << intensity_value << endl;

            for (px=0; px<ztwoDarray_ptr->get_mdim(); px++)
            {
               for (py=0; py<min_py; py++)
               {
                  ztwoDarray_ptr->put(px,py,intensity_value);
               }
            }
            for (px=0; px<ztwoDarray_ptr->get_mdim(); px++)
            {
               for (py=max_py; py<ztwoDarray_ptr->get_ndim(); py++)
               {
                  ztwoDarray_ptr->put(px,py,intensity_value);
               }
            }
            for (px=0; px<min_px; px++)
            {
               for (py=0; py<ztwoDarray_ptr->get_ndim(); py++)
               {
                  ztwoDarray_ptr->put(px,py,intensity_value);
               }
            }
            for (px=max_px; px<ztwoDarray_ptr->get_mdim(); px++)
            {
               for (py=0; py<ztwoDarray_ptr->get_ndim(); py++)
               {
                  ztwoDarray_ptr->put(px,py,intensity_value);
               }
            }

// Next, examine points inside bounding box and color those lying
// outside p:
            
            threevector currpoint;
            for (px=min_px; px<max_px; px++)
            {
               for (py=min_py; py<max_py; py++)
               {
                  ztwoDarray_ptr->pixel_to_point(px,py,currpoint);
                  if (!p_xyproj.point_inside_polygon(currpoint))
                  {
                     ztwoDarray_ptr->put(px,py,intensity_value);
                  }
               }
            }
         } // p.get_nvertices() >= 3 conditional
      }

// ---------------------------------------------------------------------
// Method null_pixels_outside_convex_polygon

   void null_pixels_outside_convex_polygon(
      double null_value,polygon* poly_ptr,twoDarray* ztwoDarray_ptr)
      {
         if (poly_ptr != NULL)
         {
            threevector currpoint;
            for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
            {
               for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
               {
                  ztwoDarray_ptr->pixel_to_point(px,py,currpoint);
                  if (!poly_ptr->point_inside_polygon(currpoint))
                  {
                     ztwoDarray_ptr->put(px,py,null_value);
                  }
               } // loop over index py
            } // loop over index px
         } // poly_ptr != NULL conditional
      }

   void null_pixels_outside_convex_polygon(
      double null_value,polygon* poly_ptr,twoDarray* ztwoDarray1_ptr,
      twoDarray* ztwoDarray2_ptr)
      {
         if (poly_ptr != NULL)
         {
            threevector currpoint;
            for (unsigned int px=0; px<ztwoDarray1_ptr->get_mdim(); px++)
            {
               for (unsigned int py=0; py<ztwoDarray1_ptr->get_ndim(); py++)
               {
                  ztwoDarray1_ptr->pixel_to_point(px,py,currpoint);
                  if (!poly_ptr->point_inside_polygon(currpoint))
                  {
                     ztwoDarray1_ptr->put(px,py,null_value);
                     ztwoDarray2_ptr->put(px,py,null_value);
                  }
               } // loop over index py
            } // loop over index px
         } // poly_ptr != NULL conditional
      }

   void null_pixels_outside_parallelogram(
      double null_value,parallelogram* parallelogram_ptr,
      twoDarray* ztwoDarray_ptr)
      {
         if (parallelogram_ptr != NULL)
         {
            threevector currpoint;
            for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
            {
               for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
               {
                  ztwoDarray_ptr->pixel_to_point(px,py,currpoint);
                  if (!parallelogram_ptr->point_inside(currpoint))
                  {
                     ztwoDarray_ptr->put(px,py,null_value);
                  }
               } // loop over index py
            } // loop over index px
         } // parallelogram_ptr != NULL conditional
      }

   void null_pixels_outside_parallelogram(
      double null_value,parallelogram* parallelogram_ptr,
      twoDarray* ztwoDarray1_ptr,twoDarray* ztwoDarray2_ptr)
      {
         if (parallelogram_ptr != NULL)
         {
            threevector currpoint;
            for (unsigned int px=0; px<ztwoDarray1_ptr->get_mdim(); px++)
            {
               for (unsigned int py=0; py<ztwoDarray1_ptr->get_ndim(); py++)
               {
                  ztwoDarray1_ptr->pixel_to_point(px,py,currpoint);
                  if (!parallelogram_ptr->point_inside(currpoint))
                  {
                     ztwoDarray1_ptr->put(px,py,null_value);
                     ztwoDarray2_ptr->put(px,py,null_value);
                  }
               } // loop over index py
            } // loop over index px
         } // parallelogram_ptr != NULL conditional
      }

// ---------------------------------------------------------------------
// Method count_and_color_lit_pixels_inside_bbox 

   int count_and_color_lit_pixels_inside_bbox(
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      double zmin,double lit_intensity,double unlit_intensity,
      twoDarray* ztwoDarray_ptr,int& npixels_inside_bbox)
      {
         npixels_inside_bbox=0;
         int npixels_above_zmin=0;
         
         unsigned int min_px,max_px,min_py,max_py;
         if (ztwoDarray_ptr->bbox_corners_to_pixels(
            minimum_x,minimum_y,maximum_x,maximum_y,
            min_px,min_py,max_px,max_py))
         {
            for (unsigned int px=min_px+1; px<=max_px; px++)
            {
               for (unsigned int py=min_py+1; py<=max_py; py++)
               {
                  npixels_inside_bbox++;
                  if (ztwoDarray_ptr->get(px,py) > zmin) 
                  {
                     npixels_above_zmin++;
                     ztwoDarray_ptr->put(px,py,lit_intensity);
                  }
                  else
                  {
                     ztwoDarray_ptr->put(px,py,unlit_intensity);
                  }
               }
            }
         } // bbox_corners_to_pixels conditional
         return npixels_above_zmin;
      }

// ---------------------------------------------------------------------
// Method restore_pixels_inside_bbox sets elements inside input
// twoDarray *ztwoDarray_ptr within a bounding box defined by pixel
// limits pxlo < px < pxhi and pylo < py < pyhi to their counterpart
// intensity_values in twoDarray *raw_ztwoDarray_ptr.

   void restore_pixels_inside_bbox(
      twoDarray* ztwoDarray_ptr,twoDarray const *raw_ztwoDarray_ptr,
      unsigned int pxlo,unsigned int pxhi,unsigned int pylo,unsigned int pyhi)
      {
         for (unsigned int i=pxlo; i<pxhi; i++)
         {
            for (unsigned int j=pylo; j<pyhi; j++)
            {
               ztwoDarray_ptr->put(i,j,raw_ztwoDarray_ptr->get(i,j));
            }
         }
      }

// ---------------------------------------------------------------------
// Method shade_parallelepiped adjusts the color intensity_value of
// each side face of input box b based upon the dotproduct between the
// side face's normal and the viewing direction defined by input unit
// vector uhat:

   void shade_parallelepiped(
      parallelepiped& p,double max_intensity_value,
      double min_intensity_value,const threevector& uhat,
      twoDarray* ztwoDarray_ptr)
      {
         shade_polygon(
            p.get_bottomface(),max_intensity_value,min_intensity_value,
            uhat,ztwoDarray_ptr);
         for (unsigned int i=p.get_n_sidefaces()-1; i>=0; i--)
//   for (unsigned int i=0; i<p.get_n_sidefaces(); i++)
         {
            shade_polygon(
               p.get_sideface(i),max_intensity_value,min_intensity_value,
               uhat,ztwoDarray_ptr);
         }
         shade_polygon(
            p.get_topface(),max_intensity_value,min_intensity_value,
            uhat,ztwoDarray_ptr);
      }

// ---------------------------------------------------------------------
// Method shade_frustum adjusts the color value of each polygonal
// facet of input frustum f based upon the dotproduct between the
// polygon's normal and the viewing direction defined by input unit
// vector uhat:

   void shade_frustum(
      frustum& f,double max_intensity_value,double min_intensity_value,
      const threevector& uhat,twoDarray* ztwoDarray_ptr)
      {
         shade_frustum_sides(f,max_intensity_value,min_intensity_value,
                             uhat,ztwoDarray_ptr);
         shade_polygon(
            f.get_top_face(),max_intensity_value,min_intensity_value,
            uhat,ztwoDarray_ptr);
         shade_polygon(
            f.get_bottom_face(),max_intensity_value,min_intensity_value,
            uhat,ztwoDarray_ptr);
      }

// ---------------------------------------------------------------------
// Method shade_frustum adjusts the color intensity_value of each
// polygonal side face of input frustum f based upon the dotproduct
// between the polygon's normal and the viewing direction defined by
// input unit vector uhat:

   void shade_frustum_sides(
      frustum& f,double max_intensity_value,
      double min_intensity_value,const threevector& uhat,
      twoDarray* ztwoDarray_ptr)
      {
         for (unsigned int i=0; i<f.get_n_sidefaces(); i++)
         {
            shade_polygon(
               f.get_sideface(i),max_intensity_value,min_intensity_value,
               uhat,ztwoDarray_ptr);
         }
      }

// ---------------------------------------------------------------------
// Method extremal_frustum_points returns the maximum and minimum x &
// y points located on the top and bottom faces of a frustum.  These
// intensity_values provide the corners of a useful bounding box which
// contains the image plane projection of a frustum:

   void extremal_frustum_points(
      frustum& f,unsigned int& min_px,unsigned int& min_py,
      unsigned int& max_px,unsigned int& max_py,
      twoDarray const *ztwoDarray_ptr)
      {
         unsigned int min_px_top,min_py_top,max_px_top,max_py_top;
         unsigned int min_px_bottom,min_py_bottom,max_px_bottom,max_py_bottom;

         ztwoDarray_ptr->locate_extremal_xy_pixels(
            f.get_top_face(),min_px_top,min_py_top,max_px_top,max_py_top);
         ztwoDarray_ptr->locate_extremal_xy_pixels(
            f.get_bottom_face(),min_px_bottom,min_py_bottom,max_px_bottom,
            max_py_bottom);

         min_px=basic_math::min(min_px_top,min_px_bottom);
         min_py=basic_math::min(min_py_top,min_py_bottom);
         max_px=basic_math::min(max_px_top,max_px_bottom);
         max_py=basic_math::min(max_py_top,max_py_bottom);
   
         ztwoDarray_ptr->keep_pnt_inside_working_region(
            min_px,min_py,max_px,max_py);
      }

// ---------------------------------------------------------------------
// Method color_frustum_interior sets the color of pixels located
// within the interior of a projected frustum equal to some specified
// intensity_value.  On 1/11/01, we attempted to minimize the number
// of calls which this method makes to the
// point_inside_xyprojected_frustum method of the frustum class in
// order to optimize this routine's speed.

   void color_frustum_interior(
      frustum& f,double intensity_value,twoDarray* ztwoDarray_ptr)
      {
         bool curr_point_inside_frustum,prev_point_inside_frustum;
         bool skip_remaining_points;
         unsigned int px,py,diagonal_y;
         unsigned int min_px,min_py,max_px,max_py;
         threevector currpoint;
   
         extremal_frustum_points(
            f,min_px,min_py,max_px,max_py,ztwoDarray_ptr);
         for (px=min_px-1; px<max_px+1; px++)
         {
            diagonal_y=basic_math::round(min_py+(px-min_px)
                                         *(max_py-min_py)/(max_px-min_px));

            prev_point_inside_frustum=skip_remaining_points=false;
            for (py=diagonal_y-1; py<max_py+1; py++)
            {
               if (!skip_remaining_points)
               {
                  ztwoDarray_ptr->pixel_to_point(px,py,currpoint);

                  if (f.point_inside_xyprojected_frustum(currpoint))
                  {
                     curr_point_inside_frustum=true;
                     ztwoDarray_ptr->put(px,py,intensity_value);
                  }
                  else
                  {
                     curr_point_inside_frustum=false;
                  }

                  if (prev_point_inside_frustum && !curr_point_inside_frustum)
                  {
                     skip_remaining_points=true;
                  }
                  prev_point_inside_frustum=curr_point_inside_frustum;
            
               }  // skip remaining points conditional
            }  // py loop

            prev_point_inside_frustum=skip_remaining_points=false;
            for (py=diagonal_y-1; py>min_py-1; py--)
            {
               if (!skip_remaining_points)
               {
                  ztwoDarray_ptr->pixel_to_point(px,py,currpoint);

                  if (f.point_inside_xyprojected_frustum(currpoint))
                  {
                     curr_point_inside_frustum=true;
                     ztwoDarray_ptr->put(px,py,intensity_value);
                  }
                  else
                  {
                     curr_point_inside_frustum=false;
                  }

                  if (prev_point_inside_frustum && !curr_point_inside_frustum)
                  {
                     skip_remaining_points=true;
                  }
                  prev_point_inside_frustum=curr_point_inside_frustum;   
               }  // skip remaining points conditional
            }  // py loop
         }  // px loop
      }

// ---------------------------------------------------------------------
// Method find_nearest_pixel_with_specified_value takes in a position
// vector r_vec and a pixel intensity_value pixel_val.  It searches
// within a distance max_search_dist about the line segment running
// through r_vec with direction vector e_hat for a pixel whose binary
// thresholded z intensity_value exceeds 0 along the ray defined by
// input unit vector eperp_hat which is orthogonal to e_hat.
// Find_nearest_pixel_with_specified_intensity_value computes the
// distance to this pixel and returns its vector position:

   threevector find_nearest_pixel_with_specified_value(
      const threevector& r_vec,const linesegment& l,
      const threevector& eperp_hat,
      double max_search_dist,int pixel_val,double& dist_to_pixel,
      twoDarray const *zbinary_twoDarray_ptr)
      {
         bool found_pixel=false;
         double ds=basic_math::min(zbinary_twoDarray_ptr->get_deltax(),
                       zbinary_twoDarray_ptr->get_deltay());
         threevector s_vec;
   
         int nsteps=basic_math::round(max_search_dist/ds);
         int i=0;
         do
         {
            s_vec=r_vec+i*ds*eperp_hat;
            unsigned int sx,sy;
            zbinary_twoDarray_ptr->point_to_pixel(
               s_vec.get(0),s_vec.get(1),sx,sy);
            if (zbinary_twoDarray_ptr->get(sx,sy)==pixel_val)
            {
               found_pixel=true;
               dist_to_pixel=i*ds;
            }
            i++;
         }
         while(!found_pixel && i<nsteps);
   
         if (!found_pixel)
         {
            dist_to_pixel=POSITIVEINFINITY;
         }
         return s_vec;
      }

// ---------------------------------------------------------------------
// Method project_rectangle_interior scans along a
// rectangular lattice defined by rectangle r's direction vectors what
// and lhat and by cell width and length dimensions dw and dl.  The x
// and y projections of each point along the rectangle which may be
// arbitrarily oriented in 3D are calculated.  The pixel corresponding
// to this x-y location along with its 8 surrounding neighbors are
// then colored within input array zarray.

//   void project_rectangle_interior(
//      const rectangle& r,double dw,double dl,double intensity_value,
//      twoDarray* ztwoDarray_ptr)
//      {
//         int nwbins=mathfunc::mytruncate(r.width/dw)+1;
//         int nlbins=mathfunc::mytruncate(r.get_length()/dl)+1;

//         for (unsigned int i=0; i<nwbins; i++)
//         {
//            double w=0+i*dw;
//            for (unsigned int j=0; j<nlbins; j++)
//            {
//               double l=0+j*dl;
//               threevector currpoint=r.get_vertex(0)+w*r.what+l*r.lhat;
//               double x=currpoint.get(0);
//               double y=currpoint.get(1);
//               unsigned int px,py;
//               if (ztwoDarray_ptr->point_to_pixel(x,y,px,py))
//               {
//   		    ztwoDarray_ptr->put(px,py,intensity_value);
//               }
//            } // loop over index j
//         } // loop over index i
//      }

// ---------------------------------------------------------------------
// Method draw_badimage_slash draws a bright colored slash through
// images to indicate that their quality has automatically been deemed
// poor.

   void draw_badimage_slash(
      twoDarray* ztwoDarray_ptr,colorfunc::Color slashcolor)
      {
         const double dy=0.3;	 // meter
         vector<threevector> vertex(4);
   
         vertex[0]=threevector(20,20-dy,0);
         vertex[1]=threevector(20,20+dy,0);
         vertex[2]=threevector(-20,-20+dy,0);
         vertex[3]=threevector(-20,-20-dy,0);
         polygon slash(vertex);
         color_convex_quadrilateral_interior(
            slash,colorfunc::color_to_value(slashcolor),ztwoDarray_ptr);
      }
   
} // drawfuncs namespace





