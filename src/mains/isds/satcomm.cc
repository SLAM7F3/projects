// ==========================================================================
// Program SATCOMM parses text files containing time, latitude,
// longitude and SNR data produced by Mark Smith in Group 63 from his
// satellite communication experiments conducted in Boston and Lowell.
// It produces an output ascii file containing colored PolyLines in
// UTM zone 19T indicating where the HUMVEE went and what signal
// strength it received.  The output file can be directly read in and
// displayed by main progrma VIEWCITIES.
// ==========================================================================
// Last updated on 7/17/06; 7/18/06; 7/20/06; 7/24/06; 11/21/07
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "color/colorfuncs.h"
#include "general/filefuncs.h"
#include "astro_geo/latlong2utmfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/threevector.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
   cout.precision(9);

   vector<string> param;
   filefunc::parameter_input(argc,argv,param);

   string filename=param[0];
//   cout << "Enter satcomm text file name:" << endl;
//   cin >> filename;
   string banner="Processing input file "+filename;
   outputfunc::write_big_banner(banner);

   int track_ID=stringfunc::string_to_integer(param[1]);
   cout << "Track ID = " << track_ID << endl;

   filefunc::ReadInfile(filename);
   
   vector<int> polyline_ID,pass_number;
   vector<double> time,snr;
   vector<threevector> V;
   vector<osg::Vec4> color;

//   const int iskip=1;
   const int iskip=2;
   const double fake_height=20;	// meters
   for (unsigned int i=0; i<filefunc::text_line.size(); i += iskip)
   {
      polyline_ID.push_back(track_ID);
      pass_number.push_back(track_ID);
      vector<string> substring=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);
      time.push_back(stringfunc::string_to_number(substring[0]));
      double latitude=stringfunc::string_to_number(substring[1]);
      double longitude=stringfunc::string_to_number(substring[2]);
      snr.push_back(stringfunc::string_to_number(substring[3]));

//      string UTM_zone="19T";
      bool northern_hemisphere_flag;
      int UTM_zonenumber;
      double curr_north,curr_east;
      latlongfunc::LLtoUTM(
         latitude,longitude,UTM_zonenumber,northern_hemisphere_flag,
         curr_north,curr_east);
      V.push_back(threevector(curr_east,curr_north,fake_height));

      bool greyscale_flag=true;
      const double min_value=-30;	// dBsm
      const double max_value=-10;	// dBsm
      colorfunc::RGB curr_rgb=colorfunc::scalar_value_to_RGB(
         min_value,max_value,snr.back(),greyscale_flag);
      color.push_back(colorfunc::get_OSG_color(curr_rgb));
   }

   string output_name=stringfunc::prefix(filename)+".fake_heights";
   ofstream outstream;
   outstream.precision(10);
   filefunc::openfile(output_name,outstream);

   for (unsigned int j=0; j<time.size()-1; j++)
   {
      outstream << time[j] << " "
                << polyline_ID[j] << " "
                << pass_number[j] << " "
                << V[j].get(0) << " " 
                << V[j].get(1) << " " 
                << V[j].get(2) << " " 
                << color[j].r() << " "
                << color[j].g() << " "
                << color[j].b() << " "
                << endl;
   }
   filefunc::closefile(output_name,outstream);
   
}
