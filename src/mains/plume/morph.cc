// ==========================================================================
// Program MORPH takes in two TDP files corresponding to point clouds
// at some starting and stopping times.  It first subtracts out the
// point clouds' COM locations.  MORPH then forms cumulative
// probability distributions along (theta,phi) rays emanating from the COMs.
// Linear interpolation is performed between the starting and stopping
// cumulative distributions for time fractions ranging from 0 to 1.
// After retranslating the morphed results by the interpolated COM,
// MORPH exports osga files labeled by the time fraction.

//   cd to subdir containing TDP files to be interpolated as well as 
//   point_clouds.dat text file.  Within this subdir chant

//				PATH/morph 

// ==========================================================================
// Last updated on 12/23/11; 12/31/11; 1/3/12
// ==========================================================================

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "image/drawfuncs.h"
#include "general/filefuncs.h"
#include "math/fourvector.h"
#include "math/prob_distribution.h"
#include "general/sysfuncs.h"
#include "osg/osg3D/tdpfuncs.h"
#include "video/texture_rectangle.h"
#include "image/TwoDarray.h"
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

// Constants:

   const double voxel_binsize=0.1;	// meter
//   const double voxel_binsize=0.25;	// meter
   const double voxel_volume=voxel_binsize*voxel_binsize*voxel_binsize;

// Repeated variable declarations:

   string banner,unix_cmd="";
   double min_prob_threshold=0.0;
   bool perturb_voxels_flag=true;

// Read in text file containing point cloud TDP filenames as well as
// their relative clock times:

   string point_cloud_datafile="point_clouds.dat";
   vector<string> tdp_filenames;
   vector<double> point_cloud_times;
   filefunc::ReadInfile(point_cloud_datafile);
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);
      tdp_filenames.push_back(substrings[0]);
      point_cloud_times.push_back(stringfunc::string_to_number(substrings[1]));
   }

// Read in starting point cloud:

   vector<double>* Xstart_ptr=new vector<double>;
   vector<double>* Ystart_ptr=new vector<double>;
   vector<double>* Zstart_ptr=new vector<double>;
   vector<double>* Pstart_ptr=new vector<double>;

   threevector start_XYZ_min(
      POSITIVEINFINITY,POSITIVEINFINITY,POSITIVEINFINITY);
   threevector start_XYZ_max(
      NEGATIVEINFINITY,NEGATIVEINFINITY,NEGATIVEINFINITY);
   tdpfunc::compute_extremal_XYZ_points_in_tdpfile(
      tdp_filenames[0],start_XYZ_min,start_XYZ_max);
   tdpfunc::read_XYZP_points_from_tdpfile(
      tdp_filenames[0],*Xstart_ptr,*Ystart_ptr,*Zstart_ptr,*Pstart_ptr);

   int n_start_points=Zstart_ptr->size();
   threevector start_COM(0,0,0);
   for (int i=0; i<n_start_points; i++)
   {
      start_COM += threevector(Xstart_ptr->at(i),Ystart_ptr->at(i),
      Zstart_ptr->at(i));
//      cout << "i = " << i << " Pstart_ptr->at(i) = " << Pstart_ptr->at(i)
//           << endl;
   }
   start_COM /= n_start_points;

// Read in stopping point cloud:

   vector<double>* Xstop_ptr=new vector<double>;
   vector<double>* Ystop_ptr=new vector<double>;
   vector<double>* Zstop_ptr=new vector<double>;
   vector<double>* Pstop_ptr=new vector<double>;

   threevector stop_XYZ_min(
      POSITIVEINFINITY,POSITIVEINFINITY,POSITIVEINFINITY);
   threevector stop_XYZ_max(
      NEGATIVEINFINITY,NEGATIVEINFINITY,NEGATIVEINFINITY);
   tdpfunc::compute_extremal_XYZ_points_in_tdpfile(
      tdp_filenames[1],stop_XYZ_min,stop_XYZ_max);
   tdpfunc::read_XYZP_points_from_tdpfile(
      tdp_filenames[1],*Xstop_ptr,*Ystop_ptr,*Zstop_ptr,*Pstop_ptr);

   int n_stop_points=Zstop_ptr->size();
   threevector stop_COM(0,0,0);
   for (int i=0; i<n_stop_points; i++)
   {
      stop_COM += threevector(Xstop_ptr->at(i),Ystop_ptr->at(i),
      Zstop_ptr->at(i));
//      cout << "i = " << i << " Pstop_ptr->at(i) = " << Pstop_ptr->at(i)
//           << endl;
   }
   stop_COM /= n_stop_points;

