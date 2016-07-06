// ==========================================================================
// DRAW3DFUNCS stand-alone methods
// ==========================================================================
// Last modified on 12/4/10; 1/29/12; 1/30/12; 4/5/14
// ==========================================================================

#include <fstream>
#include <iostream>
#include <set>
#include "threeDgraphics/character.h"
#include "color/colorfuncs.h"
#include "math/constants.h"
#include "geometry/contour.h"
#include "threeDgraphics/draw3Dfuncs.h"
#include "general/filefuncs.h"
#include "geometry/linesegment.h"
#include "geometry/mybox.h"
#include "geometry/parallelogram.h"
#include "geometry/polygon.h"
#include "geometry/regular_polygon.h"
#include "math/rotation.h"
#include "general/stringfuncs.h"
#include "threeDgraphics/threeDstring.h"
#include "math/threematrix.h"
#include "math/threevector.h"
#include "track/track.h"
#include "image/TwoDarray.h"
#include "threeDgraphics/xyzpfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ios;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

namespace draw3Dfunc
{
   
   bool draw_thick_lines=false;
   double delta_phi=30*PI/180;	// radians
   double ds=0.05;	// meters	

//   double xlo=-200;	// meters
   double xlo=-300;	// meters
//   double xhi=600;	// meters
   double xhi=2000;	// meters
//   double ylo=-200;	// meters
   double ylo=-400;	// meters
//   double yhi=600;	// meters
   double yhi=1000;	// meters
   double zlo=-100;	// meters
   double zhi=100;	// meters

   bool check_for_shadows=false;
   vector<parallelepiped>* shadow_volume_ptr=NULL;

// ==========================================================================
// Geometrical primitives drawing methods
// ==========================================================================

// Method draw_line takes in the xyz coordinates of two points, the
// spacing ds between individual points which make up the 3D line as
// well as the filename for a binary output xyzp file.  It appends to
// the xyzp file a list of (x,y,z,p) quadruples which correspond to a
// 3D line running from point_1 to point_2.

   void draw_line(
      const linesegment& l,string xyzp_filename,double annotation_value)
      {
         draw_line(l.get_v1(),l.get_v2(),xyzp_filename,annotation_value);
      }

   void draw_line(
      const threevector& point_1,const threevector& point_2,
      string xyzp_filename,double annotation_value)
      {
         if (draw_thick_lines)
         {
            draw_thick_line(point_1,point_2,xyzp_filename,annotation_value);
         }
         else
         {
            draw_thin_line(point_1,point_2,xyzp_filename,annotation_value);
         }
      }

// ---------------------------------------------------------------------
// Method draw_thin_line takes in the xyz coordinates of two points, the
// spacing ds between individual points which make up the 3D line as
// well as the filename for a binary output xyzp file.  It appends to
// the xyzp file a list of (x,y,z,p) quadruples which correspond to a
// 3D line running from point_1 to point_2.

   void draw_thin_line(
      const threevector& point_1,const threevector& point_2,
      string xyzp_filename,double annotation_value)
      {
         ofstream binary_outstream;
         binary_outstream.open(xyzp_filename.c_str(),ios::app|ios::binary);

         double distance=(point_2-point_1).magnitude();

         const int min_nsteps=5;
         int nsteps=basic_math::max(min_nsteps,basic_math::round(distance/ds));
         for (int n=0; n<nsteps; n++)
         {
            threevector curr_point=point_1+n*(point_2-point_1)/
               double(nsteps-1);

// Don't add curr_point to xyzp output file if it lies outside some
// reasonable drawing volume:

//            if (curr_point.e[0] > xlo && curr_point.e[0] < xhi &&
//                curr_point.e[1] > ylo && curr_point.e[1] < yhi &&
//                curr_point.e[2] > zlo && curr_point.e[2] < zhi)
            {
               xyzpfunc::write_single_xyzp_point(
                  binary_outstream,curr_point,annotation_value);
            }
         } // loop over index n
         binary_outstream.close();  
      }

// ---------------------------------------------------------------------
// Method draw_thick_line takes in a line segment l.  It draws
// multiple copies of this line segment in a circle around
// l.get_ehat().  The individual line segments are spaced apart in
// azimuth phi by input angle delta_phi (specified in degrees).

   void draw_thick_line(
      const threevector& point_1,const threevector& point_2,
      string xyzp_filename,double annotation_value)
      {
         linesegment l(point_1,point_2);
         threevector e_hat(l.get_ehat()),f_hat;
         double x_dotproduct=e_hat.dot(x_hat);
         double z_dotproduct=e_hat.dot(z_hat);
         if (fabs(x_dotproduct) < fabs(z_dotproduct))
         {
            f_hat=(x_hat.cross(e_hat)).unitvector();
         }
         else
         {
            f_hat=(z_hat.cross(e_hat)).unitvector();
         }
         threevector g_hat(e_hat.cross(f_hat));
         
         double phi_start=0;
         double phi_stop=2*PI;
         unsigned int nbins=basic_math::round(
            (phi_stop-phi_start)/delta_phi)-1;
         for (unsigned int n=0; n<=nbins; n++)
         {
            double phi=phi_start+n*delta_phi;
            threevector rho_hat(cos(phi)*f_hat+sin(phi)*g_hat);
            const double rho=ds;
            threevector v1(l.get_v1()+rho*rho_hat);
            threevector v2(l.get_v2()+rho*rho_hat);
            draw_thin_line(v1,v2,xyzp_filename,annotation_value);
         } // loop over index n labeling thin lines making up thick line
      }

// ---------------------------------------------------------------------
// Method draw_vector calls draw_line and then adds an arrow head at
// the tip of the line segment.  The vector's basepoint location is
// explicitly passed as an input argument.  Normal direction vector
// n_hat must be orthogonal to input vector v.

   void draw_vector(
      const threevector& v,const threevector& basepoint,
      string xyzp_filename,double annotation_value,const threevector& n_hat,
      double arrow_length_frac)
      {
         const double theta=5*PI/6;
         const double costheta=cos(theta);
         const double sintheta=sin(theta);
         double s=arrow_length_frac*v.magnitude();
//         double s=0.1*v.magnitude();
         
         threevector v_hat(v.unitvector());
         threevector w_hat(n_hat.cross(v_hat));

         rotation Tinv;
         Tinv.put_column(0,v_hat);
         Tinv.put_column(1,w_hat);
         Tinv.put_column(2,n_hat);
         rotation T(Tinv.transpose());
         
         rotation R;
         R.identity();
         R.put(0,0,costheta);
         R.put(0,1,-sintheta);
         R.put(1,0,sintheta);
         R.put(1,1,costheta);
         rotation Rinv(R.transpose());
   
         rotation Rtotal(Tinv*R*T);
         rotation Rtotal_inv(Tinv*Rinv*T);

         linesegment l(basepoint,basepoint+v);
         threevector v3(l.get_v2()+s*(Rtotal*l.get_ehat()));
         threevector v4(l.get_v2()+s*(Rtotal_inv*l.get_ehat()));
         linesegment l3(l.get_v2(),v3);
         linesegment l4(l.get_v2(),v4);

         draw_line(l,xyzp_filename,annotation_value);
         draw_line(l3,xyzp_filename,annotation_value);
         draw_line(l4,xyzp_filename,annotation_value);
      }

