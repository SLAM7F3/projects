// ==========================================================================
// Header file for WEBIMAGE class
// ==========================================================================
// Last modified on 3/24/02
// ==========================================================================

#ifndef WEBIMAGE_H
#define WEBIMAGE_H

#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <math.h>
#include <string>

#include "genfuncs.h"

class webimage
{
  private:

  public:

   static const int NMAXCOLORS;

   int border,height,width,ncolors;
   int *R;
   int *G;
   int *B;
   std::string horizposn,src;
   std::string *colorname;
   
// ---------------------------------------------------------------------
// Constructor functions:
// ---------------------------------------------------------------------

   webimage(void);
   webimage(const webimage& currimage);
   ~webimage();

// ---------------------------------------------------------------------
// Member functions:
// ---------------------------------------------------------------------

   void docopy(const webimage& currimage);
   webimage operator= (const webimage currimage);

   void initialize_RGBtable();
   bool find_RGB(std::string colorstring,int& r,int& g,int& b);
};

#endif // webimage.h

