// ==========================================================================
// Program TDP2PLY converts an XYZ or XYZRGB point cloud imported from
// a TDP file into an output PLY ascii file.

// 				tdp2ply

// ==========================================================================
// Last updated on 1/17/12; 1/18/12; 6/29/12
// ==========================================================================

#include <iostream>
#include <iomanip>
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

   vector<string> param;
   bool input_args_flag=filefunc::parameter_input(argc,argv,param);
   if (!input_args_flag || param.size() != 1)
   {
      cout << "Error: Must specify input tdp filename as command line argument!"
           << endl;
      exit(-1);
   }

   string tdp_filename=param[0];
   if (stringfunc::suffix(tdp_filename) != "tdp")
   {
      cout << "Input file needs to be of tdp type!" << endl;
      exit(-1);
   }

   cout << "Input TDP filename = " << tdp_filename << endl;
   
   vector<double> X,Y,Z;
   vector<int> R,G,B;
//   tdpfunc::read_XYZRGB_points_from_tdpfile(tdp_filename,X,Y,Z,R,G,B);
   tdpfunc::read_XYZ_points_from_tdpfile(tdp_filename,X,Y,Z);
 
   int n_points=Z.size();
   string prefix=stringfunc::prefix(tdp_filename);
   string ply_filename=prefix+".ply";
   ofstream plystream;

   filefunc::openfile(ply_filename,plystream);
   plystream << "ply" << endl;
   plystream << "format ascii 1.0" << endl;
   plystream << "element vertex " << n_points << endl;

// On 1/18/12, we empirically found that changing float to double for XYZ
// yields PLY output which Ubuntu version of meshlab 3D viewer cannot
// handle...

   plystream << "property float x" << endl;
   plystream << "property float y" << endl;
   plystream << "property float z" << endl;

//   plystream << "property uchar red" << endl;
//   plystream << "property uchar green" << endl;
//   plystream << "property uchar blue" << endl;

   plystream << "end_header" << endl;

   cout.precision(12);
   plystream.precision(10);
   cout << "Total number of points = " << n_points << endl;
   
   for (int n=0; n<n_points; n++)
   {
      double frac=double(n)/double(n_points);
      if (n%1000000==0) cout << "Fraction exported = " << frac << endl;
      plystream << stringfunc::number_to_string(X[n],3) << " "
                << stringfunc::number_to_string(Y[n],3) << " "
                << stringfunc::number_to_string(Z[n],3) << " "
//                << R[n] << "  "
//                << G[n] << "  "
//                << B[n] 
                << endl;
   } // loop over index n labeling points from TDP file
   cout << endl;
   
   filefunc::closefile(ply_filename,plystream);

   string banner="Converted input TDP file to output PLY file "+ply_filename;
   outputfunc::write_big_banner(banner);
}
