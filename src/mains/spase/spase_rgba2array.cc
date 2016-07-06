// ==========================================================================
// Program SPASE_RGBA2ARRAY reads in a sequence of spase XYZRGBA files
// and writes out a single color array file for subsequent 4D movie
// playback via program SATDRAPE.  The starting, stopping and
// increment 3D imagenumber values are also specified below.

// This program's input arguments are hardwired below.  So simply chant

// 		rgba2array ~cho/programs/c++/svn/projects/src/mains/spase/xyzp_files/output1.xyzrgba

// from within src/mains/spase.
// ==========================================================================
// Last updated on 4/6/06; 6/8/06; 11/6/06; 4/23/07
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "general/outputfuncs.h"
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
   
// Read input ladar point cloud file:
   
   PassesGroup passes_group(&arguments);
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();

// Instantiate group to hold pointcloud information:

   PointCloudsGroup clouds_group(
      passes_group.get_pass_ptr(cloudpass_ID));

   string subdir="./xyzp_files/";
   string prefix="output";
   string rgba_suffix=".xyzrgba";
   
   string color_arrays_filename="color_arrays.rgba";
   FILE* fp_out = fopen(color_arrays_filename.c_str(), "wb");

//   int imagenumber_start=0;
//   int imagenumber_stop=44;
   int imagenumber_start=1;
   int imagenumber_stop=9;
   int imagenumber_skip=1;
   for (int imagenumber=imagenumber_start; imagenumber <= imagenumber_stop;
        imagenumber += imagenumber_skip)
   {
      const int ndigits=1;
//      const int ndigits=2;
      string imagenumber_str=
         stringfunc::integer_to_string(imagenumber,ndigits);
      string xyzrgba_filename=subdir+prefix+imagenumber_str+rgba_suffix;
//      cout << "xyzrgba_filename = " << xyzrgba_filename << endl;

      string banner="Extracting colors from XYZRGBA file "+
         xyzrgba_filename;
      outputfunc::write_big_banner(banner);

      PointCloud* cloud_ptr=clouds_group.generate_new_Cloud();
      int color_array_size=4*cloud_ptr->get_npoints();
      cloud_ptr->parse_input_data();

      fwrite( &((* (cloud_ptr->get_colors_ptr()) )[0]), 
              color_array_size,1,fp_out);
  
      delete cloud_ptr;
   } // loop over imagenumber index
   fclose(fp_out);
}
