// =======================================================================
// Program MEMORY_TO_SOCKET
// =======================================================================
// Last updated on 3/18/04
// =======================================================================

#include <iostream>
#include <new>
#include <string>
#include <time.h>
#include "fakeIFOV.h"
#include "ipc/ServerSocket.h"
#include "ipc/socketfuncs.h"
#include "general/sysfuncs.h"

// ==========================================================================
int main (int argc,int argv[])
{
   using std::string;
   using std::ostream;
   using std::istream;
   using std::cout;
   using std::cin;
   using std::ios;
   using std::endl;
   std::set_new_handler(sysfunc::out_of_memory);

// Define and initialize data structure:

   fakeIFOV* ifov_ptr=new fakeIFOV;
   for (int i=0; i<7; i++)
   {
      ifov_ptr->data[i]=i*10;
      cout << "i = " << i << " ifov_ptr->data[i] = "
           << ifov_ptr->data[i] << endl;
   }
//   ifov_ptr->deltaTime=35;
//   ifov_ptr->applanixTime=0.25;
//   cout << "deltaTime = " << ifov_ptr->deltaTime << endl;
//   cout << "applanixTime = " << ifov_ptr->applanixTime << endl;
   
// Recast data structure as an array of bytes.  Note that char*
// data_byte_ptr should ultimately be replaced by a pointer to shared
// memory:

   int n_data_bytes=sizeof(fakeIFOV);
   char* data_byte_ptr=reinterpret_cast<char *>(ifov_ptr);

// Open server socket and wait until client socket has been open and
// accepted:

   int portnumber=30000;
   ServerSocket* server_socket_ptr=socketfunc::open_server_socket(portnumber);

// Initialize a large buffer to temporarily store outgoing header and
// data information before they are bundled together into a single
// packet and sent to the socket connection:

   const int nbuffer_size=1000000;
   char* buffer_ptr=new char[nbuffer_size];

// Transmit multiple packets across socket connection from server to
// client:

   time_t start_time=time(NULL);

   int n_packets=4000;
//   int n_packets=5000;
   for (int packet_number=0; packet_number<n_packets; packet_number++)
   {

//      for (int i=0; i<7; i++)
//      {
//         ifov_ptr->data[i]=i*(10+packet_number);
//         cout << "i = " << i << " ifov_ptr->data[i] = "
//              << ifov_ptr->data[i] << endl;
//      }

      bool close_socket_after_transmission=(packet_number==n_packets-1);

      int nbytes_to_transmit=server_socket_ptr->form_packet(
         buffer_ptr,packet_number,n_data_bytes,
         close_socket_after_transmission,data_byte_ptr);
      int nbytes_actually_transmitted=
         server_socket_ptr->transmit_raw_bytes(
            buffer_ptr,nbytes_to_transmit);
   } // loop over packet_number index

   time_t stop_time=time(NULL);
   double processing_time=stop_time-start_time;
   cout << "Server processing time = " << processing_time << " secs" << endl;

   delete buffer_ptr;
   delete ifov_ptr;

   socketfunc::close_server_socket(server_socket_ptr);
}


