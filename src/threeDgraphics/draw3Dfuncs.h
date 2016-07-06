// =========================================================================
// Header file for stand-alone 3D drawing functions.
// =========================================================================
// Last modified on 8/3/06; 6/21/07; 1/29/12
// =========================================================================

#ifndef DRAW3DFUNCS_H
#define DRAW3DFUNCS_H

#include <fstream>
#include <string>
#include <vector>
#include "math/constant_vectors.h"
#include "general/filefuncs.h"
#include "datastructures/Linkedlist.h"
#include "datastructures/Mynode.h"
#include "network/Network.h"
#include "geometry/parallelepiped.h"
#include "datastructures/Quadruple.h"
class character;
class contour;
class linesegment;
class parallelepiped;
class polygon;
class rotation;
class threeDstring;
class threevector;
class track;
template <class T> class TwoDarray;
typedef TwoDarray<double> twoDarray;

namespace draw3Dfunc
{
   extern double xlo,xhi,ylo,yhi,zlo,zhi; // Allowed drawing volume 
   extern bool draw_thick_lines;
   extern double delta_phi,ds;
   extern bool check_for_shadows;
   extern std::vector<parallelepiped>* shadow_volume_ptr;

   typedef Quadruple<threevector,bool,double,double> quadruple;

// Define annotation colors for use with our hue+value colormap within
// the Group 94/106 dataviewer.  Recall the (dumb!) Group 94
// convention of multiplying probability values from 0 to 1 by a
// factor of 10.  So the annotation values below range from 0 to 10
// rather than from 0 to 1....

//   const double annotation_value1=10.0;		// white
//   const double annotation_value2=9.5;		// purple
//   const double annotation_value3=0.2;		// red

   const double annotation_value1=1.0;		// white
   const double annotation_value2=0.95;		// purple
   const double annotation_value3=0.02;		// red
   const double annotation_value4=0.2;		// ?
   const double annotation_value5=0.4;		// ?
   const double annotation_value6=0.6;		// ?
   const double annotation_value7=0.8;		// ?

// ------------------------------------------------------------------------

// Geometrical primitives drawing methods:

   void draw_line(
      const linesegment& l,std::string xyzp_filename,double annotation_value);
   void draw_line(
      const threevector& point_1,const threevector& point_2,
      std::string xyzp_filename,double annotation_value);

   void draw_thin_line(
      const threevector& point_1,const threevector& point_2,
      std::string xyzp_filename,double annotation_value);
   void draw_thick_line(
      const threevector& point_1,const threevector& point_2,
      std::string xyzp_filename,double annotation_value);

   void draw_vector(
      const threevector& v,const threevector& basepoint,
      std::string xyzp_filename,
      double annotation_value,const threevector& n_hat,
      double arrow_length_frac=0.1);
   void draw_vector(
      const threevector& v,const threevector& basepoint,
      std::string xyzp_filename,double annotation_value,
      double arrow_length_frac=0.1);
   void draw_mid_vector(
      const threevector& v,const threevector& basepoint,
      std::string xyzp_filename,double annotation_value);
   void draw_symmetry_directions(
      double z_vector,double symmetry_angle,std::string xyzp_filename,
      double annotation_value);

   void draw_track(
      track* track_ptr,std::string xyzp_filename,double annotation_value);

   void draw_rectangle_grid(
      polygon& rectangle,std::string xyzp_filename,double annotation_value,
      double delta_u=1,double delta_v=1);
   void draw_zplane(
      double z_plane,double xhi,double xlo,double yhi,double ylo,
      std::string xyzp_filename,double annotation_value);
   void draw_polygon(const polygon& poly,std::string xyzp_filename,
                     double annotation_value);
   void draw_contour(const contour& c,std::string xyzp_filename,
                     double annotation_value);
   void draw_circle(double radius,threevector& center_posn,
                    std::string xyzp_filename,double annotation_value);
   void draw_parallelepiped(
      const parallelepiped& p,std::string xyzp_filename,
      double annotation_value);
   void draw_tiny_cube(const threevector& point,std::string xyzp_filename,
                       double annotation_value,double side_length=0.5);
   void draw_contour_cylinder(
      const contour& c,double height,std::string xyzp_filename,
      double annotation_value);
   void draw_contour_cylinder(
      const contour& c,std::string xyzp_filename,
      double annotation_value);
   void color_binary_region(
      std::string xyzp_filename,
      double min_x,double min_y,double max_x,double max_y,
      double z_region,double annotation_value,
      twoDarray const *zbinary_twoDarray_ptr);
   void fill_bbox(
      std::string xyzp_filename,double x0,double y0,double x1,double y1,
      double annotation_value);

