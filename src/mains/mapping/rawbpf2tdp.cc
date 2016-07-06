// ==========================================================================
// Program RAWBPF2TDP reads in a raw "level 1" Binary Point Format
// ladar file which may be in BPF1, BPF2 or BPF3 format.  It extracts
// metadata from the file's header as well as offset and extremal data
// values.  BPF2TDP fills X,Y,Z STL vectors with values.  Finally,
// this program outputs a TDP file containing the XYZP information.

// 			rawbpf2tdp 0SEI-0Z1A.bpf

// ==========================================================================
// Last updated on 12/21/10; 11/3/11; 11/11/11
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "threeDgraphics/bpffuncs.h"
#include "general/filefuncs.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "osg/osg3D/tdpfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ios;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

// Read input BPF file:
   
   string input_bpf_filename=argv[1];
   cout << "input_bpf_filename = " << input_bpf_filename << endl;

   int UTM_zonenumber;
   vector<double>* X_ptr=new vector<double>;
   vector<double>* Y_ptr=new vector<double>;
   vector<double>* Z_ptr=new vector<double>;

   bpffunc::read_bpf_L1_data(
      input_bpf_filename,X_ptr,Y_ptr,Z_ptr,UTM_zonenumber);

   int point_count=X_ptr->size();
   cout << "point_count = " << point_count << endl;

   threevector zeroth_XYZ(X_ptr->at(0),Y_ptr->at(0),Z_ptr->at(0));
   cout << "zeroth_XYZ = " << zeroth_XYZ << endl;

   string tdp_filename=stringfunc::prefix(input_bpf_filename)+".tdp";
   cout << "Output tdp_filename = " << tdp_filename << endl;

   bool northern_hemisphere_flag=true;
   if (sgn(UTM_zonenumber)==01) northern_hemisphere_flag=false;
   string UTMzone=stringfunc::number_to_string(UTM_zonenumber);
   if (northern_hemisphere_flag)
   {
      UTMzone += "N";
   }
   else
   {
      UTMzone += "S";
   }

   tdpfunc::write_xyzp_data(
      tdp_filename,UTMzone,zeroth_XYZ,X_ptr,Y_ptr,Z_ptr,NULL);

   delete X_ptr;
   delete Y_ptr;
   delete Z_ptr;

//   string unix_cmd="lodtree "+tdp_filename;
//   sysfunc::unix_command(unix_cmd);
}

