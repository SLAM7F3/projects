// ==========================================================================
// SOCKETFUNCS stand-alone methods
// ==========================================================================
// Last modified on 3/17/04
// ==========================================================================

#include "ServerSocket.h"
#include "ClientSocket.h"
#include "math/basic_math.h"
#include "general/outputfuncs.h"
#include "general/sysfuncs.h"
#include "general/stringfuncs.h"
#include "packet_header.h"
#include "socketfuncs.h"

using std::string;
using std::ostream;
using std::ofstream;
using std::cout;
using std::cin;
using std::endl;

namespace socketfunc
{

   ServerSocket* open_server_socket(int portnumber)
      {
         string banner="Opening server socket on "+sysfunc::get_hostname()
            +" & waiting for client request";
         outputfunc::write_banner(banner);
         ServerSocket server(portnumber);
         ServerSocket* new_socket_ptr=new ServerSocket;
         server.accept(*new_socket_ptr);
         outputfunc::write_banner("Client request accepted by server socket");
         return new_socket_ptr;
      }

   ClientSocket* open_client_socket(string server_IP_address,int portnumber)
      {
         string banner="Opening client socket on "+sysfunc::get_hostname();
         outputfunc::write_banner(banner);
         ClientSocket* new_socket_ptr=new ClientSocket(
            server_IP_address,portnumber);
         return new_socket_ptr;
      }

// -------------------------------------------------------------------------- 
   void close_server_socket(ServerSocket* socket_ptr)
      {
         string banner="Closing server socket on "+sysfunc::get_hostname();
         outputfunc::write_banner(banner);
         delete socket_ptr;
      }

   void close_client_socket(ClientSocket* socket_ptr)
      {
         string banner="Closing client socket on "+sysfunc::get_hostname();
         outputfunc::write_banner(banner);
         delete socket_ptr;
      }

} // socketfuncs namespace





