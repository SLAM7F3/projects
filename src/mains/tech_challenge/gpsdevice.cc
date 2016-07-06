// ========================================================================
// Program GPSDEVICE is a high-level program which continuously reads
// data from a serial port connected to an external GPS unit.
// GPSDEVICE broadcasts to ActiveMQ channel "viewer_update" the
// current time, geoposition and number of visible satellites.  It
// also records this information to a time-stamped archive file within
// /data/tech_challenge/GPS_tracks.

//				gpsdevice

// ========================================================================
// Last updated on 10/2/10; 7/16/11; 8/18/11
// ========================================================================

#include <iostream>
#include "math/constant_vectors.h"
#include "general/filefuncs.h"
#include "filter/filterfuncs.h"
#include "astro_geo/GPS_datastream.h"
#include "messenger/Messenger.h"
#include "math/statevector.h"
#include "general/sysfuncs.h"
#include "track/tracks_group.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::ofstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

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

// Instantiate GPS messenger:

   string broker_URL="tcp://127.0.0.1:61616";
//   cout << "ActiveMQ broker_URL = " << broker_URL << endl;

   string GPS_message_queue_channel_name="viewer_update";
   Messenger GPS_messenger( broker_URL, GPS_message_queue_channel_name );

   tracks_group gps_tracks_group;
   int GPS_ID=3;
   track* gps_track_ptr=gps_tracks_group.generate_new_track(GPS_ID);
   gps_track_ptr->set_description("Ground rover GPS");

// Search for GPS device attached to /dev/ttyUSBN where N=0,1,2,...
// On 7/16/2011, we pushed the search for /dev/ttyUSB0 to the end of
// the possible /dev list.  This change allows the orange XSENS box to
// be plugged in first to a laptop USB port and be assigned
// /dev/ttyUSB0.  A subsequent GPS hockeypuck device will then be
// assigned /dev/ttyUSB1 or higher.  Output from both sensors can then
// be recorded to text files.

   vector<string> candidate_serial_ports;
   candidate_serial_ports.push_back("/dev/ttyUSB1");
   candidate_serial_ports.push_back("/dev/ttyUSB2");
   candidate_serial_ports.push_back("/dev/ttyUSB3");
   candidate_serial_ports.push_back("/dev/ttyUSB4");
   candidate_serial_ports.push_back("/dev/ttyUSB5");
   candidate_serial_ports.push_back("/dev/ttyUSB0");

   bool serial_port_device_found_flag=false;   
   string serialPort;
   for (int i=0; i<candidate_serial_ports.size(); i++)
   {
      string curr_serialPort=candidate_serial_ports[i];
      if (filefunc::chardevexist(curr_serialPort))
      {
         serialPort=curr_serialPort;
         serial_port_device_found_flag=true;
         break;
      }
   }
    
   cout << endl;
   if (!serial_port_device_found_flag)
   {
      cout << "No GPS device detected attached to /dev/ttyUSBN!" << endl;
      exit(-1);
   }
   else
   {
      cout << "GPS device detected attached to "+serialPort << endl;
   }

   GPS_datastream GPS_datastream(serialPort);
   double prev_time=0; 

// Wait for GPS satellite connection before opening output GPS track
// file:

   string banner="Trying to establish satellite contact:";
   outputfunc::write_big_banner(banner);

   cout << "GPS satellite link should hopefully be soon established.  If no GPS lock"  << endl;
   cout << "is achieved in roughly 30 secs, it's reasonable to kill this program" <<endl;
   cout << "by entering control-c and restarting it again.  You may need to restart" << endl;
   cout << "the program multiple times before a solid link with GPS satellites" << endl;
   cout << "is established and useful GPS data starts recording to disk..." << endl << endl << endl;

   bool date_read_flag=false;
   bool time_read_flag=false;
   bool curr_date_read_flag,curr_time_read_flag;
   while (!date_read_flag || !time_read_flag)
   { 
      if (GPS_datastream.read_curr_data(
         curr_date_read_flag,curr_time_read_flag))
      {
         if (curr_date_read_flag) date_read_flag=true;
         if (curr_time_read_flag) time_read_flag=true;
      }
   }

   banner="Satellite contact established!";
   outputfunc::write_big_banner(banner);

// Open file for permanently archiving GPS track on computer to which
// it is physically attached:

   string GPS_subdir=base_subdir+"GPS_tracks/";
   filefunc::dircreate(GPS_subdir);

   cout << "UTC Clock time = " << GPS_datastream.get_clock().
      YYYY_MM_DD_H_M_S() << endl;

   bool display_UTC_flag=false;
   string local_time=GPS_datastream.get_clock().YYYY_MM_DD_H_M_S(
      " ",":",display_UTC_flag);
   string local_time_w_underscores=GPS_datastream.get_clock().YYYY_MM_DD_H_M_S(
      "_","_",display_UTC_flag);
   cout << "Local clock time = " << local_time << endl;

   string GPS_filename=GPS_subdir+"gps_track_"+local_time_w_underscores+".dat";

   banner="Archiving GPS data to "+GPS_filename;
   outputfunc::write_big_banner(banner);
