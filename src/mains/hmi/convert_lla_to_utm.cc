// ========================================================================
// Program CONVERT_LLA_TO_UTM assumes a text file called
// 'wagonwheel_LLA_posn_pointing.dat' sits in the cropped subdir of
// the raw D7 jpeg images directory.  This comma-separated-value file
// should contain pano ID, latitude, longitude, altitude, azimuth,
// elevation and roll values for each D7 wagonwheel.  This program
// converts the latitude and longitude angles into UTM easting and
// northing geocoordinates.  It outputs a new text file
// 'wagonwheel_UTM_posn_pointing.dat' to the same subdirectory as the
// input LLA file.  The new text file can be read into program
// GENERATE_WAGONWHEELS.
// ========================================================================
// Last updated on 7/21/11; 7/25/11
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "astro_geo/geopoint.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

int main(void)
{
   cout.precision(10);
   
   sysfunc::clearscreen();
   cout << endl;

   string images_subdir="./D7/";
   cout << "Enter full path for directory containing initial raw D7 jpeg images:" 
        << endl;
   cin >> images_subdir;
   filefunc::add_trailing_dir_slash(images_subdir);

   string cropped_images_subdir=images_subdir+"cropped/";

   string Xsens_filename=cropped_images_subdir+
      "wagonwheel_LLA_posn_pointing.dat";
//   cout << "Xsens_filename = " << Xsens_filename << endl;

   string output_filename=cropped_images_subdir+
      "wagonwheel_UTM_posn_pointing.dat";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);
   outstream.precision(12);
   outstream << "# Pano ID, Easting, Northing, Altitude, Roll, Pitch, Yaw"
             << endl << endl;

   filefunc::ReadInfile(Xsens_filename);
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      string curr_line=filefunc::text_line[i];
      if (curr_line.size() < 5) continue;

      vector<double> column_values=stringfunc::string_to_numbers(
         curr_line,",");
//      for (int j=0; j<column_values.size(); j++)
//      {
//         cout << column_values[j] << "  ";
//      }
      int pano_ID=column_values[0];
      double curr_lat=column_values[1];
      double curr_lon=column_values[2];
      double curr_alt=column_values[3];
      double curr_az=column_values[4];
      double curr_el=column_values[5];
      double curr_roll=column_values[6];
      
      geopoint curr_point(curr_lon,curr_lat,curr_alt);

      double curr_easting=curr_point.get_UTM_easting();
      double curr_northing=curr_point.get_UTM_northing();

// On 7/25/11, Lucas de la Garza empirically found this relationship
// between the azimuth reported by the orange XSENS box and the yaw
// value which GENERATE_WAGONWHEELS expects as an input:

      double curr_yaw=curr_az-90; 

      curr_yaw=basic_math::phase_to_canonical_interval(curr_yaw,0,360);

/*
      cout << pano_ID << "  "
           << curr_lat << "  "
           << curr_lon << "  "
           << curr_alt << "  "
           << curr_az << "  "
           << curr_el << "  "
           << curr_roll << endl;
      cout << curr_easting  << "  "
           << curr_northing << "  "
           << curr_yaw << endl;
*/

      outstream << pano_ID << " , "
                << stringfunc::number_to_string(curr_easting,3) << " , " 
                << stringfunc::number_to_string(curr_northing,3) << " , " 
                << stringfunc::number_to_string(curr_alt,3) << " , " 
                << stringfunc::number_to_string(curr_roll,3) << " , " 
                << stringfunc::number_to_string(curr_el,3) << " , " 
                << stringfunc::number_to_string(curr_yaw,3) 
                << endl;

      cout << endl;
   }
   
   filefunc::closefile(output_filename,outstream);

   string banner="Wagon wheel positions in UTM coords as well as pointings written out to "+output_filename;
   outputfunc::write_big_banner(banner);

}
