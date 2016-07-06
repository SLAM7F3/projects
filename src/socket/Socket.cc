// =======================================================================
// Implementation of the Socket class.  Source code comes from "Linux
// socket programming in C++" by Rob Tougher in issue 74 of online
// Linux Gazette, Jan 2002.
// =======================================================================
// Last updated on 5/21/04; 4/5/14
// =======================================================================

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <sys/types.h>		
#include <sys/socket.h>		
#include <sys/select.h>         // for data_is_ready, poll_data
#include <stdio.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <unistd.h>
#include "socket/Socket.h"
#include "socket/SocketException.h"

using std::cerr;
using std::cout;
using std::endl;

Socket::Socket() :
   m_sock ( -1 )
{
   memset (&m_addr,0,sizeof ( m_addr ));
   socket_closed_on_last_read = false;
   TYPE = AF_INET;
}

// ---------------------------------------------------------------------
Socket::~Socket()
{
   if ( is_valid() ) ::close ( m_sock );
}

// ---------------------------------------------------------------------
bool Socket::create() 
{
   if (m_sock != -1) 
      return true;
  
   // have we already connected?
   m_sock = socket(TYPE,SOCK_STREAM,0);
   if (m_sock==-1)
      perror("socket");
  
   if (!is_valid()) return false;
  
   // TIME_WAIT - argh
   int on = 1;
   if (setsockopt(m_sock,SOL_SOCKET,SO_REUSEADDR, 
                  ( const char* ) &on, sizeof ( on ) ) == -1 ) return false;
   return true;
}

// ---------------------------------------------------------------------

bool Socket::bind ( const int port )
{
   if (!is_valid()) return false;

   m_addr.sin_family = TYPE;
   m_addr.sin_addr.s_addr = INADDR_ANY;
   m_addr.sin_port = htons ( port );

   int bind_return = ::bind(m_sock,(struct sockaddr *) &m_addr,
                            sizeof (m_addr));

   return (bind_return != -1);
}

// ---------------------------------------------------------------------
bool Socket::listen() const
{
   const int MAXCONNECTIONS = 5;

   if (!is_valid()) return false;

   int listen_return = ::listen(m_sock,MAXCONNECTIONS );
   return (listen_return != -1);
}

// ---------------------------------------------------------------------
bool Socket::accept ( Socket& new_socket ) const
{
   int addr_length = sizeof ( m_addr );
   new_socket.m_sock = ::accept ( m_sock, ( sockaddr * ) &m_addr, 
                                  ( socklen_t * ) &addr_length );
   return (new_socket.m_sock > 0);
}

// ---------------------------------------------------------------------
bool Socket::connect(const std::string host,const int port)

{
   if ( ! is_valid() ) return false;
  
   m_addr.sin_family = TYPE;
   m_addr.sin_port = htons ( port );
  
   int status = inet_pton ( TYPE, host.c_str(), &m_addr.sin_addr );
   if ( errno == EAFNOSUPPORT ) return false;
   status = ::connect( m_sock, ( sockaddr * ) &m_addr, sizeof ( m_addr ));
   if (status==-1)
   {
      // if we were unsucessful, close the file handle
      ::close(m_sock);
      m_sock = -1;
   }
  
   return (status==0);
}


// ---------------------------------------------------------------------
void Socket::set_non_blocking(const bool b)
{
   int opts = fcntl(m_sock,F_GETFL);

   if ( opts < 0 ) return;
   if ( b )
      opts = ( opts | O_NONBLOCK );
   else
      opts = ( opts & ~O_NONBLOCK );

   fcntl (m_sock,F_SETFL,opts);
}

// ---------------------------------------------------------------------
// Member function write_to_socket takes in a pointer to a character
// array containing at least nbytes_attempted bytes.  It tries to
// write out this many bytes to the socket.  This method returns the
// number of bytes actually written to the socket.

int Socket::write_to_socket(
   const char* char_buffer_ptr,int nbytes_attempted) const
{
// Note: Hyrum recommends calling ::write rather than ::send for
// streaming large quantities of data...

   int nbytes_actually_transmitted = 
      ::write(m_sock,char_buffer_ptr,nbytes_attempted);
//       ::send(m_sock,char_buffer_ptr,nbytes_attempted,MSG_NOSIGNAL);
   return nbytes_actually_transmitted;
}

