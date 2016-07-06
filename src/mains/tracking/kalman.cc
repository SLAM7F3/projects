// ==========================================================================
// Program KALMAN is a testing grounds for developing a rudimentary
// understanding of Kalman filters.
// ==========================================================================
// Last updated on 3/26/07; 3/27/07
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "color/colorfuncs.h"
#include "filter/filterfuncs.h"
#include "math/genmatrix.h"
#include "math/genvector.h"
#include "track/kalman.h"
#include "math/mathfuncs.h"
#include "plot/metafile.h"
#include "numrec/nrfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

// ==========================================================================
int main (int argc, char* argv[])
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;

   nrfunc::init_time_based_seed();   

// Car (target) parameters:

   const double x_start=0;
   const double x_stop=20 * 1000;	 // car travel distance in meters
   const double mph_2_mps = 0.44704; 	 // 1 mph = 0.44704 meters/sec

   const double v0=35 * mph_2_mps;  // Avg car speed in meters/sec = 35 mph
   double T_expt=(x_stop-x_start)/v0;

// Sensor measurement parameters:

   const double dt=25;	// time between raw sensor measurements (secs)
   int nbins=T_expt/dt; // # sensor measurements made over entire expt
   double dx=(x_stop-x_start)/(nbins-1); 
	// distance car travels between sensor measurements

   vector<double> sample_time,x_true,v_true,x_raw,v_raw,x_smoothed,v_smoothed;
   vector<double> delta_x,delta_v;
   sample_time.push_back(0);
   x_true.push_back(0);
   v_true.push_back(v0);
   x_raw.push_back(x_true.back());
   x_smoothed.push_back(x_true.back());

   const int ndim=2;	// Dimension of state vector
//   const int mdim=1;
   const int mdim=2;	// Dimension of measurement vector
   kalman Kalman(ndim,mdim);

// Physical state vector:

   Kalman.get_X_ptr()->put(0,0);
   Kalman.get_X_ptr()->put(1,v0);
   
   genvector X(ndim);
   X.put(0,0);
   X.put(1,v0);

// Temporal propagation matrix:

   genmatrix Phi(ndim,ndim);
   Phi.put(0,0,1);
   Phi.put(0,1,dt);
   Phi.put(1,0,0);
   Phi.put(1,1,1);

// Relationship between physical system's state vector and measurement
// vector:

   genmatrix H(mdim,ndim);
   H.put(0,0,1);
   H.put(0,1,0);
   H.put(1,0,0);
   H.put(1,1,1);

// Process noise vector:

   const double W_sigma=10 * mph_2_mps;		// meters/sec
   genvector W(ndim);

// Covariance matrix for state vector:

   genmatrix Q(ndim,ndim);
   Q.put(0,0,0);
   Q.put(0,1,0);
   Q.put(1,0,0);
   Q.put(1,1,sqr(W_sigma));

// Measurement vector:

   genvector Z(mdim);

// Measurement noise vector:

   const double posn_sigma=30.0;		// meters
   const double speed_sigma=10 * mph_2_mps;	// meters/sec
   genvector V(mdim);

// Covariance matrix for measurement vector:

   genmatrix R(mdim,mdim);
   R.put(0,0,sqr(posn_sigma));
   R.put(0,1,0);
   R.put(1,0,0);
   R.put(1,1,sqr(speed_sigma));

// Covariance matrix for smoothed estimate:

   genmatrix P(ndim,ndim);
   P.put(0,0,0);
   P.put(0,1,0);
   P.put(1,0,0);
   P.put(1,1,0);

// Kalman gain vector:

   genmatrix K(ndim,mdim);

   genmatrix identity(ndim,ndim);
   identity.identity();

