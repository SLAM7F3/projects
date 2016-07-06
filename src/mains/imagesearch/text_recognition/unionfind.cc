// ==========================================================================
// Program UNIONFIND is a playpen for experimenting with a "non-tree"
// implementation of the union-find algorithm.
// ==========================================================================
// Last updated on 7/23/12; 7/24/12
// ==========================================================================

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "templates/mytemplates.h"
#include "general/sysfuncs.h"
#include "datastructures/union_find.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
   
   union_find* union_find_ptr=new union_find();
//   union_find_ptr->CreateNode(1,2);
//   union_find_ptr->CreateNode(2,3);
//   union_find_ptr->CreateNode(3,3);
//   union_find_ptr->CreateNode(4,3);
//   union_find_ptr->CreateNode(5,7);
//   union_find_ptr->CreateNode(6,7);
//   union_find_ptr->CreateNode(7,7);
//   union_find_ptr->CreateNode(8,3);


   union_find_ptr->MakeSet(0);
   union_find_ptr->MakeSet(1);
   union_find_ptr->MakeSet(2);
   union_find_ptr->MakeSet(3);
   union_find_ptr->MakeSet(4);
   union_find_ptr->MakeSet(5);
   union_find_ptr->MakeSet(6);
   union_find_ptr->MakeSet(7);
   union_find_ptr->MakeSet(8);
   union_find_ptr->MakeSet(9);

   union_find_ptr->Link(3,4);
   union_find_ptr->Link(4,9);
   union_find_ptr->Link(8,0);
//   union_find_ptr->Link(2,3);
   union_find_ptr->Link(5,6);
//   union_find_ptr->Link(5,9);
   union_find_ptr->Link(9,5);
   union_find_ptr->Link(7,3);
//   union_find_ptr->Link(4,8);
   union_find_ptr->Link(6,1);

   for (int node_ID=0; node_ID<union_find_ptr->get_n_nodes(); node_ID++)
   {
      int parent_ID=union_find_ptr->get_parent_ID(node_ID);
      int root_ID=union_find_ptr->Find(node_ID);
      int rank=union_find_ptr->get_rank(node_ID);

      cout << "node_ID = " << node_ID
           << " parent_ID = " << parent_ID 
           << " root_ID = " << root_ID
           << " rank = " << rank
           << endl;
   }

   delete union_find_ptr;
}
