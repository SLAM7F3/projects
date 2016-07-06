// ==========================================================================
// Program GCP (Ground Control Points) reads in 2D features extracted
// via program VIDEO and their 3D counterparts found via program LADAR.  
// It pulls out just the easting and northing coordinates from the
// input ladar features.  GCP writes out a set of lines which can be
// fed into gdal_translate in order to add ground control points into
// an image. (See README.GDAL).  

// 				./gcp

// ==========================================================================
// Last updated on 2/13/13; 4/9/13; 5/17/13; 8/4/13
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "astro_geo/geopoint.h"
#include "math/mypolynomial.h"
#include "general/outputfuncs.h"
#include "math/rubbersheet.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "templates/mytemplates.h"
#include "math/threevector.h"

using std::cin;
using std::cout;
using std::endl;
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
   const int PRECISION=15;
   cout.precision(PRECISION);

// Read in (easting,northing) geographic coordinates for tiepoint
// features:

//   string EN_features_filename="features_3D_full.txt";
//   string EN_features_filename="features_3D_HAFB_FF.txt";
//   string EN_features_filename="features_HAFB_GE_3D.txt";
//   string EN_features_filename="GE_MIT_10_geocoords.txt";
//   string EN_features_filename="features_CampEdwards_GE.txt";
   string NK_subdir="/home/cho/programs/c++/svn/projects/src/mains/OSG/NK/";
   string EN_features_filename=NK_subdir+"features_kim_square_GE.txt";
//   cout << "Enter full path for filename containing 2D features in Easting-Northing geocoordinates:" << endl;
//   cin >> EN_features_filename;
   filefunc::ReadInfile(EN_features_filename);

   vector<double> X,Y;
   vector<threevector> XYID;
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<string> substring=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);

      int feature_ID=stringfunc::string_to_number(substring[0]);
      double curr_X=stringfunc::string_to_number(substring[1]);
      double curr_Y=stringfunc::string_to_number(substring[2]);

      if (feature_ID==15) continue;
      if (feature_ID==1) continue;
      if (feature_ID==14) continue;
      if (feature_ID==7) continue;
      
      X.push_back(curr_X);
      Y.push_back(curr_Y);
      XYID.push_back(threevector(curr_X,curr_Y,feature_ID));
      cout << "i = " << i 
           << " ID = " << XYID.back().get(2)
           << " Easting = " << XYID.back().get(0)
           << " Northing = " << XYID.back().get(1)
           << endl;
   }
   int n_features=XYID.size();

// Read in (U,V) video coordinates for tiepoint features:

//   string UV_features_filename="features_2D_full.txt";
//   string UV_features_filename="features_2D_HAFB_FF.txt";
//   string UV_features_filename="features_HAFB_GE_2D.txt";
//   string UV_features_filename="GE_MIT_10_features_2D.txt";
//   string UV_features_filename="features_CampEdwards_2D.txt";
   string UV_features_filename=NK_subdir+"features_2D_kim_square_cropped2.txt";
//   cout << "Enter full path for filename containing 2D features in image plane UV coordinates:" << endl;
//   cin >> UV_features_filename;
   filefunc::ReadInfile(UV_features_filename);
   
   vector<double> U,V;
   vector<twovector> UV;
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
//      cout << filefunc::text_line[i] << endl;
      vector<string> substring=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);

      int feature_ID=stringfunc::string_to_number(substring[1]);

      if (feature_ID==15) continue;
      if (feature_ID==1) continue;
      if (feature_ID==14) continue;
      if (feature_ID==7) continue;
      
      double curr_U=stringfunc::string_to_number(substring[3]);
      double curr_V=stringfunc::string_to_number(substring[4]);
      U.push_back(curr_U);
      V.push_back(curr_V);
      UV.push_back(twovector(curr_U,curr_V));
      cout << "i = " << i << " UV = " << UV.back() << endl;
   }

   int poly_order=1;
   cout << "Enter fitting polynomial order:" << endl;
   cin >> poly_order;
   
   mypolynomial U2X(poly_order);
   mypolynomial V2Y(poly_order);
   double chisqUX,chisqVY;
   U2X.fit_coeffs(U,X,chisqUX);
   V2Y.fit_coeffs(V,Y,chisqVY);
