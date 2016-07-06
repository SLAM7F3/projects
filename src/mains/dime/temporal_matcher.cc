// =======================================================================
// Program TEMPORAL_MATCHER performs feature matching for WISP
// panel images separated in time.  It first imports a set of images
// corresponding to fixed panels at different times.
// TEMPORAL_MATCHER next extracts SIFT and ASIFT features via calls to
// Lowe's SIFT binary and the affine SIFT library.  Consolidated SIFT
// & ASIFT interest points and descriptors are exported to key files
// following Lowe's conventions. TEMPORAL_MATCHER performs tiepoint
// matching via homography estimation and RANSAC.  2D tiepoints are
// converted to 3D rays via backprojection.  The best fit 3D rotation
// which maps t > 0 rays onto t=0 counterparts is computed and
// exported to an output text file.

/*

./temporal_matcher \
--newpass ./bundler/DIME/Feb2013_DeerIsland/images/panels/subsampled_panels/subsampled_horizon_p3_res0_00000.png \
--newpass ./bundler/DIME/Feb2013_DeerIsland/images/panels/subsampled_panels/subsampled_wisp_p3_res0_00060.png \
--image_list_filename ./bundler/DIME/Feb2013_DeerIsland/image_list.dat 

*/

// =======================================================================
// Last updated on 3/18/13; 3/19/13; 3/24/13
// =======================================================================

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "video/camerafuncs.h"
#include "osg/osgFeatures/FeaturesGroup.h"
#include "general/filefuncs.h"
#include "video/image_matcher.h"
#include "plot/metafile.h"
#include "numerical/param_range.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

using std::cout;
using std::endl;
using std::map;
using std::ofstream;
using std::pair;
using std::string;


double sinusoid(double u,int n_harmonic,double A,double phi,double Umax)
{
   double arg=2*PI*n_harmonic*u/Umax+phi;
   double v=A*sin(arg);
   return v;
}


int main( int argc, char** argv ) 
{
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   int videopass_ID=passes_group.get_videopass_ID();
//   cout << "videopass_ID = " << videopass_ID << endl;
//   string image_list_filename=passes_group.get_image_list_filename();
//   cout << "image_list_filename = " << image_list_filename << endl;
//   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);

   string bundler_IO_subdir="./bundler/DIME/";
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group);
   int n_images=photogroup_ptr->get_n_photos();
   cout << "n_images = " << n_images << endl;

   string blank_image_filename="./blank_40Kx2.2K.jpg";
   texture_rectangle* input_texture_rectangle_ptr=
      new texture_rectangle(blank_image_filename,NULL);
   texture_rectangle* output_texture_rectangle_ptr=
      new texture_rectangle(blank_image_filename,NULL);

// We explicitly confirmed on 1/30/13 that the FLANN library yields
// noticeably better feature matching results than the older ANN
// library:

   bool FLANN_flag=true;
   image_matcher SIFT(photogroup_ptr,FLANN_flag);
   SIFT.set_sampson_error_flag(true);

// Note added on 2/10/13: "root-SIFT" matching appears to yield
// inferior results for Affine-SIFT features than conventional "SIFT"
// matching !

   SIFT.set_root_sift_matching_flag(false);
//   SIFT.set_root_sift_matching_flag(true);

   string features_subdir=bundler_IO_subdir+"features/";
   filefunc::dircreate(features_subdir);

   timefunc::initialize_timeofday_clock();

// --------------------------------------------------------------------------
// Feature extraction starts here:

// Extract conventional SIFT features from each input image via
// Lowe's binary:

   string sift_keys_subdir=bundler_IO_subdir+"sift_keys/";
   bool delete_pgm_file_flag=true;
//   bool delete_pgm_file_flag=false;
   SIFT.extract_SIFT_features(sift_keys_subdir,delete_pgm_file_flag);
   outputfunc::print_elapsed_time();

   string asift_keys_subdir=bundler_IO_subdir+"asift_keys/";
   SIFT.extract_ASIFT_features(asift_keys_subdir);

   outputfunc::print_elapsed_time();

