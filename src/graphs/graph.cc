// =========================================================================
// Graph class member function definitions
// =========================================================================
// Last modified on 6/20/13; 4/5/14; 8/20/16; 8/22/16
// =========================================================================

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "math/genmatrix.h"
#include "graphs/graph.h"
#include "graphs/graph_edge.h"
#include "graphs/graphdbfuncs.h"
#include "graphs/graphfuncs.h"
#include "graphs/node.h"
#include "general/outputfuncs.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/twovector.h"
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ofstream;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void graph::allocate_member_objects()
{
}

void graph::initialize_member_objects()
{
   n_distinct_colors=-1;
   parent_identity=-1;
   graph_edge_counter=0;
   gxgy_offset=twovector(0,0);
   graph_edge_counter_ptr=NULL;
   adjacency_matrix_ptr=NULL;
   nodes_kdtree_ptr=NULL;
}		 

// ---------------------------------------------------------------------
graph::graph(int ID,int level)
{
   allocate_member_objects();
   initialize_member_objects();
   this->ID=ID;
   this->level=level;
}

// ---------------------------------------------------------------------
// Copy constructor:

graph::graph(const graph& g)
{
   docopy(g);
}

graph::~graph()
{
//   cout << "inside graph destructor" << endl;
   delete adjacency_matrix_ptr;
   delete nodes_kdtree_ptr;
   purge_graph_edges();
   purge_nodes();
}

// ---------------------------------------------------------------------
void graph::docopy(const graph& g)
{
   ID=g.ID;
   level=g.level;
   parent_identity=g.parent_identity;
   n_distinct_colors=g.n_distinct_colors;
   graph_edge_counter=g.graph_edge_counter;
   graph_edge_counter_ptr=g.graph_edge_counter_ptr;

   child_IDs.clear();
   for (unsigned int i=0; i<g.child_IDs.size(); i++)
   {
      child_IDs.push_back(g.child_IDs[i]);
   }

   node_order.clear();
   for (unsigned int i=0; i<g.node_order.size(); i++)
   {
      node_order.push_back(g.node_order[i]);
   }

   graph_edge_order.clear();
   for (unsigned int i=0; i<g.graph_edge_order.size(); i++)
   {
      graph_edge_order.push_back(g.graph_edge_order[i]);
   }

   if (g.adjacency_matrix_ptr != NULL)
   {
      delete adjacency_matrix_ptr;
      adjacency_matrix_ptr=new genmatrix(*(g.adjacency_matrix_ptr));
   }

   nodes_map=g.nodes_map;
   graph_edges_map=g.graph_edges_map;
   nodes_edge_map=g.nodes_edge_map;
   clusters_map=g.clusters_map;
   edge_weights_threshold=g.edge_weights_threshold;

   nodes_kdtree_ptr=NULL;
   generate_nodes_kdtree();
}

// Overload = operator:

graph& graph::operator= (const graph& g)
{
   if (this==&g) return *this;
   docopy(g);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const graph& g)
{
   outstream << endl;
//   outstream << "ID = " << g.ID << endl;
//   outstream << "level = " << g.level << endl;
//   outstream << "Graph parent identity = " << g.parent_identity << endl;
//   outstream << "n_children = " << g.get_n_children() << endl;
   outstream << "n_nodes = " << g.get_n_nodes() << endl;
   outstream << endl;

   outstream << ".............................................." << endl;
   outstream << "Graph nodes:" << endl;
   
   for (unsigned int n=0; n<g.get_n_nodes(); n++)
   {
      const node* curr_node_ptr=g.get_ordered_node_ptr(n);

      outstream << "curr node ID = " << curr_node_ptr->get_ID() 
                << " curr_node level = " << curr_node_ptr->get_level() 
                << endl;
//      outstream << "curr_node_ptr = " << curr_node_ptr << endl;
      outstream << "curr node's parent ID = " << curr_node_ptr->
         get_parent_ID() << endl;
      outstream << "n_children for curr node = "
                << curr_node_ptr->get_n_children() << endl;
      vector<int> children_node_IDs=curr_node_ptr->get_children_node_IDs();
      outstream << "Children node IDs: " << endl;
      for (unsigned int c=0; c<children_node_IDs.size(); c++)
      {
         outstream << children_node_IDs[c] << " ";
      }
      outstream << endl;

//      outstream << " *curr_node_ptr = " << *curr_node_ptr << endl;
      outstream << endl;
   }

   return outstream;
}

// =========================================================================
// Node manipulation member functions
// =========================================================================

void graph::add_node(node* node_ptr)
{
//   cout << "inside graph::add_node()" << endl;
   int node_ID=node_ptr->get_ID();
   node_order.push_back(node_ID);

   NODES_MAP::iterator iter=nodes_map.find(node_ID);      
   if (iter == nodes_map.end())
   {
      nodes_map[node_ID]=node_ptr;
   }
   else
   {
      iter->second=node_ptr;
   }
//   cout << "get_n_nodes() = " << get_n_nodes() << endl;

// Set graph_ptr member of node_ptr to this:

   node_ptr->set_graph_ptr(this);
}

// ---------------------------------------------------------------------
// Member function erase_node() removes the input node from nodes_map.
// However, it does NOT call the node's destructor.  We wrote this
// method in June 2012 when we realized that
// photogroup::destroy_single_photograph() was previously trying to
// call a node's destructor twice.  Recall that the photograph class
// inherits from the node class.  So calling delete photo_ptr
// automatically calls the node destructor.

void graph::erase_node(node* node_ptr)
{
//   cout << "inside graph::erase_node()" << endl;
//   cout << "nodes_map.size() = " << nodes_map.size() << endl;

   if (node_ptr==NULL) return;
//   cout << "node_ptr = " << node_ptr << endl;

// Need to eliminate entry from nodes_map corresponding to node_ID

   int node_ID=node_ptr->get_ID();
//   cout << "node_ID = " << node_ID << endl;

   NODES_MAP::iterator iter=nodes_map.find(node_ID);      
   if (iter != nodes_map.end())
   {
      nodes_map.erase(iter);
   }
}

// ---------------------------------------------------------------------
void graph::delete_node(node* node_ptr)
{
//   cout << "inside graph::delete_node()" << endl;

   if (node_ptr==NULL) return;
   erase_node(node_ptr);
   delete node_ptr;
}

// ---------------------------------------------------------------------
void graph::purge_nodes()
{
   unsigned int n_nodes=get_n_nodes();
   for (unsigned int n=0; n<n_nodes; n++)
   {
      node* curr_node_ptr=get_ordered_node_ptr(n);
      delete_node(curr_node_ptr);
   }

   nodes_map.clear();
   node_order.clear();
}

// ---------------------------------------------------------------------
node* graph::get_ordered_node_ptr(int p) 
{
   return get_node_ptr(node_order[p]);
}

const node* graph::get_ordered_node_ptr(int p) const
{
//   cout << "inside graph::get_ordered_node_ptr()" << endl;
   return get_node_ptr(node_order[p]);
}

// ---------------------------------------------------------------------
graph_edge* graph::get_ordered_graph_edge_ptr(int p) 
{
   return get_graph_edge_ptr(graph_edge_order[p]);
}

const graph_edge* graph::get_ordered_graph_edge_ptr(int p) const
{
   return get_graph_edge_ptr(graph_edge_order[p]);
}

// ---------------------------------------------------------------------
// Member function reorder_nonnull_ptr_nodes() iterates over all
// entries within STL map member nodes_map.  It ignores any
// null-valued node pointers.  The remaining IDs for the non-null
// nodes are sorted and set equal to member node_order.

void graph::reorder_nonnull_ptr_nodes()
{
//   cout << "inside graph::reorder_nonnull_ptr_nodes()" << endl;
//   cout << "get_max_node_ID() = " << get_max_node_ID() << endl;

   vector<int> ordered_node_ID;
   ordered_node_ID.reserve(get_max_node_ID());

   int n=0;
   for (NODES_MAP::iterator iter=nodes_map.begin();
        iter != nodes_map.end(); ++iter)
   {
      if (n%1000==0) cout << n << " " << flush;
      n++;
      
      node* curr_node_ptr=iter->second;
      if (curr_node_ptr==NULL) 
      {
//         cout << "No node in graph with ID = " << n << endl;
         continue;
      }
      ordered_node_ID.push_back(curr_node_ptr->get_ID());
   }
   std::sort(ordered_node_ID.begin(),ordered_node_ID.end());
   reset_node_order(ordered_node_ID);
}

// ---------------------------------------------------------------------
void graph::reset_node_order(const vector<int>& ordered_node_ID)
{
   node_order.clear();
   for (unsigned int n=0; n<ordered_node_ID.size(); n++)
   {
      node_order.push_back(ordered_node_ID[n]);
   }
}

// ---------------------------------------------------------------------
bool graph::node_in_graph(int node_ID) const
{
   NODES_MAP::const_iterator iter=nodes_map.find(node_ID);      
   return (iter != nodes_map.end());
}

// ---------------------------------------------------------------------
// Recall that first node's ID does not necessarily equal 0.  So
// member function get_first_node_ID() searches for and returns the
// smallest, nonnegative ID which corresponds to a node in *this.

int graph::get_first_node_ID() const
{
//   cout << "inside graph::first_node_ID()" << endl;
   if (get_n_nodes()==0) return -1;

   int n_start=0;
   while (!node_in_graph(n_start))
   {
      n_start++;
   }
//   cout << "n_start = " << n_start << endl;
   return n_start;
}

// ---------------------------------------------------------------------
// Member function get_max_node_ID() loops over all nodes within the
// current graph.  It returns the maximum of all the nodes' IDs.

int graph::get_max_node_ID() const
{
//   cout << "inside graph::get_max_node_ID()" << endl;

   int max_node_ID=-1;
   for (unsigned int n=0; n<get_n_nodes(); n++)
   {
      const node* curr_node_ptr=get_ordered_node_ptr(n);
      max_node_ID=basic_math::max(max_node_ID,curr_node_ptr->get_ID());
   }

   return max_node_ID;
}

int graph::get_min_node_ID() const
{
//   cout << "inside graph::get_min_node_ID()" << endl;

   int min_node_ID=POSITIVEINFINITY;
   for (unsigned int n=0; n<get_n_nodes(); n++)
   {
      const node* curr_node_ptr=get_ordered_node_ptr(n);
      min_node_ID=basic_math::min(min_node_ID,curr_node_ptr->get_ID());
   }

   return min_node_ID;
}

// ---------------------------------------------------------------------
// Member function get_max_edge_ID() loops over all edges within the
// current graph.  It returns the maximum of all the edges' IDs.

int graph::get_max_edge_ID() const
{
//   cout << "inside graph::get_max_edge_ID()" << endl;

   int max_edge_ID=-1;

   for (unsigned int n=0; n<get_n_graph_edges(); n++)
   {
      const graph_edge* curr_graph_edge_ptr=get_ordered_graph_edge_ptr(n);
      max_edge_ID=basic_math::max(max_edge_ID,curr_graph_edge_ptr->get_ID());
   }
   return max_edge_ID;
}

// ---------------------------------------------------------------------
// Member function get_adjacent_edge_IDs() takes in an STL vector
// containing node IDs.  It retrieves the IDs for all these nodes'
// edges.  This method returns an STL vector containing the ID for
// each adjacent edge listed just once.

vector<int> graph::get_adjacent_edge_IDs(const vector<int>& node_IDs)
{
   vector<int> graph_edge_IDs;
   for (unsigned int n=0; n<node_IDs.size(); n++)
   {
      node* curr_node_ptr=get_node_ptr(node_IDs[n]);
      vector<graph_edge*> graph_edge_ptrs=
         curr_node_ptr->get_graph_edge_ptrs();
      for (unsigned int e=0; e<graph_edge_ptrs.size(); e++)
      {
         int curr_edge_ID=graph_edge_ptrs[e]->get_ID();
         graph_edge_IDs.push_back(curr_edge_ID);
      } // loop over index e labeling graph edges
   } // loop over index n

   std::sort(graph_edge_IDs.begin(),graph_edge_IDs.end());
   
   int prev_edge_ID=-1;
   vector<int> unique_edge_IDs;
   for (unsigned int i=0; i<graph_edge_IDs.size(); i++)
   {
      int curr_edge_ID=graph_edge_IDs[i];
      if (curr_edge_ID==prev_edge_ID) continue;
      prev_edge_ID=curr_edge_ID;
      unique_edge_IDs.push_back(curr_edge_ID);
   }

   return unique_edge_IDs;
}

// =========================================================================
// Graph_Edge manipulation member functions
// =========================================================================

graph_edge* graph::add_graph_edge(int node1_ID,int node2_ID,double weight)
{
//   cout << "inside graph::add_graph_edge() #1" << endl;
//   cout << "node1_ID = " << node1_ID 
//        << " node2_ID = " << node2_ID 
//        << " weight = " << weight << endl;
   return add_graph_edge(get_node_ptr(node1_ID),get_node_ptr(node2_ID),
                         weight);
}

graph_edge* graph::add_graph_edge(node* n1_ptr,node* n2_ptr,double weight)
{
//   cout << "inside graph::add_graph_edge() #2" << endl;
//   cout << "n1_ptr = " << n1_ptr
//        << " n2_ptr  = " << n2_ptr
//        << " weight = " << weight
//        << endl;
   int curr_graph_edge_ID=graph_edge_counter;
   if (graph_edge_counter_ptr != NULL)
   {
      curr_graph_edge_ID=*graph_edge_counter_ptr;
   }

   graph_edge* curr_graph_edge_ptr=new graph_edge(
      n1_ptr,n2_ptr,curr_graph_edge_ID);
   graph_edge_order.push_back(curr_graph_edge_ID);

   if (graph_edge_counter_ptr != NULL)
   {
      *graph_edge_counter_ptr = *graph_edge_counter_ptr+1;
   }
   else
   {
      graph_edge_counter++;
   }

   curr_graph_edge_ptr->set_weight(weight);

// Uncommented out next 2 lines on 7/29/11:

   n1_ptr->pushback_edge_ptr(curr_graph_edge_ptr);
   n2_ptr->pushback_edge_ptr(curr_graph_edge_ptr);

   add_graph_edge(curr_graph_edge_ptr);
   return curr_graph_edge_ptr;
}

void graph::add_graph_edge(graph_edge* graph_edge_ptr)
{
//   cout << "inside graph::add_graph_edge() #3" << endl;

   int graph_edge_ID=graph_edge_ptr->get_ID();
   GRAPH_EDGES_MAP::iterator iter=graph_edges_map.find(graph_edge_ID);
   if (iter == graph_edges_map.end())
   {
      graph_edges_map[graph_edge_ID]=graph_edge_ptr;
   }
   else
   {
      iter->second=graph_edge_ptr;
   }

   int node1_ID=graph_edge_ptr->get_node1_ptr()->get_ID();
   int node2_ID=graph_edge_ptr->get_node2_ptr()->get_ID();
   int n1=basic_math::min(node1_ID,node2_ID);
   int n2=basic_math::max(node1_ID,node2_ID);
   pair<int,int> p(n1,n2);
   nodes_edge_map[p]=graph_edge_ptr->get_ID();
}

// ---------------------------------------------------------------------
void graph::delete_graph_edge(graph_edge* graph_edge_ptr)
{
//   cout << "inside graph::delete_graph_edge()" << endl;

   if (graph_edge_ptr==NULL) return;

// Need to eliminate entry from graph_edges_map corresponding to graph_edge ID

   int graph_edge_ID=graph_edge_ptr->get_ID();
   GRAPH_EDGES_MAP::iterator iter=graph_edges_map.find(graph_edge_ID);      
   if (iter != graph_edges_map.end())
   {
      graph_edges_map.erase(iter);
   }
   
   delete graph_edge_ptr;
}

// ---------------------------------------------------------------------
void graph::purge_graph_edges()
{
//   cout << "inside graph::purge_graph_edges()" << endl;

   for (GRAPH_EDGES_MAP::iterator iter=graph_edges_map.begin();
        iter != graph_edges_map.end(); ++iter)
   {
      delete_graph_edge(iter->second);
   }
   graph_edges_map.clear();
}

// ---------------------------------------------------------------------
// Member function get_graph_edge_ptr() returns the graph_edge pointer
// corresponding to input ID.

