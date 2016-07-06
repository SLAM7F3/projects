// ==========================================================================
// Program P2RGBA reads in a sequence of XYZP files and writes them
// back out as XYZRGBA files.  The colormap index and dependent
// variable are hardwired within this program.  The starting, stopping
// and increment 3D imagenumber values are also specified below.  

// This program's input arguments are hardwired below.  So simply
// chant

// 			  	p2rgba 

// from within src/mains/satellite.
// ==========================================================================
// Last updated on 12/13/06; 12/19/06; 4/23/07
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "passes/PassesGroup.h"
#include "osg/osg3D/PointCloud.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);

// Initialize constants and parameters read in from command line as
// well as ascii text file:

// Read input ladar point cloud file:

   PassesGroup passes_group(&arguments);
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();

// Instantiate group to hold pointcloud information:

   PointCloudsGroup clouds_group(passes_group.get_pass_ptr(cloudpass_ID));
   PointCloud* cloud_ptr=clouds_group.generate_new_Cloud();

   string subdir="./xyzp_files/";
   string prefix="instant_sunny_";
   string suffix=".xyzp";
   string rgba_suffix=".xyzrgba";
   
   int imagenumber_start=0;
   int imagenumber_stop=48;
   int imagenumber_skip=1;
   for (int imagenumber=imagenumber_start; imagenumber <= imagenumber_stop;
        imagenumber += imagenumber_skip)
   {
      const int ndigits=2;
      string imagenumber_str=
         stringfunc::integer_to_string(imagenumber,ndigits);

      string banner="Converting p to rgba for image "+imagenumber_str;
      outputfunc::write_big_banner(banner);

      string xyzp_filename=subdir+prefix+imagenumber_str+suffix;
//      cout << "xyzp_filename = " << xyzp_filename << endl;

      cloud_ptr->parse_input_data();
      clouds_group.change_color_map(0);
      const double min_threshold=0.25;
      clouds_group.set_min_threshold(min_threshold);

      string xyzrgba_filename=prefix+imagenumber_str;
      cloud_ptr->write_XYZRGBA_file(xyzrgba_filename,subdir);

   } // loop over imagenumber index
}
