// =========================================================================
// Header file for _SYSTEMTIME structure
// =========================================================================
// Last modified on 2/24/04
// =========================================================================

#ifndef SYSTEMTIMEHEADER
#define SYSTEMTIMEHEADER

#ifndef _WINDEF_
typedef short WORD;
#endif

#ifndef _WINBASE_
typedef struct _SYSTEMTIME 
{ 
      WORD wYear; 
      WORD wMonth; 
      WORD wDayOfWeek; 
      WORD wDay; 
      WORD wHour; 
      WORD wMinute; 
      WORD wSecond; 
      WORD wMilliseconds; 
} SYSTEMTIME, *PSYSTEMTIME;
#endif

#endif // SYSTEMTIMEHEADER
