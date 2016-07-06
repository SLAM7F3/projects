// ==========================================================================
// Program scenegraph
// ==========================================================================
// Last updated on 11/7/06
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "datastructures/Tree.h"
//#include "datastructures/Treenode.h"

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
   
//   typedef int data_type;
   typedef string data_type;

   Tree<data_type>* tree_ptr=new Tree<data_type>;

   Treenode<data_type>* root_ptr=tree_ptr->get_root_ptr();
   root_ptr->set_data("root");

   Treenode<data_type>* node00_ptr=tree_ptr->addChild(root_ptr->get_ID());
   node00_ptr->set_data("LOD");
   Treenode<data_type>* node000_ptr=tree_ptr->addChild(node00_ptr->get_ID());
   node000_ptr->set_data("Geode");
   Treenode<data_type>* node001_ptr=tree_ptr->addChild(node00_ptr->get_ID());
   node001_ptr->set_data("Node");

   Treenode<data_type>* node01_ptr=tree_ptr->addChild(root_ptr->get_ID());
   node01_ptr->set_data("MatrixTransform");
   Treenode<data_type>* node010_ptr=tree_ptr->addChild(node01_ptr->get_ID());
   node010_ptr->set_data("Geode");
   Treenode<data_type>* node011_ptr=tree_ptr->addChild(node01_ptr->get_ID());
   node011_ptr->set_data("Node");
   Treenode<data_type>* node012_ptr=tree_ptr->addChild(node01_ptr->get_ID());
   node012_ptr->set_data("Geode");

   Treenode<data_type>* node02_ptr=tree_ptr->addChild(root_ptr->get_ID());
   node02_ptr->set_data("PageLOD");
   Treenode<data_type>* node020_ptr=tree_ptr->addChild(node02_ptr->get_ID());
   node020_ptr->set_data("geode");


   vector<int> tot_indices;
   tot_indices.push_back(0);
   tot_indices.push_back(2);
   tot_indices.push_back(1);
   Treenode<data_type>* node_021_ptr=tree_ptr->addChild(tot_indices);


   tot_indices.clear();
   tot_indices.push_back(0);
   tot_indices.push_back(1);
   tot_indices.push_back(1);
   tot_indices.push_back(0);
   Treenode<data_type>* node_0110_ptr=tree_ptr->addChild(tot_indices);   


/*
   vector<Treenode<data_type>* > L0=tree_ptr->retrieve_nodes_on_level(0);
   vector<Treenode<data_type>* > L1=tree_ptr->retrieve_nodes_on_level(1);
   vector<Treenode<data_type>* > L2=tree_ptr->retrieve_nodes_on_level(2);

   cout << "L0.size = " << L0.size() << endl;
   cout << "L1.size = " << L1.size() << endl;
   cout << "L2.size = " << L2.size() << endl;

   cout << "Number nodes on level 0 = " << tree_ptr->number_nodes_on_level(0)
        << endl;
   cout << "Number nodes on level 1 = " << tree_ptr->number_nodes_on_level(1)
        << endl;
   cout << "Number nodes on level 2 = " << tree_ptr->number_nodes_on_level(2)
        << endl;
   cout << "tree_ptr->get_n_levels() = " << tree_ptr->get_n_levels() << endl;

   tree_ptr->compute_columns_for_nodes_on_level(0);
   tree_ptr->compute_columns_for_nodes_on_level(1);
   tree_ptr->compute_columns_for_nodes_on_level(2);
*/

   for (int l=0; l<=tree_ptr->get_n_levels(); l++)
   {
      tree_ptr->compute_columns_for_nodes_on_level(l);
      for (int c=0; c<tree_ptr->number_nodes_on_level(l); c++)
      {
         string banner="Level = "+stringfunc::number_to_string(l)
            +" Column = "+stringfunc::number_to_string(c);
         outputfunc::write_banner(banner);
         Treenode<data_type>* currnode_ptr=tree_ptr->get_node(l,c);
         std::cout << "currnode = " << *currnode_ptr << std::endl;
      }
   } // loop over index l labeling tree levels

/*   
   vector<int> tot_indices;
   tot_indices.push_back(0);
   tot_indices.push_back(2);
   tot_indices.push_back(1);
   
   tree_ptr->addChild(tot_indices);

   Treenode<data_type>* new_treenode_ptr=
      tree_ptr->get_total_indices_labeled_Treenode_ptr(tot_indices);

   if (new_treenode_ptr==NULL)
   {
      cout << "new_treenode_ptr = NULL" << endl;
   }
   else
   {
      cout << "*new_treenode_ptr = "
           << *new_treenode_ptr << endl;
   }
*/
 

}

