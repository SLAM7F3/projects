// ==========================================================================
// Program CLEAN_L1 crops and squishes level-1 ALIRT Puerto Rico
// imagery read in from input BPF or TDP files.  It first fits a line
// to the subsampled level-1 points to determine a quite good estimate
// for the average range direction. After projecting all raw points
// onto the line and forming a 1D distribution, CLEAN_L1 finds
// conservative min/max threshold values along the range direction
// that encompass the bulk of genuine signal and crops out this middle
// section.  It next finds z-plane slices which bound the genuine
// signal and performs a further cropping.  Surviving XYZ points are
// voxelized into a 3D grid. Finally, CLEAN_L1 identifies and squishes
// "comet tails" in the uprange direction.  The cropped and squished
// voxel counts are exported to a TDP file for later aggregation by
// program DENSITY_L1.

//	clean_L1 ./tdp_files/02142010_081452_1064nm_161747-165163.xform.tdp 

// ==========================================================================
// Last updated on 11/29/11; 12/4/11; 12/8/11
// ==========================================================================

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "threeDgraphics/bpffuncs.h"
#include "color/colorfuncs.h"
#include "image/drawfuncs.h"
#include "general/filefuncs.h"
#include "math/fourvector.h"
#include "geometry/linesegment.h"
#include "plot/metafile.h"
#include "math/mypolynomial.h"
#include "numerical/param_range.h"
#include "geometry/plane.h"
#include "math/prob_distribution.h"
#include "math/rotation.h"
#include "general/sysfuncs.h"
#include "osg/osg3D/tdpfuncs.h"
#include "video/texture_rectangle.h"
#include "coincidence_processing/VolumetricCoincidenceProcessor.h"

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

// Constants 

   const double voxel_binsize=0.25;	// meter
//   const double voxel_binsize=1.0;	// meter

// Repeated variable declarations:

   string banner,unix_cmd="";
   bool perturb_voxels_flag;
   double min_prob_threshold=0;

// Parse input BPF or TDP file containing level-1 ALIRT imagery:
 
   string raw_input_filename(argv[1]);
   string dirname=filefunc::getdirname(raw_input_filename);
   string basename=filefunc::getbasename(raw_input_filename);
   string prefix=stringfunc::prefix(basename);
   string suffix=stringfunc::suffix(basename);

//   cout << "filename = " << raw_input_filename << endl;
//   cout << "dirname = " << dirname << endl;
//   cout << "prefix = " << prefix << endl;
//   cout << "suffix = " << suffix << endl;

   int UTM_zonenumber,n_frames=-1;
   vector<double>* Xraw_ptr=new vector<double>;
   vector<double>* Yraw_ptr=new vector<double>;
   vector<double>* Zraw_ptr=new vector<double>;
   vector<int>* pixel_number_ptr=new vector<int>;

   if (suffix=="tdp")
   {
      tdpfunc::read_XYZ_points_from_tdpfile(
         raw_input_filename,*Xraw_ptr,*Yraw_ptr,*Zraw_ptr);
   }
   else if (suffix=="bpf")
   {
      n_frames=bpffunc::read_bpf_L1_data(
         raw_input_filename,Xraw_ptr,Yraw_ptr,Zraw_ptr,
         pixel_number_ptr,UTM_zonenumber);
   }
   
   int n_points=Zraw_ptr->size();
   cout << "Number of raw XYZ points = " << n_points << endl;
   cout << "Number of frames = " << n_frames << endl;

// Scan through *pixel_number_ptr for negative values which correspond
// to embedded frustum info.  Compute frustum corner rays for each
// sensor IFOV:

   vector<threevector> sensor_posn;
   vector<threevector> frustum_ray[4];
   
   if (pixel_number_ptr->size() > 0)
   {
      for (int i=0; i<n_points; i++)
      {
         int pixel_number=-pixel_number_ptr->at(i);
         if (pixel_number > 0 && pixel_number < 5)
         {
            frustum_ray[pixel_number-1].push_back(threevector(
               Xraw_ptr->at(i),Yraw_ptr->at(i),Zraw_ptr->at(i)));
         }
         else if (pixel_number==5)
         {
            sensor_posn.push_back(threevector(
               Xraw_ptr->at(i),Yraw_ptr->at(i),Zraw_ptr->at(i)));
//            cout << "sensor posn = " << sensor_posn.back().get(0) << " "
//                 << sensor_posn.back().get(1) << " "
//                 << sensor_posn.back().get(2) << endl;
         }
      } // loop over index i labeling raw points
   } // pixel_number.size > 0 conditional
//   cout << "sensor_posn.size() = " << sensor_posn.size() << endl;

// Fit line to sensor track:

   threevector sensor_posn_origin(sensor_posn[0]);
   threevector t_hat; // points along track's XY direction
   for (unsigned int i=1; i<sensor_posn.size(); i++)
   {
      threevector t_vec=sensor_posn[i]-sensor_posn_origin;
      t_vec.put(2,0);
      t_hat += t_vec.unitvector();
   }
   t_hat /= (sensor_posn.size()-1);
   threevector s_hat(t_hat.get(1),-t_hat.get(0),0);

//   cout << "sensor_posn_origin = " << sensor_posn_origin << endl;
//   cout << "t_hat = " << t_hat << endl;
//   cout << "s_hat = " << s_hat << endl;
//   cout << "t.s = " << t_hat.dot(s_hat) << endl;

// Calculate average sensor position and frustum pointing direction
// vector for entire set of input frames:

   threevector avg_sensor_posn(0,0,0);
   threevector avg_frustum_ray(0,0,0);
   for (unsigned int j=0; j<sensor_posn.size(); j++)
   {
      threevector curr_sensor_posn=sensor_posn.at(j);
      avg_sensor_posn += curr_sensor_posn;
      for (int c=0; c<4; c++)
      {
         frustum_ray[c].at(j)=(frustum_ray[c].at(j)-curr_sensor_posn).
            unitvector();
         avg_frustum_ray += frustum_ray[c].at(j);
      } // loop over index c labeling frustum rays
   } // loop over index j labeling sensor IFOVs

   avg_sensor_posn /= sensor_posn.size();
   avg_frustum_ray /= (4*sensor_posn.size());
   threevector origin=avg_sensor_posn;
   cout << "origin = " << origin << endl;

// Require n_hat (= negative range direction vector) to point from
// ground up towards aircraft:

   threevector n_hat=-avg_frustum_ray;
   cout << "n_hat = " << n_hat << endl;

