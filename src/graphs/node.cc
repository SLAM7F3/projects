// =========================================================================
// Node class member function definitions
// =========================================================================
// Last modified on 7/29/11; 2/27/12; 4/5/14; 12/1/15
// =========================================================================

#include <iostream>
#include "color/colorfuncs.h"
#include "math/constants.h"
#include "graphs/graph_edge.h"
#include "graphs/node.h"
#include "templates/mytemplates.h"

using std::cout;
using std::endl;
using std::flush;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void node::allocate_member_objects()
{
}

void node::initialize_member_objects()
{
   data_ID=source_ID=parent_ID=color_ID=representative_child_ID=-1;
   level=0;
   Uposn=Vposn=NEGATIVEINFINITY;
   U2posn=V2posn=NEGATIVEINFINITY;
   degree=0;
//   centrality=-1;
   centrality=0;	// FAKE FAKE:  Sun July 1, 2012 at 9:24 am
   distance_from_start=-1;
   relative_size=1.0;
   visited_status=unvisited;
   path_predecessor_ptr=NULL;
   graph_ptr=NULL;

// Assign default grey coloring to node:

   node_RGB.first=0.5;
   node_RGB.second=0.5;
   node_RGB.third=0.5;
}		 

// ---------------------------------------------------------------------
node::node()
{
//   cout << "inside node constructor #1" << endl;
   allocate_member_objects();
   initialize_member_objects();

   ID=-1;
}

// ---------------------------------------------------------------------
node::node(int ID,int level)
{
//   cout << "inside node constructor #2" << endl;
   allocate_member_objects();
   initialize_member_objects();
   this->ID=ID;
   this->level=level;

// We initially set data_ID equal to node's ID.  This equality needs
// to be later altered for parent, grandparent, etc nodes...

   this->data_ID=ID;
}

// ---------------------------------------------------------------------
// Copy constructor:

node::node(const node& n)
{
//   cout << "inside node copy constructor" << endl;
   docopy(n);
}

node::~node()
{
//   cout << "inside node destructor" << endl;
}

// ---------------------------------------------------------------------
void node::docopy(const node& n)
{
//   cout << "inside node::docopy()" << endl;
   
   ID=n.ID;
   data_ID=n.data_ID;
   level=n.level;
   source_ID=n.source_ID;
   parent_ID=n.parent_ID;
   color_ID=n.color_ID;
   representative_child_ID=n.representative_child_ID;

   Uposn=n.Uposn;
   Vposn=n.Vposn;
   U2posn=n.U2posn;
   V2posn=n.V2posn;
   degree=n.degree;
   centrality=n.centrality;
   distance_from_start=n.distance_from_start;
   relative_size=n.relative_size;
   node_RGB=n.node_RGB;

   neighboring_node_IDs=n.neighboring_node_IDs;
   children_node_IDs=n.children_node_IDs;

   graph_ptr=n.graph_ptr;

// Note added on 2/13/10: somehow need to copy over
// graph-edge_ptrs STL vector...

}

// Overload = operator:

node& node::operator= (const node& n)
{
   if (this==&n) return *this;
   docopy(n);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const node& n)
{
   outstream << endl;
   outstream << "ID = " << n.ID << endl;
   outstream << "graph_ptr = " << n.graph_ptr << endl;
//   outstream << "color_ID = " << n.color_ID << endl;
   outstream << "posn = " << n.Uposn << " , " << n.Vposn << endl;
   outstream << "posn2 = " << n.U2posn << " , " << n.V2posn << endl;
//   outstream << "degree = " << n.degree << endl;
//   outstream << "centrality = " << n.centrality << endl;
   outstream << "rel size = " << n.relative_size << endl;
//   outstream << "node_RGB = " << n.node_RGB << endl;
//   outstream << "distance from start = " << n.get_distance_from_start()
//             << endl;

//   outstream << "source_ID = " << n.source_ID << endl;   
   outstream << "parent_ID = " << n.parent_ID << endl;   
//   outstream << "children node IDs = " << endl;
//   templatefunc::printVector(n.children_node_IDs);
   outstream << "Neighbor node IDs = " << endl;
   templatefunc::printVector(n.get_neighbor_node_IDs());

   return outstream;
}

// =========================================================================
// Set and get member functions
// =========================================================================

void node::set_hex_color(string hc)
{
//   cout << "inside node::set_hex_color()" << endl;
   hex_color=hc;
   colorfunc::RRGGBB_hex_to_RGB(hc,node_RGB);

//   cout << "node_RGB.first = " << node_RGB.first
//        << " node_RGB.second = " << node_RGB.second
//        << " node_RGB.third = " << node_RGB.third << endl;
}

string node::get_hex_color() const
{
   return hex_color;
}

