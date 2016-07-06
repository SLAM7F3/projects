// ==========================================================================
// ParkingLot class member function definitions
// ==========================================================================
// Last modified on 4/21/12; 4/5/14
// ==========================================================================

#include <string>
#include "models/ParkingLot.h"
#include "general/filefuncs.h"
#include "geometry/polyhedron.h"

using std::cout;
using std::endl;
using std::map;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void ParkingLot::allocate_member_objects()
{
//   cout << "inside ParkingLot::allocate_member_objects()" << endl;
   
}		       

void ParkingLot::initialize_member_objects()
{
   ID=-1;
   ground_z=POSITIVEINFINITY;
}

ParkingLot::ParkingLot() 
{
   allocate_member_objects();
   initialize_member_objects();
}

ParkingLot::~ParkingLot()
{
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,ParkingLot& PL)
{
   outstream << endl;
   outstream << "ParkingLot ID = " << PL.get_ID() << endl;
   for (unsigned int p=0; p<PL.polyhedra_ptrs.size(); p++)
   {
      cout << "Polyhedron p = " << p << endl;
      cout << *(PL.polyhedra_ptrs[p]) << endl;
   }
   
   return outstream;
}

// ==========================================================================
// Member function import_from_OFF_files() parses Object File Format
// (OFF) files for the particular building labeled by input ID.  It
// instantiates its polyhedra parts and returns their IDs within an
// output STL vector.

void ParkingLot::import_from_OFF_files(int ID,string OFF_subdir)
{
//   cout << "inside ParkingLot::import_from_OFF_files()" << endl;
   
   this->ID=ID;

   string prefix="parking_"+stringfunc::number_to_string(ID);
   vector<string> ParkingLot_polyhedra_filenames=
      filefunc::files_in_subdir_matching_substring(OFF_subdir,prefix);

   for (unsigned int f=0; f<ParkingLot_polyhedra_filenames.size(); f++)
   {
//      cout << "f = " << f
//           << " ParkingLot polyhedron filename = "
//           << ParkingLot_polyhedra_filenames[f] << endl;

      polyhedron* polyhedron_ptr=new polyhedron(f);
      polyhedra_ptrs.push_back(polyhedron_ptr);

      fourvector polyhedron_color;
      polyhedron_ptr->read_OFF_file(
         ParkingLot_polyhedra_filenames[f],polyhedron_color);

// Save minimal vertex Z value within ground_z
// member variable:

      unsigned int n_vertices=polyhedron_ptr->get_n_vertices();
      for (unsigned int v=0; v<n_vertices; v++)
      {
         vertex curr_vertex=polyhedron_ptr->get_vertex(v);
         ground_z=basic_math::min(ground_z,curr_vertex.get_posn().get(2));
      }

   } // loop over index f
//   cout << "ground_z = " << ground_z << endl;
}
