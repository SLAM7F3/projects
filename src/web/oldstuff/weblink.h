// ==========================================================================
// Header file for WEBLINK class
// ==========================================================================
// Last modified on 6/21/00
// ==========================================================================

#ifndef WEBLINK_H
#define WEBLINK_H

#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <math.h>
#include <string>

#include "genfuncs.h"

class weblink
{
  private:

  public:

   std::string style;
   std::string url,descriptor;

// ---------------------------------------------------------------------
// Constructor functions:
// ---------------------------------------------------------------------

   weblink(void);
   weblink(const weblink& l);
   ~weblink();

// ---------------------------------------------------------------------
// Member functions:
// ---------------------------------------------------------------------

   void docopy(const weblink& l);
   weblink operator= (const weblink l);
};

#endif // weblink.h




