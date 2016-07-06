// ==========================================================================
// Header file for SPASEFUNCS namespace
// ==========================================================================
// Last modified on 8/4/06; 10/29/08; 1/29/12
// ==========================================================================

#ifndef SPASEFUNCS_H
#define SPASEFUNCS_H

#include <string>
#include <vector>
#include "threeDgraphics/draw3Dfuncs.h"
#include "math/fourvector.h"
#include "math/threevector.h"
#include "geometry/parallelepiped.h"
#include "geometry/polygon.h"
#include "math/threevector.h"
#include "datastructures/Triple.h"

class rotation;

namespace spasefunc
{
   extern const int n_protruding_bolts;

// SPASE polygon construction methods:

   void generate_rotated_poly_counterparts(
      polygon& poly,std::vector<polygon>* poly_ptr);
   std::vector<polygon>* construct_SPASE_panel_polys();
   std::vector<polygon>* construct_SPASE_cutout_panel_polys(
      double delta_x,double delta_z);
   std::vector<polygon>* construct_SPASE_panel_center_polys(
      double delta_x,double delta_z);
   std::vector<polygon>* construct_SPASE_left_panel_strips(double delta_x);
   std::vector<polygon>* construct_SPASE_left_fat_strips(double delta_x);
   std::vector<polygon>* construct_SPASE_right_panel_strips(double delta_x);
   std::vector<polygon>* construct_SPASE_right_fat_strips(double delta_x);
   std::vector<polygon>* construct_SPASE_bottom_polys();
   std::vector<polygon>* construct_SPASE_bottom_triangles();
   std::vector<polygon>* construct_SPASE_face_polys();

// SPASE polygon manipulation methods:

   void wireframe_rotation_corresponding_to_mount_az_el(
      double azimuth,double elevation,rotation& R);
   void translate_SPASE_polygons(
      const threevector& trans,std::vector<polygon>* poly_ptr);
   void translate_SPASE_polygons(
      const threevector& trans,std::vector<polygon>* panel_ptr,
      std::vector<polygon>* bottom_ptr,std::vector<polygon>* triangle_ptr,
      std::vector<polygon>* face_ptr);
   void scale_SPASE_polygons(
      double sfactor,std::vector<polygon>* poly_ptr);
   void scale_SPASE_polygons(
      double sfactor,std::vector<polygon>* panel_ptr,
      std::vector<polygon>* bottom_ptr,std::vector<polygon>* triangle_ptr,
      std::vector<polygon>* face_ptr);

   void rotate_SPASE_polygons(
      const rotation& R,std::vector<polygon>* poly_ptr);
   void rotate_SPASE_polygons(
      const rotation& R,std::vector<polygon>* panel_ptr,
      std::vector<polygon>* bottom_ptr,std::vector<polygon>* triangle_ptr,
      std::vector<polygon>* face_ptr);
   void rotate_SPASE_polygons(
      const rotation& R,std::vector<polygon>* panel_ptr,
      std::vector<polygon>* cutout_panel_ptr,
      std::vector<polygon>* bottom_ptr,std::vector<polygon>* triangle_ptr,
      std::vector<polygon>* face_ptr);

// SPASE model construction methods:

   threevector bolt_location(int edge_number,int bolt_number);
   std::vector<fourvector>* construct_SPASE_model(
      double model_outline_annotation_value=draw3Dfunc::annotation_value1,
      double bolts_annotation_value=draw3Dfunc::annotation_value2,
      double ribs_annotation_value=draw3Dfunc::annotation_value3,
      double numerals_annotation_value=draw3Dfunc::annotation_value4,
      bool draw_thick_lines=true);
   std::vector<parallelepiped>* construct_SPASE_shadow_volumes(
      std::vector<polygon> const *face_ptr,
      std::vector<polygon>* const panel_ptr,
      double height,const threevector& u_hat);

// SPASE polygon drawing methods:

   void draw_SPASE_polygons(
      std::vector<polygon>* poly_ptr,std::string xyzp_filename,
      double annotation_value);
   void draw_SPASE_polygons(
      std::vector<polygon>* panel_ptr,std::vector<polygon>* bottom_ptr,
      std::vector<polygon>* triangle_ptr,std::vector<polygon>* face_ptr,
      std::string xyzp_filename,double annotation_value);
   void draw_model_3D_axes(
      std::string xyzp_filename,std::string xaxis_label,
      std::string yaxis_label,std::string zaxis_label,
      double annotation_value=draw3Dfunc::annotation_value2);
   void draw_axes_endpoints(std::string xyzp_filename);
//   void fuse_photo_and_SAR_images(
//      std::string input_subdir,std::string xyzp_filename,
//      std::string photo_PNGfilename,std::string photo_xyzuv_filename,
//      std::string SAR_PNGfilename,std::string SAR_xyzuv_filename,
//      const mymatrix& R,const std::vector<polygon>& polygon_face,
//      double delta_s=0.001);

// Miscellaneous SPASE model methods:

   Triple<bool,double,threevector> locate_closest_SPASE_polygon(
      const threevector& curr_point,std::vector<polygon>* poly_ptr);
   Triple<bool,double,threevector> locate_closest_SPASE_polygon(
      const threevector& curr_point,
      std::vector<polygon>* panel_ptr,std::vector<polygon>* bottom_ptr,
      std::vector<polygon>* triangle_ptr,std::vector<polygon>* face_ptr);

// Imagecdf parsing methods:

   void parse_imagecdf_textdump(
      std::string imagecdf_text_filename,
      std::vector<std::pair<int,int> >& image_size,
      std::vector<twovector>& meters_per_pixel,
      std::vector<twovector>& center_shift,
      std::vector<twovector>& translation,std::vector<rotation*>& R_ptr);

} // spasefunc namespace

// ==========================================================================
// Inlined methods:
// ==========================================================================

#endif // spasefuncs.h