   void draw_vector(
      const threevector& v,const threevector& basepoint,
      string xyzp_filename,double annotation_value,double arrow_length_frac)
      {
         draw_vector(v,basepoint,xyzp_filename,annotation_value,z_hat,
                     arrow_length_frac);
      }

// ---------------------------------------------------------------------
// Method draw_mid_vector places the arrow tip near the midpoint of
// the vector's line segment rather than at its head:

   void draw_mid_vector(
      const threevector& v,const threevector& basepoint,
      string xyzp_filename,double annotation_value)
      {
         const double arrow_length_frac=0.25;
         const double frac=0.6;
         draw_vector(frac*v,basepoint,xyzp_filename,annotation_value,
                     arrow_length_frac);
         linesegment l(basepoint+(1-frac)*v,basepoint+v);
         draw_line(l,xyzp_filename,annotation_value);
      }

// ---------------------------------------------------------------------
// Method draw_symmetry_directions adds rays emanating from
// (x,y,z)=(0,0,z_vector) in the symmetry angle & symmetry angle+90
// degrees directions to the xyzp points file.

   void draw_symmetry_directions(
      double z_vector,double symmetry_angle,string xyzp_filename,
      double annotation_value)
      {
         filefunc::gunzip_file_if_gzipped(xyzp_filename);
         threevector point1(0,0,z_vector);
         threevector point2(10*cos(symmetry_angle),10*sin(symmetry_angle),
                         z_vector);
         threevector v1=point2-point1;
         draw_vector(v1,point1,xyzp_filename,annotation_value);
         threevector point3(0,0,z_vector);
         threevector point4(-20*sin(symmetry_angle),20*cos(symmetry_angle),
                         z_vector);
         threevector v2=point4-point3;
         draw_vector(v2,point3,xyzp_filename,annotation_value);

         const threevector fake_posn(0,0,50);
         append_fake_xyzp_points_for_dataviewer_coloring(
            xyzp_filename,fake_posn);
         filefunc::gzip_file_if_gunzipped(xyzp_filename);         
      }

// ---------------------------------------------------------------------
// Method draw_track loops over each linesegment within input track
// *track_ptr and draws its to output xyzp_filename.

   void draw_track(
      track* track_ptr,std::string xyzp_filename,double annotation_value)
      {
         vector<linesegment>* segment_ptr=track_ptr->get_segment_ptr();
         for (unsigned int i=0; i<segment_ptr->size(); i++)
         {
            linesegment l( (*segment_ptr)[i] );
            draw_line(l,xyzp_filename,annotation_value);
         }
      }

// ---------------------------------------------------------------------
// Method draw_rectangle_grid takes in a rectangular polygon.  It
// draws a meshed rectangle onto the output xyzp file specified by
// input file name xyzp_filename.  The size of the mesh cells is set
// by parameters delta_u and delta_v below.

   void draw_rectangle_grid(
      polygon& rectangle,string xyzp_filename,
      double annotation_value,double delta_u,double delta_v)
      {
         rectangle.initialize_edge_segments();
         unsigned int n_ulines=basic_math::round(
            rectangle.get_edge(0).get_length()/delta_u)+1;
         unsigned int n_vlines=basic_math::round(
            rectangle.get_edge(1).get_length()/delta_v)+1;

         for (unsigned int n=0; n<=n_ulines; n++)
         {
            double f=double(n)/double(n_ulines);
            draw_line(
               rectangle.get_edge(0).get_midpoint(f),
               rectangle.get_edge(2).get_midpoint(1-f),
               xyzp_filename,annotation_value);
         }

         for (unsigned int n=0; n<=n_vlines; n++)
         {
            double f=double(n)/double(n_vlines);
            draw_line(
               rectangle.get_edge(1).get_midpoint(f),
               rectangle.get_edge(3).get_midpoint(1-f),
               xyzp_filename,annotation_value);
         }
      }
   
// ---------------------------------------------------------------------
// Method draw_zplane takes in the z value for a plane which is
// assumed to be parallel to the x-y plane.  It draws a regular
// lattice (whose cell size is set by parameter delta_x and delta_y)
// over xlo < x < xhi, ylo < y < yhi and z=z_plane onto the output
// binary xyzp file.

   void draw_zplane(
      double z_plane,double xhi,double xlo,double yhi,double ylo,
      string xyzp_filename,double annotation_value)
      {
         double delta_x=1;	// x-cell size in meters
         double delta_y=1;	// y-cell size in meters
         unsigned int n_xlines=basic_math::round((xhi-xlo)/delta_x)+1;
         unsigned int n_ylines=basic_math::round((yhi-ylo)/delta_y)+1;

         double x1,x2,y1,y2;
         for (unsigned int n=0; n<n_xlines; n++)
         {
            x1=x2=xlo+n*delta_x;
            y1=ylo;
            y2=yhi;
            draw_line(
               threevector(x1,y1,z_plane),threevector(x2,y2,z_plane),
               xyzp_filename,annotation_value);
         }
         for (unsigned int n=0; n<n_ylines; n++)
         {
            x1=xlo;
            x2=xhi;
            y1=y2=ylo+n*delta_y;
            draw_line(
               threevector(x1,y1,z_plane),threevector(x2,y2,z_plane),
               xyzp_filename,annotation_value);
         }
      }

// ---------------------------------------------------------------------
// Method draw_polygon takes in a polygon which lies in some generally
// oriented plane.  It draws the polygon's edges onto the output
// binary xyzp file.

   void draw_polygon(const polygon& poly,string xyzp_filename,
                     double annotation_value)
      {
         if (poly.get_nvertices() >= 3)
         {
            for (unsigned int i=0; i<poly.get_nvertices(); i++)
            {
               draw_line(
                  poly.get_vertex(i),
                  poly.get_vertex(modulo(i+1,poly.get_nvertices())),
                  xyzp_filename,annotation_value);
            } // loop over index i
         }
      }

// ---------------------------------------------------------------------
// Method draw_contour takes in a contour which lies in some generally
// oriented plane.  It draws the contours's edges onto the output
// binary xyzp file.

