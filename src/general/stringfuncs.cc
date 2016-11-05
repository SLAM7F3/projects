// We need to clean up and consolidate the constant numerals, lower
// and upper case letters, etc as constants defined at the top of this
// namespace.  As of 12/20/04, these symbols are defined in multiple
// methods throughout this namespace...

// ==========================================================================
// String methods definitions
// ==========================================================================
// Last updated on 1/23/16; 3/31/16; 10/4/16; 11/5/16
// ==========================================================================

#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>	// Needed for using string streams within 
			//  "number_to_string"
#include <dlib/string.h>

#include "math/basic_math.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"

#include "OleanderStemming/stemming/english_stem.h"

using std::cin;
using std::cout;
using std::endl;
using std::ios;
using std::ofstream;

using std::hex;
using std::ostringstream;
using std::stringstream;

using std::setw;
using std::string;
using std::vector;

namespace stringfunc
{

/*
  const string numerals="0123456789";
  const string number_symbols( "+-." );
  const string lowerletters="abcdefghijklmnopqrstuvwxyz";
  const string upperletters="ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  const string extrachars="~!@#$%^&*()_|=<>?/,{}][";
  const string whitespace(" \t\n");
  const string allchars=numerics+lowerletters+upperletters+extrachars;
*/

// ==========================================================================
// Ascii integer <--> character conversion methods
// ==========================================================================

// Method decompose_string_to_ascii_rep takes in a string and returns
// an STL vector containing each of its characters' integer ascii
// values.

   vector<int> decompose_string_to_ascii_rep(string inputstring)
   {
      const char* char_ptr=inputstring.c_str();
      vector<int> ascii_integer;

      for (unsigned int i=0; i<inputstring.length(); i++)
      {
         ascii_integer.push_back(char_to_ascii_integer(char_ptr[i]));
      }
      return ascii_integer;
   }

// ==========================================================================
// Comment manipulation methods
// ==========================================================================

// Method comment adds a comment string to an output ofstream
// preceded by a single hash mark:

   void comment(string inputstring,ofstream& outstream)
   {
      string commentstring="# "+inputstring;
      outstream << commentstring << endl;
   }

// ---------------------------------------------------------------------
// Brian Kavanagh's comment_trunc method takes a string as input,
// and returns the string truncated from the first pound sign `#', on.
// If there is no pound sign, it simply returns the original string:

   string comment_trunc(string inputstring)
   {
      int pound_position = inputstring.find_first_of("#",0);
      return inputstring.substr(0,pound_position);
   }

// ---------------------------------------------------------------------
// Method comment_strip eliminates all comments from an input file
// contained within string array line[].  It returns the number of
// uncommented and non-white lines within output string array
// commentlessline[].

   int comment_strip(unsigned int nlines,string line[],
                     string commentlessline[])
   {
      unsigned int new_nlines=0;
      for (unsigned int i=0; i<nlines; i++)
      {
         string cleanedline=comment_trunc(line[i]);

// Discard all blank lines:

         if (!cleanedline.empty())
         {
            commentlessline[new_nlines++]=cleanedline;
         }
      }
      return new_nlines;
   }

   void comment_strip(vector<string>& line)
   {
      vector<string> commentless_line;
         
      for (unsigned int i=0; i<line.size(); i++)
      {
         string cleanedline=comment_trunc(line[i]);

// Discard all blank lines:

         if (!cleanedline.empty())
         {
            commentless_line.push_back(cleanedline);
         }
      }
      line.clear();
      line=commentless_line;
   }

// ==========================================================================
// Byte content display methods
// ==========================================================================

// Method display_integer_bit_rep takes in an unsigned integer (which
// is typically 4-bytes long on 32-bit machines).  This method (which
// was copied from chapter 16 of "C++: How to program" by Deitel &
// Deitel) outputs its bit representation as a series of four sets of
// zeros and ones.

   void display_integer_bit_rep(unsigned int value)
   {
      const int SHIFT=8*sizeof(unsigned int)-1;
      const unsigned int MASK=1 << SHIFT;
      cout << setw(7) << value << " = " ;
         
      for (unsigned int c=1; c<=SHIFT+1; c++)
      {
         cout << (value&MASK ? '1' : '0');
         value <<= 1;
         if (c%8==0) cout << ' ';
      }
      outputfunc::newline();
   }

// ---------------------------------------------------------------------
   unsigned short byte_value(char c)
   {
      int value0=static_cast<int>(c);
      if (value0 < 0) value0=256+value0;
      unsigned short value=static_cast<unsigned short>(value0);
         
      if (value > 255)
      {
         cout << "Trouble in stringfunc::byte_value() !" << endl;
         cout << "Input byte's value lies outside range [0,255]" << endl;
         cout << "value = " << value << endl;
      }
      return value;
   }

// ---------------------------------------------------------------------
// Method byte_bits_rep() takes in a byte (in the form of a
// single character).  It converts this single byte's content into a
// character string of 0's and/or 1's whose length equals n_bits:
   
   string byte_bits_rep(char c,int n_bits)
   {
      unsigned short value=byte_value(c);

      ostringstream buffer;
      const int SHIFT=8*sizeof(unsigned short)-1;
      const unsigned short MASK=1 << SHIFT;
      
      for (unsigned short c=1; c<=SHIFT+1; c++)
      {
         if (c >= 9)
         {
            buffer << (value&MASK ? '1' : '0');
         }
         value <<= 1;
      }
      string buffer_str=buffer.str();
//      cout << "buffer_str = " << buffer_str << endl;

      string substring=buffer_str.substr(8-n_bits, n_bits);
      return substring;
   }

// ---------------------------------------------------------------------
   void display_byte_bits_rep(char c,int n_bits)
   {
      unsigned short value=byte_value(c);
      if (value > 255) return;

      cout << setw(7) << value << " = " << byte_bits_rep(c,n_bits) << endl;
   }

// ---------------------------------------------------------------------
// Method bits_rep_to_integer() performs poor-man's bit algebra.  It
// converts the bit string back into a base-10 integer.

