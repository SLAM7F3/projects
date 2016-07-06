// ==========================================================================
// Packet_Header class member function definitions 
// ==========================================================================
// Last modified on 3/18/04
// ==========================================================================

#include <iostream>
#include "packet_header.h"

using std::cout;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:
// ---------------------------------------------------------------------

packet_header::packet_header(int ID,int packet_size,bool close_socket_flag)
{
   packet_ID=ID;
   packet_size_in_bytes=packet_size;
   close_socket=close_socket_flag;
}

packet_header::~packet_header()
{
}