   void draw_contour(const contour& c,string xyzp_filename,
                     double annotation_value)
      {
         if (c.get_nvertices() >= 3)
         {
            for (unsigned int i=0; i<c.get_nvertices(); i++)
            {
               draw_line(c.get_vertex(i).first,
                         c.get_vertex(modulo(i+1,c.get_nvertices())).first,
                         xyzp_filename,annotation_value);
            } // loop over index i
         } // nvertices >= 3 conditional
      }

// ---------------------------------------------------------------------
// Method draw_circle takes in a radius and center position for a
// circle.  It draws the circle's outline onto the output binary xyzp
// file specified by xyzp_filename.

   void draw_circle(double radius,threevector& center_posn,
                    string xyzp_filename,double annotation_value)
      {
         const int nsides=50;
         regular_polygon circle(nsides,radius);
         circle.absolute_position(center_posn);
         draw_polygon(circle,xyzp_filename,annotation_value);
      }
   
// ---------------------------------------------------------------------
// Method draw_parallelepiped draws the outline of a parallelepiped
// onto the output binary xyzp file.

   void draw_parallelepiped(const parallelepiped& p,string xyzp_filename,
                            double annotation_value)
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
            draw_polygon(face[f],xyzp_filename,annotation_value);
         } // loop over index f labeling current face
      }

// ---------------------------------------------------------------------
// Method draw_tiny_cube lets the user mark individual 3-points with
// small, point-like objects:

   void draw_tiny_cube(const threevector& point,string xyzp_filename,
                       double annotation_value,double side_length)
      {
         mybox tiny_cube(side_length,side_length,side_length);
         tiny_cube.translate(point);
         draw_parallelepiped(tiny_cube,xyzp_filename,annotation_value);
      }

// ---------------------------------------------------------------------
// Method draw_contour_cylinder takes in contour c which is assumed to
// lie within the xy plane.  It also takes in a cylinder height.  This
// method draws contour c along with an extruded version of c offset
// in z by the cylinder height.  It subsequently connects each pair of
// corresponding vertices within the two contours by vertical lines.

   void draw_contour_cylinder(
      const contour& c,double height,string xyzp_filename,
      double annotation_value)
      {
         contour c_extrude(c);
         threevector z_trans=height*z_hat;
         c_extrude.translate(z_trans);

//         cout << "Contour c = " << c << endl;
//         cout << "Contour c_extrude = " << c_extrude << endl;

         draw_contour(c,xyzp_filename,annotation_value);
         draw_contour(c_extrude,xyzp_filename,annotation_value);
       
         for (unsigned int i=0; i<c.get_nvertices(); i++)
         {
            draw_line(c.get_vertex(i).first,c_extrude.get_vertex(i).first,
                      xyzp_filename,annotation_value);
         } // loop over index i
      }

// This overloaded version of method draw_contour_cylinder takes in a
// general contour c which is NOT assumed to lie within the xy plane.
// This method draws contour c along with its projection in the xy
// plane.  It subsequently connects each pair of corresponding
// vertices within the two contours by vertical lines.

   void draw_contour_cylinder(
      const contour& c,string xyzp_filename,double annotation_value)
      {
         contour c_projection(c);
         c_projection.xy_projection();
         draw_contour(c,xyzp_filename,annotation_value);
         draw_contour(c_projection,xyzp_filename,annotation_value);

         for (unsigned int i=0; i<c.get_nvertices(); i++)
         {
            draw_line(c.get_vertex(i).first,c_projection.get_vertex(i).first,
                      xyzp_filename,annotation_value);
         } // loop over index i
      }

// ---------------------------------------------------------------------
// Method color_binary_region takes in a binary image within
// *zbinary_twoDarray_ptr along with xy-bounding box information.  It
// loops over all pixels within this bounding box and searches for
// non-zero entries within *zbinary_twoDarray_ptr.  When it finds a
// non-zero valued pixel, it generates an XYZP quadruple where Z and P
// are respectively set equal to z_region and annotation_value.  It
// then writes this XYZP quadruple to the binary output file specified
// by input filename xyzp_filename.

   void color_binary_region(
      string xyzp_filename,
      double min_x,double min_y,double max_x,double max_y,
      double z_region,double annotation_value,
      twoDarray const *zbinary_twoDarray_ptr)
      {
         ofstream binary_outstream;
         binary_outstream.open(xyzp_filename.c_str(),ios::app|ios::binary);

         unsigned int px_min,py_min,px_max,py_max;
         zbinary_twoDarray_ptr->bbox_corners_to_pixels(
            min_x,min_y,max_x,max_y,px_min,py_min,px_max,py_max);
         threevector curr_point;
         
         const double SMALL=0.1;
         for (unsigned int px=px_min; px<px_max; px++)
         {
            for (unsigned int py=py_min; py<py_max; py++)
            {
               if (zbinary_twoDarray_ptr->get(px,py) > SMALL)
               {
                  zbinary_twoDarray_ptr->pixel_to_point(px,py,curr_point);
                  curr_point.put(2,z_region);
                  xyzpfunc::write_single_xyzp_point(
                     binary_outstream,curr_point,annotation_value);
               }
            } // loop over py index
         } // loop over px index

         binary_outstream.close();  
      }

// ---------------------------------------------------------------------
// Method fill_bbox takes in corner vertices (x0,y0) and (x1,y1) for a
// rectangular bounding box.  This method draws multiple lines to fill
// in the bounding box in the output binary xyzp file specified by
// the input filename.

   void fill_bbox(
      string xyzp_filename,double x0,double y0,double x1,double y1,
      double annotation_value)
      {
         ofstream binary_outstream;
         binary_outstream.open(xyzp_filename.c_str(),ios::app|ios::binary);

         threevector point_1(x0,y0);
         threevector point_2(x1,y0);
         threevector point_3(x0,y1);
         threevector point_4(x1,y1);
         double distance=(point_2-point_1).magnitude();
         int nsteps=basic_math::round(distance/ds);

         for (int n=0; n<nsteps; n++)
         {
            threevector start_point=
               point_1+n*(point_2-point_1)/double(nsteps-1);
            threevector stop_point=
               point_3+n*(point_4-point_3)/double(nsteps-1);
            draw_line(start_point,stop_point,xyzp_filename,
                      annotation_value);
         } // loop over index n

         binary_outstream.close();  
      }

// ---------------------------------------------------------------------
// Method append_fake_xyzp_points_for_dataviewer_coloring adds a few
// points into an xyzp file to fix the Group 94 dataviewer's colormap.

// p value      JET color	JET+white	Hue+value

//   0.0        indigo		indigo		dark red
//   0.1       	dark blue	dark blue	orange
//   0.2       	dark blue	dark blue	dark yellow
//   0.3        medium blue	medium blue	olive
//   0.4	blue-green	blue-green	bright green
//   0.5       	green		green		dark green
//   0.6       	yellow		yellow		dark cyan
//   0.7       	orange		orange		blue
//   0.8       	red-orange	red		indigo
//   0.9       	red		brick red	purple
//   1.0       	brick red	white		white

// p value      JET+white	Hue+value