   int bits_rep_to_integer(string bits_rep_str)
   {
      int nbits = bits_rep_str.size();
      int base10_value = 0;
      int curr_power = 1;
      for(int c = 0; c < nbits; c++)
      {
         if(bits_rep_str.substr(nbits-1-c,1) == "1")
         {
            base10_value += curr_power;
         }
         curr_power *= 2;
      }
      return base10_value;
   } 

// ==========================================================================
// Get methods
// ==========================================================================

// Method mygetchar queries the user for a single character input
// either interactively from the keyboard or passively from an input
// parameter file stored within string array inputline:

   char mygetchar(unsigned int noutputlines,string outputstring[],
                  bool input_param_file,
                  string inputline[],unsigned int& currlinenumber)
   {
      char returnchar;
   
      if (input_param_file)
      {
         returnchar=string_to_char(inputline[currlinenumber++]);
      }
      else
      {
         for (unsigned int i=0; i<noutputlines; i++)
         {
            cout << outputstring[i] << endl;
         }
         outputfunc::newline();
         cin >> returnchar;
      }
      outputfunc::newline();
      return returnchar;
   }

// ---------------------------------------------------------------------
// Method mygetdouble queries the user for a single double input
// either interactively from the keyboard or passively from an input
// parameter file stored within string array inputline:

   double mygetdouble(unsigned int noutputlines,string outputstring[],
                      bool input_param_file,
                      string inputline[],unsigned int& currlinenumber)
   {
      double returndouble;

      if (input_param_file)
      {
         returndouble=string_to_number(inputline[currlinenumber++]);
      }
      else
      {
         for (unsigned int i=0; i<noutputlines; i++)
         {
            cout << outputstring[i] << endl;
         }
         outputfunc::newline();
         cin >> returndouble;
      }
      outputfunc::newline();
      return returndouble;
   }

// ---------------------------------------------------------------------
// Method mygetinteger queries the user for a single integer input
// either interactively from the keyboard or passively from an input
// parameter file stored within string array inputline.  In the former
// case, mygetinteger performs some rudimentary checks to determine
// whether the entered string is an integer or not, and it continues
// to query the user for input until those checks are passed.

   int mygetinteger(
      unsigned int noutputlines,string outputstring[],bool input_param_file,
      string inputline[],unsigned int& currlinenumber)
   {
      bool validresponse=false;
      int returnint=0;
      double inputvalue;
      string inputstring;

      if (input_param_file)
      {
         returnint=basic_math::round(
            string_to_number(inputline[currlinenumber++]));
      }
      else
      {
         while (!validresponse)
         {
            for (unsigned int i=0; i<noutputlines; i++)
            {
               cout << outputstring[i] << endl;
            }
            outputfunc::newline();

            // Perform some rudimentary checks to determine whether
            // a string entered in at the keyboard is an integer or
            // not:

            cin >> inputstring;
            if (is_number(inputstring))
            {
               inputvalue=string_to_number(inputstring);
               if (basic_math::is_int(inputvalue))
               {
                  returnint=basic_math::round(inputvalue);
                  validresponse=true;
               }
            }
            if (!validresponse)
            {
               cout << "*** Non-integer input ***" << endl;
               outputfunc::newline();
            }
         }
      }
   
      outputfunc::newline();
      return returnint;
   }

// ---------------------------------------------------------------------
// Method mygetstring queries the user for a single line of string
// input either interactively from the keyboard or passively from an
// input parameter file stored within string array inputline:

   string mygetstring(const vector<string>& outputstring,
                      bool input_param_file,
                      string inputline[],unsigned int& currlinenumber)
   {
      string returnstring;
   
      if (input_param_file)
      {
         returnstring=inputline[currlinenumber++];
      }
      else
      {
         for (unsigned int i=0; i<outputstring.size(); i++)
         {
            cout << outputstring[i] << endl;
         }
         outputfunc::newline();
         cin >> returnstring;
      }
      outputfunc::newline();
      return returnstring;
   }

   string mygetstring(unsigned int noutputlines,string outputstring[],
                      bool input_param_file,
                      string inputline[],unsigned int& currlinenumber)
   {
      string returnstring;
   
      if (input_param_file)
      {
         returnstring=inputline[currlinenumber++];
      }
      else
      {
         for (unsigned int i=0; i<noutputlines; i++)
         {
            cout << outputstring[i] << endl;
         }
         outputfunc::newline();
         cin >> returnstring;
      }
      outputfunc::newline();
      return returnstring;
   }

// ==========================================================================
// String <--> other type conversion methods
// ==========================================================================

// Method string_to_chars() dynamically instantiates a char* array and
// fills its contents with the characters from inputstring.  A
// terminating null character is automatically added to the end of the
// char* array.

   char* string_to_chars(string inputstring)
   {
      char* cstr = new char [inputstring.size()+1];
      strcpy (cstr, inputstring.c_str());

      return cstr;
   }

// ---------------------------------------------------------------------
// Method unsigned_char_array_to_string() takes in an array of
// unsigned chars within buffer as well as the number of characters to
// convert to an STL string.  It returns the corresponding STL string.
// See
// http://bytes.com/topic/c/answers/636204-how-translate-unsigned-char-string.