//   cout << "GPS_filename = " << GPS_filename << endl;
   outputfunc::enter_continue_char();

   ofstream archive_stream;
   archive_stream.precision(12);
   filefunc::openfile(GPS_filename,archive_stream);
   archive_stream << 
      "# Timestamp EpochSecs FixQual Nsats HDOP    X	Y	Z	Vx	Vy 	Vz"
                  << endl << endl;

   cout.precision(12); 

   int counter=0;
   threevector curr_filtered_lla_posn,prev_filtered_lla_posn;
   threevector grid_origin=Zero_vector;

   while (true) 
   { 
      GPS_datastream.read_curr_data(); 
/*
      cout << "==================================================" << endl;
      cout << "Current time = "  
           << GPS_datastream.get_clock().YYYY_MM_DD_H_M_S() << endl; 
      cout << "Fix quality = " << GPS_datastream.get_fix_quality() 
           << " n_satellites = " << GPS_datastream.get_n_satellites() 
           << " horiz_dilution = " << GPS_datastream.get_horiz_dilution() 
           << endl; 
      cout << "Current geoposition = " << GPS_datastream.get_geoposn()  
           << endl; 
      cout << endl; 
*/

      double curr_time=GPS_datastream.get_clock(). 
         secs_elapsed_since_reference_date(); 
      string time_stamp=GPS_datastream.get_clock().YYYY_MM_DD_H_M_S(
         "_",":",false,1);
      
      if (abs(curr_time-prev_time) < 0.5) continue; 

      cout.precision(12); 
//      cout << "curr_time = " << curr_time 
//           << " prev_time = " << prev_time
//           << " dt = " << curr_time-prev_time << endl;
      
      prev_time=curr_time;

      double fix_quality=GPS_datastream.get_fix_quality();
      double n_satellites=GPS_datastream.get_n_satellites();
      double horizontal_dilution=GPS_datastream.get_horiz_dilution();
      threevector GPS_quality(fix_quality,n_satellites,horizontal_dilution);

      geopoint curr_geopoint=GPS_datastream.get_geoposn();
      threevector velocity=Zero_vector;

// As of May 2010, Jennifer Drexler's blue force tracker thin client
// expects to receive longitude,latitude rather than easting,northing
// geocoordinates:  

      threevector curr_lla_posn(curr_geopoint.get_longitude(),
	      curr_geopoint.get_latitude(),
	      curr_geopoint.get_altitude());

      gps_track_ptr->set_posn_velocity_GPSquality(
         curr_time,curr_lla_posn,velocity,GPS_quality);

// Record current GPS information to archive file provided satellite
// lock isn't lousy:

      if (fix_quality==0) continue;
      if (horizontal_dilution > 10) continue;

// Important note: In order to display GPS tracks within OpenLayers,
// we need to archive lon,lat and NOT UTM geocoords:

      threevector curr_velocity(0,0,0);

      if (counter==0)
      {
         curr_filtered_lla_posn=curr_lla_posn;
      }
      else
      {

// Perform alpha filtering of raw GPS measurements.  Recall smaller
// values for alpha imply more filtering with previously measured
// values:

//         const double alpha=0.0001;
         const double alpha=0.01;
//         const double alpha=0.03;
//         const double alpha=0.1;
//         const double alpha=0.33;
//         const double alpha=1.0;
         
         curr_filtered_lla_posn=filterfunc::alpha_filter(
            curr_lla_posn,prev_filtered_lla_posn,alpha);
         geopoint curr_GPS(
            curr_filtered_lla_posn.get(0),curr_filtered_lla_posn.get(1));
         geopoint prev_GPS(
            prev_filtered_lla_posn.get(0),prev_filtered_lla_posn.get(1));
         threevector delta_GPS=curr_GPS.get_UTM_posn()-prev_GPS.get_UTM_posn();
         cout << "==================================================" << endl;
         cout << "delta_GPS.mag = " << delta_GPS.magnitude() << endl;

         GPS_datastream.alpha_filter_lat_lon_alt();

         threevector raw_UTM_posn=
            GPS_datastream.get_raw_UTM_posn();
         threevector filtered_UTM_posn=
            GPS_datastream.get_alpha_filtered_UTM_posn();

         cout << "raw UTM = " << raw_UTM_posn.get(0) << " , "
              << raw_UTM_posn.get(1) << endl;
         cout << "filtered UTM = " << filtered_UTM_posn.get(0) << " , "
              << filtered_UTM_posn.get(1) << endl;
      }

      prev_filtered_lla_posn=curr_filtered_lla_posn;
      counter++;

      archive_stream << time_stamp << " "
		     << curr_time << " "
                     << fix_quality << " "
                     << n_satellites << " "
                     << horizontal_dilution << " "
                     << curr_lla_posn.get(0) << " "
                     << curr_lla_posn.get(1) << " "
                     << curr_lla_posn.get(2) << " "
                     << curr_velocity.get(0) << " "
                     << curr_velocity.get(1) << " "
                     << curr_velocity.get(2) << " "
                     << endl;

// Broadcast alpha-filtered GPS position information via ActiveMQ:

//      gps_track_ptr->broadcast_statevector(curr_time,&GPS_messenger);
      statevector curr_statevector(curr_time,curr_filtered_lla_posn,velocity);
      gps_track_ptr->broadcast_statevector(
         curr_statevector,&grid_origin,&GPS_messenger);

   } // infinite while loop

   filefunc::closefile(GPS_filename,archive_stream);
   
//   cout << "GPS track = " << *gps_track_ptr << endl;
}

