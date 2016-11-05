// ==========================================================================
// Header file for some useful string manipulation methods
// ==========================================================================
// Last updated on 4/29/14; 5/28/14; 3/5/16; 11/5/16
// ==========================================================================

#ifndef STRINGFUNCS_H
#define STRINGFUNCS_H

#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

namespace stringfunc
{

// ==========================================================================
// Ascii integer <--> character conversion methods
// ==========================================================================

   std::vector<int> decompose_string_to_ascii_rep(std::string inputstring);
   char ascii_integer_to_char(int n);
   unsigned char ascii_integer_to_unsigned_char(int n);
   int char_to_ascii_integer(char c);
   int unsigned_char_to_ascii_integer(unsigned char c);

// ==========================================================================
// Comment manipulation methods
// ==========================================================================

   void comment(std::string inputstring,std::ofstream& outstream);
   std::string comment_trunc(std::string inputstring);
   int comment_strip(
      unsigned int nlines,std::string line[],std::string commentlessline[]);
   void comment_strip(std::vector<std::string>& line);

// ==========================================================================
// Byte content display methods
// ==========================================================================

   void display_integer_bit_rep(unsigned int value);
   unsigned short byte_value(char c);
   std::string byte_bits_rep(char c,int n_bits=8);
   void display_byte_bits_rep(char c,int n_bits=8);
   int bits_rep_to_integer(std::string bits_rep_str);

// ==========================================================================
// Get methods
// ==========================================================================

   char mygetchar(unsigned int noutputlines,std::string outputstring[],
                  bool input_param_file,
                  std::string inputline[],unsigned int& currlinenumber);
   double mygetdouble(unsigned int noutputlines,std::string outputstring[],
                      bool input_param_file,
                      std::string inputline[],unsigned int& currlinenumber);
   int mygetinteger(unsigned int noutputlines,std::string outputstring[],
                    bool input_param_file,
                    std::string inputline[],unsigned int& currlinenumber);
   std::string mygetstring(
      const std::vector<std::string>& outputstring,
      bool input_param_file,std::string inputline[],
      unsigned int& currlinenumber);
   std::string mygetstring(
      unsigned int noutputlines,std::string outputstring[],
      bool input_param_file,
      std::string inputline[],unsigned int& currlinenumber);

// ==========================================================================
// String <--> other type conversion methods
// ==========================================================================

   char* string_to_chars(std::string inputstring);
   std::string unsigned_char_array_to_string(
      unsigned char* buffer,int n_chars);

   std::string char_to_string(char c);
   std::string boolean_to_string(bool flag);
   std::string integer_to_string(int i,int maximum_digits);
   std::string integer_to_hexadecimal(int i,int ndigits);
   int hexadecimal_to_integer(std::string hex_str);
   std::string number_to_string(double x);
   std::string number_to_string(double x,unsigned int nprecision);
   std::string integer_to_letters(int i);
   std::string remove_leading_zeros(std::string numberstring);
   std::string return_leading_whitespace(std::string inputstring);
   std::string remove_leading_whitespace(std::string inputstring);
   std::string remove_trailing_whitespace(std::string inputstring);
   
   std::string scinumber_to_string(double x,const int nprecision=5,
      bool nullchar_termination_flag=false);
   std::string pad_string(
      std::string input_string,int field_size,std::string pad_char=" ");
   std::string number_to_fixed_string_length(
      double x,int n_precision,unsigned int field_size);

   char string_to_char(std::string inputstring);
   bool string_to_boolean(std::string inputstring);
   int string_to_integer(std::string numberstring);
   long string_to_long(std::string numberstring);
   double string_to_number(std::string numberstring);

// ==========================================================================
// Substring decomposition methods methods
// ==========================================================================

   int compute_nfields(std::string inputstring);
   int compute_nfields(std::string inputstring,std::string separator_chars);

   std::vector<std::string> decompose_string_into_substrings(
      const std::string inputstring,std::string separator_chars=" \t\n");
   void decompose_string_into_substrings(
      std::string inputstring,std::vector<std::string>& substrings,
      std::string separator_chars=" \t\n");
   void Tokenize(
      const std::string& inputstring,std::vector<std::string>& substrings,
      std::vector<std::string>& separators,const std::string& delimiters);

   void string_to_two_numbers(std::string numberstring,int& i,int& j);
   bool string_to_two_numbers(std::string numberstring,double& x,double& y);
   void string_to_n_numbers(unsigned int n,std::string numberstring,int X[]);
   void string_to_n_numbers(unsigned int n,std::string numberstring,
                            double X[]);
   std::vector<double> string_to_numbers(std::string numberstring);
   std::vector<double> string_to_numbers(std::string numberstring,
                                         std::string separator_chars);
   std::string find_and_replace_char(
      std::string inputstring,std::string char_to_find,
      std::string replacement_char);

   int first_substring_location(
      std::string inputstring,std::string substring,int posn=0);
   std::string erase_chars_before_first_substring(
      std::string input_string,std::string substring);
   std::string erase_chars_after_first_substring(
      std::string input_string,std::string substring);
   std::string substring_between_substrings(
      std::string input_string,std::string substring_init,
      std::string substring_final);
   std::string XML_content_between_tags(
      std::string input_string,std::string XML_tag);

   void decompose_alpha_numer_string(
      const std::string& input_string,std::string& alpha_string,double& numer);

// ==========================================================================
// URL methods
// ==========================================================================

   std::string get_hostname_from_URL(std::string URL);
   int get_portnumber_from_URL(std::string URL);
   std::string get_reduced_URL(std::string URL);

// ==========================================================================
// Miscellaneous methods
// ==========================================================================

   bool is_E(std::string numberstring);
   bool is_number(std::string inputstring);
   std::string prefix(std::string inputstring,std::string separator=".");
   std::string suffix(std::string inputstring,std::string separator=".");
   std::string capitalize_word(std::string word);
   std::string capitalize_just_first_letter(std::string word);
   unsigned int edit_distance(int i, int j);
   unsigned int edit_distance(std::string s, std::string t);

// ==========================================================================
// Word stemming methods
// ==========================================================================

   std::string wstr_to_str(std::wstring wstr);
   std::wstring str_to_wstr(std::string str);
   std::string stem_word(std::string input_word);

// ==========================================================================
// Inlined methods
// ==========================================================================

   inline char ascii_integer_to_char(int n)
      {
         return static_cast<char>(static_cast<unsigned int>(n));
      }

   inline unsigned char ascii_integer_to_unsigned_char(int n)
      {
         return static_cast<unsigned char>(static_cast<unsigned int>(n));
      }
   
   inline int char_to_ascii_integer(char c)
      {
         return static_cast<int>(c);
      }

   inline int unsigned_char_to_ascii_integer(unsigned char c)
      {
         return static_cast<int>(c);
      }

   inline double string_to_number(std::string numberstring)
      {
         return atof(numberstring.c_str());
      }

   inline int string_to_integer(std::string numberstring)
      {
         return atoi(numberstring.c_str());
      }

// ---------------------------------------------------------------------
// Method first_substring_location takes in an input string as well as
// a search substring.  It performs an STL search for the substring
// within the input string.  If found, this method returns the
// position of the first instance of the substring within the input
// string.  Otherwise, it returns -1.


   inline int first_substring_location(
      std::string inputstring,std::string substring,int posn)
      {
         if (inputstring.size()==0) return -1;

//         int substring_posn=inputstring.find(substring,posn);
         return int(inputstring.find(substring,posn));
      }

} // stringfunc namespace

#endif  // general/stringfuncs.h





