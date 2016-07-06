// ==========================================================================
// Program LH_METADATA reads in Light Hawk aircraft metadata and
// reformats it so that it matches sailplane metadata read in by
// GPSREGISTER.  
// ==========================================================================
// Last updated on 1/21/11; 1/25/11
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "osg/osgGraphicals/AnimationController.h"
#include "video/camera.h"
#include "general/filefuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ios;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
   string subdir="/home/cho/programs/c++/svn/projects/src/mains/photosynth/bundler/lighthawk/";
   string lighthawk_metadata_filename=subdir+"lighthawk.metadata";

   vector<int> frame_number;
   vector<double> unix_time,latitude,longitude,altitude;
   filefunc::ReadInfile(lighthawk_metadata_filename);
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=
         stringfunc::string_to_numbers(filefunc::text_line[i],",");
      frame_number.push_back(column_values[0]);
      unix_time.push_back(column_values[1]);
      latitude.push_back(column_values[2]);
      longitude.push_back(column_values[3]);
      altitude.push_back(column_values[4]);
   } // loop over index i labeling text lines

   const double meters_per_ft=0.3048;
   string output_filename=subdir+"aircraft_gps.dat";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);
   outstream << "# image name, secs since midnight Jan 1, 1970, longitude, latitude," << endl;
   outstream << "# altitude (ft above sea level)" << endl << endl;

   for (unsigned int i=0; i<frame_number.size(); i++)
   {
      string image_filename="frame"+stringfunc::integer_to_string(
         frame_number[i],6)+".rd.jpg";
      string curr_line=image_filename+"  ";
      curr_line += stringfunc::number_to_string(unix_time[i])+"  ";
      curr_line += stringfunc::number_to_string(longitude[i],8)+"  ";
      curr_line += stringfunc::number_to_string(latitude[i],8)+"  ";
      curr_line += stringfunc::number_to_string(altitude[i]/meters_per_ft);
      outstream << curr_line << endl;
   }
   
   filefunc::closefile(output_filename,outstream);


}
