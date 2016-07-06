// ========================================================================
// Program HOMOTEST
// ========================================================================
// Last updated on 10/30/08
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <lm.h>
#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgText/Text>


#include "osg/osgGrid/AlirtGridsGroup.h"
#include "osg/osgGraphicals/AnimationController.h"
#include "video/camera.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "math/constant_vectors.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgFeatures/FeaturesGroup.h"
#include "osg/ModeController.h"
#include "osg/osg2D/MoviesGroup.h"
#include "optimum/optimizer_funcs.h"
#include "passes/PassesGroup.h"
#include "geometry/polyline.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/ViewerManager.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::string;
using std::vector;

// ==========================================================================
int main( int argc, char** argv )
{

   genmatrix K(3,3),Kprime(3,3),Kinv(3,3),Kprimeinv(3,3),G(3,3);
   
   double f=1.397564397;
   double fprime= 1.386913858;
   
   double u0=0.792056822;
   double v0=0.580084775;
   
   double u0prime=0.6782698504;
   double v0prime=0.4716247389;
 
   K.put(0,0,f);
   K.put(0,2,u0);
   K.put(1,1,f);
   K.put(1,2,v0);
   K.put(2,2,1);
   K.inverse(Kinv);

   Kprime.put(0,0,fprime);
   Kprime.put(0,2,u0prime);
   Kprime.put(1,1,fprime);
   Kprime.put(1,2,v0prime);
   Kprime.put(2,2,1);
   
   double relative_az=20.4395523*PI/180;
   double relative_el=-13.82631408*PI/180;
   double relative_roll=-4.895516406*PI/180;
   rotation R(relative_az,relative_el,relative_roll);

   G=Kprime*R*Kinv;
   double detG=G.determinant();
   cout << "detG = " << detG << endl;
   cout << "sqr(fprime/f) = " << sqr(fprime/f) << endl;
   double det_G_cuberoot=sgn(detG)*pow(fabs(detG),0.3333333333333);

   G  /= det_G_cuberoot;
   
   cout << "G = " << G << endl;
   cout << "detG = " << G.determinant() << endl;   

}