//   0.15       royal blue	light orange
//   0.45       green		green
//   0.55	yellow		dark olive
//   0.65 	light orange	bright blue
//   0.9  	brick red       purple
//   0.95  	pink		bright violet
//   1		white		white
   
   void append_fake_xyzp_points_for_dataviewer_coloring(
      string xyzp_filename,const threevector& fake_posn,bool gzip_xyzp_file)
      {
         filefunc::gunzip_file_if_gzipped(xyzp_filename);
         ofstream binary_outstream;
         binary_outstream.open(xyzp_filename.c_str(),ios::app|ios::binary);

         for (unsigned int i=0; i<=20; i++)
         {
//            threevector currpoint=fake_posn+i*threevector(0,0,0.001);
            threevector currpoint=fake_posn;
            double annotation_value=i*0.05;
            xyzpfunc::write_single_xyzp_point(
               binary_outstream,currpoint,annotation_value);
         }
         binary_outstream.close();  
         if (gzip_xyzp_file) filefunc::gzip_file_if_gunzipped(xyzp_filename);
      }

// This essentially overloaded version of
// append_fake_xyzp_points_for_dataviewer_coloring places the fake
// coloring points deep within the center of the input twoDarray
// *ftwoDarray_ptr.  This placement is sometimes necessary for ALIRT
// data processing in order not to foul up ladar data bounding box
// computations.

   void append_fake_xyzp_points_in_twoDarray_middle(
      twoDarray const *ftwoDarray_ptr,string xyzp_filename,
      bool gzip_xyzp_file)
      {
         threevector fake_point_inside_image;
         ftwoDarray_ptr->pixel_to_point(
            basic_math::round(0.5*ftwoDarray_ptr->get_mdim()),
            basic_math::round(0.5*ftwoDarray_ptr->get_ndim()),
            fake_point_inside_image);

         filefunc::gunzip_file_if_gzipped(xyzp_filename);
         append_fake_xyzp_points_for_dataviewer_coloring(
            xyzp_filename,fake_point_inside_image,gzip_xyzp_file);
      }

// Method append_fake_z_points_in_twoDarray_middle places a column of
// fake dots within the center of input twoDarray *ztwoDarray_ptr.
// The dots are displaced in height and are meant to fix the z rather
// than p colormap.  

   void append_fake_z_points_in_twoDarray_middle(
      twoDarray const *ztwoDarray_ptr,string xyzp_filename,
      double zlo,double zhi)
      {
         filefunc::gunzip_file_if_gzipped(xyzp_filename);
         threevector fake_posn;
         ztwoDarray_ptr->pixel_to_point(
            basic_math::round(0.5*ztwoDarray_ptr->get_mdim()),
            basic_math::round(0.5*ztwoDarray_ptr->get_ndim()),fake_posn);

         ofstream binary_outstream;
         binary_outstream.open(xyzp_filename.c_str(),ios::app|ios::binary);

//         double zlo=-10; 	// meters
//         double zhi=50;	// meters
         int nbins=21;
         double dz=(zhi-zlo)/(nbins-1);
         for (unsigned int i=0; i<=20; i++)
         {
            double z=zlo+i*dz;
            threevector currpoint(fake_posn+z*z_hat);
            xyzpfunc::write_single_xyzp_point(binary_outstream,currpoint,1.0);
         }
         binary_outstream.close();  
         filefunc::gzip_file_if_gunzipped(xyzp_filename);
      }

// ==========================================================================
// Coordinate system annotation methods:
// ==========================================================================

// Method draw_character takes in a 3D character object c which is
// built up out of line segments.  This method renders each of the
// segments as thick lines within the output file specified by input
// xyzp_filename.

   void draw_character(
      const character& c,string xyzp_filename,double annotation_value)
      {
         vector<linesegment> segment=c.get_segment();
         for (unsigned int i=0; i<segment.size(); i++)
         {
            draw_line(segment[i],xyzp_filename,annotation_value);
         } // loop over index i 
      }

// ---------------------------------------------------------------------
// Method draw_threeDstring takes in threeDstring object s which is
// basically an STL vector of 3D characters.  It calls the preceding
// draw_character method for each of its constiuent characters.

   void draw_threeDstring(
      const threeDstring& s,string xyzp_filename,double annotation_value)
      {
         for (unsigned int i=0; i<s.get_nchars(); i++)
         {
            draw_character(s.get_char(i),xyzp_filename,annotation_value);
         }
      }

// ---------------------------------------------------------------------
// Method draw_coordinate_system takes in a rectangle grid_rectangle
// which surrounds the data already assumed to exist within the output
// xyzp file specified by xyzp_filename.  This rectangle is oriented
// along some u and v axes, and its size is assumed to be integer
// multiples of input distances delta_u and delta_v.  This method
// first outputs a grid corresponding to the input rectangle.  It
// subsequently labels the u and v tic lines with threeDstrings
// corresponding to the integer multiples of delta_u and delta_v.
// Finally, this method convers the u and v axes' labels into
// threeDstrings which it writes to the output xyzp file.

   void draw_coordinate_system(
      polygon& grid_rectangle,string xyzp_filename,
      double annotation_value,double delta_u,double delta_v,
      string u_axis_label,string v_axis_label,double tic_label_size,
      double axes_label_size)
      {
         filefunc::gunzip_file_if_gzipped(xyzp_filename);
         grid_rectangle.initialize_edge_segments();
         threevector uhat(grid_rectangle.get_edge(0).get_ehat());
         threevector vhat(grid_rectangle.get_edge(1).get_ehat());
         unsigned int n_ulines=basic_math::round(
            grid_rectangle.get_edge(0).get_length()/delta_u);
         unsigned int n_vlines=basic_math::round(
            grid_rectangle.get_edge(1).get_length()/delta_v);

// Draw vertical grid lines:

         for (unsigned int m=0; m<=n_ulines; m++)
         {
            double f=double(m)/double(n_ulines);
            draw_line(
               grid_rectangle.get_edge(0).get_midpoint(f),
               grid_rectangle.get_edge(2).get_midpoint(1-f),
               xyzp_filename,annotation_value);

// Label horizontal u-axis tic marks:

//            if (m >= 1)
            {
               threevector tic_label_location(
                  grid_rectangle.get_edge(0).get_midpoint(f)-
                  0.33*delta_v*vhat);
               int tic_value=basic_math::round(m*delta_u);
               threeDstring tic_label(
                  stringfunc::number_to_string(tic_value));
               tic_label.scale(tic_label.get_origin().get_pnt(),tic_label_size);
               tic_label.center_upon_location(tic_label_location);
               draw_threeDstring(tic_label,xyzp_filename,annotation_value);
            }
         } // loop over index m

// Add u-axis label:

         threevector label_location(grid_rectangle.get_edge(0).get_midpoint()-
                                 1.0*delta_v*vhat);
         threeDstring axis_label(u_axis_label);
         axis_label.scale(axis_label.get_origin().get_pnt(),axes_label_size);
         axis_label.center_upon_location(label_location);
         draw_threeDstring(axis_label,xyzp_filename,annotation_value);

// Draw horizontal grid lines:

         for (unsigned int n=0; n<=n_vlines; n++)
         {
            double f=double(n_vlines-n)/double(n_vlines);
            draw_line(
               grid_rectangle.get_edge(1).get_midpoint(f),
               grid_rectangle.get_edge(3).get_midpoint(1-f),
               xyzp_filename,annotation_value);

// Label vertical v-axis tic marks:

//            if (n >= 1)
            {
               threevector label_location(
                  grid_rectangle.get_edge(3).get_midpoint(f)-
                  0.3*delta_u*uhat);
               int tic_value=basic_math::round(n*delta_v);
               threeDstring tic_label(
                  stringfunc::number_to_string(tic_value));
               tic_label.scale(tic_label.get_origin().get_pnt(),tic_label_size);
               tic_label.rotate(tic_label.get_origin().get_pnt(),0,0,0.5*PI);
               tic_label.center_upon_location(label_location);
               draw_threeDstring(tic_label,xyzp_filename,annotation_value);
            }
         } // loop over index n

// Add v-axis label:

         label_location=threevector(grid_rectangle.get_edge(3).get_midpoint()-
                                 0.7*delta_u*uhat);
         axis_label=threeDstring(v_axis_label);
         axis_label.scale(axis_label.get_origin().get_pnt(),axes_label_size);
         axis_label.rotate(axis_label.get_origin().get_pnt(),0,0,0.5*PI);
         axis_label.center_upon_location(label_location);
         draw_threeDstring(axis_label,xyzp_filename,annotation_value);

         filefunc::gzip_file_if_gunzipped(xyzp_filename);
      }

