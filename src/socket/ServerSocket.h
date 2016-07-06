// =======================================================================
// Header file for ServerSocket class.  Source code comes from "Linux
// socket programming in C++" by Rob Tougher in issue 74 of online
// Linux Gazette, Jan 2002.
// =======================================================================
// Last updated on 3/18/04
// =======================================================================

#ifndef ServerSocket_class
#define ServerSocket_class

#include "socket/Socket.h"
#include <sys/socket.h>

class ServerSocket : public Socket
{
  public:

   ServerSocket (int port);
   ServerSocket (){};
   virtual ~ServerSocket();

   void accept(ServerSocket&);

   //   int form_packet(
   //	  char* buffer_ptr,int packet_ID,int n_data_bytes,
   //	  int data_type, int recording_state,char* data_byte_ptr);
   //
   //   int transmit_raw_bytes(char* buffer_ptr,int nbytes_to_transmit);
};

#endif
