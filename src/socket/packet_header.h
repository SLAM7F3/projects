// ==========================================================================
// Header file for PACKET_HEADER class
// ==========================================================================
// Last modified on 5/21/04
// ==========================================================================

#ifndef PACKET_HEADER_H
#define PACKET_HEADER_H

enum PacketType { eOSGGroupIVE = 0, eOSGGroupOSG, eOSGGroupNET };
enum OperationType { eAdd = 0, eDelete };

struct packet_header
{
      unsigned int n_data_bytes;
      unsigned int data_type;
      char name[25];
      unsigned int operation;
      
};

#endif // packet_header.h

