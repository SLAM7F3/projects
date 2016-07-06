/*	Standard Debugging include, TW 1-4-88 inspired by Ward,
	"Debugging 'C'."  Define DEBUG externally for this to
	take effect.

	Typical use is:

		TRACE(3,(DF,"getdata: count=%d\n",count));

	NB the parentheses around the second argument to TRACE!

	-rw-r--r--   1 tw             442 Oct 30  1990 debug.h
*/

#ifndef DF
#define DF stderr	/* DF is the Debug File */

#ifdef DEBUG
// EXTERNAL int debug;
#define TRACE(level,pargs) 
// #define TRACE(level,pargs) if (debug>=level) fprintf pargs
#else
#define TRACE(level,pargs)
#endif

#endif