void node::set_label(string label)
{
//   cout << "inside node::set_label()" << endl;
   this->label=label;
}

string node::get_label() const
{
   return label;
}

// =========================================================================
// Node neighbor member functions
// =========================================================================

void node::pushback_edge_ptr(graph_edge* e_ptr)
{
//   cout << "inside node::pushback_edge_ptr()" << endl;
   
   graph_edge_ptrs.push_back(e_ptr);
   update_degree();

   int n1_ID=e_ptr->get_node1_ptr()->get_ID();
   int n2_ID=e_ptr->get_node2_ptr()->get_ID();
//   cout << "get_ID() = " << get_ID() 
//        << " n1_ID = " << n1_ID << " n2_ID = " << n2_ID << endl;

   if (get_ID() != n1_ID)
   {
      neighboring_node_IDs[n1_ID]=e_ptr->get_weight();
   }
   else if (get_ID() != n2_ID)
   {
      neighboring_node_IDs[n2_ID]=e_ptr->get_weight();
   }

//   cout << "neighboring_node_IDs.size() = "
//        << neighboring_node_IDs.size() << endl;
}

// ---------------------------------------------------------------------
void node::update_degree()
{
   degree=0;
   for (unsigned int i=0; i<graph_edge_ptrs.size(); i++)
   {
      graph_edge* curr_graph_edge_ptr=graph_edge_ptrs[i];
      degree += curr_graph_edge_ptr->get_weight();
   }
}

// ---------------------------------------------------------------------
// Member function get_neighbor_node_ID() takes in graph_edge *e_ptr.
// It returns the ID for the node neighboring *this:

int node::get_neighbor_node_ID(graph_edge* e_ptr) const
{
   int n1_ID=e_ptr->get_node1_ptr()->get_ID();
   int n2_ID=e_ptr->get_node2_ptr()->get_ID();

   if (get_ID() != n1_ID)
   {
      return n1_ID;
   }
   else // if (get_ID() != n2_ID)
   {
      return n2_ID;
   }
}

// ---------------------------------------------------------------------
// Member function is_neighboring_node() returns true if input node_ID
// corresponds to a neighbor of *this:

bool node::is_neighboring_node(int node_ID) 
{
   NEIGHBORS_MAP::iterator iter=neighboring_node_IDs.find(node_ID);      
   return (iter != neighboring_node_IDs.end());
}

graph_edge* node::get_graph_edge_ptr(int neighboring_node_ID)
{
   if (!is_neighboring_node(neighboring_node_ID)) return NULL;

   graph_edge* graph_edge_ptr=NULL;
   for (unsigned int i=0; i<graph_edge_ptrs.size(); i++)
   {
      graph_edge* curr_edge_ptr=graph_edge_ptrs[i];
      node* n1_ptr=curr_edge_ptr->get_node1_ptr();
      node* n2_ptr=curr_edge_ptr->get_node2_ptr();
      int n1_ID=n1_ptr->get_ID();
      int n2_ID=n2_ptr->get_ID();

      if ( 
         (n1_ID==get_ID() && n2_ID==neighboring_node_ID) ||
         (n2_ID==get_ID() && n1_ID==neighboring_node_ID))
      {
         graph_edge_ptr=curr_edge_ptr;
         break;
      }
   } // loop over index i labeling graph edges

   return graph_edge_ptr;
}

// ---------------------------------------------------------------------
// Member function get_neighbor_node_IDs() returns an STL vector
// containing IDs for neighboring nodes:

vector<int> node::get_neighbor_node_IDs() const
{
//   cout << "inside node::get_neighbor_node_IDs()" << endl;
//   cout << "get_ID() = " << get_ID() << endl;
   
   vector<int> neighbor_IDs;
   for (NEIGHBORS_MAP::const_iterator iter=neighboring_node_IDs.begin();
        iter != neighboring_node_IDs.end(); iter++)
   {
      neighbor_IDs.push_back(iter->first);
   }
   return neighbor_IDs;
}

// ---------------------------------------------------------------------
// Member function get_sorted_neighbor_node_IDs() returns an STL
// vector containing IDs for neighboring nodes sorted in decreasing
// order according to edge weight.

vector<int> node::get_sorted_neighbor_node_IDs() const
{
//   cout << "inside node::get_sorted_neighbor_node_IDs()" << endl;
   
   vector<int> neighbor_IDs;
   vector<double> edge_weights;
   for (NEIGHBORS_MAP::const_iterator iter=neighboring_node_IDs.begin();
        iter != neighboring_node_IDs.end(); iter++)
   {
      neighbor_IDs.push_back(iter->first);
      edge_weights.push_back(iter->second);
   }
   templatefunc::Quicksort_descending(edge_weights, neighbor_IDs);

   return neighbor_IDs;
}

