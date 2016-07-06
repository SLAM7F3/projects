// ==========================================================================
// Program CONSOLIDATE reads in two text files assumed to contain 2D
// XY and UV tiepoint pairs.  It outputs a consolidated file
// containing 4 columns corresponding to X, Y, U and V feature
// coordinates.
// ==========================================================================
// Last updated on 1/15/08; 1/29/08; 5/2/12
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "astro_geo/latlong2utmfuncs.h"
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


   string features_dir=
      "/home/cho/programs/c++/svn/projects/src/mains/sift/images/aerial_MIT/GC/";
//      "/home/cho/programs/c++/svn/projects/src/mains/sift/images/aerial_MIT/health_center/";
//      "/home/cho/programs/c++/svn/projects/src/mains/sift/images/aerial_MIT/dome/";


   string features_XY_filename=
      features_dir+"features_2D_GC_FLIR_clip5_-01728.txt";
//      features_dir+"features_2D_MIT_May25_clip07_0189.txt";
//      features_dir+"features_2D_MIT_May26_clip05_0144.txt";
   filefunc::ReadInfile(features_XY_filename);

   vector<twovector> XY;
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<string> substring=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);
      double X=stringfunc::string_to_number(substring[3]);
      double Y=stringfunc::string_to_number(substring[4]);
      XY.push_back(twovector(X,Y));
   }

   
   string features_UV_filename=
      features_dir+"features_2D_GC_FLIR_clip5_-01731.txt";
//      features_dir+"features_2D_MIT_May25_clip09_0043.txt";
//      features_dir+"features_2D_MIT_May26_clip05_0130.txt";
   filefunc::ReadInfile(features_UV_filename);
   
   vector<int> ID;
   vector<twovector> UV;
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<string> substring=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);
      ID.push_back(stringfunc::string_to_number(substring[1]));
      double U=stringfunc::string_to_number(substring[3]);
      double V=stringfunc::string_to_number(substring[4]);
      UV.push_back(twovector(U,V));
   }


   string output_filename=features_dir+"features_consolidated.txt";
   ofstream outstream;
   outstream.precision(PRECISION);
   filefunc::openfile(output_filename,outstream);
   outstream << "#  X      Y      U        V      ID" << endl;
   outstream << endl;

   for (unsigned int i=0; i<XY.size(); i++)
   {
      const int nprecision=4;
      outstream << stringfunc::number_to_string(XY.at(i).get(0),nprecision)
                << "\t" 
                << stringfunc::number_to_string(XY.at(i).get(1),nprecision) 
                << "\t"
                << stringfunc::number_to_string(UV.at(i).get(0),nprecision)
                << "\t"
                << stringfunc::number_to_string(UV.at(i).get(1),nprecision)
                << "\t"
                << stringfunc::number_to_string(ID[i])
                << endl;
   }
   filefunc::closefile(output_filename,outstream);

   string banner="Consolidated XY & UV features written to "+
      output_filename;
   outputfunc::write_big_banner(banner);
   
}
