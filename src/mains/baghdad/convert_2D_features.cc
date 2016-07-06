// ==========================================================================
// Program CONVERT_2D_FEATURES reads in a "video" feature file
// containing features extracted from a multi-resolution Baghdad (NYC)
// surface texture generated via the OSGDEM program.  It rescales the
// easting & northing values for the feature's positions into U & V
// coordinates.  The renormalized 2D feature values are written to an
// output text file.
// ==========================================================================
// Last updated on 5/2/07; 5/9/07; 8/12/07
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/twovector.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main (int argc, char * argv[])
{
   std::set_new_handler(sysfunc::out_of_memory);

// NYC satellite EO imagery (Aug 2007):

//    const double easting_extent=590968.2 - 582260.4;
   const double northing_extent=4524539.4 - 4504687.2;
   const double easting_origin=582260.4;
   const double northing_origin=4504687.2;

/*
// Baghdad satellite EO imagery:

// Tile R3C2:   

// Lower Left  (  438583.500, 3673718.700) ( 44d20'27.81"E, 33d12'2.14"N)
// Lower Right (  447185.100, 3673718.700) ( 44d26'0.03"E, 33d12'3.78"N)
// Upper Left  (  438583.500, 3682254.300) ( 44d20'25.73"E, 33d16'39.29"N)

//   const double easting_extent=447185.1 - 438583.5;	// meters
//   const double northing_extent=3682254.3 - 3673718.7;	// meters
//   const double easting_origin=438583.5;
//   const double northing_origin=3673718.7;

// rows 2 - 3, columns 1 - 3:

   const double easting_extent=455786.6875 - 429981.90625;
   const double northing_extent=3690856 - 3673718.75;
   const double easting_origin=429981.90625;
   const double northing_origin=3673718.75;
*/

//   string subdir="/data3/video/Iraq/baghdad/";
//   string feature_filename="features_3D_r3c2_l8.txt";
//   feature_filename = subdir+feature_filename;
   
   string feature_filename="f2D.txt";

   if (!filefunc::ReadInfile(feature_filename))
   {
      cout << 
         "Could not read in file containing 2D features written in 3D coords"
           << endl;
      exit(-1);
   }

   string output_filename="f2D_renorm.txt";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);

   cout.precision(12);
   vector<twovector> XY,UV;
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
//      cout << filefunc::text_line[i] << endl;
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      double curr_u=(column_values[3]-easting_origin)/northing_extent;
      double curr_v=(column_values[4]-northing_origin)/northing_extent;

      column_values[3]=curr_u;
      column_values[4]=curr_v;

      for (unsigned int j=0; j<column_values.size(); j++)
      {
//         cout << column_values[j] << "    ";
         outstream << column_values[j] << "    ";
      }
//      cout << endl;
      outstream << endl;
   }
   
   filefunc::closefile(output_filename,outstream);

} 