graph_edge* graph::get_graph_edge_ptr(int graph_edge_ID)
{
   GRAPH_EDGES_MAP::iterator iter=graph_edges_map.find(graph_edge_ID);      
   if (iter == graph_edges_map.end())
   {
      return NULL;
   }
   else
   {
      return iter->second;
   }
}

const graph_edge* graph::get_graph_edge_ptr(int graph_edge_ID) const
{
   GRAPH_EDGES_MAP::const_iterator iter=graph_edges_map.find(graph_edge_ID);  
   if (iter == graph_edges_map.end())
   {
      return NULL;
   }
   else
   {
      return iter->second;
   }
}

// ---------------------------------------------------------------------
bool graph::graph_edge_in_graph(int graph_edge_ID) const
{
   GRAPH_EDGES_MAP::const_iterator iter=graph_edges_map.find(graph_edge_ID); 
   return (iter != graph_edges_map.end());
}

// ---------------------------------------------------------------------
// Member function get_edge_ID() takes in IDs for two nodes.  If the
// nodes' share an edge, this method returns its integer ID.
// Otherwise, -1 is returned.

int graph::get_edge_ID(int node1_ID,int node2_ID)
{
//   cout << "inside graph::get_edge_ID()" << endl;
//   cout << "node1_ID = " << node1_ID << " node2_ID = " << node2_ID << endl;

   int n1=basic_math::min(node1_ID,node2_ID);
   int n2=basic_math::max(node1_ID,node2_ID);

   pair<int,int> p(n1,n2);
   NODES_EDGE_MAP::iterator iter=nodes_edge_map.find(p);
   if (iter != nodes_edge_map.end())
   {
      return iter->second;
   }
   return -1;
}

// ---------------------------------------------------------------------
graph_edge* graph::get_edge_ptr(const node* node1_ptr,const node* node2_ptr)
{
//   cout << "inside graph::get_edge_ptr()" << endl;
//   cout << "node1_ptr->get_ID() = " << node1_ptr->get_ID() << endl;
//   cout << "node2_ptr->get_ID() = " << node2_ptr->get_ID() << endl;
   int edge_ID=get_edge_ID(node1_ptr->get_ID(),
                           node2_ptr->get_ID());
//   cout << "edge_ID = " << edge_ID << endl << endl;
   if (edge_ID < 0) return NULL;
   return get_graph_edge_ptr(edge_ID);
}

// ---------------------------------------------------------------------
graph_edge* graph::get_edge_ptr(int node1_ID,int node2_ID)
{
//   cout << "inside graph::get_edge_ptr()" << endl;
   int edge_ID=get_edge_ID(node1_ID,node2_ID);
//   cout << "edge_ID = " << edge_ID << endl << endl;
   if (edge_ID < 0) return NULL;
   return get_graph_edge_ptr(edge_ID);
}

// ---------------------------------------------------------------------
// Member function remove_graph_edge_ptr() searches graph_edges_map
// for the input graph_edge_ptr.  If it is found, this method erases
// that entry from member STL maps graph_edges_map and nodes_edge_map.

void graph::remove_graph_edge(graph_edge* graph_edge_ptr)
{
//   cout << "inside graph::remove_graph_graph_edge, *graph_edge_ptr = "
//        << *graph_edge_ptr << endl;

   int graph_edge_ID=graph_edge_ptr->get_ID();
   GRAPH_EDGES_MAP::iterator iter=graph_edges_map.find(graph_edge_ID);
   if (iter != graph_edges_map.end())
   {
      graph_edges_map.erase(iter);
   }

   int node1_ID=graph_edge_ptr->get_node1_ptr()->get_ID();
   int node2_ID=graph_edge_ptr->get_node2_ptr()->get_ID();
   pair<int,int> p(node1_ID,node2_ID);
   NODES_EDGE_MAP::iterator ne_iter=nodes_edge_map.find(p);
   if (ne_iter != nodes_edge_map.end())
   {
      nodes_edge_map.erase(ne_iter);
   }
}

// ---------------------------------------------------------------------
// Member function minimal_graph_edge_weight()

double graph::minimal_graph_edge_weight()
{
//   cout << "inside graph::minimal_graph_edge_weight()" << endl;

   double minimal_graph_edge_weight=POSITIVEINFINITY;
   for (GRAPH_EDGES_MAP::const_iterator iter=graph_edges_map.begin();
        iter != graph_edges_map.end(); iter++)
   {
      graph_edge* graph_edge_ptr=iter->second;
//      cout << "Graph_edge_ptr = " << graph_edge_ptr << endl;
      minimal_graph_edge_weight=basic_math::min(
         minimal_graph_edge_weight,graph_edge_ptr->get_weight());
   }
   return minimal_graph_edge_weight;
}

// ---------------------------------------------------------------------
// Member function maximal_graph_edge_weight()

double graph::maximal_graph_edge_weight()
{
//   cout << "inside graph::maximal_graph_edge_weight()" << endl;

   double maximal_graph_edge_weight=NEGATIVEINFINITY;
   for (GRAPH_EDGES_MAP::const_iterator iter=graph_edges_map.begin();
        iter != graph_edges_map.end(); iter++)
   {
      graph_edge* graph_edge_ptr=iter->second;
//      cout << "Graph_edge_ptr = " << graph_edge_ptr << endl;
      maximal_graph_edge_weight=basic_math::max(
         maximal_graph_edge_weight,graph_edge_ptr->get_weight());
   }
   return maximal_graph_edge_weight;
}

// =========================================================================
// Graph generation member functions
// =========================================================================

// Member function compute_adjacency_matrix() instantiates member
// *adjacency_matrix_ptr.  It loops over all nodes within the current
// graph and extracts edge weights.  This method fills
// *adjacency_matrix_ptr with edge weight information.

genmatrix* graph::compute_adjacency_matrix()
{
//   cout << "inside graph::compute_adjacency_matrix()" << endl;

   delete adjacency_matrix_ptr;
   adjacency_matrix_ptr=new genmatrix(get_n_nodes(),get_n_nodes());
   adjacency_matrix_ptr->clear_values();

   for (unsigned int r=0; r<get_n_nodes(); r++)
   {
      int curr_node_ID=get_node_ID_given_order(r);
      node* curr_node_ptr=get_node_ptr(curr_node_ID);
      vector<graph_edge*> graph_edge_ptrs=curr_node_ptr->
         get_graph_edge_ptrs();
      for (unsigned int e=0; e<graph_edge_ptrs.size(); e++)
      {
         graph_edge* curr_edge_ptr=graph_edge_ptrs[e];
         int neighbor_node_ID=curr_node_ptr->get_neighbor_node_ID(
            curr_edge_ptr);
         double weight=curr_edge_ptr->get_weight();
         adjacency_matrix_ptr->put(curr_node_ID,neighbor_node_ID,weight);
      } // loop over index e labeling edges for *curr_node_ptr
   } // loop over index r labeling matrix rows

//   cout << "*adjacency_matrix_ptr = " << *adjacency_matrix_ptr << endl;
   return adjacency_matrix_ptr;
}

// ---------------------------------------------------------------------
// Member function generate_graph_from_specified_adjacency_matrix()
// takes in input adjacency matrix *adj_matrix_ptr.  It instantiates a
// graph whose nodes and edges are based upon *adj_matrix_ptr.

graph* graph::generate_graph_from_specified_adjacency_matrix(
   genmatrix* adj_matrix_ptr)
{
//   cout << "inside graph::generate_graph_from_specified_adjancency_matrix()"
//        << endl;
   
   graph* graph_ptr=new graph(get_ID()+1,get_level());
   unsigned int n_nodes=adj_matrix_ptr->get_mdim();

   for (unsigned int r=0; r<n_nodes; r++)
   {
      if (!graph_ptr->node_in_graph(r))
      {
         node* node_ptr=new node(r);
         graph_ptr->add_node(node_ptr);
      }
      
      for (unsigned int c=r+1; c<n_nodes; c++)
      {
         if (!graph_ptr->node_in_graph(c))
         {
            node* node_ptr=new node(c);
            graph_ptr->add_node(node_ptr);
         }
         double curr_edge_weight=adj_matrix_ptr->get(r,c);

         if (curr_edge_weight > 0)
         {
            graph_ptr->add_graph_edge(r,c,curr_edge_weight);
         }
      } // loop over index c labeling adjacency matrix columns
   } // loop over index r labeling adjacency matrix rows
   
   graph_ptr->compute_adjacency_matrix();
   return graph_ptr;
}

// ---------------------------------------------------------------------
// Member function create_clone() loops over all nodes and edges
// within the current graph and copies their contents onto a new
// dynamically-generated graph.  The new graph is returned by this
// method.

graph* graph::create_clone()
{
//   cout << "inside graph::create_clone()" << endl;
   
   graph* clone_graph_ptr=new graph(get_ID()+1,get_level());

   compute_adjacency_matrix();

   for (unsigned int r=0; r<get_n_nodes(); r++)
   {
      if (!clone_graph_ptr->node_in_graph(r))
      {
         node* node_ptr=new node(r);
         clone_graph_ptr->add_node(node_ptr);
         node* existing_node_ptr=get_ordered_node_ptr(r);
         *node_ptr=*existing_node_ptr;
      }
      
      for (unsigned int c=r+1; c<get_n_nodes(); c++)
      {
         if (!clone_graph_ptr->node_in_graph(c))
         {
            node* node_ptr=new node(c);
            clone_graph_ptr->add_node(node_ptr);
            node* existing_node_ptr=get_ordered_node_ptr(r);
            *node_ptr=*existing_node_ptr;
         }
         double curr_edge_weight=adjacency_matrix_ptr->get(r,c);
         clone_graph_ptr->add_graph_edge(r,c,curr_edge_weight);
         
      } // loop over index c labeling adjacency matrix columns
   } // loop over index r labeling adjacency matrix rows
   
   cout << "clone_graph_ptr->get_n_nodes() = " 
        << clone_graph_ptr->get_n_nodes() << endl;
   clone_graph_ptr->compute_adjacency_matrix();
   return clone_graph_ptr;
}

// ---------------------------------------------------------------------
// Member function generate_graph_nodes_from_clusters_map() iterates
// over all entries within member STL map clusters_map.  We assume
// that member clusters_map has been precalculated but that no graph
// nodes have been instantiated.  This method generates child nodes
// within the current graph based upon the ID information stored
// within clusters_map.  

void graph::generate_graph_nodes_from_clusters_map()
{
//   cout << "inside graph::generate_graph_nodes_from_clusters_map()"
//        << endl;

   for (CLUSTERS_MAP::iterator itr=clusters_map.begin();
        itr != clusters_map.end(); ++itr)
   {
      vector<int> children_IDs=itr->second;
 
      for (unsigned int c=0; c<children_IDs.size(); c++)
      {
         int curr_node_ID=children_IDs[c];
         node* node_ptr=new node(curr_node_ID);
         add_node(node_ptr);
      } // loop over index c labeling children nodes
   }
}

// ---------------------------------------------------------------------
// Member function recompute_clusters_map_from_graph_nodes() loops
// over all nodes within the current graph.  We assume that all nodes
// have been previously assigned valid parent IDs.  This method
// refills STL map member clusters_map with node & parent node ID
// information.

void graph::recompute_clusters_map_from_graph_nodes()
{
//   cout << "inside graph::recompute_clusters_map_from_graph_nodes()"
//        << endl;

   clusters_map.clear();

//   cout << "get_n_nodes() = " << get_n_nodes() << endl;
   for (unsigned int n=0; n<get_n_nodes(); n++)
   {
      node* curr_node_ptr=get_ordered_node_ptr(n);
      int curr_node_ID=curr_node_ptr->get_ID();
      int curr_parent_ID=curr_node_ptr->get_parent_ID();

//      cout << "n = " << n << " curr_node_ID = " << curr_node_ID
//           << " curr_parent_ID = " << curr_parent_ID << endl;

      CLUSTERS_MAP::iterator iter=clusters_map.find(curr_parent_ID);
      if (iter == clusters_map.end())
      {
         vector<int> V;
         V.push_back(curr_node_ID);
         clusters_map[curr_parent_ID]=V;
      }
      else
      {
         iter->second.push_back(curr_node_ID);
      }
   } // loop over index n labeling current graph's nodes

//   cout << "clusters_map.size() = " << clusters_map.size() << endl;
}

// =========================================================================
// GraphML/JSON output member functions
// =========================================================================

// Member function write_graph_json_file() generates a JSON text file
// containing node and edge information for the graph...

void graph::write_graph_json_file(string json_filename)
{
//   cout << "inside graph::write_graph_json_file()" << endl;

   string graph_ID="Graph";
   string edge_default="directed";

   string value="\n";
   value="{ \n";
   value += "  \"graph\": { \n";

   value +=  output_GraphML_key_value_pair(5,"id",graph_ID);
   
   value += "     \"edgedefault\": \"";
   value += edge_default;
   value += "\", \n";

// Write out nodes:

   cout << "Writing out nodes:" << endl;

   value += "     \"node\": [ \n";

   bool terminal_node_flag=false;
   int n_nodes=get_n_nodes();
//   cout << "n_nodes = " << n_nodes << endl;
   int node_counter=0;
   for (NODES_MAP::const_iterator iter=nodes_map.begin(); iter !=
           nodes_map.end(); iter++)
   {
      node* node_ptr=iter->second;
//      cout << "node_ptr = " << node_ptr << endl;

      node_counter++;
      if (node_counter==n_nodes) terminal_node_flag=true;
      value += output_node_GraphML(9,node_ptr,terminal_node_flag);
   }
   value += "    ], \n";

// Write out edges:

   cout << "Writing out edges:" << endl;
   compute_edge_weights_distribution(0);

   value += "    \"edge\": [ \n";

   bool terminal_edge_flag=false;
   int n_edges=get_n_graph_edges();
   int edge_counter=0;
   cout << "graph_edges_map.size() = "
        << graph_edges_map.size() << endl;

   for (GRAPH_EDGES_MAP::const_iterator iter=graph_edges_map.begin();
        iter != graph_edges_map.end(); iter++)
   {
      graph_edge* graph_edge_ptr=iter->second;
//      cout << "Graph_edge_ptr = " << graph_edge_ptr << endl;
      int curr_matches=graph_edge_ptr->get_weight();

      edge_counter++;
      if (edge_counter==n_edges) terminal_edge_flag=true;
      if (curr_matches > 0)
      {
         node* node1_ptr=graph_edge_ptr->get_node1_ptr();
         node* node2_ptr=graph_edge_ptr->get_node2_ptr();
  
         colorfunc::RGB edge_RGB=compute_edge_color(curr_matches);

//         double max_matches = 100;
//         double min_matches = 0;
//         colorfunc::RGB edge_RGB=compute_edge_color(
//            curr_matches, max_matches, min_matches);

         graph_edge_ptr->set_edge_RGB(edge_RGB);

         double relative_edge_thickness=1;
         if (get_level()==1)
         {
            relative_edge_thickness=2;
         }
         else if (get_level() >= 2)
         {
            relative_edge_thickness=3;
         }
         value += output_edge_GraphML(
            9,node1_ptr->get_ID(),node2_ptr->get_ID(),
            curr_matches,edge_RGB.first,edge_RGB.second,edge_RGB.third,
            relative_edge_thickness,terminal_edge_flag);
      }
   } // loop over graph_edges_map iterator
   value += "    ] \n";

   value += "  } \n";
   value += "} \n";

   write_json_file(json_filename,value);
}

// ---------------------------------------------------------------------
// Member function output_node_GraphML()

string graph::output_node_GraphML(
   int n_indent,node* node_ptr,bool terminal_node_flag)
{
//   cout << "inside graph::output_node_GraphML()" << endl;

   string node_value=indent_spaces(n_indent-2);
   node_value += "{ \n";

   node_value += output_GraphML_key_value_pair(
      n_indent,"id",stringfunc::number_to_string(node_ptr->get_ID() ));

   int time_stamp=0;
//      node_ptr->get_clock().secs_elapsed_since_reference_date();

   int parent_ID=node_ptr->get_parent_ID();
   colorfunc::RGB node_RGB=node_ptr->get_node_RGB();
//   cout << "parent_ID = " << parent_ID << endl;
//   cout << "node_RGB = " << node_RGB << endl;

   node_value += output_data_GraphML(
      n_indent,"NODE",time_stamp,
      -1,parent_ID,node_ptr->get_children_node_IDs(),
      node_ptr->get_Uposn(),node_ptr->get_Vposn(),
      node_RGB.first,node_RGB.second,node_RGB.third,
      node_ptr->get_relative_size());

   node_value += indent_spaces(n_indent-2);
   node_value += "}";
   if (!terminal_node_flag) node_value += ",";
   node_value += "\n";

//   cout << "node_value = " << node_value << endl;
   return node_value;
}

