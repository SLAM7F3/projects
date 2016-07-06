// ==========================================================================
// Program XYZP2TDP reads in point cloud data  some set of input XYZP
// files.  It writes out a corresponding Group 106 TDP file containing
// probability of detection information.

// 			xyzp2tdp lowell.xyzp

// ==========================================================================
// Last updated on 8/23/06; 10/30/07; 11/12/10
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "passes/PassesGroup.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "osg/osg3D/tdpfuncs.h"
#include "threeDgraphics/xyzpfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
   
// Use an ArgumentParser object to manage the program arguments:
   
   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);

   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
   cout << "cloudpass_ID = " << cloudpass_ID << endl;
   Pass* pass_ptr=passes_group.get_pass_ptr(cloudpass_ID);
   vector<string> input_filenames=pass_ptr->get_filenames();

// Read ladar points from input files:
   
   vector<fourvector>* xyzp_pnt_ptr=new vector<fourvector>;
   for (int i=0; i<input_filenames.size(); i++)
   {
      cout << "i = " << i << " input_filename = " << input_filenames[i]
           << endl;
      xyzpfunc::read_xyzp_float_data(input_filenames[i],0,xyzp_pnt_ptr);  
      cout << "xyzp_pnt_ptr->size() = "
           << xyzp_pnt_ptr->size() << endl;
   }

   threevector zeroth_XYZ(xyzp_pnt_ptr->at(0));
   cout << "zeroth_XYZ = " << zeroth_XYZ << endl;

   string tdp_filename=stringfunc::prefix(input_filenames[0])+".tdp";
   cout << "Output tdp_filename = " << tdp_filename << endl;

   string UTMzone="";	
   tdpfunc::write_xyzp_data(tdp_filename,UTMzone,zeroth_XYZ,xyzp_pnt_ptr);

   delete xyzp_pnt_ptr;
}
