// =========================================================================
// Header file for stand-alone drawing functions.
// =========================================================================
// Last modified on 8/5/06; 11/10/10; 11/20/11
// =========================================================================

#ifndef DRAWFUNCS_H
#define DRAWFUNCS_H

#include "color/colorfuncs.h"
#include "math/threevector.h"

class contour;
class linesegment;
class polygon;
class parallelogram;
class rectangle;
class parallelepiped;
class frustum;
template <class T> class TwoDarray;
typedef TwoDarray<double> twoDarray;

namespace drawfunc
{

// Discrete geometrical object drawing methods (e.g. lines, points,
// polygons):

   void draw_line(
      const linesegment& l,colorfunc::Color color,twoDarray* ztwoDarray_ptr);
   void draw_line(
      const linesegment& l,double intensity,twoDarray* ztwoDarray_ptr,
      bool double_line_thickness=false,bool triple_line_thickness=false);
   void draw_thick_line(
      const linesegment& l,colorfunc::Color color,
      double point_radius,twoDarray* ztwoDarray_ptr);
   void draw_thick_line(
      const linesegment& l,double intensity,
      double point_radius,twoDarray* ztwoDarray_ptr);
//   void draw_dashedline(const linesegment& l,double value,
//                        twoDarray* ztwoDarray_ptr);
   void draw_vector(threevector& v,const threevector& basepoint,
                    colorfunc::Color color,twoDarray* ztwoDarray_ptr);
   void draw_vector(const linesegment& l,colorfunc::Color color,
                    twoDarray* ztwoDarray_ptr);
   void draw_axes(colorfunc::Color color,twoDarray* ztwoDarray_ptr,
                  const threevector& origin,double theta=0);
   void draw_hugepoint(
      const threevector& v,double radius,double intensity_value,
      twoDarray* ztwoDarray_ptr);
   void draw_hugepoint(
      const threevector& v,double radius,colorfunc::Color color,
      twoDarray* ztwoDarray_ptr);
   void draw_hugepoint(
      const threevector& v,double radius,colorfunc::Color foreground_color,
      colorfunc::Color background_color,twoDarray* ztwoDarray_ptr);
   void draw_hugepoint(
      const threevector& v,double a,double b,colorfunc::Color foreground_color,
      colorfunc::Color background_color,twoDarray* ztwoDarray_ptr);
   void draw_polygon(const polygon& p,colorfunc::Color color,
                     twoDarray* ztwoDarray_ptr);
   void draw_polygon(const polygon& p,double intensity,
                     twoDarray* ztwoDarray_ptr);
   void draw_thick_polygon(
      const polygon& p,colorfunc::Color color,double point_radius,
      twoDarray* ztwoDarray_ptr);
   void draw_thick_polygon(
      const polygon& p,double intensity,double point_radius,
      twoDarray* ztwoDarray_ptr);
//   void draw_polygon_edges(const polygon& p,double intensity,
//                           twoDarray* ztwoDarray_ptr);
   void draw_contour(
      const contour& c,double edge_intensity_value,
      double vertex_intensity_value,double node_radius,
      twoDarray* ztwoDarray_ptr,bool display_edges=false,
      bool display_vertices=true);
   void draw_thick_contour(
      const contour& c,double edge_intensity_value,double point_radius,
      twoDarray* ztwoDarray_ptr);
   void draw_bbox(
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      colorfunc::Color color,twoDarray* ztwoDarray_ptr);
   void draw_parallelepiped(
      const parallelepiped& p,colorfunc::Color color,
      colorfunc::Color hidden_color,twoDarray* ztwoDarray_ptr);
   void draw_parallelepiped(
      const parallelepiped& p,colorfunc::Color color,
      twoDarray* ztwoDarray_ptr,double xscale_factor=1.0);
   void draw_frustum(frustum& f,colorfunc::Color color,
                     twoDarray* ztwoDarray_ptr);

// Continuous region drawing methods (e.g. shading polygons, coloring
// polygon interiors):

   void color_triangle_interior(
      const polygon& t,double value,twoDarray* ztwoDarray_ptr,
      bool accumulate_flag=false);
   void color_convex_quadrilateral_interior(
      const polygon& q,double value,twoDarray* ztwoDarray_ptr,
      bool accmulate_flag=false);
   void color_regular_hexagon_interior(
      const polygon& hexagon,double intensity_value,
      twoDarray* ztwoDarray_ptr,bool accumulate_flag=false);
   void color_triangle_exterior(
      const polygon& t,double value,twoDarray* ztwoDarray_ptr);
   void color_convex_quadrilateral_exterior(
      const polygon& q,double value,twoDarray* ztwoDarray_ptr);
   void fill_halfplane(const threevector& basepoint,const threevector& nhat,
                       double value,twoDarray* ztwoDarray_ptr);

