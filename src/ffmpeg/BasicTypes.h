// ========================================================================
// BasicTypes header
// ========================================================================
// Last updated on 3/21/12
// ========================================================================

#ifndef UL_BASIC_TYPES_H
#define UL_BASIC_TYPES_H

#include <cstdio>

// Apparently Visual Studio ignores the #ifndef anyway
/*
#ifndef byte
typedef char byte;
#endif
*/

#ifndef ubyte
typedef unsigned char		ubyte;
#endif

#ifndef ubyte
typedef unsigned int		uint;
#endif

#ifndef ushort
typedef unsigned short		ushort;
#endif

/*
// As of 3/21/2012, we've found that the following int64 typedef
// conflict with analogous ones in OpenCV...

#ifndef int64
 typedef long long			int64;
//typedef int64_t int64
#endif
*/

// uint64 doesn't appear to be used anywhere within ffmpeg subdir!

/*
#ifndef uint64
typedef unsigned long long	uint64;
// typedef uint64_t uint64
#endif
*/

#endif
