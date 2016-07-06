// ==========================================================================
// Program PARSE_PUMA imports the metadata text file generated for a
// PUMA UAV flight.  It exports a simpler metadata file containing UAV
// and ground target geolocation as functions of GPS time.

// ==========================================================================
// Last updated on 5/13/12
// ==========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "astro_geo/Clock.h"
#include "general/filefuncs.h"
#include "astro_geo/geopoint.h"
#include "filter/piecewise_linear.h"
#include "general/stringfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

int main (int argc, char * argv[])
{
   string puma_subdir="/data_third_disk/Puma/April24_2013/";
   string input_filename=puma_subdir+"day1_flt1.txt";
   filefunc::ReadInfile(input_filename);
   cout << "filefunc::text_line.size()-1 = " 
        << filefunc::text_line.size()-1 << endl;
   
   Clock clock;

   typedef map<double,vector<vector<double> >* > PUMA_MAP;
   PUMA_MAP puma_map;
   PUMA_MAP::iterator iter;

   int i_start=1;
   int i_stop=filefunc::text_line.size()-1;
   for (int i=i_start; i<=i_stop; i++)
   {
      if (i%100==0) cout << i << " " << flush;
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);

      for (int s=0; s<substrings.size(); s++)
      {
//         cout << "s+1 = " << s+1 << " substring = " << substrings[s] << endl;
      }
      
      int month=stringfunc::string_to_number(substrings[50]);
      int day=stringfunc::string_to_number(substrings[51]);
      int year=stringfunc::string_to_number(substrings[52]);
      int hour=stringfunc::string_to_number(substrings[53]);
      int minute=stringfunc::string_to_number(substrings[54]);
      double secs=stringfunc::string_to_number(substrings[55]);

//      cout << "year = " << year << " month = " << month 
//           << " day = " << day << " hour = " << hour
//           << " minute = " << minute << " secs = " << secs << endl;

      int UTM_zone=19;	// MA
      clock.compute_UTM_zone_time_offset(UTM_zone);
      clock.set_UTC_time(year,month,day,hour,minute,secs);
      double elapsed_secs=clock.secs_elapsed_since_reference_date();
      if (elapsed_secs < 0) continue;

      double UAV_altitude=stringfunc::string_to_number(substrings[9]);
      double UAV_latitude=stringfunc::string_to_number(substrings[56]);
      double UAV_longitude=stringfunc::string_to_number(substrings[57]);

//      cout << "UAV_lon = " << UAV_longitude 
//           << " UAV_lat = " << UAV_latitude
//           << " UAV_alt = " << UAV_altitude << endl;
      geopoint UAV_geopt(UAV_longitude,UAV_latitude,UAV_altitude);
//      cout << "UAV geolocation = " << UAV_geopt << endl;

      double ground_tgt_latitude=stringfunc::string_to_number(
         substrings[125]);
      double ground_tgt_longitude=stringfunc::string_to_number(
         substrings[126]);

// FAKE FAKE:  Tues May 14, 2013 at 1:09 pm
// Hardwire reasonable ground target altitude as surrogate for DTED
// measurement:

      double ground_tgt_altitude=51;	// meters 
//      double ground_tgt_altitude=stringfunc::string_to_number(
//         substrings[80]);

//      cout << "ground tgt: lon = " << ground_tgt_longitude
//           << " lat = " << ground_tgt_latitude << endl;
      geopoint ground_tgt_geopt(
         ground_tgt_longitude,ground_tgt_latitude,ground_tgt_altitude);
//      cout << "ground target geolocation = " << ground_tgt_geopt << endl;
//      cout << "===============================================" << endl;

      iter=puma_map.find(elapsed_secs);
      if (iter==puma_map.end())
      {
         vector<double> UAV_easting,UAV_northing,UAV_alt;
         vector<double> tgt_easting,tgt_northing,tgt_alt;
         
         UAV_easting.push_back(UAV_geopt.get_UTM_easting());
         UAV_northing.push_back(UAV_geopt.get_UTM_northing());
         UAV_alt.push_back(UAV_geopt.get_altitude());
         tgt_easting.push_back(ground_tgt_geopt.get_UTM_easting());
         tgt_northing.push_back(ground_tgt_geopt.get_UTM_northing());
         tgt_alt.push_back(ground_tgt_geopt.get_altitude());
         vector<vector<double> >* puma_metadata_ptr=
            new vector<vector<double> >;
         puma_metadata_ptr->push_back(UAV_easting);
         puma_metadata_ptr->push_back(UAV_northing);
         puma_metadata_ptr->push_back(UAV_alt);
         puma_metadata_ptr->push_back(tgt_easting);
         puma_metadata_ptr->push_back(tgt_northing);
         puma_metadata_ptr->push_back(tgt_alt);
         (puma_map)[elapsed_secs]=puma_metadata_ptr;
      }
      else
      {
         vector<vector<double> >* puma_metadata_ptr=iter->second;
         puma_metadata_ptr->at(0).push_back(UAV_geopt.get_UTM_easting());
         puma_metadata_ptr->at(1).push_back(UAV_geopt.get_UTM_northing());
         puma_metadata_ptr->at(2).push_back(UAV_geopt.get_altitude());
         puma_metadata_ptr->at(3).push_back(
            ground_tgt_geopt.get_UTM_easting());
         puma_metadata_ptr->at(4).push_back(
            ground_tgt_geopt.get_UTM_northing());
         puma_metadata_ptr->at(5).push_back(
            ground_tgt_geopt.get_altitude());
      }
   } // loop over index i labeling input file text line
   cout << endl;

   vector<double> elapsed_secs,UAV_easting,UAV_northing,UAV_altitude;
   vector<double> tgt_easting,tgt_northing,tgt_altitude;
   for (iter=puma_map.begin(); iter != puma_map.end(); iter++)
   {
      elapsed_secs.push_back(iter->first);
      vector<vector<double> >* puma_metadata_ptr=iter->second;
      UAV_easting.push_back(mathfunc::mean(puma_metadata_ptr->at(0)));
      UAV_northing.push_back(mathfunc::mean(puma_metadata_ptr->at(1)));
      UAV_altitude.push_back(mathfunc::mean(puma_metadata_ptr->at(2)));
      tgt_easting.push_back(mathfunc::mean(puma_metadata_ptr->at(3)));
      tgt_northing.push_back(mathfunc::mean(puma_metadata_ptr->at(4)));
      tgt_altitude.push_back(mathfunc::mean(puma_metadata_ptr->at(5)));
   } // loop over puma_map iterator

   string output_filename="puma.metadata";
   ofstream outstream;
   outstream.precision(10);
   filefunc::openfile(output_filename,outstream);
   outstream << "# image_filename   epoch (secs) UAV_easting UAV_northing UAV_alt  tgt_easting  tgt_northing  tgt_alt" << endl;
   outstream << endl;

   int takeoff_frame_ID=375;
   int touchdown_frame_ID=2203;

   double epoch_takeoff=1366826870;
   double epoch_touchdown=1366828693;

   for (int frame_ID=takeoff_frame_ID; frame_ID <= touchdown_frame_ID;
        frame_ID++)
   {
      string frame_filename="frame-"+stringfunc::integer_to_string(
         frame_ID,5)+".png";
      double flight_frac=double(frame_ID-takeoff_frame_ID)/
         double(touchdown_frame_ID-takeoff_frame_ID);
      double curr_epoch=epoch_takeoff+flight_frac*
         (epoch_touchdown-epoch_takeoff);

      piecewise_linear UAV_easting_piecewise_linear(
         elapsed_secs,UAV_easting);
      piecewise_linear UAV_northing_piecewise_linear(
         elapsed_secs,UAV_northing);
      piecewise_linear UAV_altitude_piecewise_linear(
         elapsed_secs,UAV_altitude);
      piecewise_linear tgt_easting_piecewise_linear(
         elapsed_secs,tgt_easting);
      piecewise_linear tgt_northing_piecewise_linear(
         elapsed_secs,tgt_northing);
      piecewise_linear tgt_altitude_piecewise_linear(
         elapsed_secs,tgt_altitude);

      double curr_UAV_easting=UAV_easting_piecewise_linear.value(curr_epoch);
      double curr_UAV_northing=UAV_northing_piecewise_linear.value(curr_epoch);
      double curr_UAV_altitude=UAV_altitude_piecewise_linear.value(curr_epoch);
      double curr_tgt_easting=tgt_easting_piecewise_linear.value(curr_epoch);
      double curr_tgt_northing=tgt_northing_piecewise_linear.value(curr_epoch);
      double curr_tgt_altitude=tgt_altitude_piecewise_linear.value(curr_epoch);

      outstream << frame_filename << "  "
                << curr_epoch << "  "
                << curr_UAV_easting << "  "
                << curr_UAV_northing << "  "
                << curr_UAV_altitude << "  "
                << curr_tgt_easting << "  "
                << curr_tgt_northing << "  "
                << curr_tgt_altitude << "  "
                << endl;

   } // loop over frame_ID index
   filefunc::closefile(output_filename,outstream);

   string banner="Exported "+output_filename;
   outputfunc::write_big_banner(banner);
} 