/*
   banner="Finding extremal Z values";
   outputfunc::write_big_banner(banner);

   double Zmin=mathfunc::minimal_value(*Zraw_ptr);
   double Zmax=mathfunc::maximal_value(*Zraw_ptr);
//   cout << "Zmin = " << Zmin << " Zmax = " << Zmax << endl;

   threevector l_start(XZ_poly.value(Zmin),YZ_poly.value(Zmin),Zmin);
   threevector l_stop(XZ_poly.value(Zmax),YZ_poly.value(Zmax),Zmax);
   linesegment l(l_start,l_stop);

// Generate TDP & OSGA files displaying fitted line:

   Zmin -= 200;
   Zmax += 200;
   double z=Zmin;
   double dz=1;
   vector<threevector> line_XYZ,avg_ray;
   int counter=0;
   while (z < Zmax)
   {
      double x=XZ_poly.value(z);
      double y=YZ_poly.value(z);
      line_XYZ.push_back(threevector(x,y,z));
      avg_ray.push_back(avg_sensor_posn+(2000+counter)*avg_frustum_ray);
      z += dz;
      counter++;
   }
   
   string line_tdp_filename="fitted_line.tdp";
   tdpfunc::write_xyz_data("",line_tdp_filename,line_XYZ);
   unix_cmd="lodtree "+line_tdp_filename;
   sysfunc::unix_command(unix_cmd);

   string avg_ray_tdp_filename="avg_ray.tdp";
   tdpfunc::write_xyz_data("",avg_ray_tdp_filename,avg_ray);
   unix_cmd="lodtree "+avg_ray_tdp_filename;
   sysfunc::unix_command(unix_cmd);
*/

// Project all raw points onto line running through average sensor
// position in avg_frustum_ray direction:

   banner="Projecting raw points onto fitted line:";
   outputfunc::write_big_banner(banner);
   
   vector<double>* n_ptr=new vector<double>;
   n_ptr->reserve(6000000);

   cout << "Number of raw points = " << n_points << endl;

   int n_sampled_points=50000*9;
//   int n_sampled_points=5000000;
   int istep=n_points/n_sampled_points;

//   vector<double>* Xsampled_raw_ptr=new vector<double>;
//   vector<double>* Ysampled_raw_ptr=new vector<double>;
//   vector<double>* Zsampled_raw_ptr=new vector<double>;

   for (int i=0; i<n_points; i += istep)
   {
      if (pixel_number_ptr->at(i) < 0) continue;

      threevector curr_r(
         Xraw_ptr->at(i)-origin.get(0),Yraw_ptr->at(i)-origin.get(1),
         Zraw_ptr->at(i)-origin.get(2));
      n_ptr->push_back(curr_r.dot(n_hat));

//      Xsampled_raw_ptr->push_back(Xraw_ptr->at(i));
//      Ysampled_raw_ptr->push_back(Yraw_ptr->at(i));
//      Zsampled_raw_ptr->push_back(Zraw_ptr->at(i));
      
   }
   cout << endl << endl;

/*
   banner="Generating sampled raw TDP file";
   outputfunc::write_big_banner(banner);
   string sampled_raw_tdp_filename="sampled_raw_points.tdp";
   cout << "Sampled raw tdp filename = " << sampled_raw_tdp_filename << endl;
   tdpfunc::write_xyz_data(
      sampled_raw_tdp_filename,Xsampled_raw_ptr,Ysampled_raw_ptr,
      Zsampled_raw_ptr);
   unix_cmd="lodtree "+sampled_raw_tdp_filename;
//   sysfunc::unix_command(unix_cmd);

   delete Xsampled_raw_ptr;
   delete Ysampled_raw_ptr;
   delete Zsampled_raw_ptr;
*/

// Search for peak within n-distribution of projected raw points:

   int n_output_bins=10000;
   prob_distribution prob_n(*n_ptr,n_output_bins);
//   prob_n.writeprobdists(false);
   delete n_ptr;

   string prob_dist_meta_filename=dirname+"prob_density.meta";
   string prob_dist_jpg_filename=dirname+"prob_density.jpg";
   string prob_ndist_jpg_filename=dirname+"prob_n_density.jpg";
   unix_cmd="mv "+prob_dist_jpg_filename+" "+prob_ndist_jpg_filename;
//   sysfunc::unix_command(unix_cmd);

   int n_max_bin=-1;
   double peak_density=prob_n.peak_density_value(n_max_bin);
   double n_peak=prob_n.get_x(n_max_bin);
   cout << "n_peak = " << n_peak << " peak_density = " << peak_density << endl;

// Crop away points which do not lie within interval [n_peak-delta_n_neg,
// n_peak+delta_n_pos] : 

   banner="Cropping points not in interval around n_peak";
   outputfunc::write_big_banner(banner);

//   const double dn=25;	// meters
   const double dn=75;	// meters
   const double delta_n=dn/fabs(n_hat.get(2));	// meters
   double delta_n_pos=delta_n+dn;
   double delta_n_neg=delta_n+dn;
   cout << "delta_n_pos = " << delta_n << endl;
   cout << "delta_n_neg = " << delta_n_neg << endl;

   vector<double>* X_cropped_ptr=new vector<double>;
   vector<double>* Y_cropped_ptr=new vector<double>;
   vector<double>* Z_cropped_ptr=new vector<double>;
   X_cropped_ptr->reserve(n_points);
   Y_cropped_ptr->reserve(n_points);
   Z_cropped_ptr->reserve(n_points);

   for (int i=0; i<n_points; i++)
   {
      if (pixel_number_ptr->at(i) < 0) continue;

      double curr_n=n_hat.dot(threevector(
         Xraw_ptr->at(i)-origin.get(0),Yraw_ptr->at(i)-origin.get(1),
         Zraw_ptr->at(i)-origin.get(2)));
      if (curr_n < n_peak-delta_n_neg || curr_n > n_peak+delta_n_pos) continue;

      X_cropped_ptr->push_back(Xraw_ptr->at(i));
      Y_cropped_ptr->push_back(Yraw_ptr->at(i));
      Z_cropped_ptr->push_back(Zraw_ptr->at(i));
   }
   cout << endl;
   int n_cropped_points=Z_cropped_ptr->size();
   cout << "Number of cropped points = " << n_cropped_points << endl;

   delete Xraw_ptr;
   delete Yraw_ptr;
   delete Zraw_ptr;
   delete pixel_number_ptr;

