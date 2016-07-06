// ========================================================================
// Program HOG

// ========================================================================
// Last updated on 3/14/12
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgText/Text>

#include "osg/osgGrid/AlirtGridsGroup.h"
#include "osg/osgGraphicals/AnimationController.h"
#include "video/camera.h"
#include "video/camerafuncs.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "math/constant_vectors.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgFeatures/FeaturesGroup.h"
#include "general/filefuncs.h"
#include "math/fourvector.h"
#include "osg/ModeController.h"
#include "templates/mytemplates.h"
#include "osg/osgOperations/Operations.h"
#include "optimum/optimizer.h"
#include "optimum/optimizer_funcs.h"
#include "passes/PassesGroup.h"
#include "video/photograph.h"
#include "video/photogroup.h"
#include "video/sift_detector.h"
#include "osg/osg3D/Terrain_Manipulator.h"
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

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
//   photogroup_ptr->read_photographs(passes_group);

   sift_detector* sift_detector_ptr=new sift_detector(photogroup_ptr);
   
   int n_columns,n_rows;
   cout << "Enter n_columns:" << endl;
   cin >> n_columns;
   cout << "Enter n_rows:" << endl;
   cin >> n_rows;

   string image_filename="/data/ImageEngine/kermit/kermit000.jpg";
   vector<sift_detector::feature_pair> currimage_feature_info;
   sift_detector_ptr->extract_HOG_features(
      image_filename,n_columns,n_rows,currimage_feature_info);
   

   for (int i=0; i<currimage_feature_info.size(); i++)
   {
      genvector* F_ptr=currimage_feature_info[i].first;
      genvector* D_ptr=currimage_feature_info[i].second;
      
      cout << "i = " << i 
           << " U = " << F_ptr->get(1)
           << " V = " << F_ptr->get(2) 
           << endl;
      cout << "D = " << *D_ptr << endl;
   }
   

}

