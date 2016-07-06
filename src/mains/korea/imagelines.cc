// ========================================================================
// Program IMAGELINES imports a set of manually selected points along
// lines which are parallel in world-space.  It exports the parallel
// segment information in polyline format which can be visualized via
// program VIDEO.
// ========================================================================
// Last updated on 9/17/13; 9/18/13
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "osg/osgGraphicals/AnimationController.h"
#include "models/Building.h"
#include "models/BuildingsGroup.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "osg/osgGrid/LatLongGridsGroup.h"
#include "osg/ModeController.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "geometry/plane.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgGeometry/TrianglesGroup.h"
#include "osg/osgWindow/ViewerManager.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ifstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Import manually-selected points lying along imageplane lines:

//   string input_filename="features_streetlines.txt";
    string input_filename="features_cross_streetlines.txt";
   filefunc::ReadInfile(input_filename);

   string output_filename="polylines_2D.txt";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);
   outstream << "# Time   PolyLine_ID   Passnumber   X Y R G B A" << endl
             << endl;

   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      double time=0;
      int polyline_ID=i/2;
      int passnumber=0;

      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      double U=column_values[3];
      double V=column_values[4];

      double red=1;
      double green=1;
      double blue=1;
      double alpha=1;
         
      outstream << time << "  "
                << polyline_ID << "  "
                << passnumber << "  "
                << U << "   "
                << V << "   "
                << red << "  "
                << green << "  "
                << blue << "  "
                << alpha << endl;
   } // loop over index i labeling 2D line segment points

}

