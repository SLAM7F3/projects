// ========================================================================
// Program SYNC_CLOCK_W_GPS is a stripped variant of GPSDEVICE.  It
// attempts to establish a connection to GPS satellites via a GPS unit
// attached as a USB device.  If a satellite connection is
// established, this program resets the computer's local time to GPS
// time.  SET_CLOCK must be run by the root superuser within a tcsh
// shell with Peter's .cshrc environmental variables enabled.

//				sync_clock_w_gps

// In order to run this program, user needs to perform following steps:

// 1.  sudo su root

// 2.  /bin/tcsh (to change shell from bash to tcsh)

// 3.  source /home/cho/.cshrc to load Peter's environment variables

// Prompt should now look something like Belka-19:root% 

// 4.  /usr/local/bin/sync_clock_w_gps

// 5.  exit  (out of tcsh)

// 6.  exit  (out of sudo)

// 7.  whoami (to be sure that root has been succcessfully exited)

// ========================================================================
// Last updated on 7/27/10; 8/15/10; 8/22/10
// ========================================================================

#include <iostream>
#include "general/filefuncs.h"
#include "astro_geo/GPS_datastream.h"
#include "general/sysfuncs.h"

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

// Search for GPS device attached to /dev/ttyUSBN where N=0,1,2,...

   vector<string> candidate_serial_ports;
   candidate_serial_ports.push_back("/dev/ttyUSB0");
   candidate_serial_ports.push_back("/dev/ttyUSB1");
   candidate_serial_ports.push_back("/dev/ttyUSB2");
   candidate_serial_ports.push_back("/dev/ttyUSB3");
   candidate_serial_ports.push_back("/dev/ttyUSB4");
   candidate_serial_ports.push_back("/dev/ttyUSB5");

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

// Wait for GPS satellite connection:

   string banner="Trying to establish satellite contact:";
   outputfunc::write_big_banner(banner);

   bool date_read_flag=false;
   bool time_read_flag=false;
   bool curr_date_read_flag,curr_time_read_flag;
   int counter=0;
   while (!date_read_flag || !time_read_flag)
   { 
      banner="Waiting for good GPS satellite lock...Need horiz_dilution less than 2.1";
      outputfunc::write_banner(banner);

      if (GPS_datastream.read_curr_data(
         curr_date_read_flag,curr_time_read_flag))
      {
         
// Make sure GPS lock is reasonably good:

         if (GPS_datastream.get_fix_quality()==0) 
         {
            counter++;
            continue;
         }

         if (GPS_datastream.get_horiz_dilution() > 2.1 &&
	     GPS_datastream.get_n_satellites() <= 4)
         {
            counter++;
            continue;
         }
         
         if (curr_date_read_flag) date_read_flag=true;
         if (curr_time_read_flag) time_read_flag=true;
         counter++;
      }
   }

   banner="Satellite contact established!";
   outputfunc::write_big_banner(banner);
   cout << "Contact counter = " << counter << endl;

   GPS_datastream.get_clock().reset_local_computer_time();
}