// Export consolidated sets of SIFT & ASIFT features to output keyfiles:

   string all_keys_subdir=bundler_IO_subdir+"all_keys/";
   filefunc::dircreate(all_keys_subdir);

   for (int i=0; i<n_images; i++)
   {
      photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(i);

      string basename=filefunc::getbasename(photo_ptr->get_filename());
      string prefix=stringfunc::prefix(basename);
      string all_keys_filename=all_keys_subdir+prefix+".key";

      cout << "all_keys_filename = " << all_keys_filename << endl;
      if (filefunc::fileexist(all_keys_filename))
      {
         cout << "all_keys_filename = " << all_keys_filename
              << " already exists in " << all_keys_subdir << endl;
      }
      else
      {
         SIFT.export_features_to_Lowe_keyfile(
            photo_ptr->get_ydim(),all_keys_filename,
            SIFT.get_image_feature_info(i));
         string banner="Exported "+all_keys_filename;
         outputfunc::write_banner(banner);
      }
   } // loop over index i labeling images

// --------------------------------------------------------------------------
// Feature matching starts here:

//   double max_ratio=0.5;    	// DIME
   double max_ratio=0.6;    	// DIME
//   double max_ratio=0.7;    	// OK for GEO
//   cout << "Enter max Lowe ratio:" << endl;
//   cin >> max_ratio;

   double sqrd_max_ratio=sqr(max_ratio);
   double worst_frac_to_reject=0;
//         double max_scalar_product=0.001;
//         double max_scalar_product=0.000333;
   double max_scalar_product=1E-4;

   double sqrt_max_sqrd_delta=0.015;
   cout << "Enter sqrt(max_sqrd_delta):" << endl;
