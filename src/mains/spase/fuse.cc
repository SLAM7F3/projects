// ==========================================================================
// Program FUSE converts RGB information from PNG photos and SAR
// images of the SPASE satellite into HSV color space.  Hue
// information within the final fused image is based upon SAR
// intensity; value information is based upon photograph intensities;
// saturation in the fused image is set equal to unity.  XYZ points
// where SAR intensity information is either very weak or unavailable
// appear as various shades of grey in the final fused image.  
// ==========================================================================
// Last updated on 2/4/05; 7/30/06
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "threeDgraphics/draw3Dfuncs.h"
#include "general/filefuncs.h"
#include "geometry/polygon.h"
#include "space/spasefuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "threeDgraphics/xyzpfuncs.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::ios;
   using std::ofstream;
   using std::ostream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// ==========================================================================
// Constant definitions
// ==========================================================================

// Generate satellite model faces in XYZ space:

   vector<polygon>* panel_ptr=spasefunc::construct_SPASE_panel_polys();
   vector<polygon>* cutout_panel_ptr=
      spasefunc::construct_SPASE_cutout_panel_polys(0.035,0.035);
   vector<polygon>* panel_center_ptr=
      spasefunc::construct_SPASE_panel_center_polys(0.035,0.035);
   vector<polygon>* left_panelstrip_ptr=
      spasefunc::construct_SPASE_left_panel_strips(0.035);
   vector<polygon>* left_fatstrip_ptr=
      spasefunc::construct_SPASE_left_fat_strips(0.035);
   vector<polygon>* right_panelstrip_ptr=
      spasefunc::construct_SPASE_right_panel_strips(0.035);
   vector<polygon>* right_fatstrip_ptr=
      spasefunc::construct_SPASE_right_fat_strips(0.035);
   vector<polygon>* bottom_ptr=spasefunc::construct_SPASE_bottom_polys();
   vector<polygon>* triangle_ptr=
      spasefunc::construct_SPASE_bottom_triangles();
   vector<polygon>* face_ptr=spasefunc::construct_SPASE_face_polys();
   vector<polygon> polygon_face;

// Open output XYZP file:

   string input_subdir="./2D_images/";
   string output_subdir="./xyzp_files/";
   filefunc::dircreate(output_subdir);
   string xyzp_filename=output_subdir+"fuse.xyzp";
   filefunc::deletefile(xyzp_filename);

// Rotation matrix needed to bring canonical model into the physical
// orientation at the compact test range where Weber Hoen took his SAR
// data:

   double azimuth=185;	// degrees
//   cout << "Enter azimuth (in degs) of SPASE model:" << endl;
//   cin >> azimuth;
   double elevation=20;	// degrees
   rotation R(spasefunc::wireframe_rotation_corresponding_to_mount_az_el(
      azimuth,elevation));

// Compute shadow volumes for various polygons:

   double height=2.0;	// meters
   draw3Dfunc::shadow_volume_ptr=spasefunc::construct_SPASE_shadow_volumes(
      face_ptr,panel_ptr,height,y_hat);

   string photo_filename[4],xyzuv_filename[4];
   photo_filename[0]="topface.png";
   photo_filename[1]="415179-03D.png";
   photo_filename[2]="415179-08D.png";
   photo_filename[3]="415179-12D.png";
   xyzuv_filename[0]="topface_xyz_uv.txt";
   xyzuv_filename[1]="415179-03D_xyz_uv.txt";
   xyzuv_filename[2]="415179-08D_xyz_uv.txt";
   xyzuv_filename[3]="415179-12D_xyz_uv.txt";

   string SAR_PNGfilename="spase_interpx4_"
      +stringfunc::number_to_string(azimuth)+".png";
   string SAR_xyzuv_filename="xyzuv_"
      +stringfunc::number_to_string(azimuth)+".txt";

// --------------------------------------------------------------------------
// Fuse top face photo #0 and ISAR image #4:

   polygon_face.clear();
   polygon_face.push_back((*face_ptr)[0]);
   spasefunc::fuse_photo_and_SAR_images(
      input_subdir,xyzp_filename,photo_filename[0],xyzuv_filename[0],
      SAR_PNGfilename,SAR_xyzuv_filename,R,
      polygon_face);
   
// --------------------------------------------------------------------------
// Fuse cutout panel 0 from photo #3 with ISAR image #4:

   polygon_face.clear();
   polygon_face.push_back((*cutout_panel_ptr)[0]);
   spasefunc::fuse_photo_and_SAR_images(
      input_subdir,xyzp_filename,photo_filename[3],xyzuv_filename[3],
      SAR_PNGfilename,SAR_xyzuv_filename,R,
      polygon_face);

