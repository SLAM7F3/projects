// ==========================================================================
// Program RACETRACK simulates the flight path and camera orientation
// for a persistent surveillance system conducting a race-track orbit
// above our favorite Lowell suburb.  It writes out camera position
// and attitude information in a form which can be read in as an
// animation path by program VIEWPOINTS.
// ==========================================================================
// Last updated on 11/7/05
// ==========================================================================

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <osg/Quat>
#include <osg/Vec3f>
#include "math/constants.h"
#include "general/filefuncs.h"
#include "general/stringfuncs.h"
#include "math/threevector.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ios;
   using std::ofstream;
   using std::string;
   using std::vector;
  
   double x_avg=300;
   double y_avg=275;
   double R=275;
//   double height_angle=75*PI/180;
   double height_angle=48.5*PI/180;
   double H=R*tan(height_angle);
//   double H=308;
   double T=20;
   double omega=2*PI/T;

   double t_start=0;
   double t_stop=T;
   int nbins=180;
   double dt=(t_stop-t_start)/(nbins-1);

   string orbit_filename="spin.path";
   string los_filename="los.txt";
   string racetrack_filename="racetrack.txt";
   ofstream orbitstream,losstream,racestream;
   filefunc::openfile(orbit_filename,orbitstream);
   filefunc::openfile(los_filename,losstream);
   filefunc::openfile(racetrack_filename,racestream);

   osg::Vec3f xhat(1,0,0);
   osg::Vec3f yhat(0,1,0);
   osg::Vec3f zhat(0,0,1);
   osg::Quat q,q1,qz;

   const threevector origin(x_avg,y_avg,0);
   for (int n=0; n<nbins; n++)
   {
      double t=t_start+n*dt;
      double theta=omega*t;
      double x=x_avg+R*cos(theta);
      double y=y_avg+R*sin(theta);
      double z=H;
     
      threevector curr_posn(x,y,z);
      threevector n_hat=(origin-curr_posn).unitvector();

      qz.makeRotate(theta,zhat);
      q1.makeRotate(-zhat,osg::Vec3f(n_hat.get(0),n_hat.get(1),n_hat.get(2)));
      q=qz*q1;

//      cout << "x = " << x << " y = " << y
//           << " nx = " << n_hat.get(0)
//           << " ny = " << n_hat.get(1)
//           << " nz = " << n_hat.get(2) << endl;

      double qx=q._v[0];
      double qy=q._v[1];
      double qz=q._v[2];
      double qw=q._v[3];

      orbitstream << t << "  " << x << "  " << y << "  " << z << "  "
                  << qx << "  " << qy << "  " << qz << "  " << qw << endl;

// Write line segment emanating from camera's instantaneous position
// down to ground origin:

      const int passnumber=1;
      const int linecolor=11;	// yellow
      losstream << n << "  " << n << "   " << passnumber << "  "
                << x << "  " << y << "  " << z << "  "
                << origin.get(0) << "  " << origin.get(1) << "  " 
                << origin.get(2)
                << "  " << linecolor << endl;

      double roll=0;
      double pitch=0;
//      double yaw=0;
      double yaw=-theta*180/PI;
      racestream << n << "  " << n << "  "
                 << x << "  " << y << "  " << z << "  "
                 << roll << "  " << pitch << "  " << yaw << endl;
   }
   filefunc::closefile(orbit_filename,orbitstream);
   filefunc::closefile(los_filename,losstream);
   filefunc::closefile(racetrack_filename,racestream);

}
