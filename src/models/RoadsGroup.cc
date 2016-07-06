// ==========================================================================
// RoadsGroup class member function definitions
// ==========================================================================
// Last modified on 4/21/12; 4/5/14
// ==========================================================================

#include <iostream>
#include "models/Road.h"
#include "models/RoadsGroup.h"
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

void RoadsGroup::allocate_member_objects()
{
   Road_polyhedron_map_ptr=new ROAD_POLYHEDRON_MAP;
}		       

void RoadsGroup::initialize_member_objects()
{
   ID=0;
}

RoadsGroup::RoadsGroup() 
{
   allocate_member_objects();
   initialize_member_objects();
}

RoadsGroup::~RoadsGroup()
{
   delete Road_polyhedron_map_ptr;
}

// ---------------------------------------------------------------------
// Member function import_from_OFF_files() loops over all Object File
// Format (OFF) files within OFF_subdir.  It extracts Road and
// polyhedron IDs from each OFF file.  This method instantiates all
// Road polyhedra, and it stores their correlated integer IDs
// within STL map member *Road_polyhedron_map_ptr.

void RoadsGroup::import_from_OFF_files(string OFF_subdir)
{
//   cout << "inside RoadsGroup::import_from_OFF_files()" << endl;
//   cout << "OFF_subdir = " << OFF_subdir << endl;

   string prefix="roads_";
   vector<string> Road_filenames=
      filefunc::files_in_subdir_matching_substring(OFF_subdir,prefix);
   string separator_chars="_";

// First establish maximum Road ID based upon input OFF filenames:

   int max_road_ID=-1;
   for (unsigned int f=0; f<Road_filenames.size(); f++)
   {
      string basename=filefunc::getbasename(Road_filenames[f]);
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         basename,separator_chars);
      int road_ID=stringfunc::string_to_number(substrings[1]);
      max_road_ID=basic_math::max(road_ID,max_road_ID);
   } // loop over index f
//   cout << " max_road_ID = " << max_road_ID << endl;
   unsigned int n_Roads=max_road_ID+1;

// Next instantiate each Road and their polyhedra:

   for (unsigned int ID=0; ID < n_Roads; ID++)
   {
      Road* Road_ptr=new Road();
      Road_ptrs.push_back(Road_ptr);
      Road_ptr->import_from_OFF_files(ID,OFF_subdir);
      vector<polyhedron*> polyhedra_ptrs=Road_ptr->get_polyhedra_ptrs();
      
      for (unsigned int p=0; p<polyhedra_ptrs.size(); p++)
      {
         polyhedron* curr_polyhedron_ptr=polyhedra_ptrs.at(p);
         twovector curr_Road_polyhedron_ids(
            ID,curr_polyhedron_ptr->get_ID());
         (*Road_polyhedron_map_ptr)[curr_Road_polyhedron_ids]=
            curr_polyhedron_ptr;
      }
      
//      cout << "ID = " << ID 
//           << " Road = " << *Road_ptr << endl;
   }
}