// ---------------------------------------------------------------------
// Method draw_3D_axes draws 3 orthogonal vectors intersecting at the
// input origin point to the output xyzp file.

   void draw_3D_axes(
      const threevector& origin,double max_extent,double axes_label_size,
      string xyzp_filename,double annotation_value,
      string X_axis_label,string Y_axis_label,string Z_axis_label)
      {
         filefunc::gunzip_file_if_gzipped(xyzp_filename);

         threevector x_end(origin+max_extent*x_hat);
         threevector y_end(origin+max_extent*y_hat);
         threevector z_end(origin+max_extent*z_hat);

//         cout << "x_end = " << x_end << endl;
//         cout << "y_end = " << y_end << endl;
//         cout << "z_end = " << z_end << endl;

         draw_thick_lines=true;

         draw_vector(x_end,origin,xyzp_filename,annotation_value);
         draw_vector(y_end,origin,xyzp_filename,annotation_value);
         draw_vector(z_end,origin,xyzp_filename,annotation_value,x_hat);

// Add axes labels:

         threevector xlabel_location(x_end+0.1*max_extent*y_hat);
         threeDstring x_axis_label(X_axis_label);
         x_axis_label.scale(x_axis_label.get_origin().get_pnt(),axes_label_size);
         x_axis_label.center_upon_location(xlabel_location);
         draw_threeDstring(x_axis_label,xyzp_filename,annotation_value);

         threevector ylabel_location(y_end-0.1*max_extent*x_hat);
         threeDstring y_axis_label(Y_axis_label);
         y_axis_label.scale(y_axis_label.get_origin().get_pnt(),axes_label_size);
         y_axis_label.center_upon_location(ylabel_location);
         draw_threeDstring(y_axis_label,xyzp_filename,annotation_value);

         threevector zlabel_location(z_end-0.1*max_extent*y_hat);
         threeDstring z_axis_label(Z_axis_label);
//         z_axis_label.rotate(z_axis_label.get_origin().get_pnt(),PI/2,0,PI/2);
         z_axis_label.scale(z_axis_label.get_origin().get_pnt(),axes_label_size);
         z_axis_label.center_upon_location(zlabel_location);

         z_axis_label.translate(threevector(0,10*axes_label_size,0));

         draw_threeDstring(z_axis_label,xyzp_filename,annotation_value);

         draw_thick_lines=false;

//         filefunc::gzip_file_if_gunzipped(xyzp_filename);
      }

// ==========================================================================
// Colorbar display methods:
// ==========================================================================

// Method draw_colorbar adds a colorbar into the XYZP file
// colorbar_filename.  If boolean flag display_powers_of_two==true,
// the numerals on this colorbar are 1,2,4,8,16,...  Otherwise, they
// range over the set {0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6,
// 0.7, 0.8, 0.9, 1.0}.

// As of 12/17/04, this method still needs a lot more work before
// it'll be robust and general...

   void draw_colorbar(
      int n_color_tiles,double charsize,
      const threevector& trans_global,string colorbar_filename,
      double delta_color_value,bool display_powers_of_two)
      {
         const double du=0.1; // meter
         const double dv=du;
         double xsize=charsize;
         double ysize=xsize;
         parallelogram rectangle(xsize,ysize);

         unsigned int i_max=n_color_tiles;
         double char_color_value;
         if (!display_powers_of_two) 
         {
            i_max=n_color_tiles+1;
         }
         
         for (unsigned int i=0; i<i_max; i++)
         {
            parallelogram working_rectangle(rectangle);
            threevector trans=i*xsize*x_hat+trans_global;
            working_rectangle.translate(trans);

// If boolean display_powers_of_two==true, we assume that the xyzp
// file contains p values shown on a log10 scale.  We then generate
// color tiles which are also on this same log10 scale.  But we
// superpose numerals on top of the tiles which are not on a log10
// scale but rather genuine powers of 2...

//            const double delta_color_value=0.02;
//            const double delta_color_value=0.1;
            double annotation_value;
            if (display_powers_of_two)
            {
               annotation_value=basic_math::round(pow(2,i));
            }
            else
            {
               annotation_value=delta_color_value*i;
            }
            
// Convert annotation_value from a double into a string and then into
// a threeDstring:

            string numberstring;
            if (basic_math::is_int(annotation_value))
            {
               numberstring=stringfunc::number_to_string(annotation_value);
            }
            else
            {
               numberstring=stringfunc::number_to_string(annotation_value,2);
            }

// Strip away any leading zeros appearing before a decimal place:

            string stripped_numberstring(
               stringfunc::remove_leading_zeros(numberstring));
            threeDstring cstring(stripped_numberstring);
            cstring.scale(cstring.get_origin().get_pnt(),charsize/10.0);

            double char_xtrans=0;
            double char_ytrans=-0.33*ysize;
            double char_ztrans=0.25;	// meter
            if (numberstring.length()==1)
            {
               char_xtrans=-0.1*xsize;
            }
            else if (numberstring.length()==2)
            {
               char_xtrans=-0.33*xsize;
            }
            else if (numberstring.length() >= 3)
            {
               char_xtrans=-0.33*xsize;
//         cstring.scale(cstring.get_origin().get_pnt(),
//                       2.0/numberstring.length());
            }

            threevector char_trans(
               trans+threevector(char_xtrans,char_ytrans,char_ztrans));
            cstring.translate(char_trans);
            if (display_powers_of_two)
            {
               char_color_value=log10(pow(2,n_color_tiles-1))+1;
            }
            else
            {

// Recall that we have to incorporate a 0.1 fudge factor in order to
// offset the stupid group 94 convention of multiplying all output
// probabilities by a factor of 10...

               char_color_value=delta_color_value*i_max;
            }
            draw_threeDstring(cstring,colorbar_filename,char_color_value);

            cout << "Color tile #" << i << " corresponds to annotation_value "
                 << annotation_value << endl;

            double output_value=annotation_value;
            if (display_powers_of_two)
            {
               output_value=log10(annotation_value);
            }
            draw_rectangle_grid(
               working_rectangle,colorbar_filename,output_value,du,dv);
         } // loop over index i labeling color tiles

// Add a tiny cube in order to give some depth to the 2D planar color
// bar so that it can be viewed with the Group 94 dataviewer (as of
// late 2004):

         draw_tiny_cube(trans_global,colorbar_filename,0);

         cout << "Colorbar xyzp file written to " << colorbar_filename 
              << endl;
      }