   string unsigned_char_array_to_string(
      unsigned char* buffer,int n_chars)
   {
      string buffer_str(buffer,buffer+n_chars);
      return buffer_str;
   }

// ---------------------------------------------------------------------
   string char_to_string(char c)
   {
//         cout << "inside char_to_string, c = " << c << endl;
      string curr_char_str;
      curr_char_str += c;
//         cout << "curr_char_str = " << curr_char_str << endl;
      return curr_char_str;
   }

// ---------------------------------------------------------------------
   string boolean_to_string(bool flag)
   {
      if (flag)
      {
         return "TRUE";
      }
      else
      {
         return "FALSE";
      }
   }
   
// ---------------------------------------------------------------------
// Method integer_to_string takes in a non-negative integer i
// along with some specified maximum number of digits parameter.  It
// returns a string which may be padded with leading zeros so that the
// total number of digits within the string equals maximum_digits.  We
// cooked up this little routine in order to uniformly label image
// numbers.

   string integer_to_string(int i,int maximum_digits)
   {
      int ndigits=1;
      if (i != 0)
      {
         ndigits=basic_math::mytruncate(log10(fabs(double(i))))+1;
      }
      
//         int maxdigits=basic_math::max(maximum_digits,ndigits);
//         if (maxdigits < ndigits) maxdigits=ndigits;
//         int digit_diff=maxdigits-ndigits;
      unsigned int digit_diff=basic_math::max(maximum_digits,ndigits)
         -ndigits;

      string integer_string="";
      for (unsigned int j=0; j<digit_diff; j++)
      {
         integer_string += "0";
      }
      integer_string += number_to_string(i);
      return integer_string;
   }

/*
  string longlong_to_string(longlong i,int maximum_digits)
  {
  int ndigits=1;
  if (i != 0)
  {
  ndigits=basic_math::mytruncate(log10(fabs(double(i))))+1;
  }
      
  int digit_diff=basic_math::max(maximum_digits,ndigits)-ndigits;

  string integer_string="";
  for (unsigned int j=0; j<digit_diff; j++)
  {
  integer_string += "0";
  }
  integer_string += number_to_string(i);
  return integer_string;
  }
*/

// ---------------------------------------------------------------------
// Method integer_to_hexadecimal takes in integer i along with the
// number of desired output hexadecimal digits.  It returns a string
// containing the hexadecimal format for the integer which is
// pre-padded with zeros.

   string integer_to_hexadecimal(int i,int ndigits)
   {
      char buffer[50];
      stringstream ss;

      ss << hex << i;
      ss >> buffer;
      string output=buffer;

      unsigned int initial_ndigits=output.size();
      for (unsigned int n=0; n<ndigits-initial_ndigits; n++)
      {
         output="0"+output;
      }
      return output;
   }

// ---------------------------------------------------------------------
// Method hexadecimal_to_integer() returns the base 10 integer
// corresponding to an input hex string.

   int hexadecimal_to_integer(string hex_str)
   {
      unsigned int x;
      stringstream ss;

      ss << hex << hex_str;
      ss >> x;
      return static_cast<int>(x);
   }

// ---------------------------------------------------------------------
// On 7/13/00, we learned from James Wanken that we should in
// principle be able to use C++ stringstreams which act as internal
// buffers to write out ints and doubles to C++ strings.  However, we
// found out on 7/19/00 that the stringstream class appears to have a
// memory leak.  So James suggested that we instead take the following
// C approach to creating our own "itoa" and "ftoa" function:

   string number_to_string(double x)
   {
      if (basic_math::is_int(x))
      {
         char buffer[100];
         sprintf(buffer,"%d",basic_math::round(x));
         return string(buffer);
      }
      else
      {
         return number_to_string(x,5);
      }
   }

// ---------------------------------------------------------------------
// In this overloaded version of method number_to_string, the user can
// specify the number of decimal places for the double x which will be
// returned within the output string.  nprecision worth of trailing
// zeros are added onto integers so that they line up with doubles
// with nonzero fractional parts.

   string number_to_string(double x,unsigned int nprecision)
   {
      char buffer[100];
      if (basic_math::is_int(x))
      {
         sprintf(buffer,"%d",basic_math::round(x));
      }
      else
      {
         string precisionstr="%."+number_to_string(nprecision)+"f";
         sprintf(buffer,precisionstr.c_str(),x);
      }
      string numberstring(buffer);
      if (basic_math::is_int(x))
      {
         numberstring += ".";
         for (unsigned int n=0; n<nprecision; n++)
         {
            numberstring += "0";
         }
      }

      return numberstring;
   }

// ---------------------------------------------------------------------
// Method integer_to_letters() takes in integer i ranging from 0 to
// 675.  It maps these input integers onto the following letter
// combinations:

//	1	2	3			24	25	27
//	A	B	C			X	Y	Z

//	27 	28 	29			50	51	52
//	AA	AB	AC			AX	AY	AZ

//	53	54	55
//	BA	BB	BC	


   string integer_to_letters(int i)
   {
//         cout << "input i = " << i << endl;
         
      const int ascii_A=65;

      string letters_symbol;
      if (i==0)
      {
         letters_symbol="0";
         return letters_symbol;
      }
      else if (i >=1 && i <= 26)
      {
         char new_char=stringfunc::ascii_integer_to_char(ascii_A+i-1);
         letters_symbol=new_char;
         return letters_symbol;
      }
      else if (i >= 27 && i <= 675)
      {
         for (unsigned int n=1; n>=0; n--)
         {
            int number=i;
            int power_of_26=basic_math::round(pow(26,n));
            int first_digit=number/power_of_26;
            int second_digit=number-power_of_26*first_digit;

            if (second_digit==0)
            {
               first_digit--;
               second_digit += 26;
            }
               
            char first_char=stringfunc::ascii_integer_to_char(
               ascii_A+first_digit-1);
            char second_char=stringfunc::ascii_integer_to_char(
               ascii_A+second_digit-1);

//               cout << "first_digit = " << first_digit
//                    << " second_digit = " << second_digit << endl;
            letters_symbol=first_char;
            letters_symbol += second_char;
            return letters_symbol;
         }
      }
      else
      {
         letters_symbol="infinity";
         return letters_symbol;
      }

   }

// ---------------------------------------------------------------------
// Method remove_leading_zeros is a quick-and-dirty (and not very
// general nor robust!) subroutine which searches for leading zeros in
// number strings (e.g. "0.123").  It strips such zeros away and
// returns a reduced substring (e.g. ".123").

