// ==========================================================================
// Program ESTIMATE_F implements a modified version of the constrained
// "checkerboard" calibration algorithm described in I. Shimizu,
// Z. Zhang, S. Akamatsu and K. Deguchi, "Head pose determination from
// one image using a generic mode camera calibration", IEEE 3rd Intl
// Conf on Automatic Face and Gesture Recognition, pp 100-105, 1998.
// It first reads in a set of image-plane UV and corresponding
// world-plane XY tiepoints from some number of photos (preferably 4
// or more).  For each photo, a least-squares homography relating the
// image and world planes is next calculated.  We assume the internal
// matrix K is constrained so that it depends upon a SINGLE unknown
// focal parameter (i.e. f=fu=fv; skew=0; u0=0.5*aspect_ratio and
// v0=0.5).  ESTIMATE_F computes and returns the average of all
// reasonable image f values.

// As of 1/21/12, we believe "checkerboard" calibration is only
// quasi-robust for (highly constrained) f parameter estimation and
// NOT for finding any other internal or external camera parameters...

//				estimate_f

// Important note: As of 1/21/12, we believe the following
// relationship holds between our dimensionless focal parameter f, the
// physical focal length F (measured in mm) and d = VERTICAL physical
// size of the CCD chip: f_peter = - F/d.  Digital camera chip sizes
// are commonly expressed in terms of 35 mm equivalents.  A "35 mm"
// chip has horizontal dimension = 36 mm and vertical dimension = 24
// mm.  So f_peter = - F_equivalent / 24 mm . 

// For Powershot SX1501S, 35 mm film equivalent is 28 mm (wide) and
// 336 mm (telephoto).  So at its default zoom setting, f_peter =
// -28mm/24mm=-1.16 .  

// For Fujifilm Finepix S800fd, F = 27 mm (wide) to 488 mm
// (telephoto).  So f_peter = - 27mm / 24mm = -1.13 for default zoom
// setting.

// ==========================================================================
// Last updated on 1/19/12; 1/20/12; 1/21/12; 5/30/15
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "video/camerafuncs.h"
#include "general/filefuncs.h"
#include "geometry/homography.h"
#include "astro_geo/latlong2utmfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "math/twovector.h"

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
   const int PRECISION=12;
   cout.precision(PRECISION);

// Read in imageplane-worldplane tiepoint pairs:

   vector<string> feature_filenames;

   string features_subdir="./calib/Fuji/";
   feature_filenames.push_back(features_subdir+"features_2D_DSCF9083.txt");
   feature_filenames.push_back(features_subdir+"features_2D_DSCF9084.txt");
   feature_filenames.push_back(features_subdir+"features_2D_DSCF9085.txt");
   feature_filenames.push_back(features_subdir+"features_2D_DSCF9086.txt");
   double aspect_ratio=3264.0/2448.0;	// DSCF9083-6.JPG 
   unsigned int n_tiepoints=9;
//    const double f_actual=-1.13;
//    const double f_calculated=-1.12;

/*
   string features_subdir="./data/calib/";
   feature_filenames.push_back(features_subdir+"features_2D_IMG_0133.txt");
   feature_filenames.push_back(features_subdir+"features_2D_IMG_0134.txt");
   feature_filenames.push_back(features_subdir+"features_2D_IMG_0135.txt");
   feature_filenames.push_back(features_subdir+"features_2D_IMG_0136.txt");
   int n_tiepoints=9;
   double aspect_ratio=4000.0/3000.0;	// IMG_0133-0136.JPG 
   const double f_actual=-1.16;
   const double f_calculated=-1.27;
*/

/*
   string features_subdir="./calib/simul_camera/";
   feature_filenames.push_back(features_subdir+"features_2D_vs1.txt");
   feature_filenames.push_back(features_subdir+"features_2D_vs2.txt");
   feature_filenames.push_back(features_subdir+"features_2D_vs3.txt");
   feature_filenames.push_back(features_subdir+"features_2D_vs4.txt");
   int n_tiepoints=8;
//    double aspect_ratio=1067.0/800.0;	// Simulated image
   double aspect_ratio=1.33375;		// Simulated image (calculated value)
   const double FOV_u_actual=40;	// degs
   const double FOV_v_actual=30;	// degs
   const double f_actual=-1.866;
   const double f_calculated=-1.71;
*/