// ==========================================================================
// 2D/3D imagery fusion methods:
// ==========================================================================

/*

// Method drape_PNG_image_onto_point_cloud takes in an XYZP point
// cloud, a PNG RGB image as well as a text file containing XYZ-UV tie
// point information.  It first computes the 3x4 projection matrix
// linking XYZ world space to UV image space.  This method next
// projects every XYZ point within the cloud onto the PNG image and
// retrieves its corresponding RGB values.  The XYZ point's P value is
// set equal to the RGB colormap value which approximates the PNG
// image's RGB values.  The draped point cloud is written to
// output_xyzp_filename.

   void drape_PNG_image_onto_point_cloud(
      string input_subdir,string png_filename,string xyzuv_filename,
      string input_xyzp_filename,string output_xyzp_filename,
      double missing_data_value,double min_range,double max_range,
      double min_height,double max_height)
      {

// Read and store contents of PNG photo:

         pngfunc::open_png_file(input_subdir+png_filename);
         pngfunc::parse_png_file();

// Perform geometric camera calibration for PNG photo:

         camerafunc::parse_xyzuv_data_file(input_subdir+xyzuv_filename);
         camerafunc::compute_projection_matrix();
         camerafunc::check_projection_matrix(input_subdir+xyzuv_filename);

         vector<fourvector>* xyzp_pnt_ptr=
            xyzpfunc::read_xyzp_float_data(input_xyzp_filename);

         ofstream binary_outstream;
         binary_outstream.open(
            output_xyzp_filename.c_str(),ios::app|ios::binary);

         fourvector curr_point;
         for (unsigned int i=0; i<xyzp_pnt_ptr->size(); i++)
         {
            curr_point=(*xyzp_pnt_ptr)[i];
            threevector XYZ_point(curr_point.get(0),curr_point.get(1),
                               curr_point.get(2));

// For draping photos onto static basement ladar lab 3D images, we
// restrict which XYZ points are colored based upon their range and
// height values:

            if (XYZ_point.get(1) > min_range && XYZ_point.get(1) < 
                max_range &&
                XYZ_point.get(2) > min_height && XYZ_point.get(2) < 
                max_height)
            {
               twovector image_point=
                  camerafunc::project_world_to_image_coordinates(XYZ_point);

               const double denom_factor=1.0/255.0;
               double rgb_colormap_value=missing_data_value;

               Triple<int,int,int> rgb;
               if (pngfunc::get_RGB_values(image_point,rgb))
               {
                  double r=rgb.first*denom_factor;
                  double g=rgb.second*denom_factor;
                  double b=rgb.third*denom_factor;
                  rgb_colormap_value=colorfunc::rgb_colormap_value(r,g,b);
               } // pngfunc::get_RGB_values conditional
               xyzpfunc::write_single_xyzp_point(
                  binary_outstream,XYZ_point,rgb_colormap_value);
            
            } // min_range & max_range conditional
            
         } // loop over index i labeling points within *xyzp_pnt_ptr

         delete xyzp_pnt_ptr;
         binary_outstream.close();  
         
         pngfunc::close_png_file();
         camerafunc::delete_allocated_matrices();
      }

// ---------------------------------------------------------------------
// Method drape_PNG_image_onto_polygons is intended to be high-level
// and user-friendly.  It takes in the names of a subdirectory, a PNG
// file, and an xyzuv text file containing correspondences between XYZ
// world space and UV image space.  The latter two files are assumed
// to reside within the subdirectory.  It also takes in the name of an
// output XYZP file as well as an STL vector of polygonal faces in
// world space.  This method drapes the RGB values within the 2D PNG
// file onto the 3D polygons.  It assigns p-values to XYZ points
// within the polygons so that when they are viewed using our "RGB"
// colormap in the group 94/106 3D dataviewer, their colors match
// fairly closely with those seen in the 2D PNG file.

   void drape_PNG_image_onto_polygons(
      string input_subdir,string png_filename,string xyzuv_filename,
      string xyzp_filename,const vector<polygon>& polygon_face,
      const double max_distance,const double delta_s,
      bool grayscale_output,bool orthographic_projection)
      {
         vector<double> max_dist_to_poly_edge;
         for (unsigned int i=0; i<polygon_face.size(); i++)
         {
            max_dist_to_poly_edge.push_back(max_distance);
         }
         drape_PNG_image_onto_polygons(
            input_subdir,png_filename,xyzuv_filename,xyzp_filename,
            polygon_face,max_dist_to_poly_edge,delta_s,grayscale_output,
            orthographic_projection);
      }
   
   void drape_PNG_image_onto_polygons(
      string input_subdir,string png_filename,string xyzuv_filename,
      string xyzp_filename,const vector<polygon>& polygon_face,
      const vector<double>& max_dist_to_poly_edge,const double delta_s,
      bool grayscale_output,bool orthographic_projection)
      {

// Read and store contents of PNG photo:

         pngfunc::open_png_file(input_subdir+png_filename);
         pngfunc::parse_png_file();

// Perform geometric camera calibration for PNG photo:

         if (orthographic_projection)
         {
            camerafunc::parse_orthographic_xyzuv_data_file(
               input_subdir+xyzuv_filename);
            camerafunc::compute_orthographic_projection_matrix();
         }
         else
         {
            camerafunc::parse_xyzuv_data_file(
               input_subdir+xyzuv_filename);
            camerafunc::compute_projection_matrix();
         }
         camerafunc::check_projection_matrix(
            input_subdir+xyzuv_filename,orthographic_projection);

// Drape PNG photo onto polygonal faces:

         for (unsigned int i=0; i<polygon_face.size(); i++)
         {
            polygon curr_face(polygon_face[i]);
            drape_PNG_image_onto_polygon(
               xyzp_filename,delta_s,curr_face,
               max_dist_to_poly_edge[i],grayscale_output,
               orthographic_projection);
         }
         
         pngfunc::close_png_file();
         camerafunc::delete_allocated_matrices();
      }

// ---------------------------------------------------------------------
// Method drape_PNG_image_onto_polygon takes in a world-space polygon
// as well as a linear sampling distance delta_s measured in meters.
// It first generates a 2D lattice of XYZ points which cover the
// polygon within *interior_points_ptr.  This method then projects
// each XYZ point in *interior_points_ptr onto the PNG image's UV
// coordinate system.  It looks up the RGB values at the UV location
// within the PNG image and computes the corresponding one-dimensional
// value for our "RGB" colormap within the Group 94/106 dataviewer.
// The XYZ points are written to the output xyzp_filename with their
// p-values set equal to the "RGB" colormap values.  

   void drape_PNG_image_onto_polygon(
      string xyzp_filename,double delta_s,polygon& poly,
      double max_dist_to_poly_edge,bool grayscale_output,
      bool orthographic_projection)
      {
         ofstream binary_outstream;
         binary_outstream.open(xyzp_filename.c_str(),ios::app|ios::binary);

         mymatrix Rtrans(poly.natural_coordinate_system());
         vector<pair<threevector,bool> >* interior_points_ptr=
            poly.generate_interior_points_list(
               delta_s,Rtrans,max_dist_to_poly_edge);
         cout << "interior_points_ptr->size() = "
              << interior_points_ptr->size() << endl;

         for (unsigned int i=0; i<interior_points_ptr->size(); i++)
         {
            threevector XYZ_point((*interior_points_ptr)[i].first);

// Check whether shadow testing is being performed, and if so, whether
// current XYZ point lies inside some shadow volume:

            if (!point_in_shadow(XYZ_point))
            {
               twovector image_point;
               if (orthographic_projection)
               {
                  image_point=
                     camerafunc::
                     orthographic_project_world_to_image_coordinates(
                        XYZ_point);
               }
               else
               {
                  image_point=
                     camerafunc::project_world_to_image_coordinates(
                        XYZ_point);
               }
            
               Triple<int,int,int> rgb;
               const double denom_factor=1.0/255.0;

               const double missing_data_value=colorfunc::rgb_colormap_value(
                  colorfunc::grey);
               double rgb_colormap_value=missing_data_value;
               if (pngfunc::get_RGB_values(image_point,rgb))
               {
                  double r=rgb.first*denom_factor;
                  double g=rgb.second*denom_factor;
                  double b=rgb.third*denom_factor;

// Don't bother to write out XYZP point if RGB lookup value is too
// close to pure black:

                  const double min_rgb_strength=0.005;
                  if (sqr(r)+sqr(g)+sqr(b) > min_rgb_strength)
                  {
                     if (grayscale_output)
                     {

// To convert an RGB image into its greyscale counterpart, set the
// saturation corresponding to every RGB triple equal to zero:

                        double h,s,v;
                        colorfunc::RGB_to_hsv(r,g,b,h,s,v);
                        colorfunc::hsv_to_RGB(h,0,v,r,g,b);
                     }
                     rgb_colormap_value=colorfunc::rgb_colormap_value(r,g,b);
                  } // RGB > min strength conditional
               } // pngfunc::get_RGB_values conditional
               xyzpfunc::write_single_xyzp_point(
                  binary_outstream,(*interior_points_ptr)[i].first,
                  rgb_colormap_value);
            } // XYZ point not in shadows conditional
         } // loop over index i labeling XYZ polygon interior points

         binary_outstream.close();  
         delete interior_points_ptr;
      }
*/