/*
   banner="Generating cropped TDP file";
   outputfunc::write_big_banner(banner);
//   string cropped_tdp_filename=dirname+prefix+"_cropped.tdp";
   string cropped_tdp_filename="n_cropped_points.tdp";
   cout << "Cropped tdp filename = " << cropped_tdp_filename << endl;
   tdpfunc::write_xyz_data(
      "",cropped_tdp_filename,*X_cropped_ptr,*Y_cropped_ptr,*Z_cropped_ptr);
   unix_cmd="lodtree "+cropped_tdp_filename;
   sysfunc::unix_command(unix_cmd);
*/

// Calculate symmetry direction vectors a_hat and b_hat which are
// orthogonal to n_hat:

   plane nslice(n_hat,origin);
   nslice.construct_2D_coord_system();
   threevector a_hat=nslice.get_ahat();
   threevector b_hat=nslice.get_bhat();

   const double slice_thickness=5;	// meters
   vector<twovector>* rhos_ptr=new vector<twovector>;
   rhos_ptr->reserve(n_cropped_points);
   for (int i=0; i<n_cropped_points; i++)
   {
      threevector curr_r(
         X_cropped_ptr->at(i)-origin.get(0),
         Y_cropped_ptr->at(i)-origin.get(1),
         Z_cropped_ptr->at(i)-origin.get(2));
      double curr_n=curr_r.dot(n_hat);
      if (curr_n < n_peak-0.5*slice_thickness || 
	  curr_n > n_peak+0.5*slice_thickness) continue;

      double curr_a=curr_r.dot(a_hat);
      double curr_b=curr_r.dot(b_hat);
      rhos_ptr->push_back(twovector(curr_a,curr_b));
   }
   cout << endl;
//   cout << "rhos_ptr->size() = " << rhos_ptr->size() << endl;

   double Imin,Imax;
   twovector Imin_hat,Imax_hat,twoOrigin(0,0);
   mathfunc::moment_of_inertia_2D(
      twoOrigin,Imin,Imax,Imin_hat,Imax_hat,*rhos_ptr);
   delete rhos_ptr;

   threevector A_hat=Imin_hat.get(0)*a_hat+Imin_hat.get(1)*b_hat;
   threevector B_hat=Imax_hat.get(0)*a_hat+Imax_hat.get(1)*b_hat;
   
   cout << "A_hat = " << A_hat 
        << " B_hat = " << B_hat 
        << "n_hat = " << n_hat << endl;
   cout << "A_hat . B_hat = " << A_hat.dot(B_hat) << endl;
   cout << "A_hat . n_hat = " << A_hat.dot(n_hat) << endl;
   cout << "B_hat . n_hat = " << B_hat.dot(n_hat) << endl;

   double min_A=POSITIVEINFINITY;
   double max_A=NEGATIVEINFINITY;
   double min_B=POSITIVEINFINITY;
   double max_B=NEGATIVEINFINITY;
   double min_N=POSITIVEINFINITY;
   double max_N=NEGATIVEINFINITY;
   for (int i=0; i<n_cropped_points; i++)
   {
//      if (i%1000000==0) cout << double(i)/n_cropped_points << " " << endl;

      threevector curr_r(
         X_cropped_ptr->at(i)-origin.get(0),
         Y_cropped_ptr->at(i)-origin.get(1),
         Z_cropped_ptr->at(i)-origin.get(2));
      double curr_n=curr_r.dot(n_hat);
      double curr_A=curr_r.dot(A_hat);
      double curr_B=curr_r.dot(B_hat);

      min_A=basic_math::min(min_A,curr_A);
      max_A=basic_math::max(max_A,curr_A);
      min_B=basic_math::min(min_B,curr_B);
      max_B=basic_math::max(max_B,curr_B);
      min_N=basic_math::min(min_N,curr_n);
      max_N=basic_math::max(max_N,curr_n);
   }

   cout << "min_A = " << min_A << " max_A = " << max_A << endl;
   cout << "min_B = " << min_B << " max_B = " << max_B << endl;
   cout << "min_N = " << min_N << " max_N = " << max_N << endl;

// Construct min_N slice corners in XYZ coordinates:

   vector<threevector> min_Nslice_corners;
   min_Nslice_corners.push_back(min_A*A_hat + min_B*B_hat + min_N*n_hat);
   min_Nslice_corners.push_back(max_A*A_hat + min_B*B_hat + min_N*n_hat);
   min_Nslice_corners.push_back(min_A*A_hat + max_B*B_hat + min_N*n_hat);
   min_Nslice_corners.push_back(max_A*A_hat + max_B*B_hat + min_N*n_hat);

   double min_Nslice_z=NEGATIVEINFINITY;
   for (int c=0; c<4; c++)
   {
      cout << "c = " << c
           << " min_Nslice_corner = " << min_Nslice_corners[c] << endl;
      min_Nslice_z=basic_math::max(min_Nslice_z,min_Nslice_corners[c].get(2));
   }

   vector<threevector> max_Nslice_corners;
   max_Nslice_corners.push_back(min_A*A_hat + min_B*B_hat + max_N*n_hat);
   max_Nslice_corners.push_back(max_A*A_hat + min_B*B_hat + max_N*n_hat);
   max_Nslice_corners.push_back(min_A*A_hat + max_B*B_hat + max_N*n_hat);
   max_Nslice_corners.push_back(max_A*A_hat + max_B*B_hat + max_N*n_hat);

   double max_Nslice_z=POSITIVEINFINITY;
   for (int c=0; c<4; c++)
   {
      cout << "c = " << c
           << " max_Nslice_corner = " << max_Nslice_corners[c] << endl;
      max_Nslice_z=basic_math::min(max_Nslice_z,max_Nslice_corners[c].get(2));
   }

   cout << "min_Nslice_z = " << min_Nslice_z << endl;
   cout << "max_Nslice_z = " << max_Nslice_z << endl;