/*
   string features_subdir="./calib/calib_front_camera/grid/cropped_frames/";
   feature_filenames.push_back(features_subdir+"features_2D_frame0007.txt");
   feature_filenames.push_back(features_subdir+"features_2D_frame0021.txt");
   feature_filenames.push_back(features_subdir+"features_2D_frame0035.txt");
   feature_filenames.push_back(features_subdir+"features_2D_frame0055.txt");
   feature_filenames.push_back(features_subdir+"features_2D_frame0088.txt");
   int n_tiepoints=9;
   double aspect_ratio=647.0/480.0;    // Cropped Raven front video frame
*/

/*
   string features_subdir="./calib/calib_side_camera/grid/";
   feature_filenames.push_back(features_subdir+"features_2D_frame0006.txt");
   feature_filenames.push_back(features_subdir+"features_2D_frame0110.txt");
   feature_filenames.push_back(features_subdir+"features_2D_frame0173.txt");
   feature_filenames.push_back(features_subdir+"features_2D_frame0203.txt");
   int n_tiepoints=9;
   double aspect_ratio=720.0/480.0;	// Uncropped Raven video frame
*/

/*
   string features_subdir="./calib/calib_front_camera/grid/";
   feature_filenames.push_back(features_subdir+"features_2D_frame0007.txt");
   feature_filenames.push_back(features_subdir+"features_2D_frame0021.txt");
   feature_filenames.push_back(features_subdir+"features_2D_frame0035.txt");
   feature_filenames.push_back(features_subdir+"features_2D_frame0055.txt");
   feature_filenames.push_back(features_subdir+"features_2D_frame0088.txt");
   int n_tiepoints=9;
   double aspect_ratio=720.0/480.0;	// Uncropped Raven video frame
*/


   int n_images=feature_filenames.size();
   double FOV_u,FOV_v;
   vector<double> f_values;

   for (int imagenumber=0; imagenumber<n_images; imagenumber++)
   {
      cout << "imagenumber = " << imagenumber << endl;
      filefunc::ReadInfile(feature_filenames[imagenumber]);
   
      vector<twovector> XY,UV;
      for (unsigned int i=0; i<filefunc::text_line.size(); i++)
      {
         vector<string> substring=stringfunc::decompose_string_into_substrings(
            filefunc::text_line[i]);

         if (i < n_tiepoints)	
         {
            double U=stringfunc::string_to_number(substring[3]);
            double V=stringfunc::string_to_number(substring[4]);

// Need to recenter u0,v0 to lie at (0,0):

            double Uprime=U-0.5*aspect_ratio;
            double Vprime=V-0.5;

            UV.push_back(twovector(Uprime,Vprime));
         }
         else
         {
            double X=stringfunc::string_to_number(substring[0]);
            double Y=stringfunc::string_to_number(substring[1]);
            XY.push_back(twovector(X,Y));
         }
      }
   
//      for (int i=0; i<XY.size(); i++)
//      {
//         cout << "i = " << i 
//              << " X = " << XY[i].get(0) 
//              << " Y = " << XY[i].get(1) 
//              << " U = " << UV[i].get(0) 
//              << " V = " << UV[i].get(1)
//              << endl;
//      }

// Calculate homography relating image and world planes:

      homography H;
      H.parse_homography_inputs(XY,UV);
      H.compute_homography_matrix();
      H.compute_homography_inverse();
   
      double RMS_residual=H.check_homography_matrix(XY,UV);
      cout << "RMS_residual = " << RMS_residual << endl;
      cout << "H = " << H << endl;

      genmatrix* H_ptr=H.get_H_ptr();
//      cout << "*H_ptr = " << *H_ptr << endl;

// Calculate average value for proportionality constant lambda defined by 

// 		  (U,V,1) = lambda H (X,Y,1)

// Then rescale H by 1/lambda s.t. lambda -> 1:

      vector<double> lambda_values;
      for (unsigned int t=0; t<XY.size(); t++)
      {
         threevector XYZ(XY[t].get(0),XY[t].get(1),1);
//         cout << "XYZ = " << XYZ << endl;
         threevector HXYZ=*H_ptr * XYZ;
//      cout << "H*XYZ = " << HXYZ << endl;
         lambda_values.push_back(HXYZ.get(2));
//         cout << "lambda = " << lambda << endl;
      } // loop over index t labeling tiepoint pairs
      double lambda=mathfunc::mean(lambda_values);
      cout << "Average proportionality constant value lambda = " 
           << lambda << endl;
      *H_ptr = *H_ptr/lambda;

/*
      for (int t=0; t<XY.size(); t++)
      {
         threevector XYZ(XY[t].get(0),XY[t].get(1),1);
         cout << "t = " << t << " XYZ = " << XYZ << endl;
         threevector HXYZ=*H_ptr * XYZ;
         threevector UVW(UV[t].get(0),UV[t].get(1),1);
         cout << " H*XYZ = " << HXYZ 
              << " UVW = " << UVW << endl;
      }
*/

      double h11=H_ptr->get(0,0);
      double h12=H_ptr->get(0,1);
//      double h13=H_ptr->get(0,2);
      double h21=H_ptr->get(1,0);
      double h22=H_ptr->get(1,1);
//      double h23=H_ptr->get(1,2);
      double h31=H_ptr->get(2,0);
      double h32=H_ptr->get(2,1);
//      double h33=H_ptr->get(2,2);

//      cout << "h11 = " << h11 << " h12 = " << h12 << " h13 = " << h13 << endl;
//      cout << "h21 = " << h21 << " h22 = " << h22 << " h23 = " << h23 << endl;
//      cout << "h31 = " << h31 << " h32 = " << h32 << " h33 = " << h33 << endl;

/*
      genmatrix M(2,2),Minv(2,2);
      M.put(0,0,h11*h22);
      M.put(0,1,h21*h22);
      M.put(1,0,sqr(h11)-sqr(h12));
      M.put(1,1,sqr(h21)-sqr(h22));
      M.inverse(Minv);
      twovector X,Y;
      Y.put(0,-h31*h32);
      Y.put(1,sqr(h32)-sqr(h31));
      X=Minv*Y;
      cout << "X = " << X << endl;
*/

// Solve Eqn 32 for 1/fu**2 = 1/fv**2 where fu and fv are forced to be
// equal:

      double numer=h11*h12+h21*h22;
      double denom=h31*h32;
      double fsq=-numer/denom;
      cout << "fsq = " << fsq << endl;

// As of 1/21/12, we do NOT understand why we cannot get eqns 33 and
// 34 to even remotely work!  But at least eqn 32 does yield
// reasonable estimates for f**2 in most cases...

/*
      numer=sqr(h11)+sqr(h21);
      denom=1-sqr(h31);
      double fsq2=numer/denom;

      numer=sqr(h12)+sqr(h22);
      denom=1-sqr(h32);
      double fsq3=numer/denom;
      
      cout << "fsq2 = " << fsq2 << endl;
      cout << "fsq3 = " << fsq3 << endl;
*/

      double f=POSITIVEINFINITY;
      if (fsq > 0)
      {
         f=-sqrt(fsq);
         cout << "f = " << f << endl;

         f_values.push_back(f);

         camerafunc::horiz_vert_FOVs_from_f_and_aspect_ratio(
            f,aspect_ratio,FOV_u,FOV_v);
         cout << "FOV_u = " << FOV_u*180/PI << endl;
         cout << "FOV_v = " << FOV_v*180/PI << endl;

/*
         double Eqn33=sqr(h11/f)+sqr(h21/f)+sqr(h31);
         double Eqn34=sqr(h12/f)+sqr(h22/f)+sqr(h32);
         cout << "Eqn 33 = " << Eqn33 << endl;
         cout << "Eqn 34 = " << Eqn34 << endl;
*/

      }

//      outputfunc::enter_continue_char();
      cout << endl;
      
   } // loop over imagenumber index

   double numer=0;
   double denom=0;
   double f_median=mathfunc::median_value(f_values);
   for (unsigned int i=0; i<f_values.size(); i++)
   {
      double curr_f=f_values[i];
      if (fabs(curr_f-f_median) > 0.5) continue;
      numer += curr_f;
      denom += 1;
   }
   double f_avg=numer/denom;

   camerafunc::horiz_vert_FOVs_from_f_and_aspect_ratio(
      f_avg,aspect_ratio,FOV_u,FOV_v);
         
   cout << "======================================================" << endl;
   cout << "Best average estimate for f=fu=fv=" << f_avg << endl;
   cout << "FOV_u = " << FOV_u*180/PI << endl;
   cout << "FOV_v = " << FOV_v*180/PI << endl;
   cout << "Aspect ratio = width/height = " << aspect_ratio << endl;
   cout << "======================================================" << endl;
   cout << endl;

}