//   U2X.fit_coeffs_using_residuals(U,X,chisqUX);
//   V2Y.fit_coeffs_using_residuals(V,Y,chisqVY);
   vector<double> residual_UX=U2X.compute_residuals(U,X);
   vector<double> residual_VY=V2Y.compute_residuals(V,Y);

   cout << "U2X = " << U2X << endl;
   cout << "V2Y = " << V2Y << endl;
   cout << "chisqUX = " << chisqUX << endl;
   cout << "chisqVY = " << chisqVY << endl;

   vector<int> feature_ID;
   vector<double> fitted_X,fitted_Y,dx,dy,ds;
   for (int i=0; i<U.size(); i++)
   {
      feature_ID.push_back(i);
      fitted_X.push_back(U2X.value(U[i]));
      fitted_Y.push_back(V2Y.value(V[i]));
      dx.push_back(fitted_X.back()-X[i]);
      dy.push_back(fitted_Y.back()-Y[i]);
      ds.push_back(sqrt(sqr(dx.back())+sqr(dy.back())));
//      cout << "i = " << i << " dEasting = " << dx << " dNorthing = " << dy 
//           << " ds = " << ds.back() << endl;
//      cout << "residual_UX = " << residual_UX[i]
//           << " residual_VY = " << residual_VY[i] << endl;
   }

/*
// Use linear transformation parameters to compute easting-northing
// coordinates corresponding to UV image plane corners:

   double minU=0;
   double minV=0;
   double maxU=0.907645; // Ft Devens 10 A horseshoe image snapped from GE
   double maxV=1;
//   cout << "Enter min_U:" << endl;
//   cin >> minU;
//   cout << "Enter max_U:" << endl;
//   cin >> maxU;

   double minX=U2X.value(minU);
   double maxX=U2X.value(maxU);
   double minY=V2Y.value(minV);
   double maxY=V2Y.value(maxV);
   
   cout << "minX = " << minX << " maxX = " << maxX << endl;
   cout << "minY = " << minY << " maxY = " << maxY << endl;
*/

// Write out ground control point arguments to gdal_translate in order
// to incorporate tiepoints into an aerial image:

   int Nx,Ny;
//   cout << "Enter aerial EO image width in pixels:" << endl;
//   cin >> Nx;
//   cout << "Enter aerial EO image height in pixels:" << endl;
//   cin >> Ny;

//   Nx=2023;	// GE snap of HAFB around flight facility
//   Ny=1291;

//   Nx=2192;	// Feb 2013 GE snap of HAFB around 2005 G99 aerial frame 279
//   Ny=1419;


//   Nx=2552;	// Apr 2013 snapshot GE_MIT_10.png 
//   Ny=1419;

//   Nx=1950;	// May 2013 snapshot CampEdwards_GE.png
//   Ny=1320;

   Nx=1706;	// Aug 2013 snapshop kim_square_cropped.png
   Ny=997;

   cout << endl;
   for (int i=0; i<UV.size(); i++)
   {
      double curr_U=UV[i].get(0);
      double curr_V=UV[i].get(1);
      int px=curr_U*Ny;
      int py=(1-curr_V)*Ny;

      double picked_easting=XYID[i].get(0);
      double picked_northing=XYID[i].get(1);
      double fitted_easting=U2X.value(curr_U);
      double fitted_northing=V2Y.value(curr_V);

//      double easting=fitted_X[i];
//      double northing=fitted_Y[i];
      double easting=fitted_easting;
      double northing=fitted_northing;

//      cout << "picked easting = " << easting 
//           << " fitted easting = " << fitted_easting 
//           << " picked northing = " << northing
//           << " fitted northing = " << fitted_northing
//           << endl;

//      cout << "fitted_easting-fitted_X = " 
//           << fitted_easting-fitted_X[i] 
//           << " fitted_northing-fitted_Y = "
//           << fitted_northing-fitted_Y[i] << endl;

// Note: -gcp = ground control point for gdal_translate

      cout << "-gcp " << px << " " << py << " "
           << easting << " " << northing;
      if (i < UV.size()-1) cout << " \\";
      cout << endl;
   } // loop over index i labeling tiepoint pairs

// Display discrepancies between fitted and raw (easting,northing)
// pairs sorted according to increasing size:

   cout << endl << endl;
   templatefunc::Quicksort(ds,feature_ID,dx,dy);
   for (int i=0; i<feature_ID.size(); i++)
   {
      cout << "feature_ID = " << feature_ID[i]
           << " dEasting = " << dx[i]
           << " dNorthing = " << dy[i]
           << " ds = " << ds[i] << endl;
   }
   cout << "n_features = " << n_features << endl;
   cout << "<ds> = " << mathfunc::mean(ds) << endl;

   
}