// ---------------------------------------------------------------------
// Member function get_n_neighbors() returns the number of nodes which
// are immediate neighbors to *this.

int node::get_n_neighbors() const
{
   return neighboring_node_IDs.size();
}

// =========================================================================
// Node coloring member functions
// =========================================================================

// Member function set_pure_hue_color()

void node::set_pure_hue_color(double hue)
{
//   cout << "inside node::set_pure_hue_color()" << endl;

   set_node_RGB(colorfunc::get_RGB_values(colorfunc::grey));

   double saturation=1.0;
   double value=1.0;

   colorfunc::HSV curr_hsv;
   curr_hsv.first=hue;
   curr_hsv.second=saturation;
   curr_hsv.third=value;
   set_node_RGB(colorfunc::hsv_to_RGB(curr_hsv));
}

// =========================================================================
// A* search member functions
// =========================================================================

bool node::IsGoal(node& nodeGoal )
{
//   cout << "inside node::IsGoal, curr_ID = " << get_ID()
//        << " goal ID = " << nodeGoal.get_ID() << endl;
   return (get_ID()==nodeGoal.get_ID());
}

// ---------------------------------------------------------------------
bool node::IsSameState( node& rhs )
{
//   cout << "inside node::IsSameState, curr_ID = " << get_ID()
//        << " rhs.get_ID = " << rhs.get_ID() << endl;
   return (get_ID()==rhs.get_ID());
}

// ---------------------------------------------------------------------
// Member function GetCost() evaluates a dimensionless cost function
// to a candidate successor location.

double node::GetCost( node& successor )
{
//   cout << "inside node::GetCost()" << endl;
//   cout << "this node ID = " << get_ID() 
//        << "successor.ID = " << successor.get_ID() << endl;
//   cout << "&successor = " << &successor << endl;
//   cout << "successor = " << successor << endl;
//   cout << "graph_ptr = " << graph_ptr << endl;

   graph_edge* edge_ptr=graph_ptr->get_edge_ptr(get_ID(),successor.get_ID());
   double edge_weight=edge_ptr->get_weight();

   double step_cost=POSITIVEINFINITY;
   if (edge_weight > 0)
   {
      step_cost=1.0/edge_weight;
   }
//   cout << "Step_cost = " << step_cost << endl;
   return step_cost;
}

// ---------------------------------------------------------------------
// Here's the heuristic function that supplies a lower bound for the
// path integral of the cost function from the current Node to the
// Goal.

double node::GoalDistanceEstimate( node& nodeGoal )
{
//   cout << "inside node::GoalDistanceEstimate(), nodeGoal.id = "
//        << nodeGoal.get_ID() << endl;
   double goal_cost=0;
   return goal_cost;
}

// ---------------------------------------------------------------------
// This generates the successors to the given Node. It uses a helper
// function called AddSuccessor to give the successors to the AStar
// class. The A* specific initialization is done for each node
// internally, so here you just set the state information that is
// specific to the application

bool node::GetSuccessors( 
   AStarSearch<node>* astarsearch, node* parent_node_ptr )
{
//   cout << "inside node::GetSuccessors()" << endl;
//   cout << "get_ID() = " << get_ID() << endl;
   
   int parent_ID = -1; 
   if ( parent_node_ptr != NULL )
   {
      parent_ID = parent_node_ptr->get_ID();
   }
//   cout << "parent_ID = " << parent_ID << endl;

// Push each possible move except allowing the search to go backwards
	
//   cout << "n_neighbors = " << get_n_neighbors() << endl;

   node NewNode;
   vector<int> neighbor_IDs=get_neighbor_node_IDs();
//   cout << "neighbor_IDs.size() = " << neighbor_IDs.size() << endl;
   for (unsigned int i=0; i<neighbor_IDs.size(); i++)
   {
      int neighbor_node_ID=neighbor_IDs[i];
//      cout << "neighbor_node_ID = " << neighbor_node_ID << endl;
      if (parent_ID == neighbor_node_ID) continue;
//      cout << "graph_ptr = " << graph_ptr << endl;
      
      node* neighbor_node_ptr=graph_ptr->get_node_ptr(neighbor_node_ID);
//      cout << "neighbor_node_ptr = " << neighbor_node_ptr << endl;
//      cout << "*neighbor_node_ptr = " << *neighbor_node_ptr << endl;
      NewNode = node( *neighbor_node_ptr);

      graph_edge* curr_edge_ptr=graph_ptr->get_edge_ptr(
         get_ID(),neighbor_node_ID);
//      cout << "curr_edge_ptr = " << curr_edge_ptr << endl;
      NewNode.pushback_edge_ptr(curr_edge_ptr);
      NewNode.set_graph_ptr(graph_ptr);

      astarsearch->AddSuccessor( NewNode );
   } // loop over index i labeling neighbor node IDs

   return true;
}
