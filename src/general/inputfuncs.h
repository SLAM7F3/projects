// =========================================================================
// Header file for stand-alone input functions.
// =========================================================================
// Last modified on 8/15/05; 5/16/06; 7/30/06
// =========================================================================

#ifndef INPUTFUNCS_H
#define INPUTFUNCS_H

#include <iostream>
#include <string>
#include <vector>
#include "math/threevector.h"

namespace inputfunc
{
   int enter_nonnegative_integer(std::string label_command);
   std::string enter_string(std::string label_command);
   threevector enter_threevector(std::string label_command);

/*
   std::vector<double> read_from_text_line(
      int n_doubles,const std::string& line);
   
// --------------------------------------------------------------------------
// Method read_from_text_line takes an input string along with some
// number of inputs to read from that string.  It returns an STL
// vector whose entries correspond to the parsed inputs.

   template <class T> inline std::vector<T> read_from_text_line(
      int n_inputs,const std::string& line)
      {
         std::istringstream input_string_stream(line);

         std::vector<T> V;
         T curr_input;
         for (int n=0; n<n_inputs; n++)
         {
            input_string_stream >> curr_input;
            V.push_back(curr_input);
         }
         return V;
      }

*/

}

#endif // general/inputfuncs.h