// ---------------------------------------------------------------------
// Member function output_data_GraphML() exports the current graph's
// nodes content to an output string in JSON format.

string graph::output_data_GraphML(
   int n_indent,double edge_weights,double r,double g,double b,
   double relative_size)
{
   string data_type="";
   int time_stamp=-1;
   int parent_ID=-1;
   vector<int> children_IDs;
   string thumbnail_URL="";
   return output_data_GraphML(
      n_indent,data_type,time_stamp,
      edge_weights,parent_ID,children_IDs,
      NEGATIVEINFINITY,NEGATIVEINFINITY,r,g,b,relative_size);
}

string graph::output_data_GraphML(
   int n_indent,string data_type,int time_stamp,
   double edge_weights,int parent_ID,const vector<int>& children_IDs,
   double gx,double gy,double r,double g,double b,double relative_size)
{
//   cout << "inside graph::output_data_GraphML()" << endl;
//   cout << "gx = " << gx << " gy = " << gy << endl;

   string data_value=indent_spaces(n_indent);

   data_value += "\"data\": {\n";
   if (data_type.size() > 0)
   {
      data_value += output_GraphML_key_value_pair(
         n_indent+2,"type",data_type);
   }

   if (time_stamp > 0)
   {
      data_value += output_GraphML_key_value_pair(
         n_indent+2,"time_stamp",stringfunc::number_to_string(time_stamp));
   }

   if (edge_weights > 0)
   {
      data_value += output_GraphML_key_value_pair(
         n_indent+2,"edge_weights",stringfunc::number_to_string(edge_weights));
   }
   if (parent_ID >= 0)
   {
      data_value += output_GraphML_key_value_pair(
         n_indent+2,"parent_ID",stringfunc::number_to_string(parent_ID));
   }

   if (children_IDs.size() > 0)
   {
      string children_IDs_str;
      for (unsigned int c=0; c<children_IDs.size(); c++)
      {
         int child_ID=children_IDs[c];
         children_IDs_str += stringfunc::number_to_string(child_ID)+" ";
      }

      data_value += output_GraphML_key_value_pair(
         n_indent+2,"children_ID",children_IDs_str);
   }

   if (gx >= 0)
   {
      data_value += output_GraphML_key_value_pair(
         n_indent+2,"U",stringfunc::number_to_string(gx));
   }
   if (gy >= 0)
   {
      data_value += output_GraphML_key_value_pair(
         n_indent+2,"V",stringfunc::number_to_string(gy));
   }

   data_value += output_GraphML_key_value_pair(
      n_indent+2,"relativeSize",stringfunc::number_to_string(relative_size));
   
   if (r > -0.5 && g > -0.5 && b > -0.5)
   {
      string rgb_str = stringfunc::number_to_string(r)+" "+
         stringfunc::number_to_string(g)+" "+
         stringfunc::number_to_string(b)+"\" \n";
      bool terminal_comma_flag=false;
      data_value += output_GraphML_key_value_pair(
         n_indent+2,"rgbColor",rgb_str,terminal_comma_flag);
   }
   data_value += indent_spaces(n_indent);
   data_value += "}\n";

   return data_value;
}

// ---------------------------------------------------------------------
// Member function output_edge_GraphML() generates an output string
// source and target node indices along with edge weight and color
// information.  The output string is returned by this method.

string graph::output_edge_GraphML(
   int n_indent,int i,int j,double edge_weights,
   double r,double g,double b,double relative_thickness,
   bool terminal_edge_flag)
{
//   cout << "inside graph::output_edge_GraphML()" << endl;

   string edge_value=indent_spaces(n_indent-2);
   edge_value += "{ \n";

   edge_value +=  output_GraphML_key_value_pair(
      n_indent,"source",stringfunc::number_to_string(i));
   edge_value +=  output_GraphML_key_value_pair(
      n_indent,"target",stringfunc::number_to_string(j));

   edge_value += output_data_GraphML(
      n_indent,edge_weights,r,g,b,relative_thickness);

   edge_value += indent_spaces(n_indent-2)+"}";
   if (!terminal_edge_flag) edge_value += ",";
   edge_value += "\n";

   return edge_value;
}

// ---------------------------------------------------------------------
// Member function compute_edge_weights_distribution() iterates over
// numbers of SIFT feature matches stored within member STL map
// graph_edges_map.  It computes the probability distribution for the
// numbers of feature matches.  This method then fills member STL
// vector edge_weights_threshold with reasonable threshold values based
// upon cumulative distribution lookup.

void graph::compute_edge_weights_distribution(
   double minimal_edge_weights_threshold)
{
//   cout << "inside graph::compute_edge_weights_distribution()" << endl;
//   cout << "graph_edges_map.size() = " << graph_edges_map.size() << endl;

   if (graph_edges_map.size()==0)
   {
      return;
   }
   else if (graph_edges_map.size()==1)
   {
      edge_weights_threshold.push_back(0);
      return;
   }

   vector<double> edge_weights;
   for (GRAPH_EDGES_MAP::const_iterator iter=graph_edges_map.begin();
        iter != graph_edges_map.end(); iter++)
   {
      graph_edge* curr_edge_ptr=iter->second;
      double curr_weight=curr_edge_ptr->get_weight();
      if (curr_weight > minimal_edge_weights_threshold)
      {
         edge_weights.push_back(curr_weight);
//         cout << "edge_weights = " << edge_weights.back() << endl;
      }
   } // loop over graph_edges_map iterator

//   cout << "edge_weights.size() = " << edge_weights.size() << endl;
   if (edge_weights.size() < 2) 
   {
      for (unsigned int e=0; e<edge_weights.size(); e++)
      {
         edge_weights_threshold.push_back(0);
      }
      return;
   }

   int n_output_bins=100;
   double xlo=0;
   double median_edge_weight=mathfunc::median_value(edge_weights);
   double delta_x=median_edge_weight/5.0;
//   cout << "median_edge_weight = " << median_edge_weight 
//        << " delta_x = " << delta_x << endl;
//   outputfunc::enter_continue_char();
   prob_distribution prob(edge_weights,n_output_bins,xlo,delta_x);

   unsigned int n_pbins=8;
   double p_start=0;
   double dp=1.0/(n_pbins-1);
   for (unsigned int n=0; n<n_pbins; n++)
   {
      double p=p_start+n*dp;
      double x=prob.find_x_corresponding_to_pcum(p);
//      cout << "p = " << p << " x = " << x << endl;
      if (n > 0 && n < n_pbins-1) 
      {
         edge_weights_threshold.push_back(x);
//         cout <<  "edge_weights_threshold = " << edge_weights_threshold.back()
//              << endl;
      }
   }
}

// ---------------------------------------------------------------------
// Member function compute_edge_color() sets the color of an edge
// linking one photo to another based upon the number of SIFT features
// which they share in common.

colorfunc::RGB graph::compute_edge_color(int n_SIFT_matches)
{
//   cout << "inside graph::compute_edge_color()" << endl;

   vector<double> hue;
   hue.push_back(300);		// purple
   hue.push_back(240);		// blue
   hue.push_back(180);		// cyan
   hue.push_back(120);		// green
   hue.push_back(60);		// yellow
   hue.push_back(0);		// red

   double curr_hue;
   if (n_SIFT_matches < edge_weights_threshold[0])
   {
      curr_hue=hue[0];
   }
   else if (n_SIFT_matches >= edge_weights_threshold[0] &&
            n_SIFT_matches < edge_weights_threshold[1])
   {
      curr_hue=hue[0]+double(n_SIFT_matches-edge_weights_threshold[0])/
         (edge_weights_threshold[1]-edge_weights_threshold[0])*
         (hue[1]-hue[0]);
   }
   else if (n_SIFT_matches >= edge_weights_threshold[1] &&
            n_SIFT_matches < edge_weights_threshold[2])
   {
      curr_hue=hue[1]+double(n_SIFT_matches-edge_weights_threshold[1])/
         (edge_weights_threshold[2]-edge_weights_threshold[1])*
         (hue[2]-hue[1]);
   }
   else if (n_SIFT_matches >= edge_weights_threshold[2] &&
            n_SIFT_matches < edge_weights_threshold[3])
   {
      curr_hue=hue[2]+double(n_SIFT_matches-edge_weights_threshold[2])/
         (edge_weights_threshold[3]-edge_weights_threshold[2])*
         (hue[3]-hue[2]);
   }
   else if (n_SIFT_matches >= edge_weights_threshold[3] &&
            n_SIFT_matches < edge_weights_threshold[4])
   {
      curr_hue=hue[3]+double(n_SIFT_matches-edge_weights_threshold[3])/
         (edge_weights_threshold[4]-edge_weights_threshold[3])*
         (hue[4]-hue[3]);
   }
   else if (n_SIFT_matches >= edge_weights_threshold[4] &&
            n_SIFT_matches < edge_weights_threshold[5])
   {
      curr_hue=hue[4]+double(n_SIFT_matches-edge_weights_threshold[4])/
         (edge_weights_threshold[5]-edge_weights_threshold[4])*
         (hue[5]-hue[4]);
   }
   else if (n_SIFT_matches > edge_weights_threshold[5])
   {
      curr_hue=hue[5];
   }

   colorfunc::HSV curr_hsv;
   curr_hsv.first=curr_hue;
   curr_hsv.second=1.0;
   curr_hsv.third=1.0;
   colorfunc::RGB curr_RGB=colorfunc::hsv_to_RGB(curr_hsv);

   if (curr_RGB.first < 0 || curr_RGB.second < 0 || curr_RGB.third < 0)
   {
      cout << "hue = " << curr_hue << endl;
      cout << "curr_RGB.first = " << curr_RGB.first << endl;
      cout << "curr_RGB.second = " << curr_RGB.second << endl;
      cout << "curr_RGB.third = " << curr_RGB.third << endl;
      
      outputfunc::enter_continue_char();
   }
   
//   curr_RGB.first=1;
//   curr_RGB.second=1;
//   curr_RGB.third=1;

//   cout << "n_SIFT_matches = " << n_SIFT_matches
//        << " edge RGB = " << curr_RGB.first << "  "
//        << curr_RGB.second << "  " << curr_RGB.third << endl << endl;

   return curr_RGB;
}

// ---------------------------------------------------------------------
// This overloaded version of compute_edge_color() sets the hsv
// coloring of an edge linking one photo to another based upon the
// edge weight relative to the extremal weight values.  If the weight
// value lies close to its maximum [minimum], the edge color is set to
// bright red [blue].  If the weight value lies close to the average
// of the min & max weights, the edge color is set to a dark grey
// value.  

// We wrote this method in Aug 2016 in order to accentuate neural
// network weights which are positively or negatively correlated with
// adjacent layer filters.

colorfunc::RGB graph::compute_edge_color(
  double weight, double max_weight, double min_weight)
{
   if(nearly_equal(max_weight, -1) && nearly_equal(min_weight, -1))
   {
      return compute_edge_color(weight);
   }

   double weight_mean = 0.5 * (max_weight + min_weight);
   double frac_weight = (weight - weight_mean) / (max_weight - weight_mean);
   if(frac_weight > 1) frac_weight = 1.0;
   if(frac_weight < -1) frac_weight = -1.0;

   // frac_weight = 1 --> hue = red     sat = 1     value = 1
   // frac_weight = 0 --> hue = green,  sat = 0.25  value = 0.25
   // frac_weight = -1 --> hue = blue   sat = 1     value = 1

   colorfunc::HSV curr_hsv;
   curr_hsv.first = (1 - frac_weight) * 120;
   curr_hsv.second = 0.15 + 0.85 * fabs(frac_weight);
   curr_hsv.third = 0.25 + 0.75 * fabs(frac_weight);
   return colorfunc::hsv_to_RGB(curr_hsv);
}

// ---------------------------------------------------------------------
// Member function indent_spaces()

string graph::indent_spaces(unsigned int n_indent)
{
   string value;
   for (unsigned int n=0; n<n_indent; n++)
   {
      value += " ";
   }
   return value;
}

// ---------------------------------------------------------------------
// Member function output_GraphML_key_value_pair() generates an
// indented line of text containing a key surrounded in double quotes
// separated by a colon from a value surrounded in double quotes.

string graph::output_GraphML_key_value_pair(
   int n_indent,string key,string value,bool terminal_comma_flag)
{
//   cout << "inside graph::output_GraphML_key_value_pair()" << endl;

   string curr_line=indent_spaces(n_indent);
   curr_line += "\""+key+"\": \"";
   curr_line += value;
   if (terminal_comma_flag)
   {
      curr_line += "\", \n";
   }
   else
   {
//      curr_line += "\n";   
   }

   return curr_line;
}

// ---------------------------------------------------------------------
// Member function write_json_file() generates a JSON text file
// containing graph node and edge information.

void graph::write_json_file(const string& value)
{
   string json_filename="graph.json";
   write_json_file(json_filename,value);
}

void graph::write_json_file(string json_filename,const string& value)
{
   ofstream outstream;
   filefunc::openfile(json_filename,outstream);
   outstream << value << endl;
   filefunc::closefile(json_filename,outstream);
}

// =========================================================================
// Node properties I/O member functions:
// =========================================================================

// Member function export_edgelist() takes in an adjacency matrix
// along with an edge weight threshold.  Looping over the matrix' rows
// and columns, it exports a file of the form node_id node_id'
// edge_weight.  Edges whose weights lie below edgeweight_threshold
// are not included in the output file.

void graph::export_edgelist(
   string edgelist_filename,double edgeweight_threshold)
{
   compute_adjacency_matrix();
   return export_edgelist(
      adjacency_matrix_ptr,edgelist_filename,edgeweight_threshold);
}

void graph::export_edgelist(
   genmatrix* adjacency_ptr,string edgelist_filename,
   double edgeweight_threshold)
{
//   cout << "inside graph::export_edgelist()" << endl;
 
   if (adjacency_ptr==NULL)
   {
      cout << "Error in graph::export_edgelist()!" << endl;
      cout << "adjacency_ptr=NULL! " << endl;
      outputfunc::enter_continue_char();
      return;
   }

   ofstream outstream;
   filefunc::openfile(edgelist_filename,outstream);
   outstream << "# Total number of nodes = " << get_n_nodes() << endl;
   outstream << "# Edge weight threshold = " << edgeweight_threshold << endl;
   outstream << "# NodeID  NodeID'  Edge weight" << endl << endl;

   for (unsigned int r=0; r<adjacency_ptr->get_mdim(); r++)
   {
      for (unsigned int c=r+1; c<adjacency_ptr->get_ndim(); c++)
      {
         double edge_weight=adjacency_ptr->get(r,c);
         if (edge_weight < edgeweight_threshold) continue;
         outstream << r << "  " << c << "  " << edge_weight << endl;
      } // loop over index c labeling columns in adjacency matrix
   } // loop over index r labeling rows in adjacency matrix
   filefunc::closefile(edgelist_filename,outstream);

   string banner="Graph edge list written to output file "+edgelist_filename;
   outputfunc::write_big_banner(banner);
}

// ---------------------------------------------------------------------
// Member function read_nodes_layout() parses the specified layout
// text file.  It assigns each node's U & V coordinates based upon the
// parsed graph layout information.

void graph::read_nodes_layout(string layout_filename)
{
//   cout << "inside graph::read_nodes_layout()" << endl;
   if (!filefunc::fileexist(layout_filename)) return;

   string banner="Importing node layout information from "+layout_filename;
   outputfunc::write_banner(banner);

   filefunc::ReadInfile(layout_filename);
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> IDUV=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      int node_ID=IDUV[0];
      double Uposn=IDUV[1];
      double Vposn=IDUV[2];

      node* node_ptr=get_node_ptr(node_ID);
      if(node_ptr != NULL)
      {
         node_ptr->set_posn(twovector(Uposn,Vposn));
      }
      else
      {
         cout << "Warning in graph::read_nodes_layout()" << endl;
         cout << " node_ptr = NULL " << endl;
         cout << "i = " << i << " node_ID = " << node_ID
              << " U = " << Uposn
              << " V = " << Vposn << endl;
         cout << " get_n_nodes() = " << get_n_nodes() << endl;
      }
   }
}

