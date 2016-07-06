/*
 *  config.h
 *  data_viewer
 *
 *  Created by Ross Anderson on 12/27/05.
 *  Copyright 2005 MIT Lincoln Laboratory. All rights reserved.
 *
 */
#ifndef __CONFIG_H__
#define __CONFIG_H__ 1

#if defined( _WIN32 )
#define DIRECTORY_SEPARATOR '\\'

#include <windows.h>
#undef min
#undef max
	
// define some symbols that exist by default in gcc
#include <assert.h>
#include <float.h>
#include <xmath.h>

// define standard types
typedef __int8  int8_t;
typedef __int16 int16_t;
typedef __int32 int32_t;
typedef __int64 int64_t;
typedef unsigned __int8  uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;

	// disable some common and unnecessary warnings
#pragma warning(disable:4100)	// unreferenced formal parameter
#else
#define DIRECTORY_SEPARATOR '/'
#endif


#endif
