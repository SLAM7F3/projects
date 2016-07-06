// ==========================================================================
// Buffer class member function definitions 
// ==========================================================================
// Last modified on 3/19/04
// ==========================================================================

#include <iostream>
#include <string>
#include "math/basic_math.h"
#include "ipc/buffer.h"
#include "general/outputfuncs.h"
#include "ipc/packet_header.h"
#include "ipc/Socket.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::string;
using std::ostream;
using std::cout;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:
// ---------------------------------------------------------------------

void buffer::allocate_member_objects() 
{

// We create a ring buffer data structure from an ordinary linear
// array rather than from some other more complicated and potentially
// expensive data structure such as a looped linked list.  Since we do
// not a priori know the size of data packets before it is stored
// within the ring buffer, we must allow for some slop space at the
// end of the linear array to temporarily hold data packets as they
// are read in from a socket.  The total PHYSICAL size of the linear
// array must consequenty exceed the ring buffer's WORKING size.  Any
// data temporarily lying beyond relative address location buffer_size
// must be wrapped around within the ring buffer onto relative address
// location zero.  

// Give linear array 1 Mbyte slop space beyond ring buffer's maximum
// working size:

   physical_buffer_size=buffer_size+1000000;	
   char_buffer_ptr=new char[physical_buffer_size];
   memset(char_buffer_ptr,0,physical_buffer_size);
}

void buffer::initialize_member_objects() 
{
   parsed_packet_header_flag=false;
   packet_start_posn=packet_stop_posn=0;
   raw_data_end_posn=0;
   ndata_bytes_in_buffer=0;
   curr_packet_ID=curr_packet_size=-1;
}

buffer::buffer(void)
{
   buffer_size=1000000;		// Default buffer capacity = 1 Mbyte 
   allocate_member_objects();
   initialize_member_objects();
}

buffer::buffer(int nbytes_size)
{
   buffer_size=nbytes_size;
   allocate_member_objects();
   initialize_member_objects();
}

buffer::~buffer()
{
   delete char_buffer_ptr;
}

// --------------------------------------------------------------------------
void buffer::update_byte_array_counters(int nbytes)
{
   ndata_bytes_in_buffer += nbytes;

   raw_data_end_posn += nbytes;
   if (raw_data_end_posn >= buffer_size)
   {
      wrap_data_past_buffer_end_onto_beginning(raw_data_end_posn);
      raw_data_end_posn -= buffer_size;
   }

//   cout << "Buffer = " << endl;
//   display_byte_array_contents(char_buffer_ptr,buffer_size);
//   outputfunc::newline();
}

// --------------------------------------------------------------------------
void buffer::wrap_data_past_buffer_end_onto_beginning(int byte_posn)
{
   int trailing_portion_size=byte_posn-buffer_size;
   memcpy(char_buffer_ptr,char_buffer_ptr+buffer_size,trailing_portion_size);
}

// --------------------------------------------------------------------------
// Member function parse_packet_header reads the contents of packet
// header bytes within the ring buffer array and extracts out the
// packet's ID number as well as the size in bytes of the packet's
// data.  

void buffer::parse_packet_header(bool& close_socket_flag)
{
   int packet_header_size=sizeof(packet_header);
   if (!parsed_packet_header_flag && 
       ndata_bytes_in_buffer >= packet_header_size)
   {
      packet_header const *header_ptr=
         reinterpret_cast<packet_header*>(char_buffer_ptr+packet_start_posn);
      curr_packet_ID=header_ptr->packet_ID;
      curr_packet_size=header_ptr->packet_size_in_bytes;
      close_socket_flag=header_ptr->close_socket;
      if (curr_packet_ID%1000==0)
      {
         cout << "curr_packet_ID = " << curr_packet_ID << endl;
         cout << "curr_packet_size = " << curr_packet_size << endl;
         cout << "close_socket_flag = " << close_socket_flag << endl;
         outputfunc::newline();
      }
      
      ndata_bytes_in_buffer -= packet_header_size;
      curr_packet_size -= packet_header_size;

      packet_start_posn=advance_buffer_counter(
         packet_start_posn,packet_header_size);
      packet_stop_posn=advance_buffer_counter(
         packet_start_posn,curr_packet_size);

      parsed_packet_header_flag=true;
   }
}

// --------------------------------------------------------------------------
// Member function copy_packet_data_to_shared_memory moves packet data
// from the temporary ring buffer into shared memory.  It returns the
// numbers of copied bytes.

void buffer::copy_packet_data_to_shared_memory(
   int nbytes_to_copy,char* shared_memory_ptr)
{
   if (packet_stop_posn > packet_start_posn)
   {
      memcpy(shared_memory_ptr,char_buffer_ptr+packet_start_posn,
             nbytes_to_copy);
   }
   else
   {
      int beginning_portion_size=buffer_size-packet_start_posn;
      int ending_portion_size=nbytes_to_copy-beginning_portion_size;
      memcpy(shared_memory_ptr,char_buffer_ptr+packet_start_posn,
             beginning_portion_size);
      memcpy(shared_memory_ptr+beginning_portion_size,char_buffer_ptr,
             ending_portion_size);
   }

   packet_start_posn=advance_buffer_counter(
      packet_start_posn,nbytes_to_copy);
   ndata_bytes_in_buffer -= nbytes_to_copy;

// Reset parsed_packet_header_flag back to false for next data packet

   parsed_packet_header_flag=false;   
}

// --------------------------------------------------------------------------
// Member function display_byte_array_contents takes in a character
// array along with an integer nbytes parameter.  For bytes 1 through
// nbytes, this method displays their bit representations.

void buffer::display_byte_array_contents(char* byte_ptr,int nbytes)
{
   cout << "inside buffer::display_byte_array_contents()" << endl;
   for (int n=0; n<nbytes; n++)
   {
      cout << "byte " << n << endl;
      stringfunc::display_byte_bits_rep(byte_ptr[n]);
      outputfunc::newline();
   }
}