// --------------------------------------------------------------------------
// Fuse photo #1 and ISAR image #4:

   polygon_face.clear();
   polygon_face.push_back((*cutout_panel_ptr)[1]);
   polygon_face.push_back((*cutout_panel_ptr)[2]);
   polygon_face.push_back((*bottom_ptr)[1]);
   polygon_face.push_back((*triangle_ptr)[0]);
   polygon_face.push_back((*triangle_ptr)[3]);
   spasefunc::fuse_photo_and_SAR_images(
      input_subdir,xyzp_filename,photo_filename[1],xyzuv_filename[1],
      SAR_PNGfilename,SAR_xyzuv_filename,R,
      polygon_face);

/*
// --------------------------------------------------------------------------
// Fuse left strip on panel 3 from photo #2 and ISAR image #4:

   polygon_face.clear();
   polygon_face.push_back((*left_panelstrip_ptr)[3]);
   spasefunc::fuse_photo_and_SAR_images(
      input_subdir,xyzp_filename,photo_filename[2],xyzuv_filename[2],
      SAR_PNGfilename,SAR_xyzuv_filename,R,
      polygon_face);

// --------------------------------------------------------------------------
// Fuse right strip on panel 5 from photo #3 and ISAR image #4:

   polygon_face.clear();
   polygon_face.push_back((*right_panelstrip_ptr)[5]);
   spasefunc::fuse_photo_and_SAR_images(
      input_subdir,xyzp_filename,photo_filename[3],xyzuv_filename[3],
      SAR_PNGfilename,SAR_xyzuv_filename,R,
      polygon_face);
*/

// --------------------------------------------------------------------------
// Drape panel center regions from photo #1:

   bool grayscale_output=true;
   bool orthographic_projection=false;
   double max_distance=POSITIVEINFINITY;
   const double delta_s=0.001;  // meter

   polygon_face.clear();
   polygon_face.push_back((*panel_center_ptr)[1]);
   polygon_face.push_back((*panel_center_ptr)[2]);
   draw3Dfunc::drape_PNG_image_onto_polygons(
      input_subdir,photo_filename[1],xyzuv_filename[1],xyzp_filename,
      polygon_face,max_distance,delta_s,grayscale_output,
      orthographic_projection);

// --------------------------------------------------------------------------
// Drape PNG photo #2:

   polygon_face.clear();
//   polygon_face.push_back((*right_fatstrip_ptr)[3]);
//   polygon_face.push_back((*left_panelstrip_ptr)[3]);
   polygon_face.push_back((*panel_ptr)[3]);
   polygon_face.push_back((*panel_ptr)[4]);
   polygon_face.push_back((*bottom_ptr)[2]);
   polygon_face.push_back((*triangle_ptr)[1]);
   draw3Dfunc::drape_PNG_image_onto_polygons(
      input_subdir,photo_filename[2],xyzuv_filename[2],xyzp_filename,
      polygon_face,max_distance,delta_s,grayscale_output,
      orthographic_projection);

// --------------------------------------------------------------------------
// Drape PNG photo #3:

   polygon_face.clear();
   polygon_face.push_back((*panel_center_ptr)[0]);
//   polygon_face.push_back((*left_fatstrip_ptr)[5]);
//   polygon_face.push_back((*right_panelstrip_ptr)[5]);
   polygon_face.push_back((*panel_ptr)[5]);
   polygon_face.push_back((*bottom_ptr)[3]);
   polygon_face.push_back((*bottom_ptr)[0]);
   polygon_face.push_back((*triangle_ptr)[2]);
   draw3Dfunc::drape_PNG_image_onto_polygons(
      input_subdir,photo_filename[3],xyzuv_filename[3],xyzp_filename,
      polygon_face,max_distance,delta_s,grayscale_output,
      orthographic_projection);

   delete panel_ptr;
   delete cutout_panel_ptr;
   delete panel_center_ptr;
   delete left_panelstrip_ptr;
   delete left_fatstrip_ptr;
   delete right_panelstrip_ptr;
   delete right_fatstrip_ptr;
   delete bottom_ptr;
   delete triangle_ptr;
   delete face_ptr;

// --------------------------------------------------------------------------
// Superpose wireframe onto XYZP output file:

/*
   bool draw_thick_lines=true;
   vector<genvector>* xyzp_pnt_ptr=spasefunc::construct_SPASE_model(
      colorfunc::rgb_colormap_value(colorfunc::grey),
      colorfunc::rgb_colormap_value(colorfunc::purple),
      colorfunc::rgb_colormap_value(colorfunc::green),
      colorfunc::rgb_colormap_value(colorfunc::blue),
      draw_thick_lines);
   bool gzip_output_file=false;
   xyzpfunc::write_xyzp_data(xyzp_filename,xyzp_pnt_ptr,gzip_output_file);
   delete xyzp_pnt_ptr;
*/

// Add fake color map points into final XYZP file:

   draw3Dfunc::append_fake_xyzp_points_for_dataviewer_coloring(
      xyzp_filename,Zero_vector);
   filefunc::gunzip_file_if_gzipped(xyzp_filename);
}
