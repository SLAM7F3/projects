// ==========================================================================
// dtree_node_data class member function definitions
// ==========================================================================
// Last modified on 8/9/15; 8/14/15
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "classification/dtree_node_data.h"

using std::cin;
using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void dtree_node_data::allocate_member_objects()
{
}		       

void dtree_node_data::initialize_member_objects()
{
   classification_value="";
}		       

dtree_node_data::dtree_node_data(int id)
{
   allocate_member_objects();
   initialize_member_objects();

   ID = id;
}

// Copy constructor:

/*
dtree_node_data::dtree_node_data(const dtree_node_data& dt)
{
   docopy(dt);
}
*/

// ---------------------------------------------------------------------
dtree_node_data::~dtree_node_data()
{
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const dtree_node_data& node)
{
   outstream << endl;
   outstream << "Tree node " << node.get_ID() << endl;
   return outstream;
}

// ==========================================================================

void dtree_node_data::set_training_example_IDs(const vector<int>& IDs)
{
   for(unsigned int i = 0; i < IDs.size(); i++)
   {
      training_example_IDs.push_back(IDs[i]);
   }
}

void dtree_node_data::set_active_feature_IDs(const vector<int>& IDs)
{
   for(unsigned int i = 0; i < IDs.size(); i++)
   {
      active_feature_IDs.push_back(IDs[i]);
   }
}
