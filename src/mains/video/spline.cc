// ==========================================================================
// Program SPLINE reads in raw GPS/IMU information after it is
// extracted from G99 video data via program TELEMETRY.  It expects to
// find an ascii file called "TPA.txt" in which TELEMETRY's output
// resides.  SPLINE first fits splines to the aircraft's raw position
// and attitude and generates a new interpolated set of data points
// corresponding to every 1/24th of a second.  SPLINE subsequently
// convolves the evenly sampled data with a gaussian whose width is
// set by time constant tau.  The interpolated and filtered results
// are written out to ascii file "TPA_filtered.txt".
// ==========================================================================
// Last updated on 11/10/05
// ==========================================================================

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "math/threevector.h"
#include "numrec/nr.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/videofuncs.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ios;
   using std::ofstream;
   using std::ostream;
   using std::setw;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

   string TPA_filename="TPA.txt";
   vector<string> line;
   filefunc::ReadInfile(TPA_filename,line);
   stringfunc::comment_strip(line);

   const int n_fields=8;
   double X[n_fields];
   vector<int> imagenumber;
   vector<double> rel_time,rel_X,rel_Y,rel_Z;
   vector<double> roll,pitch,yaw;
   for (int i=0; i<line.size(); i++)
   {
      stringfunc::string_to_n_numbers(n_fields,line[i],X);
      imagenumber.push_back(static_cast<int>(X[0]));
      rel_time.push_back(X[1]);
      rel_X.push_back(X[2]);
      rel_Y.push_back(X[3]);
      rel_Z.push_back(X[4]);
      roll.push_back(X[5]);
      pitch.push_back(X[6]);
      yaw.push_back(X[7]);

//      cout << rel_time.back() << "\t\t" << rel_Y.back() << endl;
      
//      cout << "image = " << imagenumber.back()
//           << " t = " << rel_time.back()
//           << " posn = " << rel_posn.back()
//           << " att = " << attitude.back() << endl;
   }

   const double PRF=24.0;	// Hz
   const double t_start=0;
   const double t_stop=rel_time.back();
   const double dt=1.0/PRF;
   int nbins=(t_stop-t_start)/dt+1;
   vector<double> t_reg;
   for (int n=0; n<nbins; n++)
   {
      t_reg.push_back(t_start+n*dt);
   }

// Use spline interpolations to fill gaps in raw GPS/IMU measurements
// and convert to evenly spaced time samples:

   vector<double> interpolated_X,interpolated_Y,interpolated_Z;
   vector<double> interpolated_roll,interpolated_pitch,interpolated_yaw;

   numrec::init_spline_2nd_derivs(rel_time,rel_X);
   for (int n=0; n<nbins; n++)
   {
      interpolated_X.push_back(numrec::spline_interp(
         rel_time,rel_X,t_reg[n]));
   }

   numrec::init_spline_2nd_derivs(rel_time,rel_Y);
   for (int n=0; n<nbins; n++)
   {
      interpolated_Y.push_back(numrec::spline_interp(
         rel_time,rel_Y,t_reg[n]));
   }

   numrec::init_spline_2nd_derivs(rel_time,rel_Z);
   for (int n=0; n<nbins; n++)
   {
      interpolated_Z.push_back(numrec::spline_interp(
         rel_time,rel_Z,t_reg[n]));
   }

   numrec::init_spline_2nd_derivs(rel_time,roll);
   for (int n=0; n<nbins; n++)
   {
      interpolated_roll.push_back(numrec::spline_interp(
         rel_time,roll,t_reg[n]));
   }

   numrec::init_spline_2nd_derivs(rel_time,pitch);
   for (int n=0; n<nbins; n++)
   {
      interpolated_pitch.push_back(numrec::spline_interp(
         rel_time,pitch,t_reg[n]));
   }

   numrec::init_spline_2nd_derivs(rel_time,yaw);
   for (int n=0; n<nbins; n++)
   {
      interpolated_yaw.push_back(numrec::spline_interp(
         rel_time,yaw,t_reg[n]));
   }

// Convolve interpolated sensor readings with a gaussian filter to
// remove high-frequency jitter:

   vector<threevector> sensor_position,sensor_attitude,sensor_velocity;
   for (int n=0; n<nbins; n++)
   {
      sensor_position.push_back(threevector(
         interpolated_X[n],interpolated_Y[n],interpolated_Z[n]));
      sensor_attitude.push_back(threevector(
         interpolated_roll[n],interpolated_pitch[n],interpolated_yaw[n]));
   }

   const double video_PRF=24;	// Hz
   const double tau=1.0/video_PRF; // filter time constant (secs)
//   const double tau=0.5;	// filter time constant measured in secs
   videofunc::filter_raw_sensor_positions(nbins,dt,tau,sensor_position);
   videofunc::filter_raw_sensor_positions(nbins,dt,tau,sensor_attitude);

   string filtered_TPA_filename="TPA_filtered.txt";
   filefunc::deletefile(filtered_TPA_filename);
   ofstream filtered_TPA_stream;
   filefunc::openfile(filtered_TPA_filename,filtered_TPA_stream);

   filtered_TPA_stream << "# Img  Rel time       Rel_x  	 Rel_y	     Rel_z       Roll 	      Pitch        Yaw"
                       << endl << endl;
   filtered_TPA_stream.precision(7);
   const int column_width=9;
   for (int n=0; n<nbins; n++)
   {
      if (n > 0)
      {
         sensor_velocity.push_back((sensor_position[n]-
                                    sensor_position[n-1])/dt);
      }
      filtered_TPA_stream << setw(4) << n << "   "
                          << setw(column_width) << t_reg[n] << "   "
                          << setw(column_width)
                          << sensor_position[n].get(0) << "   "
                          << setw(column_width)
                          << sensor_position[n].get(1) << "   "
                          << setw(column_width)
                          << sensor_position[n].get(2) << "   "
                          << setw(column_width)
                          << sensor_attitude[n].get(0) << "   "
                          << setw(column_width)
                          << sensor_attitude[n].get(1) << "   "
                          << setw(column_width)
                          << sensor_attitude[n].get(2) << "   "
                          << endl;
   }
   filefunc::closefile(filtered_TPA_filename,filtered_TPA_stream);

   double avg_sensor_speed=0;
   for (int n=0; n<sensor_velocity.size(); n++)
   {
      avg_sensor_speed += sensor_velocity[n].magnitude();
   }
   avg_sensor_speed /= sensor_velocity.size();
   cout << "Average sensor speed = " << avg_sensor_speed << endl;

}
