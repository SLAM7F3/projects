// ==========================================================================
// Program RECONSTRUCTED_POSNS reads in the XYZ bundler locations for
// all reconstructed camera from
// bundler_IO_subdir/reconstructed_camera_posns.dat.  It generates a
// TDP file containing the point cloud corresponding to all the
// reconstructed camera positions.
// ==========================================================================
// Last updated on 5/18/12; 5/23/12; 2/28/13
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "osg/osg3D/tdpfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   string bundler_IO_subdir="./bundler/Pushcart/";
   string camera_posns_filename=bundler_IO_subdir+
      "reconstructed_camera_posns.dat";
   filefunc::ReadInfile(camera_posns_filename);
   cout << "filefunc::text_line.size() = " << filefunc::text_line.size()
        << endl;

   vector<threevector> XYZ;
   double max_X,max_Y,max_Z,min_X,min_Y,min_Z;
   max_X=max_Y=max_Z=-1E15;
   min_X=min_Y=min_Z=1E15;

   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);
      double x=stringfunc::string_to_number(substrings[2]);
      double y=stringfunc::string_to_number(substrings[3]);
      double z=stringfunc::string_to_number(substrings[4]);

      min_X=basic_math::min(x,min_X);
      max_X=basic_math::max(x,max_X);
      min_Y=basic_math::min(y,min_Y);
      max_Y=basic_math::max(y,max_Y);
      min_Z=basic_math::min(z,min_Z);
      max_Z=basic_math::max(z,max_Z);
      
      XYZ.push_back(threevector(x,y,z));
   }

   cout << "min_X = " << min_X << " max_X = " << max_X << endl;
   cout << "min_Y = " << min_Y << " max_Y = " << max_Y << endl;
   cout << "min_Z = " << min_Z << " max_Z = " << max_Z << endl;

   string tdp_filename="reconstructed_camera_posns.tdp";
   tdpfunc::write_relative_xyz_data(tdp_filename,XYZ);
}