// Eliminate more noise by cropping points which do not lie
// within the Z interval [min_Nslice_z, max_Nslice_z]:

   vector<double>* X_zplane_cropped_ptr=new vector<double>;
   vector<double>* Y_zplane_cropped_ptr=new vector<double>;
   vector<double>* Z_zplane_cropped_ptr=new vector<double>;
   X_zplane_cropped_ptr->reserve(n_cropped_points);
   Y_zplane_cropped_ptr->reserve(n_cropped_points);
   Z_zplane_cropped_ptr->reserve(n_cropped_points);

   for (int i=0; i<n_cropped_points; i++)
   {
//      if (i%1000000==0) cout << double(i)/n_cropped_points << " " << endl;

      double curr_X=X_cropped_ptr->at(i);
      double curr_Y=Y_cropped_ptr->at(i);
      double curr_Z=Z_cropped_ptr->at(i);
      
      double rel_Z=curr_Z-origin.get(2);
      if (rel_Z < min_Nslice_z || rel_Z > max_Nslice_z) continue;

      X_zplane_cropped_ptr->push_back(curr_X);
      Y_zplane_cropped_ptr->push_back(curr_Y);
      Z_zplane_cropped_ptr->push_back(curr_Z);
   }
   cout << endl;

   delete X_cropped_ptr;
   delete Y_cropped_ptr;
   delete Z_cropped_ptr;

   int n_zplane_cropped_points=Z_zplane_cropped_ptr->size();
   cout << "n_zplane_cropped_points = " << n_zplane_cropped_points << endl;

// Write out Z-plane cropped points to TDP file:

/*
   banner="Generating Z-plane cropped points TDP file";
   outputfunc::write_big_banner(banner);
   string zplane_cropped_tdp_filename="zplane_cropped_points.tdp";
   tdpfunc::write_xyz_data(
      "",zplane_cropped_tdp_filename,*X_zplane_cropped_ptr,
      *Y_zplane_cropped_ptr,*Z_zplane_cropped_ptr);
   unix_cmd="lodtree "+zplane_cropped_tdp_filename;
   sysfunc::unix_command(unix_cmd);
*/

// Project all Z-plane cropped points onto Z_hat:

   banner="Projecting Z-plane cropped points onto z_hat:";
   outputfunc::write_big_banner(banner);
   
   vector<double> Z_cropped;
   Z_cropped.reserve(6000000);
   if (n_zplane_cropped_points < 5000000) 
   {
      istep=1;
   }
   else
   {
      istep=n_zplane_cropped_points/5000000;
   }
   for (int i=0; i<n_zplane_cropped_points; i += istep)
   {
      double curr_Z=Z_zplane_cropped_ptr->at(i);
      Z_cropped.push_back(curr_Z);
   }

   n_output_bins=1000;
   prob_distribution prob_cropped_Z(Z_cropped,n_output_bins);
//   prob_cropped_Z.writeprobdists(false);

   string prob_zdist_jpg_filename=dirname+"prob_z_density.jpg";
   unix_cmd="mv  "+prob_dist_jpg_filename+" "+prob_zdist_jpg_filename;
//   sysfunc::unix_command(unix_cmd);
   
   int n_Zpeak_bin=-1;
   peak_density=prob_cropped_Z.peak_density_value(n_Zpeak_bin);
   double Z_peak=prob_cropped_Z.get_x(n_Zpeak_bin);
   cout << "Z_peak = " << Z_peak 
        << " peak_density = " << peak_density 
        << " n_Zpeak_bin = " << n_Zpeak_bin 
        << endl;

// Search for characteristic prob densities for noise above and below ground:

   double Z_first=prob_cropped_Z.get_x(0);
   double Z_last=prob_cropped_Z.get_x(n_output_bins-1);
   
   double Zlo_start=Z_first+0.4*(Z_peak-Z_first);
   double Zlo_stop=Z_first+0.6*(Z_peak-Z_first);
   int n_lo_start=prob_cropped_Z.get_bin_number(Zlo_start);
   int n_lo_stop=prob_cropped_Z.get_bin_number(Zlo_stop);
   
   double Zhi_start=Z_peak+0.4*(Z_last-Z_peak);
   double Zhi_stop=Z_peak+0.6*(Z_last-Z_peak);
   int n_hi_start=prob_cropped_Z.get_bin_number(Zhi_start);
   int n_hi_stop=prob_cropped_Z.get_bin_number(Zhi_stop);

   cout << "Z_first = " << Z_first << " Z_last = " << Z_last << endl;
   cout << "Zlo_start = " << Zlo_start << " Zlo_stop = " << Zlo_stop << endl;
   cout << "Zhi_start = " << Zhi_start << " Zhi_stop = " << Zhi_stop << endl;

   vector<double> pnoise_below_ground,pnoise_above_ground;
   for (int i=n_lo_start; i<n_lo_stop; i++)
   {
      pnoise_below_ground.push_back(prob_cropped_Z.get_p(i));
   }
   for (int i=n_hi_start; i<n_hi_stop; i++)
   {
      pnoise_above_ground.push_back(prob_cropped_Z.get_p(i));
   }

   prob_distribution prob_noise_below_ground(pnoise_below_ground,100);
   double pnoise_below_ground_median=prob_noise_below_ground.median();
   double pnoise_below_ground_sigma=prob_noise_below_ground.quartile_width();

   prob_distribution prob_noise_above_ground(pnoise_above_ground,100);
   double pnoise_above_ground_median=prob_noise_above_ground.median();
   double pnoise_above_ground_sigma=prob_noise_above_ground.quartile_width();

   cout << "Pnoise_below_ground = " << pnoise_below_ground_median << " +/- "
        << pnoise_below_ground_sigma << endl;
   cout << "Pnoise_above_ground = " << pnoise_above_ground_median << " +/- "
        << pnoise_above_ground_sigma << endl;

   double min_Z = 0;
   for (int i=n_lo_stop; i<n_Zpeak_bin; i++)
   {
      double curr_p=prob_cropped_Z.get_p(i);
      if (curr_p > pnoise_below_ground_median+7.5*pnoise_below_ground_sigma)
      {
         min_Z=prob_cropped_Z.get_x(i);
         break;
      }
   }
   cout << "min_Z = " << min_Z << endl;   

// Decrease Zmin in order to set conservative lower bound on ground
// level:

   min_Z -= 5;	// meters
   cout << "More conservative min_Z = " << min_Z << endl;   

   double max_Z = 0;
   for (int i=n_hi_start; i>n_Zpeak_bin; i--)
   {
      double curr_p=prob_cropped_Z.get_p(i);
      if (curr_p > pnoise_above_ground_median+10*pnoise_above_ground_sigma)
      {
         max_Z=prob_cropped_Z.get_x(i);
         break;
      }
   }
   cout << "max_Z = " << max_Z << endl;
//    outputfunc::enter_continue_char();

