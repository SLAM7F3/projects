// =======================================================================
// Program AFFINE_FIT imports 2D tiepoint pairs between two WISP panel
// images separated in time.  It performs and returns an affine fit to
// the tiepoint pairs.
// ========================================================================
// Last updated on 3/5/13; 3/18/13
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "geometry/homography.h"
#include "image/imagefuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::string;
using std::vector;

// ==========================================================================
int main( int argc, char** argv )
{
   string bundler_IO_subdir="./bundler/DIME/Feb2013_DeerIsland/";
   string features_subdir=bundler_IO_subdir+"features/";

   string features1_filename=features_subdir+
      "features_2D_subsampled_wisp_p2_res0_00090.txt";
   string features2_filename=features_subdir+
      "features_2D_subsampled_horizon_p2_res0_00000.txt";
   vector<twovector> UV1,UV2;

   filefunc::ReadInfile(features1_filename);
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      twovector curr_UV1(column_values[3],column_values[4]);
      UV1.push_back(curr_UV1);
   }

   filefunc::ReadInfile(features2_filename);
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      twovector curr_UV2(column_values[3],column_values[4]);
      UV2.push_back(curr_UV2);
   }
   
   genmatrix A(2,2);
   twovector trans;
   double RMS=mathfunc::fit_2D_affine_transformation(UV1,UV2,A,trans);

   cout << "RMS error = " << RMS << endl;
   cout << "A = " << A << " trans = " << trans << endl;
   cout << "A.det = " << A.determinant() << endl;

/*
   homography H;
   H.parse_homography_inputs(UV1,UV2);
   H.compute_homography_matrix();
   bool print_flag=false;
   double RMS_homography=H.check_homography_matrix(UV1,UV2,print_flag);
   cout << "H = " << H << endl;
*/

}