// ---------------------------------------------------------------------
// Method generate_3D_polygon_interior_points scans over every polygon
// within input STL vector polygon_face.  It computes interior XYZ
// points for each polygon within the STL vector, and it stores this
// spatial geometry information within the first field of the output
// STL vector *interior_points_ptr.  This method also checks how close
// the interior points lie to their polygon's edges.  The second
// boolean fields in *interior_points_ptr are set to true for those
// XYZ points lying within max_dist_to_poly_edge from the polygon's
// borders.  The remaining 3rd and 4th fields in *interior_points_ptr
// are set equal to dummy 0 values in this method.

   vector<quadruple>* 
      generate_3D_polygon_interior_points(
         const vector<polygon>& polygon_face,const double delta_s)
      {
         vector<double> max_dist_to_poly_edge;
         for (unsigned int i=0; i<polygon_face.size(); i++)
         {
            max_dist_to_poly_edge.push_back(POSITIVEINFINITY);
         }
         return generate_3D_polygon_interior_points(
            polygon_face,max_dist_to_poly_edge,delta_s);
      }

   vector<quadruple>* 
      generate_3D_polygon_interior_points(
         const vector<polygon>& polygon_face,
         const vector<double>& max_dist_to_poly_edge,const double delta_s)
      {
         vector<quadruple>* interior_points_ptr=new vector<quadruple>;
         
         for (unsigned int i=0; i<polygon_face.size(); i++)
         {
            polygon poly(polygon_face[i]);
            rotation Rtrans(poly.natural_coordinate_system());
            vector<pair<threevector,bool> >* interior_xyz_ptr=
               poly.generate_interior_points_list(
                  delta_s,Rtrans,max_dist_to_poly_edge[i],true);
            for (unsigned int j=0; j<interior_xyz_ptr->size(); j++)
            {
               interior_points_ptr->push_back(quadruple(
                  (*interior_xyz_ptr)[j].first,(*interior_xyz_ptr)[j].second,
                  0,0));
            } // loop over index j labeling interior xyz points
         } // loop over index i labeling polygon faces
         return interior_points_ptr;
      }

// ---------------------------------------------------------------------
// Method rotate_3D_polygon_interior_points applies rotation matrix R
// to the first XYZ member of every STL vector entry in
// *interior_points_ptr:

   void rotate_3D_polygon_interior_points(
      const rotation& R,vector<quadruple>* interior_points_ptr)
      {
         for (unsigned int i=0; i<interior_points_ptr->size(); i++)
         {
            threevector XYZ_point((*interior_points_ptr)[i].first);
            (*interior_points_ptr)[i].first=R*XYZ_point;
         }
      }

