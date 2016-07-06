// ==========================================================================
// Program SOCKET_TO_MEMORY
// ==========================================================================
// Last updated on 3/19/04
// ==========================================================================

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <new>
#include <string>
#include <time.h>
#include "math/basic_math.h"
#include "ipc/buffer.h"
#include "ipc/ClientSocket.h"
#include "fakeIFOV.h"
#include "general/outputfuncs.h"
#include "ipc/socketfuncs.h"
#include "general/sysfuncs.h"

// ==========================================================================
int main(int argc, char *argv[])
{
   using std::string;
   using std::ostream;
   using std::istream;
   using std::ifstream;
   using std::ofstream;
   using std::cout;
   using std::cin;
   using std::ios;
   using std::endl;
   std::set_new_handler(sysfunc::out_of_memory);

// Open client socket:

   int portnumber=30000;
   string quantum1_IP_address="155.34.164.53";
   string photon_IP_address="155.34.174.11";
   string localhost_IP_address="127.0.0.1";
   string server_IP_address=quantum1_IP_address;
//   string server_IP_address=photon_IP_address;
//   string server_IP_address=localhost_IP_address;
   ClientSocket* client_socket_ptr=socketfunc::open_client_socket(
      server_IP_address,portnumber);

// Initialize ring buffer to hold incoming data from socket
// connection:

//   const int buffer_size=50;
   const int buffer_size=10000;
   buffer byte_buffer(buffer_size);

   char* shared_memory_ptr=new char[1000000];

// Continuously read in data packets from socket connection until a
// close_client_socket signal is received from server:

   time_t start_time=time(NULL);

   int nbytes_to_receive=sizeof(fakeIFOV)+12;

   bool close_client_socket_flag=false;
   while (!close_client_socket_flag)
   {
      int nbytes_actually_received=client_socket_ptr->receive_raw_bytes(
         byte_buffer.char_buffer_ptr+byte_buffer.raw_data_end_posn,
         nbytes_to_receive);

      byte_buffer.update_byte_array_counters(nbytes_actually_received);
      byte_buffer.parse_packet_header(close_client_socket_flag);

      if (byte_buffer.ndata_bytes_in_buffer >= byte_buffer.curr_packet_size)
      {
// Get shared memory pointer
// Lock access to shared memory
         byte_buffer.copy_packet_data_to_shared_memory(
            byte_buffer.curr_packet_size,shared_memory_ptr);
// Free access to shared memory

//         byte_buffer.display_byte_array_contents(
//            shared_memory_ptr,nbytes_copied);

// Recast received character array as data structure:

//      fakeIFOV const *ifov_new_ptr=
//         reinterpret_cast<tagIFOV*>(char_buffer_ptr);
//      outputfunc::newline();
//      cout << "new data[0] = " << ifov_new_ptr->data[0] 
//           << " new data[1] = " << ifov_new_ptr->data[1] 
//           << " new data[2] = " << ifov_new_ptr->data[2] << endl;
//      cout << "new deltaTime = " << ifov_new_ptr->deltaTime 
//           << endl;
//      cout << "new applanixTime = " << ifov_new_ptr->applanixTime 
//           << endl;

// Delete dynamically generated data structure once we're done with it:
      
//      delete ifov_new_ptr;

      } // ndata_bytes_in_buffer >= curr_packet_size conditional
   } // close_client_socket_flag conditional

   time_t stop_time=time(NULL);
   double processing_time=stop_time-start_time;
   cout << "Server processing time = " << processing_time << " secs" << endl;

   socketfunc::close_client_socket(client_socket_ptr);

   delete [] shared_memory_ptr;
}