   string remove_leading_zeros(string numberstring)
   {
      const string numerics=".123456789";
      string stripped_numberstring=numberstring;
      unsigned int first_zero_posn=numberstring.find_first_of(
         "0",0);
      unsigned int first_numeric_posn=numberstring.find_first_of(
         numerics,0);

      if (numberstring != "0" && first_zero_posn < first_numeric_posn)
      {
         stripped_numberstring=
            numberstring.substr(
               first_numeric_posn,numberstring.length()
               -first_numeric_posn);
      }
      return stripped_numberstring;
   }

// ---------------------------------------------------------------------
// Method return_leading_whitespace returns a substring 
// ranging from the input string's 0th up to but not including its
// first non-white character.

   string return_leading_whitespace(string inputstring)
   {
//         cout << "inside stringfunc::return_leading_whitespace" << endl;

      int s_first=0;
      for (unsigned int s=0; s<inputstring.size(); s++)
      {
         if (inputstring.substr(s,1) != " " &&
             inputstring.substr(s,1) != "\t")
         {
            s_first=s;
            break;
         }
      }
      string white_space=inputstring.substr(0,s_first);
//         cout << "white_space = " << white_space
//              << " white_space.size() = " << white_space.size()
//              << endl;
      return white_space;
   }

// ---------------------------------------------------------------------
// Method remove_leading_whitespace searches forwards within
// inputstring for its first non-white character.

   string remove_leading_whitespace(string inputstring)
   {
//         cout << "inside stringfunc::remove_leading_whitespace" << endl;

      int s_first=0;
      for (unsigned int s=0; s<inputstring.size(); s++)
      {
         if (inputstring.substr(s,1) != " " &&
             inputstring.substr(s,1) != "\t")
         {
            s_first=s;
            break;
         }
      }
      string stripped_inputstring=
         inputstring.substr(s_first,inputstring.size()-s_first+1);
//         cout << "inputstring = " << inputstring
//              << " size =  " << inputstring.size() << endl;
//         cout << "stripped_inputstring = " << stripped_inputstring
//              << " size = " << stripped_inputstring.size() << endl;
      return stripped_inputstring;
   }

// Method remove_trailing_whitespace() searches for spaces, tabs or
// carriage returns at the end of inputstring.  It returns the input
// string without these trailing whitespace characters.

   string remove_trailing_whitespace(string inputstring)
   {
//         cout << "inside stringfunc::remove_trailing_whitespace" << endl;
//         cout << "inputstring = " << inputstring << endl;

      if (inputstring.size()==0) return "";

      int s_last=inputstring.size()-1;
      for (int s=inputstring.size()-1; s>0; s--)
      {
         if (inputstring.substr(s,1) != " " &&
             inputstring.substr(s,1) != "\t" &&
             inputstring.substr(s,1) != "\n")	
         {
            s_last=s;
            break;
         }
      }
      string stripped_inputstring=inputstring.substr(0,s_last+1);
      return stripped_inputstring;
   }

// ---------------------------------------------------------------------
// In method scinumber_to_string, the user can specify the number of
// decimal places for the double x which will be returned within the
// output string:

   string scinumber_to_string(
      double x,const int nprecision,bool nullchar_termination_flag)
   {
      ostringstream buffer;
 
      buffer.precision(nprecision);
      buffer.setf(ios::scientific);
      buffer.setf(ios::showpoint);
      buffer << x;

// Terminate buffer input with a null '/0' character.  All characters
// within the string beyond this null character will be ignored by any
// string processing commands:

      if (nullchar_termination_flag) buffer << '\0';

      return buffer.str();
   }

// ---------------------------------------------------------------------
// Method pad_string takes in some input_string whose size should be
// less than input integer parameter field_size.  This method adds
// leading blank spaces on the left so that the output string's size
// precisely equals field_size.

   string pad_string(string input_string,int field_size,string pad_char)
   {
      unsigned int n_padding_spaces=field_size-input_string.size();

      if (n_padding_spaces < 0)
      {
         cout << "Error in stringfunc::pad_string() !" << endl;
         cout << "input_string.size() = " << input_string.size() 
              << " > field_size = " << field_size << endl;
         return input_string;
      }
         
      string output_string="";
      for (unsigned int j=0; j<n_padding_spaces; j++)
      {
         output_string += pad_char;
      }
      output_string += input_string;
      return output_string;
   }

// ---------------------------------------------------------------------
// Method number_to_fixed_string_length takes in input double argument
// x, the desired decimal precision for its output format and the
// total desired size for the output string.  It necessary, it formats
// the number in scientific notation in order to force the output
// string result to have a fixed specified length.  We wrote this
// utility in Nov 2006 for matrix display purposes.