   void append_fake_xyzp_points_for_dataviewer_coloring(
      std::string xyzp_filename,const threevector& fake_posn,
      bool gzip_xyzp_file=true);
   void append_fake_xyzp_points_in_twoDarray_middle(
      twoDarray const *ftwoDarray_ptr,std::string xyzp_filename,
      bool gzip_xyzp_file=true);
   void append_fake_z_points_in_twoDarray_middle(
      twoDarray const *ftwoDarray_ptr,std::string xyzp_filename,
      double zlo=-10,double zhi=50);

// Network display methods:

   template <class T> void draw_3D_network_posns(
      std::string xyzp_filename,Network<T*>* network_ptr,
      double annotation_value);
   template <class T> void draw_3D_nearest_neighbor_links(
      std::string xyzp_filename,Network<T*>* network_ptr,
      double annotation_value,double delta_z=0);
   template <class T> void draw_3D_network_centers(
      std::string xyzp_filename,Network<T*>* network_ptr,
      double annotation_value);
   template <class T> void draw_3D_neighbor_links(
      int n,std::string xyzp_filename,Network<T*>* network_ptr,
      double annotation_value,double delta_z=0);
   template <class T> void draw_3D_neighbor_link(
      int n,int i,Network<T*>* network_ptr,std::string xyzp_filename,
      double annotation_value,double delta_z=0);
   template <class T> void draw_3D_voronoi_regions(
      polygon const *data_bbox_ptr,std::string xyzp_filename,
      Network<T*>* network_ptr,double annotation_value);

// Coordinate system annotation methods:

   void draw_character(
      const character& c,std::string xyzp_filename,
      double annotation_value);
   void draw_threeDstring(
      const threeDstring& s,std::string xyzp_filename,
      double annotation_value);
   void draw_coordinate_system(
      polygon& rectangle,std::string xyzp_filename,
      double annotation_value,double delta_u,double delta_v,
      std::string u_axis_label,std::string v_axis_label,
      double tic_label_size=3.0,double axes_label_size=6.0);
   void draw_3D_axes(
      const threevector& origin,double max_extent,double axes_label_size,
      std::string xyzp_filename,double annotation_value,
      std::string X_axis_label="X",std::string Y_axis_label="Y",
      std::string Z_axis_label="Z");

// Colorbar display methods:

   void draw_colorbar(
      int n_color_tiles,double charsize,
      const threevector& trans_global,std::string colorbar_filename,
      double delta_color_value=0.1,bool display_powers_of_two=false);

// 2D/3D imagery fusion methods:

/*
   void drape_PNG_image_onto_point_cloud(
      std::string input_subdir,std::string png_filename,
      std::string xyzuv_filename,std::string input_xyzp_filename,
      std::string output_xyzp_filename,double missing_data_value,
      double min_range=NEGATIVEINFINITY,double max_range=POSITIVEINFINITY,
      double min_height=NEGATIVEINFINITY,double max_height=POSITIVEINFINITY);
   void drape_PNG_image_onto_polygons(
      std::string input_subdir,std::string png_filename,
      std::string xyzuv_filename,std::string xyzp_filename,
      const std::vector<polygon>& polygon,
      const double max_distance=POSITIVEINFINITY,const double delta_s=0.001,
      bool grayscale_output=false,bool orthographic_projection=false);
   void drape_PNG_image_onto_polygons(
      std::string input_subdir,std::string png_filename,
      std::string xyzuv_filename,std::string xyzp_filename,
      const std::vector<polygon>& polygon,
      const std::vector<double>& max_dist_to_poly_edge,
      const double delta_s=0.001,
      bool grayscale_output=false,bool orthographic_projection=false);
   void drape_PNG_image_onto_polygon(
      std::string xyzp_filename,double delta_s,polygon& poly,
      double max_dist_to_poly_edge=POSITIVEINFINITY,
      bool grayscale_output=false,bool orthographic_projection=false);
*/

