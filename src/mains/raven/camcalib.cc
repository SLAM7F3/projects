// ==========================================================================
// Program CAMCALIB implements the "checkerboard" calibration
// algorithm described in Z. Zhang, "A flexible new technique for
// camera calibration", IEEE Trans on Pattern Analysis and Machine
// Intelligence, 22 (1330) 2000.  It first reads in a set of
// image-plane UV and corresponding world-plane XY tiepoints from some
// number of photos (preferably 4 or more).  The least-squares
// homography relating the image and world planes is next
// calculated. A 2n_image x 6 data matrix is subsequently formed, and
// its singular value decomposition is evaluated.  Internal camera
// parameters fu, fv, u0, v0 and pixel skew parameter gamma are
// extracted from the SVD results.

//				camcalib

// Important Note: As of 1/21/12, we believe unconstrained
// "checkerboard" camera calibration is NOT generally robust!  If we
// force fu=fv=f; skew=0; u0=0.5*aspect_ratio and v0=0.5, then we can
// pull out quasi-reasonable estimates for f.  So this program
// CAMCALIB is deprecated!  See instead program ESTIMATE_F.

// ==========================================================================
// Last updated on 1/19/12; 1/20/12; 1/21/12
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

   string features_subdir="./calib/simul_camera/";
   feature_filenames.push_back(features_subdir+"features_2D_vs1.txt");
//   feature_filenames.push_back(features_subdir+"features_2D_vs2.txt");
//   feature_filenames.push_back(features_subdir+"features_2D_vs3.txt");
//   feature_filenames.push_back(features_subdir+"features_2D_vs4.txt");

/*
   string features_subdir="./calib/calib_front_camera/grid/cropped_frames/";
   feature_filenames.push_back(features_subdir+"features_2D_frame0007.txt");
   feature_filenames.push_back(features_subdir+"features_2D_frame0021.txt");
   feature_filenames.push_back(features_subdir+"features_2D_frame0035.txt");
   feature_filenames.push_back(features_subdir+"features_2D_frame0055.txt");
   feature_filenames.push_back(features_subdir+"features_2D_frame0088.txt");
*/

/*
   string features_subdir="./calib/calib_side_camera/grid/";
   feature_filenames.push_back(features_subdir+"features_2D_frame0006.txt");
   feature_filenames.push_back(features_subdir+"features_2D_frame0110.txt");
   feature_filenames.push_back(features_subdir+"features_2D_frame0173.txt");
   feature_filenames.push_back(features_subdir+"features_2D_frame0203.txt");
*/

/*
   string features_subdir="./calib/calib_front_camera/grid/";
   feature_filenames.push_back(features_subdir+"features_2D_frame0007.txt");
   feature_filenames.push_back(features_subdir+"features_2D_frame0021.txt");
   feature_filenames.push_back(features_subdir+"features_2D_frame0035.txt");
   feature_filenames.push_back(features_subdir+"features_2D_frame0055.txt");
   feature_filenames.push_back(features_subdir+"features_2D_frame0088.txt");
*/

