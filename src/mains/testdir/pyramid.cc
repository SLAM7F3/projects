// ==========================================================================
// Program PYRAMID is a testing grounds for our new geometry/pyramid class.
// ==========================================================================
// Last updated on 7/4/07
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "geometry/pyramid.h"
#include "math/threevector.h"

int main (int argc, char* argv[])
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;


   threevector apex(0,0,-3);
   vector<threevector> base_vertices;
   base_vertices.push_back(threevector(1,1,0));
   base_vertices.push_back(threevector(-1,1,0));
   base_vertices.push_back(threevector(-1,-1,0));
   base_vertices.push_back(threevector(1,-1,0));

   pyramid P(apex,base_vertices);
   

   cout << "Pyramid P: " << endl;
   cout << P << endl;

   double z=0;
   P.base_part_above_Zplane(z);
   
}

