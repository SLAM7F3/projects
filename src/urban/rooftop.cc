// ==========================================================================
// ROOFTOP base class member function definitions
// ==========================================================================
// Last modified on 6/21/05; 7/29/06; 7/30/06
// ==========================================================================

#include <iostream>
#include <sstream>
#include "math/constants.h"
#include "math/constant_vectors.h"
#include "general/filefuncs.h"
#include "geometry/linesegment.h"
#include "urban/rooftop.h"

using std::cout;
using std::endl;
using std::istringstream;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void rooftop::allocate_member_objects()
{
}		       

void rooftop::initialize_member_objects()
{
   none_flag=true;
   flat_flag=pyramid_flag=false;
   COM=Zero_vector;
   spine_ptr=NULL;
}		       

rooftop::rooftop()
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

// Copy constructor:

rooftop::rooftop(const rooftop& r)
{
   initialize_member_objects();
   allocate_member_objects();
   docopy(r);
}

rooftop::~rooftop()
{
//   cout << "inside rooftop destructor" << endl;
   delete spine_ptr;
//   cout << "After call to delete spine_ptr" << endl;
   spine_ptr=NULL;
}

// ---------------------------------------------------------------------
void rooftop::docopy(const rooftop& r)
{
   none_flag=r.none_flag;
   flat_flag=r.flat_flag;
   pyramid_flag=r.pyramid_flag;
   COM=r.COM;
   *spine_ptr=*(r.spine_ptr);
}

// ---------------------------------------------------------------------
// Overload = operator:

rooftop& rooftop::operator= (const rooftop& r)
{
   if (this==&r) return *this;
   docopy(r);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const rooftop& r)
{
   outstream << endl;
   if (r.spine_ptr != NULL)
   {
      if (r.spine_ptr != NULL)
      {
         outstream << "spine = " << *(r.spine_ptr) << endl;
      }
   }
   return outstream;
}

// =========================================================================
// Text file I/O
// ==========================================================================

ostream& rooftop::write_to_textstream(ostream& textstream)
{
   textstream << none_flag << " # none rooftop flag" << endl;
   textstream << flat_flag << " # flat rooftop flag" << endl;
   textstream << pyramid_flag << " # pyramid rooftop flag" << endl;
   textstream << COM.get(0) << " " << COM.get(1) << " " << COM.get(2) 
              << endl;
   if (spine_ptr != NULL) 
   {
      spine_ptr->write_to_textstream(textstream);
   }
   else
   {
//      textstream << "NULL" << endl;
   }
   return textstream;
}

// ---------------------------------------------------------------------
void rooftop::read_from_text_lines(unsigned int& i,vector<string>& line)
{
   istringstream input_string_stream(line[i++]);
   input_string_stream >> none_flag;
   istringstream input_string_stream2(line[i++]);
   input_string_stream2 >> flat_flag;
   istringstream input_string_stream3(line[i++]);
   input_string_stream3 >> pyramid_flag;

   vector<double> V=stringfunc::string_to_numbers(line[i++]);
   COM=threevector(V[0],V[1],V[2]);

/*
   double x,y,z;
   istringstream input_string_stream4(line[i++]);
   input_string_stream4 >> x;
   input_string_stream4 >> y;
   input_string_stream4 >> z;
   COM.put(0,x);
   COM.put(1,x);
   COM.put(2,x);
*/
 
   if (!none_flag && !flat_flag && !pyramid_flag)
   {
      spine_ptr=new linesegment;
      spine_ptr->read_from_text_lines(i,line);
   }
//   cout << "*this = " << *this << endl;
}
