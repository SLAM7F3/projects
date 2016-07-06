#ifndef _jsa_arch_deps_
#define _jsa_arch_deps_

#ifdef __unix__
	#define INCLUDE_GL #include <GL/gl.h>
	#define INCLUDE_GLU #include <GL/glu.h>
#endif

/*********************************************************************
 *
 *                Linux
 *
 *
 **********************************************************************/

#if defined(__linux__) || __unix__ || __UNIX__ || __LINUX__

  // GCC under linux
  #if defined(__GNUC__)
	#include <GL/gl.h>
	#include <GL/glu.h>

     // define the byte order
	#if(__POWERPC__==1)
		#define __BIG_ENDIAN__
	#else
		#define __LITTLE_ENDIAN__
	#endif
	#define PI 3.141509
  #endif // end of gcc
#endif // end of linux

/*********************************************************************
 *
 *               OS X
 *
 *
 **********************************************************************/


/* os X thinks that it is 
   mach, __APPPLE__ or (_Mac_OS_X_Executable_)
*/
#if defined (__APPLE__)
	#include "OpenGL/gl.h" 
	#include "OpenGL/glu.h" 

#define __BIG_ENDIAN__
#endif


/*********************************************************************
 *
 *               Win32
 *
 *
 **********************************************************************/

#if defined WIN32 || _WIN32


/* RIP FROM glut.h */

/* GLUT 3.7 now tries to avoid including <windows.h>
   to avoid name space pollution, but Win32's <GL/gl.h> 
   needs APIENTRY and WINGDIAPI defined properly. */
# if 0
#  define  WIN32_LEAN_AND_MEAN
#  include <windows.h>
# else
   /* XXX This is from Win32's <windef.h> */
#  ifndef APIENTRY
//#   define GLUT_APIENTRY_DEFINED
#   if (_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED)
#    define APIENTRY    __stdcall
#   else
#    define APIENTRY
#   endif
#  endif
   /* XXX This is from Win32's <winnt.h> */
#  ifndef CALLBACK
#   if (defined(_M_MRX000) || defined(_M_IX86) || defined(_M_ALPHA) || defined(_M_PPC)) && !defined(MIDL_PASS)
#    define CALLBACK __stdcall
#   else
#    define CALLBACK
#   endif
#  endif
   /* XXX This is from Win32's <wingdi.h> and <winnt.h> */
#  ifndef WINGDIAPI
//#   define GLUT_WINGDIAPI_DEFINED
#   define WINGDIAPI __declspec(dllimport)
#  endif
   /* XXX This is from Win32's <ctype.h> */
#  ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#   define _WCHAR_T_DEFINED
#  endif
# endif

#pragma comment (lib, "winmm.lib")     /* link with Windows MultiMedia lib */
#pragma comment (lib, "opengl32.lib")  /* link with Microsoft OpenGL lib */
#pragma comment (lib, "glu32.lib")     /* link with OpenGL Utility lib */
//#pragma comment (lib, "glut32.lib")    /* link with Win32 GLUT lib */

#pragma warning (disable:4244)	/* Disable bogus conversion warnings. */
#pragma warning (disable:4305)  /* VC++ 5.0 version of above warning. */

#endif

#include <GL/gl.h>
#include <GL/glu.h>

/* END RIP FROM glut.h*/

#define PI 3.141509
#define __LITTLE_ENDIAN__
 


#endif