// --------------------------------------------------------------------------
// Kalman filter loop starts here:
   
   for (int k=1; k<nbins; k++)
   {
      double t=k*dt;
      sample_time.push_back(t);
      x_true.push_back(x_true.back()+v_true.back()*dt);
      v_true.push_back(v_true.back());

// Compute Kalman gain:

      genmatrix tmp1(mdim,mdim),inverse_tmp1(mdim,mdim);
      tmp1=H*P*H.transpose()+R;
      tmp1.inverse(inverse_tmp1);
      K=P*H.transpose()*inverse_tmp1;

// Update estimate with latest measurement:

      Z.put(0,x_true.back());
      Z.put(1,v_true.back());
      V.put(0,posn_sigma*nrfunc::gasdev());
      V.put(1,speed_sigma*nrfunc::gasdev());
      Z=Z+V;

      x_raw.push_back(Z.get(0));
      v_raw.push_back(Z.get(1));

      X=X+K*(Z-H*X);
      x_smoothed.push_back(X.get(0));
      v_smoothed.push_back(X.get(1));

      if (fabs(x_smoothed[k]-x_true[k]) < 350)
      {
         delta_x.push_back(x_smoothed[k]-x_true[k]);
         delta_v.push_back(v_smoothed[k]-v_true[k]);

         cout << "k = " << k 
//           << " K[0] = " << K.get(0)
//           << " K[1] = " << K.get(1) 
              << " delta_x = " << delta_x.back()
              << " delta_v = " << delta_v.back()
              << endl;
      }

// Compute error covariance for updated estimate:

      P=(identity-K*H)*P;
   
// Project ahead:

      X=Phi*X;
      P=Phi*P*Phi.transpose()+Q;

   } // loop over index k labeling time step
// --------------------------------------------------------------------------

   double mean_delta_x=mathfunc::mean(delta_x);
   double std_dev_delta_x=mathfunc::std_dev(delta_x);
   cout << "delta_x = " << mean_delta_x << " +/- "
        << std_dev_delta_x << endl;
   
   double mean_delta_v=mathfunc::mean(delta_v);
   double std_dev_delta_v=mathfunc::std_dev(delta_v);
   cout << "delta_v = " << mean_delta_v << " +/- "
        << std_dev_delta_v << endl;

// Generate metafile plot outputs:

   metafile x_meta;
   string metafile_name="kalman_x";
   x_meta.set_parameters(
      metafile_name,"Position of const speed vehicle vs time",
      "Time (secs)","Vehicle posn (meters)",
      0,T_expt,100,50,
      x_start,x_stop,2000,1000);
   x_meta.add_extrainfo(
      "Avg speed = "+stringfunc::number_to_string(v0/mph_2_mps)+" mph");
   x_meta.add_extrainfo(
      "delta_x = "+stringfunc::number_to_string(mean_delta_x)+" +/- "
      +stringfunc::number_to_string(std_dev_delta_x)+" m");
   x_meta.add_extrainfo(
      "delta_v = "+stringfunc::number_to_string(mean_delta_v)+" +/- "
      +stringfunc::number_to_string(std_dev_delta_v)+" m/sec");

   x_meta.openmetafile();
   x_meta.write_header();
   x_meta.write_curve(sample_time,x_raw,colorfunc::red);
   x_meta.write_curve(sample_time,x_true,colorfunc::blue);
   x_meta.write_curve(sample_time,x_smoothed,colorfunc::green);
   x_meta.closemetafile();

   string unixcommandstr="meta_to_pdf "+metafile_name;
   sysfunc::unix_command(unixcommandstr);

// --------------------------------------------------------------------------
   metafile deltax_meta;
   metafile_name="delta_x";
   deltax_meta.set_parameters(
      metafile_name,"Filtered-true vehicle posn vs time",
      "Time (secs)","Filtered - true vehicle posn (meters)",
      0,T_expt,100,50,
      -60,60,10,10);
   deltax_meta.add_extrainfo(
      "Avg speed = "+stringfunc::number_to_string(v0/mph_2_mps)+" mph");
   deltax_meta.add_extrainfo(
      "delta_x = "+stringfunc::number_to_string(mean_delta_x)+" +/- "
      +stringfunc::number_to_string(std_dev_delta_x)+" m");
   deltax_meta.add_extrainfo(
      "delta_v = "+stringfunc::number_to_string(mean_delta_v)+" +/- "
      +stringfunc::number_to_string(std_dev_delta_v)+" m/sec");

   deltax_meta.openmetafile();
   deltax_meta.write_header();
   deltax_meta.write_curve(sample_time,delta_x,colorfunc::red);
   deltax_meta.closemetafile();

   unixcommandstr="meta_to_pdf "+metafile_name;
   sysfunc::unix_command(unixcommandstr);

}