   string number_to_fixed_string_length(
      double x,int n_precision,unsigned int field_size)
   {
      string number_str=number_to_string(x,n_precision);
      if (nearly_equal(x,0,1.0E-10))
      {
         number_str="0";
      }
      else if (number_str.size() > field_size || 
               fabs(x) < pow(10,-n_precision))
      {
         number_str=scinumber_to_string(x,n_precision);
      }
//         string pad_char="X";
      return pad_string(number_str,field_size);
   }

// ---------------------------------------------------------------------
// Method string_to_char returns the first non-white character
// located within inputstring:

   char string_to_char(string inputstring)
   {
      const string numerics="0123456789";
      const string lowerletters="abcdefghijklmnopqrstuvwxyz";
      const string upperletters="ABCDEFGHIJKLMNOPQRSTUVWXYZ";
      const string punctuation=".,:;'`-+_^=/~|!$#@%&*?{}()[]<>\"";
      string allchars=numerics+lowerletters+upperletters+punctuation;

      string firstcharstring;
      string::size_type firstcharposn=inputstring.find_first_of(
         allchars,0);

      // C++ find function for string class returns the
      // specialvalue string::npos to indicate no match.  See page
      // 276 in C++ Primer, 3rd edition by Lippman and Lajoie:

      if (inputstring.empty() || firstcharposn==string::npos)
      {
         cout << "Error inside string_to_char!" << endl;
         cout << "Input string does not contain any characters" << endl;
         char blankchar=' ';
         return blankchar;
      }
      else
      {
         firstcharstring=inputstring.substr(firstcharposn,1);
         const char *firstchar=firstcharstring.c_str();
         return *firstchar;
      }
   }

// ---------------------------------------------------------------------
   bool string_to_boolean(string inputstring)
   {
      char first_char=string_to_char(inputstring);
      bool boolean_value=false;
      if (first_char=='f' || first_char=='F')
      {
         boolean_value=false;
      }
      else if (first_char=='t' || first_char=='T')
      {
         boolean_value=true;
      }
      else
      {
         cout << "Error in string_to_boolean()" << endl;
         cout << "first_char = " << first_char << endl;
      }
      return boolean_value;
   }

// ---------------------------------------------------------------------
// Method string_to_long() takes in an inputstring which we assume
// contains an integer with up to 19 digits.  This method returns its
// long (8 byte) integer equivalent.

   long string_to_long(string inputstring)
   {
      char* pEnd;
      long int l=strtol(inputstring.c_str(),&pEnd,10);
      return l;
   }

// ==========================================================================
// Substring decomposition methods methods
// ==========================================================================

// Method compute_nfields takes in an input string which is assumed to
// correspond to a row of several numbers.  It returns the number of
// numbers which the inputstring contains.

   int compute_nfields(string inputstring)
   {
      string whitespace(" \t\n");
      return compute_nfields(inputstring,whitespace);
   }

   int compute_nfields(string inputstring,string separator_chars)
   {
//         cout << "inside stringfunc::compute_nfields()" << endl;
//         cout << "separator_chars = " << separator_chars << endl;
      vector<string> substring(
         decompose_string_into_substrings(inputstring,separator_chars));
      return substring.size();
   }
   
// ---------------------------------------------------------------------
// Method decompose_string_into_substrings() simply calls dlib's split
// method.  As of 10/24/13, we strongly suspect this is the best
// string tokenizer code available from the web.

   vector<string> decompose_string_into_substrings(
      string input_str,string separator_chars)
   {
//      cout << "inside stringfunc::decompose_string_into_substrings() #1"
//           << endl;
//      cout << "input_str = " << input_str << endl;
      return dlib::split(input_str,separator_chars);
   }

// This overloaded version of decompose_string_into_substrings()
// returns an ordered set of tokens as well as the ordered substrings
// which separate the tokens within output STL vector separators:

   void decompose_string_into_substrings(
      string inputstring,string delimeter_chars,
      vector<string>& tokens,vector<string>& separators)
   {
      Tokenize(inputstring,tokens,separators,delimeter_chars);
   }

// Following tokenizer code taken from
// http://oopweb.com/CPP/Documents/CPPHOWTO/Volume/C++Programming-HOWTO-7.html:

   void Tokenize(const string& str,vector<string>& tokens,
	         vector<string>& separators, const string& delimiters)
   {
//      cout << "inside stringfunc::Tokenize()" << endl;
//      cout << "str = " << str << " delimeters.size() = " << delimiters.size()
//           << endl;

      tokens.clear();
      separators.clear();

      // Skip delimiters at beginning.
      string::size_type lastPos = str.find_first_not_of(delimiters, 0);
      // Find first "non-delimiter".
      string::size_type pos = str.find_first_of(delimiters, lastPos);

      string::size_type prev_pos=pos;
      
//      cout << "lastPos = " << lastPos << " pos = " << pos << endl;
      while (string::npos != pos || string::npos != lastPos)
      {
         // Found a token, add it to the vector.
         tokens.push_back(str.substr(lastPos, pos - lastPos));
         // Skip delimiters.  Note the "not_of"
         lastPos = str.find_first_not_of(delimiters, pos);
         // Find next "non-delimiter"
         pos = str.find_first_of(delimiters, lastPos);

         if (prev_pos != string::npos)
         {
            separators.push_back(str.substr(prev_pos, lastPos-prev_pos));
            prev_pos=pos;
         }

//         cout << "lastPos = " << lastPos
//              << " pos = " << pos 
//              << " token = " << tokens.back() 
//              << " separator = " << separators.back() << endl;

      }
   }

// ---------------------------------------------------------------------
// This "string_to_two_numbers" routine is a virtual carbon copy of
// the above "string_to_number" routine.  (After using the built in
// string functions to separate the number strings.)  In this routine,
// we read in an ordered (i,j) pair of integers rather than a single
// integer i from a string.  This routine is useful for reading in the
// contents of meta file plots.

