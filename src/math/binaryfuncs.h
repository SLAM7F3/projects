// ==========================================================================
// Header file for stand-alone binary math functions 
// ==========================================================================
// Last updated on 4/28/13; 4/29/13; 5/31/13
// ==========================================================================

#ifndef BINARYFUNCS_H
#define BINARYFUNCS_H

#include <sstream>	// Needed for using string streams
#include <string>

class descriptor;
class genvector;

namespace binaryfunc
{
   unsigned long binary_string_to_eight_byte_integer(std::string binNum);
   std::string eight_byte_integer_to_binary_string(unsigned long long_form);
   void eight_byte_integer_to_binary(
      unsigned long long_form,std::stringstream& ss);
   std::string genvector_to_string(genvector* x_ptr);
   genvector* string_to_genvector(std::string binNum);
   descriptor* string_to_descriptor(std::string binNum);

// Hamming distance methods

   unsigned long hamming_distance(unsigned long x,unsigned long y);
   unsigned long hamming_distance(
      std::string& binary_str1,std::string& binary_str2);
   unsigned long hamming_distance(genvector* x_ptr,genvector* y_ptr);
   unsigned long hamming_distance(descriptor* x_ptr,descriptor* y_ptr);
}

#endif  // binaryfunc namespace

