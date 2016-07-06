// ==========================================================================
// Program TIEPOINTS reads in a set of input ground digital photos.
// It then uses SIFT feature matching, ANN candidate pair searching
// and RANSAC outlier determination to identify feature tiepoint
// pairs.  TIEPOINTS exports feature information to output feature
// text files which can be read in by programs VIDEO and PANORAMA.
// Only features which appear in at least 2 photos are written out.

/*

/home/cho/programs/c++/svn/projects/src/mains/sift/tiepoints \
--newpass ./images/Green_bldg_rooftop_towards_Boston/Green_bldg_rooftop_towards_Boston-055.JPG \
--newpass ./images/Oct15_2008/videos/subframes/frame_25.png \

*/

// ==========================================================================
// Last updated on 4/6/12; 9/27/12; 11/12/12
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <lmcurve.h>
#include "osg/osgFeatures/FeaturesGroup.h"
#include "general/filefuncs.h"
#include "structmotion/fundamental.h"
#include "video/image_matcher.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "geometry/plane.h"
#include "math/quaternion.h"
#include "general/sysfuncs.h"

#include "numrec/nrfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ios;
using std::ofstream;
using std::ostream;
using std::pair;
using std::string;
using std::vector;


// n_params = dimension of parameter vector param

// par = parameter vector.  On input, it must contain reasonable guess
// entries. On output, it contains soln found to minimize |fvec| .

// m_dat = dimension of residue vector fvec
// m_dat >= n_par

// data  = pointer forwarded to evaluate and printout

// evaluate = routine that calculates residue vector fvec for given
// parameter vector par

// *info = setting *info to negative value causes lm_minimize to terminate


void gold_standard_cost(
   const double* par, int m_dat, const void* data, double* fvec, int* info)
{
//   cout << "inside gold_standard_cost()" << endl;
   
   void* data_new=const_cast<void*>(data);
   int n_points=static_cast<double*>(data_new)[0];
//   cout << "n_points = " << n_points << endl;

   int data_counter=1;
   vector<twovector> x,xprime;
   for (int n=0; n<n_points; n++)
   {
      double u=static_cast<double*>(data_new)[data_counter+2*n];
      double v=static_cast<double*>(data_new)[data_counter+2*n+1];
      x.push_back(twovector(u,v));
//      cout << "n = " << n << " u = " << u << " v = " << v << endl;
   }
   data_counter += 2*n_points;

   for (int n=0; n<n_points; n++)
   {
      double u_prime=static_cast<double*>(data_new)[data_counter+2*n];
      double v_prime=static_cast<double*>(data_new)[data_counter+2*n+1];
      xprime.push_back(twovector(u_prime,v_prime));
//      cout << "n = " << n << " u' = " << u_prime 
//           << " v' = " << v_prime << endl;
   }
   data_counter += 2*n_points;

   int param_counter=0;
   vector<fourvector> X;
   for (int n=0; n<n_points; n++)
   {
      fourvector curr_X(par[3*n+0],par[3*n+1],par[3*n+2],1);
      X.push_back(curr_X);
   } // loop over index n 
   param_counter += 3*n_points;
   
   genmatrix P(3,4);
   P.clear_values();
   P.put(0,0,1);
   P.put(1,1,1);
   P.put(2,2,1);

   genmatrix Pprime(3,4);
   for (int r=0; r<3; r++)
   {
      for (int c=0; c<4; c++)
      {
         Pprime.put(r,c,par[param_counter]);
         param_counter++;
      }
   } // loop over index r

   vector<twovector> x_hat,xprime_hat;
   for (int n=0; n<n_points; n++)
   {
      x_hat.push_back(twovector(P*X[n]));
      xprime_hat.push_back(twovector(Pprime*X[n]));
   }

   double cost=0;
   for (int n=0; n<n_points; n++)
   {
      twovector delta_x(x_hat[n]-x[n]);
      twovector delta_xprime(xprime_hat[n]-xprime[n]);
      cost  += delta_x.sqrd_magnitude()+delta_xprime.sqrd_magnitude();
   }
   cout << "cost = " << cost << endl;
   fvec[0]=cost;
   fvec[1]=cost;
}

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   int videopass_ID=passes_group.get_videopass_ID();
//   cout << "videopass_ID = " << videopass_ID << endl;


// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group);

//   bool FLANN_flag=false;
   bool FLANN_flag=true;
   image_matcher SIFT(photogroup_ptr,FLANN_flag);

// For FLIR video images, do not accept SIFT features at very
// top/bottom of image nor too far to the left:

//   SIFT.set_min_allowed_V(0.136);
//   SIFT.set_max_allowed_V(0.878);
   
   string sift_keys_subdir="./";
//   bool delete_pgm_file_flag=true;
   bool delete_pgm_file_flag=false;
   SIFT.extract_SIFT_features(sift_keys_subdir,delete_pgm_file_flag);

/*
   int n_rows=50;
   int n_columns=50;
   SIFT.extract_HOG_features(n_rows,n_columns);

   double max_ratio=0.80;
   double worst_frac_to_reject=0.10;
   double max_scalar_product=0.01;
   SIFT.identify_CHOG_feature_matches_via_fundamental_matrix(
      max_ratio,worst_frac_to_reject,max_scalar_product);
*/

//   SIFT.extract_CHOG_features(2500);
//   SIFT.print_features(3);

// SIFT feature matching becomes LESS stringent as sqrd_max_ratio
// increases.

// SIFT tiepoint inlier identification becomes LESS stringent as
// max_scalar_product increases.

/*
// HOG matching:

   const int n_min_quadrant_features=1;		
   const double sqrd_max_ratio=sqr(0.85);
   const double worst_frac_to_reject=0.15;
   const double max_scalar_product=0.01;
*/

// As of late 2011, we believe the following parameters are reasonable
// for fundamental matrix tiepoint matching on generic image pairs:

   const int n_min_quadrant_features=1;		
//   const int n_min_quadrant_features=2;

//   const double sqrd_max_ratio=sqr(0.6);	
//   const double sqrd_max_ratio=sqr(0.675);
   double max_ratio=0.7;
   cout << "Enter Lowe ratio threshold:" << endl;
   cout << "(Default value = 0.7)" << endl;
   cin >> max_ratio;
   double sqrd_max_ratio=sqr(max_ratio);

   double worst_frac_to_reject=0;

   double max_scalar_product=0.001;
   cout << "Enter max value for fundamental matrix scalar product:" << endl;
   cout << "(Default value = 0.001)" << endl;
   cin >> max_scalar_product;

// SIFT tiepoint inlier identification becomes LESS stringent as
// max_sqrd_delta increases.

//   const int n_min_quadrant_features=1;		// marriott example
//   const double sqrd_max_ratio=sqr(0.9);	// marriott example
//   const double worst_frac_to_reject=0.001;
//   const double max_sqrd_delta=sqr(0.2);


//   SIFT.identify_candidate_feature_matches_via_homography(
//      n_min_quadrant_features,sqrd_max_ratio,worst_frac_to_reject,
//      max_sqrd_delta);

   SIFT.identify_candidate_feature_matches_via_fundamental_matrix(
      n_min_quadrant_features,sqrd_max_ratio,worst_frac_to_reject,
      max_scalar_product);

// Export matched features to output html file:

   FeaturesGroup* FeaturesGroup_ptr=new FeaturesGroup(
      ndims,passes_group.get_pass_ptr(videopass_ID),NULL);

   string features_subdir="./features/";
   FeaturesGroup_ptr->read_in_photo_features(photogroup_ptr,features_subdir);
   bool output_only_multicoord_features_flag=true;
   FeaturesGroup_ptr->write_feature_html_file(
      photogroup_ptr,output_only_multicoord_features_flag);

 

   fundamental* fundamental_ptr=SIFT.get_fundamental_ptr();
   genmatrix* F_ptr=fundamental_ptr->get_F_ptr();