// Crop away points which do not lie within interval signal Z interval
// [Zmin,Zmax] :

   double Xmin=1E15;
   double Xmax=-1E15;
   double Ymin=1E15;
   double Ymax=-1E15;
   double Zmin=1E15;
   double Zmax=-1E15;
   vector<double>* X_refined_cropped_ptr=new vector<double>;
   vector<double>* Y_refined_cropped_ptr=new vector<double>;
   vector<double>* Z_refined_cropped_ptr=new vector<double>;

   X_refined_cropped_ptr->reserve(n_zplane_cropped_points);
   Y_refined_cropped_ptr->reserve(n_zplane_cropped_points);
   Z_refined_cropped_ptr->reserve(n_zplane_cropped_points);

   double Xnoise_min=1E15;
   double Xnoise_max=-1E15;
   double Ynoise_min=1E15;
   double Ynoise_max=-1E15;
   double Znoise_min=1E15;
   double Znoise_max=-1E15;

   vector<double>* X_noise_ptr=new vector<double>;
   vector<double>* Y_noise_ptr=new vector<double>;
   vector<double>* Z_noise_ptr=new vector<double>;

   X_noise_ptr->reserve(n_zplane_cropped_points);
   Y_noise_ptr->reserve(n_zplane_cropped_points);
   Z_noise_ptr->reserve(n_zplane_cropped_points);

   for (int i=0; i<n_zplane_cropped_points; i++)
   {
      double curr_X=X_zplane_cropped_ptr->at(i);
      double curr_Y=Y_zplane_cropped_ptr->at(i);
      double curr_Z=Z_zplane_cropped_ptr->at(i);
      
      if (curr_Z < min_Z || curr_Z > max_Z)
      {
         X_noise_ptr->push_back(curr_X);
         Y_noise_ptr->push_back(curr_Y);
         Z_noise_ptr->push_back(curr_Z);

         Xnoise_min=basic_math::min(Xnoise_min,curr_X);
         Xnoise_max=basic_math::max(Xnoise_max,curr_X);
         Ynoise_min=basic_math::min(Ynoise_min,curr_Y);
         Ynoise_max=basic_math::max(Ynoise_max,curr_Y);
         Znoise_min=basic_math::min(Znoise_min,curr_Z);
         Znoise_max=basic_math::max(Znoise_max,curr_Z);
      }
      else
      {
         X_refined_cropped_ptr->push_back(curr_X);
         Y_refined_cropped_ptr->push_back(curr_Y);
         Z_refined_cropped_ptr->push_back(curr_Z);
      
         Xmin=basic_math::min(Xmin,curr_X);
         Xmax=basic_math::max(Xmax,curr_X);
         Ymin=basic_math::min(Ymin,curr_Y);
         Ymax=basic_math::max(Ymax,curr_Y);
         Zmin=basic_math::min(Zmin,curr_Z);
         Zmax=basic_math::max(Zmax,curr_Z);
      }
   }
   cout << endl;

   delete X_zplane_cropped_ptr;
   delete Y_zplane_cropped_ptr;
   delete Z_zplane_cropped_ptr;

   int n_refined_cropped_points=Z_refined_cropped_ptr->size();
   cout << "n_refined_cropped_points = " << n_refined_cropped_points
        << endl;
   int n_noise_points=Z_noise_ptr->size();
   cout << "n_noise_points = " << n_noise_points << endl;

   cout << Xmin << " < X < " << Xmax << endl;
   cout << Ymin << " < Y < " << Ymax << endl;
   cout << Zmin << " < Z < " << Zmax << endl;
   cout << "Xmax-Xmin = " << Xmax-Xmin << endl;
   cout << "Ymax-Ymin = " << Ymax-Ymin << endl;
   cout << "Zmax-Zmin = " << Zmax-Zmin << endl;

// Write out noise points to TDP file:

/*
   banner="Generating noise points TDP file";
   outputfunc::write_big_banner(banner);
   string noise_tdp_filename="noise_points.tdp";
   tdpfunc::write_xyz_data(
      "",noise_tdp_filename,*X_noise_ptr,*Y_noise_ptr,*Z_noise_ptr);
   unix_cmd="lodtree "+noise_tdp_filename;
   sysfunc::unix_command(unix_cmd);
*/

// Write out refined cropped points to TDP file:

/*
   banner="Generating refined cropped points TDP file";
   outputfunc::write_big_banner(banner);
   string refined_cropped_tdp_filename="refined_cropped_points.tdp";
   tdpfunc::write_xyz_data(
      "",refined_cropped_tdp_filename,*X_refined_cropped_ptr,
      *Y_refined_cropped_ptr,*Z_refined_cropped_ptr);
   unix_cmd="lodtree "+refined_cropped_tdp_filename;
   sysfunc::unix_command(unix_cmd);
*/

// Calculate illumination pattern within a constant z-plane:

   banner="Calculating illumination pattern in constant z-plane";
   outputfunc::write_big_banner(banner);

   double Z_avg=0.5*(Zmin+Zmax);

   int mdim=(Xmax-Xmin)/voxel_binsize;
   int ndim=(Ymax-Ymin)/voxel_binsize;

   twoDarray* illumpattern_twoDarray_ptr=new twoDarray(mdim,ndim);
   illumpattern_twoDarray_ptr->init_coord_system(Xmin,Xmax,Ymin,Ymax);
   illumpattern_twoDarray_ptr->clear_values();

// As of 11/19/2011, we have empirically found that the frustum
// corners encoded via negative pixel numbers in the input BFP file
// are neither clockwise nor counter-clockwise ordered:

   vector<int> corner_index;
   for (int k=0; k<4; k++)
   {
      if (k==0) 
      {
         corner_index.push_back(0);
      }
      else if (k==1)
      {
         corner_index.push_back(2);
      }
      else if (k==2)
      {
         corner_index.push_back(3);
      }
      else if (k==3)
      {
         corner_index.push_back(1);
      }
   }

   vector<double> frame_number,delta_s;
   vector<threevector> vertices;
   vector<polygon*> polygon_footprint_ptrs;
   int n_curr_patterns=sensor_posn.size();
   for (int j=0; j<n_curr_patterns; j++)
   {
      threevector curr_sensor_posn = sensor_posn[j];
      double Z_sensor=curr_sensor_posn.get(2);

      threevector curr_vertex_center(Zero_vector);
      vertices.clear();
      for (int k=0; k<4; k++)
      {
         double lambda=(Z_avg-Z_sensor)/frustum_ray[corner_index[k]].at(j).
            get(2);
         vertices.push_back(
            curr_sensor_posn+lambda*frustum_ray[corner_index[k]].at(j));
//         cout << "corner_index = " << corner_index[k] 
//              << " vertex.x = " << vertices.back().get(0)
//              << " vertex.y = " << vertices.back().get(1)
//              << " vertex.z = " << vertices.back().get(2) << endl;
         curr_vertex_center += vertices.back();
      } // loop over index c labeling frustum corner

      delta_s.push_back(curr_vertex_center.dot(s_hat));
      frame_number.push_back(j);
//      cout << "frame number = " << frame_number.back() 
//           << " delta_s = " << delta_s.back() << endl;

      polygon_footprint_ptrs.push_back(new polygon(vertices));
   } // loop over index j labeling sensor position

