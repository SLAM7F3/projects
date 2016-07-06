// ==========================================================================
// Program SLOWPATH reads in an OSG animation path which contains
// camera positions and attitudes as functions of time extracted via
// waypoint selection.  This program multiplies all of the time
// entires within the animation path by some user specified
// multiplicative factor.  We have empirically found that animation
// paths which look reasonable when played back under OSG need to be
// temporally stretched by a factor of 10 prior to generating movies.
// ==========================================================================
// Last updated on 2/27/06; 8/13/06; 9/5/07; 3/4/09
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "color/colorfuncs.h"
#include "general/filefuncs.h"
#include "math/fourvector.h"
#include "plot/metafile.h"
#include "templates/mytemplates.h"
#include "numrec/nr.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/threevector.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
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
   
   string anim_path_filename="tour_interp.path";
   cout << "Enter OSG animation path filename:" << endl;
   cin >> anim_path_filename;

   vector<double> time;
   vector<threevector> XYZ;
   vector<fourvector> Q;

   filefunc::ReadInfile(anim_path_filename);
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> text_line_value=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      time.push_back(text_line_value[0]);
      XYZ.push_back(threevector(text_line_value[1],text_line_value[2],
                                text_line_value[3]));
      Q.push_back(fourvector(text_line_value[4],text_line_value[5],
                             text_line_value[6],text_line_value[7]));
   }

   double time_factor=1;
//   double t0=0;
   cout << "Enter multiplicative time factor:" << endl;
//   cout << "Enter additive time factor:" << endl;
   cin >> time_factor;

//   cout << "Enter time after which to apply additive constant:" << endl;
//   cin >> t0;

   double delta_time=0;
   cout << "Enter additive delta_time offset:" << endl;
   cin >> delta_time;
   
   string outfilename="slow.path";
//   string outfilename="displaced.path";
   ofstream outstream;
   outstream.precision(12);
   filefunc::openfile(outfilename,outstream);
   for (unsigned int n=0; n<time.size(); n++)
   {
      double curr_time=time[n];
//      curr_time *= time_factor;
      curr_time = time_factor*(curr_time-time[0]);
//      if (curr_time >= t0) curr_time += time_factor;
      curr_time += delta_time;

      outstream << curr_time << "  " 
                << XYZ[n].get(0) << "  "
                << XYZ[n].get(1) << "  " 
                << XYZ[n].get(2) << "  "
                << Q[n].get(0) << "  " 
                << Q[n].get(1) << "  " 
                << Q[n].get(2) << "  " 
                << Q[n].get(3) << endl;
   }
   filefunc::closefile(outfilename,outstream);
}