   void string_to_two_numbers(string numberstring,int& i,int& j)
   {
      const string numerics( "0123456789+-." );
      const string whitespace(" \t\n");

      int number_position = numberstring.find_first_of(numerics, 0);
      int separator_position = numberstring.find_first_of(
         whitespace,number_position);

      string value_1 = numberstring.substr(0,separator_position);
      string value_2 = numberstring.substr(separator_position);
   
      i= atoi(value_1.c_str());
      j = atoi(value_2.c_str());
   }

// ---------------------------------------------------------------------
// In this overloaded of version of the above string_to_two_numbers
// method, an ordered pair of doubles (x,y) is returned:

   bool string_to_two_numbers(string numberstring,double& x,double& y)
   {
      const string numerics( "0123456789+-." );
      const string whitespace(" \t\n");

      int number_position = numberstring.find_first_of(numerics, 0);
      int separator_position = numberstring.find_first_of(
         whitespace,number_position);

      bool first_numeral_found=(number_position >= 0);
      if (first_numeral_found)
      {
         string value_1 = numberstring.substr(0,separator_position);
         string value_2 = numberstring.substr(separator_position);
         x = atof(value_1.c_str());
         y = atof(value_2.c_str());
      }
      return first_numeral_found;
   }

// ---------------------------------------------------------------------
// These "string_to_n_numbers" routines generalize the preceding
// "string_to_number" and "string_to_two_numbers" methods

   void string_to_n_numbers(unsigned int n,string numberstring,int X[])
   {
      const int MAXN=50;

      int number_posn[MAXN],separator_posn[MAXN+1];
      string numerics("0123456789+-.");
      string whitespace(" \t\n");

      separator_posn[0]=0;
      for (unsigned int i=0; i<n; i++)
      {
         number_posn[i]=numberstring.find_first_of(
            numerics,separator_posn[i]);
         separator_posn[i+1]=numberstring.find_first_of(
            whitespace,number_posn[i]+1);
      }

      for (unsigned int i=0; i<n; i++)
      {
         string currnumber_substring=
            numberstring.substr(number_posn[i],
                                separator_posn[i+1]-number_posn[i]);
         X[i]=atoi(currnumber_substring.c_str());
      }
   }

// This next method is deprecated as of Dec 2005.  Use the following
// simpler "string_to_numbers" method instead...

   void string_to_n_numbers(unsigned int n,string numberstring,double X[])
   {
      const int MAXN=50;

      int number_posn[MAXN],separator_posn[MAXN+1];
      string numerics("0123456789+-.");
      string whitespace(" \t\n");

      separator_posn[0]=0;
      for (unsigned int i=0; i<n; i++)
      {
         number_posn[i]=numberstring.find_first_of(
            numerics,separator_posn[i]);
         separator_posn[i+1]=numberstring.find_first_of(
            whitespace,number_posn[i]+1);
      }

      for (unsigned int i=0; i<n; i++)
      {
         string currnumber_substring=numberstring.substr(
            number_posn[i],separator_posn[i+1]-number_posn[i]);
         X[i]=atof(currnumber_substring.c_str());
      }
   }

// ---------------------------------------------------------------------
   vector<double> string_to_numbers(string numberstring)
   {
      const string whitespace(" \t\n");
      return string_to_numbers(numberstring,whitespace);
   }

// Method string_to_numbers() attempts to extract as many legitimate
// numbers from the input numberstring as it can reasonably parse.

   vector<double> string_to_numbers(
      string numberstring,string separator_chars)
   {
//         cout << "inside stringfunc::string_to_numbers, numberstring = "
//              << numberstring << endl;
//         cout << "separator_chars = " << separator_chars << endl;

      vector<string> substrings(
         decompose_string_into_substrings(numberstring,separator_chars));

      vector<double> X;
      for (unsigned int i=0; i<substrings.size(); i++)
      {
//            cout << "i = " << i << " substrings[i] = " << substrings[i]
//                 << endl;
         if (is_number(substrings[i]))
         {
            X.push_back(atof(substrings[i].c_str()));
         }
      }
      return X;
   }

// ---------------------------------------------------------------------
// Method find_and_replace_char scans through the input string and
// replaces every instance of char_to_find with replacement_char.  The
// modified string is returned by this method.

   string find_and_replace_char(
      string inputstring,string char_to_find,string replacement_char)
   {
//         cout << "inside stringfunc::find_and_replace_char()" << endl;

      string outputstring;
      for (unsigned int i=0; i<inputstring.size(); i++)
      {
         string curr_char=char_to_string(inputstring[i]);
         if (curr_char==char_to_find)
         {
            outputstring += replacement_char;
         }
         else
         {
            outputstring += inputstring[i];
         }
      }
      return outputstring;
   }

// ---------------------------------------------------------------------
// Methods erase_chars_before_first_substring(),
// erase_chars_after_first_substring() and
// substring_between_substrings() were written for XML parsing
// purposes.  They return zero length null strings if the substring
// search is not successful.

   string erase_chars_before_first_substring(
      string input_string,string substring)
   {
      int posn=first_substring_location(input_string,substring);
      string chopped_input="";
      if (posn >= 0) 
      {
         chopped_input=input_string.substr(posn,input_string.size()-posn);
      }
      return chopped_input;
   }

   string erase_chars_after_first_substring(
      string input_string,string substring)
   {
//         cout << "inside stringfunc::erase_chars_after_first_substring()"
//              << endl;
      int posn=first_substring_location(input_string,substring);
      string chopped_input="";
      if (posn >= 0)
      {
//            posn += substring.size()+1;
         posn += substring.size();
         chopped_input=input_string.substr(0,posn);
      }
//         cout << "chopped_input = " << chopped_input << endl;
      return chopped_input;
   }