   std::vector<quadruple>* 
      generate_3D_polygon_interior_points(
         const std::vector<polygon>& polygon_face,const double delta_s);
   std::vector<quadruple>* 
      generate_3D_polygon_interior_points(
         const std::vector<polygon>& polygon_face,
         const std::vector<double>& max_dist_to_poly_edge,
         const double delta_s);
   void rotate_3D_polygon_interior_points(
      const rotation& R,std::vector<quadruple>* interior_points_ptr);

/*
   void drape_PNG_image_onto_multiple_polygons(
      std::string input_subdir,std::string png_filename,
      std::string xyzuv_filename,
      std::vector<quadruple>* interior_points_ptr,
      bool orthographic_projection);
   void map_SAR_and_optical_intensities_to_hue_and_value(
      std::vector<quadruple>* interior_points_ptr,
      bool orthographic_projection);
*/

   void write_out_fused_poly_info(
      std::vector<quadruple> const *interior_points_ptr,
      std::string xyzp_filename);
   bool point_in_shadow(const threevector& XYZ_point);

// ==========================================================================
// Templatized network display methods
// ==========================================================================

// Method draw_3D_network_posns loops over all sites within input
// network *network_ptr and draws their positions onto the binary xyzp
// file specified by input string xyzp_filename.

   template <class T> void draw_3D_network_posns(
      std::string xyzp_filename,Network<T*>* network_ptr,
      double annotation_value)
      {
         const double radius=1.0;	 // meter
   
         filefunc::gunzip_file_if_gzipped(xyzp_filename);
         for (Mynode<int>* currnode_ptr=network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int n=currnode_ptr->get_data();
            draw_circle(radius,network_ptr->get_site_data_ptr(n)->get_posn(),
                        xyzp_filename,annotation_value);
         } // loop over nodes in network entries list
//   threevector origin(Zero_vector);
//   append_fake_xyzp_points_for_dataviewer_coloring(
//      xyzp_filename,origin);
//         filefunc::gzip_file_if_ungizpped(xyzp_filename);
      }

// ---------------------------------------------------------------------
// Method draw_3D_nearest_neighbor_links draws line segment links
// between network sites which are assumed to be adjacent neighbors
// onto an output xyzp file.

   template <class T> void draw_3D_nearest_neighbor_links(
      std::string xyzp_filename,Network<T*>* network_ptr,
      double annotation_value,double delta_z)
      {
         filefunc::gunzip_file_if_gzipped(xyzp_filename);
         for (Mynode<int>* currnode_ptr=network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int n=currnode_ptr->get_data();
            draw_3D_neighbor_links(
               n,xyzp_filename,network_ptr,annotation_value,delta_z);
         } // loop over nodes in network's entries list

//         append_fake_xyzp_points_for_dataviewer_coloring(
//            xyzp_filename,Zero_vector);
//         filefunc::gzip_file_if_ungzipped(xyzp_filename);         
      }

// ---------------------------------------------------------------------
// Member function draw_3D_network_centers draws a small cube at the
// location of each network element's center within the output xyzp
// file.

   template <class T> void draw_3D_network_centers(
      std::string xyzp_filename,Network<T*>* network_ptr,
      double annotation_value)
      {
         filefunc::gunzip_file_if_gzipped(xyzp_filename);
         for (Mynode<int>* currnode_ptr=network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int n=currnode_ptr->get_data();
            draw_tiny_cube(network_ptr->get_site_data_ptr(n)->get_center(),
                           xyzp_filename,annotation_value);
         } // loop over nodes in buildings network entries list
//         filefunc::gzip_file_if_ungzipped(xyzp_filename);
      }

// ---------------------------------------------------------------------
// Method draw_3D_neighbor_links takes in ID label n for some site in
// the input network *network_ptr.  It draws all of the links between
// this site and its nearest neighbors onto an output xyzp file.

