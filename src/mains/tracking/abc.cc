// ==========================================================================
// Program ABC is a testing grounds for alpha-beta-gamma filtering
// ==========================================================================
// Last updated on 3/21/07
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "color/colorfuncs.h"
#include "filter/filterfuncs.h"
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

// Car (target) parameters:

   const double x_start=0;
   const double x_stop=2000;	// car travel distance in meters
   const double mph_2_mps = 0.44704;  // 1 mph = 0.44704 meters/sec

   const double v0=15.6;	// Avg car speed in meters/sec = 35 mph
   double T_expt=(x_stop-x_start)/v0;
   double Delta_v=4.5;	// amplitude for car speed oscillation = 10 mph
   double T_speed_osc=60;	// secs
   double omega_v=2*PI/T_speed_osc;

// Sensor measurement parameters:

   const double dt=20;	// time between raw sensor measurements (secs)
   int nbins=T_expt/dt; // # sensor measurements made over entire expt
   double dx=(x_stop-x_start)/(nbins-1); 
	// distance car travels between sensor measurements
   const double S2N=1;
//   const double S2N=10;

   double sigma=20;	// meters
//   cout << "Enter sigma for noise fluctuations:" << endl;
//   cin >> sigma;

   cout << "T_expt = " << T_expt << " nbins = " << nbins << endl;
   cout << "dx = " << dx << endl;

   vector<double> sample_time,x_true,x_raw,v_true;
   for (int n=0; n<nbins; n++)
   {
      double t=n*dt;
      sample_time.push_back(t);
      x_true.push_back(x_start+v0*t+Delta_v/omega_v*(1-cos(omega_v*t)));
      v_true.push_back(v0+Delta_v*sin(omega_v*t));

      double noise=sigma*nrfunc::gasdev();
      x_raw.push_back(x_true.back()+noise/sqrt(S2N));
   }

   double alpha,beta,gamma,theta;
   cout << "Enter theta:" << endl;
   cin >> theta;
//   cout << "Enter alpha:" << endl;
//   cin >> alpha;
//   cout << "Enter beta:" << endl;
//   cin >> beta;
   
   alpha=1-pow(theta,3);
   beta=1.5*(1-sqr(theta))*(1-theta);
   gamma=pow(1-theta,3);
   
   double curr_filtered_x,curr_filtered_xdot,curr_filtered_xdotdot;
   double prev_filtered_x=0;
   double prev_filtered_xdot=0;
   double prev_filtered_xdotdot=0;
   vector<double> x_filtered,xdot_filtered,xdotdot_filtered,delta_x,delta_v;
   for (int n=0; n<nbins; n++)
   {
      double curr_raw_x=x_raw[n];
//      filterfunc::alphabeta_filter(
//         curr_raw_x,prev_filtered_x,prev_filtered_xdot,
//         curr_filtered_x,curr_filtered_xdot,alpha,beta,dt);
      filterfunc::alphabetagamma_filter(
         curr_raw_x,prev_filtered_x,prev_filtered_xdot,prev_filtered_xdotdot,
         curr_filtered_x,curr_filtered_xdot,curr_filtered_xdotdot,
         alpha,beta,gamma,dt);

      x_filtered.push_back(curr_filtered_x);
      xdot_filtered.push_back(curr_filtered_xdot);
      xdotdot_filtered.push_back(curr_filtered_xdotdot);

      delta_x.push_back(x_filtered[n]-x_true[n]);
      delta_v.push_back(xdot_filtered[n]-v_true[n]);

/*
      cout << n 
           << " t = " << sample_time[n] 
           << " x_true = " << x_true[n] 
           << " x_raw-x_true = " << x_raw[n]-x_true[n]
           << " x_filtered-x_true = " << delta_x[n] << endl;
*/
      cout << n 
           << " t = " << sample_time[n] 
           << " v_true = " << v_true[n]
           << " xdot_filtered-v_true = " << delta_v[n]
           << endl;

      prev_filtered_x=curr_filtered_x;
      prev_filtered_xdot=curr_filtered_xdot;
      prev_filtered_xdotdot=curr_filtered_xdotdot;
      
   } // loop over index n

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
   string metafile_name="posn";
   x_meta.set_parameters(
      metafile_name,"Vehicle position vs time",
      "Time (secs)","Vehicle position (meters)",
      0,T_expt,30,15,
      x_start,x_stop,1000,250);
   x_meta.add_extrainfo(
      "Avg vehicle speed = "+
      stringfunc::number_to_string(v0/mph_2_mps)+" mph");
   x_meta.add_extrainfo(
      "Sample time step = "+stringfunc::number_to_string(dt));
   x_meta.add_extrainfo(
      "Filter theta = "+stringfunc::number_to_string(theta));
   x_meta.add_extrainfo(
      "delta_x = "+stringfunc::number_to_string(mean_delta_x)+" +/- "
      +stringfunc::number_to_string(std_dev_delta_x)+" m");
   x_meta.add_extrainfo(
      "delta_v = "+stringfunc::number_to_string(mean_delta_v)+" +/- "
      +stringfunc::number_to_string(std_dev_delta_v)+" m/sec");

   x_meta.openmetafile();
   x_meta.write_header();
   x_meta.write_curve(sample_time,x_true,colorfunc::red);
   x_meta.write_curve(sample_time,x_filtered,colorfunc::blue);
   x_meta.closemetafile();

   string unixcommandstr="meta_to_pdf "+metafile_name;
   sysfunc::unix_command(unixcommandstr);

}