// ---------------------------------------------------------------------
// Member function read_from_socket takes in a pointer to a character
// array as well as the number of bytes which are to be read from the
// socket.  It attempts to read in this many bytes from the socket
// into the character array buffer.  The number of bytes actually read
// from the socket is returned by this method.

int Socket::read_from_socket(
   char* char_buffer_ptr,int nbytes_requested) const
{
   int nbytes_actually_received = 
      ::read(m_sock,char_buffer_ptr,nbytes_requested);
	
   if (!nbytes_actually_received && nbytes_requested)
   {
      socket_closed_on_last_read = true;
      m_sock = -1;
   }
   
   return nbytes_actually_received;
}

// ---------------------------------------------------------------------
// Member function write_all_to_socket takes in a pointer to a
// character array containing at least nbytes_originally_requested
// bytes.  It repeatedly calls ::write until all of the requested
// bytes have been successfully exported to the socket.

void Socket::write_all_to_socket(
   const char* char_buffer_ptr,unsigned long nbytes_originally_requested) const
{
   unsigned long nbytes_requested=nbytes_originally_requested;
   unsigned long n=0;
   while (n<nbytes_requested)
   {
      int nbytes_actually_written = ::write(
         m_sock,char_buffer_ptr,nbytes_requested);
      
      n += nbytes_actually_written;
      char_buffer_ptr += nbytes_actually_written;
      nbytes_requested -= nbytes_actually_written;
   }
}

// ---------------------------------------------------------------------
// Member function read_all_from_socket takes in a pointer to a
// character array as well as the number of bytes which are to be read
// from the socket.  It repeatedly calls ::read until all of the
// requested bytes have been successfully imported from the socket.

void Socket::read_all_from_socket(
   char* char_buffer_ptr,unsigned long nbytes_originally_requested) const
{
   socket_closed_on_last_read = false;
   unsigned long nbytes_requested=nbytes_originally_requested;
   unsigned long n=0;
   while (n<nbytes_originally_requested)
   {
      int nbytes_actually_read = ::read(
         m_sock,char_buffer_ptr,nbytes_requested);

      if (!nbytes_actually_read)
      {
         socket_closed_on_last_read = true;
         m_sock = -1;
         return;
      }

      n += nbytes_actually_read;
      char_buffer_ptr += nbytes_actually_read;
      nbytes_requested -= nbytes_actually_read;
   }
}


bool Socket::connection_is_ready()
{
   fd_set fds;

   FD_ZERO(&fds);
   FD_SET(m_sock,&fds);

   // Wait for some input
   return select(m_sock+1, &fds, NULL, NULL, NULL ) > 0;
}


bool Socket::poll_connection(int sec, int usec)
{
   fd_set fds;

   FD_ZERO(&fds);
   FD_SET(m_sock,&fds);

   struct timeval tv;
   tv.tv_sec = sec;
   tv.tv_usec = usec;

   // Wait for some input
   return select(m_sock+1, &fds, NULL, NULL, &tv ) > 0;
} 

void Socket::disconnect()
{
   if (is_valid() ) ::close(m_sock);
}

bool Socket::can_be_read()
{
   if (socket_closed_on_last_read)
      return false;

   if (!is_valid())
      return false;

   int sr;
   struct timeval to;
   fd_set rread;
  
   FD_ZERO(&rread);      // clear the fd_set
   FD_SET(m_sock,&rread); // indicate that we want to 

   // check the m_sock socket only
   memset((char *)&to,0,sizeof(to)); // clear the timeval struct
   to.tv_sec=0;                // timeout select after 0 secs 
    
   // do the select.  
   // select returns > 0 if there is a read event on the socket 
   // e.g. data waiting to be read

   sr=select(m_sock+1, &rread, (fd_set *)0, (fd_set *)0, &to); 
   if (sr < 0) {
      perror("select");
      return false;
   }

   return (sr) && (FD_ISSET(m_sock, &rread));
};

