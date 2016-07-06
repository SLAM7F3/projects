/********************************************************************
 *
 *
 * Name: swapbyte.h
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


#ifndef _jsa_swapbyte_
#define _jsa_swapbyte_
#include <cstdio>
//#include "arch_dependent_headers.h"


void swap_short_2(short *tni2);                   /* 2 byte signed integers   */
void swap_u_short_2(unsigned short *tni2);        /* 2 byte unsigned integers */
void swap_int_4(int *tni4);                       /* 4 byte signed integers   */
void swap_u_int_4(unsigned int *tni4);            /* 4 byte unsigned integers */
void swap_long_4(long *tni4);                     /* 4 byte signed long integers */
void swap_u_long_4(unsigned long *tni4);          /* 4 byte unsigned long integers */
void swap_float_4(float *tnf4);                   /* 4 byte floating point numbers */
void swap_double_8(double *tndd8);                /* 8 byte double numbers          */
void swap_2(void *tni2);							/* 2 byte  swapping */
void swap_4(void *tni4);							/* 4 byte  swapping */
void swap_8(void *tni8);							/* 8 byte swapping */
void byte_swap(void * addr, size_t size, size_t count);
void byte_swap(void * addr, size_t size);			/* generic swapping */
void little_endian_fread(void* addr, size_t size, size_t count, FILE* fp);
void big_endian_fread(void* addr, size_t size, size_t count, FILE* fp);
void little_endian_fwrite(const void* addr, size_t size, size_t count, FILE* fp);
void big_endian_fwrite(const void* addr, size_t size, size_t count, FILE* fp);
#endif
