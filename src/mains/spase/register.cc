// ==========================================================================
// Program REGISTER reads in 3D tiepoints manually picked from Hyrum's
// composite SPASE ISAR XYZP file as well as our double-bolt SPASE
// model XYZP file.  It computes the best-fit 4x4 projective
// transformation which maps the ISAR tiepoints into the model
// tiepoints.  REGISTER subsequently rotates/translates all of the
// composite ISAR points.  It writes out a new XYZP file which can be
// simply concatenated with the model XYZP file for viewing purposes.
// ==========================================================================
// Last updated on 2/20/06; 3/31/06
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "geometry/mybox.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "geometry/projective.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "threeDgraphics/xyzpfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   string subdir="./xyzp_files/";
   string isar_features_filename=subdir+"isar_features.txt";
   string model_features_filename=subdir+"model_features.txt";

   vector<string> isar_line,model_line;
   filefunc::ReadInfile(isar_features_filename,isar_line);
   filefunc::ReadInfile(model_features_filename,model_line);

   const int N_vertices=12;
   vector<threevector> pvec,qvec;
   for (int i=0; i<N_vertices; i++)
   {
      vector<double> number=stringfunc::string_to_numbers(
         isar_line[i]);
      pvec.push_back(threevector(number[3],number[4],number[5]));
      cout << "p = " << pvec.back() << endl;
   }

   for (int i=0; i<N_vertices; i++)
   {
      vector<double> number=stringfunc::string_to_numbers(
         model_line[i]);
      qvec.push_back(threevector(number[3],number[4],number[5]));
      cout << "q = " << qvec.back() << endl;
   }
   
   projective proj(N_vertices);
   proj.parse_projective_inputs(pvec,qvec);
   proj.compute_projective_matrix();
   proj.check_projective_matrix(pvec,qvec);

   vector<threevector> rvec;
   proj.transform_XYZs_to_UVWs(pvec,rvec);

   for (int n=0; n<pvec.size(); n++)
   {
      cout << "n = " << n << " diff = " 
           << (qvec[n]-rvec[n]).magnitude() << endl;
   }

// Transform every XYZP point within ISAR point cloud into model
// coordinate system in order to combine the former with the latter:

   for (int n=1; n<=9; n++)
   {
      string imagenumber=stringfunc::integer_to_string(n,1);
      string isar_xyzp_filename=subdir+"output"+imagenumber+".xyzp";
      vector<fourvector>* XYZP_ptr=xyzpfunc::read_xyzp_float_data(
         isar_xyzp_filename);
      xyzpfunc::reset_null_p_values(0.5*xyzpfunc::null_value,0,XYZP_ptr);

/*
  for (unsigned int i=0; i<XYZP_ptr->size(); i++)
  {
  if (  (*XYZP_ptr)[i].get(3) < -1)
  {
  cout << "i = " << i << " p = " << (*XYZP_ptr)[i].get(3) << endl;
  }
  }
*/

      vector<fourvector>* UVWP_ptr=new vector<fourvector>;
      proj.transform_XYZPs_to_UVWPs(*XYZP_ptr,*UVWP_ptr,xyzpfunc::null_value);
      delete XYZP_ptr;
   
      string output_isar_xyzp_filename=subdir+"rotated_isar_"+imagenumber
         +".xyzp";
      xyzpfunc::write_xyzp_data(output_isar_xyzp_filename,UVWP_ptr,false);
//   string output_isar_xyzp_filename=subdir+"nonull_isar.xyzp";
//   xyzpfunc::write_xyzp_data(output_isar_xyzp_filename,XYZP_ptr,false);

      delete UVWP_ptr;
   } // loop over index n labeling Hyrum's output.xyzp SPASE files

}
