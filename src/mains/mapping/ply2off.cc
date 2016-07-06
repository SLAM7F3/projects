// ==========================================================================
// Program PLY2OFF imports a triangulated polyhedron from some PLY
// file passed as a command line argument.  It converts the
// input .ply file into Object File Format (.off).
// ==========================================================================
// Last updated on 4/20/12
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "geometry/polyhedron.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ofstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);
   
   vector<string> input_params;
   filefunc::parameter_input(argc,argv,input_params);
   string PLY_filename=input_params[0];
   
   string dirname=filefunc::getdirname(PLY_filename);
   string basename=filefunc::getbasename(PLY_filename);
   string OFF_filename=dirname+stringfunc::prefix(basename)+".off";

   polyhedron* polyhedron_ptr=new polyhedron();

   fourvector volume_color;
   polyhedron_ptr->read_PLY_file(PLY_filename);
//   cout << "*polyhedron_ptr = " << *polyhedron_ptr << endl;
   
   polyhedron_ptr->write_OFF_file(OFF_filename);
   string banner="Converted input "+PLY_filename+" to output "+
      OFF_filename;
   outputfunc::write_big_banner(banner);

}
