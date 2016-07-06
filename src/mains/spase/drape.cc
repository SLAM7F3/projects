// ==========================================================================
// Program DRAPE constructs polygons for the SPASE model in XYZ world
// space.  It then reads in PNG photos of the SPASE satellite along
// with text files containing manually derived tie point information
// between XYZ world and UV image coordinates.  It uses this tiepoint
// information to calibrate the camera's intrinsic and extrinsic
// parameters.  DRAPE subsequently instantiates grids of XYZ points on
// each polygonal model face and fetches RGB color information from
// the corresponding point within the appropriate PNG image.  P-values
// for the XYZ grid points are adjusted so that they reasonably
// closely reproduce PNG image colors when the final output XYZP file
// is viewed using the RGB color map with the Group 94/106 dataviewer.
// A 3D wireframe model can also be draped onto the final XYZP file.
// ==========================================================================
// Last updated on 8/26/05; 7/30/06
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
// #include "image/camerafuncs.h"
#include "color/colorfuncs.h"
#include "math/constants.h"
#include "threeDgraphics/draw3Dfuncs.h"
#include "general/filefuncs.h"
#include "math/mymatrix.h"
#include "geometry/parallelepiped.h"
#include "geometry/polygon.h"
#include "space/spasefuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "templates/mytemplates.h"
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

// Generate satellite model polygons in XYZ space:

   vector<polygon>* panel_ptr=spasefunc::construct_SPASE_panel_polys();
   vector<polygon>* cutout_panel_ptr=
      spasefunc::construct_SPASE_cutout_panel_polys(0.035,0.035);
   vector<polygon>* left_panelstrip_ptr=
      spasefunc::construct_SPASE_left_panel_strips(0.035);
   vector<polygon>* right_panelstrip_ptr=
      spasefunc::construct_SPASE_right_panel_strips(0.035);

   vector<polygon>* bottom_ptr=spasefunc::construct_SPASE_bottom_polys();
   vector<polygon>* triangle_ptr=
      spasefunc::construct_SPASE_bottom_triangles();
   vector<polygon>* face_ptr=spasefunc::construct_SPASE_face_polys();

// Open output XYZP file:

   string input_subdir="./2D_images/";
   string output_subdir="./xyzp_files/";
   filefunc::dircreate(output_subdir);
   string xyzp_filename=output_subdir+"drape.xyzp";
   filefunc::deletefile(xyzp_filename);

   mymatrix R;
   R.identity();
   vector<polygon> polygon_face;

/*
// Drape visible PNG photo #0:

   polygon_face.push_back((*face_ptr)[0]);
   polygon_face.push_back((*triangle_ptr)[0]);
   draw3Dfunc::drape_PNG_image_onto_polygons(
      input_subdir,"topface.png","topface_xyz_uv.txt",
      xyzp_filename,polygon_face);

// Drape visible PNG photo #1:

//   polygon_face.push_back((*face_ptr)[0]);
//   polygon_face.push_back((*panel_ptr)[0]);
   polygon_face.push_back((*panel_ptr)[1]);
   polygon_face.push_back((*panel_ptr)[2]);
   polygon_face.push_back((*bottom_ptr)[1]);
//   polygon_face.push_back((*triangle_ptr)[0]);
   polygon_face.push_back((*triangle_ptr)[3]);
   draw3Dfunc::drape_PNG_image_onto_polygons(
      input_subdir,"415179-03D.png","415179-03D_xyz_uv.txt",
      xyzp_filename,polygon_face);

// Drape visible PNG photo #2:

   polygon_face.clear();
   polygon_face.push_back((*panel_ptr)[3]);
   polygon_face.push_back((*panel_ptr)[4]);
//   polygon_face.push_back((*panel_ptr)[5]);
   polygon_face.push_back((*bottom_ptr)[2]);
   polygon_face.push_back((*triangle_ptr)[1]);
   draw3Dfunc::drape_PNG_image_onto_polygons(
      input_subdir,"415179-08D.png","415179-08D_xyz_uv.txt",
      xyzp_filename,polygon_face);

// Drape visible PNG photo #3:

   polygon_face.clear();
   polygon_face.push_back((*panel_ptr)[5]);
   polygon_face.push_back((*panel_ptr)[0]);
   polygon_face.push_back((*bottom_ptr)[3]);
   polygon_face.push_back((*bottom_ptr)[0]);
   polygon_face.push_back((*triangle_ptr)[2]);
   draw3Dfunc::drape_PNG_image_onto_polygons(
      input_subdir,"415179-12D.png","415179-12D_xyz_uv.txt",
      xyzp_filename,polygon_face);
*/

