/********************************************************************
 *
 *
 * Name: pa_struct.cc
 *
 *
 * Author: Joseph Adams
 *
 * Description:
 *
 *
 * --------------------------------------------------------------
 *    $Revision: 1.1.1.1 $
 * ---------------------------------------------------------------
 *
 *
 *
 **********************************************************************/
#pragma pack( push, enter_include1 )
#include "video/pa_struct.h"
#pragma pack( pop, enter_include1 )

// we will use a constructor to intialize some stuff
pa_struct::pa_struct(void)
{
   strncpy(endian,"IIII",4);
   strncpy(stop,"STOP",4);

   packet_size=sizeof(*this);
   checksum=0;
};

void pa_struct::ascii_dump(FILE *stream) const
{

   fprintf(stream,"\n");

   int i;

   // prints a line
   for (i=0; i<70; i++) fprintf(stream,"_");

   fprintf(stream,"\n");

   char c_endian[5]="XXXX";
   char c_stop[5]="XXXX";

   strncpy(c_endian,endian,4);
   strncpy(c_stop,stop,4);

   // add in the null terminator
   c_stop[4]='\0';
   c_endian[4]='\0';

   fprintf(stream,"\n\t     endian=\t%s",c_endian);
   fprintf(stream,"\n\tpacket_size=\t%d",packet_size);
   fprintf(stream,"\n\tpacket_counter=\t%d",packet_counter);
   fprintf(stream,"\n\tframe_count=\t%d",frame_count);

   fprintf(stream,"\n\t   sync_word=\t0x%04X",sync_word);

   fprintf(stream,"\n\t       stop=\t%s",c_stop);

   // prints a line
   for (i=0; i<70; i++) fprintf(stream,"_");

   fprintf(stream,"\n");

}
