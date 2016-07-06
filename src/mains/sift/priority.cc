// ==========================================================================
// Program PRIORITY
// ==========================================================================
// Last updated on 3/19/12
// ==========================================================================

#include <iostream>
#include <queue>
#include <string>
#include <vector>
#include "graphs/samet_comparison.h"
#include "math/twovector.h"


int main()
{
   using std::cout;
   using std::endl;
   using std::priority_queue;
   using std::string;
   using std::vector;

   priority_queue<threevector,vector<threevector>,samet_comparison>* 
      search_queue_ptr=
      new priority_queue<threevector,vector<threevector>,samet_comparison>;

   priority_queue<threevector,vector<threevector>,samet_comparison>* 
      copy_queue_ptr=
      new priority_queue<threevector,vector<threevector>,samet_comparison>;

   threevector curr_e;

// Priority queue notes:

// Let q represent some element within the priority queue.

// First coordinate within q corresponds to distance
// Second coordinate within q represents node type

// 	node type==0 --> genvector metric space element
// 	node type==1 --> Metric subspace

// Third entry within q holds VP-tree node ID 

   curr_e=threevector(0,1,4);
   search_queue_ptr->push(curr_e);

   curr_e=threevector(0,0,4);
   search_queue_ptr->push(curr_e);

   curr_e=threevector(0,1,25);
   search_queue_ptr->push(curr_e);

/*
   curr_e=threevector(4,1);
   search_queue_ptr->push(curr_e);

   curr_e=threevector(1,1);
   search_queue_ptr->push(curr_e);   
   
   curr_e=threevector(1,0);
   search_queue_ptr->push(curr_e);   

   curr_e=threevector(2,1);
   search_queue_ptr->push(curr_e);
*/

   *copy_queue_ptr=*search_queue_ptr;

   cout << "search_queue_ptr->size() = " << search_queue_ptr->size() << endl;
   int queue_size=search_queue_ptr->size();
   for (int n=0; n<queue_size; n++)
   {
      threevector top_node=search_queue_ptr->top();
      cout << "n = " << n << " top node = " << top_node
           << endl;
      search_queue_ptr->pop();
   }

   for (int n=0; n<queue_size; n++)
   {
      threevector copy_node=copy_queue_ptr->top();
      cout << "n = " << n << " copy node = " << copy_node
           << endl;
      copy_queue_ptr->pop();
   }
   
   


}