// ---------------------------------------------------------------------
// Member function read_cluster_info() parses clustering text file
// generated by the Markov Cluster Algorithm (see
// http://micans.org/mcl ).  It fills STL map member clusters_map with
// parent_ID keys and children_ID values based upon the parsed input.

void graph::read_cluster_info(string cluster_filename)
{
   read_cluster_info(cluster_filename,0);
}

void graph::read_cluster_info(string cluster_filename,int min_parent_ID)
{
//   cout << "inside graph::read_cluster_info()" << endl;
//   cout << "cluster_filename = " << cluster_filename << endl;
   
   clusters_map.clear();
   int curr_parent_ID=min_parent_ID;

   filefunc::ReadInfile(cluster_filename);
   for (unsigned int n=0; n<filefunc::text_line.size(); n++)
   {
      vector<double> clustered_node_IDs=
         stringfunc::string_to_numbers(filefunc::text_line[n]);
//      cout << "clustered_node_IDs.size() = "
//           << clustered_node_IDs.size() << endl;
      
      vector<int> integer_clustered_node_IDs;
      for (unsigned int i=0; i<clustered_node_IDs.size(); i++)
      {
         int curr_node_ID=clustered_node_IDs[i];
//         cout << "curr_node_ID = " << curr_node_ID << endl;
         integer_clustered_node_IDs.push_back(curr_node_ID);
      }
      clusters_map[curr_parent_ID]=integer_clustered_node_IDs;

      curr_parent_ID++;
   } // loop over index n labeling input file lines
//   cout << "curr_parent_ID = " << curr_parent_ID << endl;
}

// ---------------------------------------------------------------------
// Member function export_cluster_info() outputs the contents of STL
// map member clusters_map to the specified output filename.

void graph::export_cluster_info(string output_cluster_filename)
{
//   cout << "inside graph::export_cluster_info()" << endl;
//   cout << "output_cluster_filename = " << output_cluster_filename << endl;
//   cout << "clusters_map.size() = " << clusters_map.size() << endl;

   ofstream outstream;
   filefunc::openfile(output_cluster_filename,outstream);

   for (CLUSTERS_MAP::iterator itr=get_clusters_map().begin();
        itr != get_clusters_map().end(); ++itr)
   {
      int parent_node_ID=itr->first;
      vector<int> clustered_node_IDs=
         get_clusters_map().at(parent_node_ID);

//      outstream << parent_node_ID << "       ";
      for (unsigned int i=0; i<clustered_node_IDs.size(); i++)
      {
         outstream << clustered_node_IDs[i] << " ";
      }
      outstream << endl;
   }
   filefunc::closefile(output_cluster_filename,outstream);
}

// =========================================================================
// Node coloring member functions
// =========================================================================

// Member function output_DIMACS_edgelist_file() generates a text file
// containing graph edge information of the form "node_i node_j".  It
// follows the DIMACS conventions discussed in "Clique and Coloring
// Problems Graph Format", May 08, 1993).  This DIMACS formatted
// output file can be read in by Andrea Arbore's nRLF graph vertex
// coloring program.  

void graph::output_DIMACS_edgelist_file(string DIMACS_edgelist_filename)
{

//   cout << "inside graph::output_DIMACS_edgelist_file()" << endl;

   ofstream outstream;
   filefunc::openfile(DIMACS_edgelist_filename,outstream);
   outstream << "c Edge list in DIMACS graph format" << endl;
   outstream << "p edge " 
             << get_n_nodes() << " " << get_n_graph_edges() << endl;

   for (GRAPH_EDGES_MAP::iterator itr=graph_edges_map.begin();
        itr != graph_edges_map.end(); ++itr)
   {
      graph_edge* curr_edge_ptr=itr->second;
      node* node1_ptr=curr_edge_ptr->get_node1_ptr();
      node* node2_ptr=curr_edge_ptr->get_node2_ptr();
      int node1_ID=node1_ptr->get_ID();
      int node2_ID=node2_ptr->get_ID();
      if (node1_ID < node2_ID)
      {
         outstream << "e " << node1_ID+1 << " " << node2_ID+1 << endl;
      }
   } // loop over graph_edges_map 

   filefunc::closefile(DIMACS_edgelist_filename,outstream);
}

// ---------------------------------------------------------------------
// Member function run_Arbore_graph_coloring() 

void graph::run_Arbore_graph_coloring(string DIMACS_edgelist_filename)
{
//   cout << "inside graph::run_Arbore_graph_coloring()" << endl;
//   cout << "DIMACS_edgelist_filename = " << DIMACS_edgelist_filename << endl;
   string subdir=filefunc::getdirname(DIMACS_edgelist_filename);
   string filename=filefunc::getbasename(DIMACS_edgelist_filename);

   cout << "subdir = " << subdir << endl;
   cout << "filename = " << filename << endl;
   
   string unix_command="cd "+subdir;
   cout << "unix command #1 = " << unix_command << endl;
   sysfunc::unix_command(unix_command);
   unix_command="arbore2_graph_coloring "+filename;
   cout << "unix command #2 = " << unix_command << endl;
   sysfunc::unix_command(unix_command);
}

// ---------------------------------------------------------------------
// Member function read_Arbore_vertex_coloring() parses the output
// file generated by Andrea Arbore's nRLF graph vertex coloring
// algorithm (A simple variant of the RLF algorithm (nRLF v2.0) for
// Graph Coloring - arbore2.c ) . The format of the input
// vertex_coloring_filename is simply a column of numbers
// corresponding to the nodes with the same row number.  The input
// file (generated by Arbore's c program) has nodes and colors always
// starting from 1 rather than 0.

// This method sets each node's color_ID and RGB color based upon the
// contents of the input file.  The graph's total number of distinct
// vertex colors is returned by this method.

int graph::read_Arbore_vertex_coloring(string vertex_coloring_filename)
{
//   cout << "inside graph::read_Arbore_vertex_coloring()" << endl;
//   cout << "vertex_coloring_filename = " << vertex_coloring_filename << endl;
   
   filefunc::ReadInfile(vertex_coloring_filename);
   n_distinct_colors=0;
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      node* currnode_ptr=get_ordered_node_ptr(i);
      int color_ID=stringfunc::string_to_number(filefunc::text_line[i])-1;
      currnode_ptr->set_color_ID(color_ID);
      cout << "node ID = " << currnode_ptr->get_ID()
           << " color_ID = " << color_ID << endl;
      n_distinct_colors=basic_math::max(n_distinct_colors,color_ID);
   }
   n_distinct_colors++;
//   cout << "n_distinct_colors = " << n_distinct_colors << endl;

// After number of distinct colors has been read in, assign actual RGB
// values to each cluster node:

   for (unsigned int n=0; n<get_n_nodes(); n++)
   {
      node* curr_node_ptr=get_ordered_node_ptr(n);
      compute_cluster_color(curr_node_ptr);
   } // loop over index n labeling nodes within current graph

// Confirm that neighbor colorings are distinct:

   for (unsigned int n=0; n<get_n_nodes(); n++)
   {
      node* curr_node_ptr=get_ordered_node_ptr(n);
      int curr_color_ID=curr_node_ptr->get_color_ID();
      vector<int> neighbor_node_IDs=curr_node_ptr->get_neighbor_node_IDs();

      for (unsigned int i=0; i<neighbor_node_IDs.size(); i++)
      {
         node* neighbor_node_ptr=get_node_ptr(neighbor_node_IDs[i]);
         int neighbor_color_ID=neighbor_node_ptr->get_color_ID();
         if (neighbor_color_ID==curr_color_ID)
         {
            cout << "curr node ID = " << curr_node_ptr->get_ID()
                 << " curr color ID = " << curr_color_ID << endl;
            cout << "neighbor node ID = " << neighbor_node_ptr->get_ID()
                 << " neighbor color ID = " << neighbor_color_ID << endl;
         }
      } // loop over index i labeling current neighbor nodes
   } // loop over index n labeling nodes

   return n_distinct_colors;
}

// ---------------------------------------------------------------------
// Member function compute_cluster_color() takes in a node which is
// assumed to have a color_ID assigned based upon some reasonable
// graph vertex coloring scheme.  The node is also assumed to have a
// valid parent_ID assignment.  Cluster_IDs are generally assumed to
// significantly exceed n_distinct_colors.  This method performs a
// "zig-zag" assignment of hues, saturations and values in order to
// maximally assign disgtinguishable colors to the first
// 4*n_distinct_color nodes.

void graph::compute_cluster_color(node* node_ptr)
{
//   cout << "inside graph::compute_cluster_color()" << endl;

   node_ptr->set_node_RGB(colorfunc::get_RGB_values(colorfunc::grey));
   int color_ID=node_ptr->get_color_ID();
//   cout << "color_ID = " << color_ID << endl;
   if (color_ID < 0) return;

   int n_hues=n_distinct_colors;
   double hue_start=0;
   double hue_stop=360;
   double delta_hue=(hue_stop-hue_start)/n_hues;
//   cout << "n_hues = " << n_hues << " delta_hue = " << delta_hue << endl;
   double hue=delta_hue*color_ID;

// In order to mix up colors as much as possible, perform "zig-zag"
// sampling of hues and values or hues and saturations.  Also, add hue
// offsets in order to guarantee every node with cluster ID ranging
// from 0 to 4*n_hues-1 receives a unique (h,s,v) color assignment:

   int parent_ID=node_ptr->get_parent_ID();
//   cout << "parent_ID = " << parent_ID << endl;

   int n_saturations=2;
   double saturation=1.0;
   if (is_odd(parent_ID/n_hues))
   {
      saturation=0.6;
      hue += 0.66*delta_hue;
   }

   double value=1.0;
   if (is_odd(parent_ID/(n_saturations*n_hues)))
   {
      value=0.40;
      hue += 0.33*delta_hue;
   }

   colorfunc::HSV curr_hsv;
   curr_hsv.first=hue;
   curr_hsv.second=saturation;
   curr_hsv.third=value;
   node_ptr->set_node_RGB(colorfunc::hsv_to_RGB(curr_hsv));
}

// ---------------------------------------------------------------------
// Member function compute_node_cluster_color() recovers the
// parent_ID calculated by the Markov Cluster Algorithm (see
// http://micans.org/mcl ) for the input *node_ptr.  If the
// parent_ID < 0, this method returns a grey coloring.  Otherwise, it
// computes and returns a quasi-random hue, saturation and value based
// upon the parent_ID.

colorfunc::RGB graph::compute_node_cluster_color(
   node* node_ptr,int parent_ID)
{
//   cout << "inside graph::compute_node_cluster_color()" << endl;

   colorfunc::RGB curr_RGB=colorfunc::get_RGB_values(colorfunc::grey);

//   cout << "parent_ID = " << parent_ID << endl;
   if (parent_ID < 0) return curr_RGB;

   double hue_start=0;
   double hue_stop=360;
   double delta_hue=24;	// degs
   int n_hues=(hue_stop-hue_start)/delta_hue;	// = 15

// Mix up hues by adding a ~90 deg offset corresponding to some
// multiple of delta_hue which does NOT divide evenly into n_hues:

   delta_hue=4*delta_hue;    // Note: 4 does not divide evenly into 15 !

   int n_saturations=2;
   double saturation_start=1.0;
   double saturation_stop=0.5;
   double delta_saturation=(saturation_stop-saturation_start)/
      double(n_saturations-1);

   int n_values=3;
   double value_start=1;
   double value_stop=0.25;
   double delta_value=(value_stop-value_start)/double(n_values-1);
//   cout << "delta_value = " << delta_value << endl;

   int n_colors=n_hues*n_values*n_saturations;	// = 15*3*2=90
//   cout << "n_colors = " << n_colors << endl;
   
   parent_ID=modulo(parent_ID,n_colors);
//   cout << "Moduloed parent_ID = " << parent_ID << endl;

   double hue=basic_math::phase_to_canonical_interval(
      delta_hue*double(parent_ID%n_hues),0,360);
//   cout << "parent_ID/n_hues = " << parent_ID/n_hues << endl;
   double saturation=saturation_start+(parent_ID/(n_hues*n_values))*
      delta_saturation;
   double value=value_start+(parent_ID/(n_hues*n_saturations))*delta_value;

   colorfunc::HSV curr_hsv;
   curr_hsv.first=hue;
   curr_hsv.second=saturation;
   curr_hsv.third=value;
   curr_RGB=colorfunc::hsv_to_RGB(curr_hsv);
//   cout << "curr_RGB = " << curr_RGB << endl;

   return curr_RGB;
}

// ---------------------------------------------------------------------
// Member function compute_node_centrality_color() extracts the
// centrality for the input *node_ptr.  If the centrality exceeds 0,
// this method sets a hue based upon the logarithm (base 10) of the
// centrality.  It returns an RGB value containing the hue (and
// maximal saturation plus value) color information.

colorfunc::RGB graph::compute_node_centrality_color(node* node_ptr)
{
//   cout << "inside graph::compute_node_centrality_color()" << endl;

   colorfunc::RGB curr_RGB=colorfunc::get_RGB_values(colorfunc::grey);

   double centrality=node_ptr->get_centrality();
//   cout << "centrality = " << centrality << endl;
   if (centrality < 0) return curr_RGB;

   double hue=0;
   double log10centrality=log10(centrality);
   const double log10_centrality_start=-1;
   const double log10_centrality_stop=5;

   const double hue_start=300;	// purple
   const double hue_stop=0;	// red
   hue=hue_start;
   if (log10centrality < log10_centrality_start)
   {
      hue=hue_start;
   }
   else if (log10centrality > log10_centrality_stop)
   {
      hue=hue_stop;
   }
   else
   {
      hue=hue_start+(log10centrality-log10_centrality_start)/
         (log10_centrality_stop-log10_centrality_start)*(hue_stop-hue_start);
   }

//   cout << "hue = " << hue << endl;

   colorfunc::HSV curr_hsv;
   curr_hsv.first=hue;
   curr_hsv.second=1.0;
   curr_hsv.third=1.0;
   curr_RGB=colorfunc::hsv_to_RGB(curr_hsv);

   return curr_RGB;
}

// ---------------------------------------------------------------------
// Member function assign_grandparent_node_colors() attempts to
// diversify hues among ~10**1 grandparent nodes.  Ideally, no two
// neighboring grandparent nodes should have hues which are close in
// value.

void graph::assign_grandparent_node_colors()
{
//   cout << "inside graph::assign_grandparent_node_colors()" << endl;

   n_distinct_colors=get_n_nodes();
//   cout << "n_distinct_colors = " << n_distinct_colors << endl;

   int prime_factor=1;
   if (n_distinct_colors%5 > 0)
   {
      prime_factor=5;
   }
   else if (n_distinct_colors%3 > 0)
   {
      prime_factor=3;
   }
   else if (n_distinct_colors%7 > 0)
   {
      prime_factor=7;
   }

   bool redo_coloring_flag=false;
   unsigned int ID_offset=0;
   do
   {
      redo_coloring_flag=assign_grandparent_node_colors(
         prime_factor,ID_offset);
      ID_offset++;
   }
   while (redo_coloring_flag && ID_offset < get_n_nodes());

//   cout << "prime_factor = " << prime_factor << endl;
//   cout << "redo_coloring = " << redo_coloring_flag << endl;
//   cout << "ID_offset = " << ID_offset << " get_n_nodes() = "
//        << get_n_nodes() << endl;
//   outputfunc::enter_continue_char();
}

// ---------------------------------------------------------------------
// Member function assign_grandparent_node_color() assigns colors to
// all nodes within the current graph (which is assumed to represent
// at least a grandparent level within a pyramid scheme).  Grandparent
// node colors are set equal to pure hues.  If any two neighboring
// nodes have hues whose difference in absolute value is less than
// min_delta_hue, this boolean method returns redo_coloring_flag=true.

