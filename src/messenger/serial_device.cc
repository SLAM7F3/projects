// ==========================================================================
// Serial_Device class member function definitions
// ==========================================================================
// Last modified on 5/10/10; 7/14/10; 7/15/10
// ==========================================================================

#include <iostream>
#include <vector>
#include "general/filefuncs.h"
#include "messenger/serial_device.h"

using namespace LibSerial;

using std::cout;
using std::endl;
using std::flush;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void serial_device::allocate_member_objects()
{
}		       

void serial_device::initialize_member_objects()
{
   output_byte_counter=0;
}

serial_device::serial_device(string serialPort,int baud_rate)
{
   allocate_member_objects();
   initialize_member_objects();

   this->baud_rate=baud_rate;
   open(serialPort);
}

serial_device::~serial_device()
{
}

// ---------------------------------------------------------------------
void serial_device::docopy(const serial_device& s)
{
//   cout << "inside serial_device::docopy()" << endl;

}

// =====================================================================
// Member function open()

void serial_device::open(string serialPort)
{
    _portName = serialPort;
    _port.Open( _portName );
    if (baud_rate==4800)
    {
       _port.SetBaudRate( SerialStreamBuf::BAUD_4800 );
    }
    else if (baud_rate==115200)
    {
       _port.SetBaudRate( SerialStreamBuf::BAUD_115200 );
    }

    _port.SetCharSize( SerialStreamBuf::CHAR_SIZE_8 );
    _port.SetParity( SerialStreamBuf::PARITY_NONE );
    _port.SetNumOfStopBits( 1 );
    _port.SetFlowControl( SerialStreamBuf::FLOW_CONTROL_NONE );
    _port.SetVMin( 1 );
    _port.SetVTime( 255 );
}

void serial_device::close()
{
    _port.Close();
}

string serial_device::readData()
{
//   cout << "Inside serial_device::readData()" << endl;
   char tmp[1024];
   _port >> tmp;

   string buffer = tmp;
   return buffer;
}

void serial_device::open_binary_output_file(string binary_filename)
{
   filefunc::open_binaryfile(binary_filename,binary_stream);
}

void serial_device::readData_into_binary_file()
{
   
//   cout << "inside serial_device::readData_into_binary_file()" << endl;

   const int buffer_size=1024;
   char tmp[buffer_size];
//   char tmp;
   _port.read(tmp,buffer_size);
//   int nbytes_read=_port.read(tmp,buffer_size);
//   cout << "nbytes read = " << nbytes_read << endl;

//   _port.readsome(tmp,buffer_size);
   
//   _port >> tmp;

/*
   string buffer=tmp;
   cout << "buffer.size() = " << buffer.size() << endl;
   cout << "buffer = " << buffer << endl << endl;
   output_byte_counter += buffer.size();
   cout << "output_byte_counter = " << output_byte_counter << endl;
*/

//   binary_stream << tmp << flush;
   binary_stream.write(tmp,buffer_size);
   
//   binary_stream << buffer.c_str() << flush;
//   binary_stream << buffer.c_str();

//   binary_stream.write(tmp,sizeof(tmp));
//   binary_stream.write(tmp,buffer.size());
}

void serial_device::close_binary_output_file()
{
   binary_stream.close();
}
