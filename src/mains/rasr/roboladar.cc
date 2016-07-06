// ========================================================================
// Program ROBOLADAR is a little utility program which parses Div 7
// robot ladar XYZP text files and generates an output TDP file.

//				roboladar

// ========================================================================
// Last updated on 4/27/10; 6/9/10; 12/4/10
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgText/Text>

#include "osg/osgGrid/AlirtGridsGroup.h"
#include "osg/osgGraphicals/AnimationController.h"
#include "video/camera.h"
#include "osg/CompassHUD.h"
#include "math/constant_vectors.h"
#include "geometry/contour.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "osg/osgGrid/LatLongGridsGroup.h"
#include "osg/ModeController.h"
#include "osg/ModeKeyHandler.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osg2D/MovieKeyHandler.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "osg/osgPanoramas/PanoramasGroup.h"
#include "osg/osgPanoramas/PanoramasKeyHandler.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "geometry/polyline.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osg3D/tdpfuncs.h"
#include "osg/osgWindow/ViewerManager.h"

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

   cout << endl << endl;
   cout << "======================================================" << endl;
   cout << "Text file containing raw robot ladar data is assumed to reside in" << endl;
   cout << "~/programs/c++/svn/projects/src/mains/rasr/ladar/" << endl 
        << endl;

   string subdir="./ladar/";
   string input_filename;
   cout << "Enter name of input ladar data file:" << endl;
//   string input_filename=subdir+"s4_atrium_3d_points.dat";
   cin >> input_filename;
   input_filename=subdir+input_filename;

   filefunc::ReadInfile(input_filename);

   int n_points=filefunc::text_line.size();
   string banner="Number of points in raw ladar file = "+
      stringfunc::number_to_string(n_points);
   outputfunc::write_banner(banner);

// Scan through all intensities in order to determine extremal values:

   double max_intensity=NEGATIVEINFINITY;
   double min_intensity=POSITIVEINFINITY;
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      double curr_intensity=values[6];
      max_intensity=basic_math::max(max_intensity,curr_intensity);
      min_intensity=basic_math::min(min_intensity,curr_intensity);
   }

   banner="Max raw ladar intensity value = "+stringfunc::number_to_string(
      max_intensity);
   outputfunc::write_banner(banner);   
   banner="Min raw ladar intensity value = "+stringfunc::number_to_string(
      min_intensity);
   outputfunc::write_banner(banner);   

   vector<fourvector>* xyzp_pnt_ptr=new vector<fourvector>;
   
   double min_p=POSITIVEINFINITY;
   double max_p=NEGATIVEINFINITY;
//   double max_intensity=3969;
//   double min_intensity=72;
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
//      cout << filefunc::text_line[i] << endl;
      vector<double> values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      double x=values[3];
      double y=values[4];
      double z=values[5];
      double p=(values[6]-min_intensity)/
         (max_intensity-min_intensity);

      fourvector curr_xyzp(x,y,z,p);
      xyzp_pnt_ptr->push_back(curr_xyzp);

      max_p=basic_math::max(p,max_p);
      min_p=basic_math::min(p,min_p);
   }

   banner="Max renormalized ladar intensity value = "
      +stringfunc::number_to_string(max_p);
   outputfunc::write_banner(banner);   
   banner="Min renormalized ladar intensity value = "
      +stringfunc::number_to_string(min_p);
   outputfunc::write_banner(banner);   

   string tdp_filename="roboladar.tdp";
   tdpfunc::write_xyzp_data(tdp_filename,xyzp_pnt_ptr);

   banner="Wrote intermediate ladar file to "+tdp_filename;
   outputfunc::write_banner(banner);   
}

