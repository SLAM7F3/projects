
// As of July 03, this ancient class is deprecated and no longer
// supported.

// ==========================================================================
// Header file for ANGLEARRAY class.  This class was cooked up for our
// towed decoy vulnerability study.  It essentially just contains a
// 2-dimensional array of projected areas as a function of elevation
// and azimuthal angles el and az.  Member function routines to read
// in and write out az-el data to and from RCS files are included.
// Brian Kavanagh's interpolation routine is also enclosed as a member
// function of this class.
// ==========================================================================
// Last modified on 3/1/03
// ==========================================================================

#ifndef ANGLEARRAY_H
#define ANGLEARRAY_H

#include "general/outputfuncs.h"

class anglearray
{
  private:

  public:

// Note added on 2/16/00: String ANSI C++ does not allow one to define
// constant variables within header files.  So we have hardwired in
// the following numerical constants:

//   const int NAZ_COLUMNS=2*721;
//  const int NEL_ROWS=2*181;

   int az_num_steps,el_num_steps;
   double delta_az,delta_el;
   double az_first,az_last;
   double el_first,el_last;
   double (*angle_value_array)[2*721];

// ---------------------------------------------------------------------
// Constructor functions:
// ---------------------------------------------------------------------

   anglearray(void);
   anglearray(const anglearray& a);
   ~anglearray(void);

// ---------------------------------------------------------------------
// Member functions:
// ---------------------------------------------------------------------

   void docopy(const anglearray& a);
   anglearray& operator= (const anglearray& a);
   void read_rcs_file(std::string filename);
   void read_rcs_file(std::ifstream& instream);
   void write_rcs_file(std::string filename);
   double interpangles(double az_input,double el_input);
};

#endif // anglearray.h












