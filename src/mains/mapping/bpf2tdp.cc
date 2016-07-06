// ==========================================================================
// Program BPF2TDP reads in a Binary Point Format ladar file which may
// be in BPF1, BPF2 or BPF3 format.  It extracts metadata from the
// file's header as well as offset and extremal data values.  BPF2TDP
// then fills X,Y,Z and P arrays with values.  Finally, this program
// outputs a TDP file containing the XYZP information.

// 			bpf2tdp 0SEI-0Z1A.bpf

// ==========================================================================
// Last updated on 11/11/11; 11/16/11; 12/3/11
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
   vector<double>* P_ptr=new vector<double>;
   vector<double>* NWC_ptr=new vector<double>;
   vector<int>* pixel_number_ptr=new vector<int>;

   bpffunc::read_bpf_L1_data(
      input_bpf_filename,X_ptr,Y_ptr,Z_ptr,
      pixel_number_ptr,UTM_zonenumber);
   
/*
   ifstream binary_instream;
   filefunc::open_binaryfile(input_bpf_filename,binary_instream);
   vector<double>* P_ptr=new vector<double>;
   vector<double>* NWC_ptr=new vector<double>;

   if (bpffunc::is_bpf3_file(binary_instream))
   {
      cout << "BPF3 format" << endl;
      bpffunc::parse_bpf3_points(
         binary_instream,X_ptr,Y_ptr,Z_ptr,NWC_ptr,P_ptr,UTM_zonenumber);
   }
   else
   {
      cout << "BPF1 or BPF2 format" << endl;
      bpffunc::parse_bpf12_points(
         binary_instream,X_ptr,Y_ptr,Z_ptr,NWC_ptr,P_ptr,UTM_zonenumber);
   }
   binary_instream.close();  

   cout << "Finished reading in BPF data" << endl << endl;
*/
 
   int point_count=X_ptr->size();
   cout << "point_count = " << point_count << endl;

/*
// Compute Z-profile:

   prob_distribution prob_Z(*Z_ptr,100);
   double z_median=prob_Z.median();

//   double Z0001=prob_Z.find_x_corresponding_to_pcum(0.0001);
   double Z001=prob_Z.find_x_corresponding_to_pcum(0.001);
   double Z01=prob_Z.find_x_corresponding_to_pcum(0.01);
   double Z10=prob_Z.find_x_corresponding_to_pcum(0.10);

   double Z90=prob_Z.find_x_corresponding_to_pcum(0.90);
   double Z99=prob_Z.find_x_corresponding_to_pcum(0.99);
   double Z9999=prob_Z.find_x_corresponding_to_pcum(0.9999);
//   const double Zfloor=-5;	// meters
//   const double Zceiling=90;	// meters

   cout << "Z_001 = " << Z001 << endl;
   cout << "Z median = " << z_median << endl;
   cout << "Z_9999 = " << Z9999 << endl;
*/

/*
// Eliminate snow flakes by imposing nontrivial minimal threshold on
// neighborhood weighted count field:

   const double NWC_min=0;	
//   const double NWC_min=2.39/7.7089;	// Sortie 23 20101025-131010-1064nm
   
   fourvector curr_point;
   for (int n=0; n<point_count; n++)
   {
      if (n%1000000==0) cout << n/1000000 << " " << flush;

      double curr_Z=Z_ptr->at(n);
//      if (curr_Z < Z01) continue;
//      if (curr_Z > Z99) continue;
      if (curr_Z < Z001) continue;
      if (curr_Z > Z9999) continue;

      double curr_neighborhood_weighted_count=NWC_ptr->at(n);
      if (curr_neighborhood_weighted_count < NWC_min) continue;

      curr_point.put(0,X_ptr->at(n));
      curr_point.put(1,Y_ptr->at(n));
      curr_point.put(2,Z_ptr->at(n));
//      curr_point.put(3,NWC_ptr->at(n));
      curr_point.put(3,P_ptr->at(n));
   }
   cout << endl;
*/

   delete NWC_ptr;


   string tdp_filename=stringfunc::prefix(input_bpf_filename)+".tdp";
   cout << "Output tdp_filename = " << tdp_filename << endl;


/*
   threevector zeroth_XYZ(X_ptr->at(0),Y_ptr->at(0),Z_ptr->at(0));
   cout << "zeroth_XYZ = " << zeroth_XYZ << endl;

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
*/

   cout << "X_ptr->size() = " << X_ptr->size() << endl;
   cout << "Y_ptr->size() = " << Y_ptr->size() << endl;
   cout << "Z_ptr->size() = " << Z_ptr->size() << endl;
   cout << "P_ptr->size() = " << P_ptr->size() << endl;
   
   if (P_ptr->size() != Z_ptr->size())
   {
      tdpfunc::write_xyz_data(
         tdp_filename,X_ptr,Y_ptr,Z_ptr);
   }
   else
   {
      tdpfunc::write_xyzp_data(
         tdp_filename,X_ptr,Y_ptr,Z_ptr,P_ptr);
   }
   
   delete X_ptr;
   delete Y_ptr;
   delete Z_ptr;
   delete P_ptr;
   delete pixel_number_ptr;

   string unix_cmd="lodtree "+tdp_filename;
   sysfunc::unix_command(unix_cmd);
}

