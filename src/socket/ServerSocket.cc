// =======================================================================
// Implementation of the ServerSocket class.  Source code comes from
// "Linux socket programming in C++" by Rob Tougher in issue 74 of
// online Linux Gazette, Jan 2002.
// =======================================================================
// Last updated on 5/21/04
// =======================================================================

#include <iostream>
#include "socket/packet_header.h"
#include "socket/ServerSocket.h"
#include "socket/SocketException.h"

using std::cout;
using std::endl;

// Constant declarations:

const int packet_header_size=sizeof(packet_header);


ServerSocket::ServerSocket (int port)
{
   if ( ! Socket::create() )
   {
      throw SocketException ( "Could not create server socket." );
   }

   if ( ! Socket::bind ( port) )
   {
      throw SocketException ( "Could not bind to port." );
   }

   if ( ! Socket::listen() )
   {
      throw SocketException ( "Could not listen to socket." );
   }

}


// ---------------------------------------------------------------------
ServerSocket::~ServerSocket()
{
//   std::cout << "inside ServerSocket destructor" << std::endl;
}

// ---------------------------------------------------------------------
void ServerSocket::accept ( ServerSocket& sock )
{
   if ( ! Socket::accept ( sock ) )
   {
      throw SocketException ( "Could not accept socket." );
   }
}

// --------------------------------------------------------------------------
// Member function form_packet takes in header information as well as
// a pointer to data information.  This method generates a single
// packet within buffer array *buffer_ptr.  It returns the packet's
// total size in bytes.

//int ServerSocket::form_packet(
//   char* buffer_ptr,int packet_ID,int n_data_bytes,
//   int data_type, int recording_state,char* data_byte_ptr)
//{
// First generate packet header and copy its contents onto beginning
// of buffer:

//   int ntotal_bytes=packet_header_size+n_data_bytes;
//
//   packet_header curr_header;
//   curr_header.n_data_bytes = ntotal_bytes;
//   curr_header.packet_ID = packet_ID;
//   curr_header.recording_state = recording_state;
//   curr_header.data_type = data_type;
//
//   memcpy(buffer_ptr,&curr_header,packet_header_size);
//
// Next copy data into buffer:
//
//   memcpy(buffer_ptr+packet_header_size,data_byte_ptr,n_data_bytes);
//
//   return ntotal_bytes;
//}

// --------------------------------------------------------------------------
// Member function transmit_raw_bytes attempts to write to the socket
// nbytes_to_transmit bytes located at the start of *buffer_ptr.  It
// returns the number of bytes actually transmitted.

//int ServerSocket::transmit_raw_bytes(char* buffer_ptr,int nbytes_to_transmit)
//{
//   int nbytes_actually_transmitted=write_to_socket(
//      buffer_ptr,nbytes_to_transmit);
//
//   cout << "Server on "+sysfunc::get_hostname() 
//        << " actually transmitted " << nbytes_actually_transmitted 
//        << " bytes" << endl;
//
//   return nbytes_actually_transmitted;
//}

