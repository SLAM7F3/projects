// =======================================================================
// Socket class header file.  Source code comes from "Linux socket
// programming in C++" by Rob Tougher in issue 74 of online Linux
// Gazette, Jan 2002.
// =======================================================================
// Last updated on 5/21/04
// =======================================================================

#ifndef Socket_class
#define Socket_class

#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/un.h> // for UNIX sockets

class Socket
{
  public:

   Socket();
   virtual ~Socket();

   // Server initialization
   bool create();
   bool bind(const int port);
   bool listen() const;
   bool accept(Socket&) const;

   // Client initialization
   bool connect (const std::string host, const int port);

   // Data Transmission
   void set_non_blocking(const bool);
   int write_to_socket(const char* char_buffer_ptr,int nbytes_attempted) 
      const;
   int read_from_socket(char* char_buffer_ptr,int nbytes_requested) const;
   void write_all_to_socket(
      const char* char_buffer_ptr,unsigned long nbytes_requested) const;
   void read_all_from_socket(
      char* char_buffer_ptr,unsigned long nbytes_requested) const;

   bool is_valid() const { return m_sock != -1; }

   // connection status
   bool connection_is_ready();
   bool poll_connection(int sec, int usec);
   void disconnect();
   
   bool can_be_read();

   mutable bool socket_closed_on_last_read;
   
// protected:
   
   int TYPE;
   mutable int m_sock;
   sockaddr_in m_addr;
};

#endif // Socket_class







