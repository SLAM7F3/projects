// =========================================================================
// Graph_Edge class member function definitions
// =========================================================================
// Last modified on 2/14/10; 5/29/10; 2/16/11
// =========================================================================

#include <iostream>
#include "graphs/graph_edge.h"
#include "graphs/node.h"

using std::cout;
using std::endl;
using std::flush;
using std::ostream;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void graph_edge::allocate_member_objects()
{
}

void graph_edge::initialize_member_objects()
{
   directed_flag=false;
   weight=relative_size=1;

// Assign default grey coloring to graph edge:

   edge_RGB.first=0.5;
   edge_RGB.second=0.5;
   edge_RGB.third=0.5;
}		 

// ---------------------------------------------------------------------

graph_edge::graph_edge(node* n1_ptr,node* n2_ptr,int ID)
{
   allocate_member_objects();
   initialize_member_objects();
   node1_ptr=n1_ptr;
   node2_ptr=n2_ptr;
   this->ID=ID;

   node1_ptr->pushback_edge_ptr(this);
   node2_ptr->pushback_edge_ptr(this);
}

// ---------------------------------------------------------------------
// Copy constructor:

graph_edge::graph_edge(const graph_edge& e)
{
   docopy(e);
}

graph_edge::~graph_edge()
{
}

// ---------------------------------------------------------------------
void graph_edge::docopy(const graph_edge& e)
{
   directed_flag=e.directed_flag;
   ID=e.ID;
   weight=e.weight;
   centrality=e.centrality;
   relative_size=e.relative_size;
   hex_color=e.hex_color;
   edge_RGB=e.edge_RGB;
}

// Overload = operator:

graph_edge& graph_edge::operator= (const graph_edge& e)
{
   if (this==&e) return *this;
   docopy(e);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const graph_edge& e)
{
   outstream << endl;
   outstream << "Edge ID = " << e.ID << endl;
   outstream << "weight = " << e.weight << endl;
   outstream << "node1 ID = " << e.get_node1_ptr()->get_ID() << endl;
   outstream << "node2 ID = " << e.get_node2_ptr()->get_ID() << endl;
   return outstream;
}

// =========================================================================
// =========================================================================

void graph_edge::set_node_ptrs(node* n1_ptr,node* n2_ptr)
{
   node1_ptr=n1_ptr;
   node2_ptr=n2_ptr;
}

node* graph_edge::get_node1_ptr() 
{
   return node1_ptr;
}

const node* graph_edge::get_node1_ptr() const
{
   return node1_ptr;
}

node* graph_edge::get_node2_ptr() 
{
   return node2_ptr;
}

const node* graph_edge::get_node2_ptr() const
{
   return node2_ptr;
}

// Member function get_centrality() implements a poor-man's
// edge-centrality which we simply set equal to the summed
// centralities of *node1_ptr and *node2_ptr.  

double graph_edge::get_centrality() const
{
   return node1_ptr->get_centrality() + node2_ptr->get_centrality();
}


