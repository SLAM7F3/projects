// ==========================================================================
// Program PARSE_NOVATEL
// ==========================================================================
// Last updated on 9/8/11; 2/28/13
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "astro_geo/Clock.h"
#include "general/filefuncs.h"
#include "astro_geo/geopoint.h"
#include "math/mypolynomial.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "osg/osg3D/tdpfuncs.h"
#include "track/tracks_group.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

// Set pass-dependent parameter values:

   int pass_number;
   cout << "Enter pass number (4 or 15)" << endl;
   cin >> pass_number;
   if (pass_number != 4 && pass_number != 15)
   {
      cout << "Invalid pass number entered!" << endl;
      exit(-1);
   }
   
   string novatel_file,output_filename,tdp_filename,gps_filename;
   string image_filename_prefix;
   int start_frame_number=0,stop_frame_number=0;
   if (pass_number==4)
   {
      novatel_file="pass4.ASC";
      output_filename="pass4_lla.dat";
      tdp_filename="novatel_path4.tdp";
      start_frame_number=224;	// pass 4
      stop_frame_number=535;	// pass 4
      gps_filename="interp_GPS_waypoints4.txt";
      image_filename_prefix="Passfour_";
   }
   else if (pass_number==15)
   {
      novatel_file="p15.ASC";
      output_filename="pass15_lla.dat";
      tdp_filename="novatel_path15.tdp";
      start_frame_number=159;	// pass 15
      stop_frame_number=572;	// pass 15
      gps_filename="interp_GPS_waypoints15.txt";
      image_filename_prefix="Passfifteen_";
   }

   vector<double> frame,time;
   if (pass_number==4)
   {
      frame.push_back(224);
      frame.push_back(225);
      frame.push_back(226);
      frame.push_back(227);

      frame.push_back(532);
      frame.push_back(533);
      frame.push_back(534);
      frame.push_back(535);
   
      time.push_back(1312038324);
      time.push_back(1312038325);
      time.push_back(1312038326);
      time.push_back(1312038327);

      time.push_back(1312038632);
      time.push_back(1312038633);
      time.push_back(1312038634);
      time.push_back(1312038635);
   }
   else if (pass_number==15)
   {
      frame.push_back(159);
      frame.push_back(160);
      frame.push_back(161);
      frame.push_back(162);

      frame.push_back(560);
      frame.push_back(561);
      frame.push_back(562);
      frame.push_back(563);

      time.push_back(1312045285);
      time.push_back(1312045286);
      time.push_back(1312045287);
      time.push_back(1312045288);

      time.push_back(1312045686);
      time.push_back(1312045687);
      time.push_back(1312045688);
      time.push_back(1312045689);
   }

// Compute linear fit for time as a function of frame number

   double chisq;
   mypolynomial frame_time_poly(1);
   frame_time_poly.fit_coeffs(frame,time,chisq);
   cout.precision(12);

   cout << "chisq = " << chisq << endl;
   for (unsigned int i=0; i<frame.size(); i++)
   {
      double curr_t=frame_time_poly.value(frame[i]);
      double dt=time[i]-curr_t;
      cout << "frame = " << frame[i]
           << " time[i] = " << time[i]
           << " dt = " << dt << endl;
   }

   Clock clock;

/*
   while (true)
   {
      int year=2011;
      int month=7;
      int day=30;

      int hour,minute;
      double sec;
      cout << "Enter hour:" << endl;
      cin >> hour;
      cout << "Enter minute:" << endl;
      cin >> minute;
      cout << "Enter sec:" << endl;
      cin >> sec;
   
      clock.set_local_time(year,month,day,hour,minute,sec);
      double secs_since_epoch=clock.secs_elapsed_since_reference_date();
      cout.precision(12);
      cout << "secs_since_epoch = " << secs_since_epoch << endl;
   }
*/

   bool strip_comments_flag=false;
   filefunc::ReadInfile(novatel_file,strip_comments_flag);

   ofstream outstream;
   filefunc::openfile(output_filename,outstream);
   outstream.precision(12);

   outstream << "#i GPS wk GPS secs      date           elapsed secs  lat          lon       alt" << endl << endl;

   tracks_group GPStracks_group;
   track* GPS_track_ptr=GPStracks_group.generate_new_track();

   int counter=0;
   cout.precision(12);
   vector<threevector> UTM_waypoints;
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<string> substrings=
         stringfunc::decompose_string_into_substrings(
            filefunc::text_line[i],",;");
      if (substrings[0] != "#INSPVAA") continue;
//      cout << filefunc::text_line[i] << endl;      

      int GPS_week=stringfunc::string_to_number(substrings[5]);
      double GPS_secs=stringfunc::string_to_number(substrings[6]);
      double lat=stringfunc::string_to_number(substrings[12]);
      double lon=stringfunc::string_to_number(substrings[13]);
      double alt=stringfunc::string_to_number(substrings[14]);

      geopoint curr_geo(lon,lat,alt);
      UTM_waypoints.push_back(curr_geo.get_UTM_posn());

      double secs_since_epoch=
         clock.GPS_time_to_elapsed_secs(GPS_week,GPS_secs);
      clock.convert_elapsed_secs_to_date(secs_since_epoch);

      GPS_track_ptr->set_XYZ_coords(secs_since_epoch,UTM_waypoints.back());

      outstream << counter << " "
                << GPS_week << " "
                << GPS_secs << " "
                << clock.YYYY_MM_DD_H_M_S() << " "
                << clock.secs_elapsed_since_reference_date() << " "
                << lat << " "
                << lon << " "
                << alt << endl;
      counter++;

   } // loop over index i labeling lines within novatel input ascii file

   filefunc::closefile(output_filename,outstream);

// Generate TDP file containing Novatel GPS path:

   tdpfunc::write_relative_xyz_data(tdp_filename,UTM_waypoints);

   string unix_cmd="lodtree "+tdp_filename;
   sysfunc::unix_command(unix_cmd);

// Write out interpolated time and GPS position vs frame number:

   outstream.precision(12);
   filefunc::openfile(gps_filename,outstream);
   outstream << "# Filename      Epoch Time (s) Easting (m)    Northing (m) Altitude (m)" << endl;
   outstream << endl;
   
   threevector curr_posn;
   for (int frame=start_frame_number; frame <= stop_frame_number; frame++)
   {
      double curr_t=frame_time_poly.value(frame);
      GPS_track_ptr->get_interpolated_posn(curr_t,curr_posn);

      string image_filename=image_filename_prefix
         +stringfunc::integer_to_string(frame,4)+".rd.jpg";

      outstream << image_filename << "  "
                << stringfunc::number_to_string(curr_t,2) << "  "
                << stringfunc::number_to_string(curr_posn.get(0),2) << "  "
                << stringfunc::number_to_string(curr_posn.get(1),2) << "  "
                << stringfunc::number_to_string(curr_posn.get(2),2) << endl;
   } // loop over frame number

   filefunc::closefile(gps_filename,outstream);

}