// Drape IR photo:

   polygon_face.clear();
   polygon_face.push_back((*face_ptr)[0]);
   polygon_face.push_back((*panel_ptr)[4]);
   polygon_face.push_back((*panel_ptr)[5]);
   polygon_face.push_back((*panel_ptr)[0]);
   polygon_face.push_back((*bottom_ptr)[3]);
   polygon_face.push_back((*triangle_ptr)[1]);
   polygon_face.push_back((*triangle_ptr)[2]);
   draw3Dfunc::drape_PNG_image_onto_polygons(
      input_subdir,"Min20NoLights_-124_to_-27.png",
      "Min20NoLights_xyz_uv.txt",xyzp_filename,polygon_face);

/*
// SAR image draping:

// Rotation matrix needed to bring canonical model into the physical
// orientation at the compact test range where Weber Hoen took his SAR
// data:

   double azimuth=185;	// degrees
   cout << "Enter azimuth (in degs) angle for SPASE model:" << endl;
   cin >> azimuth;
   double elevation=20;	// degrees
   R=mymatrix(spasefunc::wireframe_rotation_corresponding_to_mount_az_el(
      azimuth,elevation));
   spasefunc::rotate_SPASE_polygons(
      R,panel_ptr,cutout_panel_ptr,bottom_ptr,triangle_ptr,face_ptr);
   spasefunc::rotate_SPASE_polygons(R,left_panelstrip_ptr);
   spasefunc::rotate_SPASE_polygons(R,right_panelstrip_ptr);

// Compute shadow volumes for various polygons:

   double height=2.0;	// meters
   draw3Dfunc::check_for_shadows=true;
//   draw3Dfunc::check_for_shadows=false;
   draw3Dfunc::shadow_volume_ptr=spasefunc::construct_SPASE_shadow_volumes(
      face_ptr,panel_ptr,height,y_hat);

//   draw3Dfunc::ds=0.001;	// meter
//   for (int i=0; i<draw3Dfunc::shadow_volume_ptr->size(); i++)
//   {
//      draw3Dfunc::draw_parallelepiped(
//         (*draw3Dfunc::shadow_volume_ptr)[i],xyzp_filename,0.4);
//   }

// Drape PNG photo #4:

   polygon_face.clear();

   polygon_face.push_back((*face_ptr)[0]);
//   polygon_face.push_back((*panel_ptr)[2]);
//   polygon_face.push_back((*panel_ptr)[1]);
//   polygon_face.push_back((*panel_ptr)[0]);
   polygon_face.push_back((*cutout_panel_ptr)[0]);
   polygon_face.push_back((*cutout_panel_ptr)[1]);
   polygon_face.push_back((*cutout_panel_ptr)[2]);

   polygon_face.push_back((*bottom_ptr)[2]);
   polygon_face.push_back((*bottom_ptr)[1]);
   polygon_face.push_back((*bottom_ptr)[0]);
   polygon_face.push_back((*triangle_ptr)[0]);
   polygon_face.push_back((*triangle_ptr)[3]);

   vector<double> max_dist_to_poly_edge;
   max_dist_to_poly_edge.push_back(POSITIVEINFINITY);
//   max_dist_to_poly_edge.push_back(0.035);
//   max_dist_to_poly_edge.push_back(0.035);
//   max_dist_to_poly_edge.push_back(0.035);
   max_dist_to_poly_edge.push_back(POSITIVEINFINITY);
   max_dist_to_poly_edge.push_back(POSITIVEINFINITY);
   max_dist_to_poly_edge.push_back(POSITIVEINFINITY);
   max_dist_to_poly_edge.push_back(POSITIVEINFINITY);
   max_dist_to_poly_edge.push_back(POSITIVEINFINITY);
   max_dist_to_poly_edge.push_back(POSITIVEINFINITY);
   max_dist_to_poly_edge.push_back(POSITIVEINFINITY);
   max_dist_to_poly_edge.push_back(POSITIVEINFINITY);

//   cout << "Enter delta_u:" << endl;
//   cin >> camerafunc::delta_u;
//   cout << "Enter delta_v:" << endl;
//   cin >> camerafunc::delta_v;

   double delta_s=0.001;	// meter
   bool grayscale_output=false;
   bool orthographic_projection=true;
   string PNGfilename="spase_interpx4_"
      +stringfunc::number_to_string(azimuth)+".png";
   string xyzuv_filename="xyzuv_"
      +stringfunc::number_to_string(azimuth)+".txt";
//   string PNGfilename="spase_isar.png";
//   string xyzuv_filename="spase_isar_185az_20el_xyzuv.txt";
   draw3Dfunc::drape_PNG_image_onto_polygons(
      input_subdir,PNGfilename,xyzuv_filename,xyzp_filename,
      polygon_face,max_dist_to_poly_edge,delta_s,
      grayscale_output,orthographic_projection);

// In order to capture all bolts visible for az=185, el=20, we drape
// SAR image onto two small vertical strips located on solar panels 5
// and 3:

   polygon_face.clear();
   polygon_face.push_back((*right_panelstrip_ptr)[5]);
   polygon_face.push_back((*left_panelstrip_ptr)[3]);

   max_dist_to_poly_edge.clear();
   max_dist_to_poly_edge.push_back(POSITIVEINFINITY);
   max_dist_to_poly_edge.push_back(POSITIVEINFINITY);
   draw3Dfunc::check_for_shadows=false;
   draw3Dfunc::drape_PNG_image_onto_polygons(
      input_subdir,PNGfilename,xyzuv_filename,xyzp_filename,
      polygon_face,max_dist_to_poly_edge,delta_s,
      grayscale_output,orthographic_projection);
*/

   delete panel_ptr;
   delete cutout_panel_ptr;
   delete left_panelstrip_ptr;
   delete right_panelstrip_ptr;
   delete bottom_ptr;
   delete triangle_ptr;
   delete face_ptr;

// Superpose wireframe onto XYZP output file:

//   bool draw_thick_lines=true;
   bool draw_thick_lines=false;
   vector<fourvector>* xyzp_pnt_ptr=spasefunc::construct_SPASE_model(
      1.0,
//      colorfunc::rgb_colormap_value(colorfunc::grey),
      colorfunc::rgb_colormap_value(colorfunc::purple),
      colorfunc::rgb_colormap_value(colorfunc::green),
      colorfunc::rgb_colormap_value(colorfunc::blue),
      draw_thick_lines);
   xyzpfunc::rotate(R,xyzp_pnt_ptr);

   bool gzip_output_file=false;
   xyzpfunc::write_xyzp_data(xyzp_filename,xyzp_pnt_ptr,gzip_output_file);
   delete xyzp_pnt_ptr;

// Add fake color map points into final XYZP file:

   draw3Dfunc::append_fake_xyzp_points_for_dataviewer_coloring(
      xyzp_filename,Zero_vector);
   filefunc::gunzip_file_if_gzipped(xyzp_filename);
}
