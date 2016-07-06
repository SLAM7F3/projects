// ==========================================================================
// Program PARSE_COLOR_TRIPLES


//                     ./parse_color_triples

// ==========================================================================
// Last updated on 1/27/16
// ==========================================================================

#include <iostream>
#include <Magick++.h>
#include <string>
#include <vector>
#include "color/colorfuncs.h"
#include "math/constants.h"
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "osg/osg3D/tdpfuncs.h"
#include "math/threevector.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::string;
using std::vector;


// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{

   int imax = 500;
   vector<threevector> XYZ;
   vector<double> P;
   
   for(int i = 0; i < imax; i++)
   {
      threevector curr_XYZ(nrfunc::ran1(), nrfunc::ran1(), nrfunc::ran1());

      double X = curr_XYZ.get(0);
      double Y = curr_XYZ.get(1);
      double curr_P;
      if(X > Y)
      {
         curr_P = 1.0;
      }
      else
      {
         curr_P = 0.5;
      }
      XYZ.push_back(curr_XYZ);
      P.push_back(curr_P);

   } // loop over index i 

   string tdp_filename="test.tdp";
   tdpfunc::write_xyzp_data(tdp_filename, &XYZ, &P);
   string banner="Exported "+tdp_filename;
   outputfunc::write_banner(banner);

   string output_filename="test.txt";
   ofstream outstream;
   filefunc::openfile(output_filename, outstream);
   for(unsigned int i = 0; i < XYZ.size(); i++)
   {
      threevector curr_XYZ = XYZ[i];
      double curr_P = P.at(i);
      int label = 1;
      if(curr_P < 0.7)
      {
         label = -1;
      }
      
      outstream << curr_XYZ.get(0) << "  "
                << curr_XYZ.get(1) << "  "
                << curr_XYZ.get(2) << "  "
                << label << endl;
   }
   filefunc::closefile(output_filename, outstream);

   banner="Exported "+output_filename;
   outputfunc::write_banner(banner);
}


