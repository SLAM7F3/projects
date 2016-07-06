// =========================================================================
// Pixel_cc class member function definitions
// =========================================================================
// Last modified on 6/20/16; 6/21/16; 6/22/16
// =========================================================================

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "video/pixel_cc.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ios;
using std::istream;
using std::map;
using std::ofstream;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void pixel_cc::allocate_member_objects()
{
}

void pixel_cc::initialize_member_objects()
{
   rejected_flag = false;
   score = -1;
   px_center = py_center = -1;
}		 

// ---------------------------------------------------------------------
pixel_cc::pixel_cc(int ID, int class_ID)
{
   this->ID = ID;
   this->class_ID = class_ID;
   initialize_member_objects();
   allocate_member_objects();
}

// ---------------------------------------------------------------------
// Copy constructor:

pixel_cc::pixel_cc(const pixel_cc& cc) 
{
//   cout << "inside pixel_cc copy constructor, this(pixel_cc) = " << this << endl;
   initialize_member_objects();
   allocate_member_objects();
   docopy(cc);
}

pixel_cc::~pixel_cc()
{
//   cout << "inside pixel_cc destructor" << endl;
}

// ---------------------------------------------------------------------
void pixel_cc::docopy(const pixel_cc& cc)
{
//   cout << "inside pixel_cc::docopy()" << endl;
//   cout << "this = " << this << endl;
   rejected_flag = cc.rejected_flag;
   ID = cc.ID;
   class_ID = cc.class_ID;
   imagesize = cc.imagesize;
   score = cc.score;
   score_threshold = cc.score_threshold;
   px_center = cc.px_center;
   py_center = cc.py_center;
   bbox = cc.bbox;

   for(unsigned int p = 0; p < cc.pixel_coords.size(); p++)
   {
      pixel_coords.push_back(cc.pixel_coords[p]);
   }
}

// Overload = operator:

pixel_cc& pixel_cc::operator= (const pixel_cc& cc)
{
//   cout << "inside pixel_cc::operator=" << endl;
//   cout << "this(pixel_cc) = " << this << endl;
   if (this==&cc) return *this;
   docopy(cc);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const pixel_cc& cc)
{
   outstream << "Pixel_CC:  ID = " << cc.ID 
             << " class_ID = " << cc.class_ID
             << " n_pixels = " << cc.pixel_coords.size()
             << " rejected_flag = " << cc.rejected_flag
             << " score = " << cc.score
             << endl;
   outstream << " Bbox = " << cc.bbox << endl;
   return outstream;
}

// =========================================================================
// Set & get member functions
// =========================================================================