   string substring_between_substrings(
      string input_string,string substring_init,string substring_final)
   {
      string chopped_input1=erase_chars_before_first_substring(
         input_string,substring_init);
//         cout << "chopped_input1 = " << chopped_input1 << endl;
      string chopped_input2=erase_chars_after_first_substring(
         chopped_input1,substring_final);
//         cout << "chopped_input2 = " << chopped_input2 << endl;
      return chopped_input2;
   }

// ---------------------------------------------------------------------
// Method XML_content_between_tags() strips away any substrings within
// the input_string preceding the opening or following the closing XML
// tag passed in as an argument.  The pruned XML content is returned
// by this method.

   string XML_content_between_tags(string input_string,string XML_tag)
   {
//   cout << "inside stringfunc::XML_content_between_tags()" << endl;
      string open_tag="<"+XML_tag+">";
      string close_tag="</"+XML_tag+">";
      string xml_content=substring_between_substrings(
         input_string,open_tag,close_tag);
//   cout << "xml_content = " << xml_content << endl;
      return xml_content;
   }

// ---------------------------------------------------------------------
// Method decompose_alpha_numer_string() takes in a string which is
// assumed to be of the form "XXX####" where XXX denotes some string
// consisting of non-numerals and #### denotes some string comprised
// only of numerals (and possibly includes a decimal point).  This
// method extracts and returns the alpha and numerical parts of the
// input string.

   void decompose_alpha_numer_string(
      const string& input_string,string& alpha_string,double& number)
   {
      string separator_chars="0123456789";
      vector<string> substrings(
         stringfunc::decompose_string_into_substrings(
            input_string,separator_chars));

      alpha_string=substrings[0];
      int alpha_length=alpha_string.size();

      int number_length=input_string.size()-alpha_length;
//      cout << "number_length = " << number_length << endl;
      string number_str=input_string.substr(
         input_string.size()-number_length,number_length);
//      cout << "number_str = " << number_str << endl;
      number=stringfunc::string_to_number(number_str);
//      cout << "number = " << number << endl;
   }

// ==========================================================================
// URL methods
// ==========================================================================

   string get_hostname_from_URL(string URL)
   {
      return prefix(URL,":");
   }
   
   int get_portnumber_from_URL(string URL)
   {
      return string_to_integer(suffix(URL,":"));
   }

// Method get_reduced_URL() strips off any http://IP:portnumber prefix
// within the input URL.  It returns the reduced URL without this prefix.

   string get_reduced_URL(string URL)
   {
      string separator_chars=":";
      vector<string> substrings(
         stringfunc::decompose_string_into_substrings(URL,separator_chars));
      
      string reduced_URL=substrings.back();
//      cout << "reduced_URL = " << reduced_URL << endl;
      reduced_URL=stringfunc::erase_chars_before_first_substring(
         reduced_URL,"/");
      
//      cout << "reduced_URL = " << reduced_URL << endl;
      return reduced_URL;
   }

// ==========================================================================
// Miscellaneous methods
// ==========================================================================

// Boolean function is_E tests for the presence of "E" within an input
// string.  This function is useful for reading in RCS files.

   bool is_E(string numberstring)
   {
      const string eletter="E";

      string::size_type epos=numberstring.find_first_of(eletter);

      // C++ find function for string class returns the
      // specialvalue string::npos to indicate no match.  See page
      // 276 in C++ Primer, 3rd edition by Lippman and Lajoie:

      if (numberstring.empty() || epos==string::npos)
      {
         return false;
      }
      else
      {
         return true;
      }
   }

// ---------------------------------------------------------------------
// Routine is_number has some limited ability to determine whether an
// input string contains non-numeric characters within it.  We do not
// (yet) check for the existence of more than one "e" or "E" within
// the string.  So number strings expressed in exponential forms are
// regarded as numbers

   bool is_number(string inputstring)
   {
//         cout << "inside stringfunc::is_number()" << endl;
//         cout << "inputstring = " << inputstring << endl;
         
      const string numerics="0123456789";
      const string lowerletters="abcdfghijklmnopqrstuvwxyz";
      const string upperletters="ABCDFGHIJKLMNOPQRSTUVWXYZ";
      const string extrachars="~!@#$%^&*()_|=<>?/';[]{}";
      string::size_type numberpos=inputstring.find_first_of(numerics);
      string::size_type lletterpos=inputstring.find_first_of(lowerletters);
      string::size_type uletterpos=inputstring.find_first_of(upperletters);
      string::size_type extracharpos=inputstring.find_first_of(extrachars);
      //   string::size_type dashpos=inputstring.find_last_of("-");

//         cout << "numberpos = " << numberpos << endl;
//         cout << "lletterpos = " << lletterpos << endl;
//         cout << "uletterpos = " << uletterpos << endl;

      // C++ find function for string class returns the special value
      // string::npos to indicate no match.  See page 276 in C++ Primer, 3rd
      // edition by Lippman and Lajoie:

      if (inputstring.empty() || numberpos==string::npos 
          || lletterpos != string::npos || uletterpos != string::npos
          || extracharpos != string::npos)
      {
         return false;
      }
      //   else if (dashpos != string::npos && dashpos>numberpos)
      //   {
      //      return false;
      //   }
      else
      {
         return true;
      }
   }

// ---------------------------------------------------------------------
// Methods prefix and suffix take in an inputstring which is assumed
// to be of the form "prefix.suffix".  These methods return the
// substrings before and after the dot within the inputstring.

