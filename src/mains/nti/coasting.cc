// ==========================================================================
// Program COASTING computes the vehicle track lifetimes in the
// presence of occlusions as a function of Kalman filter coasting
// time.  We assume occlusion event durations are exponentially
// distributed in time with a 1.5 second mean.  We further assume that
// occlusion events are Poisson distributed in time.  The latter
// distribution depends upon the average obscuration fraction of
// roadways.  This program simulates multiple arrangements of
// occluding events.  It then computes the number of such occluders
// which would break vehicle track as a function of coasting time.
// This number should tend to infinity [zero] as tau_coast goes
// towards zero [infinity].  In order to remove the dependence of
// final results upon the simulation length time, we report the
// average vehicle track lifetime.  In early Jan 2006, we found that
// occluded vehicle track lifetime grows approximately quadratically
// with increasing coasting time.
// ==========================================================================
// Last updated on 12/29/05; 12/4/10
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "image/binaryimagefuncs.h"
#include "geometry/contour.h"
#include "geometry/convexhull.h"
#include "image/drawfuncs.h"
#include "ladar/featurefuncs.h"
#include "filter/filterfuncs.h"
#include "general/filefuncs.h"
#include "math/genmatrix.h"
#include "math/genvector.h"
#include "geometry/geometry_funcs.h"
#include "image/graphicsfuncs.h"
#include "image/imagefuncs.h"
#include "ladar/ladarfuncs.h"
#include "datastructures/Linkedlist.h"
#include "math/mypolynomial.h"
#include "image/myimage.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "math/prob_distribution.h"
#include "image/recursivefuncs.h"
#include "geometry/regular_polygon.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "templates/mytemplates.h"
#include "geometry/triangulate_funcs.h"
#include "image/TwoDarray.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::ios;
   using std::ifstream;
   using std::ofstream;
   using std::ostream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

//   nrfunc::init_default_seed();
//   nrfunc::init_time_based_seed();

// Initialize various probability density parameters:

   const double Tgood=1000;
   const double kappa=1.0/1.5;	// avg occlusion duration rate in sec**-1

   double f_occlusion=0;
//   const double f_occlusion=0.15;	// Occlusion road coverage fraction
//   const double f_occlusion=0.1;	// Occlusion road coverage fraction
//   const double f_occlusion=0.05;	// Occlusion road coverage fraction
   cout << "Enter occlusion road coverage fraction:" << endl;
   cin >> f_occlusion;
   
   const double lambda=f_occlusion*kappa;
   const int mu_expected=basic_math::round(lambda*Tgood);
   cout << "Expected # occlusions per simulation = " 
        << mu_expected << endl;
   const double image_duration=0.5;	// sec
   const int nimages_per_simulation=basic_math::round(Tgood/image_duration);

   cout << "nimages_per_simulation = "
        << nimages_per_simulation << endl;

   const int n_taucoast_bins=20;
   genvector nbroken_tracks(n_taucoast_bins);

