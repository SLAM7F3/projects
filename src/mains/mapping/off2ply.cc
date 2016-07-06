// ==========================================================================
// Program OFF2PLY imports a polyhedron from some Object File Format
// (.off) file passed as a command line argument.  It converts the
// input .off file into .ply file format.
// ==========================================================================
// Last updated on 1/25/12
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
   string off_filename=input_params[0];
   
   string dirname=filefunc::getdirname(off_filename);
   string basename=filefunc::getbasename(off_filename);
   string ply_filename=dirname+stringfunc::prefix(basename)+".ply";

   polyhedron* polyhedron_ptr=new polyhedron();

   fourvector volume_color;
   polyhedron_ptr->read_OFF_file(off_filename,volume_color);
//   cout << "*polyhedron_ptr = " << *polyhedron_ptr << endl;
   
   polyhedron_ptr->write_PLY_file(ply_filename);
   string banner="Converted input "+off_filename+" to output "+
      ply_filename;
   outputfunc::write_big_banner(banner);

}
