// ==========================================================================
// Program EXTEND_GPS_WAYPOINTS 
// ==========================================================================
// Last updated on 12/6/13
// ==========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "astro_geo/Clock.h"
#include "math/constant_vectors.h"
#include "general/filefuncs.h"
#include "passes/PassesGroup.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "templates/mytemplates.h"
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
   string gps_waypoints_filename=bundler_IO_subdir+"GPS_waypoints.txt";

// First import rows from puma_geoposns_filename as function of epoch
// times.  Store within STL map:

   typedef map<int,string> EPOCH_GEOPOSN_MAP;
   EPOCH_GEOPOSN_MAP epoch_geoposn_map;
   EPOCH_GEOPOSN_MAP::iterator iter;
   
   vector<vector<string> > column_substrings=filefunc::ReadInSubstrings(
      puma_geoposns_filename);
   
   vector<string> geoposn_strings;
   for (int i=0; i<column_substrings.size(); i++)
   {
      int epoch=stringfunc::string_to_number(column_substrings[i].at(2));

      string geoposn_string;
      for (int c=1; c<=5; c++)
      {
         geoposn_string += "   "+column_substrings[i].at(c);
      }
      epoch_geoposn_map[epoch]=geoposn_string;
   }
   
// Next import contents of gps_waypoints_filename:

   vector<double> VSFM_index,epoch;
   vector<vector<double> > RowNumbers=
      filefunc::ReadInRowNumbers(gps_waypoints_filename);
   for (int r=0; r<RowNumbers.size(); r++)
   {
      VSFM_index.push_back(RowNumbers[r].at(0));
      epoch.push_back(RowNumbers[r].at(2));
//      cout << "r = " << r << " index = " << VSFM_index.back()
//           << " epoch = " << epoch.back() << endl;
   } // loop over index r labeling rows in GPS_waypoints.txt
   
   int min_vsfm_index=mathfunc::minimal_value(VSFM_index);
   int max_vsfm_index=mathfunc::maximal_value(VSFM_index);
   cout << "min_vsfm_index = " << min_vsfm_index << endl;
   cout << "max_vsfm_index = " << max_vsfm_index << endl;

   int min_epoch=mathfunc::minimal_value(epoch);
   int max_epoch=mathfunc::maximal_value(epoch);
   cout << "min epoch = " << min_epoch << endl;
   cout << "max epoch = " << max_epoch << endl;

   string output_filename=bundler_IO_subdir+"extended_GPS_waypoints.txt";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);
   outstream << "# i  HHMMSS     Epoch secs      Easting      Northing      Altitude (m)" << endl << endl;
   
   Clock clock;
   int UTM_zone=19;
   clock.compute_UTM_zone_time_offset(UTM_zone);

   int n_extra_entries=10;
   for (int n=0; n<n_extra_entries; n++)
   {
      int curr_vsfm_index=min_vsfm_index-n_extra_entries+n;
      int curr_epoch=min_epoch-n_extra_entries+n;

      iter=epoch_geoposn_map.find(curr_epoch);
      if (iter==epoch_geoposn_map.end()) continue;
      string geoposn_string=iter->second;

      outstream << curr_vsfm_index << "  "
                << geoposn_string 
                << endl;
   } // loop over index n labeling extra entries

   for (int r=0; r<VSFM_index.size(); r++)
   {
      int curr_vsfm_index=VSFM_index[r];
      int curr_epoch=epoch[r];
      iter=epoch_geoposn_map.find(curr_epoch);
      if (iter==epoch_geoposn_map.end()) continue;
      string geoposn_string=iter->second;

      outstream << curr_vsfm_index << "  "
                << geoposn_string 
                << endl;
   }
   
   for (int n=1; n<=n_extra_entries; n++)
   {
      int curr_vsfm_index=max_vsfm_index+n;
      int curr_epoch=max_epoch+n;

      iter=epoch_geoposn_map.find(curr_epoch);
      if (iter==epoch_geoposn_map.end()) continue;
      string geoposn_string=iter->second;

      outstream << curr_vsfm_index << "  "
                << geoposn_string 
                << endl;

   } // loop over index n labeling extra entries

   filefunc::closefile(output_filename,outstream);
   
   string banner="Exported "+output_filename;
   outputfunc::write_big_banner(banner);
}