bool graph::assign_grandparent_node_colors(int prime_factor,int ID_offset)
{
//   cout << "inside graph::assign_grandparent_node_colors()" << endl;

// As of 2/15/10, we have empirically found that setting
// min_delta_hue=52.5 or 50 degrees yields reasonable coloring for the
// 18 node grandparents graph derived from the MIT 2.3K photo set...

//   const double min_delta_hue=50;
   const double min_delta_hue=52.5;

   int n_hues=n_distinct_colors;
   double hue_start=0;
   double hue_stop=360;
   double delta_hue=(hue_stop-hue_start)/n_hues;
//   cout << "n_hues = " << n_hues << " delta_hue = " << delta_hue << endl;

   typedef map<int,double> NODE_HUES_MAP;
   NODE_HUES_MAP node_hues_map;

   bool redo_coloring_flag=false;
   for (unsigned int n=0; n<get_n_nodes(); n++)
   {
      node* curr_node_ptr=get_ordered_node_ptr(n);
      int curr_node_ID=curr_node_ptr->get_ID();
      double curr_hue=prime_factor*delta_hue*(curr_node_ID+ID_offset);
      curr_hue=basic_math::phase_to_canonical_interval(curr_hue,0,360);

      vector<int> neighbor_node_IDs=curr_node_ptr->get_neighbor_node_IDs();

      int max_iters=get_n_nodes();
      int iter_count=0;

      bool recheck_neighbor_colors_flag;
      do
      {
         recheck_neighbor_colors_flag=false;
         iter_count++;
         
         for (unsigned int n=0; n<neighbor_node_IDs.size(); n++)
         {
            int curr_neighbor_node_ID=neighbor_node_IDs[n];
         
            NODE_HUES_MAP::iterator iter=node_hues_map.find(
               curr_neighbor_node_ID);      
            if (iter == node_hues_map.end()) continue;

            double neighbor_hue=iter->second;
            double delta_hue=curr_hue-neighbor_hue;
            delta_hue=basic_math::phase_to_canonical_interval(
               delta_hue,-180,180);

            if (fabs(delta_hue) < min_delta_hue)
            {
               if (delta_hue > 0)
               {
                  curr_hue=neighbor_hue+min_delta_hue;
               }
               else
               {
                  curr_hue=neighbor_hue-delta_hue;
               }
               recheck_neighbor_colors_flag=true;
            }
         } // loop over index n labeling neighboring nodes
      }    
      while (recheck_neighbor_colors_flag && iter_count < max_iters);

      if (iter_count >= max_iters)
      {
         redo_coloring_flag=true;
      }
      
//      cout << "n = " << n 
//           << " iter_count = " << iter_count
//           << " max_iters = " << max_iters << endl;

      curr_node_ptr->set_pure_hue_color(curr_hue);
      node_hues_map[curr_node_ID]=curr_hue;
   } // loop over index n labeling grandparent graph nodes

   return redo_coloring_flag;
}

// ---------------------------------------------------------------------
// Member function assign_parent_node_colors() takes in
// *grandparents_graph_ptr whose nodes are assumed to have already
// been assigned pure hue colors.  The current parents graph
// clusters_map is also assumed to be filled with valid
// parent-grandparent node relationships.  For each parent node within
// a grandparent cluster, this method quasi-randomly varies the
// grandparent node's saturation and value and perturbs the hue color
// coordinates.  The resulting parent offspring nodes are thus
// assigned colors which are derived from the grandparent's but
// noticeably differ from each other.

void graph::assign_parent_node_colors(graph* grandparents_graph_ptr)
{
//   cout << "inside graph::assign_parent_node_colors()" << endl;
//   cout << "get_n_clusters() = " << get_n_clusters() << endl;

//   int n_hues=n_distinct_colors;
   int n_hues=grandparents_graph_ptr->get_n_distinct_colors();
   double hue_start=0;
   double hue_stop=360;
   double delta_hue=(hue_stop-hue_start)/n_hues;
//   cout << "n_hues = " << n_hues << " delta_hue = " << delta_hue << endl;
   
   for (unsigned int grandparent_counter=0; grandparent_counter<
           grandparents_graph_ptr->get_n_nodes(); grandparent_counter++)
   {
      node* grandparent_node_ptr=grandparents_graph_ptr->
         get_ordered_node_ptr(grandparent_counter);
      colorfunc::RGB grandparent_RGB=grandparent_node_ptr->get_node_RGB();
      colorfunc::HSV grandparent_HSV=colorfunc::RGB_to_hsv(grandparent_RGB);
      double grandparent_hue=grandparent_HSV.first;
//      cout << "grandparent_ID = " << cluster_ID
//           << " grandparent hue = " << grandparent_hue << endl;

      vector<int> node_IDs=grandparent_node_ptr->get_children_node_IDs();
      for (unsigned int i=0; i<node_IDs.size(); i++)
      {
         double hue=grandparent_hue;
         double saturation=1.0;
         double value=1.0;

         int n_color_variations=8;
         if (i%n_color_variations==0)
         {
         }
         else if (i%n_color_variations==2)
         {
            value=0.85;
            hue -= 0.2*delta_hue;
         }
         else if (i%n_color_variations==4)
         {
            value=0.85;
            hue += 0.2*delta_hue;
         }
         else if (i%n_color_variations==6)
         {
            saturation=0.85;
            value=0.7;
            hue -= 0.1*delta_hue;
         }
         else if (i%n_color_variations==1)
         {
            saturation=0.85;
            value=0.7;
            hue += 0.1*delta_hue;
         }
         else if (i%n_color_variations==3)
         {
            saturation=0.7;
            value=0.55;
            hue += 0.3*delta_hue;
         }
         else if (i%n_color_variations==5)
         {
            saturation=0.7;
            value=0.55;
            hue += 0.3*delta_hue;
         }
         else if (i%n_color_variations==7)
         {
            saturation=0.55;
            value=0.4;
         }

         colorfunc::HSV curr_hsv;
         curr_hsv.first=hue;
         curr_hsv.second=saturation;
         curr_hsv.third=value;
            
         node* node_ptr=get_node_ptr(node_IDs[i]);
         node_ptr->set_node_RGB(colorfunc::hsv_to_RGB(curr_hsv));
            
      } // loop over index i labeling parent nodes
   } // loop over cluster_ID index
}

// ---------------------------------------------------------------------
// Member function compute_ID_based_node_colors() assigns variable
// hues and saturations to each node within the current graph based
// solely upon its ID.  We wrote this method in Mar 2010 in order to
// quasi-randomly color reconstructed iPhone positions within Jennifer
// Drexler's Google Map display.

void graph::compute_ID_based_node_colors()
{
   cout << "inside graph::compute_ID_based_node_colors()" << endl;

   for (unsigned int n=0; n<get_n_nodes(); n++)
   {
      node* node_ptr=get_ordered_node_ptr(n);
      int node_ID=node_ptr->get_ID();

      int n_hues=13;
      double hue_start=0;
      double hue_stop=360;
      double delta_hue=(hue_stop-hue_start)/n_hues;
//   cout << "n_hues = " << n_hues << " delta_hue = " << delta_hue << endl;
      int hue_counter=node_ID%n_hues;
      hue_counter *= 3;
      hue_counter=hue_counter%n_hues;
      double hue=delta_hue*hue_counter;

// In order to mix up colors as much as possible, perform "zig-zag"
// sampling of hues and values or hues and saturations.  Also, add hue
// offsets in order to guarantee every node with cluster ID ranging
// from 0 to 4*n_hues-1 receives a unique (h,s,v) color assignment:

      int n_saturations=2;
      double saturation=1.0;
      if (is_odd(node_ID/n_hues))
      {
         saturation=0.65;
         hue += 0.66*delta_hue;
      }

      double value=1.0;
      if (is_odd(node_ID/(n_saturations*n_hues)))
      {
//         value=0.75;
         hue += 0.33*delta_hue;
      }

      colorfunc::HSV curr_hsv;
      curr_hsv.first=hue;
      curr_hsv.second=saturation;
      curr_hsv.third=value;
      node_ptr->set_node_RGB(colorfunc::hsv_to_RGB(curr_hsv));
   
   } // loop over index n labeling all graph nodes
}

// =========================================================================
// Graph properties member functions
// =========================================================================

// Member function compute_median_degree() loops over all nodes in the
// current graph.  It computes the median node degree and median
// number of node edges.

double graph::compute_median_degree()
{
   cout << "inside graph::compute_median_degree()" << endl;

   vector<double> number_of_edges;
   vector<double> degree;
   for (unsigned int n=0; n<get_n_nodes(); n++)
   {
      node* curr_node_ptr=get_ordered_node_ptr(n);
      number_of_edges.push_back(curr_node_ptr->get_n_edges());
      degree.push_back(curr_node_ptr->get_degree());
//      cout << "node n = " << n
//           << " n_edges = " << number_of_edges.back() << endl;
   }

   double mu_edges=mathfunc::mean(number_of_edges);
   double sigma_edges=mathfunc::std_dev(number_of_edges);
   double median_edges=mathfunc::median_value(number_of_edges);

   cout << "Mean n_edges averaged over entire graph = " 
        << mu_edges << " +/- " << sigma_edges << endl;
   cout << "Median edges = " << median_edges << endl;

   double mu_degree=mathfunc::mean(degree);   
   double sigma_degree=mathfunc::std_dev(degree);
   double median_degree=mathfunc::median_value(degree);

   cout << "Mean node degree averaged over entire graph = " 
        << mu_degree << " +/- " << sigma_degree << endl;
   cout << "Median degree = " << median_degree << endl;

   cout << "Median degree / median n_edges = "
        << median_degree/median_edges << endl;

   return median_degree;
}

// ---------------------------------------------------------------------
// Member function compute_edgeweight_distribution() loops over all
// nodes in the current graph and extracts their edges' weights.  This
// method compute and returns a probability distribution for all edge
// weights within the graph.

prob_distribution graph::compute_edgeweight_distribution()
{
   cout << "inside graph::compute_edgeweight_distribution()" << endl;

   vector<double> edge_weights;
   for (unsigned int n=0; n<get_n_nodes(); n++)
   {
      node* curr_node_ptr=get_ordered_node_ptr(n);

      vector<graph_edge*> graph_edge_ptrs=
         curr_node_ptr->get_graph_edge_ptrs();
      for (unsigned int e=0; e<graph_edge_ptrs.size(); e++)
      {
         graph_edge* curr_edge_ptr=graph_edge_ptrs[e];
         double curr_weight=curr_edge_ptr->get_weight();
         edge_weights.push_back(curr_weight);
      }
   }

   int n_output_bins=100;
   double xlo=0;
   prob_distribution edge_distribution(edge_weights,n_output_bins,xlo);

   cout << "edge_distribution.mu = " << edge_distribution.mean() << endl;
   cout << "edge_distribution.sigma = " << edge_distribution.std_dev() 
        << endl;
   cout << "edge_distribution.median = " << edge_distribution.median() 
        << endl;
   cout << "edge_distribution.quartile_width = " 
        << edge_distribution.quartile_width() << endl;

   return edge_distribution;
}

// ---------------------------------------------------------------------
// Member function node_COM() loops over all nodes within the current
// graph.  It returns the average U and V position for the
// nodes' center-of-mass weighted by their relative sizes.

twovector graph::node_COM()
{
//   cout << "inside graph::node_COM()" << endl;

   vector<double> weighted_U,weighted_V,denom;
   for (unsigned int n=0; n<get_n_nodes(); n++)
   {
      node* curr_node_ptr=get_ordered_node_ptr(n);
      double relative_size=curr_node_ptr->get_relative_size();
      weighted_U.push_back(relative_size*curr_node_ptr->get_Uposn());
      weighted_V.push_back(relative_size*curr_node_ptr->get_Vposn());
      denom.push_back(relative_size);
   } // loop over index n labeling nodes

   double Uavg=mathfunc::mean(weighted_U)/mathfunc::mean(denom);
   double Vavg=mathfunc::mean(weighted_V)/mathfunc::mean(denom);
   twovector COM(Uavg,Vavg);
//   cout << "global node COM = " << COM << endl;

   return COM;
}

// ---------------------------------------------------------------------
// Member function node_std_dev_from_COM() loops over all nodes within
// the current graph.  It returns the standard U and V deviation from
// the the nodes' center-of-mass weighted by the nodes' relative
// sizes.

twovector graph::node_std_dev_from_COM()
{
//   cout << "inside graph::node_std_dev_from_COM()" << endl;

   vector<double> weighted_U,weighted_V,denom;
   for (unsigned int n=0; n<get_n_nodes(); n++)
   {
      node* curr_node_ptr=get_ordered_node_ptr(n);
      double relative_size=curr_node_ptr->get_relative_size();
      weighted_U.push_back(relative_size*curr_node_ptr->get_Uposn());
      weighted_V.push_back(relative_size*curr_node_ptr->get_Vposn());
      denom.push_back(relative_size);
   } // loop over index n labeling nodes

   double Usigma=mathfunc::std_dev(weighted_U)/mathfunc::mean(denom);
   double Vsigma=mathfunc::std_dev(weighted_V)/mathfunc::mean(denom);
//   cout << "Usigma = " << Usigma 
//        << " Vsigma = " << Vsigma << endl;
   twovector SIGMA(Usigma,Vsigma);
   return SIGMA;
}

// ---------------------------------------------------------------------
// Member function generate_nodes_kdtree() loops over all nodes within
// the current graph.  It instantiates and fills member
// *nodes_ktree_ptr with (U,V,ID) information for each of the graph's
// nodes.

void graph::generate_nodes_kdtree()
{
//   cout << "inside graph::generate_nodes_kdtree()" << endl;
//   cout << "get_n_nodes() = " << get_n_nodes() << endl;

   vector<threevector> node_posns;
   for (unsigned int n=0; n<get_n_nodes(); n++)
   {
      node* curr_node_ptr=get_ordered_node_ptr(n);
      threevector curr_node_posn(curr_node_ptr->get_posn(),
                                 curr_node_ptr->get_ID());
      node_posns.push_back(curr_node_posn);
   } // loop over index n labeling nodes

   delete nodes_kdtree_ptr;
   nodes_kdtree_ptr=kdtreefunc::generate_2D_kdtree(node_posns);
}

// ---------------------------------------------------------------------
// Member function find_nearby_nodes() takes in *curr_node_ptr along
// with a radius measured in the UV graph plane.  It uses
// *nodes_kdtree_ptr to rapidly find all graph nodes located within
// radius of *curr_node_ptr's UV position.  This method returns
// pointers to all nearby nodes within output STL vector
// nearby_node_ptrs.

void graph::find_nearby_nodes(
   node* curr_node_ptr,double radius,vector<node*>& nearby_node_ptrs)
{
//   cout << "inside graph::find_nearby_nodes()" << endl;
   threevector UV_posn(curr_node_ptr->get_posn());

   vector<threevector> nearby_nodes;
   kdtreefunc::sorted_nodes_within_range(
      nodes_kdtree_ptr,UV_posn,radius,nearby_nodes);

   for (unsigned int n=0; n<nearby_nodes.size(); n++)
   {
      int nearby_node_ID=nearby_nodes[n].get(2);
      node* nearby_node_ptr=get_node_ptr(nearby_node_ID);
      nearby_node_ptrs.push_back(nearby_node_ptr);
   } // loop over index n labeling nearby nodes
}

// =========================================================================
// Graph layout member functions
// =========================================================================

// Member function attractive_force() loops over all nodes within the
// current graph.  For each node, this method extracts its edge
// weights and neighboring node positions.  The attractive strength of
// each neighboring node is set equal to the inverse cumulative
// probability of its edge weight.  This method resets the current
// node's position to equal the weighted sum of the attractive
// displacements towards each neighboring node.

void graph::attractive_force(prob_distribution& edgeweight_prob)
{
//   cout << "inside graph::attractive_force()" << endl;

   vector<twovector> new_node_posns;
   for (unsigned int n=0; n<get_n_nodes(); n++)
   {
      node* curr_node_ptr=get_ordered_node_ptr(n);
      twovector curr_posn(curr_node_ptr->get_posn());

      twovector numer(0,0);
      double denom=0;
      
      vector<graph_edge*> graph_edge_ptrs=
         curr_node_ptr->get_graph_edge_ptrs();
      for (unsigned int e=0; e<graph_edge_ptrs.size(); e++)
      {
         graph_edge* curr_edge_ptr=graph_edge_ptrs[e];
         double curr_weight=curr_edge_ptr->get_weight();
         int neighbor_node_ID=curr_node_ptr->get_neighbor_node_ID(
            curr_edge_ptr);
         node* neighbor_node_ptr=get_node_ptr(neighbor_node_ID);
         twovector neighbor_posn(neighbor_node_ptr->get_posn());
         twovector delta_posn=neighbor_posn-curr_posn;
         
         int nbin=edgeweight_prob.get_bin_number(curr_weight);
         double curr_pcum=edgeweight_prob.get_pcum(nbin);
         double frac=curr_pcum;
         numer += frac*curr_weight*delta_posn;
         denom += curr_weight;

      } // loop over index e labeling edges for *curr_node_ptr

      twovector new_posn=curr_posn+numer/denom;
      new_node_posns.push_back(new_posn);
//      cout << "node n = " << n
//           << " n_edges = " << number_of_edges.back() << endl;
   } // loop over index n labeling nodes

   for (unsigned int n=0; n<get_n_nodes(); n++)
   {
      node* curr_node_ptr=get_ordered_node_ptr(n);
      curr_node_ptr->set_posn(new_node_posns[n]);
   } // loop over index n labeling nodes
}

