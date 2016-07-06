// ==========================================================================
// Program POLY is a testing grounds for our new polyhedron class.
// ==========================================================================
// Last updated on 3/2/07
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "geometry/face.h"
#include "math/threevector.h"

int main (int argc, char* argv[])
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;

   vector<threevector> V;
   V.push_back(threevector(0,0,0));
   V.push_back(threevector(0.5,0,0));
   V.push_back(threevector(1,0,0));
   V.push_back(threevector(1,1,0));
   V.push_back(threevector(0,1,0));
   V.push_back(threevector(0,0.5,0));

   face F(V);
   cout << "F = " << F << endl;

//   F.triangulate_convex_face();
   F.triangulate();
   
}

