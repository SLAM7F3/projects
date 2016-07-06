// ==========================================================================
// Header file for BUFFER class
// ==========================================================================
// Last modified on 3/19/04
// ==========================================================================

#ifndef BUFFER_H
#define BUFFER_H

#include "math/basic_math.h"

class Socket;

class buffer
{
  private:

   bool parsed_packet_header_flag;
   int buffer_size,physical_buffer_size;
   int packet_start_posn,packet_stop_posn;

   int advance_buffer_counter(int counter,int nbytes);
   void wrap_data_past_buffer_end_onto_beginning(int byte_posn);

  public:

   char* char_buffer_ptr;
   int ndata_bytes_in_buffer;
   int raw_data_end_posn;
   int curr_packet_ID,curr_packet_size;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:
// ---------------------------------------------------------------------

   void allocate_member_objects();
   void initialize_member_objects();
   buffer(void);
   buffer(int nbytes_size);
   ~buffer();

   void update_byte_array_counters(int nbytes);
   void parse_packet_header(bool& close_client_socket_flag);
   void copy_packet_data_to_shared_memory(
      int nbyte_to_copy,char* shared_memory_ptr);
   void display_byte_array_contents(char* byte_ptr,int nbytes);
};

// ==========================================================================
// Inlined methods
// ==========================================================================

inline int buffer::advance_buffer_counter(int counter,int nbytes)
{
   return modulo(counter+nbytes,buffer_size);
}


#endif // buffer.h