// ---------------------------------------------------------------------
// Member function median_node_separation_distance() loops over all
// nodes within the current graph.  It computes the probability
// distribution for neighboring node separation distance.  The median
// neighboring node separation distance is returned by this method.

double graph::median_node_separation_distance()
{
//   cout << "inside graph::median_node_separation_distance()" << endl;

   vector<double> node_separation_distances;
   for (unsigned int n=0; n<get_n_nodes(); n++)
   {
      node* curr_node_ptr=get_ordered_node_ptr(n);
      twovector curr_posn(curr_node_ptr->get_posn());

      vector<int> neighbor_node_IDs=curr_node_ptr->get_neighbor_node_IDs();
      for (unsigned int i=0; i<neighbor_node_IDs.size(); i++)
      {
         node* neighbor_node_ptr=get_node_ptr(neighbor_node_IDs[i]);
         twovector neighbor_posn(neighbor_node_ptr->get_posn());
         twovector delta_posn=neighbor_posn-curr_posn;
         node_separation_distances.push_back(delta_posn.magnitude());
      } // loop over index i labeling neighbor nodes
   }

   int n_output_bins=100;
   double xlo=0;
   prob_distribution separation_distances_distribution(
      node_separation_distances,n_output_bins,xlo);

//   cout << "separation_distances_distribution.mu = " 
//        << separation_distances_distribution.mean() << endl;
//   cout << "separation_distances_distribution.sigma = " 
//        << separation_distances_distribution.std_dev() 
//        << endl;
//   cout << "separation_distances_distribution.median = " 
//        << separation_distances_distribution.median() 
//        << endl;
//   cout << "separation_distances_distribution.quartile_width = " 
//        << separation_distances_distribution.quartile_width() << endl;

   return separation_distances_distribution.median();
}

// ---------------------------------------------------------------------
// Member function repulsive_force() takes in length scale
// median_separation_distance as well as a minimum separation
// fraction.  Looping over all nodes within the graph, it performs a
// kdtree search lookup for any nearby nodes lying within
// min_separation_distance=min_separation_frac*median_separation_distance
// of a current node.  The radial separation for any such nearby node
// is increased to min_separation_distance.  We wrote this method in
// Feb 2010 in order to separate not only neighboring nodes but also
// non-neighbor but nearby nodes within the MIT 2.3K graph.

void graph::repulsive_force(
   double median_separation_distance,double min_separation_frac)
{
//   cout << "inside graph::repulsive_force()" << endl;

   const double min_separation_distance=min_separation_frac*
      median_separation_distance;

   vector<twovector> new_node_posns;
   for (unsigned int n=0; n<get_n_nodes(); n++)
   {
      node* curr_node_ptr=get_ordered_node_ptr(n);
      twovector curr_posn(curr_node_ptr->get_posn());

      vector<node*> nearby_node_ptrs;
      find_nearby_nodes(curr_node_ptr,min_separation_distance,
                        nearby_node_ptrs);

      twovector displacement(0,0);
      for (unsigned int i=0; i<nearby_node_ptrs.size(); i++)
      {
         node* nearby_node_ptr=nearby_node_ptrs[i];
         if (nearby_node_ptr->get_ID()==curr_node_ptr->get_ID()) continue;

         twovector nearby_posn(nearby_node_ptr->get_posn());
         twovector delta_posn=nearby_posn-curr_posn;
         displacement += min_separation_distance*delta_posn.unitvector();
      } // loop over index e labeling edges for *curr_node_ptr

      twovector new_posn=curr_posn-displacement;
      new_node_posns.push_back(new_posn);
//      cout << "node n = " << n
//           << " n_edges = " << number_of_edges.back() << endl;
   } // loop over index n labeling nodes

   for (unsigned int n=0; n<get_n_nodes(); n++)
   {
      node* curr_node_ptr=get_ordered_node_ptr(n);
      curr_node_ptr->set_posn(new_node_posns[n]);
   } // loop over index n labeling nodes
}

// ---------------------------------------------------------------------
// Member function repulsive_force_between_neighbor_nodes() takes in
// length scale median_separation_distance as well as a minimum
// separation fraction between neighboring nodes.  The distance
// between any 2 neighbors is forced to equal at least
// min_separation_frac*median_separation_distance.

void graph::repulsive_force_between_neighbor_nodes(
   double median_separation_distance,double min_separation_frac)
{
//   cout << "inside graph::repulsive_force_between_neighbor_nodes()" << endl;

   const double min_separation_distance=min_separation_frac*
      median_separation_distance;

   vector<twovector> new_node_posns;
   for (unsigned int n=0; n<get_n_nodes(); n++)
   {
      node* curr_node_ptr=get_ordered_node_ptr(n);
      twovector curr_posn(
         curr_node_ptr->get_Uposn(),curr_node_ptr->get_Vposn());

      twovector displacement(0,0);
      vector<int> neighbor_node_IDs=curr_node_ptr->get_neighbor_node_IDs();
      for (unsigned int i=0; i<neighbor_node_IDs.size(); i++)
      {
         node* neighbor_node_ptr=get_node_ptr(neighbor_node_IDs[i]);
         twovector neighbor_posn(neighbor_node_ptr->get_posn());
         twovector delta_posn=neighbor_posn-curr_posn;
         double separation_distance=delta_posn.magnitude();
         if (separation_distance < min_separation_distance)
         {
            displacement += min_separation_distance*delta_posn.unitvector();
         }
      } // loop over index e labeling edges for *curr_node_ptr

      twovector new_posn=curr_posn-displacement;
      new_node_posns.push_back(new_posn);
//      cout << "node n = " << n
//           << " n_edges = " << number_of_edges.back() << endl;
   } // loop over index n labeling nodes

   for (unsigned int n=0; n<get_n_nodes(); n++)
   {
      node* curr_node_ptr=get_ordered_node_ptr(n);
      curr_node_ptr->set_posn(new_node_posns[n]);
   } // loop over index n labeling nodes
}

// ---------------------------------------------------------------------
// Member function redistribute_nodes_in_angle() loops over all nodes
// in the current graph and computes their angles theta wrt the
// graph's COM.  It then evenly redistributes the angle of each node
// while preserving radius information.  

// In early Feb 2010, we experimented with using this approach to
// separating out supernodes within the MIT 2.3K supergraph.  But we
// have abandoned this approach...

void graph::redistribute_nodes_in_angle()
{
//   cout << "inside graph::redistribute_nodes_in_angle()" << endl;

   twovector graph_COM=node_COM();
 
// Compute angle of each node about graph_COM:

   vector<double> theta;
   vector<node*> node_ptrs;
   for (unsigned int i=0; i<get_n_nodes(); i++)
   {
      node* curr_node_ptr=get_ordered_node_ptr(i);
      twovector curr_posn(curr_node_ptr->get_posn());
      twovector rel_curr_posn(curr_posn-graph_COM);
      double curr_theta=atan2(rel_curr_posn.get(1),rel_curr_posn.get(0));
      theta.push_back(curr_theta);
      node_ptrs.push_back(curr_node_ptr);
   } // loop over index i labeling nodes within current cluster

   templatefunc::Quicksort(theta,node_ptrs);
   double delta_theta=2*PI/theta.size();

// Evenly redistribute angles of all nodes about graph_COM:

   for (unsigned int i=0; i<get_n_nodes(); i++)
   {
      node* curr_node_ptr=get_ordered_node_ptr(i);
      double modified_theta=theta[0]+i*delta_theta;
         
      twovector curr_posn(curr_node_ptr->get_posn());
      twovector rel_curr_posn(curr_posn-graph_COM);
      double curr_radius=rel_curr_posn.magnitude();
      twovector r_hat(cos(modified_theta),sin(modified_theta));
         
      curr_posn=graph_COM+curr_radius*r_hat;
      curr_node_ptr->set_posn(curr_posn);
   } // loop over index i labeling nodes 
}

// ---------------------------------------------------------------------
// Member function sink_heavy_degree_nodes()

// This method is no longer used as of 2/9/10

void graph::sink_heavy_degree_nodes(
   double median_radius,double median_degree)
{
   cout << "inside graph::sink_heavy_degree_nodes()" << endl;

   unsigned int n_clusters=clusters_map.size();
   cout << "n_clusters = " << n_clusters << endl;

   for (unsigned int parent_ID=0; parent_ID<n_clusters; parent_ID++)
   {
      vector<int> clustered_node_IDs=clusters_map[parent_ID];

      vector<double> U,V;
      for (unsigned int i=0; i<clustered_node_IDs.size(); i++)
      {
         node* curr_node_ptr=get_node_ptr(clustered_node_IDs[i]);
         U.push_back(curr_node_ptr->get_Uposn());
         V.push_back(curr_node_ptr->get_Vposn());

      } // loop over index i labeling nodes within current cluster
      double Uavg=mathfunc::mean(U);
      double Vavg=mathfunc::mean(V);
      twovector COM(Uavg,Vavg);
   
// Contract radius of a node if it exceeds the average cluster radius:

      for (unsigned int i=0; i<clustered_node_IDs.size(); i++)
      {
         node* curr_node_ptr=get_node_ptr(clustered_node_IDs[i]);
         twovector curr_posn(curr_node_ptr->get_posn());
         twovector rel_curr_posn(curr_posn-COM);
         double curr_radius=rel_curr_posn.magnitude();

         double curr_degree=curr_node_ptr->get_degree();
         double delta_degree=curr_degree-median_degree;

         double modified_radius=curr_radius;
         if (delta_degree >= 0)
         {
//            const double alpha=1;
            const double alpha=3;
            modified_radius = exp(-alpha*delta_degree)*curr_radius;
//            modified_radius = exp(-alpha*delta_degree)*median_radius;
         }
//         else
//         {
//            double ratio=median_degree/curr_degree;
//            const double max_ratio=3.0;
//            ratio=basic_math::min(ratio,max_ratio);
//            modified_radius = ratio*median_radius;
//         }
         curr_posn=COM+modified_radius*rel_curr_posn.unitvector();

         curr_node_ptr->set_posn(curr_posn);
      } // loop over index i labeling nodes within current cluster
   } // loop over index parent_ID labeling clusters
} 

// =========================================================================
// Graph parent/child member functions
// =========================================================================

// Member function max_degree_child_node() takes in the ID for some
// parent node within the current graph.  After looping over all
// children nodes of the specified parent, this method returns a
// pointer to the child node with the maximum degree.

node* graph::max_degree_child_node(int parent_node_ID,graph* child_graph_ptr)
{
   cout << "inside graph::max_degree_child_node()" << endl;

   double max_child_degree=NEGATIVEINFINITY;
   node* representative_child_node_ptr=NULL;

   node* parent_node_ptr=get_node_ptr(parent_node_ID);
   vector<int> children_node_IDs=parent_node_ptr->get_children_node_IDs(); 

   for (unsigned int n=0; n<children_node_IDs.size(); n++)
   {
      node* child_node_ptr=child_graph_ptr->get_node_ptr(
         children_node_IDs[n]);
      double child_degree=child_node_ptr->get_degree();
      if (child_degree > max_child_degree)
      {
         max_child_degree=child_degree;
         representative_child_node_ptr=child_node_ptr;
      }
   } // loop over index n labeling all nodes in graph

   return representative_child_node_ptr;
}

/*
// Member function max_degree_node_in_cluster() takes in the ID for
// some cluster of nodes in the current graph.  After looping over all
// nodes within the specified cluster, this method returns a pointer
// to the node with the maximum degree.

node* graph::max_degree_node_in_cluster(int parent_ID)
{
   cout << "inside graph::max_degree_node_in_cluster()" << endl;

   double max_degree=NEGATIVEINFINITY;
   node* representative_node_ptr=NULL;
   
   vector<int> clustered_node_IDs=clusters_map[parent_ID];
   for (unsigned int i=0; i<clustered_node_IDs.size(); i++)
   {
      node* curr_node_ptr=get_node_ptr(clustered_node_IDs[i]);
      double curr_degree=curr_node_ptr->get_degree();
      if (curr_degree > max_degree)
      {
         max_degree=curr_degree;
         representative_node_ptr=curr_node_ptr;
      }
   } // loop over index i labeling nodes within current cluster

   return representative_node_ptr;
}
*/

// ---------------------------------------------------------------------
// Member function max_relative_size_node_in_cluster() takes in the ID
// for some cluster of nodes in the current graph.  After looping over
// all nodes within the specified cluster, this method returns a
// pointer to the node with the maximum relative_size.

node* graph::max_relative_size_node_in_cluster(int parent_ID)
{
   cout << "inside graph::max_relative_size_node_in_cluster()" << endl;

   double max_relative_size=NEGATIVEINFINITY;
   node* representative_node_ptr=NULL;
   
   vector<int> clustered_node_IDs=clusters_map[parent_ID];
   for (unsigned int i=0; i<clustered_node_IDs.size(); i++)
   {
      node* curr_node_ptr=get_node_ptr(clustered_node_IDs[i]);
      double curr_relative_size=curr_node_ptr->get_relative_size();
      if (curr_relative_size > max_relative_size)
      {
         max_relative_size=curr_relative_size;
         representative_node_ptr=curr_node_ptr;
      }
   } // loop over index i labeling nodes within current cluster

   return representative_node_ptr;
}

// ---------------------------------------------------------------------
// Member function transfer_children_IDs_to_parent_nodes() loops over
// all clusters within the current graph.  It sets each parent node's
// children_node_IDs equal to clusters_map[parent_ID].

void graph::transfer_children_IDs_to_parent_nodes(graph* parent_graph_ptr)
{
//   cout << "inside graph::transfer_children_IDs_to_parent_nodes()" << endl;

   int n_total_children=0;
   for (CLUSTERS_MAP::iterator itr=clusters_map.begin();
        itr != clusters_map.end(); ++itr)
   {
      int parent_node_ID=itr->first;
      node* parent_node_ptr=parent_graph_ptr->get_node_ptr(parent_node_ID);
      parent_node_ptr->set_children_node_IDs(clusters_map[parent_node_ID]);
      n_total_children += parent_node_ptr->get_n_children();
//      cout << "parent_node_ID = " << parent_node_ID
//           << " n_children = " << parent_node_ptr->get_n_children() << endl;
   }
//   cout << "n_total_children = " << n_total_children << endl;
}

// ---------------------------------------------------------------------
// Member function inherit_cluster_colors() first sets
// n_distinct_colors equal to n_distinct_colors for input
// *parent_graph_ptr.  Looping over all nodes within the current
// graph, this method also transfers the cluster's color to each of
// its child nodes.

void graph::inherit_cluster_colors(graph* parent_graph_ptr)
{
//   cout << "inside graph::inherit_cluster_colors()" << endl;

   n_distinct_colors=parent_graph_ptr->get_n_distinct_colors();

   for (unsigned int n=0; n<get_n_nodes(); n++)
   {
      node* curr_node_ptr=get_ordered_node_ptr(n);
      int parent_ID=curr_node_ptr->get_parent_ID();
      if (parent_ID < 0) continue;

      node* parent_node_ptr=parent_graph_ptr->get_node_ptr(parent_ID);
      compute_cluster_color(parent_node_ptr);

      curr_node_ptr->set_color_ID(parent_node_ptr->get_color_ID());
      curr_node_ptr->set_node_RGB(parent_node_ptr->get_node_RGB());
      
//      cout << "n = " << n 
//           << " parent_ID = " << parent_ID 
//           << " color_ID = " << curr_node_ptr->get_color_ID()
//           << " node_RGB = " << curr_node_ptr->get_node_RGB()
//           << endl;

   } // loop over index n labeling nodes within current graph
}

