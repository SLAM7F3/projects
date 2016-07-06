#ifndef _jsa_typedefs_
#define _jsa_typedefs_

#ifndef GLIB_MAJOR_VERSION
typedef char        gchar8;
typedef signed char     gschar8;
typedef unsigned char   guchar8;
typedef signed char     gint8;
typedef unsigned char   guint8;
typedef signed short    gint16;
typedef unsigned short  guint16;
typedef signed int      gint32;
typedef unsigned int    guint32;
//  these are already defined (ints are the same as longs)
//  typedef long                gint32;
//  typedef unsigned long       guint32;

// typedef long long		gint64;
//    typedef unsigned long long	guint64;

typedef float       gfloat32;
typedef double      gdouble64;
typedef long double     gdouble96;

typedef gchar8      gchar;
typedef gfloat32        gfloat;
typedef gdouble64       gdouble;
#endif
#endif
