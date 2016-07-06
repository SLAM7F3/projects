// ==========================================================================
// Program LiMIT_orbit fits an ellipse to a set of
// (longitude,latitude) waypoints for the LiMIT model's racetrack
// orbit in the integrated ISDS demo.  The 8 waypoints were provided
// by Peter Jones on 2/16/07.  This auxilliary program computes the
// best-fit ellipse's center position, its semi major and minor axes'
// lengths and the orientation angle between the ellipse's major axis
// and +x_hat.  
// ==========================================================================
// Last updated on 2/19/07; 4/11/12
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "geometry/ellipse.h"
#include "math/genmatrix.h"
#include "astro_geo/geopoint.h"
#include "general/sysfuncs.h"
#include "math/twovector.h"


using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
   cout.precision(15);

   vector<twovector> input_long_lat;
   input_long_lat.push_back(twovector(-118.385100,33.185000));
   input_long_lat.push_back(twovector(-118.799800,33.226800));
   input_long_lat.push_back(twovector(-118.849000,33.176000));
   input_long_lat.push_back(twovector(-118.803200,32.796700));
   input_long_lat.push_back(twovector(-118.726000,32.715800));
   input_long_lat.push_back(twovector(-118.359900,32.709400));
   input_long_lat.push_back(twovector(-118.290800,32.770400));
   input_long_lat.push_back(twovector(-118.326200,33.129900));

   bool northern_hemisphere_flag;
   int UTM_zone;
   vector<twovector> XY;
   for (unsigned int s=0; s<input_long_lat.size(); s++)
   {
      double longitude=input_long_lat[s].get(0);
      double latitude=input_long_lat[s].get(1);
      geopoint curr_geopoint(longitude,latitude);
      northern_hemisphere_flag=curr_geopoint.get_northern_hemisphere_flag();
      UTM_zone=curr_geopoint.get_UTM_zonenumber();
      XY.push_back(twovector(curr_geopoint.get_UTM_easting(),
                             curr_geopoint.get_UTM_northing()));
//      cout << "X = " <<  XY.back().get(0) 
//           << " Y = " << XY.back().get(1) << endl;
   }
   cout << "UTM_zone = " << UTM_zone << endl;
   
   twovector XY_avg;
   for (unsigned int r=0; r<XY.size(); r++)
   {
      XY_avg += XY[r];
   }
   XY_avg /= XY.size();
   cout << "XY_avg = " << XY_avg << endl;

   vector<twovector> delta_XY;
   for (unsigned int r=0; r<XY.size(); r++)
   {
      delta_XY.push_back(XY[r]-XY_avg);
//      cout << "r = "  << r << " delta_XY = " << delta_XY[r] << endl;
   }
   
   genmatrix M(delta_XY.size(),6);
   for (unsigned int r=0; r<delta_XY.size(); r++)
   {
      double x=delta_XY[r].get(0);
      double y=delta_XY[r].get(1);
      M.put(r,0,x*x);
      M.put(r,1,x*y);
      M.put(r,2,y*y);
      M.put(r,3,x);
      M.put(r,4,y);
      M.put(r,5,1);
   }
//   cout << "M = " << M << endl;

   genvector A(6);
   M.homogeneous_soln(A);
//      cout << "A = " << A << endl;
//      cout << "M*A = " << M*A << endl;
      
   genmatrix S(2,2);
   double a=A.get(0);
   double b=A.get(1);
   double c=A.get(2);
   double d=A.get(3);
   double e=A.get(4);
   S.put(0,0,a);
   S.put(0,1,0.5*b);
   S.put(1,0,S.get(0,1));
   S.put(1,1,c);
//   cout << "S = " << S << endl;
   double detS;
   S.determinant(detS);
//   cout << "det S = " << detS << endl;

   genmatrix D(2,2),U(2,2);
   
   S.sym_eigen_decomposition(D,U);
//   cout << "S - U * D * Utrans = " << S - U*D*U.transpose() << endl;
//   cout << "D = " << D << endl;
//   cout << "U = " << U << endl;

   double lambda1=D.get(0,0);
   double lambda2=D.get(1,1);

   double detU;
   U.determinant(detU);
//   cout << "det U = " << detU << endl;

   double cos_theta=U.get(0,0);
   double sin_theta=U.get(1,0);
   double theta=atan2(sin_theta,cos_theta);
   cos_theta=cos(theta);
   sin_theta=sin(theta);

   cout << "theta = " << theta*180/PI << endl;
   cout << "cos(theta) = " << cos_theta << " sin(theta) = " << sin_theta
        << endl;
   genmatrix R(2,2);
   R.put(0,0,cos_theta);
   R.put(1,0,sin_theta);
   R.put(0,1,-sin_theta);
   R.put(1,1,cos_theta);
//   cout << "R = " << R << endl;
//   cout << "S - R * D * Rtrans = "
//        << S - R*D*R.transpose() << endl;
   
   double u0=-(d*cos_theta+e*sin_theta)/(2*lambda1);
   double v0=(d*sin_theta-e*cos_theta)/(2*lambda2);
   cout << "u0 = " << u0 << " v0 = " << v0 << endl;

   twovector delta_x0y0=R*twovector(u0,v0);
   threevector XY_center=XY_avg+delta_x0y0;
   cout << "XY_center = " << XY_center << endl;

   geopoint ellipse_center(northern_hemisphere_flag,UTM_zone,
                           XY_center.get(0),XY_center.get(1));
   cout << "ellipse center = " << ellipse_center << endl;

   double G=1+sqr(lambda1*u0)+sqr(lambda2*v0);
   double major_axis=sqrt(G/lambda1);
   double minor_axis=sqrt(G/lambda2);
   cout << "major_axis = " << major_axis 
        << " minor_axis = " << minor_axis << endl;
   cout << "major/minor = " << major_axis/minor_axis << endl;

   ellipse racetrack_orbit(XY_center,major_axis,minor_axis,theta);
   vector<threevector> vertex=racetrack_orbit.generate_vertices(10);
   for (unsigned int v=0; v<vertex.size(); v++)
   {
      cout << "v = " << v << " vertex = " << vertex[v] << endl;
   }
   
}

