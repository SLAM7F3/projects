// ==========================================================================
// Program FLOORPLAN is a specialized utility for converting XY ladar
// point measurements collected by the Group76 robot into a TDP output
// file.

// 				floorplan

// ==========================================================================
// Last updated on 12/28/09; 12/30/09
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "passes/PassesGroup.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "osg/osg3D/tdpfuncs.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::ios;
   using std::ofstream;
   using std::ostream;
   using std::string;
   using std::vector;

   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);

/*
   string subdir="./south_lab/";
   string x_readings_filename=subdir+"x_readings.dat";
   string y_readings_filename=subdir+"y_readings.dat";
//   string x_odometry_filename=subdir+"x_odometry.dat";
//   string y_odometry_filename=subdir+"y_odometry.dat";

   vector<double> x_readings,y_readings;
//   vector<double> x_odometry,y_odometry;

   filefunc::ReadInfile(x_odometry_filename);
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      double curr_x=stringfunc::string_to_number(
         filefunc::text_line[i]);
      x_odometry.push_back(curr_x);
   }
   cout << "x_odometry.size() = " << x_odometry.size() << endl;

   filefunc::ReadInfile(y_odometry_filename);
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      double curr_y=stringfunc::string_to_number(
         filefunc::text_line[i]);
      y_odometry.push_back(curr_y);
   }
   cout << "y_odometry.size() = " << y_odometry.size() << endl;

   cout << "Importing x_readings file:" << endl;
   filefunc::ReadInfile(x_readings_filename);
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      double curr_x=stringfunc::string_to_number(
         filefunc::text_line[i]);
      x_readings.push_back(curr_x);
   }
   cout << "x_readings.size() = " << x_readings.size() << endl;

   cout << "Importing y_readings file:" << endl;
   filefunc::ReadInfile(y_readings_filename);
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      double curr_y=stringfunc::string_to_number(
         filefunc::text_line[i]);
      y_readings.push_back(curr_y);
   }
   cout << "y_readings.size() = " << y_readings.size() << endl;

   string output_filename="floorplan_xy.dat";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);
  
   for (int i=0; i<x_readings.size(); i++)
   {
      if (i%10000==0) cout << i/10000 << "  " << flush;
      outstream << x_readings[i] << "   " << y_readings[i] << endl;
   } // loop over index i labeling odometry readings
   cout << endl;

//   int n_readings_per_robot_posn=1085;
//   int l=0;
//   for (int i=0; i<x_odometry.size(); i++)
//   {
//      if (i%100==0) cout << i/100 << "  " << flush;
//      for (int j=0; j<n_readings_per_robot_posn; j++)
//      {
//         int k=i*n_readings_per_robot_posn+j;

//         double x_total=x_odometry[i]+x_readings[k];
//         double y_total=y_odometry[i]+y_readings[k];

//         double x_total=x_readings[l];
//         double y_total=y_readings[l];
//         l++;
//         outstream << x_total << "   " << y_total << endl;
//      } // loop over index j labeling readings per robot posn
//   } // loop over index i labeling odometry readings
//   cout << endl;

   filefunc::closefile(output_filename,outstream);

   string banner="Wrote out "+output_filename;
   outputfunc::write_banner(banner);

   exit(-1);
*/


//   string subdir="./south_lab/";
   string subdir="./";
   string floorplan_xy_filename=subdir+"floorplan_xy.dat";
   filefunc::ReadInfile(floorplan_xy_filename);

   cout << "filefunc::text_line.size() = " << filefunc::text_line.size()
        << endl;

   vector<double> X,Y,Z;
   const double Z_height=1;	// meters
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      if (i%10000==0) cout << i/10000 << " " << flush;
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);
      if (!stringfunc::is_number(substrings[0]) ||
          !stringfunc::is_number(substrings[1])) continue;

      X.push_back(stringfunc::string_to_number(substrings[0]));
      Y.push_back(stringfunc::string_to_number(substrings[1]));
      Z.push_back(Z_height);

//      cout << "i = " << i << " X = " << X.back() << " Y = " << Y.back()
//           << endl;
   }
   cout << endl;
   cout << "X.size() = " << X.size() << endl;

   double x_max=mathfunc::maximal_value(X);
   double x_min=mathfunc::minimal_value(X);
   double y_max=mathfunc::maximal_value(Y);
   double y_min=mathfunc::minimal_value(Y);

   cout << "x_max = " << x_max << " x_min = " << x_min << endl;
   cout << "y_max = " << y_max << " y_min = " << y_min << endl;

   string tdp_filename="floorplan.tdp";
   string UTMzone="19";
   tdpfunc::write_xyz_data(UTMzone,tdp_filename,X,Y,Z);



}
