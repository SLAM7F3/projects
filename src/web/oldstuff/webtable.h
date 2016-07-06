// ==========================================================================
// Header file for WEBTABLE class
// ==========================================================================
// Last modified on 7/4/02
// ==========================================================================

#ifndef WEBTABLE_H
#define WEBTABLE_H

#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <math.h>
#include <string>

#include "genfuncs.h"

class webtable
{
  private:

  public:

   static const int NMAXROWS=320;
   static const int NMAXCOLUMNS=10;

// In order to achieve a consistently larger [smaller] font for
// entries within tables, we set their sizes equal to BIGFONT
// [SMALLFONT]:

   static const int BIGFONT;
   static const int SMALLFONT;	

   bool border;
   int cellspacing,cellpadding,nrows,ncolumns,width;
   std::string table_alignment,bgcolor;
   std::string caption,caption_style,caption_fontcolor;
   int caption_fontsize;

   std::string (*entry_alignment)[NMAXCOLUMNS];
   std::string (*entry_valignment)[NMAXCOLUMNS];
   int (*entry_fontsize)[NMAXCOLUMNS];
   std::string (*entry_fontcolor)[NMAXCOLUMNS];
   std::string (*entry_style)[NMAXCOLUMNS];
   int (*entry_colspan)[NMAXCOLUMNS];
   std::string (*entry)[NMAXCOLUMNS];

// ---------------------------------------------------------------------
// Constructor functions:
// ---------------------------------------------------------------------

   webtable(void);
   webtable(const webtable& t);
   ~webtable();

// ---------------------------------------------------------------------
// Member functions:
// ---------------------------------------------------------------------

   void docopy(const webtable& t);
   webtable operator= (const webtable t);

};

#endif // webtable.h