//   cin >> sqrt_max_sqrd_delta;
   double max_sqrd_delta=sqr(sqrt_max_sqrd_delta);

   photograph* zeroth_photo_ptr=photogroup_ptr->get_photograph_ptr(0);      
   double Umax=zeroth_photo_ptr->get_maxU();

   int i=0;      
   for (int j=1; j<n_images; j++)
   {
      outputfunc::print_elapsed_time();
      cout << " j = " << j << endl;

      photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(j);      
      string input_image_filename=photo_ptr->get_filename();
//      cout << "input_image_filename = " << input_image_filename << endl;

      string horizons_subdir=filefunc::getdirname(input_image_filename);
//      cout << "horizons_subdir = " << horizons_subdir << endl;
      string image_basename=filefunc::getbasename(input_image_filename);
      string vcorrected_image_basename=
         image_basename.substr(11,image_basename.size()-11);
      string vcorrected_image_filename=horizons_subdir+
         vcorrected_image_basename;

      string uvcorrected_image_basename="u"+vcorrected_image_basename;
      string uvcorrected_image_filename=horizons_subdir+
         uvcorrected_image_basename;

//      cout << "vorrected_image_filename = "
//           << vcorrected_image_filename << endl;
//      cout << "uvorrected_image_filename = "
//           << uvcorrected_image_filename << endl;
      input_texture_rectangle_ptr->import_photo_from_file(
         vcorrected_image_filename);

// Match SIFT & ASIFT features across image pairs:

      string banner="Matching SIFT/ASIFT features:";
      outputfunc::write_big_banner(banner);

      SIFT.identify_candidate_feature_matches_via_Lowe_ratio_test(
         i,j,j,sqrd_max_ratio);

      if (!SIFT.identify_inlier_matches_via_homography(
         i,j,worst_frac_to_reject,max_sqrd_delta)) continue;

      SIFT.rename_feature_IDs(i,j); // Rename tiepoint pair labels
      SIFT.export_feature_tracks(i,features_subdir);
      SIFT.export_feature_tracks(j,features_subdir);

      int n_inliers=SIFT.get_inlier_XY().size();
      double avg_Delta=0;
      vector<double> X,U,Delta;
      for (int r=0; r<n_inliers; r++)
      {
         twovector curr_XY=SIFT.get_inlier_XY().at(r); // zeroth photo
         twovector curr_UV=SIFT.get_inlier_UV().at(r); // later photo
         X.push_back(curr_XY.get(0)); // zeroth photo
         U.push_back(curr_UV.get(0)); // later photo
         Delta.push_back(U.back()-X.back());
         avg_Delta += Delta.back();
      }
      avg_Delta /= n_inliers;
//      cout << "alpha = avg_Delta = " << avg_Delta << endl;

// Compute maximal absolute deviation of Delta from avg_Delta:

      double beta_start=0;
      double beta_stop=0;
      for (int r=0; r<n_inliers; r++)
      {
         beta_stop=max(fabs(Delta[r]-avg_Delta),beta_stop);
      }
//         cout << "beta_stop= " << beta_stop << endl;
      param_range beta(beta_start,beta_stop,7);
      param_range gamma(beta_start,beta_stop,7);

      double phi_start=0;
      double phi_stop=2*PI;
      param_range phi(phi_start,phi_stop,37);

      int n_iters=10;
      double min_score=POSITIVEINFINITY;
      for (int iter=0; iter<n_iters; iter++)
      {
         cout << "iter = " << iter << " of " << n_iters << endl;

         while (beta.prepare_next_value())
         {
            while (gamma.prepare_next_value())
            {
               while (phi.prepare_next_value())
               {

// Score match between each Delta measurement with current sinusoidal
// function:

                  double curr_score=0;
                  for (int j=0; j<Delta.size(); j++)
                  {
                     double curr_X=X[j];
//                     double curr_U=U[j];
                     double curr_Delta=Delta[j];
                     double fit_func=
                        avg_Delta+
                        sinusoid(curr_X,1,beta.get_value(),
                        phi.get_value(),Umax)+
                        sinusoid(curr_X,2,gamma.get_value(),
                        phi.get_value(),Umax);

                     double curr_residual=fabs(curr_Delta-fit_func);
                     curr_score += curr_residual;
                  } // loop over index j labeling Delta measurements
                  
                  if (curr_score < min_score)
                  {
                     min_score=curr_score;
                     beta.set_best_value();
                     gamma.set_best_value();
                     phi.set_best_value();
                     cout << "min_score = " << min_score
                          << " best_beta = " << beta.get_best_value()
                          << " best_gamma = " << gamma.get_best_value()
                          << " best_phi = " << phi.get_best_value()*180/PI
                          << endl;
                  }
                  
               } // phi while loop
            } // gamma while loop
         } // beta while loop

         double frac=0.55;
         beta.shrink_search_interval(beta.get_best_value(),frac);
         gamma.shrink_search_interval(gamma.get_best_value(),frac);
         phi.shrink_search_interval(phi.get_best_value(),frac);
      } // loop over iter index
         
      double best_beta=beta.get_best_value();
      double best_gamma=gamma.get_best_value();
      double best_phi=phi.get_best_value();
      best_phi=basic_math::phase_to_canonical_interval(best_phi,0,2*PI);

      cout << endl;
      cout << "min_score = " << min_score << endl;
      cout << "Best beta value = " << best_beta << endl;
      cout << "Best gamma value = " << best_gamma << endl;
      cout << "Best phi value = " << best_phi*180/PI << endl;

      camerafunc::ucorrect_WISP_image(
         input_texture_rectangle_ptr,
         output_texture_rectangle_ptr,
         avg_Delta,best_beta,best_gamma,best_phi);
      output_texture_rectangle_ptr->write_curr_frame(
         uvcorrected_image_filename);

// Generate curve corresponding to sinusoidal fit to Delta=U-X vs X:

      int n_Xbins=360;
      double dX=Umax/(n_Xbins-1);
      vector<double> sinusoid_indep,sinusoid_depend;
      for (int n=0; n<n_Xbins; n++)
      {
         double curr_X=0+n*dX;
         double Delta_fit=avg_Delta
            +sinusoid(curr_X,1,best_beta,best_phi,Umax)
            +sinusoid(curr_X,2,best_gamma,best_phi,Umax);
         sinusoid_indep.push_back(curr_X);
         sinusoid_depend.push_back(Delta_fit);
      }

      string metafile_filename=features_subdir+"Delta_vs_X";
      string title="Delta vs X";
      string x_label="X";
      string y_label="Delta";
      double x_min=0;
      double x_max=Umax;
      double y_min=avg_Delta-1.1*beta_stop;
      double y_max=avg_Delta+1.1*beta_stop;

      metafile* metafile_ptr=new metafile();
      metafile_ptr->set_parameters(
         metafile_filename,title,x_label,y_label,x_min,x_max,y_min,y_max);
      metafile_ptr->openmetafile();
      metafile_ptr->write_header();
      metafile_ptr->write_markers(X,Delta);
      metafile_ptr->write_curve(
         sinusoid_indep,sinusoid_depend,colorfunc::blue);
      metafile_ptr->closemetafile();
      delete metafile_ptr;

      string unix_cmd="/home/cho/bin/meta_to_jpeg "+features_subdir+
         "Delta_vs_X";
      sysfunc::unix_command(unix_cmd);

   } // loop over index j labeling input images

   outputfunc::print_elapsed_time();

   delete input_texture_rectangle_ptr;
   delete output_texture_rectangle_ptr;

}
