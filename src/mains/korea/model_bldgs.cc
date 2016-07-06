// ==========================================================================
// Program MODEL_BLDGS reads in easting, northing, altitude
// geocoordinates for "skyline" corners on Korean buildings in UTM
// zone 51.  It generates and exports simple polyhedra models for
// these buildings.

//       			 model_bldgs 

// ==========================================================================
// Last updated on 8/5/13; 8/6/13
// ==========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "geometry/contour.h"
#include "general/filefuncs.h"
#include "math/genvector.h"
#include "geometry/geometry_funcs.h"
#include "passes/PassesGroup.h"
#include "geometry/plane.h"
#include "geometry/polyline.h"
#include "geometry/polyhedron.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/threevector.h"
#include "osg/osgTiles/TilesGroup.h"

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

   double Z_ground=19;

   string polyline_filename="./bldg_2D_footprints.txt";
   cout << "polyline_filename = " << polyline_filename << endl;
   filefunc::ReadInfile(polyline_filename);

   int n_polylines=0;
   int prev_polyline_ID=-1;
   vector<int> polyline_IDs;
   vector<fourvector> footprint_XYZID;
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);

      int polyline_ID=column_values[1];
      if (polyline_ID != prev_polyline_ID)
      {
         polyline_IDs.push_back(polyline_ID);
         n_polylines++;
         prev_polyline_ID=polyline_ID;
      }
      
      double time=column_values[0];
      int passnumber=column_values[2];
      double X=column_values[3];
      double Y=column_values[4];
      double Z=column_values[5];
      footprint_XYZID.push_back(fourvector(X,Y,Z,polyline_ID));
   } // loop over index i 

// Note added on Weds, Jan 11, 2012:

// Need to renumber integer IDs within footprint_XYZID STL vector so
// that they sequentially range from 0 to number of input polylines:

   cout << "n_polylines = " << n_polylines << endl;
   cout << "polyline_IDs.size() = " << polyline_IDs.size() << endl;

// Purge contents of OFF subdirectory:

   string OFF_subdir="./OFF/";
   string off_suffix="off";
   filefunc::purge_files_with_suffix_in_subdir(OFF_subdir,off_suffix);

   double bldg_height_above_ground;
   cout << "Enter bldg rooftop height above ground:" << endl;
   cin >> bldg_height_above_ground;

// ==========================================================================
// Iteration over building footprint polylines starts here

   int prev_building_ID=-1;
   int building_part_counter=0;
   for (int polyline_counter=0; polyline_counter<n_polylines; 
        polyline_counter++)
   {
      int polyline_ID=polyline_IDs[polyline_counter];
//      cout << "polyline_counter = " << polyline_counter
//           << " polyline_ID = " << polyline_ID << endl;

// As of 1/12/12, we assume IDs for polylines all corresponding to the
// same building are offset by 1000: e.g. 7, 1007, 2007, etc...

      int building_ID=polyline_ID%1000;
      if (building_ID != prev_building_ID)
      {
         prev_building_ID=building_ID;
         building_part_counter=0;
      }

      vector<threevector> bottom_footprint_XYZs;
      for (int i=0; i<footprint_XYZID.size(); i++)
      {
         int curr_footprint_ID=footprint_XYZID[i].get(3);
         if (curr_footprint_ID == polyline_ID) 
         {
            threevector curr_XYZ=threevector(footprint_XYZID[i]);
//            bldg_height_above_ground=curr_XYZ.get(2)-Z_ground;
            curr_XYZ.put(2,Z_ground);
            bottom_footprint_XYZs.push_back(curr_XYZ);
         }
      } // loop over index i 

//      cout << "bottom_footprint_XYZs.size() = "
//           << bottom_footprint_XYZs.size() << endl;

// Generate contour corresponding to building's footprint:

      polyline bottom_footprint(bottom_footprint_XYZs);
//      cout << "Bottom footprint = " << bottom_footprint << endl;

      polyhedron building_polyhedron;
      building_polyhedron.generate_prism_with_rectangular_faces(
         bottom_footprint,bldg_height_above_ground);
      cout << "building_polyhedron = " << building_polyhedron << endl;

      filefunc::dircreate(OFF_subdir);

      string OFF_filename=OFF_subdir
         +"building_"+stringfunc::number_to_string(building_ID)
         +"_part_"+stringfunc::number_to_string(building_part_counter++)
         +".off";
      building_polyhedron.write_OFF_file(OFF_filename);

   } // loop over index polyline_ID labeling manually derived bldg footprints

// ========================================================================== 

   string banner="Building polyhedra exported to "+OFF_subdir;
   outputfunc::write_big_banner(banner);
}
