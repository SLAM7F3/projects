// ==========================================================================
// Program FOOTPRINTS reads in 2D polylines generated via VIDEO from
// orthorectified aerial EO imagery.  It also takes in an affine 3x4
// projection matrix for the aerial image.  FOOTPRINTS converts from
// UV image plane coordinates to easting,northing geocoordinates.
// After combining with ladar-derived height information, FOOTPRINTS
// generates simple polyhedra models for urban buildings.
// ==========================================================================
// Last updated on 1/8/12; 1/9/12
// ==========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "math/genvector.h"
#include "geometry/geometry_funcs.h"
#include "geometry/linesegment.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "geometry/plane.h"
#include "geometry/polyline.h"
#include "geometry/polyhedron.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/threevector.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ofstream;
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

   cout.precision(12);

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group);
   int n_photos=photogroup_ptr->get_n_photos();
   cout << "n_photos = " << n_photos << endl;

   genmatrix* P_ptr=NULL;
   vector<camera*> camera_ptrs;
   for (int n=0; n<n_photos; n++)
   {
      PassInfo* PassInfo_ptr=passes_group.get_passinfo_ptr(n);
      P_ptr=PassInfo_ptr->get_projection_matrix_ptr();

      photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(n);
//      cout << "n = " << n << " *photo_ptr = " << *photo_ptr << endl;
      camera* camera_ptr=photo_ptr->get_camera_ptr();
      camera_ptrs.push_back(camera_ptr);

      camera_ptr->set_projection_matrix(*P_ptr);
      
//      cout << "n = " << n
//           << " camera = " << *camera_ptr << endl;
   }

   double Xlo=-P_ptr->get(0,3);
   double Ylo=-P_ptr->get(1,3);
   double deltaY=P_ptr->get(2,3);

// Read in line segments manually extracted from orthorectified aerial photo:

   string polyline_filename="./polylines_2D_footprints.txt";
   filefunc::ReadInfile(polyline_filename);

// Z_bottom for MIT health center from ladar map approximately equals 3 meters
// Z_top for MIT health center from ladar map approximately equals 20
//   At least 3 tiers of rooftop with delta Z = 4 meters

   double Z_bottom=3;	// meters
   double Z_top=20;	// meters
   const double Z_height=Z_top-Z_bottom;
   
   vector<threevector> bottom_footprint_XYZs;
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      int polyline_ID=column_values[2];
      double U=column_values[3];
      double V=column_values[4];
      double X=Xlo+deltaY*U;
      double Y=Ylo+deltaY*V;

      threevector curr_bottom_XYZ(X,Y,Z_bottom);
      bottom_footprint_XYZs.push_back(curr_bottom_XYZ);

//      cout << "U = " << U << " V = " << V
//           << " X = " << X << " Y = " << Y << endl;
   }

   polyline bottom_footprint(bottom_footprint_XYZs);
//   cout << "Bottom footprint = " << bottom_footprint << endl;

   polyhedron building_polyhedron;
   building_polyhedron.generate_prism_with_rectangular_faces(
      bottom_footprint,Z_height);

   cout << "building_polyhedron = " << building_polyhedron << endl;

   string OFF_filename="building.off";
   building_polyhedron.write_OFF_file(OFF_filename);

   polyhedron reconstructed_polyhedron;
   fourvector volume_color;
   reconstructed_polyhedron.read_OFF_file(OFF_filename,volume_color);

   cout << "reconstructed polyhedron = " << reconstructed_polyhedron 
        << endl;

}
