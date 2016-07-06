// ==========================================================================
// Program INTERSECT 

// ==========================================================================
// Last updated on 3/2/12
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "geometry/geometry_funcs.h"
#include "geometry/linesegment.h"
#include "geometry/plane.h"
#include "geometry/polygon.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

int main (int argc, char * argv[])
{

   double delta_phi=0;
   cout << "Enter delta_phi in degs:" << endl;
   cin >> delta_phi;

   double radius=10;
   double phi0=90*PI/180;
   double phi1=phi0+delta_phi*PI/180;

   double theta0=0;
   double theta1=0;

   threevector ehat_0=mathfunc::construct_direction_vector(phi0,theta0);
   threevector ehat_1=mathfunc::construct_direction_vector(phi1,theta1);

   cout << "ehat_0 = " << ehat_0 << endl;
   cout << "ehat_1 = " << ehat_1 << endl;

   threevector origin(0,0,0);
   threevector V1_0=origin-radius*ehat_0;
   threevector V1_1=origin-radius*ehat_1;

   cout << "V1_0 = " << V1_0 << endl;
   cout << "V1_1 = " << V1_1 << endl;
   
   vector<linesegment> lines;
   lines.push_back(linesegment(V1_0,origin));
   lines.push_back(linesegment(V1_1,origin));
   
   cout << "lines[0] = " << lines[0] << endl;
   cout << "lines[1] = " << lines[1] << endl;

   threevector intersection_point,sigma_intersection_point;
   bool flag=geometry_func::multi_line_intersection_point(
      lines,intersection_point,sigma_intersection_point);
//      lines,intersection_point);

   if (flag)
   {
      cout << "intersection_point = " << intersection_point << endl;
      cout << "sigma_intersection_point = " << sigma_intersection_point << endl;
   }
   else
   {
      cout << "Intersection cannot be reliably computed" << endl;
   }
   

} 

