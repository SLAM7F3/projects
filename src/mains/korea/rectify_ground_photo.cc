// ========================================================================
// Program RECTIFY_GROUND_PHOTO is a playground for orthorectifying
// photos based upon Google Earth UTM geocoords for observed street
// lines.  We identify 2D lines in a ground photo with their
// counterparts in an overhead Google Earth view.  

// We deduce a homography for the particular case of a Korean rocket
// internet photo.  Though the derived homography looks valid, the
// orthorectified internet image does not look especially useful.

// ========================================================================
// Last updated on 8/8/13; 8/9/13 
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "video/camerafuncs.h"
#include "general/filefuncs.h"
#include "geometry/geometry_funcs.h"
#include "geometry/homography.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "math/threevector.h"
#include "math/twovector.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ifstream;
   using std::ofstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Import manually-selected 2D features lying along lines within
// ground photo:
   
   string photo_features_filename="features_2D_ground_photo2.txt";
   filefunc::ReadInfile(photo_features_filename);
   int n_lines=filefunc::text_line.size()/2;

// Compute homogeneous coefficients for 2D ground photo lines:

   ofstream outstream;
   outstream.precision(12);
   string photo_lines_filename="lines_2D_ground_photo2.txt";
   filefunc::openfile(photo_lines_filename,outstream);
   outstream << "# LineID   a         b         c" << endl << endl;

   vector<threevector> photo_line_coeffs;
   for (int l=0; l<n_lines; l++)
   {
      int i=2*l;
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      twovector UV1(column_values[3],column_values[4]);
      vector<double> column_values2=stringfunc::string_to_numbers(
         filefunc::text_line[i+1]);
      twovector UV2(column_values2[3],column_values2[4]);
//      cout << "UV1 = " << UV1 << " UV2 = " << UV2 << endl;

      photo_line_coeffs.push_back(
         geometry_func::homogeneous_line_coords(UV1,UV2));
//      cout << "l = " << l << " line coeffs = " 
//           << photo_line_coeffs.back() << endl;

      outstream << l << "   "
                << photo_line_coeffs.back().get(0) << "   "
                << photo_line_coeffs.back().get(1) << "   "
                << photo_line_coeffs.back().get(2) 
                << endl;

   } // loop over index l labeling 2D imageplane lines
   filefunc::closefile(photo_lines_filename,outstream);

// Import 2D features lying within GE image corresponding to ground
// photo lines:

   string GE_features_filename="features_2D_ground_photo2_GE.txt";
   filefunc::ReadInfile(GE_features_filename);
   n_lines=filefunc::text_line.size()/2;

   string GE_lines_filename="lines_2D_ground_photo2_GE.txt";
   filefunc::openfile(GE_lines_filename,outstream);
   outstream << "# LineID   a         b         c" << endl << endl;

// Compute homogeneous coefficients for 2D GE lines:   

   vector<threevector> GE_line_coeffs;
   for (int l=0; l<n_lines; l++)
   {
      int i=2*l;
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      twovector UV1(column_values[3],column_values[4]);
      vector<double> column_values2=stringfunc::string_to_numbers(
         filefunc::text_line[i+1]);
      twovector UV2(column_values2[3],column_values2[4]);
//      cout << "UV1 = " << UV1 << " UV2 = " << UV2 << endl;

      GE_line_coeffs.push_back(
         geometry_func::homogeneous_line_coords(UV1,UV2));
//      cout << "l = " << l << " line coeffs = " 
//           << GE_line_coeffs.back() << endl;

      outstream << l << "   "
                << GE_line_coeffs.back().get(0) << "   "
                << GE_line_coeffs.back().get(1) << "   "
                << GE_line_coeffs.back().get(2) 
                << endl;

   } // loop over index l labeling 2D imageplane lines

   filefunc::closefile(GE_lines_filename,outstream);
   
   homography H;
   H.parse_tieline_inputs(photo_line_coeffs,GE_line_coeffs);
//   H.parse_tieline_inputs(GE_line_coeffs,photo_line_coeffs);
   bool tielines_flag=true;
   H.compute_homography_matrix(tielines_flag);
   
   H.check_tieline_homography_matrix(photo_line_coeffs,GE_line_coeffs);
//   H.check_tieline_homography_matrix(GE_line_coeffs,photo_line_coeffs);

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();

   string photo_filename="ground_photo2.jpg";
   texture_rectangle_ptr->import_photo_from_file(photo_filename);
   int width=texture_rectangle_ptr->getWidth();
   int height=texture_rectangle_ptr->getHeight();

   double Umin=0;
   double Umax=double(width)/double(height);
   double Vmin=0;
   double Vmax=1;

   double x0,x1,x2,x3,y0,y1,y2,y3;
   H.project_world_plane_to_image_plane(Umin,Vmin,x0,y0);
   H.project_world_plane_to_image_plane(Umax,Vmin,x1,y1);
   H.project_world_plane_to_image_plane(Umax,Vmax,x2,y2);
   H.project_world_plane_to_image_plane(Umin,Vmax,x3,y3);

   double Emin=basic_math::min(x0,x1,x2,x3);
   double Emax=basic_math::max(x0,x1,x2,x3);
   double Nmin=basic_math::min(y0,y1,y2,y3);
   double Nmax=basic_math::max(y0,y1,y2,y3);

   cout.precision(12);
   cout << "Emin = " << Emin << " Emax = " << Emax << endl;
   cout << "Nmin = " << Nmin << " Nmax = " << Nmax << endl;

   string rectified_image_filename="rectified_ground_photo2.jpg";
//   bool image_to_world_flag=true;

   bool world_to_image_flag=false;
   int rectified_width=width/2;
   int rectified_height=height/2;
   camerafunc::orthorectify_image(
      texture_rectangle_ptr,
      rectified_width,rectified_height,
      Emin,Emax,Nmin,Nmax,
      H,rectified_image_filename,
      world_to_image_flag);
   


}

