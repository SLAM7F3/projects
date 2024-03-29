// ==========================================================================
// Program SORT_CAMERA_VIEWS takes in a camera views file generated by
// BUNDLER_CONVERT.  It sorts the input text file according to photo
// ID.  Sorted SIFT feature output is written to sorted_camera_views.dat.

// sort_camera_views --region_filename ./bundler/kermit/packages/bundler_photos.pkg

// ==========================================================================
// Last updated on 11/23/09
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "passes/PassesGroup.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/threevector.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
   cout.precision(15);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);
   
   string camera_views_filename=passes_group.get_camera_views_filename();
   cout << "camera_views_filename = " << camera_views_filename << endl;
   filefunc::ReadInfile(camera_views_filename);

   vector<int> photo_ID;
   vector<double> X_sift,Y_sift,Z_sift,U_sift,V_sift;
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> fields=
         stringfunc::string_to_numbers(filefunc::text_line[i]);
      photo_ID.push_back(fields[0]);
      X_sift.push_back(fields[1]);
      Y_sift.push_back(fields[2]);
      Z_sift.push_back(fields[3]);
      U_sift.push_back(fields[4]);
      V_sift.push_back(fields[5]);
   } // loop over index i labeling lines in camera_views_filename

   templatefunc::Quicksort(photo_ID,X_sift,Y_sift,Z_sift,U_sift,V_sift);

// Open text file to hold SIFT features sorted by photo ID:

   string output_filename="sorted_camera_views.dat";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);

   for (unsigned int i=0; i<photo_ID.size(); i++)
   {
      outstream << photo_ID[i] << "   "
                << X_sift[i] << "   "
                << Y_sift[i] << "   "
                << Z_sift[i] << "   "
                << U_sift[i] << "   "
                << V_sift[i] << endl;
   } // loop over index i labeling photos

   filefunc::closefile(output_filename,outstream);

   string banner="3D/2D SIFT features sorted by photo ID written to output file "+output_filename;
   outputfunc::write_big_banner(banner);

}

