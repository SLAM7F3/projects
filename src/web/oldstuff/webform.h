// ==========================================================================
// Header file for WEBFORM class
// ==========================================================================
// Last modified on 6/26/00
// ==========================================================================

#ifndef WEBFORM_H
#define WEBFORM_H

#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <math.h>
#include <string>

#include "genfuncs.h"
#include "webtable.h"

class webform
{
  private:

  public:

   std::string method;
   std::string action;
   webtable formtable;

// ---------------------------------------------------------------------
// Constructor functions:
// ---------------------------------------------------------------------

   webform(void);
   webform(const webform& f);
   ~webform();

// ---------------------------------------------------------------------
// Member functions:
// ---------------------------------------------------------------------

   void docopy(const webform& f);
   webform operator= (const webform f);
};

#endif // webform.h