/*
// ---------------------------------------------------------------------
// Method drape_PNG_image_onto_multiple_polygons takes in a PNG image
// along with a text file containing XYZ-UV tiepoint information.  If
// input boolean flag orthographic_projection==true [false], the input
// image is assumed to represent SAR [optical] data.  This high-level
// method converts SAR [optical] intensity information into hue
// [value] which it then stores within STL vector
// *interior_points_ptr.

   void drape_PNG_image_onto_multiple_polygons(
      string input_subdir,string png_filename,string xyzuv_filename,
      vector<quadruple>* interior_points_ptr,bool orthographic_projection)
      {
         outputfunc::write_banner(
            "Draping PNG image information onto multiple polygons");

         pngfunc::open_png_file(input_subdir+png_filename);
         pngfunc::parse_png_file();
         camerafunc::parse_xyzuv_data_file(
            input_subdir+xyzuv_filename,orthographic_projection);
         camerafunc::compute_projection_matrix(orthographic_projection);
         camerafunc::check_projection_matrix(
            input_subdir+xyzuv_filename,orthographic_projection);
         map_SAR_and_optical_intensities_to_hue_and_value(
            interior_points_ptr,orthographic_projection);
         pngfunc::close_png_file();
         camerafunc::delete_allocated_matrices();
      }

// ---------------------------------------------------------------------
// Method map_SAR_and_optical_intensities_to_hue_and_value takes in
// STL vector *interior_points_ptr which contains XYZ geometry
// information in its first fields and point proximity to polygon edge
// information in its second fields.  RGB information from the current
// open PNG file is converted into HSV color coordinates.  If the PNG
// data corresponds to an orthographic SAR image, the hue information
// is stored within the 3rd field of *interior_points_ptr.  If the
// PNG data correspnds to a perspective optical image, its value
// information is stored within the 4th fields of
// *interior_points_ptr.

   void map_SAR_and_optical_intensities_to_hue_and_value(
      vector<quadruple>* interior_points_ptr,bool orthographic_projection)
      {
         for (unsigned int i=0; i<interior_points_ptr->size(); i++)
         {
            threevector XYZ_point((*interior_points_ptr)[i].first);
            bool XYZ_point_near_edge((*interior_points_ptr)[i].second);

            twovector image_point;
            if (orthographic_projection)	// SAR image
            {
               image_point=
                  camerafunc::
                  orthographic_project_world_to_image_coordinates(
                     XYZ_point);
            }
            else	// Optical image
            {
               image_point=
                  camerafunc::project_world_to_image_coordinates(
                     XYZ_point);
            }
            
            double h,s,v;
            h=s=v=0;
            Triple<int,int,int> rgb;
            const double denom_factor=1.0/255.0;
            if (pngfunc::get_RGB_values(image_point,rgb))
            {
               double r=rgb.first*denom_factor;
               double g=rgb.second*denom_factor;
               double b=rgb.third*denom_factor;

               const double min_rgb_strength=0.005;
               if (sqr(r)+sqr(g)+sqr(b) > min_rgb_strength)
               {
                  colorfunc::RGB_to_hsv(r,g,b,h,s,v);
               }
            }

            if (orthographic_projection)
            {
               if (nearly_equal(h+s+v,0) || !XYZ_point_near_edge ||
                   point_in_shadow(XYZ_point))
               {
                  h=-999; // sentinel hue value indicates "black" SAR pixel
               }
               (*interior_points_ptr)[i].third=h;
            }
            else
            {
               (*interior_points_ptr)[i].fourth=v;
            } // orthographic_projection conditional

         } // loop over index i labeling XYZ polygon interior points
      }
*/

// ---------------------------------------------------------------------
// Method write_out_fused_poly_info takes in STL vector
// *interior_points_ptr which contains XYZ point geometry information
// in its first fields, point proximity to polygon edge information in
// its second fields, SAR & optical imagery information in its 3rd and
// 4th fields.  It scans over every XYZ interior point of some 3D
// polygon.  Those interior points which lie far from the polygon's
// edge or which correspond to very low SAR intensity values are
// colored on a grey-scale according the optical imagery information.
// Otherwise, hue is used to convey SAR intensity information, while
// value is used to illustrate optical imagery intensity.  The final
// colored set of XYZ points is written to output file xyzp_filename.

   void write_out_fused_poly_info(
      vector<quadruple> const *interior_points_ptr,string xyzp_filename)
      {
         ofstream binary_outstream;
         binary_outstream.open(xyzp_filename.c_str(),ios::app|ios::binary);

         for (unsigned int i=0; i<interior_points_ptr->size(); i++)
         {
            threevector XYZ_point((*interior_points_ptr)[i].first);
            bool XYZ_point_near_edge((*interior_points_ptr)[i].second);

            double h=(*interior_points_ptr)[i].third;
            double s=1.0;
            double v=(*interior_points_ptr)[i].fourth;

            if (nearly_equal(h,-999) || !XYZ_point_near_edge)
            {
               h=s=0;
            }
            else
            {

// We have intentionally played with the fused hues in order to
// accentuate "hot spots" within the SPASE SAR image.  In
// particular, we want to avoid the appearance of any green hues
// within the final fused image.  So we map all hues less than
// h_threshold below to the "red-to-yellow" interval [0 degs,60 degs],
// and we map all hues greater than h_threshold to the "blue" interval
// [h_threshold,270 degs]:

               const double h_threshold=215.0;
               if (h < h_threshold) 
               {
                  h *= 60./h_threshold;
               }
               else
               {
                  h = h_threshold+(h-h_threshold)*
                     (270.-h_threshold)/(360.-h_threshold);
               }
            }
            
            double r,g,b;
            colorfunc::hsv_to_RGB(h,s,v,r,g,b);

            double rgb_colormap_value=
               colorfunc::rgb_colormap_value(r,g,b);
            xyzpfunc::write_single_xyzp_point(
               binary_outstream,XYZ_point,rgb_colormap_value);
         } // loop over index i labeling XYZ polygon interior points

         binary_outstream.close();  
      }

// ---------------------------------------------------------------------
// Boolean method point_in_shadow returns true if the input XYZ point
// definitely lies inside some shadow volume within STL vector
// *shadow_volume_ptr.

   bool point_in_shadow(const threevector& XYZ_point)
      {
         bool XYZ_point_in_shadow=false;
         if (check_for_shadows)
         {
            bool point_inside,point_outside,point_on;
            const double delta_half_fraction=5E-6;
//            const double delta_half_fraction=0.05;
            for (unsigned int v=0; v<shadow_volume_ptr->size(); v++)
            {
               (*shadow_volume_ptr)[v].point_location(
                  XYZ_point,point_inside,point_outside,point_on,
                  delta_half_fraction);
//            cout << "v = " << v << " in = " << point_inside
//                 << " out = " << point_outside << " on = " 
//                 << point_on << endl;
               if (point_inside) XYZ_point_in_shadow=true;
            } // loop over index v labeling shadow volumes
         } // check_for_shadows conditional
         
         return XYZ_point_in_shadow;
      }


} // draw3Dfuncs namespace





