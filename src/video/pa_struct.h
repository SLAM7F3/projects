/********************************************************************
 *
 *
 * Name: pa_struct.h
 *
 *
 * Author: Joe Adams
 *
 * Description:
 *    c struct to describe the entire PA (platform attitude) packet
 *
 *
 *
 **********************************************************************/

#ifndef _jsa_pa_structs_
#define _jsa_pa_structs_

#include <stdio.h>
#include <string.h>

#include "video/data_types.h"
#include "video/scb_struct.h"

#pragma pack(4)

#// pragma pack( push, 4 )

struct pa_struct
{
      gchar8 endian[4];
      guint16 packet_size;                          // size of entire packet
      guint16 packet_counter;                       // packet counter

      guint32 frame_count;                          // counter on laser pulses

      scb_struct scb;	                    // scan control struct
//      scb_struct_v1 scb;	                    // scan control struct

      guint16 irig_days_bcd;                        // Binary Coded Decimal 
						    // IRIG day
      guint32 irig_useconds_20_bit;
      guint32 irig_seconds_17_bit;                  // Seconds since midnight

      guint32   sync_word;                          // should always be 0xA5A5
      guint16   checksum;
      gchar8    stop[4];

      pa_struct(void);
      void ascii_dump(FILE *stream=stdout) const;

};

#pragma pack( pop )

#endif