   template <class T> void draw_3D_neighbor_links(
      int n,std::string xyzp_filename,Network<T*>* network_ptr,
      double annotation_value,double delta_z)
      {
         filefunc::gunzip_file_if_gzipped(xyzp_filename);
         Site<T*>* curr_site_ptr=network_ptr->get_site_ptr(n);
         if (curr_site_ptr != NULL)
         {
            std::vector<int> neighbors=curr_site_ptr->get_neighbors();
            for (unsigned int i=0; i<neighbors.size(); i++)
            {
               draw_3D_neighbor_link(
                  n,neighbors[i],network_ptr,xyzp_filename,annotation_value,
                  delta_z);
            }
         } // curr_site_ptr != NULL conditional
//         filefunc::gzip_file_if_ungzipped(xyzp_filename);         
      }

// ---------------------------------------------------------------------
// Method draw_3D_neighbor_link takes in ID labels n and i for two
// network sites which are assumed to be adjacent neighbors.  It draws
// a line segment between their center points onto an output xyzp
// file.

   template <class T> void draw_3D_neighbor_link(
      int n,int i,Network<T*>* network_ptr,std::string xyzp_filename,
      double annotation_value,double delta_z)
      {
         Site<T*>* curr_site_ptr=network_ptr->get_site_ptr(n);

         Site<T*>* neighbor_site_ptr=network_ptr->get_site_ptr(i);
         T* curr_data_ptr=curr_site_ptr->get_data();
         T* neighbor_data_ptr=neighbor_site_ptr->get_data();
         
         threevector curr_posn(curr_data_ptr->get_center()+delta_z*z_hat);
         threevector neighbor_posn(
            neighbor_data_ptr->get_center()+delta_z*z_hat);
         
//         std::cout << "RHS neighbor = " << curr_site_ptr->get_RHS_neighbor()
//                   << std::endl;
//         std::cout << "LHS neighbor = " << curr_site_ptr->get_LHS_neighbor()
//                   << std::endl;
         
         if (i==curr_site_ptr->get_RHS_neighbor())
         {
            threevector V(neighbor_posn-curr_posn);
            draw_mid_vector(V,curr_posn,xyzp_filename,annotation_value);
         }
         else if (i==curr_site_ptr->get_LHS_neighbor())
         {
            threevector V(curr_posn-neighbor_posn);
            draw_mid_vector(V,neighbor_posn,xyzp_filename,annotation_value);
         }
         else
         {
            draw_line(curr_posn,neighbor_posn,xyzp_filename,annotation_value);
         }
      }
   
// ---------------------------------------------------------------------
// Method draw_3D_voronoi_regions loops over every site within input
// network *network_ptr.  It assumes that the Voronoi region for each
// site has already been allocated and assigned.  This method draws
// all site Voronoi polygons onto the output xyzp file.

   template <class T> void draw_3D_voronoi_regions(
      polygon const *data_bbox_ptr,std::string xyzp_filename,
      Network<T*>* network_ptr,double annotation_value)
      {
         filefunc::gunzip_file_if_gzipped(xyzp_filename);
         for (Mynode<int>* currnode_ptr=network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int n=currnode_ptr->get_data();
            polygon Voronoi_poly(
               *(network_ptr->get_site_data_ptr(n)->
                 get_voronoi_region_ptr()));
            draw_polygon(Voronoi_poly,xyzp_filename,annotation_value);
//            draw_tiny_cube(
//               Voronoi_poly.get_origin(),xyzp_filename,annotation_value);
            if (data_bbox_ptr != NULL)
            {
               draw_polygon(*data_bbox_ptr,xyzp_filename,annotation_value);
            }
         } // loop over sites in network's entries list

         append_fake_xyzp_points_for_dataviewer_coloring(
            xyzp_filename,Zero_vector);
//         filefunc::gzip_file_if_ungzipped(xyzp_filename);         
      }

}

#endif // draw3Dfuncs.h




