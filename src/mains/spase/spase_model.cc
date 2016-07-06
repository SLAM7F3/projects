// ==========================================================================
// Program SPASE_MODEL constructs a 3D wireframe model for the SPASE
// satellite.  It consists of a hexagonal cylinder sitting atop a
// rectangular parallelepiped.  Approximate locations of bolts along
// the vertical edges of the hexagonal cylinder are included within
// the model.  Numerals annotate the satellite's base sidefaces, and a
// set of QRP image plane coordinate system axes are also included.
// This program generates an XYZP file containing the 3D model output
// information.  It also generates an orthographic projection of the
// 3D model into the radar image plane.
// ==========================================================================
// Last updated on 2/3/05; 3/21/06; 7/30/06; 1/29/12
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "math/constants.h"
#include "threeDgraphics/draw3Dfuncs.h"
#include "general/filefuncs.h"
#include "space/spasefuncs.h"
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
   using std::ostream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// ==========================================================================

// Initialize 3D image parameters:

   bool draw_thick_lines=false;
//   bool draw_thick_lines=true;
//   vector<genvector>* xyzp_pnt_ptr=spasefunc::construct_SPASE_model(
//      colorfunc::rgb_colormap_value(colorfunc::red),
//      colorfunc::rgb_colormap_value(colorfunc::purple),
//      colorfunc::rgb_colormap_value(colorfunc::green),
//      colorfunc::rgb_colormap_value(colorfunc::blue),
//      draw_thick_lines);
   vector<fourvector>* xyzp_pnt_ptr=spasefunc::construct_SPASE_model(
      draw3Dfunc::annotation_value1,
      draw3Dfunc::annotation_value2,1,1,draw_thick_lines);
   
// Rotation matrix needed to bring canonical model into the physical
// orientation at the compact test range where Weber Hoen took his SAR
// data:

   double azimuth,elevation;
   cout << "Enter azimuth angle (in degs) for SPASE model:" << endl;
   cin >> azimuth;
//   azimuth=185;	// degrees

   cout << "Enter elevation angle (in degs) for SPASE model:" << endl;
   cin >> elevation;
//   elevation=20;	// degrees
   rotation R;
   spasefunc::wireframe_rotation_corresponding_to_mount_az_el(
      azimuth,elevation,R);
   xyzpfunc::rotate(R,xyzp_pnt_ptr);

// Write all XYZP files to output_dir subdirectory:

   string output_subdir="./xyzp_files/";
   filefunc::dircreate(output_subdir);
   
   bool gzip_output_file=false;
   string xyzp_filename=output_subdir+"spase_model.xyzp";
   filefunc::deletefile(xyzp_filename);
   xyzpfunc::write_xyzp_data(xyzp_filename,xyzp_pnt_ptr,gzip_output_file);

   string ortho_filename=output_subdir+"ortho_proj.xyzp";
   vector<fourvector>* xy0p_pnt_ptr=xyzpfunc::XY0P_projection(xyzp_pnt_ptr);
   filefunc::deletefile(ortho_filename);
   xyzpfunc::write_xyzp_data(ortho_filename,xy0p_pnt_ptr,gzip_output_file);
   delete xyzp_pnt_ptr;
   delete xy0p_pnt_ptr;

// Add 3D axes to output XYZP files:

   spasefunc::draw_model_3D_axes(
      xyzp_filename,"X","Y","Z",
      colorfunc::rgb_colormap_value(colorfunc::red));
   spasefunc::draw_model_3D_axes(ortho_filename,"Q","R","P");
   spasefunc::draw_axes_endpoints(xyzp_filename);

// Add fake color map points into final XYZP file:

   draw3Dfunc::append_fake_xyzp_points_for_dataviewer_coloring(
      xyzp_filename,Zero_vector);
   filefunc::gunzip_file_if_gzipped(xyzp_filename);
   draw3Dfunc::append_fake_xyzp_points_for_dataviewer_coloring(
      ortho_filename,Zero_vector);
   filefunc::gunzip_file_if_gzipped(ortho_filename);
}
