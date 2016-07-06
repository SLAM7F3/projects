// ========================================================================
// Program WHITE_BOX opens a serial port to Darin Marriot's "white"
// IMU box and writes its data output to a binary file.

//				white_box

// ========================================================================
// Last updated on 7/14/10; 7/15/10
// ========================================================================

#include <iostream>
#include "math/constant_vectors.h"
#include "general/filefuncs.h"
#include "astro_geo/GPS_datastream.h"
#include "messenger/Messenger.h"
#include "messenger/serial_device.h"
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

// Instantiate GPS messenger:

   string broker_URL="tcp://127.0.0.1:61616";
   cout << "ActiveMQ broker_URL = " << broker_URL << endl;

   string GPS_message_queue_channel_name="viewer_update";
   Messenger GPS_messenger( broker_URL, GPS_message_queue_channel_name );

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
      cout << "No serial device detected attached to /dev/ttyUSBN!" << endl;
      exit(-1);
   }
   else
   {
      cout << "Serial device detected attached to "+serialPort << endl;
   }

   int baud_rate=115200;
   serial_device white_device(serialPort,baud_rate);

   string output_binary_filename="white_box_binary.dat";
   white_device.open_binary_output_file(output_binary_filename);

   while(true)
   {
      white_device.readData_into_binary_file();
   }
   
   white_device.close_binary_output_file();
}

