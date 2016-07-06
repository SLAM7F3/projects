// ==========================================================================
// Program METRICTEST
// ==========================================================================
// Last updated on 3/17/12; 3/18/12; 3/19/12
// ==========================================================================

#include <iostream>
#include <map>
#include <queue>
#include <string>
#include <vector>

#include "datastructures/BinaryTree.h"
#include "color/colorfuncs.h"
#include "general/filefuncs.h"
#include "graphs/graph.h"
#include "math/gttwovector.h"
#include "graphs/node.h"
#include "numrec/nrfuncs.h"
#include "image/pngfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/twovector.h"
#include "graphs/vptree.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::priority_queue;
using std::string;
using std::vector;

int main (int argc, char * argv[])
{
   std::set_new_handler(sysfunc::out_of_memory);

//   int n_elements=3;
//   int n_elements=6;
   int n_elements=10;
//   cout << "Enter number of metric space elements:" << endl;
//   cin >> n_elements;

   vector<genvector*> metric_space_elements;
   metric_space_elements.reserve(n_elements);
   for (int n=0; n<n_elements; n++)
   {
      double x=10*nrfunc::ran1();
      double y=10*nrfunc::ran1();
//      double x=n;
//      double x=n*n;
//      double y=0;
      twovector* curr_point_ptr=new twovector(x,y);
      metric_space_elements.push_back(
         dynamic_cast<genvector*>(curr_point_ptr));
      cout << "n = " << n 
           << " element.x = " << metric_space_elements.back()->get(0)
           << " element.y = " << metric_space_elements.back()->get(1)
           << endl;
   }

   vptree* vptree_ptr=new vptree;
   vptree_ptr->construct_tree(metric_space_elements);

   cout << "VP tree = " << endl;
   vptree_ptr->print_vp_tree();

   while (true)
   {
      double X;
      cout << "Enter x coordinate:" << endl;
      cin >> X;
      double Y;
      cout << "Enter y coordinate:" << endl;
      cin >> Y;

      genvector* query_point_ptr=dynamic_cast<genvector*>(new twovector(X,Y));
      
/*
      genvector* closest_node_payload_ptr=
         vptree_ptr->find_closest_node(query_point_ptr);
      cout << "Payload lying closest query point: X = " 
           << closest_node_payload_ptr->get(0) << endl;
      cout << "Distance of query point to closest node = " 
           << vptree_ptr->get_tau() << endl;
*/

//      int k=-1;
      int k=2;
      vector<int> nearest_neighbor_node_IDs;
      vector<double> query_to_neighbor_distances;
      vector<genvector*> metric_space_element_ptrs;

      vptree_ptr->incrementally_find_nearest_nodes(
         k,query_point_ptr,nearest_neighbor_node_IDs,
         query_to_neighbor_distances,metric_space_element_ptrs);

      for (int n=0; n<nearest_neighbor_node_IDs.size(); n++)
      {
         cout << "n = " << n 
              << " nearest neighbor node ID = "
              << nearest_neighbor_node_IDs[n] 
              << " query_to_node distance = "
              << query_to_neighbor_distances[n]
              << " X = " << metric_space_element_ptrs[n]->get(0)
              << endl;
      }
      
   }
   

/*
   int starting_node_ID;
   cout << "Enter starting node ID:" << endl;
   cin >> starting_node_ID;

   vector<tree_func::BTreeNode*> DescendantNode_ptrs=
      BinaryTree_ptr->get_descendant_node_ptrs(starting_node_ID);
   for (int i=0; i<DescendantNode_ptrs.size(); i++)
   {
      cout << "Descendant node ID = "
           << DescendantNode_ptrs[i]->get_ID() << endl;
   }
*/
 

/*
// Convert Vantage point binary tree into a graph:

   graph* graph_ptr=new graph();

   map<int,tree_func::BTreeNode* >::iterator iter;
   for (iter=BinaryTree_ptr->get_BinaryTreeNodes_map_ptr()->begin(); 
        iter != BinaryTree_ptr->get_BinaryTreeNodes_map_ptr()->end(); 
        iter++)
   {
      tree_func::BTreeNode* BTnode_ptr=iter->second;
      int node_ID=BTnode_ptr->get_ID();
      
      node* node_ptr=new node(node_ID);
      node_ptr->set_Uposn(BTnode_ptr->get_gx());
      node_ptr->set_Vposn(BTnode_ptr->get_gy());

      graph_ptr->add_node(node_ptr);
      tree_func::BTreeNode* BT_parent_node_ptr=BinaryTree_ptr->
         get_parent_node_ptr(node_ID);
      if (BT_parent_node_ptr != NULL)
      {
         int parent_node_ID=BT_parent_node_ptr->get_ID();
         if (parent_node_ID >= 0)
         {
            graph_ptr->add_graph_edge(node_ID,parent_node_ID);
            node_ptr->set_parent_ID(parent_node_ID);
            node* parent_node_ptr=graph_ptr->get_node_ptr(parent_node_ID);
            parent_node_ptr->get_children_node_IDs().push_back(
               node_ID);
         }
      }

// Generate PNG file with current BTreeNode information:

      vector<string> text_lines=tree_func::extract_vp_node_info(BTnode_ptr);

      string subdir="./node_png_images/";
      filefunc::dircreate(subdir);
      string png_filename=subdir+"node_"+
         stringfunc::integer_to_string(node_ID,3)+".png";
      cout << "png_filename = " << png_filename << endl;

      double background_greyscale_intensity=0.33;	
      colorfunc::Color text_color=colorfunc::red;
      pngfunc::convert_textlines_to_PNG(
         text_lines,png_filename,text_color,
         background_greyscale_intensity);
   } // iterator loop over BinaryTreeNodes

   cout << "*graph_ptr = " << *graph_ptr << endl;
*/

} 

