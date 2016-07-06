// ========================================================================
// Program US2TDP takes in ultra-sound scans from binary data files
// supplied by Laura Brattain.  Following Laura's matrix conventions,
// it transforms each individual scan into the reference frame of the
// first scan.  After registering, the data from a particular scan is
// aggregated into an STL map which keeps track of numerator and
// denominator factors for each voxel.  The average ultrasound
// intensity for each voxel is calculated and exported to TDP file
// "avg_us.tdp".

//				us2tdp

// ========================================================================
// Last updated on 6/21/11; 6/22/11; 6/30/11
// ========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "math/fourvector.h"
#include "math/genmatrix.h"
#include "math/ltthreevector.h"
#include "numrec/nrfuncs.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "osg/osg3D/tdpfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::map;
using std::string;
using std::vector;

int main()
{
   string us_subdir="./brattain/data4peter/us/";
   string em_subdir="./brattain/data4peter/em/";
   string precalc_subdir="./brattain/data4peter/precalc/";
   vector<string> us_filenames=filefunc::files_in_subdir(us_subdir);
   vector<string> em_filenames=filefunc::files_in_subdir(em_subdir);

// First read in S and TC transformation matrices:

   string s_filename=precalc_subdir+"s";
   int n_floats=filefunc::size_of_file_in_bytes(s_filename)/sizeof(float);
//   cout << "n_floats = " << n_floats << endl;
   float* s_data_ptr=filefunc::ReadFloats(s_filename,n_floats);
   genmatrix S(4,4),Sinv(4,4);
   for (int f=0; f<n_floats; f++)
   {
      double value=s_data_ptr[f];
      int row=f/4;
      int column=f%4;
//      cout << "row = " << row << " column = " << column 
//           << " value = " << value << endl;
      S.put(row,column,value);
   }

   S.inverse(Sinv);
   cout << "S = " << S << endl;
   cout << "Sinv = " << Sinv << endl;

   string tc_filename=precalc_subdir+"tc";
   n_floats=filefunc::size_of_file_in_bytes(tc_filename)/sizeof(float);
   float* tc_data_ptr=filefunc::ReadFloats(tc_filename,n_floats);
   genmatrix TC(4,4),TCinv(4,4);
   for (int f=0; f<n_floats; f++)
   {
      double value=tc_data_ptr[f];
      int row=f/4;
      int column=f%4;
//      cout << "row = " << row << " column = " << column 
//           << " value = " << value << endl;
      TC.put(row,column,value);
   }
   TC.inverse(TCinv);
   cout << "TC = " << TC << endl;
   cout << "TCinv = " << TCinv << endl;

// ========================================================================
// Loop over ultrasound binary files starts here:

   vector<genmatrix*> EM_matrix_ptrs;
   genmatrix EM0(4,4),EM0inv(4,4);

// 0 <= x_index <= 127
// 0 <= y_index <= 47
// 0 <= z_index <= 207

   const int x_index_min=0;
   const int x_index_max=127;
   const int y_index_min=0;
   const int y_index_max=47;
   const int z_index_min=0;
   const int z_index_max=207;

// VOXELGRID_MAP independent variable: (X,Y,Z)
// VOXELGRID_MAP dependent variable: 
//   (intensity integral,
//       number non-null contributions from ultrasound binary files)

   typedef map<threevector,twovector,ltthreevector > VOXELGRID_MAP;
   VOXELGRID_MAP* voxelgrid_map_ptr=new VOXELGRID_MAP;

   for (unsigned int i=0; i<us_filenames.size(); i++)
//   for (int i=0; i<5; i++)
//   for (int i=0; i<3; i++)
   {

// Read in ultrasound voxel data:

      string curr_us_filename=us_filenames[i];
      string banner="i = "+stringfunc::number_to_string(i)+
         " us_filename = "+curr_us_filename;
      outputfunc::write_big_banner(banner);
//      int us_ID=stringfunc::string_to_number(
//         filefunc::getbasename(curr_us_filename));

      int n_bytes=filefunc::size_of_file_in_bytes(curr_us_filename);
//      cout << "n_bytes = " << n_bytes << endl;
      unsigned char* us_data_ptr=
         filefunc::ReadUnsignedChars(curr_us_filename,n_bytes);

      vector<fourvector>* xyzp_pnt_ptr=new vector<fourvector>;
      xyzp_pnt_ptr->reserve(n_bytes);

// 0 <= z_index <= 207
// 0 <= y_index <= 47
// 0 <= x_index <= 127

      const int max_z_index=207;

      for (int c=0; c<n_bytes; c++)
      {
         int voxel_value=stringfunc::unsigned_char_to_ascii_integer(
            us_data_ptr[c]);
         double curr_p=voxel_value/255.0;

         int z_index=c/(128*48);
         int remainder=c-(128*48)*z_index;

         int y_index=remainder/128;
         remainder -= y_index*128;

         int x_index=remainder;
         fourvector xyzp(x_index,y_index,z_index,curr_p);
         xyzp_pnt_ptr->push_back(xyzp);
      } // loop over index c labeling unsigned chars

//      string tdp_filename="us_"+stringfunc::number_to_string(us_ID)+".tdp";
//      tdpfunc::write_xyzp_data(tdp_filename,xyzp_pnt_ptr);

// Read in tracking rotation matrix corresponding to current
// ultrasound data file:

      string curr_em_filename=em_filenames[i];
      cout << "curr_em_filename = " << curr_em_filename << endl;

      n_floats=filefunc::size_of_file_in_bytes(curr_em_filename)/sizeof(float);
//      cout << "n_floats = " << n_floats << endl;
      float* em_data_ptr=filefunc::ReadFloats(curr_em_filename,n_floats);
//      cout << "em_data_ptr = " << em_data_ptr << endl;

      genmatrix* curr_EM_ptr=new genmatrix(4,4);
      for (int f=0; f<n_floats; f++)
      {
         double value=em_data_ptr[f];
         int row=f/4;
         int column=f%4;
//         cout << "row = " << row << " column = " << column 
//              << " value = " << value << endl;
         curr_EM_ptr->put(row,column,value);
      } // loop over index f
      EM_matrix_ptrs.push_back(curr_EM_ptr);
//      cout << "EM = " << *(EM_matrix_ptrs.back()) << endl;

      if (i==0)
      {
         EM0=*(EM_matrix_ptrs[0]);
         EM0.inverse(EM0inv);
      }

// Aggregate voxel values into single 3D grid:
      
      genmatrix M(4,4);
      M=Sinv * TCinv * EM0inv * (*curr_EM_ptr) * TC * S;
//      v1 = inv(s)*inv(tc)*inv(em_t1)*em_t2*tc*s*v2      
      delete curr_EM_ptr;

      fourvector curr_xyzp,transformed_xyzp;
      for (unsigned int j=0; j<xyzp_pnt_ptr->size(); j++)
      {
         curr_xyzp=xyzp_pnt_ptr->at(j);
         double curr_p=curr_xyzp.get(3);
         curr_xyzp.put(3,1);
         transformed_xyzp=M*curr_xyzp;

         int curr_x=basic_math::round(transformed_xyzp.get(0));
         int curr_y=basic_math::round(transformed_xyzp.get(1));
         int curr_z=basic_math::round(transformed_xyzp.get(2));

         if (curr_x < x_index_min || curr_x > x_index_max) continue;
         if (curr_y < y_index_min || curr_y > y_index_max) continue;
         if (curr_z < z_index_min || curr_z > z_index_max) continue;

// Flip z value so that probe is located "above" sample rather than
// "below" it:

         curr_z = max_z_index-curr_z;
         threevector transformed_xyz(curr_x,curr_y,curr_z);

/*
// VOXELGRID_MAP independent variable: (X,Y,Z)
// VOXELGRID_MAP dependent variable: 
//   (intensity integral,
//       number non-null contributions from ultrasound binary files)

   typedef map<threevector,twovector,ltthreevector > VOXELGRID_MAP;
   VOXELGRID_MAP* voxelgrid_map_ptr=new VOXELGRID_MAP;
*/

         VOXELGRID_MAP::iterator iter=voxelgrid_map_ptr->find(transformed_xyz);
         twovector curr_numer_denom(0,0);
         if (iter != voxelgrid_map_ptr->end())
         {
            curr_numer_denom=iter->second;
         }
//         double prev_p=curr_numer_denom.get(0);

// High pass filtering:

//         curr_p=max(curr_p,prev_p);

// Low pass filtering:

         curr_numer_denom += twovector(curr_p,1);

         (*voxelgrid_map_ptr)[transformed_xyz]=curr_numer_denom;

      } // loop over index j labeling XYZP voxels in current ultrasound 
	//  data file
      
      delete xyzp_pnt_ptr;

   } // loop over index i labeling ultrasound binary data files

// Compute averaged intensities as well as their probability
// distribution:

   vector<threevector>* xyz_ptr=new vector<threevector>;
   vector<double> avg_p_values;

   for (VOXELGRID_MAP::iterator iter=voxelgrid_map_ptr->begin(); 
        iter != voxelgrid_map_ptr->end(); iter++)
   {
      threevector curr_xyz=iter->first;
      xyz_ptr->push_back(curr_xyz);
      twovector curr_numerdenom=iter->second;
      double p_avg=curr_numerdenom.get(0)/curr_numerdenom.get(1);
      avg_p_values.push_back(p_avg);
   } // loop over VOXELGRID_MAP iterator

   prob_distribution prob(avg_p_values,1000);
   bool gzip_flag=false;
   prob.writeprobdists(gzip_flag);

   double cumprob_min=0.70;
   double p_threshold=prob.find_x_corresponding_to_pcum(cumprob_min);
//   p_threshold=-1;
   cout << "p_threshold = " << p_threshold << endl;

// Write out voxel grid's integrated contents:
   
   vector<fourvector>* xyzp_pnt_ptr=new vector<fourvector>;
   for (unsigned int i=0; i<avg_p_values.size(); i++)
   {
      double curr_avg_p=avg_p_values[i];
      if (curr_avg_p < p_threshold) continue;

// On 6/30/11, Ross Anderson suggested adding small random
// fluctuations to (x,y,z) values in order to disrupt ugly Moire
// pattern effects in voxelized data:

      threevector curr_xyz(xyz_ptr->at(i));
      double dx=nrfunc::ran1()*1;
      double dy=nrfunc::ran1()*1;
      double dz=nrfunc::ran1()*1;
      curr_xyz += threevector(dx,dy,dz);
      fourvector xyzp(curr_xyz,curr_avg_p);

      xyzp_pnt_ptr->push_back(xyzp);
   } // loop over index c labeling unsigned chars

   string tdp_filename="avg_us.tdp";
   tdpfunc::write_xyzp_data(tdp_filename,xyzp_pnt_ptr);
}
 
