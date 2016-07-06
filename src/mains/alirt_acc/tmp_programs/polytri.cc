// ==========================================================================
// Program POLYTRI
// ==========================================================================
// Last updated on 6/21/04
// ==========================================================================

#include <iostream>
#include <iomanip>
#include "datastructures/Linkedlist.h"
#include "math/myvector.h"
#include "polygon.h"
#include "general/sysfuncs.h"
#include "triangulate_funcs.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ios;
   using std::iostream;
   std::set_new_handler(sysfunc::out_of_memory);

   int npoints=7;
   myvector vertex[npoints];
   vertex[0]=myvector(0,0);
   vertex[1]=myvector(10,0);
   vertex[2]=myvector(15,5);
   vertex[3]=myvector(10,10);
   vertex[4]=myvector(5,20);
   vertex[5]=myvector(0,10);
   vertex[6]=myvector(-10,-5);
   triangulate_func::read_polygon_vertices(npoints,vertex);

   cout << "Area of polygon = " << 0.5*triangulate_func::AreaPoly2() << endl;
   Linkedlist<polygon*>* triangle_list_ptr=triangulate_func::Triangulate();

   for (Mynode<polygon*>* currnode_ptr=triangle_list_ptr->get_start_ptr();
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      cout << "Triangle = " 
           << *(currnode_ptr->get_data()) << endl;
   }

   triangulate_func::delete_triangle_list(triangle_list_ptr);
 
}
