// ==========================================================================
// Program CONSOLIDATE reads in two text files containing
// easting-northing and UV feature information.  It outputs a
// consolidated feature information file containing 4 columns
// corresponding to X, Y, U and V.  The X and Y values come from the
// Quickbird Lubbock osga files, while the U and V represent
// corresponding UTM easting and northing values.
// ==========================================================================
// Last updated on 6/19/06; 1/31/07; 12/9/07; 2/7/08; 9/14/08
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "general/filefuncs.h"
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

//   string features_filename="features_2D_ch_ar1.txt";
//   string features_filename="features_2D_best_1350.txt";
   string features_filename="features_2D_full.txt";
   cout << "Enter filename containing (U,V) feature coords" << endl;
   cin >> features_filename;
   filefunc::ReadInfile(features_filename);
   
   vector<twovector> UV;
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
//      cout << filefunc::text_line[i] << endl;
      vector<string> substring=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);
      
//      double U=stringfunc::string_to_number(substring[1]);
//      double V=stringfunc::string_to_number(substring[2]);
      double U=stringfunc::string_to_number(substring[3]);
      double V=stringfunc::string_to_number(substring[4]);
      UV.push_back(twovector(U,V));
   }

//   string quickbird_filename="features_quickbird_ar1.txt";
//   string quickbird_filename="features_3D_best_1350.txt";
   string quickbird_filename="features_3D_full.txt";
   cout << "Enter filename containing (easting,northing) feature coords"
        << endl;
   cin >> quickbird_filename;
   filefunc::ReadInfile(quickbird_filename);

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

   string output_filename="features_consolidated.txt";
   cout << "Enter name of output filename to contain consolidated feature coords:" << endl;
   cin >> output_filename;
   ofstream outstream;
   outstream.precision(PRECISION);
   filefunc::openfile(output_filename,outstream);
   for (unsigned int i=0; i<XY.size(); i++)
   {
      outstream 
         << stringfunc::number_to_string(XY.at(i).get(0),5) << "   " 
         << stringfunc::number_to_string(XY.at(i).get(1),5) << "   " 
         << stringfunc::number_to_string(UV.at(i).get(0),5) << "   " 
         << stringfunc::number_to_string(UV.at(i).get(1),5) << endl;
   }
   filefunc::closefile(output_filename,outstream);
}