// Subtract mean from delta_s STL vector:

   double delta_s_mean=mathfunc::mean(delta_s);
   for (unsigned int j=0; j<delta_s.size(); j++)
   {
      delta_s[j]=delta_s[j]-delta_s_mean;
//      cout << "j = " << j << " delta_s = " << delta_s[j] << endl;
   }

// Perform search over sinusoidal parameters for best fit to delta_s
// as a function of frame number:
   
   prob_distribution prob_s(delta_s,1000);
   double s_01=prob_s.find_x_corresponding_to_pcum(0.01);
   double s_99=prob_s.find_x_corresponding_to_pcum(0.99);
   cout << "s_01 = " << s_01 << " s_99 = " << s_99 << endl;

   double Astart=0.5*(s_99-s_01);
   double Bstart=0.5*(s_01+s_99);
   double Period_start=510;

   param_range A(Astart-0.1,Astart+0.1,5);
   param_range B(Bstart-0.1,Bstart+0.1,5);
   param_range Period(Period_start-20,Period_start+20,5);
   param_range phi(0,2*PI,9);
   
   double min_error_sum=POSITIVEINFINITY;
   int n_iters=10;
   for (int iter=0; iter<n_iters; iter++)
   {
      cout << "iter = " << iter << " of " << n_iters << endl;

// Begin while loop over sinusoid parameters

      while (A.prepare_next_value())
      {
         while (B.prepare_next_value())
         {
            while (Period.prepare_next_value())
            {
               double omega=2*PI/Period.get_value();
               while (phi.prepare_next_value())
               {

                  double error_sum=0;
                  for (unsigned int j=1; j<frame_number.size(); j++)
                  {
                     double curr_ds=A.get_value()*sin(
                        omega*frame_number[j]+phi.get_value())+B.get_value();
                     error_sum += fabs(delta_s[j]-curr_ds);
                  } // loop over index j labeling frames

                  if (error_sum < min_error_sum)
                  {
                     min_error_sum=error_sum;
                     A.set_best_value();
                     B.set_best_value();
                     Period.set_best_value();
                     phi.set_best_value();
//                     cout << "min_error_sum = " << min_error_sum << endl;
                  }
               } // phi while loop
            } // Period while loop
         } // B while loop
      } // A while loop
      
      double frac=0.55;
      A.shrink_search_interval(A.get_best_value(),frac);
      B.shrink_search_interval(B.get_best_value(),frac);
      Period.shrink_search_interval(Period.get_best_value(),frac);
      phi.shrink_search_interval(phi.get_best_value(),frac);
   } // loop over iter index

   cout << "min_error_sum = " << min_error_sum << endl;
   cout << "A = " << A.get_best_value() << endl;
   cout << "B = " << B.get_best_value() << endl;
   cout << "Period = " << Period.get_best_value() << endl;
   cout << "phi = " << phi.get_best_value()*180/PI << endl;

// Mark frusta corresponding to extremal and mid-cycle cross-track
// frustum locations with non-nominal frustum_weight values:

   vector<double> ds,frustum_weight;
   double omega=2*PI/Period.get_best_value();
   for (unsigned int j=0; j<frame_number.size(); j++)
   {
      double curr_phase=omega*frame_number[j]+phi.get_best_value();
      curr_phase=basic_math::phase_to_canonical_interval(
         curr_phase,0,2*PI);
      double curr_weight=1;
      if (nearly_equal(curr_phase,PI/2,2*PI/180) ||
          nearly_equal(curr_phase,3*PI/2,2*PI/180) )	// osc extrema
      {
         curr_weight=99999;
      }
      if (nearly_equal(curr_phase,0,2*PI/180) ||
          nearly_equal(curr_phase,PI,2*PI/180) )	// osc mid-cycle
      {
         curr_weight=-1;
      }
      frustum_weight.push_back(curr_weight);
      ds.push_back(A.get_best_value()*sin(curr_phase)+B.get_best_value());
   }

/*
// Write out delta_s and ds to metafile plot:
   
   metafile M;
   string meta_filename="delta_s";
   string title="delta_s vs frame number";
   string x_label="Frame number";
   string y_label="Delta s";
   double x_min=0;
   double x_max=frame_number.back();
   double y_min=1.1*mathfunc::minimal_value(delta_s);
   double y_max=1.1*mathfunc::maximal_value(delta_s);
   M.set_parameters(
      meta_filename,title,x_label,y_label,x_min,x_max,y_min,y_max);
   M.openmetafile();
   M.write_header();
   M.write_curve(frame_number,delta_s);
   M.write_curve(frame_number,ds,colorfunc::blue);
   M.closemetafile();
   unix_cmd="meta_to_jpeg "+meta_filename;
   sysfunc::unix_command(unix_cmd);
*/

// Export frusta metadata to output text file:

   string frustum_filename=dirname+prefix+".frusta";
   ofstream frustum_stream;
   filefunc::openfile(frustum_filename,frustum_stream);
   frustum_stream.precision(10);

   for (unsigned int j=0; j<sensor_posn.size(); j++)
   {
      threevector curr_sensor_posn=sensor_posn.at(j);
      frustum_stream << j << "  "
                     << frustum_weight[j] << "  "
                     << curr_sensor_posn.get(0) << "  "
                     << curr_sensor_posn.get(1) << "  "
                     << curr_sensor_posn.get(2) << endl;
      for (int c=0; c<4; c++)
      {
         frustum_stream << c << "  "
                        <<  frustum_ray[c].at(j).get(0) << "  "
                        <<  frustum_ray[c].at(j).get(1) << "  "
                        <<  frustum_ray[c].at(j).get(2) 
                        << endl;
//         cout << "j = " << j << " c = " << c
//              << " rx = " << frustum_ray[c].at(j).get(0)
//              << " ry = " << frustum_ray[c].at(j).get(1)
//              << " rx = " << frustum_ray[c].at(j).get(2) << endl;
      } // loop over index c labeling frustum rays
   } // loop over index j labeling sensor IFOVs

   filefunc::closefile(frustum_filename,frustum_stream);

