// ==========================================================================
// Program CONSOLIDATE reads in two text files which should have the
// form prefix_LL_UTM.txt and prefix_features.txt where prefix is some
// user specified string.  It outputs the consolidated file
// prefix_consolidated.txt containing 4 columns corresponding to X, Y,
// U and V.  The X and Y values come from the 3D point cloud, while
// the U and V represent corresponding UTM easting and northing
// values.
// ==========================================================================
// Last updated on 1/31/07; 4/18/07; 6/8/07
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "astro_geo/geopoint.h"
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

//   string subdir="./textfiles/";
   string subdir="/media/usbdisk/Lowell/";
   string prefix="newyork";
//   cout << "Enter prefix:" << endl;
//   cin >> prefix;
   prefix=subdir+prefix;

//   string google_filename=prefix+"_latlong_features.txt";
//   string google_filename="alirt_longlat_features.txt";
   string google_filename=subdir+"lowell_rtv_longlat_features.txt";
   filefunc::ReadInfile(google_filename);
   
   vector<twovector> UV;
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
//      cout << filefunc::text_line[i] << endl;

      vector<string> substring=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);
      double longitude=stringfunc::string_to_number(substring[1]);
      double latitude=stringfunc::string_to_number(substring[2]);
      geopoint curr_point(longitude,latitude);
      double UTMEasting=curr_point.get_UTM_easting();
      double UTMNorthing=curr_point.get_UTM_northing();

//      bool northern_hemisphere_flag;
//      int UTM_zonenumber;
//      double UTMNorthing,UTMEasting;
//      latlongfunc::LLtoUTM(
//         latitude,longitude,UTM_zonenumber,northern_hemisphere_flag,
//         UTMNorthing,UTMEasting);

      cout << "long = " << longitude << " lat = " << latitude 
           << " east = " << UTMEasting << " north = " << UTMNorthing
           << endl;

//      cout << "UTM_zonenumber = " << UTM_zonenumber
//           << " northern_hemisphere_flag = " 
//           << northern_hemisphere_flag << endl;

      UV.push_back(twovector(UTMEasting,UTMNorthing));
   }

//   string features_filename=prefix+"_XY_features.txt";
//   string features_filename="alirt_xyz_features.txt";
   string features_filename=subdir+"lowell_rtv_xyz_features.txt";
   filefunc::ReadInfile(features_filename);

   vector<twovector> XY;
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<string> substring=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);
      double X=stringfunc::string_to_number(substring[1]);
      double Y=stringfunc::string_to_number(substring[2]);
      XY.push_back(twovector(X,Y));
      cout << "i = " << i 
           << " X = " << XY.back().get(0) 
           << " Y = " << XY.back().get(1)
           << endl;
   }

//   string output_filename=prefix+"_consolidated.txt";
//   string output_filename="alirt_ny_features_consolidated.txt";
//   string output_filename="alirt_features_consolidated.txt";
   string output_filename="lowell_rtv_features_consolidated.txt";

   ofstream outstream;
   outstream.precision(PRECISION);
   filefunc::openfile(output_filename,outstream);
   for (int i=0; i<XY.size(); i++)
   {
      outstream << stringfunc::number_to_string(XY.at(i).get(0),4) << "\t" 
                << stringfunc::number_to_string(XY.at(i).get(1),4) << "\t"
                << stringfunc::number_to_string(UV.at(i).get(0),4) << "\t"
                << stringfunc::number_to_string(UV.at(i).get(1),4) << endl;
   }
   filefunc::closefile(output_filename,outstream);
}
