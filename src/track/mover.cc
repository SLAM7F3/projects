// ==========================================================================
// Mover class member function definitions
// ==========================================================================
// Last modified on 9/29/08; 12/5/08; 2/13/09
// ==========================================================================

#include "track/mover.h"

using std::cout;
using std::endl;
using std::ostream;
using std::ofstream;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void mover::allocate_member_objects()
{
}		       

void mover::initialize_member_objects()
{
   previously_encountered_flag=false;
   relative_size=-1.0;
   avg_time_duration=-1;
   RGB_color.first=RGB_color.second=RGB_color.third=-1;
   track_ptr=NULL;
   orig_track_ptr=NULL;
}

mover::mover(MoverType t,int i)
{
   allocate_member_objects();
   initialize_member_objects();

   type=t;
   ID=i;
}

// Copy constructor:

mover::mover(const mover& m)
{
   allocate_member_objects();
   initialize_member_objects();
   docopy(m);
}

mover::~mover()
{
}

// ---------------------------------------------------------------------
void mover::docopy(const mover& m)
{
//   cout << "inside mover::docopy()" << endl;
   type=m.type;
   ID=m.ID;
}

// Overload = operator:

mover& mover::operator= (const mover& m)
{
   if (this==&m) return *this;
   docopy(m);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const mover& m)
{
   outstream << endl;

//   cout << "inside mover::operator<<" << endl;

   if (m.type==mover::VEHICLE)
   {
      outstream << "Type = VEHICLE" << endl;
   }
   else if (m.type==mover::ROI)
   {
      outstream << "Type = ROI" << endl;
   }
   else if (m.type==mover::UAV)
   {
      outstream << "Type = UAV" << endl;
   }
   else if (m.type==mover::KOZ)
   {
      outstream << "Type = KOZ" << endl;
   }
   
   outstream << "ID = " << m.ID << endl;
   if (m.get_annotation_label().size() > 0)
   {
      outstream << "Annotation label = " << m.get_annotation_label()
                << endl;
   }
   
   return(outstream);
}

// =====================================================================