// ---------------------------------------------------------------------
// Member function inherit_parent_colors() first sets
// n_distinct_colors equal to n_distinct_colors for input
// *parent_graph_ptr.  Looping over all nodes within the current
// graph, this method also transfers the parent's color to each of
// its child nodes.

void graph::inherit_parent_colors(graph* parent_graph_ptr)
{
//   cout << "inside graph::inherit_parent_colors()" << endl;

   n_distinct_colors=parent_graph_ptr->get_n_distinct_colors();

   for (unsigned int n=0; n<get_n_nodes(); n++)
   {
      node* curr_node_ptr=get_ordered_node_ptr(n);
      int parent_ID=curr_node_ptr->get_parent_ID();
      if (parent_ID < 0) continue;

      node* parent_node_ptr=parent_graph_ptr->get_node_ptr(parent_ID);

      curr_node_ptr->set_color_ID(parent_node_ptr->get_color_ID());
      curr_node_ptr->set_node_RGB(parent_node_ptr->get_node_RGB());
      
//      cout << "n = " << n 
//           << " parent_ID = " << parent_ID 
//           << " color_ID = " << curr_node_ptr->get_color_ID()
//           << " node_RGB = " << curr_node_ptr->get_node_RGB()
//           << endl;

   } // loop over index n labeling nodes within current graph
}

// ---------------------------------------------------------------------
// Member function resize_nodes_based_on_n_children() sets the
// relative_size of each node within the current graph equal to the
// natural log of its number of children.  We wrote this method for
// super graph display purposes.

void graph::resize_nodes_based_on_n_children(double prefactor)
{
   cout << "inside graph::resize_nodes_based_on_n_children()" << endl;

   for (unsigned int n=0; n<get_n_nodes(); n++)
   {
      node* curr_node_ptr=get_ordered_node_ptr(n);
      double n_children=curr_node_ptr->get_n_children();
      double size=prefactor*log(n_children);
      size=basic_math::max(0.5,size);
      cout << "n_children = " << n_children 
           << " size = " << size << endl;
      curr_node_ptr->set_relative_size(size);
   }
}

// -------------------------------------------------------------------------
// Member function compute_node_positions_weighted_by_relative_size()
// loops over clusters within the current "child" graph (for which
// clusters_map must be filled with valid data).  It sets the position
// of each parent node within input *parents_graph_ptr equal to the
// average of the children node positions weighted by their relative
// size.

void graph::compute_node_positions_weighted_by_relative_size(
   graph* cluster_graph_ptr,graph* child_graph_ptr,graph* parents_graph_ptr)
{
   cout << "inside graph::compute_node_positions_weighted_by_relative_size()" 
        << endl;

// First generate parent graph's nodes using precalculated information
// stored within clusters_map:

   for (unsigned int n=0; n<parents_graph_ptr->get_n_nodes(); n++)
   {
      node* parent_node_ptr=parents_graph_ptr->get_ordered_node_ptr(n);
      int parent_ID=parent_node_ptr->get_ID();

      bool take_medians_flag=false;
      twovector cluster_COM=graphfunc::compute_cluster_COM(
         cluster_graph_ptr,child_graph_ptr,parent_ID,take_medians_flag);

      parent_node_ptr->set_posn(cluster_COM);
   } // loop over index n labeling grandparent clusters
}

// ---------------------------------------------------------------------
// Member function hierarchical_grandparents_clustering() is assumed
// to work on a current "parents" graph whose children nodes are
// known.  It takes in *grandparents_graph_ptr whose grandchildren
// nodes are also assumed to be known.  In general, we assume that the
// coarse and finer clustering used to form the grandparents and
// parents graph are not strictly hierarchical.  Candidate grandparent
// IDs flow to candidate parents via the common children base layer.
// This method implements a "voting" approach to determine which
// parent nodes should be grouped together into a grandparent cluster
// based upon their common children.  Each parent node within the
// current graph is assigned a unique grandparent ancestor.

void graph::hierarchical_grandparents_clustering(
   graph* base_graph_ptr,graph* grandparents_graph_ptr)
{
   cout << "inside graph::hierarchical_grandparents_clustering()" << endl;
   cout << "get_n_nodes() = " << get_n_nodes() << endl;

   typedef map<int,int> CHILD_PARENT_MAP;
   CHILD_PARENT_MAP child_parent_map;

// Loop over current parents map clusters.  Store parent cluster ID as
// a function of child node ID within child_parent_map:

   for (CLUSTERS_MAP::iterator itr=base_graph_ptr->get_clusters_map().begin();
        itr != base_graph_ptr->get_clusters_map().end(); ++itr)
   {
      int parent_node_ID=itr->first;
      vector<int> children_node_IDs=itr->second;

      cout << "parent_node_ID = " << parent_node_ID
           << " children_node_IDs.size() = "
           << children_node_IDs.size() << endl;
      for (unsigned int c=0; c<children_node_IDs.size(); c++)
      {
         int curr_child_node_ID=children_node_IDs[c];
         child_parent_map[curr_child_node_ID]=parent_node_ID;
      } // loop over index c labeling children within current parent cluster
   } // loop over parent clusters

   typedef map<int,vector<int> > PARENT_GRANDPARENTS_MAP;
   PARENT_GRANDPARENTS_MAP parent_grandparents_map;

   typedef map<int,int> GRANDPARENTS_FREQ_MAP;
   GRANDPARENTS_FREQ_MAP grandparents_freq_map;
   
// Loop over grand parents map clusters.  Extract each grandparent's
// grandchildren.  Look up each grand child's parent.  Identify
// grandparents with parents via the children:

   for (CLUSTERS_MAP::iterator itr=grandparents_graph_ptr->get_clusters_map()
           .begin(); itr != grandparents_graph_ptr->get_clusters_map().end(); 
        ++itr)
   {
      int grandparent_node_ID=itr->first;
      vector<int> children_node_IDs=itr->second;

      for (unsigned int c=0; c<children_node_IDs.size(); c++)
      {
         int curr_child_ID=children_node_IDs[c];
         CHILD_PARENT_MAP::iterator iter=child_parent_map.find(
            curr_child_ID);               
         int curr_parent_ID=-1;
         if (iter != child_parent_map.end())
         {
            curr_parent_ID=iter->second;
         }

//         cout << "grandparent cluster ID = " << grandparent_node_ID
//              << " curr_parent_ID = " << curr_parent_ID << endl;

         PARENT_GRANDPARENTS_MAP::iterator pgp_iter=
            parent_grandparents_map.find(curr_parent_ID);               
         if (pgp_iter != parent_grandparents_map.end())
         {
            pgp_iter->second.push_back(grandparent_node_ID);
         }
         else
         {
            vector<int> V;
            V.push_back(grandparent_node_ID);
            parent_grandparents_map[curr_parent_ID]=V;
         }

      } // loop over index c labeling children within grandparent cluster
      cout << "---------------------------------------------------" << endl;
   } // loop over grandparent cluster IDs
   
// Iterate over all parents within parents_grandparents_map.  Sort
// grandparent IDs.  Uniquely assign each parent to its highest
// frequency grandparent:

   for (PARENT_GRANDPARENTS_MAP::iterator pgp_itr=
           parent_grandparents_map.begin();
        pgp_itr != parent_grandparents_map.end(); ++pgp_itr)
   {
      int parent_ID=pgp_itr->first;
      vector<int> candidate_grandparent_IDs=pgp_itr->second;
      std::sort(candidate_grandparent_IDs.begin(),
                candidate_grandparent_IDs.end());

      grandparents_freq_map.clear();

      for (unsigned int c=0; c<candidate_grandparent_IDs.size(); c++)
      {
         int curr_grandparent_ID=candidate_grandparent_IDs[c];

         GRANDPARENTS_FREQ_MAP::iterator freq_itr=
            grandparents_freq_map.find(curr_grandparent_ID);               
         if (freq_itr != grandparents_freq_map.end())
         {
            grandparents_freq_map[curr_grandparent_ID]=freq_itr->second+1;
         }
         else
         {
            grandparents_freq_map[curr_grandparent_ID]=1;
         }
      } // loop over index c labeling candidate grandparent ID

      cout << "parent_ID = " << parent_ID << endl;
      cout << "Candidate grandparent IDs = " << endl;
      templatefunc::printVector(candidate_grandparent_IDs);

      int unique_grandparent_ID=-1;
      int max_frequency=-1;
      for (GRANDPARENTS_FREQ_MAP::iterator freq_itr=
              grandparents_freq_map.begin();
           freq_itr != grandparents_freq_map.end(); ++freq_itr)
      {
         int curr_freq=freq_itr->second;
         if (curr_freq > max_frequency)
         {
            unique_grandparent_ID=freq_itr->first;
            max_frequency=curr_freq;
         }
         cout << "Grandparent ID = " << freq_itr->first
              << " frequency = " << curr_freq << endl;
      }

      cout << "Unique grandparent ID = " << unique_grandparent_ID << endl;
      cout << "---------------------------------------------------" << endl;

      node* parent_node_ptr=get_node_ptr(parent_ID);
      parent_node_ptr->set_parent_ID(unique_grandparent_ID);

      cout << "parent ID = " << parent_node_ptr->get_ID()
           << " grandparent ID = " << unique_grandparent_ID << endl;

   } // loop over parent IDs within parent_grandparents_map

   cout << "at end of graph::hierarchical_grandparents_clustering()" << endl;
}

// =========================================================================
// Graph path finding member functions
// =========================================================================

// Member function compute_Dijkstra_edge_weights() implements
// Dijkstra's algorithm for finding paths between nodes within a
// connected graph.

void graph::compute_Dijkstra_edge_weights(node* initial_node_ptr)
{
   cout << "inside graph::compute_Dijkstra_edge_weights()" << endl;

// Initialize greedy search:

   node* current_node_ptr=NULL;
   for (unsigned int n=0; n<get_n_nodes(); n++)
   {
      node* curr_node_ptr=get_ordered_node_ptr(n);
      if (curr_node_ptr->get_ID()==initial_node_ptr->get_ID())
      {
         curr_node_ptr->set_distance_from_start(0);
         curr_node_ptr->set_visited_status(node::current);
         current_node_ptr=curr_node_ptr;
      }
      else
      {
         curr_node_ptr->set_distance_from_start(POSITIVEINFINITY);
         curr_node_ptr->set_visited_status(node::unvisited);
      }
   } // loop over index n labeling nodes

   unsigned int n_visited_nodes=0;
   while (n_visited_nodes < get_n_nodes() && current_node_ptr != NULL)
   {
      if (n_visited_nodes%1000==0) cout << n_visited_nodes/1000 << " "
                                        << flush;
      compute_neighbor_node_distances_from_start(current_node_ptr);
      n_visited_nodes++;
      current_node_ptr=next_node_to_visit();
   } // while loop over unvisited nodes
   cout << endl;

   cout << "at end of graph::compute_Dijkstra_edge_weights()" << endl;
}

// ---------------------------------------------------------------------
// Member function compute_neighbor_node_distances_from_start() takes
// in *curr_node_ptr which is assumed to be simply connected to some
// initial node within the graph.  This method considers all unvisited
// neighbors of *curr_node_ptr and calculates their separation
// distance from the initial node.  The current node's visited_status
// is set equal to visited at the end of this method.

void graph::compute_neighbor_node_distances_from_start(node* curr_node_ptr)
{
//   cout << "inside graph::compute_neighbor_node_distances_from_start()" 
//        << endl;
//   cout << "curr_node_ptr->get_ID() = " << curr_node_ptr->get_ID() << endl;
   vector<int> neighbor_node_IDs=curr_node_ptr->get_neighbor_node_IDs();

   for (unsigned int n=0; n<neighbor_node_IDs.size(); n++)
   {
      node* neighbor_node_ptr=get_node_ptr(neighbor_node_IDs[n]);
//      cout << "n = " << n
//           <<  " neighbor_node_ptr->get_ID() = " 
//           << neighbor_node_ptr->get_ID() << endl;
//      cout << "status = " << neighbor_node_ptr->get_visited_status() 
//           << endl;

      if (neighbor_node_ptr->get_visited_status() != node::unvisited) 
         continue;
      graph_edge* curr_edge_ptr=get_edge_ptr(curr_node_ptr,neighbor_node_ptr);
      if (curr_edge_ptr==NULL) continue;
         
      double curr_neighbor_distance_from_start=
         curr_node_ptr->get_distance_from_start()+curr_edge_ptr->get_weight();
//      cout << "curr_neighbor_distance_from_start = "
//           << curr_neighbor_distance_from_start << endl;
      if (curr_neighbor_distance_from_start < neighbor_node_ptr->
          get_distance_from_start())
      {
         neighbor_node_ptr->set_distance_from_start(
            curr_neighbor_distance_from_start);
         neighbor_node_ptr->set_path_predecessor_ptr(curr_node_ptr);
      }

   } // loop over index n labeling neighbor node IDs
   curr_node_ptr->set_visited_status(node::visited);
}

// ---------------------------------------------------------------------
// Member function next_node_to_visit() loops over all nodes in the
// current graph.  It sets the unvisited node with the smallest
// separation distance to some initial node as the next node to visit.

node* graph::next_node_to_visit()
{
//   cout << "inside graph::next_node_to_visit()" << endl;

   double min_distance_from_start=2*POSITIVEINFINITY;
   node* next_node_ptr=NULL;
   for (unsigned int n=0; n<get_n_nodes(); n++)
   {
      node* curr_node_ptr=get_ordered_node_ptr(n);
      if (curr_node_ptr->get_visited_status() !=
          node::unvisited) continue;

      double curr_distance_from_start=
         curr_node_ptr->get_distance_from_start();
      if (curr_distance_from_start < min_distance_from_start)
      {
         min_distance_from_start=curr_distance_from_start;
         next_node_ptr=curr_node_ptr;
      }
   } // loop over index n labeling neighbor node IDs

//   if (next_node_ptr != NULL)
//   {
//      cout << "next_node_ptr->get_ID() = "
//           << next_node_ptr->get_ID() << endl;
//   }
   
   return next_node_ptr;
}

// ---------------------------------------------------------------------
// Member function node_separation_degree() takes in the ID for some
// starting node within a connected graph.  It first forms a new
// adjacency matrix whose binary entries equal 1 to indicate edge
// connectivity between neighboring nodes or 0 otherwise.  It then
// instantiates a temporary graph based upon the binary adjacency
// matrix.  After running Dijkstra's algorithm, it returns an STL
// vector containing integer step distances for each node in the graph
// from the input initial node.

vector<int> graph::node_separation_degree(int initial_node_ID)
{
//   cout << "inside graph::node_separation()" << endl;
//   cout << "initial_node_ID = " << initial_node_ID << endl;

// Form new adjacency matrix *A_ptr whose binary entries either equal
// 1 to indicate edge connectivity or 0 otherwise:

   genmatrix* A_ptr=new genmatrix(get_n_nodes(),get_n_nodes());
   A_ptr->clear_values();

   for (unsigned int r=0; r<get_n_nodes(); r++)
   {
      for (unsigned int c=0; c<get_n_nodes(); c++)
      {
         if (adjacency_matrix_ptr->get(r,c) > 0)
         {
            A_ptr->put(r,c,1);
         }
      }
   }
//   cout << "*A_ptr = " << *A_ptr << endl;
  
   graph g;
   graph* graph_ptr=g.generate_graph_from_specified_adjacency_matrix(
      A_ptr);

   node* start_node_ptr=graph_ptr->get_node_ptr(initial_node_ID);
   graph_ptr->compute_Dijkstra_edge_weights(start_node_ptr);

   vector<int> separation_degree_from_start;
   for (unsigned int n=0; n<graph_ptr->get_n_nodes(); n++)
   {
      node* curr_node_ptr=graph_ptr->get_ordered_node_ptr(n);
      separation_degree_from_start.push_back(
         curr_node_ptr->get_distance_from_start());
   }

   delete graph_ptr;
   return separation_degree_from_start;
}

