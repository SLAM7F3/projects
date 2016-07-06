// ==========================================================================
// Program SEALEVEL reads in a text file containing XYZ values for
// features selected within the waterways surrounding New York City in
// the RTV data set.  It computes the average waterway height value
// which will subsequently be rescaled to zero.

// Average raw sea height for ALIRT-A NYC data: -78.886 meters

// ==========================================================================
// Last updated on 4/12/07; 4/18/07
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "astro_geo/latlong2utmfuncs.h"
#include "math/mathfuncs.h"
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
   const int PRECISION=12;
   cout.precision(PRECISION);

//   string subdir="./textfiles/";
//   string sealevel_filename=subdir+"sealevel_features.txt";
   string sealevel_filename="water_features_alirt.txt";
   filefunc::ReadInfile(sealevel_filename);
   
   vector<double> Z;
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
//      cout << filefunc::text_line[i] << endl;
      
      vector<string> substring=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);
      Z.push_back(stringfunc::string_to_number(substring[5]));
      cout << i << "   Z = " << Z.back() << endl;
   }

   cout << "average Z = " << mathfunc::mean(Z) << endl;

}
