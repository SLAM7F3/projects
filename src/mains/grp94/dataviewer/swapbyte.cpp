/********************************************************************
 *
 *
 * Name: swapbyte.cpp
 *
 *
 *
 * Description:
 * byte swapping routines for endian-ness
 *
 * --------------------------------------------------------------
 *    $Revision: 1.2 $
 * ---------------------------------------------------------------
 * $log:
 *
 */
#include "swapbyte.h"
#include <string>  // for memcpy
#include <cstdio>

/*#ifdef __APPLE__ 
#else 
#include <malloc.h>	// for malloc 
#endif */

char byte_swap_temp;
char * byte_swap_address;
void swap_2(void *tni2)                    /* 2 byte   */
{
   byte_swap_address=(char*)tni2;
   byte_swap_temp=byte_swap_address[0];
   byte_swap_address[0]=byte_swap_address[1];
   byte_swap_address[1]=byte_swap_temp;
}

void swap_4(void *tni4)                        /* 4 byte   */
{
   byte_swap_address=(char*)tni4;
   byte_swap_temp=byte_swap_address[0];
   byte_swap_address[0]=byte_swap_address[3];
   byte_swap_address[3]=byte_swap_temp;
   byte_swap_temp=byte_swap_address[1];
   byte_swap_address[1]=byte_swap_address[2];
   byte_swap_address[2]=byte_swap_temp;
}
void swap_8(void *tni8)                        /* 8 byte   */
{
   byte_swap_address=(char*)tni8;
   byte_swap_temp=byte_swap_address[0];
   byte_swap_address[0]=byte_swap_address[7];
   byte_swap_address[7]=byte_swap_temp;
   byte_swap_temp=byte_swap_address[1];
   byte_swap_address[1]=byte_swap_address[6];
   byte_swap_address[6]=byte_swap_temp;
   byte_swap_temp=byte_swap_address[2];
   byte_swap_address[2]=byte_swap_address[5];
   byte_swap_address[5]=byte_swap_temp;
   byte_swap_temp=byte_swap_address[3];
   byte_swap_address[3]=byte_swap_address[4];
   byte_swap_address[4]=byte_swap_temp;
}

void little_endian_fread(void* addr, size_t size, size_t count, FILE* fp){
   fread(addr,size,count,fp);
#if defined (__BIG_ENDIAN__)
   byte_swap(addr,size,count);
#endif
}

void big_endian_fread(void* addr, size_t size, size_t count, FILE* fp){
   fread(addr,size,count,fp);
#if defined (__LITTLE_ENDIAN__)
   byte_swap(addr,size,count);
#endif
}

void little_endian_fwrite(const void* addr, size_t size, size_t count, FILE* fp){
#if defined (__BIG_ENDIAN__)
   void * temp=malloc(size*count);
   memcpy(temp,addr,size*count);
   byte_swap(temp,size,count);
   fwrite(temp,size,count,fp);
   free(temp);
#else
   fwrite(addr,size,count,fp);
#endif
}

void big_endian_fwrite(const void* addr, size_t size, size_t count, FILE* fp){
#if defined (__LITTLE_ENDIAN__)
   void * temp=malloc(size*count);
   memcpy(temp,addr,size*count);
   byte_swap(temp,size,count);
   fwrite(temp,size,count,fp);
   free(temp);
#else
   fwrite(addr,size,count,fp);
#endif
}

void byte_swap(void * addr, size_t size){
   unsigned int i;
   byte_swap_address=(char*)addr;
   for(i=0;i<size/2;i++){
      byte_swap_temp=byte_swap_address[i];
      byte_swap_address[i]=byte_swap_address[size-i-1];
      byte_swap_address[size-i-1]=byte_swap_temp;
   }
}

void byte_swap(void * addr, size_t size, size_t count){
   unsigned int i;
   for(i=0;i<count;i++){
      byte_swap(((char*)addr)+i*size,size);
   }
}

void swap_short_2(short *tni2)                    /* 2 byte signed integers   */
{
   *tni2=(((*tni2>>8)&0xff) | ((*tni2&0xff)<<8));
}

void swap_u_short_2(unsigned short *tni2)         /* 2 byte unsigned integers */
{
   *tni2=(((*tni2>>8)&0xff) | ((*tni2&0xff)<<8));
}

void swap_int_4(int *tni4)                        /* 4 byte signed integers   */
{
   *tni4=(((*tni4>>24)&0xff) | ((*tni4&0xff)<<24) |
          ((*tni4>>8)&0xff00) | ((*tni4&0xff00)<<8));
}

void swap_u_int_4(unsigned int *tni4)             /* 4 byte unsigned integers */
{
   *tni4=(((*tni4>>24)&0xff) | ((*tni4&0xff)<<24) |
          ((*tni4>>8)&0xff00) | ((*tni4&0xff00)<<8));
}

void swap_long_4(long *tni4)                      /* 4 byte signed long integers */
{
   *tni4=(((*tni4>>24)&0xff) | ((*tni4&0xff)<<24) |
          ((*tni4>>8)&0xff00) | ((*tni4&0xff00)<<8));
}


void swap_u_long_4(unsigned long *tni4)           /* 4 byte unsigned long integers */
{
   *tni4=(((*tni4>>24)&0xff) | ((*tni4&0xff)<<24) |
          ((*tni4>>8)&0xff00) | ((*tni4&0xff00)<<8));
}


void swap_float_4(float *tnf4)                    /* 4 byte floating point numbers */
{
   int *tni4=(int *)tnf4;
   *tni4=(((*tni4>>24)&0xff) | ((*tni4&0xff)<<24) |
          ((*tni4>>8)&0xff00) | ((*tni4&0xff00)<<8));
}


void swap_double_8(double *tndd8)                 /* 8 byte double numbers          */
{
   char *tnd8=(char *)tndd8;
   char tnc;

   tnc=*tnd8;
   *tnd8=*(tnd8+7);
   *(tnd8+7)=tnc;

   tnc=*(tnd8+1);
   *(tnd8+1)=*(tnd8+6);
   *(tnd8+6)=tnc;

   tnc=*(tnd8+2);
   *(tnd8+2)=*(tnd8+5);
   *(tnd8+5)=tnc;

   tnc=*(tnd8+3);
   *(tnd8+3)=*(tnd8+4);
   *(tnd8+4)=tnc;
}
