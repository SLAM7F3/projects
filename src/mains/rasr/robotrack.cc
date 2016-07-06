// ========================================================================
// Program ROBOTRACK reads in robot position information (specified in
// some 3D world coordinate system).  It translates and rescales the
// robot positions so that they reasonably fit within our canonical
// image plane coordinates 0 < U < Umax and 0 < V < 1.
// ========================================================================
// Last updated on 2/9/11
// ========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "math/threevector.h"

using std::cin;
using std::cout;
using std::endl;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main( int argc, char** argv )
{
   string bundler_IO_subdir=
      "/home/cho/programs/c++/svn/projects/src/mains/photosynth/bundler/rasr/";
   string pano_centers_filename=bundler_IO_subdir+"pano_centers.dat";
   
   vector<int> pano_ID;
   vector<twovector> pano_center_posn;

   double min_X=POSITIVEINFINITY;
   double max_X=NEGATIVEINFINITY;
   double min_Y=POSITIVEINFINITY;
   double max_Y=NEGATIVEINFINITY;
   filefunc::ReadInfile(pano_centers_filename);
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      pano_ID.push_back(column_values[0]);

      double curr_x=column_values[1];
      double curr_y=column_values[2];
      
      pano_center_posn.push_back(twovector(curr_x,curr_y));
      min_X=basic_math::min(min_X,curr_x);
      max_X=basic_math::max(max_X,curr_x);
      min_Y=basic_math::min(min_Y,curr_y);
      max_Y=basic_math::max(max_Y,curr_y);
   }
   
// Renormalize robot track to U,V imageplane coordinates:

   for (int i=0; i<pano_ID.size(); i++)
   {
      twovector curr_xy=pano_center_posn[i];
      curr_xy=curr_xy-twovector(min_X,min_Y);
      double x=curr_xy.get(0);
      double y=curr_xy.get(1);
      double u=x/(max_Y-min_Y);
      double v=y/(max_Y-min_Y);
      pano_center_posn[i]=twovector(u,v);
   }

   string polyline_filename="robotrack_polyline_2D.txt";
   ofstream outstream;
   filefunc::openfile(polyline_filename,outstream);

   outstream << "# Time   PolyLine_ID   Passnumber   X Y R G B A" << endl;
   outstream << endl;

   double curr_t=0;
   int polyline_ID=0;
   int pass_number=0;
   
   const double red_color=1;
   const double green_color=1;
   const double blue_color=1;
   const double alpha_color=1;

   for (int i=0; i<pano_ID.size(); i++)
   {
      twovector curr_robo_posn(pano_center_posn[i]);
      outstream << curr_t << "  "
                << polyline_ID << "  "
                << pass_number << "  "
                << curr_robo_posn.get(0) << "  "
                << curr_robo_posn.get(1) << "  "
                << red_color << "  "
                << green_color << "  "
                << blue_color << "  "
                << alpha_color << endl;
   }
   
   filefunc::closefile(polyline_filename,outstream);
}
