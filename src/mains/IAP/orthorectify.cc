// ==========================================================================
// Program ORTHORECTIFY reads in consolidated XY and UV feature
// information for photos of the upside-down "Videotrace" paper
// photographed on a table. It computes the homography which maps
// features the paper's corners in UV coordinates onto XY values
// proportional to 8.5"x11".  ORTHORECTIFY then exports a rectified
// view of the Videotrace paper to "rectified_view.jpg".  The paper's
// contents are much easier to read in the rectified image.

//	               	./orthorectify

// ==========================================================================
// Last updated on 1/2/12; 1/4/12; 2/3/13
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "video/camerafuncs.h"
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

   string filename="./features/videotrace_features.txt";
   filefunc::ReadInfile(filename);
   
   vector<twovector> XY,UV;
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<string> substring=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);
      double X=stringfunc::string_to_number(substring[0]);
      double Y=stringfunc::string_to_number(substring[1]);
      double U=stringfunc::string_to_number(substring[2]);
      double V=stringfunc::string_to_number(substring[3]);
      XY.push_back(twovector(X,Y));
      UV.push_back(twovector(U,V));
      cout << "i = " << i 
           << " X = " << XY.back().get(0) 
           << " Y = " << XY.back().get(1) 
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

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();

//   string photo_filename="videotrace.jpg";
   string photo_filename="UpsideDownPaper.jpg";
   texture_rectangle_ptr->import_photo_from_file(photo_filename);
   int width=texture_rectangle_ptr->getWidth();
   int height=texture_rectangle_ptr->getHeight();

   string rectified_image_filename="rectified_view.jpg";
   camerafunc::rectify_image(
      texture_rectangle_ptr,width,height,H,rectified_image_filename);
}
