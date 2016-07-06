// ==========================================================================
// Program CROP_POINTS crops ladar imagery
// ==========================================================================
// Last updated on 12/7/11
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

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

   string tdp_filename;
   cout << "Enter input TDP filename:" << endl;
   cin >> tdp_filename;
   
   vector<double>* X_ptr=new vector<double>;
   vector<double>* Y_ptr=new vector<double>;
   vector<double>* Z_ptr=new vector<double>;

   int npoints=tdpfunc::npoints_in_tdpfile(tdp_filename);
   X_ptr->reserve(npoints);
   Y_ptr->reserve(npoints);
   Z_ptr->reserve(npoints);
   tdpfunc::read_XYZ_points_from_tdpfile(tdp_filename,*X_ptr,*Y_ptr,*Z_ptr);


   double max_Z;
   cout << "Enter max Z value threshold:" << endl;
   cin >> max_Z;

   vector<double>* Xcropped_ptr=new vector<double>;
   vector<double>* Ycropped_ptr=new vector<double>;
   vector<double>* Zcropped_ptr=new vector<double>;

   for (int i=0; i<Z_ptr->size(); i++)
   {
      if (Z_ptr->at(i) > max_Z ) continue;
      Xcropped_ptr->push_back(X_ptr->at(i));
      Ycropped_ptr->push_back(Y_ptr->at(i));
      Zcropped_ptr->push_back(Z_ptr->at(i));
   }

   threevector zeroth_XYZ(
      Xcropped_ptr->at(0),Ycropped_ptr->at(0),Zcropped_ptr->at(0));

   delete X_ptr;
   delete Y_ptr;
   delete Z_ptr;

   string prefix=stringfunc::prefix(tdp_filename);
   string cropped_tdp_filename="cropped_"+prefix+".tdp";
   tdpfunc::write_xyz_data(
      cropped_tdp_filename,"",zeroth_XYZ,
      Xcropped_ptr,Ycropped_ptr,Zcropped_ptr);
   string unix_cmd="lodtree "+cropped_tdp_filename;
   sysfunc::unix_command(unix_cmd);

}
