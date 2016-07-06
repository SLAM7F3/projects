// ==========================================================================
// Header file for WEBPAGE class
// ==========================================================================
// Last modified on 3/24/02
// ==========================================================================

#ifndef WEBPAGE_H
#define WEBPAGE_H

#include <stdlib.h>     // Needed for system() function to perform Unix calls
#include <fstream>
#include <iostream>
#include <iomanip>
#include <math.h>
#include <new>

// Note: We need to import the following library in order to access
// the default C++ string class:

#include <string>

#include "math/complex.h"
#include "genfuncs.h"
#include "numrec/nr.h"
#include "webform.h"
#include "webimage.h"
#include "weblink.h"
#include "webtable.h"

class webpage
{
  private:

  public:

   static const int NMAXMETALINES;
   static const int NMAXFORMS;
   static const int NMAXIMAGES;
   static const int NMAXLINKS;
   static const int NMAXTABLES;
   static const int BIGFONT;

   std::string filenamestr,title,titlecolor;
   std::string *metaline;
   std::string backgroundcolor,textcolor,linkcolor,vlinkcolor;
   std::ofstream webpagefilestream;
   std::ostream* webpagestream;
   
   webimage *form;
   webimage *image;
   weblink *link;
   webtable *table;
   
// ---------------------------------------------------------------------
// Constructor functions:
// ---------------------------------------------------------------------

   webpage(void);
   webpage(const webpage& w);
   ~webpage();

// ---------------------------------------------------------------------
// Member functions:
// ---------------------------------------------------------------------

   void docopy(const webpage& w);
   webpage operator= (const webpage w);


//   void openwebpagefile();
   void comment(std::string commentstr);
   void horizrule();
   void paragraph();
   void vertspace();

   void header();
   void startbody();
   std::string insertimage(webimage& i);
   std::string insertlink(weblink& l);
   void writeform(webform& f);
   void writetable(webtable& t);
   void writeparagraph(bool html_line_breaks,int vertspace,
                       std::string lines[],int nlines);
   void footer();
};

#endif // webpage.h




