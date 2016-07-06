// =========================================================================
// Header file for stand-alone drawing functions.
// =========================================================================
// Last modified on 3/17/04
// =========================================================================

#ifndef SOCKETFUNCS_H
#define SOCKETFUNCS_H

#include <string>
class ServerSocket;
class ClientSocket;

namespace socketfunc
{
   ServerSocket* open_server_socket(int portnumber);
   ClientSocket* open_client_socket(
      std::string server_IP_address,int portnumber);

   void close_server_socket(ServerSocket* socket_ptr);
   void close_client_socket(ClientSocket* socket_ptr);
}

#endif // socketfuncs.h