/*
// Kermit2 - Kermit 0 (after 2500 iterations):

   F_ptr->put(0,0,0.0540883607736);
   F_ptr->put(0,1,0.522405344665);
   F_ptr->put(0,2,-0.723810898926);
   F_ptr->put(1,0,0.709012990053);
   F_ptr->put(1,1,0.18360697482);
   F_ptr->put(1,2,-3.22897943509);
   F_ptr->put(2,0,-0.682319061899);
   F_ptr->put(2,1,2.06716772825);
   F_ptr->put(2,2,1);
*/

   cout.precision(12);
   cout << "*fundamental_ptr = " << *fundamental_ptr << endl;
   cout << "fundamental rank = " << F_ptr->rank() << endl;
   cout << "Det(F) = " << F_ptr->determinant() << endl;
   
   cout << "eprime epipole = " << fundamental_ptr->get_epipole_XY()
        << endl;
   cout << "e epipole = " << fundamental_ptr->get_epipole_UV()
        << endl;

   cout << endl;
   for (int r=0; r<3; r++)
   {
      for (int c=0; c<3; c++)
      {
         cout << "F_ptr->put(" << r << "," << c << ","
              << F_ptr->get(r,c) << ");" << endl;
      }
   }
   cout << endl;

   vector<twovector> inlier_XY=SIFT.get_inlier_XY();
   vector<twovector> inlier_UV=SIFT.get_inlier_UV();
   int n_inliers=inlier_XY.size();

   genmatrix* P_ptr=fundamental_ptr->compute_trivial_projection_matrix();
   genmatrix* Pprime_ptr=fundamental_ptr->
      compute_nontrivial_projection_matrix();

   vector<threevector> triangulated_points;
   for (int i=0; i<n_inliers; i++)
   {
      triangulated_points.push_back(
         fundamental_ptr->triangulate_noisy_tiepoints(
            inlier_UV[i],inlier_XY[i]));
//      cout << "i = " << i 
//           << " triangulated point = " << triangulated_points[i]
//           << endl;
   }

   exit(-1);

   cout << "Before starting LM iterations:" << endl;
   cout << "*fundamental_ptr = " << *fundamental_ptr << endl;

   for (int n=0; n<n_inliers; n++)
   {
      threevector curr_XY(inlier_XY[n],1);
      threevector curr_UV(inlier_UV[n],1);
      threevector FUV=(*F_ptr) * curr_UV;
      double scalar_product=curr_XY.dot(FUV);
      cout << "n = " << n << " scalar product = " << scalar_product
           << endl;
   }

// Perform LM refinement of initial estimate for fundamental matrix:

// Initialize independent variables array

   cout << "n_inliers = " << n_inliers << endl;
   int n_params = 3*n_inliers+12; // number of parameters in model function f
   cout << "n_params = " << n_params << endl;
   
   double param[n_params];

   for (int n=0; n<n_inliers; n++)
   {
      param[3*n+0]=triangulated_points[n].get(0);
      param[3*n+1]=triangulated_points[n].get(1);
      param[3*n+2]=triangulated_points[n].get(2);
   }
   int param_counter=3*n_inliers;
   for (int r=0; r<3; r++)
   {
      for (int c=0; c<4; c++)
      {
         cout << "r = " << r << " c = " << c
              << " param_counter = " << param_counter << endl;
         param[param_counter]=Pprime_ptr->get(r,c);
         param_counter++;
      }
   }

// Fill parameters array

   int m_dat = 1+2*2*n_inliers;
   double data[m_dat];

   data[0]=n_inliers;

   cout << "data[0] = " << data[0] << endl;

   int data_counter=1;
   vector<twovector> x,xprime;
   for (int n=0; n<n_inliers; n++)
   {
      data[data_counter+2*n]=inlier_UV[n].get(0);
      data[data_counter+2*n+1]=inlier_UV[n].get(1);
   }
   data_counter += 2*n_inliers;

   for (int n=0; n<n_inliers; n++)
   {
      data[data_counter+2*n]=inlier_XY[n].get(0);
      data[data_counter+2*n+1]=inlier_XY[n].get(1);
   }
   data_counter += 2*n_inliers;

   cout << "m_dat = " << m_dat << endl;
   cout << "data_counter = " << data_counter << endl;
   
   void* data_ptr=static_cast<void*>(data);

// Auxiliary parameters:

   lm_status_struct status;
   lm_control_struct control = lm_control_double;
   control.printflags = 0;
//   control.printflags = 3; // monitor status (+1) and parameters (+2)
//   control.ftol=1E-9;
//   control.xtol=1E-9;
//   control.gtol=1E-9;
   control.maxcall=50000;

// Perform the LM fit:

   cout << "Performing LM fit:" << endl;
   lmmin( 
      n_params, param, m_dat, data_ptr,
      gold_standard_cost, &control, &status, lm_printout_std );

// Print LM minimization results:
    
   cout << "Fitted parameter values:" << endl;
   for ( int i=0; i<n_params; i++ )
   {
      cout << "param[" << i << "] = " << param[i] << endl;
   }

   for (int i=n_inliers-2; i<n_inliers; i++)
   {
      cout << "i = " << i 
           << " triangulated point = " << triangulated_points[i]
           << endl;
   }
   cout << "*Pprime_ptr = " << *Pprime_ptr << endl;



   cout << "Fit results:" << endl;
   cout << "Status after " << status.nfev << " function evaluations : "
        << lm_infmsg[status.info] << endl;
  
   cout << "Residual norm = " << status.fnorm << endl;

}

   
