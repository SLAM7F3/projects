// ==========================================================================
// Program MOSAIC reads in a set of 2D XY-UV tiepoints manually
// selected from two photos shot by a rotating camera atop a tripod.
// It computes the homography which maps the "world plane XY"
// coordinate to the "image plane UV" coordinates.  MOSAIC then
// instantiates a texture rectangle which can hold an enlarged, warped
// version of the "world" image.  It iterates over all pixels within
// the warped image plane in UV coordinates and computes the
// corresponding XY coordinates via the inverse homography.  The RGB
// value from the XY pixel is then transfered to the warped UV pixel.  
// MOSAIC exports a warped version of the "world" image which aligns
// reasonably well with the fixed "image plane" photo.

//				./mosaic

// ==========================================================================
// Last updated on 1/2/12; 1/4/12
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "geometry/homography.h"
#include "astro_geo/latlong2utmfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "math/twovector.h"

using std::cin;
using std::cout;
using std::endl;
using std::ios;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
   const int PRECISION=12;
   cout.precision(PRECISION);

//   string filename="./features/features_13_15.txt";
   string filename="./features/features_18_15.txt";
   filefunc::ReadInfile(filename);
   
   int n_lines=filefunc::text_line.size();

// XY = coordinates in "world" plane (to be warped)
// UV = coordinates in "image" plane (remains fixed)

   vector<twovector> XY,UV;
   for (int i=0; i<n_lines/2; i++)
   {
      vector<string> substring=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);
      int ID=stringfunc::string_to_number(substring[0]);
      double X=stringfunc::string_to_number(substring[1]);
      double Y=stringfunc::string_to_number(substring[2]);
      XY.push_back(twovector(X,Y));
      cout << "ID = " << ID
           << " X = " << XY.back().get(0) 
           << " Y = " << XY.back().get(1) 
           << endl;

      substring.clear();
      substring=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i+n_lines/2]);
      ID=stringfunc::string_to_number(substring[0]);
      double U=stringfunc::string_to_number(substring[1]);
      double V=stringfunc::string_to_number(substring[2]);
      UV.push_back(twovector(U,V));
      cout << "ID = " << ID
           << " U = " << UV.back().get(0) 
           << " V = " << UV.back().get(1) 
           << endl;
   }

   homography H;
   H.parse_homography_inputs(XY,UV);
   H.compute_homography_matrix();
   H.compute_homography_inverse();
   
   double RMS_residual=H.check_homography_matrix(XY,UV);
   cout << "RMS_residual = " << RMS_residual << endl;
   cout << "H = " << H << endl;

   outputfunc::enter_continue_char();

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();

   string photo_subdir="./images/";
//   string photo_basename="dome13.jpg";
   string photo_basename="dome18.jpg";
   string photo_filename=photo_subdir+photo_basename;
   texture_rectangle_ptr->import_photo_from_file(photo_filename);
   int width=texture_rectangle_ptr->getWidth();
   int height=texture_rectangle_ptr->getHeight();
   double aspect_ratio=double(width)/double(height);

   texture_rectangle* warped_texture_rectangle_ptr=new texture_rectangle(
      width,height,1,3,NULL);
   double minX=0;
   double maxX=aspect_ratio;
   double minY=0;
   double maxY=1;

   cout << "minX = " << minX << " maxX = " << maxX
        << " minY = " << minY << " maxY = " << maxY << endl;

// Find bounding box corners of "world plane" within fixed "image
// plane" UV coordinate system:

   double u0,u1,u2,u3;
   double v0,v1,v2,v3;
   H.project_world_plane_to_image_plane(minX,minY,u0,v0);
   H.project_world_plane_to_image_plane(maxX,minY,u1,v1);
   H.project_world_plane_to_image_plane(maxX,maxY,u2,v2);
   H.project_world_plane_to_image_plane(minX,maxY,u3,v3);

   cout << "u0,v0 = " << u0 << "  " << v0 << endl;
   cout << "u1,v1 = " << u1 << "  " << v1 << endl;
   cout << "u2,v2 = " << u2 << "  " << v2 << endl;
   cout << "u3,v3 = " << u3 << "  " << v3 << endl;

   vector<double> ucorner,vcorner;
   ucorner.push_back(u0);
   ucorner.push_back(u1);
   ucorner.push_back(u2);
   ucorner.push_back(u3);

   vcorner.push_back(v0);
   vcorner.push_back(v1);
   vcorner.push_back(v2);
   vcorner.push_back(v3);
   
   double min_warped_U=POSITIVEINFINITY;
   double min_warped_V=POSITIVEINFINITY;
   double max_warped_U=NEGATIVEINFINITY;
   double max_warped_V=NEGATIVEINFINITY;
   for (int c=0; c<4; c++)
   {
      min_warped_U=basic_math::min(min_warped_U,ucorner[c]);
      max_warped_U=basic_math::max(max_warped_U,ucorner[c]);
      min_warped_V=basic_math::min(min_warped_V,vcorner[c]);
      max_warped_V=basic_math::max(max_warped_V,vcorner[c]);
   }

   cout << "min_warped_U,min_warped_V = " << min_warped_U << "  "
        << min_warped_V << endl;
   cout << "max_warped_U,max_warped_V = " << max_warped_U << "  "
        << max_warped_V << endl;

   int warped_width=(max_warped_U-min_warped_U)*width/aspect_ratio;
   int warped_height=(max_warped_V-min_warped_V)*height;

   cout << "warped_width = " << warped_width 
        << " width = " << width << endl;
   cout << "warped height = " << warped_height
        << " height = " << height << endl;

   string blank_filename="blank.jpg";
   warped_texture_rectangle_ptr->generate_blank_image_file(
      warped_width,warped_height,blank_filename,0.5);

   for (int pu=0; pu<warped_width; pu++)
   {
      outputfunc::update_progress_fraction(pu,100,warped_width);
      double warped_u_frac=double(pu)/(warped_width-1);
      double warped_u=min_warped_U+warped_u_frac*(max_warped_U-min_warped_U);
      for (int pv=0; pv<warped_height; pv++)
      {
         double warped_v_frac=1-double(pv)/(warped_height-1);
         double warped_v=min_warped_V+warped_v_frac*(
            max_warped_V-min_warped_V);

         double x,y;
         H.project_image_plane_to_world_plane(warped_u,warped_v,x,y);

         int R,G,B;
         if (x < minX || x > maxX || y < minY || y > maxY)
         {
            R=G=B=0;
         }
         else
         {
//         texture_rectangle_ptr->get_pixel_RGB_values(pu,pv,R,G,B);
//         texture_rectangle_ptr->get_RGB_values(u,v,R,G,B);
            texture_rectangle_ptr->get_RGB_values(x,y,R,G,B);
         }
         
//         cout << "R = " << R << " G = " << G << " B = " << B << endl;
         warped_texture_rectangle_ptr->set_pixel_RGB_values(pu,pv,R,G,B);
      } // loop over pv index
   } // loop over pu index


   string warped_basename="warped_"+photo_basename;
   string warped_filename=photo_subdir+warped_basename;
   warped_texture_rectangle_ptr->write_curr_frame(warped_filename);

}
