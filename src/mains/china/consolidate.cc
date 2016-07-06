// ==========================================================================
// Program CONSOLIDATE reads in two text files containing UV feature
// information manually generated via program VIDEO.  It outputs the
// consolidated file containing 4 columns corresponding to X, Y, U and
// V.
// ==========================================================================
// Last updated on 5/29/07
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "geometry/homography.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
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
   const int PRECISION=15;
   cout.precision(PRECISION);

   string image1_features_filename,image2_features_filename;
   cout << "Enter image1 features filename:" << endl;
   cin >> image1_features_filename;
//   image1_features_filename="features_2D_Shanghai_04.txt";

   cout << "Enter image2 features filename:" << endl;
   cin >> image2_features_filename;
//   image2_features_filename="features_2D_Shanghai_05.txt";
   
   filefunc::ReadInfile(image1_features_filename);
   
   vector<twovector> UV;
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      cout << filefunc::text_line[i] << endl;
      
      vector<string> substring=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);
      double U=stringfunc::string_to_number(substring[3]);
      double V=stringfunc::string_to_number(substring[4]);
      UV.push_back(twovector(U,V));
   }

   filefunc::ReadInfile(image2_features_filename);

   vector<twovector> XY;
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<string> substring=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);
      double X=stringfunc::string_to_number(substring[3]);
      double Y=stringfunc::string_to_number(substring[4]);
      XY.push_back(twovector(X,Y));
//      cout << "i = " << i 
//           << " X = " << XY.back().get(0) 
//           << " Y = " << XY.back().get(1)
//           << endl;
   }

/*
   string output_filename="consolidated_features.txt";
   ofstream outstream;
   outstream.precision(PRECISION);
   filefunc::openfile(output_filename,outstream);
   for (int i=0; i<XY.size(); i++)
   {
      outstream << XY.at(i).get(0) << "\t" 
                << XY.at(i).get(1) << "\t"
                << UV.at(i).get(0) << "\t"
                << UV.at(i).get(1) << endl;
   }
   filefunc::closefile(output_filename,outstream);
*/

   homography H;
   H.parse_homography_inputs(UV,XY);
   H.compute_homography_matrix();
   H.check_homography_matrix(UV,XY);

   H.enforce_unit_determinant();
   H.compute_homography_inverse();

   cout << "H = " << H << endl;
   cout << "H.det = " << H.get_H_ptr()->determinant() << endl;
   cout << "Hinv = " << *(H.get_Hinv_ptr()) << endl;

   exit(-1);

   const double min_x=0;
   const double max_x=1.3333333;
   const double min_y=0;
   const double max_y=1;

//   while (true)
   {
//      double x,y;
      double u,v;
/*
      cout << "Enter input x:" << endl;
      cin >> x;
      cout << "Enter input y:" << endl;
      cin >> y;
      H.project_world_plane_to_image_plane(x,y,u,v);
*/

      H.project_world_plane_to_image_plane(min_x,min_y,u,v);
//      cout << "(min_x,min_y) -> " << endl;
//      cout << "Output u = " << u << " v = " << v << endl << endl;
      cout << "UV_corners.push_back(twovector(" << u << "," << v << "));"
           << endl;

      H.project_world_plane_to_image_plane(max_x,min_y,u,v);
//      cout << "(max_x,min_y) -> " << endl;
//      cout << "Output u = " << u << " v = " << v << endl << endl;
      cout << "UV_corners.push_back(twovector(" << u << "," << v << "));"
           << endl;

      H.project_world_plane_to_image_plane(max_x,max_y,u,v);
//      cout << "(max_x,max_y) -> " << endl;
//      cout << "Output u = " << u << " v = " << v << endl << endl;
      cout << "UV_corners.push_back(twovector(" << u << "," << v << "));"
           << endl;

      H.project_world_plane_to_image_plane(min_x,max_y,u,v);
//      cout << "(min_x,max_y) -> " << endl;
//      cout << "Output u = " << u << " v = " << v << endl << endl;
      cout << "UV_corners.push_back(twovector(" << u << "," << v << "));"
           << endl;



   }

/*
RMS residual between measured and calculated UV points = 0.000118818043528294
(min_x,min_y) ->
Output u = -1.09559666090817 v = -0.100627798697513

(max_x,min_y) ->
Output u = 0.61809365980911 v = -0.055050418360053

(max_x,max_y) ->
Output u = 0.705896126564344 v = 0.856527414838622

(min_x,max_y) ->
Output u = -0.815988430941159 v = 1.22164000153095
*/

/*

(min_x,min_y) ->
Output u = -0.885381548036862 v = -0.0156026931332867

(max_x,min_y) ->
Output u = 0.613945164428027 v = -0.0505828945838871

(max_x,max_y) ->
Output u = 0.731665527983946 v = 0.881800370448249

(min_x,max_y) ->
Output u = -0.754495085664019 v = 1.18392267651993


*/
   
}
