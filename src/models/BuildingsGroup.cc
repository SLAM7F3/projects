// ==========================================================================
// BuildingsGroup class member function definitions
// ==========================================================================
// Last modified on 3/13/12; 3/14/12; 4/24/12; 4/5/14
// ==========================================================================

#include <iostream>
#include "models/Building.h"
#include "models/BuildingsGroup.h"
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

void BuildingsGroup::allocate_member_objects()
{
   Building_polyhedron_map_ptr=new BUILDING_POLYHEDRON_MAP;
   ClippedPolygon_building_polyhedron_rectangle_map_ptr=new
      CLIPPEDPOLYGON_BUILDING_POLYHEDRON_RECTANGLE_MAP;
}		       

void BuildingsGroup::initialize_member_objects()
{
   ID=0;
}

BuildingsGroup::BuildingsGroup() 
{
   allocate_member_objects();
   initialize_member_objects();
}

BuildingsGroup::~BuildingsGroup()
{
   delete Building_polyhedron_map_ptr;
   delete ClippedPolygon_building_polyhedron_rectangle_map_ptr;
}

// ---------------------------------------------------------------------
// Member function import_from_OFF_files() loops over all Object File
// Format (OFF) files within OFF_subdir.  It extracts building and
// polyhedron IDs from each OFF file.  This method instantiates all
// building polyhedra, and it stores their correlated integer IDs
// within STL map member *Building_polyhedron_map_ptr.

void BuildingsGroup::import_from_OFF_files(string OFF_subdir)
{
   cout << "inside BuildingsGroup::import_from_OFF_files()" << endl;

   string prefix="building_";
   vector<string> building_filenames=
      filefunc::files_in_subdir_matching_substring(OFF_subdir,prefix);
   string separator_chars="_";

// First establish maximum building ID based upon input OFF filenames:

   int max_bldg_ID=-1;
   for (unsigned int f=0; f<building_filenames.size(); f++)
   {
      string basename=filefunc::getbasename(building_filenames[f]);
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         basename,separator_chars);
      int bldg_ID=stringfunc::string_to_number(substrings[1]);
      max_bldg_ID=basic_math::max(bldg_ID,max_bldg_ID);
   } // loop over index f
   cout << " max_bldg_ID = " << max_bldg_ID << endl;
   unsigned int n_buildings=max_bldg_ID+1;

// Next instantiate each building and their polyhedra:

   for (unsigned int ID=0; ID < n_buildings; ID++)
   {
      Building* Building_ptr=new Building();
      Building_ptrs.push_back(Building_ptr);
      Building_ptr->import_from_OFF_files(ID,OFF_subdir);
      vector<polyhedron*> polyhedra_ptrs=Building_ptr->get_polyhedra_ptrs();
      
      cout << "polyhedra_ptrs.size() = " 
           << polyhedra_ptrs.size() << endl;
      for (unsigned int p=0; p<polyhedra_ptrs.size(); p++)
      {
         polyhedron* curr_polyhedron_ptr=polyhedra_ptrs.at(p);
         twovector curr_building_polyhedron_ids(
            ID,curr_polyhedron_ptr->get_ID());
         (*Building_polyhedron_map_ptr)[curr_building_polyhedron_ids]=
            curr_polyhedron_ptr;
      }
      
      cout << "Building = " << *Building_ptr << endl;
   }
}

// ---------------------------------------------------------------------
// Member function compute_primary_Building_colors()

void BuildingsGroup::compute_primary_Building_colors()
{
   cout << "inside BuildingsGroup::compute_primary_Building_colors()" << endl;
   
   for (unsigned int i=0; i<Building_ptrs.size(); i++)
   {
      Building_ptrs[i]->compute_primary_color();
   }

}
