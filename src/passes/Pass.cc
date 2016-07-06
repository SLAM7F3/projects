// ==========================================================================
// Pass class member function definitions
// ==========================================================================
// Last updated on 5/14/08; 7/2/08; 9/7/08; 3/8/09; 4/5/14
// ==========================================================================

#include "passes/Pass.h"
#include "general/stringfuncs.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:

void Pass::allocate_member_objects()
{
}

void Pass::initialize_member_objects()
{
   pass_type=Pass::other;	// Should probably set this as default value
   PassInfo_ptr=NULL;
   input_filetype=unknown;
   northern_hemisphere_flag=false;
   UTM_zonenumber=-1;
}

Pass::Pass()
{
   allocate_member_objects();
   initialize_member_objects();
}

Pass::Pass(int id)
{
   allocate_member_objects();
   initialize_member_objects();
   ID=id;
}

Pass::~Pass()
{
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const Pass& p)
{
   outstream << endl;
   outstream << "ID = " << p.get_ID() << endl;
   outstream << "First filename = " << p.get_first_filename() << endl;
   outstream << "pass type = " ;
   if (p.get_passtype()==Pass::cloud)
   {
      outstream << "point cloud" << endl;
   }
   else if (p.get_passtype()==Pass::video)
   {
      outstream << "video" << endl;
   }
   else if (p.get_passtype()==Pass::earth)
   {
      outstream << "earth" << endl;
   }
   else if (p.get_passtype()==Pass::surface_texture)
   {
      outstream << "surface texture" << endl;
   }
   else if (p.get_passtype()==Pass::GIS_layer)
   {
      outstream << "GIS layer" << endl;
   }
   else if (p.get_passtype()==Pass::dataserver)
   {
      outstream << "data server" << endl;
   }
   else if (p.get_passtype()==Pass::sensor_metadata)
   {
      outstream << "sensor metadata" << endl;
   }
   else if (p.get_passtype()==Pass::dted)
   {
      outstream << "dted" << endl;
   }
   else
   {
      outstream << "unknown" << endl;
   }

   outstream << "Passinfo:" << *(p.get_PassInfo_ptr()) << endl << endl;
   return outstream;
}

// ---------------------------------------------------------------------
// Set & get member functions:

// Member function get_passname_prefix retrieves the name of the file
// containing the raw pass data.  It strips off this file's suffix and
// returns its prefix.

string Pass::get_passname_prefix() const
{
//   cout << "inside Pass::get_passname_prefix()" << endl;
   return stringfunc::prefix(get_first_filename());
}

// ==========================================================================