// Instantiate and fill starting VCP:

   VolumetricCoincidenceProcessor* start_vcp_ptr=
      new VolumetricCoincidenceProcessor(n_start_points);
   start_vcp_ptr->initialize_coord_system(
      start_XYZ_min-start_COM,start_XYZ_max-start_COM,voxel_binsize);

   threevector rel_XYZ;
   for (int i=0; i<n_start_points; i++)
   {
      rel_XYZ=
         threevector(Xstart_ptr->at(i),Ystart_ptr->at(i),Zstart_ptr->at(i))
         -start_COM;
      start_vcp_ptr->accumulate_points_and_probs(
         rel_XYZ.get(0),rel_XYZ.get(1),rel_XYZ.get(2),Pstart_ptr->at(i));
   }

// Instantiate and fill stopping VCP:

   VolumetricCoincidenceProcessor* stop_vcp_ptr=
      new VolumetricCoincidenceProcessor(n_stop_points);
   stop_vcp_ptr->initialize_coord_system(
      stop_XYZ_min-stop_COM,stop_XYZ_max-stop_COM,voxel_binsize);

   for (int i=0; i<n_stop_points; i++)
   {
      rel_XYZ=
         threevector(Xstop_ptr->at(i),Ystop_ptr->at(i),Zstop_ptr->at(i))
         -stop_COM;
      stop_vcp_ptr->accumulate_points_and_probs(
         rel_XYZ.get(0),rel_XYZ.get(1),rel_XYZ.get(2),Pstop_ptr->at(i));
   }

   cout << "*start_vcp_ptr = " << *start_vcp_ptr << endl;
   cout << "*stop_vcp_ptr = " << *stop_vcp_ptr << endl;

   cout << "start_COM = " << start_COM << endl;
   cout << "stop_COM = " << stop_COM << endl;

   cout << "start_XYZ_min = " << start_XYZ_min
        << " stop_XYZ_min = " << stop_XYZ_min << endl;

   cout << "start_XYZ_max = " << start_XYZ_max
        << " stop_XYZ_max = " << stop_XYZ_max << endl;

   double start_volume=start_vcp_ptr->size()*voxel_volume;
   double stop_volume=stop_vcp_ptr->size()*voxel_volume;
   
   cout << "Starting VCP volume = " << start_volume << endl;
   cout << "Stopping VCP volume = " << stop_volume << endl;