   void color_parallelogram_interior(
      const parallelogram& p,double value,twoDarray* ztwoDarray_ptr);
   void color_polygon_interior(
      const polygon& p,colorfunc::Color color,twoDarray* ztwoDarray_ptr);
   void color_polygon_interior(
      const polygon& p,double intensity_value,twoDarray* ztwoDarray_ptr);
   void color_polygon_interior(
      const polygon& p,double foreground_intensity,
      double background_intensity,twoDarray* ztwoDarray_ptr);

   void color_contour_interior(
      const contour& c,double intensity_value,twoDarray* ftwoDarray_ptr);
   void color_contour_interior_shell(
      double curr_scale,const contour& c,double intensity_value,
      twoDarray* ztwoDarray_ptr);
   
   void shade_polygon(polygon& p,double max_value,double min_value,
                      const threevector& uhat,twoDarray* ztwoDarray_ptr);
   void color_bbox_interior(
      const polygon& bbox,double value,twoDarray* ztwoDarray_ptr);
   void color_bbox_interior(
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      double value,twoDarray* ztwoDarray_ptr);
   void color_bbox_interior(
      unsigned int min_px,unsigned int min_py,
      unsigned int max_px,unsigned int max_py,
      double value,twoDarray* ztwoDarray_ptr);
   void color_bbox_exterior(
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      double value,twoDarray* ztwoDarray_ptr);
   void color_bbox_exterior(
      const polygon& bbox,double value,twoDarray* ztwoDarray_ptr);

   void color_polygon_exterior(
      const polygon& p,double value,twoDarray* ztwoDarray_ptr);

   void null_pixels_outside_convex_polygon(
      double null_value,polygon* poly_ptr,twoDarray* ztwoDarray_ptr);
   void null_pixels_outside_convex_polygon(
      double null_value,polygon* poly_ptr,twoDarray* ztwoDarray1_ptr,
      twoDarray* ztwoDarray2_ptr);
   void null_pixels_outside_parallelogram(
      double null_value,parallelogram* parallelogram_ptr,
      twoDarray* ztwoDarray_ptr);
   void null_pixels_outside_parallelogram(
      double null_value,parallelogram* parallelogram_ptr,
      twoDarray* ztwoDarray1_ptr,twoDarray* ztwoDarray2_ptr);

   int count_and_color_lit_pixels_inside_bbox(
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      double zmin,double z_lit,double z_unlit,twoDarray* ztwoDarray_ptr,
      int& npixels_inside_bbox);

   void restore_pixels_inside_bbox(
      twoDarray* ztwoDarray_ptr,twoDarray const *raw_ztwoDarray_ptr,
      unsigned int pxlo,unsigned int pxhi,
      unsigned int pylo,unsigned int pyhi);

   void shade_parallelepiped(
      parallelepiped& p,double max_value,double min_value,
      const threevector& uhat,twoDarray* ztwoDarray_ptr);
   void shade_frustum(frustum& f,double max_value,double min_value,
                      const threevector& uhat,twoDarray* ztwoDarray_ptr);
   void shade_frustum_sides(
      frustum& f,double max_value,double min_value,
      const threevector& uhat,twoDarray* ztwoDarray_ptr);
   void extremal_frustum_points(
      frustum& f,unsigned int& min_px,unsigned int& min_py,
      unsigned int& max_px,unsigned int& max_py,
      twoDarray const *ztwoDarray_ptr);
   void color_frustum_interior(
      frustum& f,double value,twoDarray* ztwoDarray_ptr);
   threevector find_nearest_pixel_with_specified_value(
      const threevector& r_vec,const linesegment& l,
      const threevector& eperp_hat,
      double max_search_dist,int pixel_val,double& dist_to_nonzero_pixel,
      twoDarray const *zbinary_twoDarray_ptr);
   void project_rectangle_interior(
      const rectangle& r,double dw,double dl,
      double value,twoDarray* ztwoDarray_ptr);
   void draw_badimage_slash(
      twoDarray* ztwoDarray_ptr,colorfunc::Color slashcolor=colorfunc::red);
}

#endif // drawfuncs.h




