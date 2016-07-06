// =======================================================================
// Implementation of the ClientSocket class.  Source code comes from "Linux
// socket programming in C++" by Rob Tougher in issue 74 of online
// Linux Gazette, Jan 2002.
// =======================================================================

#include <iostream>
#include "socket/ClientSocket.h"
#include "socket/SocketException.h"

using std::cout;
using std::endl;

ClientSocket::ClientSocket ( std::string host, int port )
{
  connect_to_server(host,port);
}


bool ClientSocket::connect_to_server( std::string host, int port )
{
  if ( ! Socket::create() )
    {
      m_sock = -1;
      return false;
      
    }
  
  if ( ! Socket::connect ( host, port ) )
    {
      m_sock = -1;
      return false;
    }
  
  return true;
}


// --------------------------------------------------------------------------
int ClientSocket::receive_raw_bytes(char* buffer_ptr,int nbytes_to_receive)
{
   int nbytes_actually_received=read_from_socket(
      buffer_ptr,nbytes_to_receive);
   return nbytes_actually_received;
}