// ---------------------------------------------------------------------
// Member function girvan_newman_dendogram() 
/*
The algorithm's steps for community detection are summarized below

   1. The betweenness of all existing edges in the network is calculated first.
   2. The edge with the highest betweenness is removed.
   3. The betweenness of all edges affected by the removal is recalculated.
   4. Steps 2 and 3 are repeated until no edges remain.

The fact that the only betweennesses being recalculated are only the
ones which are affected by the removal, may lessen the running time of
the process' simulation in computers. However, the betweenness
centrality must be recalculated with each step, or severe errors
occur. The reason is that the network adapts itself to the new
conditions set after the edge removal. For instance, if two
communities are connected by more than one edge, then there is no
guarantee that all of these edges will have high
betweenness. According to the method, we know that at least one of
them will have, but nothing more than that is known. By recalculating
betweennesses after the removal of each edge, it is ensured that at
least one of the remaining edges between two communities will always
have a high value.

The end result of the Girvan%Gâ%@Newman algorithm is a
dendrogram. As the Girvan%Gâ%@Newman algorithm runs, the
dendrogram is produced from the top down (i.e. the network splits up
into different communities with the successive removal of links). The
leaves of the dendrogram are individual nodes.  */

genmatrix* graph::girvan_newman_dendogram()
{
   cout << "inside graph::girvan_newman_dendogram()" << endl;
   
   genmatrix* dendogram_ptr=new genmatrix(
      get_n_graph_edges()+1,get_n_nodes());
   dendogram_ptr->initialize_values(-1);

   genmatrix* curr_adjacency_matrix_ptr=new genmatrix(*adjacency_matrix_ptr);

   for (unsigned int r=0; r<dendogram_ptr->get_mdim(); r++)
   {
//      cout << "r = " << r << endl;
      graph* graph_ptr=generate_graph_from_specified_adjacency_matrix(
         curr_adjacency_matrix_ptr);
      graph_ptr->compute_node_centralities();

      int subgroup_ID=0;
      for (unsigned int c=0; c<dendogram_ptr->get_ndim(); c++)
      {
//         cout << "c = " << c 
//              << " subgroup_ID = " << subgroup_ID << endl;
         node* curr_node_ptr=get_ordered_node_ptr(c);
         vector<int> separation_degree_from_start=
            graph_ptr->node_separation_degree(curr_node_ptr->get_ID());
//         cout << "separation_degree_from_start = " << endl;
//         templatefunc::printVector(separation_degree_from_start);
         for (unsigned int i=0; i<separation_degree_from_start.size(); i++)
         {
            double curr_separation_degree=separation_degree_from_start[i];
            if (curr_separation_degree > 0.5*POSITIVEINFINITY) continue;
            if (dendogram_ptr->get(r,i) < 0)
            {
               dendogram_ptr->put(r,i,subgroup_ID);
            }
         } // loop over index i labeling separation_degree_from_start
         subgroup_ID++;
      } // loop over index c labeling graph nodes

// Eliminate edge with minimal centrality from graph:

      graph_edge* edge_to_remove_ptr=
         graph_ptr->edge_with_minimal_centrality();
      cout << "Edge with min centrality = "
           << *edge_to_remove_ptr << endl;

// Chop out one edge from curr_adj_matrix

      delete graph_ptr;

   } // loop over index r labeling number of original graph edges
   
   delete curr_adjacency_matrix_ptr;

   cout << "At end of graph::GN_dendogram()" << endl;
   cout << "*dendogram_ptr = " << *dendogram_ptr << endl;

   return dendogram_ptr;
}

// ---------------------------------------------------------------------
// Member function compute_Astar_path() takes in starting and stopping
// node IDs.  

vector<int> graph::compute_Astar_path(int start_node_ID,int stop_node_ID)
{
   cout << "inside graph::compute_Astar_path()" << endl;
   cout << "Start_node_ID = " << start_node_ID << endl;
   cout << "Stop_node_ID = " << stop_node_ID << endl;

   vector<int> path_node_IDs;

   int SearchCount=0;
   const int NumSearches=1;
   AStarSearch<node> astarsearch;
   while (SearchCount < NumSearches)
   {
      
// Create start and goal states:

      node* nodeStart_ptr=get_node_ptr(start_node_ID);
      node* nodeStop_ptr=get_node_ptr(stop_node_ID);
      
      cout << "nodeStart_ptr = " << nodeStart_ptr << endl;
      cout << "nodeStop_ptr = " << nodeStop_ptr << endl;

      node nodeStart(*nodeStart_ptr);
      node nodeStop(*nodeStop_ptr);
      nodeStart.set_graph_ptr(this);
      nodeStop.set_graph_ptr(this);
      
//      vector<int> start_neighbor_IDs=nodeStart_ptr->get_neighbor_node_IDs();
//      vector<int> stop_neighbor_IDs=nodeStop_ptr->get_neighbor_node_IDs();

      cout << "nodeStart = " << nodeStart << endl;
      cout << "nodeStop = " << nodeStop << endl;
      
      astarsearch.SetStartAndGoalStates( nodeStart, nodeStop );

      unsigned int SearchState;
      unsigned int SearchSteps = 0;
      do
      {
         if (SearchSteps%1000==0) cout << SearchSteps/1000 << " " << flush;
         SearchState = astarsearch.SearchStep();
         SearchSteps++;
      }
      while( SearchState == AStarSearch<node>::SEARCH_STATE_SEARCHING );
      cout << endl;

      if (SearchState==AStarSearch<node>::SEARCH_STATE_SUCCEEDED)
      {
         cout << "Search found goal state" << endl;
         node* node_ptr = astarsearch.GetSolutionStart();

         int steps = 0;
         for( ;; )
         {
            path_node_IDs.push_back(node_ptr->get_ID());

            node_ptr = astarsearch.GetSolutionNext();
            if (node_ptr==NULL) break;
            steps++;
         };
         cout << "Solution steps " << steps << endl;

// Once you're done with the solution, you can free the node_ptrs up:

         astarsearch.FreeSolutionNodes();

         for (unsigned int i=0; i<path_node_IDs.size(); i++)
         {
            cout << "i = " << i << " path_node_ID = " << path_node_IDs[i]
                 << endl;
         }

//         draw_path();
      }
      else if( SearchState==AStarSearch<node>::SEARCH_STATE_FAILED) 
      {
         cout << "Search terminated. Did not find goal state" << endl;
      }

      // Display the number of loops the search went through
      cout << "SearchSteps : " << SearchSteps << endl;
      SearchCount++;
      astarsearch.EnsureMemoryFreed();
   }

   return path_node_IDs;
}

// =========================================================================
// Graph centrality member functions
// =========================================================================

// Member function read_node_centrality_info() parses a centrality
// text file generated by the Markov Centrality Algorithm (see
// http://micans.org/mcl ).  It assigns centrality IDs based upon the
// parsed input.

void graph::read_node_centrality_info(string centrality_filename)
{
   cout << "inside graph::read_node_centrality_info()" << endl;
   cout << "centrality_filename = " << centrality_filename << endl;

   filefunc::ReadInfile(centrality_filename);
   for (unsigned int n=0; n<filefunc::text_line.size(); n++)
   {
      vector<double> nodeID_centrality=
         stringfunc::string_to_numbers(filefunc::text_line[n]);
      int curr_node_ID=nodeID_centrality[0];
      double centrality=nodeID_centrality[1];
      node* node_ptr=get_node_ptr(curr_node_ID);
      node_ptr->set_centrality(centrality);
      cout << "nodeID = " << curr_node_ID
           << " centrality = " << centrality << endl;
   }
}

// ---------------------------------------------------------------------
// Member function edge_with_minimal_centrality() iterates over all
// graph edges within the current graph's graph_edges_map.  It returns
// a pointer to the edge with the minimal centrality.

graph_edge* graph::edge_with_minimal_centrality()
{
   double min_edge_centrality=POSITIVEINFINITY;
   graph_edge* min_central_edge_ptr=NULL;

   for (GRAPH_EDGES_MAP::iterator itr=graph_edges_map.begin();
        itr != graph_edges_map.end(); ++itr)
   {
      graph_edge* curr_edge_ptr=itr->second;
      double curr_centrality=curr_edge_ptr->get_centrality();
      if (curr_centrality < min_edge_centrality)
      {
         min_edge_centrality=curr_centrality;
         min_central_edge_ptr=curr_edge_ptr;
      }
   } // loop over curr graph's edges
   
   return min_central_edge_ptr;
}

// ---------------------------------------------------------------------
// Member function compute_node_centralities() exports the current
// graph's adjacency matrix as an edge list.  It then calls the Markov
// Cluster Library (see http://micans.org/mcl ) which computes the
// graph nodes' centralities (ignoring edge weights).  

void graph::compute_node_centralities()
{
   cout << "inside graph::compute_node_centralities()" << endl;

   string tmp_edgelist_filename="./tmp_edgelist.dat";
   double edgeweight_threshold=0;
   export_edgelist(tmp_edgelist_filename,edgeweight_threshold);

// Calculate parents graph nodes' centrality using MCL algorithm:

   string tmp_centrality_filename=".//tmp_centrality.dat";
   string unix_cmd="mcx ctty -abc "+tmp_edgelist_filename
      +" > "+tmp_centrality_filename;
//   cout << "unix_cmd = " << unix_cmd << endl;
   sysfunc::unix_command(unix_cmd);

// Sort centrality results based upon parent graph node IDs:

   filefunc::ReadInfile(tmp_centrality_filename);
   vector<int> node_ID;
   vector<double> centrality;
   for (unsigned int n=0; n<filefunc::text_line.size(); n++)
   {
      vector<double> substrings=stringfunc::string_to_numbers(
         filefunc::text_line[n]);
      node_ID.push_back(substrings[0]);
      centrality.push_back(substrings[1]);
      node* node_ptr=get_node_ptr(node_ID.back());
      node_ptr->set_centrality(centrality.back());
   } // loop over index n labeling lines in centrality file

   unix_cmd="rm "+tmp_edgelist_filename;
   sysfunc::unix_command(unix_cmd);
   unix_cmd="rm "+tmp_centrality_filename;
   sysfunc::unix_command(unix_cmd);
   
   templatefunc::Quicksort(node_ID,centrality);
   for (unsigned int n=0; n<node_ID.size(); n++)
   {
      cout << "node ID = " << node_ID[n]
           << " centrality = " << centrality[n] << endl;
   }
}

// ==========================================================================
// SQL output member functions
// ==========================================================================

// Member function write_SQL_insert_node_commands() generates SQL
// insert commands for graph nodes.  It returns the number of nodes
// exported to the SQL file.

int graph::write_SQL_insert_node_commands(
   int graph_hierarchy_ID,int connected_component,string SQL_node_filename)
{
//   cout << "inside graph::write_SQL_insert_node_commands()" << endl;

   int n_exported_nodes=0;

//   int n_orphan_nodes=0;
   ofstream SQL_node_stream;
   filefunc::openfile(SQL_node_filename,SQL_node_stream);

   string insert_cmd;
   for (unsigned int n=0; n<get_n_nodes(); n++)
   {
      node* node_ptr=get_ordered_node_ptr(n);
//      int source_ID=node_ptr->get_source_ID();
//      if (source_ID < 0) 
//      {
//         n_orphan_nodes++;
//         continue;
//      }

      if (!output_node_to_SQL(graph_hierarchy_ID,connected_component,
                              node_ptr,insert_cmd))
         continue;
      
      SQL_node_stream << insert_cmd << endl;
      n_exported_nodes++;
   } // loop over index n labeling current graph's nodes
   filefunc::closefile(SQL_node_filename,SQL_node_stream);

//    cout << "n_nodes = " << get_n_nodes() << endl;
//   cout << "n_orphan_nodes = " << n_orphan_nodes << endl;
//   outputfunc::enter_continue_char();
   return n_exported_nodes;
}

// ---------------------------------------------------------------------
// Member function output_node_to_SQL() takes in a particular
// *node_ptr.  It extracts the metadata from *node_ptr needed to
// populate the columns of the nodes table within the IMAGERY
// database.  If the node's gx or gy coordinates lie close to their
// default NEGATIVEINFINITY values, this boolean method returns false.

bool graph::output_node_to_SQL(
   int graph_hierarchy_ID,int connected_component,node* node_ptr,
   string& insert_command)
{
//   cout << "inside graph::output_node_to_SQL()" << endl;

   int node_ID=node_ptr->get_ID();
   int node_level=node_ptr->get_level();
   int parent_node_ID=node_ptr->get_parent_ID();
   int data_ID=node_ptr->get_data_ID();

   colorfunc::RGB node_RGB=node_ptr->get_node_RGB();
//   cout << "parent_ID = " << parent_ID << endl;
//   cout << "node_RGB = " << node_RGB << endl;
   
   double rel_size=node_ptr->get_relative_size();
   double gx=node_ptr->get_Uposn()+gxgy_offset.get(0);
   double gy=node_ptr->get_Vposn()+gxgy_offset.get(1);

//   cout << "gx = " << gx << " gy = " << gy << endl;

   if (nearly_equal(gx,NEGATIVEINFINITY,100) || nearly_equal(
      gy,NEGATIVEINFINITY,100))
   {
      return false;
   }
   
   insert_command = graphdbfunc::generate_insert_node_SQL_command(
      graph_hierarchy_ID,get_ID(),node_level,connected_component,
      node_ID,parent_node_ID,data_ID,node_RGB,rel_size,gx,gy);
   
//   cout << "insert_command = " << insert_command << endl;
   return true;
}

// ---------------------------------------------------------------------
// Member function write_SQL_insert_link_commands() generates SQL
// insert commands for graph edges.  It returns the number of edges
// exported to the output SQL file.

int graph::write_SQL_insert_link_commands(
   int graph_hierarchy_ID,string SQL_link_filename,
   int minimal_edge_weights_threshold)
{
//   cout << "inside graph::write_SQL_insert_link_commands()" << endl;

   compute_edge_weights_distribution(minimal_edge_weights_threshold);

   int n_exported_edges=0;
   ofstream SQL_link_stream;
   filefunc::openfile(SQL_link_filename,SQL_link_stream);
   for (GRAPH_EDGES_MAP::const_iterator iter=graph_edges_map.begin();
        iter != graph_edges_map.end(); iter++)
   {
      graph_edge* graph_edge_ptr=iter->second;
      int curr_matches=graph_edge_ptr->get_weight();
      if (curr_matches > minimal_edge_weights_threshold)
      {
         node* node1_ptr=graph_edge_ptr->get_node1_ptr();
         node* node2_ptr=graph_edge_ptr->get_node2_ptr();

// FAKE FAKE:  Sat Aut 20, 2016 at 2 pm

// Hardwire new edge coloring algorithm for trained neural network
// display purposes...

//         colorfunc::RGB edge_RGB=compute_edge_color(curr_matches);

         double max_matches = 100;
         double min_matches = 0;
         colorfunc::RGB edge_RGB=compute_edge_color(
            curr_matches, max_matches, min_matches);

// FAKE FAKE:  Sat Aug 20, 2016 at 2 pm

// Hardwire edge weight offset for trained neural network display
// purposes...

         double edge_weight = curr_matches;         
//         double edge_weight = curr_matches - 0.5*(max_matches + min_matches);

         graph_edge_ptr->set_edge_RGB(edge_RGB);
            
         SQL_link_stream << output_link_to_SQL(
            graph_edge_ptr->get_ID(),graph_hierarchy_ID,node1_ptr,node2_ptr,
            edge_weight,edge_RGB) << endl;
//            curr_matches,graph_edge_ptr->get_edge_RGB()) << endl;
         n_exported_edges++;
      } // curr_matches > 0 conditional
   } // loop over graph_edges_map iterator
      
   filefunc::closefile(SQL_link_filename,SQL_link_stream);
   return n_exported_edges;
}

// ---------------------------------------------------------------------
// Member function output_link_to_SQL() takes in two edge nodes along
// with edge color and weight information.  It converts this metadata
// into a SQL command which populates columns of the links table within
// the IMAGERY database.

string graph::output_link_to_SQL(
   int link_ID,int graph_hierarchy_ID,node* node1_ptr,node* node2_ptr,
   double weight,const colorfunc::RGB& link_RGB)
{
//   cout << "inside graph::output_link_to_SQL()" << endl;

   int level=node1_ptr->get_level();
   int source_node_ID=node1_ptr->get_ID();
   int target_node_ID=node2_ptr->get_ID();
   bool directed_link_flag=false;
   double relative_size=1.0;

   string SQL_command=graphdbfunc::generate_insert_link_SQL_command(
      graph_hierarchy_ID,get_ID(),level,link_ID,source_node_ID,target_node_ID,
      directed_link_flag,weight,link_RGB,relative_size);

//   cout << SQL_command << endl;
//   outputfunc::enter_continue_char();
   return SQL_command;
}