/* 
   feature_filenames.push_back("./data/calib/features_2D_IMG_0133.txt");
   feature_filenames.push_back("./data/calib/features_2D_IMG_0134.txt");
   feature_filenames.push_back("./data/calib/features_2D_IMG_0135.txt");
   feature_filenames.push_back("./data/calib/features_2D_IMG_0136.txt");
*/

   int n_images=feature_filenames.size();
   int row=0;
   genmatrix A(2*n_images,6);

   for (int imagenumber=0; imagenumber<n_images; imagenumber++)
   {
      cout << "imagenumber = " << imagenumber << endl;
      filefunc::ReadInfile(feature_filenames[imagenumber]);
   
      vector<twovector> XY,UV;
      for (int i=0; i<filefunc::text_line.size(); i++)
      {
         vector<string> substring=stringfunc::decompose_string_into_substrings(
            filefunc::text_line[i]);

         if (i < 8)	// simulated image 
//         if (i < 9)
         {
            double U=stringfunc::string_to_number(substring[3]);
            double V=stringfunc::string_to_number(substring[4]);
            UV.push_back(twovector(U,V));
         }
         else
         {
            double X=stringfunc::string_to_number(substring[0]);
            double Y=stringfunc::string_to_number(substring[1]);
            XY.push_back(twovector(X,Y));
         }
      }
   
      for (int i=0; i<XY.size(); i++)
      {
         cout << "i = " << i 
              << " X = " << XY[i].get(0) 
              << " Y = " << XY[i].get(1) 
              << " U = " << UV[i].get(0) 
              << " V = " << UV[i].get(1)
              << endl;
      }

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

      threevector h1,h2,h3;
      H_ptr->get_column(0,h1);
      H_ptr->get_column(1,h2);
      H_ptr->get_column(2,h3);
//      cout << "h1 = " << h1 << " h2 = " << h2 << " h3 = " << h3 << endl;

      genvector V11(6),V22(6),V12(6);
   
      V11.put(0,h1.get(0)*h1.get(0));
      V11.put(1,h1.get(0)*h1.get(1)+h1.get(1)*h1.get(0));
      V11.put(2,h1.get(1)*h1.get(1));
      V11.put(3,h1.get(2)*h1.get(0)+h1.get(0)*h1.get(2));
      V11.put(4,h1.get(2)*h1.get(1)+h1.get(1)*h1.get(2));
      V11.put(5,h1.get(2)*h1.get(2));

      V22.put(0,h2.get(0)*h2.get(0));
      V22.put(1,h2.get(0)*h2.get(1)+h2.get(1)*h2.get(0));
      V22.put(2,h2.get(1)*h2.get(1));
      V22.put(3,h2.get(2)*h2.get(0)+h2.get(0)*h2.get(2));
      V22.put(4,h2.get(2)*h2.get(1)+h2.get(1)*h2.get(2));
      V22.put(5,h2.get(2)*h2.get(2));

      V12.put(0,h1.get(0)*h2.get(0));
      V12.put(1,h1.get(0)*h2.get(1)+h1.get(1)*h2.get(0));
      V12.put(2,h1.get(1)*h2.get(1));
      V12.put(3,h1.get(2)*h2.get(0)+h1.get(0)*h2.get(2));
      V12.put(4,h1.get(2)*h2.get(1)+h1.get(1)*h2.get(2));
      V12.put(5,h1.get(2)*h2.get(2));

// Fill rows of 2n_images x 6 data matrix A:

      for (int c=0; c<6; c++)
      {
         A.put(row,c,V12.get(c));
         A.put(row+1,c,V11.get(c)-V22.get(c));
      }
      row += 2;

   } // loop over imagenumber index

   cout << "Data matrix A = " << A << endl;

   genmatrix B(3,3);
   genvector b(6);
   if (A.homogeneous_soln(b))
   {
      B.put(0,0,b.get(0));
      B.put(0,1,b.get(1));
      B.put(1,0,b.get(1));
      B.put(1,1,b.get(2));

      B.put(0,2,b.get(3));
      B.put(1,2,b.get(4));
      B.put(2,2,b.get(5));

      B.put(2,0,b.get(3));
      B.put(2,1,b.get(4));
      B.put(2,2,b.get(5));
   }
   cout << "B = " << B << endl;

   outputfunc::enter_continue_char();
 
   double numer=B.get(0,1)*B.get(0,2)-B.get(0,0)*B.get(1,2);
   double denom=B.get(0,0)*B.get(1,1)-sqr(B.get(0,1));
   double v0=numer/denom;
   
   double term1=sqr(B.get(0,2))+
      v0*(B.get(0,1)*B.get(0,2)-B.get(0,0)*B.get(1,2));
   double lambda=B.get(2,2)-term1/B.get(0,0);
   cout << "lambda = " << lambda << endl;
   cout << "denom = " << denom << endl;
   
   double fu=-sqrt(lambda/B.get(0,0));
   double fv=-sqrt(lambda*B.get(0,0)/denom);
   double f=0.5*(fu+fv);
   double gamma=-B.get(0,1)*sqr(fu)*fv/lambda;
   double u0=gamma*v0/fv-B.get(0,2)*sqr(fu)/lambda;

   string banner="Least square values for camera internal parameters:";
   outputfunc::write_big_banner(banner);

   cout << "fu = " << fu << endl;
   cout << "fv = " << fv << endl;
   cout << "0.5*(fu+fv) = " << f << endl;
   cout << "u0 = " << u0 << endl;
   cout << "v0 = " << v0 << endl;
   cout << "gamma = " << gamma << endl;
  
   double aspect_ratio=1067.0/800.0;	// Simulated image
//   double aspect_ratio=647.0/480.0;	// Cropped Raven front video frame
//   double aspect_ratio=720.0/480.0;	// Uncropped Raven video frame
   cout << "True aspect ratio = width/height = " << aspect_ratio << endl;
   cout << "0.5*aspect_ratio = " << 0.5*aspect_ratio << endl;

   double FOV_u,FOV_v;
   camerafunc::horiz_vert_FOVs_from_f_and_aspect_ratio(
      f,aspect_ratio,FOV_u,FOV_v);

   cout << "FOV_u = " << FOV_u*180/PI << endl;
   cout << "FOV_v = " << FOV_v*180/PI << endl;
}
