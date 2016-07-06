// ==========================================================================
// Program LL2UTM
// ==========================================================================
// Last updated on 3/8/09
// ==========================================================================

#include <iostream>
#include <math.h>
#include <set>
#include <string>
#include <vector>
#include "math/constants.h"
#include "math/constant_vectors.h"
#include "general/filefuncs.h"
#include "astro_geo/geopoint.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/twovector.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
   cout.precision(15);

   double longitude,latitude;
   cout << "Enter longitude:" << endl;
   cin >> longitude;
   cout << "Enter latitude:" << endl;
   cin >> latitude;

   geopoint curr_point(longitude,latitude);
   cout << "geopoint = " << curr_point << endl;
}

