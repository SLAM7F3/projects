// ==========================================================================
// Program GPS_WAYPOINTS imports Puma geoposition information exported
// via program PARSE_PUMA_METADA.  It also imports relationships
// between originally named and VSFM renamed image filenames generated
// by program VSFM_VS_ORIG_IMAGES.  GPS_WAYPOINTS exports a GPS
// waypoints to bundler_IO_subdir in a format which can be ingested by
// program GPSFIT.

// ./gps_waypoints --region_filename ./bundler/Puma/May30_2013/Day1_flt2/VSFM_subset/packages/peter_inputs.pkg

// ==========================================================================
// Last updated on 12/3/13; 12/4/13; 12/30/13
// ==========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "math/constant_vectors.h"
#include "general/filefuncs.h"
#include "passes/PassesGroup.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ios;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
   cout.precision(10);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   int videopass_ID=passes_group.get_videopass_ID();
//   cout << "videopass_ID = " << videopass_ID << endl;
   string image_list_filename=passes_group.get_image_list_filename();
//   cout << " image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
//   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string orig_vs_vsfm_filename=bundler_IO_subdir+"orig_vs_VSFM_images.dat";
   string puma_geoposns_filename=bundler_IO_subdir+"puma.geoposns";

// Import relationships between original and VSFM image filenames
// calculated via program VSFM_VS_ORIG_IMAGES and store within STL
// map:
   
   typedef map<string,string> ORIG_VSFM_FILENAMES_MAP;
   ORIG_VSFM_FILENAMES_MAP orig_vsfm_filenames_map;
   ORIG_VSFM_FILENAMES_MAP::iterator iter;

   vector<vector<string> > substrings=filefunc::ReadInSubstrings(
      orig_vs_vsfm_filename);
   for (int s=0; s<substrings.size(); s++)
   {
      string orig_filename=substrings[s].at(0);
      string vsfm_filename=substrings[s].at(1);
      orig_vsfm_filenames_map[orig_filename]=vsfm_filename;
//      cout << s << "  "
//           << orig_filename << "  "      
//           << vsfm_filename << endl;
   } // loop over index s labeling lines in orig_vs_vsfm_filename
   
   vector<vector<string> > column_substrings=filefunc::ReadInSubstrings(
      puma_geoposns_filename);
   
   vector<int> vsfm_IDs;
   vector<string> geoposn_strings;
   for (int i=0; i<column_substrings.size(); i++)
   {
      string orig_filename=column_substrings[i].at(0);

      string separator_chars="-.";
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         orig_filename,separator_chars);
//      cout << "i = " << i 
//           << " orig_filename = " << orig_filename 
//           << " substring =  " << substrings[1] 
//           << endl;
      
      iter=orig_vsfm_filenames_map.find(orig_filename);
      if (iter==orig_vsfm_filenames_map.end()) continue;
      string vsfm_filename=iter->second;
      vsfm_IDs.push_back(stringfunc::string_to_number(
         filefunc::getprefix(vsfm_filename)));

      string geoposn_string;
      for (int c=1; c<=5; c++)
      {
//         cout << "c = " << c << " column_substrings[i].at(c) = "
//              << column_substrings[i].at(c) << endl;
         geoposn_string += "   "+column_substrings[i].at(c);
      }
      geoposn_strings.push_back(geoposn_string);
      
   } // loop over index i labeling lines in puma_geoposns_filename

   templatefunc::Quicksort(vsfm_IDs,geoposn_strings);

   string output_filename=bundler_IO_subdir+"GPS_waypoints.txt";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);
   outstream << "# i  HHMMSS     Epoch secs      Easting      Northing      Altitude (m)" << endl;
   outstream << endl;

   for (int i=0; i<vsfm_IDs.size(); i++)
   {
      outstream << vsfm_IDs[i] << geoposn_strings[i] << endl;
   }
   filefunc::closefile(output_filename,outstream);

   string banner="Exported "+output_filename;
   outputfunc::write_big_banner(banner);
}
