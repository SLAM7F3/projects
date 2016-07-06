// =======================================================================
// Header file for ClientSocket class.  Source code comes from "Linux
// socket programming in C++" by Rob Tougher in issue 74 of online
// Linux Gazette, Jan 2002.
// =======================================================================

#ifndef CLIENT_SOCKET_H
#define CLIENT_SOCKET_H

#include "socket/Socket.h"
#include <sys/socket.h>

class ClientSocket : public Socket
{
  public:

   ClientSocket()
      {
         m_sock = -1;
      }
   
   ClientSocket(std::string host,int port);
   virtual ~ClientSocket(){};

   bool connect_to_server(std::string host, int port);
   int receive_raw_bytes(char* buffer_ptr,int nbytes_to_receive);
};


#endif