   string prefix(string inputstring,string separator)
   {
      int last_dot_position = inputstring.find_last_of(
         separator,inputstring.size());
      return inputstring.substr(0,last_dot_position);
   }

   string suffix(string inputstring,string separator)
   {
//         cout << "inside stringfunc::suffix, inputstring = " << inputstring
//              << " separator string = " << separator << endl;

      int last_dot_position = inputstring.find_last_of(
         separator,inputstring.length());
//         cout << "last_dot_posn = " << last_dot_position << endl;
      string suffix_str=inputstring.substr(
         last_dot_position+1,inputstring.length());
//         cout << "suffix_str = " << suffix_str << endl;
      return stringfunc::remove_trailing_whitespace(suffix_str);
   }

// ---------------------------------------------------------------------
// Method capitalize_word() changes the first letter within the
// input string to its capitalized form.  

   string capitalize_word(string word)
   {
      const int ascii_a=97;
      const int ascii_z=122;
      const int ascii_A=65;
//         const int ascii_Z=90;

      char first_char=string_to_char(word);
      int first_char_ascii=char_to_ascii_integer(first_char);
      if (first_char_ascii < ascii_a || first_char_ascii > ascii_z) 
         return word;

      first_char_ascii += ascii_A-ascii_a;
      char capitalized_first_char=ascii_integer_to_char(first_char_ascii);
      string capitalized_first_char_str=
         char_to_string(capitalized_first_char);
      string capitalized_word=capitalized_first_char_str+
         word.substr(1,word.length()-1);
      return capitalized_word;
   }

// ---------------------------------------------------------------------
// Method capitalize_just_first_letter() takes in a word.  It returns
// the word with only its first letter capitalized and all other
// letters in lower case.

string capitalize_just_first_letter(string word)
{
   const int ascii_a=97;
   const int ascii_z=122;
   const int ascii_A=65;
   const int ascii_Z=90;

   string capitalized_first_char_str=word.substr(0,1);
   char first_char=stringfunc::string_to_char(word);
   int first_char_ascii=stringfunc::char_to_ascii_integer(first_char);
   if (first_char_ascii >= ascii_a && first_char_ascii <= ascii_z) 
   {
      first_char_ascii += ascii_A-ascii_a;
      char capitalized_first_char=stringfunc::ascii_integer_to_char(
         first_char_ascii);
      capitalized_first_char_str=stringfunc::char_to_string(
         capitalized_first_char);
   }
   string reformed_word = capitalized_first_char_str;

   for(unsigned int i = 1; i < word.size() - 1; i++)
   {
      string curr_char_str = word.substr(i,1);
      char curr_char = stringfunc::string_to_char(curr_char_str);
      int curr_char_ascii=stringfunc::char_to_ascii_integer(curr_char);
      if(curr_char_ascii >= ascii_A && curr_char_ascii <= ascii_Z)
      {
         curr_char_ascii += ascii_a - ascii_A;
         char lower_curr_char = stringfunc::ascii_integer_to_char(
            curr_char_ascii);
         curr_char_str = stringfunc::char_to_string(lower_curr_char);
      }
      reformed_word += curr_char_str;
   }

   return reformed_word;
}

// ---------------------------------------------------------------------
// Method edit_distance() computes the "Levenshtein distance" between
// two strings.  It returns the total number of character deletions,
// insertions or substitutions required to transform a source string
// into a target string.

   unsigned int edit_distance(int i, int j)
   {
      string s = stringfunc::number_to_string(i);
      string t = stringfunc::number_to_string(j);
      return edit_distance(s,t);
   }

   unsigned int edit_distance(string s, string t)
   {

// First check for degenerate cases:

      if (s==t) return 0;
      if (s.size()==0) return t.size();
      if (t.size()==0) return s.size();

// Create two work vectors of integer distances:

      vector<unsigned int> v0,v1;
      v0.reserve(t.size()+1);
      v1.reserve(t.size()+1);

// Initialize v0 (previous row of distances)
// Edit distance for an empty s is just the number of characters
// to delete from t      

      for (unsigned int i=0; i<t.size()+1; i++)
      {
         v0[i]=i;
      }

      for (unsigned int i=0; i<s.size(); i++)
      {

// Calculate v1 (current row distances) from the previous row v0.
// Edit distance is i+1 characters to delete from s to match empty t

         v1[0]=i+1;
         
         for (unsigned int j=0; j<t.size(); j++)
         {
            unsigned int cost=1;
            if (s[i]==t[j]) cost = 0;
            v1[j+1] = basic_math::min(
               basic_math::min(v1[j]+1, v0[j+1]+1), v0[j]+cost);
         }

// Copy v1 (current row) to v0 (previous row) for next iteration         

         for (unsigned int j=0; j<t.size()+1; j++)
         {
            v0[j]=v1[j];
         }

      } // loop over index i 
      
      return v1[t.size()];
   }

// ==========================================================================
// Word stemming methods
// ==========================================================================

   string wstr_to_str(std::wstring wstr)
   {
      string str;
      str.assign(wstr.begin(),wstr.end());
      return str;
   }

   std::wstring str_to_wstr(string str)
   {
      std::wstring wstr;
      wstr.assign(str.begin(),str.end());
      return wstr;
   }

// ---------------------------------------------------------------------   

   string stem_word(string input_word)
   {

// As of Jan 2016, Oleander Stemming appears to compile OK under
// compile correctly under gcc-4.8.1/gcc-4.8.2

      std::wstring wstr=str_to_wstr(input_word);
      stemming::english_stem<> StemEnglish;
      StemEnglish(wstr);
      
      string stemmed_word=wstr_to_str(wstr);
      return stemmed_word;
   }
   

   
} // stringfunc namespace



