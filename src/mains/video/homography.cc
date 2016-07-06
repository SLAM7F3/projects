// ==========================================================================
// Program HOMOGRAPHY
// ==========================================================================
// Last updated on 11/7/05
// ==========================================================================

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "image/camerafuncs.h"
#include "math/genmatrix.h"
#include "math/threevector.h"
#include "templates/mytemplates.h"

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
   using std::string;
   using std::vector;

   vector<double> X,Y,Z,u,v;
   
   X.push_back(242.3);
   X.push_back(559.2);
   X.push_back(357.3);
   X.push_back(41.1);
   
   Y.push_back(42.6);
   Y.push_back(239.5);
   Y.push_back(507.1);
   Y.push_back(310.0);

   u.push_back(1.00);
   u.push_back(0.96);
   u.push_back(0.41);
   u.push_back(0.27);

   v.push_back(0.31);
   v.push_back(0.72);
   v.push_back(0.74);
   v.push_back(0.34);

   vector<threevector> P,Q;
   for (int j=0; j<X.size(); j++)
   {
      P.push_back(threevector(X[j],Y[j],1));
      Q.push_back(threevector(u[j],v[j],1));
   }

   genmatrix* H_ptr=new genmatrix(3,3);
   genmatrix Hinv(3,3);
   camerafunc::compute_homography(X,Y,u,v,H_ptr);
   H_ptr->inverse(Hinv);
   cout << "*H_ptr = " << *H_ptr << endl;
   cout << "Hinv = " << Hinv << endl;
   cout << "H*Hinv = " << *H_ptr * Hinv << endl;

   twovector UV;
   for (int j=0; j<X.size(); j++)
   {
      threevector uvw=(*H_ptr)*P[j];
      UV.put(0,uvw.get(0)/uvw.get(2));
      UV.put(1,uvw.get(1)/uvw.get(2));
      cout << "j = " << j << " UV = " << UV << endl;
   }

   threevector XY;
   for (int j=0; j<X.size(); j++)
   {
      threevector xyz=Hinv*Q[j];
      XY.put(0,xyz.get(0)/xyz.get(2));
      XY.put(1,xyz.get(1)/xyz.get(2));
      cout << "j = " << j << " XY = " << XY << endl;
   }
   

}

