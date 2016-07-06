// ==========================================================================
// Header file for PACKET_HEADER structure
// ==========================================================================
// Last modified on 3/17/04
// ==========================================================================

#ifndef PACKET_HEADER_H
#define PACKET_HEADER_H

struct packet_header
{
      int packet_ID;
      int nbytes;
      bool close_socket;
};

#endif // packet_header.h

