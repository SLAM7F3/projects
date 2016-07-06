// ==========================================================================
// Program GENERATE_SHAPEFILE reads in lat-lon points from an input
// text file.  It uses the SHAPELIB in order to create and populate a
// shape file with the points from the input text file.
// ==========================================================================
// Last updated on 7/23/12
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

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
   std::set_new_handler(sysfunc::out_of_memory);

   string shape_filename="points.shp";
   string unix_cmd="shpcreate "+shape_filename+" point";
   sysfunc::unix_command(unix_cmd);
   
   string flightpath_filename="Andrew_flightpath.txt";
   filefunc::ReadInfile(flightpath_filename);
   
   string separator_chars=",";
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> curr_lat_lon=
         stringfunc::string_to_numbers(filefunc::text_line[i],separator_chars);
      unix_cmd="shpadd "+shape_filename+" "+stringfunc::number_to_string(
         curr_lat_lon[0])+" "+stringfunc::number_to_string(curr_lat_lon[1]);
      sysfunc::unix_command(unix_cmd);
//      cout << unix_cmd << endl;
   }
   


}
