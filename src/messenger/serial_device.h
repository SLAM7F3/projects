// ==========================================================================
// Header file for serial_device class
// ==========================================================================
// Last modified on 5/7/10; 7/14/10; 7/15/10
// ==========================================================================

#ifndef SERIAL_DEVICE_H
#define SERIAL_DEVICE_H

#include <SerialStream.h>
#include <fstream>
#include <iostream>
#include <string> 

class serial_device
{

  public:

// Initialization, constructor and destructor functions:

   serial_device(std::string serialPort,int baud_rate);

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~serial_device();

// Set and get member functions:

   void open(std::string serialPort);
   void close();
   std::string readData();

   void open_binary_output_file(std::string binary_filename);
   void readData_into_binary_file();
   void close_binary_output_file();

  private: 

   int baud_rate,output_byte_counter;
   LibSerial::SerialStream _port;
   std::string _portName;
   std::ofstream binary_stream;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const serial_device& s);

   void initialize_heading_change_matrices();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

/*
inline int serial_device::get_ID() const
{
   return ID;
}
*/

#endif  // serial_device.h



