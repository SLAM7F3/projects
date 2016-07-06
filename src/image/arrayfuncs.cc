// ==========================================================================
// ARRAYFUNCS stand-alone methods
// ==========================================================================
// Last modified on 11/27/12; 1/11/13; 1/25/13
// ==========================================================================

#include <iostream>
#include <vector>

#include "general/filefuncs.h"
#include "general/stringfuncs.h"
#include "image/TwoDarray.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

namespace arrayfunc
{

// Method parse_CSV_file() imports a comma-separated-value file which
// is assumed to be filled with a rectangular array of
// double-precision numbers.  It instantiates and returns
// *ztwoDarray_ptr which is filled with the parsed values.

   twoDarray* parse_CSV_file(string input_filename)
   {
      filefunc::ReadInfile(input_filename);

      string separator_chars=",";
      int n_columns=-1;
      int n_rows=0;
      for (unsigned int i=0; i<filefunc::text_line.size(); i++)
      {
//         cout << filefunc::text_line[i] << endl;

         vector<double> column_values=stringfunc::string_to_numbers(
            filefunc::text_line[i],separator_chars);
         if (n_columns < 0)
         {
            n_columns=column_values.size();
         }
         else
         {
            if (n_columns != int(column_values.size()))
            {
               cout << "Error in arrayfunc::parse_CSV_file()" << endl;
               cout << "Number of columns is not constant throughout CSV file..." << endl;
               exit(-1);
            }
         }
         n_rows++;
      }

//      cout << "n_columns = " << n_columns
//           << " n_rows = " << n_rows << endl;

      twoDarray* ztwoDarray_ptr=new twoDarray(n_columns,n_rows);
//      cout << "*ztwoDarray_ptr = " << *ztwoDarray_ptr << endl;

      for (int py=0; py<n_rows; py++)
      {
         string curr_line=filefunc::text_line[py];
         vector<double> column_values=stringfunc::string_to_numbers(
            curr_line,separator_chars);
         for (int px=0; px<n_columns; px++)
         {
            ztwoDarray_ptr->put(px,py,column_values[px]);
//            cout << "px = " << px << " py = " << py 
//                 << " z = " << ztwoDarray_ptr->get(px,py) << endl;
         }
      }

      return ztwoDarray_ptr;
   }

// --------------------------------------------------------------------------
// Method parse_binary_shorts_file() imports a binary file which is
// assumed to be filled with xdim*ydim 2-byte unsigned short integers.  It
// instantiates a twoDarray and fills it with values read in from the
// binary file after they are multiplied by magnitude_factor.  The
// dynamically instantiated twoDarray is returned by this method.

   twoDarray* parse_binary_shorts_file(
      int xdim,int ydim,string input_filename,double magnitude_factor)
   {

// Bunzip2 input file if necessary:

      if (stringfunc::suffix(input_filename)=="bz2")
      {
         filefunc::bunzip2_file(input_filename);
         input_filename=stringfunc::prefix(input_filename);
      }
//      cout << "input_filename = " << input_filename << endl;

      ifstream binary_instream;
      filefunc::open_binaryfile(input_filename,binary_instream);

      twoDarray* qtwoDarray_ptr=new twoDarray(xdim,ydim);
      qtwoDarray_ptr->clear_values();

//      double q_min=POSITIVEINFINITY;
//      double q_max=NEGATIVEINFINITY;
      
      unsigned short q_short;
      for (int py=0; py<ydim; py++)
      {
         for (int px=0; px<xdim; px++)
         {
            filefunc::readobject(binary_instream,q_short);
            double q=magnitude_factor*q_short;
            qtwoDarray_ptr->put(px,py,q);
//            q_min=basic_math::min(q_min,q);
//            q_max=basic_math::max(q_max,q);
         }
      }
      binary_instream.close();

//      cout << "q_min = " << q_min << " q_max = " << q_max << endl;

      return qtwoDarray_ptr;
   }
   
} // arrayfuncs namespace