// Generate weighted illumination pattern:

   bool accumulate_flag=true;
   const double uniform_frustum_weight=1;
   for (unsigned int j=0; j<frame_number.size(); j++)
   {
      polygon* footprint_ptr=polygon_footprint_ptrs.at(j);
      drawfunc::color_convex_quadrilateral_interior(
//         *footprint_ptr,frustum_weight[j],
         *footprint_ptr,uniform_frustum_weight,
         illumpattern_twoDarray_ptr,accumulate_flag);
      delete footprint_ptr;
   } // loop over index j labeling sensor position

// Write out illumation pattern to JPG file:

   int max_counts=0;
   for (int px=0; px<mdim; px++)
   {
      for (int py=0; py<ndim; py++)
      {
         max_counts=basic_math::max(
            max_counts,int(illumpattern_twoDarray_ptr->get(px,py)));
      }
   }
   cout << " max_counts = " << max_counts << endl;

   twoDarray* tmp_pattern_twoDarray_ptr=new twoDarray(
      illumpattern_twoDarray_ptr);
   for (int px=0; px<mdim; px++)
   {
      for (int py=0; py<ndim; py++)
      {
         tmp_pattern_twoDarray_ptr->put(px,py,
         illumpattern_twoDarray_ptr->get(px,py)/max_counts);
      }
   }

   int n_channels=3;
   texture_rectangle* texture_rectangle_ptr=new texture_rectangle(
      mdim,ndim,1,n_channels,NULL);
   texture_rectangle_ptr->initialize_twoDarray_image(
      tmp_pattern_twoDarray_ptr);
   texture_rectangle_ptr->fill_twoDarray_image(
      tmp_pattern_twoDarray_ptr,n_channels);
   delete tmp_pattern_twoDarray_ptr;

   string output_filename=dirname+prefix+"illumination_pattern.jpg";
   texture_rectangle_ptr->write_curr_frame(output_filename);
   delete texture_rectangle_ptr;

/* 
// Voxelize pure noise XYZ points:

   banner="Voxelizing pure noise points";
   outputfunc::write_big_banner(banner);

   cout << "Xnoise_min = " << Xnoise_min << " Xnoise_max = " << Xnoise_max
        << endl;
   cout << "Ynoise_min = " << Ynoise_min << " Ynoise_max = " << Ynoise_max
        << endl;
   cout << "Znoise_min = " << Znoise_min << " Znoise_max = " << Znoise_max
        << endl;

   VolumetricCoincidenceProcessor* noise_vcp_ptr=
      new VolumetricCoincidenceProcessor(n_noise_points);
   noise_vcp_ptr->initialize_coord_system(
      threevector(Xnoise_min,Ynoise_min,Znoise_min),
      threevector(Xnoise_max,Ynoise_max,Znoise_max),voxel_binsize);

   banner="Accumulating pure noise points within *noise_vcp_ptr";
   outputfunc::write_big_banner(banner);

   for (int i=0; i<n_noise_points; i++)
   {
      outputfunc::update_progress_fraction(i,1000000,n_noise_points);
      noise_vcp_ptr->accumulate_points(
         X_noise_ptr->at(i),Y_noise_ptr->at(i),Z_noise_ptr->at(i));
   }
   cout << endl;
   cout << "After accumulating points, noise_vcp_ptr->size() = "
        << noise_vcp_ptr->size() << endl;
   
   delete X_noise_ptr;
   delete Y_noise_ptr;
   delete Z_noise_ptr;
 
   vector<int> noise_counts=noise_vcp_ptr->extract_pure_noise_counts(
      A_hat,B_hat,origin,min_A,max_A,min_B,max_B,min_Z,max_Z);

// Write out voxelized noise counts:

   vector<double>* Xbinned_noise_ptr=new vector<double>;
   vector<double>* Ybinned_noise_ptr=new vector<double>;
   vector<double>* Zbinned_noise_ptr=new vector<double>;
   vector<double>* Pbinned_noise_ptr=new vector<double>;

   Xbinned_noise_ptr->reserve(noise_vcp_ptr->size());
   Ybinned_noise_ptr->reserve(noise_vcp_ptr->size());
   Zbinned_noise_ptr->reserve(noise_vcp_ptr->size());
   Pbinned_noise_ptr->reserve(noise_vcp_ptr->size());
   
   min_prob_threshold=0;
   perturb_voxels_flag=true;
//    perturb_voxels_flag=false;

    noise_vcp_ptr->retrieve_XYZP_points(
       Xbinned_noise_ptr,Ybinned_noise_ptr,Zbinned_noise_ptr,
       Pbinned_noise_ptr,min_prob_threshold,perturb_voxels_flag);
    delete noise_vcp_ptr;

    string binned_noise_tdp_filename=dirname+prefix+"_binned_noise.tdp";
    tdpfunc::write_xyzp_data(
       binned_noise_tdp_filename,Xbinned_noise_ptr,Ybinned_noise_ptr,
       Zbinned_noise_ptr,Pbinned_noise_ptr);

   delete Xbinned_noise_ptr;
   delete Ybinned_noise_ptr;
   delete Zbinned_noise_ptr;
   delete Pbinned_noise_ptr;

   unix_cmd="lodtree "+binned_noise_tdp_filename;
   sysfunc::unix_command(unix_cmd);

// Divide pure voxel noise counts by total number of IFOVs and
// voxel volume to derive normalized spacetime density-rate:

   vector<double> pure_noise_density_rate;
   for (int i=0; i<noise_counts.size(); i++)
   {
      pure_noise_density_rate.push_back(
         noise_counts[i]/(n_frames*voxel_volume) );
   }

   n_output_bins=100;
   prob_distribution prob_noise(pure_noise_density_rate,n_output_bins);
//   prob_noise.set_freq_histogram(true);
   prob_noise.set_color(colorfunc::red);
   prob_noise.set_xmin(-0.01);
   prob_noise.set_xlabel("Pure noise spacetime density [counts/IFOV * m**3] ");
   prob_noise.writeprobdists(false);

   string prob_noise_hist_meta_filename=dirname+prefix+
      "_prob_noise.meta";
   unix_cmd="mv "+prob_dist_meta_filename+" "+prob_noise_hist_meta_filename;
   sysfunc::unix_command(unix_cmd);

   string prob_noise_hist_jpg_filename=dirname+prefix+
      "_prob_noise.jpg";
   unix_cmd="mv "+prob_dist_jpg_filename+" "+prob_noise_hist_jpg_filename;
   sysfunc::unix_command(unix_cmd);

// Append pure noise density-rate mu and sigma values to text file:

   double pure_noise_density_rate_mu=prob_noise.mean();
   double pure_noise_density_rate_sigma=prob_noise.std_dev();

   string noise_density_rate_text_filename=dirname+"noise_density_rate.txt";
   ofstream outstream;
   filefunc::appendfile(noise_density_rate_text_filename,outstream);
   outstream << prefix << "  "  
             << pure_noise_density_rate_mu << "  "
             << pure_noise_density_rate_sigma << endl;
   filefunc::closefile(noise_density_rate_text_filename,outstream);

// Store probabilities for voxelized pure noise counts within STL vector:

   vector<double> pure_noise_probs;
   int ncounts_max=basic_math::mytruncate(prob_noise.get_maximum_x());
   cout << "ncounts_max = " << ncounts_max << endl;
   for (int n=0; n<=ncounts_max; n++)
   {
      double n_max=n+0.5;
      double n_min=n-0.5;
      n_min=basic_math::max(0.0,n_min);
      n_max=basic_math::min(double(ncounts_max),n_max);
      int bin_lo=prob_noise.get_bin_number(n_min);
      int bin_hi=prob_noise.get_bin_number(n_max);
      double prob_lo=0;
      if (n > 0) prob_lo=prob_noise.get_pcum(bin_lo);
      double prob_hi=prob_noise.get_pcum(bin_hi);
      double curr_p=prob_hi-prob_lo;
      pure_noise_probs.push_back(curr_p);
      cout << "Pure noise counts n = " << n 
           << " prob = " << pure_noise_probs.back() << endl;
   } // loop over n labeling pure noise counts
*/

