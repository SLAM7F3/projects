// =========================================================================
// Sift_Feature class member function definitions
// =========================================================================
// Last modified on 3/17/11
// =========================================================================

#include <iostream>
#include "video/sift_feature.h"
#include "general/stringfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void sift_feature::allocate_member_objects()
{
}

void sift_feature::initialize_member_objects()
{
   photo_ID=-1;
}		 

// ---------------------------------------------------------------------
sift_feature::sift_feature(int ID)
{
   initialize_member_objects();
   allocate_member_objects();

   this->ID=ID;
}

// ---------------------------------------------------------------------
// Copy constructor:

sift_feature::sift_feature(const sift_feature& s)
{
//   cout << "inside sift_feature copy constructor, this(sift_feature) = " << this << endl;
   initialize_member_objects();
   allocate_member_objects();
   docopy(s);
}

sift_feature::~sift_feature()
{
}

// ---------------------------------------------------------------------
void sift_feature::docopy(const sift_feature& s)
{
//   cout << "inside sift_feature::docopy()" << endl;
//   cout << "this = " << this << endl;
}

// Overload = operator:

sift_feature& sift_feature::operator= (const sift_feature& s)
{
//   cout << "inside sift_feature::operator=" << endl;
//   cout << "this(sift_feature) = " << this << endl;
   if (this==&s) return *this;
   docopy(s);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const sift_feature& s)
{
   outstream << endl;
   outstream << "Sift_Feature ID = " << s.ID << endl;
   outstream << "u = " << s.u << " v = " << s.v 
             << " orientation = " << s.orientation
             << " scale = " << s.scale << endl;
   for (unsigned int i=0; i<s.descriptor.size(); i++)
   {
      outstream << s.descriptor[i] << " ";
   }
   outstream << endl << endl;
   
   return outstream;
}

// =========================================================================

void sift_feature::set_descriptor(const vector<double>& values)
{
   descriptor.clear();
   for (unsigned int v=0; v<values.size(); v++)
   {
      descriptor.push_back(values[v]);
   }
}

string sift_feature::get_descriptor_as_string() const
{
   string descriptor_str;
   for (unsigned int v=0; v<descriptor.size(); v++)
   {
      double curr_value=descriptor[v];
      descriptor_str += stringfunc::number_to_string(curr_value)+" ";
   }
   return descriptor_str;
}