// Instantiate and fill interpolation VCP:

   VolumetricCoincidenceProcessor* interp_vcp_ptr=
      new VolumetricCoincidenceProcessor(n_stop_points);
   interp_vcp_ptr->initialize_coord_system(
      stop_XYZ_min-stop_COM,stop_XYZ_max-stop_COM,voxel_binsize);

   vector<double>* Xinterp_ptr=new vector<double>;
   vector<double>* Yinterp_ptr=new vector<double>;
   vector<double>* Zinterp_ptr=new vector<double>;
   vector<double>* Pinterp_ptr=new vector<double>;

   int n_iters=2;
   cout << "Enter number of interpolation time steps:" << endl;
   cin >> n_iters;
   n_iters--;

   double t_start=0;
   double t_stop=1;
   double dt=(t_stop-t_start)/(n_iters+1);

   string volume_filename="interpolated_volumes.dat";
   ofstream volume_stream;
   filefunc::appendfile(volume_filename,volume_stream);
   for (int iter=0; iter<=n_iters+1; iter++)
   {
      double t_frac=t_start+iter*dt;
      double curr_time=point_cloud_times.front()+t_frac;
      double interp_volume=start_volume+t_frac*(stop_volume-start_volume);
      volume_stream << stringfunc::number_to_string(curr_time,3) 
                    << "  " << interp_volume << endl;
   }
   filefunc::closefile(volume_filename,volume_stream);   

   for (int iter=1; iter<=n_iters; iter++)
   {
      double t_frac=t_start+iter*dt;
      string banner="iter = "+stringfunc::number_to_string(iter)+
         " t_frac = "+stringfunc::number_to_string(t_frac);
      outputfunc::write_big_banner(banner);
      
      interp_vcp_ptr->delete_all_voxels();

      double phi_lo=0*PI/180;
      double phi_hi=360*PI/180;
//      double d_phi=0.5*PI/180;
      double d_phi=1*PI/180;
      int n_phi_bins=(phi_hi-phi_lo)/d_phi+1;

      double theta_lo=-90*PI/180;
      double theta_hi=90*PI/180;
//      double d_theta=0.5*PI/180;
      double d_theta=1*PI/180;
      int n_theta_bins=(theta_hi-theta_lo)/d_theta+1;
   
      int m,n,p,counts;
      double dr=voxel_binsize;
      double prob;

      cout << "n_phi_bins = " << n_phi_bins 
           << " n_theta_bins = " << n_theta_bins << endl;

      int n_prob_bins=100;
      vector<double> start_radii,stop_radii,start_p,stop_p;
      for (int i=0; i<n_phi_bins; i++)
      {
         outputfunc::update_progress_fraction(i,36,n_phi_bins);
         double phi=phi_lo+i*d_phi;
         for (int j=0; j<n_theta_bins; j++)
         {
            double theta=theta_lo+j*d_theta;

            threevector r_hat(cos(theta)*cos(phi),cos(theta)*sin(phi),
            sin(theta));

// Calculate cumulative prob distribution from start_COM out along ray
// r_hat:
         
            double r=0;
            start_radii.clear();
            start_p.clear();
            while (true)
            {
               threevector rvec=r*r_hat;

               if (!start_vcp_ptr->xyz_inside_volume(
                  rvec.get(0),rvec.get(1),rvec.get(2))) break;
            
               start_vcp_ptr->xyz_to_mnp(
                  rvec.get(0),rvec.get(1),rvec.get(2),m,n,p);

               start_vcp_ptr->mnp_to_voxel_counts_and_prob(m,n,p,counts,prob);
               if (counts > 0) 
               {
                  start_radii.push_back(r);
                  start_p.push_back(prob);
//                  cout << "start_p = " << start_p.back() << endl;
               }

               r += dr;
            } // while loop
//         cout << "start_radii.size() = " << start_radii.size() << endl;
         
            if (start_radii.size()==0) 
            {
               start_radii.push_back(0);
               start_p.push_back(0);
            }
            
            prob_distribution start_radii_prob(start_radii,n_prob_bins);

// Sort start_radii values and convert into fractions ranging from 0 to 1:

            templatefunc::Quicksort(start_radii,start_p);
            double rstart_max=start_radii.back();
            vector<double> r_frac_start;
            for (int k=0; k<start_radii.size(); k++)
            {
               double r_frac=start_radii[k]/rstart_max;
               if (r_frac < 0 || r_frac > 1)
               {
                  cout << "Error: start r_frac = " << r_frac << endl;
               }
               
               r_frac_start.push_back(r_frac);
            }

//            cout << "start_radii.size() = " << start_radii.size()
//                 << " r_frac_start.size() = " << r_frac_start.size()
//                 << " start_p.size() = " << start_p.size() << endl;

// Calculate cumulative prob distribution from stop_COM out along ray
// r_hat:
         
            r=0;
            stop_radii.clear();
            stop_p.clear();
            while (true)
            {
               threevector rvec=r*r_hat;

               if (!stop_vcp_ptr->xyz_inside_volume(
                  rvec.get(0),rvec.get(1),rvec.get(2))) break;
            
               stop_vcp_ptr->xyz_to_mnp(
                  rvec.get(0),rvec.get(1),rvec.get(2),m,n,p);
            
               stop_vcp_ptr->mnp_to_voxel_counts_and_prob(m,n,p,counts,prob);
               if (counts > 0) 
               {
                  stop_radii.push_back(r);
                  stop_p.push_back(prob);
//                  cout << "stop_p = " << stop_p.back() << endl;
               }
               
               r += dr;
            } // while loop
//         cout << "stop_radii.size() = " << stop_radii.size() << endl
//              << endl;
         
            if (stop_radii.size()==0) 
            {
               stop_radii.push_back(0);
               stop_p.push_back(0);
            }
            
            prob_distribution stop_radii_prob(stop_radii,n_prob_bins);

// Sort stop_radii values and convert into fractions ranging from 0 to 1:

            templatefunc::Quicksort(stop_radii,stop_p);
            double rstop_max=stop_radii.back();
            vector<double> r_frac_stop;
            for (int k=0; k<stop_radii.size(); k++)
            {
               double r_frac=stop_radii[k]/rstop_max;
               if (r_frac < 0 || r_frac > 1)
               {
                  cout << "Error: stop r_frac = " << r_frac << endl;
               }

               r_frac_stop.push_back(r_frac);
            }

//            cout << "stop_radii.size() = " << stop_radii.size()
//                 << " r_frac_stop.size() = " << r_frac_stop.size()
//                 << " stop_p.size() = " << stop_p.size() << endl;

            for (int k=0; k<n_prob_bins; k++)
            {
               double cumprob=double(k)/double(n_prob_bins-1);
               double r_start=start_radii_prob.find_x_corresponding_to_pcum(
                  cumprob);
               double r_stop=stop_radii_prob.find_x_corresponding_to_pcum(
                  cumprob);
               double r_interp=(1-t_frac)*r_start+(t_frac)*r_stop;
               threevector r_interp_vec=r_interp*r_hat;
//            cout << "r_interp_vec = " << r_interp_vec << endl;

// Compute interpolated value for p_start:

               double p_start=-1;
               int bin_number=mathfunc::mylocate(r_frac_start,cumprob);
               if (bin_number <= 0)
               {
                  p_start=start_p[0];
               }
               else if (bin_number >= r_frac_start.size()-1)
               {
                  p_start=start_p[r_frac_start.size()-1];
               }
               else
               {
                  double rfrac_start_lo=r_frac_start[bin_number];
                  double rfrac_start_hi=r_frac_start[bin_number+1];
                  double p_start_lo(start_p[bin_number]);
                  double p_start_hi(start_p[bin_number+1]);

                  p_start=mathfunc::linefit(
                     cumprob,rfrac_start_lo,p_start_lo,
                     rfrac_start_hi,p_start_hi);
               }

// Compute interpolated value for p_stop:

               double p_stop=-1;
               bin_number=mathfunc::mylocate(r_frac_stop,cumprob);
               if (bin_number <= 0)
               {
                  p_stop=stop_p[0];
               }
               else if (bin_number >= r_frac_stop.size()-1)
               {
                  p_stop=stop_p[r_frac_stop.size()-1];
               }
               else
               {
                  double rfrac_stop_lo=r_frac_stop[bin_number];
                  double rfrac_stop_hi=r_frac_stop[bin_number+1];
                  double p_stop_lo(stop_p[bin_number]);
                  double p_stop_hi(stop_p[bin_number+1]);

                  p_stop=mathfunc::linefit(
                     cumprob,rfrac_stop_lo,p_stop_lo,
                     rfrac_stop_hi,p_stop_hi);
               }

               double p_interp=(1-t_frac)*p_start+(t_frac)*p_stop;
            
               interp_vcp_ptr->accumulate_points_and_probs(
                  r_interp_vec.get(0),r_interp_vec.get(1),r_interp_vec.get(2),
                  p_interp);
            } // loop over index k labeling temporally interpolated voxels
         
         } // loop over index j labeling theta
//      cout << "interp_vcp_ptr->size() = "
//           << interp_vcp_ptr->size() << endl;
      
      } // loop over index i labeling phi

      cout << "After filling, interp_vcp_ptr->size() = "
           << interp_vcp_ptr->size() << endl;
//      cout << "Calculated interp volume = " 
//           << interp_vcp_ptr->size()*voxel_volume << endl;

      Xinterp_ptr->clear();
      Yinterp_ptr->clear();
      Zinterp_ptr->clear();
      Pinterp_ptr->clear();
      
      Xinterp_ptr->reserve(n_stop_points);
      Yinterp_ptr->reserve(n_stop_points);
      Zinterp_ptr->reserve(n_stop_points);
      Pinterp_ptr->reserve(n_stop_points);
   
      interp_vcp_ptr->retrieve_XYZP_points(
         Xinterp_ptr,Yinterp_ptr,Zinterp_ptr,Pinterp_ptr,
         min_prob_threshold,perturb_voxels_flag);

// Add interpolated COM to relative XYZ points:

      threevector COM_interp=(1-t_frac)*start_COM+(t_frac)*stop_COM;
      for (int i=0; i<Zinterp_ptr->size(); i++)
      {
         threevector rel_XYZ(
            Xinterp_ptr->at(i),Yinterp_ptr->at(i),Zinterp_ptr->at(i));
         threevector abs_XYZ=rel_XYZ+COM_interp;
         Xinterp_ptr->at(i)=abs_XYZ.get(0);
         Yinterp_ptr->at(i)=abs_XYZ.get(1);
         Zinterp_ptr->at(i)=abs_XYZ.get(2);
      }
      cout << "Interpolated points.size() = " << Zinterp_ptr->size() << endl;

      double curr_time=point_cloud_times.front()+t_frac;
      string interpolated_tdp_filename="morph_"+stringfunc::number_to_string(
         curr_time,3)+".tdp";
      tdpfunc::write_xyzp_data(
         interpolated_tdp_filename,Xinterp_ptr,Yinterp_ptr,
         Zinterp_ptr,Pinterp_ptr);
      unix_cmd="lodtree "+interpolated_tdp_filename;
      sysfunc::unix_command(unix_cmd);

   } // loop over iter index

   delete Xinterp_ptr;
   delete Yinterp_ptr;
   delete Zinterp_ptr;
   delete Pinterp_ptr;
}
