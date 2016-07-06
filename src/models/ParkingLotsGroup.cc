// ==========================================================================
// ParkingLotsGroup class member function definitions
// ==========================================================================
// Last modified on 4/21/12; 4/5/14
// ==========================================================================

#include <iostream>
#include "models/ParkingLot.h"
#include "models/ParkingLotsGroup.h"
#include "general/filefuncs.h"
#include "geometry/polyhedron.h"
#include "general/stringfuncs.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void ParkingLotsGroup::allocate_member_objects()
{
   ParkingLot_polyhedron_map_ptr=new PARKINGLOT_POLYHEDRON_MAP;
}		       

void ParkingLotsGroup::initialize_member_objects()
{
   ID=0;
}

ParkingLotsGroup::ParkingLotsGroup() 
{
   allocate_member_objects();
   initialize_member_objects();
}

ParkingLotsGroup::~ParkingLotsGroup()
{
   delete ParkingLot_polyhedron_map_ptr;
}

// ---------------------------------------------------------------------
// Member function import_from_OFF_files() loops over all Object File
// Format (OFF) files within OFF_subdir.  It extracts ParkingLot and
// polyhedron IDs from each OFF file.  This method instantiates all
// ParkingLot polyhedra, and it stores their correlated integer IDs
// within STL map member *ParkingLot_polyhedron_map_ptr.

void ParkingLotsGroup::import_from_OFF_files(string OFF_subdir)
{
//   cout << "inside ParkingLotsGroup::import_from_OFF_files()" << endl;

   string prefix="parking_";
   vector<string> ParkingLot_filenames=
      filefunc::files_in_subdir_matching_substring(OFF_subdir,prefix);
   string separator_chars="_";

// First establish maximum ParkingLot ID based upon input OFF filenames:

   int max_lot_ID=-1;
   for (unsigned int f=0; f<ParkingLot_filenames.size(); f++)
   {
      string basename=filefunc::getbasename(ParkingLot_filenames[f]);
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         basename,separator_chars);
      int lot_ID=stringfunc::string_to_number(substrings[1]);
      max_lot_ID=basic_math::max(lot_ID,max_lot_ID);
   } // loop over index f
//   cout << " max_lot_ID = " << max_lot_ID << endl;
   unsigned int n_ParkingLots=max_lot_ID+1;

// Next instantiate each ParkingLot and their polyhedra:

   for (unsigned int ID=0; ID < n_ParkingLots; ID++)
   {
      ParkingLot* ParkingLot_ptr=new ParkingLot();
      ParkingLot_ptrs.push_back(ParkingLot_ptr);
      ParkingLot_ptr->import_from_OFF_files(ID,OFF_subdir);
      vector<polyhedron*> polyhedra_ptrs=ParkingLot_ptr->get_polyhedra_ptrs();
      
      for (unsigned int p=0; p<polyhedra_ptrs.size(); p++)
      {
         polyhedron* curr_polyhedron_ptr=polyhedra_ptrs.at(p);
         twovector curr_ParkingLot_polyhedron_ids(
            ID,curr_polyhedron_ptr->get_ID());
         (*ParkingLot_polyhedron_map_ptr)[curr_ParkingLot_polyhedron_ids]=
            curr_polyhedron_ptr;
      }
      
//      cout << "ID = " << ID 
//           << " ParkingLot = " << *ParkingLot_ptr << endl;
   }
}
