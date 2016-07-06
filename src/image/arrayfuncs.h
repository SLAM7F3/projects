// Note added on 12/27/12: We should move get_pixel_ID() and
// get_pixel_px_py() out of graphicsfuncs namespace into arrayfuncs

// =========================================================================
// Header file for stand-alone array functions.
// =========================================================================
// Last modified on 11/27/12; 1/11/12
// =========================================================================

#ifndef ARRAYFUNCS_H
#define ARRAYFUNCS_H

#include <string>

template <class T> class TwoDarray;
typedef TwoDarray<double> twoDarray;

namespace arrayfunc
{

   twoDarray* parse_CSV_file(std::string input_filename);
   twoDarray* parse_binary_shorts_file(
      int xdim,int ydim,std::string input_filename,double magnitude_factor);

}

#endif // arrayfuncs.h