// Voxelize refined cropped XYZ points:

   banner="Voxelizing refined cropped points";
   outputfunc::write_big_banner(banner);

   VolumetricCoincidenceProcessor* vcp_ptr=
      new VolumetricCoincidenceProcessor(n_refined_cropped_points);
   vcp_ptr->initialize_coord_system(
      threevector(Xmin,Ymin,Zmin),threevector(Xmax,Ymax,Zmax),voxel_binsize);

   banner="Accumulating points within *vcp_ptr";
   outputfunc::write_big_banner(banner);
   cout << "n_refined_cropped_points = " << n_refined_cropped_points
        << endl;

   for (int i=0; i<n_refined_cropped_points; i++)
   {
      outputfunc::update_progress_fraction(i,1000000,n_refined_cropped_points);
      vcp_ptr->accumulate_points(
         X_refined_cropped_ptr->at(i),
         Y_refined_cropped_ptr->at(i),
         Z_refined_cropped_ptr->at(i));
   }
   cout << endl;
   cout << "After accumulating points, vcp_ptr->size() = "
        << vcp_ptr->size() << endl;
   cout << "Z_refined_cropped_ptr->size() = "
        << Z_refined_cropped_ptr->size() << endl;
   
   delete X_refined_cropped_ptr;
   delete Y_refined_cropped_ptr;
   delete Z_refined_cropped_ptr;
   
//   vcp_ptr->assign_pure_noise_probabilities(pure_noise_probs);

// Write out TDP file containing voxelized points:

/*
   vector<double>* Xbinned_ptr=new vector<double>;
   vector<double>* Ybinned_ptr=new vector<double>;
   vector<double>* Zbinned_ptr=new vector<double>;
   vector<double>* Pbinned_ptr=new vector<double>;

   Xbinned_ptr->reserve(vcp_ptr->size());
   Ybinned_ptr->reserve(vcp_ptr->size());
   Zbinned_ptr->reserve(vcp_ptr->size());
   Pbinned_ptr->reserve(vcp_ptr->size());
   
   min_prob_threshold=0;
    perturb_voxels_flag=true;
//    perturb_voxels_flag=false;

   vcp_ptr->retrieve_XYZP_points(
      Xbinned_ptr,Ybinned_ptr,Zbinned_ptr,Pbinned_ptr,min_prob_threshold,
      perturb_voxels_flag);

   string binned_tdp_filename=dirname+prefix+"_binned.tdp";
   tdpfunc::write_xyzp_data(
      binned_tdp_filename,Xbinned_ptr,Ybinned_ptr,Zbinned_ptr,Pbinned_ptr);

   delete Xbinned_ptr;
   delete Ybinned_ptr;
   delete Zbinned_ptr;
   delete Pbinned_ptr;

   unix_cmd="lodtree "+binned_tdp_filename;
   sysfunc::unix_command(unix_cmd);
*/

// Squish "comet tails" in uprange direction:

   threevector r_hat=-n_hat;

   double squish_distance=1;	// meters
//   double squish_distance=2;	// meters
   vcp_ptr->squish_range_tails(squish_distance,r_hat);

   vcp_ptr->copy_counts_onto_probs();

   vector<double>* Xsquished_ptr=new vector<double>;
   vector<double>* Ysquished_ptr=new vector<double>;
   vector<double>* Zsquished_ptr=new vector<double>;
   vector<double>* Psquished_ptr=new vector<double>;

   Xsquished_ptr->reserve(vcp_ptr->size());
   Ysquished_ptr->reserve(vcp_ptr->size());
   Zsquished_ptr->reserve(vcp_ptr->size());
   Psquished_ptr->reserve(vcp_ptr->size());

   min_prob_threshold=0;
   perturb_voxels_flag=true;
// perturb_voxels_flag=false;

   vcp_ptr->retrieve_XYZP_points(
      Xsquished_ptr,Ysquished_ptr,Zsquished_ptr,Psquished_ptr,
      min_prob_threshold,perturb_voxels_flag);

   string squished_tdp_filename=dirname+prefix+"_squished.tdp";
   cout << "Squished tdp filename = " << squished_tdp_filename << endl;
   tdpfunc::write_xyzp_data(
      squished_tdp_filename,Xsquished_ptr,Ysquished_ptr,Zsquished_ptr,
      Psquished_ptr);

   delete Xsquished_ptr;
   delete Ysquished_ptr;
   delete Zsquished_ptr;
   delete Psquished_ptr;
   
   unix_cmd="lodtree "+squished_tdp_filename;
   sysfunc::unix_command(unix_cmd);
}