// Loop over simulations starts here:

   const int n_iter=10000;
   vector<double> number_occlusions;
   vector<double> occlusion_time;
   occlusion_time.reserve(2*mu_expected*n_iter);
   for (int iter=0; iter<n_iter; iter++)
   {

// Generate waiting times between occlusion events obeying the
// exponential distribution p(t) = lambda * exp(-lambda t):

      int n_occlusions=0;
      double curr_occlusion_time=0;
      double prev_occlusion_time=0;
      while (curr_occlusion_time < Tgood)
      {
         curr_occlusion_time=prev_occlusion_time+nrfunc::expdev(lambda);
         if (curr_occlusion_time < Tgood)
         {
            occlusion_time.push_back(curr_occlusion_time);
            prev_occlusion_time=curr_occlusion_time;
//            cout << "occlusion time = " << curr_occlusion_time << endl;
            n_occlusions++;
         }
      }
      number_occlusions.push_back(n_occlusions);

// Generate occlusion duration times obeying the exponential
// distribution p(tau) = kappa exp(-kappa*tau):

      genvector vehicle_occluded(nimages_per_simulation);
      vehicle_occluded.clear_values();

      for (int i=0; i<number_occlusions.back(); i++)
      {
         double curr_occlusion_time=occlusion_time[
            occlusion_time.size()-number_occlusions.back()+i];
         int curr_imagenumber=
            basic_math::round(curr_occlusion_time/image_duration);
//         cout << "i = " << i 
//              << " occlusion_time = " << curr_occlusion_time
//              << " curr_imagenumber = " << curr_imagenumber << endl;
         double curr_occlusion_duration=nrfunc::expdev(kappa);

//         int n_occluded_images=basic_math::round(curr_occlusion_duration/
//                                                 image_duration);
         int n_occluded_images=ceil(curr_occlusion_duration/
                                    image_duration);
//         cout << "curr_occ_duration = " << curr_occlusion_duration
//              << " n_occluded_images = " << n_occluded_images << endl;

         for (int j=0; j<n_occluded_images; j++)
         {
            int occluded_imagenumber=curr_imagenumber+j;
//            cout << "j = " << j
//                 << " occluded imagenumber = " << curr_imagenumber+j
//                 << endl;
            if (occluded_imagenumber < nimages_per_simulation)
            {
               vehicle_occluded.put(curr_imagenumber+j,1.0);
            }
         } // loop over index j labeling occluded image numbers
      } // loop over index i labeling occlusion events occuring in current
        //  simulation

// Loop over all entries in vehicle_occluded genvector.  Find all
// contiguous occluded intervals and form an STL vector of their
// sizes:

//      cout << "iter = " << iter 
//           << " vehicle_occluded genvector = "
//           << vehicle_occluded 
//           << endl;

      bool vehicle_in_occlusion_flag=false;
      int occlusion_length=0;
      vector<int> occlusion_duration;
      for (int n=0; n<vehicle_occluded.get_mdim(); n++)
      {
         if (nearly_equal(vehicle_occluded.get(n),1.0))
         {
            vehicle_in_occlusion_flag=true;
            occlusion_length++;
            if (n==vehicle_occluded.get_mdim()-1)
            {
               occlusion_duration.push_back(occlusion_length);
//               cout << "occlusion length = " 
//                    << occlusion_duration.back() << endl;
            }
         }
         else
         {
            if (vehicle_in_occlusion_flag)
            {
               occlusion_duration.push_back(occlusion_length);
//               cout << "occlusion length = " 
//                    << occlusion_duration.back() << endl;
               occlusion_length=0;
            }
            vehicle_in_occlusion_flag=false;
         }
      } // loop over index n labeling entries in vehicle_occluded genvector

//  Loop of tau_coast values.  Count number of entries in
//  occlusion_duration which exceed tau_coast.  Save results in
//  genvector nbroken_tracks:

      for (int i=0; i<occlusion_duration.size(); i++)
      {
         for (int n=0; n<n_taucoast_bins; n++)
         {
            int tau_coast=n;	// coasting time measured in numbers of images
            if (occlusion_duration[i] > tau_coast)
            {
               nbroken_tracks.put(n,nbroken_tracks.get(n)+1);
            }
         } // loop over index n
      } // loop over i index labeling occlusion_duration entries
      
//      cout << endl;

   } // loop over iter index labeling simulation

   string metafilename="broken_tracks.meta";
   ofstream metastream;
   filefunc::openfile(metafilename,metastream);

   for (int n=0; n<n_taucoast_bins; n++)
   {
      double tau_coast=n*image_duration;  // coasting time measured in secs
      double nbroken_tracks_per_sim=nbroken_tracks.get(n)/double(n_iter);
      nbroken_tracks.put(n,nbroken_tracks_per_sim);
      double avg_track_lifetime=basic_math::min(
         Tgood/nbroken_tracks.get(n),10000.0);
//      cout << "n = " << n << " tau_coast = " << tau_coast*image_duration
//           << " nbroken_tracks/simulation = " << nbroken_tracks_per_sim
//           << endl;
      cout << "tau_coast = " << tau_coast
           << " avg track lifetime = " << avg_track_lifetime
           << " nbroken_tracks/expected # occlusions = "
           << nbroken_tracks_per_sim/mu_expected << endl;
      metastream << tau_coast << "    " 
                 << avg_track_lifetime << endl;
   } // loop over index n

   filefunc::closefile(metafilename,metastream);

/*
  const int n_output_bins=100;
  const double xlo=0;
  const double dx=1.0;
  prob_distribution prob(number_occlusions,n_output_bins,xlo,dx);
  prob.densityfilenamestr="poisson.meta";
  prob.xlabel="Number of occlusion events per simulation";
  bool gzip_flag=false;
  prob.freq_histogram=true;
  prob.write_density_dist(gzip_flag);
*/
   

 
}
