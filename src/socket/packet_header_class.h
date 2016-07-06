// ==========================================================================
// Header file for PACKET_HEADER class
// ==========================================================================
// Last modified on 3/18/04
// ==========================================================================

#ifndef PACKET_HEADER_H
#define PACKET_HEADER_H

class packet_header
{
  public:

   int packet_ID;
   int packet_size_in_bytes;
   bool close_socket;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:
// ---------------------------------------------------------------------

   packet_header(int ID,int packet_size,bool close_socket_flag);
   ~packet_header();
};

#endif // packet_header.h

