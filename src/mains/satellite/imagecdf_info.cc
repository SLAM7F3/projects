// ==========================================================================
// Program IMAGECDF_INFO reads in an imagecdf file containing ISAR
// imagery.  It parses the header information within this file.

// 			imagecdf_info SJ7.imagecdf

//			imagecdf_info RH2.imagecdf

// ==========================================================================
// Last updated on 7/20/06
// ==========================================================================

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "space/satellitepass.h"
#include "general/sysfuncs.h"
#include "osg/osg3D/Terrain_Manipulator.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::ostream;
   using std::cout;
   using std::ios;
   using std::endl;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

   cout.precision(3);
   cout.setf(ios::showpoint);

// Parse imagecdf filename:

   vector<string> param;
   if (!filefunc::parameter_input(argc,argv,param))
   {
      cout << "Error: Invalid argument passed on command line" << endl;
      exit(-1);
   }
   string filename=param[0];

   string sat_name="simul";
   satellitepass pass(sat_name);
   bool regularize_images=false;	 
   if (pass.initialize_pass(filename,regularize_images))
   {

// Determine pass geometry:
   
      pass.determine_pass_handedness();
      pass.plot_target_orbit_geometry_wrt_sensor();
//      pass.summarize_results();
//      pass.update_logfile();
      outputfunc::write_program_finished_message();
   } // initialize_pass conditional
}

   





