// ==========================================================================
// Little utility program UV_COORDS computes the video-space (U,V)
// coordinates for the corners of the Baghdad satellite EO tiles lying
// within rows 2 - 3 and columns 1 - 3.
// ==========================================================================
// Last updated on 5/10/07
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
   const int PRECISION=12;
   cout.precision(PRECISION);

   vector<double> R,C;
   R.push_back(3673718.7);
   R.push_back(3682254.3);
   R.push_back(3690855.9);

   C.push_back(429981.9);
   C.push_back(438583.5);
   C.push_back(447185.1);
   C.push_back(455786.7);

   double R_extent=R.back()-R.front();
   double C_extent=C.back()-C.front();
   
   for (int i=0; i<C.size(); i++)
   {
      double U=(C[i]-C[0])/R_extent;
      cout << "i = " << i << " U = " << U << endl;
   }
   
   for (int i=0; i<R.size(); i++)
   {
      double V=(R[i]-R[0])/R_extent;
      cout << "i = " << i << " V = " << V << endl;
   }
   
}

/*

UV coordinates for baghdad satellite EO tiles in rows 2 - 3 and
columns 1 - 3:

i = 0 U = 0
i = 1 U = 0.50192563546
i = 2 U = 1.00385127092
i = 3 U = 1.50577690638
i = 0 V = 0
i = 1 V = 0.49807436454
i = 2 V = 1

*/
