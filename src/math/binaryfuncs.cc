// ==========================================================================
// Binary math functions 
// ==========================================================================
// Last updated on 4/28/13; 4/29/13; 5/4/13; 5/31/13; 6/7/14
// ==========================================================================

#include <bitset>
#include <iostream>
#include "math/binaryfuncs.h"
#include "datastructures/descriptor.h"
#include "math/genvector.h"

using std::bitset;
using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::stringstream;

namespace binaryfunc
{

// Method binary_string_to_eight_byte_integer() takes in a string such
// as "0101" and returns its 64-bit unsigned integer equivalent 5.

   unsigned long binary_string_to_eight_byte_integer(string binary_str)
   {
//      cout << "sizeof = " << sizeof(bitset<64>(binNum).to_ulong())
//           << endl;
//      unsigned long long_form=bitset<64>(binNum).to_ulong();
//      return long_form;

      return bitset<64>(binary_str).to_ulong();
   }

// ---------------------------------------------------------------------
// Method eight_byte_integer_to_binary_string() takes in a 64-bit
// integer such as 5 and returns its binary string equivalent "101".

   string eight_byte_integer_to_binary_string(unsigned long long_form)
   {
      stringstream ss;
      binaryfunc::eight_byte_integer_to_binary(long_form,ss);

      char buffer[64];
      ss >> buffer;
      string binary_str=buffer;
      return binary_str;
   }

   void eight_byte_integer_to_binary(
      unsigned long long_form,stringstream& ss)
   {
      if (long_form <= 1)
      {
         ss << long_form;
         return;
      }
      unsigned long remainder=long_form%2;
      eight_byte_integer_to_binary(long_form >> 1,ss);
      ss << remainder;
   }
   
// ---------------------------------------------------------------------
// Method genvector_to_string() takes in a genvector *x_ptr whose
// entries are assumed to equal 0 or 1.  It returns a string
// corresponding to the input binary quantized genvector.

   string genvector_to_string(genvector* x_ptr)
   {
      string x_str="";
      for (unsigned int d=0; d<x_ptr->get_mdim(); d++)
      {
         if (x_ptr->get(d)==0)
         {
            x_str += "0";
         }
         else
         {
            x_str += "1";
         }
      }
      return x_str;
   }

   string descriptor_to_string(descriptor* x_ptr)
   {
      string x_str="";
      for (unsigned int d=0; d<x_ptr->get_mdim(); d++)
      {
         if (x_ptr->get(d)==0)
         {
            x_str += "0";
         }
         else
         {
            x_str += "1";
         }
      }
      return x_str;
   }
   
// ---------------------------------------------------------------------
// Method string_to_genvector() takes in a string whose characters
// are assumed to equal 0 or 1.  It instantiates and returns a genvector
// that corresponds to the input binary quantized string.

   genvector* string_to_genvector(string binNum)
   {
      int n_chars=binNum.size();
      genvector* genvector_ptr=new genvector(n_chars);
      for (int s=0; s<n_chars; s++)
      {
         if (binNum.substr(s,1)=="1")
         {
            genvector_ptr->put(s,1);
         }
         else if (binNum.substr(s,1)=="0")
         {
            genvector_ptr->put(s,0);
         }
         else
         {
            cout << "Error!" << endl;
            cout << "binNum = " << binNum << endl;
            exit(-1);
         }
      } // loop over index s labeling binNum characters

      return genvector_ptr;
   }

   descriptor* string_to_descriptor(string binNum)
   {
      int n_chars=binNum.size();
      descriptor* descriptor_ptr=new descriptor(n_chars);
      for (int s=0; s<n_chars; s++)
      {
         if (binNum.substr(s,1)=="1")
         {
            descriptor_ptr->put(s,1);
         }
         else if (binNum.substr(s,1)=="0")
         {
            descriptor_ptr->put(s,0);
         }
         else
         {
            cout << "Error!" << endl;
            cout << "binNum = " << binNum << endl;
            exit(-1);
         }
      } // loop over index s labeling binNum characters

      return descriptor_ptr;
   }
 
// ==========================================================================
// Hamming distance methods
// ==========================================================================

// Note added on 5/4/13.  This "hamming_distance()" method is
// deprecated.  We should instead call Davis King's super-efficient
// dlib::hamming_distance() method instead which exists as of dlib
// version 18.1!  Need to "#include dlib/general_hash/count_bits.h"

// Method hamming_distance() computes the Hamming distance of two
// integers (considered as binary sequences of bits).  The run time
// of this procedure is proportional to the Hamming distance rather
// than to the number of input bits.  It computes the bitwise
// exclusive or of the 2 inputs.  It then returns the Hamming weight
// of the result (= number of nonzero bits) using an algorithm of
// Wegner (1960) that repeatedly finds and clears the lowest-order
// nonzero bit.

   unsigned long hamming_distance(unsigned long x,unsigned long y)
   {
      unsigned long dist=0;
      unsigned long value=x^y;
      
// Count number of set bits:

      while (value)
      {
         ++dist;
         value &= value-1;
      }
      return dist;
   }
   
// ---------------------------------------------------------------------
// This overloaded version of hamming_distance() takes in two
// binary strings whose characters are assumed to equal either 0 or 1.

   unsigned long hamming_distance(string& binary_str1,string& binary_str2)
   {
      unsigned long_form1=binary_string_to_eight_byte_integer(binary_str1);
      unsigned long_form2=binary_string_to_eight_byte_integer(binary_str2);
      return hamming_distance(long_form1,long_form2);
   }

// ---------------------------------------------------------------------
// This overloaded version of hamming_distance() takes in two
// genvectors whose entries are assumed to equal either 0 or 1.

   unsigned long hamming_distance(genvector* x_ptr,genvector* y_ptr)
   {
//      cout << "inside binaryfunc::hamming_distance()" << endl;
//      cout << "*x_ptr = " << *x_ptr << endl;
//      cout << "*y_ptr = " << *y_ptr << endl;

      string x_str=genvector_to_string(x_ptr);
      string y_str=genvector_to_string(y_ptr);
      return hamming_distance(x_str,y_str);
   }


   unsigned long hamming_distance(descriptor* x_ptr,descriptor* y_ptr)
   {
//      cout << "inside binaryfunc::hamming_distance()" << endl;
//      cout << "*x_ptr = " << *x_ptr << endl;
//      cout << "*y_ptr = " << *y_ptr << endl;

      string x_str=descriptor_to_string(x_ptr);
      string y_str=descriptor_to_string(y_ptr);
      return hamming_distance(x_str,y_str);
   }
   


} // binaryfunc namespace




