// ========================================================================
// Program XSENS writes out and records geolocation and geoorientation
// as a function of time.
// ========================================================================
// Last updated on 7/7/11; 8/18/11; 8/24/11
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "astro_geo/Clock.h"
#include "general/filefuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"
#include "ins/xsens_ins.h"

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

   string base_subdir;

// Search for a mounted SD card: (e.g. /media/CANON_DC or
// /media/34E9-289E).  If one exists, write output JPG files to its
// mount point.  Otherwise, write JPG files to subdir of
// /data/tech_challenge/ :

   vector<string> media_files=filefunc::files_in_subdir("/media/");
   for (int i=0; i<media_files.size(); i++)
   {
//      cout << "i = " << i
//           << " media file = " << media_files[i] << endl;
      string curr_subdir_name=filefunc::getbasename(media_files[i]);
//      cout << " curr_subdir_name = " << curr_subdir_name << endl;

      if (curr_subdir_name=="CANON_DC")
      {
         base_subdir=media_files[i]+"/";
         break;
      }
      else
      {
         vector<string> substrings=
            stringfunc::decompose_string_into_substrings(
               curr_subdir_name,"-_");
         if (stringfunc::is_number(substrings[0]))
         {
            base_subdir=media_files[i]+"/";
            break;
         }
         else
         {
            if (media_files.size()==1)
            {
               base_subdir=media_files[i]+"/";
            }
         }
      }
   } // loop over index i labeling subdirs & files within /media

   bool SDcard_inserted_flag=false;
   if (base_subdir.size() > 0)
   {
      cout << "There is *probably* an inserted SD card mounted to "
           << base_subdir << endl;
      SDcard_inserted_flag=true;
   }
   else
   {
      base_subdir="/data/tech_challenge/";
      cout << "No inserted SD card detected..." << endl;
   }
   cout << "base_subdir = " << base_subdir << endl;
   string xsens_subdir=base_subdir+"XSENS/";
   filefunc::dircreate(xsens_subdir);

   bool daylight_savings_flag=true;
   int UTM_zonenumber=19;

   Clock clock;
   clock.set_time_based_on_local_computer_clock(
      UTM_zonenumber,daylight_savings_flag);

   bool display_UTC_flag=false;
   string local_time_w_underscores=clock.YYYY_MM_DD_H_M_S(
      "_","_",display_UTC_flag);
   string xsens_filename=xsens_subdir+"xsens_"+local_time_w_underscores+".dat";
   cout << "xsens_filename = " << xsens_filename << endl;
//    outputfunc::enter_continue_char();

   ofstream outstream;
   outstream.precision(10);
   filefunc::openfile(xsens_filename,outstream);
   outstream << "# t     Elapsed secs          Timestamp" << endl;
   outstream << "# Lat   	Lon     	Alt" << endl;
   outstream << "# Az    	El	        Roll" << endl;
   outstream << endl;

   xsens_ins XSENS;
   while (!XSENS.initialize_xsens())
   {
      cout << "Trying to initialize XSENS" << endl;
   }
   XSENS.initialize_xsens_2();

   timefunc::initialize_timeofday_clock();

   double curr_time=1;
   double prev_time=0;
   double max_daz=NEGATIVEINFINITY;
   while (true)
   {
      XSENS.update_ins_metadata();

      double t=XSENS.get_elapsed_time();
      XSENS.update_ins_metadata_2();

//      XSENS.update_avg_az_el_roll();
      XSENS.update_median_az_el_roll();
      XSENS.alpha_filter_az_el_roll();

      XSENS.update_median_lat_lon_alt();
      XSENS.alpha_filter_lat_lon_alt();
//      XSENS.update_avg_physical_acceleration();

      double curr_daz=fabs(XSENS.get_daz());
      if (curr_daz > max_daz && t > 5)
      {
         max_daz=curr_daz;
//         cout << "max_daz = " << max_daz << endl;
      }

      clock.set_time_based_on_local_computer_clock(
         UTM_zonenumber,daylight_savings_flag);
      double curr_time=clock.secs_elapsed_since_reference_date(); 
//      cout << "curr_time = " << curr_time 
//           << " prev_time = " << prev_time
//           << " dt = " << curr_time-prev_time << endl;

//      if (abs(curr_time-prev_time) < 0.1) continue; 

      prev_time=curr_time;

      string time_stamp=clock.YYYY_MM_DD_H_M_S("_",":",false,1);
//      string time_stamp=clock.YYYY_MM_DD_H_M_S("_",":",true,1);

/*
      cout << "t = " << t
           << " elapsed_secs = " << curr_time 
           << " timestamp = " << time_stamp << endl;
*/

      if (fabs(XSENS.get_lat()) > 10)
      {
//         cout << "raw lat = " << XSENS.get_lat()
//              << " lon = " << XSENS.get_lon()
//              << " alt = " << XSENS.get_alt() << endl;
         cout << "filtered lat = " << XSENS.get_alpha_filtered_lat()
              << " lon = " << XSENS.get_alpha_filtered_lon()
              << " alt = " << XSENS.get_alpha_filtered_alt() << endl;
      }

//      cout << " az = " << XSENS.get_alpha_filtered_az()*180/PI
//           << " el = " << XSENS.get_alpha_filtered_el()*180/PI
//           << " roll = " << XSENS.get_alpha_filtered_roll()*180/PI  << endl;

      outstream << "# -----------------------------------------------------" 
                << endl;
      outstream << t << "  "
                << curr_time << "  " 
                << time_stamp << endl;
      outstream  << XSENS.get_lat() << "  "
                 << XSENS.get_lon() << "  "
                 << XSENS.get_alt() << endl;
      outstream << XSENS.get_az()*180/PI << "  "
                << XSENS.get_el()*180/PI << "  "
                << XSENS.get_roll()*180/PI 
                << endl << endl;

   }
}
