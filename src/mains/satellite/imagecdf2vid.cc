// ==========================================================================
// Program IMAGECDF2VID reads in an imagecdf file containing ISAR
// imagery.  It parses the header information within this file and
// converts it RCS values into a Group 99 video file format which can
// be displayed using our OSG video player program VIDEO.

// 			imagecdf2vid SJ7.imagecdf

//			imagecdf2vid RH2.imagecdf

// ==========================================================================
// Last updated on 4/27/06; 7/18/06; 7/28/06
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
    bool regularize_images=true;
//   bool regularize_images=false;	 // for debugging only...
   if (pass.initialize_pass(filename,regularize_images))
   {

// Determine pass geometry:
   
      pass.determine_pass_handedness();
      pass.plot_target_orbit_geometry_wrt_sensor();

//      pass.summarize_results();
//      pass.update_logfile();

      bool delete_grey_video_flag=true;
      bool equalize_intensity_histogram_flag=false;
      pass.write_videofile(
         delete_grey_video_flag,equalize_intensity_histogram_flag);
      
      outputfunc::write_program_finished_message();
   } // initialize_pass conditional
}

   





