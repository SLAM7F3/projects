// ==========================================================================
// Program POLY is a testing grounds for our new polyhedron class.
// ==========================================================================
// Last updated on 3/2/07
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "geometry/polyhedron.h"
#include "math/threevector.h"

int main (int argc, char* argv[])
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;

/*
   vector<threevector> V;
   V.push_back(threevector(0,0,0));
   V.push_back(threevector(1,0,0));
   V.push_back(threevector(0,1,0));
   V.push_back(threevector(1,1,2));

   polyhedron P(V);

   P.set_edge(1,0);
   P.set_edge(1,2);
   P.set_edge(1,3);
   P.set_edge(2,0);
   P.set_edge(3,0);
   P.set_edge(2,3);

   P.set_face(0,3,1);
   P.set_face(4,0,2);
   P.set_face(5,2,1);
   P.set_face(5,3,4);

   cout << "P = " << P << endl;
*/


   vector<threevector> V;


/*
   V.push_back(threevector(0,0,1));
   V.push_back(threevector(1,-1,-1));
   V.push_back(threevector(0,1,-1));
   V.push_back(threevector(-1,-1,-1));

   polyhedron P;
   P.generate_tetrahedron(V);
   
   cout << P << endl;
*/
 

   V.push_back(threevector(0,0,-1));
   V.push_back(threevector(1,0,1));
   V.push_back(threevector(-1,0,1));

//   V.push_back(threevector(0,0,1));
//   V.push_back(threevector(-1,0,-1));
//   V.push_back(threevector(1,0,-1));

//   V.push_back(threevector(0,0,1));
//   V.push_back(threevector(1,0,2));
//   V.push_back(threevector(-1,0,2));

/*
   polyhedron P(V);
   P.set_edge(0,1);
   P.set_edge(1,2);
   P.set_edge(2,0);

   P.set_face(0,1,2);

   face* face_ptr=P.get_face_ptr(0);
*/


   face* face_ptr=new face(V);
   cout << "*face_ptr = " << *face_ptr << endl;

   face_ptr->swap_vertex_order();
   cout << "After swapping, *face_ptr = " << *face_ptr << endl;

   
//   double z=0;
//   face_ptr->intersection_with_Zplane(z);
//   P.intersection_with_Zplane(z);
}

