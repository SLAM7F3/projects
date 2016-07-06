// ==========================================================================
// Header file for kmeans namespace functions
// ==========================================================================
// Last updated on 8/25/13
// ==========================================================================

#ifndef KMEANSFUNCS_H
#define KMEANSFUNCS_H

namespace kmeansfunc
{
   void copyright (void);
   void LOG(int level, const char *fmt, ...);
   void show_params (void);
   void status (int signum);
   void run (void);
   void stop (int signum);
   size_t strtos (const char *str);
   void usage (void);
}

#endif  // cluster/kmeansfuncs.h





