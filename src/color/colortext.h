// ==========================================================================
// Header file for colortext class
// ==========================================================================
// Last modified on 4/9/14
// ==========================================================================

// See http://stackoverflow.com/questions/2616906/how-do-i-output-coloured-text-to-a-linux-terminal

#ifndef COLORTEXT_H
#define COLORTEXT_H

#include <iostream>

namespace Color {
   enum Code {
      FG_RED      = 91,
      FG_GREEN    = 92,
      FG_YELLOW   = 93,
      FG_ORANGE   = 33,
      FG_BLUE     = 34,
      FG_PURPLE   = 95,
      FG_CYAN     = 96,
      FG_GREY     = 37,
      FG_BLACK    = 30,
      FG_WHITE    = 97,
      FG_DEFAULT  = 39,

      BG_RED      = 101,
      BG_GREEN    = 102,
      BG_YELLOW   = 103,
      BG_ORANGE   = 43,
      BG_BLUE     = 44,
      BG_PURPLE   = 105,
      BG_CYAN     = 106,
      BG_GREY    =  47,
      BG_WHITE    = 107,
      BG_BLACK    = 40,
      BG_DEFAULT  = 49

   };
   class Modifier {
      Code code;
     public:
     Modifier(Code pCode) : code(pCode) {}
      friend std::ostream&
         operator<<(std::ostream& os, const Modifier& mod) {
         return os << "\033[" << mod.code << "m";
      }
   };
}

#endif  // colortext.h



