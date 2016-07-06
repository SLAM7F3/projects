// ==========================================================================
// Header file for character functions
// ==========================================================================
// Last modified on 7/25/04
// ==========================================================================

#ifndef CHARACTERFUNCS_H
#define CHARACTERFUNCS_H

#include <vector>
class character;

namespace characterfunc
{
   character* generate_ascii_characters();
   void generate_numeral_characters(character* ascii_char);
   void generate_letter_characters(character* ascii_char);
   void generate_special_characters(character* ascii_char);
}

#endif // characterfuncs.h



