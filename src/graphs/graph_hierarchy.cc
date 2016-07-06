// =========================================================================
// Graph_Hierarchy class member function definitions
// =========================================================================
// Last modified on 12/30/12; 5/26/13; 7/22/13; 4/5/14
// =========================================================================

#include <algorithm>
#include <map>
#include <string>

#include "general/filefuncs.h"
#include "graphs/graph.h"
#include "graphs/graph_edge.h"
#include "graphs/graphdbfuncs.h"
#include "graphs/graphfuncs.h"
#include "graphs/graph_hierarchy.h"
#include "graphs/node.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ostream;
using std::ofstream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void graph_hierarchy::allocate_member_objects()
{
}

void graph_hierarchy::initialize_member_objects()
{
   global_graph_edge_counter=0;
}		 

// ---------------------------------------------------------------------
graph_hierarchy::graph_hierarchy(int ID,int levelzero_graph_ID)
{
   allocate_member_objects();
   initialize_member_objects();
   this->ID=ID;
   this->levelzero_graph_ID=levelzero_graph_ID;
}

// ---------------------------------------------------------------------
// Copy constructor:

graph_hierarchy::graph_hierarchy(const graph_hierarchy& g)
{
   docopy(g);
}

graph_hierarchy::~graph_hierarchy()
{
//   cout << "inside graph_hierarchy destructor" << endl;
//   cout << "Graphs_map.size() = " << graphs_map.size() << endl;

   int graph_counter=0;
   unsigned int n_levels=get_n_levels();
   for (unsigned int level=0; level < n_levels; level++)
   {
      graph* curr_graph_ptr=get_graph_ptr(level);
      if (curr_graph_ptr==NULL) continue;

//      int curr_graph_ID=curr_graph_ptr->get_ID();
//      cout << "curr_graph_ptr = " << curr_graph_ptr << endl;
//      cout << "level = " << level << " graph_ID = "
//           << curr_graph_ID << endl;
      
      delete curr_graph_ptr;
      graph_counter++;
   }
}

// ---------------------------------------------------------------------
void graph_hierarchy::docopy(const graph_hierarchy& g)
{
   ID=g.ID;
}

// Overload = operator:

graph_hierarchy& graph_hierarchy::operator= (const graph_hierarchy& g)
{
   if (this==&g) return *this;
   docopy(g);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const graph_hierarchy& g)
{
   outstream << endl;
   outstream << "Graph hierarchy ID = " << g.ID << endl;
         
   for (unsigned int level=0; level<g.get_n_levels(); level++)
   {
      outstream << "===============================================" << endl;
      const graph* graph_ptr=g.get_graph_ptr(level);
      
      outstream << "level = " << level 
                << " graph_ptr = " << graph_ptr << endl;
      if (graph_ptr != NULL)
      {
         outstream << "*graph_ptr = " << *graph_ptr << endl;
      }
   } // loop over index level

   return outstream;
}

// =========================================================================
// Graph manipulation member functions
// =========================================================================

// Member function add_graph_ptr() inserts *graph_ptr into the graph
// hierarchy at the level stored within the input graph itself.

void graph_hierarchy::add_graph_ptr(graph* graph_ptr)
{
//   cout << "inside graph_hierarchy::add_graph_ptr()" << endl;

   int level=graph_ptr->get_level();

   GRAPHS_MAP::iterator iter=graphs_map.find(level);
   if (iter == graphs_map.end())
   {
      graphs_map[level]=graph_ptr;
   }
   else
   {
      iter->second=graph_ptr;
   }
//   cout << "get_n_levels() = " << get_n_levels() << endl;
}

// ---------------------------------------------------------------------
// Member function get_graph_ptr() returns the graph pointer
// corresponding to input level.

graph* graph_hierarchy::get_graph_ptr(int level)
{
   GRAPHS_MAP::iterator iter=graphs_map.find(level);      
   if (iter == graphs_map.end())
   {
      return NULL;
   }
   else
   {
      return iter->second;
   }
}

const graph* graph_hierarchy::get_graph_ptr(int level) const
{
   GRAPHS_MAP::const_iterator iter=graphs_map.find(level);      
   if (iter == graphs_map.end())
   {
      return NULL;
   }
   else
   {
      return iter->second;
   }
}

// =========================================================================
// Children retrieval member functions
// =========================================================================

// Member function get_children_node_ptrs() takes in some node within
// the graph corresponding to level curr_level.  This method returns
// an STL vector filled with pointers to nodes in the child graph
// corresponding to level curr_level-1.

vector<node*> graph_hierarchy::get_children_node_ptrs(node* curr_node_ptr)
{
   int curr_level=curr_node_ptr->get_level();
   vector<node*> children_node_ptrs;
   if (curr_level > 0) 
   {
      vector<int> child_IDs=curr_node_ptr->get_children_node_IDs();
      graph* child_graph_ptr=get_graph_ptr(curr_level-1);

      for (unsigned int n=0; n<child_IDs.size(); n++)
      {
         node* child_node_ptr=child_graph_ptr->get_node_ptr(child_IDs[n]);
         children_node_ptrs.push_back(child_node_ptr);
      }
   }
   return children_node_ptrs;
}

// ---------------------------------------------------------------------
// Member function get_parent_node_ptr() takes in some node at some
// level in the graph hierarchy.  If the node corresponds to the
// hierarchy's root, this method returns a NULL pointer.  Otherwise,
// it returns a pointer to the unique parent node for the input
// current node.

node* graph_hierarchy::get_parent_node_ptr(node* curr_node_ptr)
{
   if (curr_node_ptr==NULL) return NULL;

   unsigned int curr_level=curr_node_ptr->get_level();
   if (curr_level > get_n_levels()-1) return NULL;

   int parent_ID=curr_node_ptr->get_parent_ID();
   graph* parent_graph_ptr=get_graph_ptr(curr_level+1);

   node* parent_node_ptr=parent_graph_ptr->get_node_ptr(parent_ID);
   return parent_node_ptr;
}

// ---------------------------------------------------------------------
// Member function get_ancestor_node_ptrs() takes in some starting
// node at some level within the graph hierarchy.  This method follows
// that node up its ancestral chain.  It returns an STL vector with
// pointers to the starting node's parent, grand parent, great grand
// parent, etc.

vector<node*> graph_hierarchy::get_ancestor_node_ptrs(node* start_node_ptr)
{
//   cout << "inside graph_hierarchy::get_ancestor_node_ptrs()" << endl;
//   cout << "start_node_ptr = " << start_node_ptr << endl;
   
   vector<node*> ancestor_node_ptrs;

   if (start_node_ptr==NULL) 
   {
      cout << "Error in graph_hierarchy::get_ancestor_node_ptrs()"
           << endl;
      cout << "start_node_ptr = NULL!" << endl;
      exit(-1);
   }

   int start_level=start_node_ptr->get_level();
   unsigned int n_ancestor_levels=get_n_levels()-1-start_level;
   int curr_level=start_level;
   node* curr_node_ptr=start_node_ptr;
   for (unsigned int l=0; l<n_ancestor_levels; l++)
   {
      node* parent_node_ptr=get_parent_node_ptr(curr_node_ptr);
      if (parent_node_ptr==NULL) break;
      ancestor_node_ptrs.push_back(parent_node_ptr);
      curr_node_ptr=parent_node_ptr;
      curr_level++;
   }

   return ancestor_node_ptrs;
}

// ---------------------------------------------------------------------
// Member function source_node_ID() takes in some starting
// node at some level within the graph hierarchy.  It returns the ID
// for the node furthest up its ancestral chain.

int graph_hierarchy::source_node_ID(node* start_node_ptr)
{
//   cout << "inside graph_hierarchy::source_node_ID()" << endl;
   vector<node*> ancestor_node_ptrs=get_ancestor_node_ptrs(start_node_ptr);
   if (ancestor_node_ptrs.size()==0)
   {
      return -1;
   }
   else
   {
      return ancestor_node_ptrs.back()->get_ID();
   }
}

// ---------------------------------------------------------------------
// Member function compute_all_ancestors() iterates over every level
// within the graph hierarchy.  Within each level, this method loops
// over every node.  It computes the chain of ancestors (parent,
// grandparent, great grandparent, etc) for each node.  For each
// node, this method extracts and stores its source node ID.

void graph_hierarchy::compute_all_ancestors()
{
//   cout << "inside graph_hierarchy::compute_all_ancestors()" << endl;
   
   for (unsigned int level=0; level<get_n_levels(); level++)
   {
//      cout << "level = " << level << endl;
      int n_orphan_nodes=0;

      graph* curr_graph_ptr=get_graph_ptr(level);
//      cout << "Total number of nodes = " << curr_graph_ptr->get_n_nodes()
//           << endl;

      for (unsigned int n=0; n<curr_graph_ptr->get_n_nodes(); n++)
      {
         node* curr_node_ptr=curr_graph_ptr->get_ordered_node_ptr(n);
         
         vector<node*> ancestor_node_ptrs=
            get_ancestor_node_ptrs(curr_node_ptr);
         if (ancestor_node_ptrs.size() > 0)
         {
            curr_node_ptr->set_source_ID(ancestor_node_ptrs.back()->get_ID());
         }
         else
         {
            curr_node_ptr->set_source_ID(-1);
            n_orphan_nodes++;
         }
//         cout << "node level = " << curr_node_ptr->get_level()
//              << " curr node ID = " << curr_node_ptr->get_ID() << endl;

//         for (unsigned int a=0; a<ancestor_node_ptrs.size(); a++)
//         {
//            node* ancestor_node_ptr=ancestor_node_ptrs[a];
//            cout << "a = " << a 
//                 << " ancestor level = " << ancestor_node_ptr->get_level()
//                 << " ancestor ID = " << ancestor_node_ptr->get_ID() 
//                 << endl;
//         } // loop over index a labeling ancestor nodes
//         cout << endl;
      } // loop over index n labeling nodes within *curr_graph_ptr
//      cout << "n_orphan_nodes = " << n_orphan_nodes << endl << endl;

   } // loop over index l labeling levels
}

// ---------------------------------------------------------------------
// Member function get_descendant_node_ptrs() takes in node
// *curr_node_ptr whose level must be greater than input
// descendant_level.  This method performs a brute-force search over
// every node within the descendant_level graph and checks if
// *curr_node_ptr is an ancestor.  If so, the descendant node's
// pointer is returned within the output STL vector.

vector<node*> graph_hierarchy::get_descendant_node_ptrs(
   node* curr_node_ptr,int descendant_level)
{
   vector<node*> descendant_node_ptrs;
   int delta_level=curr_node_ptr->get_level()-descendant_level;
   if (delta_level > 0)
   {
      graph* descendant_graph_ptr=get_graph_ptr(descendant_level);
      for (unsigned int n=0; n<descendant_graph_ptr->get_n_nodes(); n++)
      {
         node* descendant_node_ptr=descendant_graph_ptr->
            get_ordered_node_ptr(n);

         vector<node*> ancestor_node_ptrs=
            get_ancestor_node_ptrs(descendant_node_ptr);
         node* ancestor_node_ptr=ancestor_node_ptrs[delta_level-1];
         if (ancestor_node_ptr==curr_node_ptr)
         {
            descendant_node_ptrs.push_back(descendant_node_ptr);
         }
      } // loop over index n labeling nodes within *descendant_graph_ptr

//      cout << "curr node level = " << curr_node_ptr->get_level()
//           << " curr node ID = " << curr_node_ptr->get_ID() << endl;

/*
      for (unsigned int d=0; d<descendant_node_ptrs.size(); d++)
      {
         node* descendant_node_ptr=descendant_node_ptrs[d];
//         cout << "descendant node level = " 
//              << descendant_node_ptr->get_level()
//              << " descendant node ID = "
//              << descendant_node_ptr->get_ID() << endl;
      } 
*/

   }
   return descendant_node_ptrs;
}

// ---------------------------------------------------------------------
// Member function rename_parental_cluster_labels() loops over every
// level except the lowest within the graph hierarchy.  This method
// extracts the maximal ID value from the preceding level's nodes.  It
// then adds this maximal ID as an offset to the current level.  The
// output from this method is a set of unique node IDs for the entire
// graph pyramid.
	
void graph_hierarchy::rename_parental_cluster_labels(int max_child_node_ID)
{
//   cout << "inside graph_hierarchy::rename_parental_cluster_labels()"
//        << endl;
   
   for (unsigned int l=1; l<get_n_levels(); l++)
   {
      graph* parent_graph_ptr=get_graph_ptr(l);
      graph* child_graph_ptr=get_graph_ptr(l-1);
//      int n_children_nodes=child_graph_ptr->get_n_nodes();
      int max_node_ID=child_graph_ptr->get_max_node_ID();
      if (l==1) max_node_ID=max_child_node_ID;

//      cout << "l = " << l 
//           << " max node ID = " << max_node_ID
//           << " n_children_nodes = " << n_children_nodes
//           << endl;
      for (unsigned int n=0; n<parent_graph_ptr->get_n_nodes(); n++)
      {
         node* parent_node_ptr=parent_graph_ptr->get_ordered_node_ptr(n);
         parent_node_ptr->set_ID(
            parent_node_ptr->get_ID()+max_node_ID+1);
      }
   } // loop over index l labeling order-of-magnitude cluster levels
}

// ---------------------------------------------------------------------
// Member function compute_order_of_magnitude_clusters() loops over
// all entries within input STL vector selected_mclcm_clusters which
// correspond to monotonically increasing (but not necessarily by +1!)
// integers.  It opens an output file named level_l_clusters.dat where
// l=0,1,2,3,....  The ID for the parent cluster is first written to
// the start of each line within the output file followed by the IDs
// for the cluster's children.

void graph_hierarchy::compute_order_of_magnitude_clusters(
   const vector<twovector>& selected_mclcm_clusters)
{
//   cout << "inside graph_hierarchy::compute_order_of_magnitude_clusters()"
//        << endl;
   
   int stop_level=-1;
   for (unsigned int l=0; l<selected_mclcm_clusters.size(); l++)
   {
      int start_level=selected_mclcm_clusters[l].get(0);
      string level_filename="level_"+stringfunc::number_to_string(l)
         +"_clusters.dat";

//      cout << "start_level = " << start_level 
//           << " stop_level = " << stop_level << endl;
//      cout << "level_filename = " << level_filename << endl;

      compute_descendants(start_level,stop_level,level_filename);
      stop_level=start_level;
   } // loop over index l labeling order-of-magnitude cluster levels
}

// ---------------------------------------------------------------------
// Member function compute_descendants() takes in indices start_level
// > stop_level coming from MCLCM.  For each node within the starting
// level graph, this method finds the node's descendants (which may be
// children, grand children, great grand children depending upon
// start_level-stop_level).  It records both the starting level node's
// ID as well as its descendants' IDs within output level_filename.

void graph_hierarchy::compute_descendants(
   int start_level,int stop_level,string level_filename)
{
//   cout << "inside graph_hierarchy::compute_descendants()" << endl;
//   cout << "start_level = " << start_level 
//        << " stop_level = " << stop_level << endl;

   if (stop_level >= start_level || stop_level < 0) return;

   ofstream level_stream;
   filefunc::openfile(level_filename,level_stream);

   graph* start_graph_ptr=get_graph_ptr(start_level);
   for (unsigned int n=0; n<start_graph_ptr->get_n_nodes(); n++)
   {
      if (n%500==0) cout << n << " " << flush;
      node* curr_node_ptr=start_graph_ptr->get_ordered_node_ptr(n);
      vector<node*> descendant_node_ptrs=get_descendant_node_ptrs(
         curr_node_ptr,stop_level);

      level_stream << curr_node_ptr->get_ID() << "         ";

//      cout << "curr node level = " << curr_node_ptr->get_level()
//           << " curr node ID = " << curr_node_ptr->get_ID() << endl;
      for (unsigned int d=0; d<descendant_node_ptrs.size(); d++)
      {
         node* descendant_node_ptr=descendant_node_ptrs[d];
//         cout << "d = " << d
//              << " descendant level = " << descendant_node_ptr->get_level()
//              << " descendant ID = " << descendant_node_ptr->get_ID() 
//              << endl;
         level_stream << descendant_node_ptr->get_ID() << " ";
      } // loop over index d labeling descendant nodes
      level_stream << endl;
//      cout << endl;
   } // loop over index n labeling nodes within *start_graph_ptr

   filefunc::closefile(level_filename,level_stream);
}

void graph_hierarchy::export_descendants(
   int start_level,int stop_level,string subdir)
{
//   cout << "inside graph_hierarchy::export_descendants()" << endl;
//   cout << "start_level = " << start_level 
//        << " stop_level = " << stop_level << endl;

   if (stop_level >= start_level || stop_level < 0) return;

   filefunc::dircreate(subdir);

   graph* start_graph_ptr=get_graph_ptr(start_level);
   for (unsigned int n=0; n<start_graph_ptr->get_n_nodes(); n++)
   {
      if (n%500==0) cout << n << " " << flush;
      node* curr_node_ptr=start_graph_ptr->get_ordered_node_ptr(n);
      vector<node*> descendant_node_ptrs=get_descendant_node_ptrs(
         curr_node_ptr,stop_level);

      string descendants_filename=subdir+"node_"+stringfunc::number_to_string(
         curr_node_ptr->get_ID())+".dat";
      ofstream descendants_stream;
      filefunc::openfile(descendants_filename,descendants_stream);

//      cout << "curr node level = " << curr_node_ptr->get_level()
//           << " curr node ID = " << curr_node_ptr->get_ID() << endl;
      for (unsigned int d=0; d<descendant_node_ptrs.size(); d++)
      {
         node* descendant_node_ptr=descendant_node_ptrs[d];
//         cout << "d = " << d
//              << " descendant level = " << descendant_node_ptr->get_level()
//              << " descendant ID = " << descendant_node_ptr->get_ID() 
//              << endl;
         descendants_stream << descendant_node_ptr->get_ID() << " ";
      } // loop over index d labeling descendant nodes
      descendants_stream << endl;
//      cout << endl;
      filefunc::closefile(descendants_filename,descendants_stream);
   } // loop over index n labeling nodes within *start_graph_ptr
}

// =========================================================================
// Hierarchy building member functions
// =========================================================================

// Member function generate_child_graph_from_MCLCM_cluster_info()
// parses an MCLCM cluster file generated by
// graph_hierarchy::compute_descendants().  

void graph_hierarchy::generate_child_graph_from_MCLCM_cluster_info(
   string cluster_filename)
{
   cout << "inside graph_hierarchy::generate_child_graph_from_MCLCM_cluster_info()" << endl;
   cout << "cluster_filename = " << cluster_filename << endl;

   int max_child_node_ID=-1;

   int level=0;
   graph* child_graph_ptr=new graph(levelzero_graph_ID,level);

   filefunc::ReadInfile(cluster_filename);
   for (unsigned int n=0; n<filefunc::text_line.size(); n++)
   {
      vector<double> values=stringfunc::string_to_numbers(
         filefunc::text_line[n]);

      for (unsigned int i=1; i<values.size(); i++)
      {
         int child_node_ID=values[i];
         int child_level=0;
         max_child_node_ID=basic_math::max(max_child_node_ID,child_node_ID);

         node* child_node_ptr=new node(child_node_ID,child_level);
         child_graph_ptr->add_node(child_node_ptr);
      }
   } // loop over index n labeling input file lines

   cout << "child_graph_ptr->get_n_nodes() = "
        << child_graph_ptr->get_n_nodes() << endl;
   cout << "max_child_node_ID = " << max_child_node_ID << endl;
   for (int m=0; m<=max_child_node_ID; m++)
   {
      node* child_node_ptr=child_graph_ptr->get_node_ptr(m);
      if (child_node_ptr==NULL)
      {
         cout << "No child node with ID = " << m 
              << " inside level_1_clusters.dat file" << endl;
      }
   }
//   outputfunc::enter_continue_char();
}

// ---------------------------------------------------------------------
// Member function generate_parent_graph_from_MCLCM_cluster_info()
// parses an MCLCM cluster file generated by
// graph_hierarchy::compute_descendants().  It first instantiates a
// new parent graph.  This method then extracts the node ID for
// parents from the first entry of each line within the input file.  A
// new parent node is added to the parent graph for each entry in the
// cluster file, and children node IDs are assigned to the new parent
// based upon the remaining entries in each line of the input file.
// This method also assigns the parent node's ID to each of its
// children within input *child_graph_ptr.

graph* graph_hierarchy::generate_parent_graph_from_MCLCM_cluster_info(
   string cluster_filename,int parent_graph_ID,int parent_level,
   graph* child_graph_ptr)
{
   cout << "inside graph_hierarchy::generate_parent_graph_from_MCLCM_cluster_info()" << endl;
   cout << "cluster_filename = " << cluster_filename << endl;

   graph* parent_graph_ptr=new graph(parent_graph_ID,parent_level);
   
   vector<int> children_node_IDs,missing_children_IDs;

   filefunc::ReadInfile(cluster_filename);
   for (unsigned int n=0; n<filefunc::text_line.size(); n++)
   {
      vector<double> values=stringfunc::string_to_numbers(
         filefunc::text_line[n]);

      int parent_node_ID=values[0];
      node* parent_node_ptr=new node(parent_node_ID,parent_level);
      parent_graph_ptr->add_node(parent_node_ptr);

      children_node_IDs.clear();
      for (unsigned int i=1; i<values.size(); i++)
      {
         int child_node_ID=values[i];
         children_node_IDs.push_back(child_node_ID);

         node* child_node_ptr=child_graph_ptr->get_node_ptr(child_node_ID);
         if (child_node_ptr != NULL)
         {
            child_node_ptr->set_parent_ID(parent_node_ID);
         }
         else
         {
            missing_children_IDs.push_back(child_node_ID);
         }
      }
      parent_node_ptr->set_children_node_IDs(children_node_IDs);

   } // loop over index n labeling input file lines

   for (unsigned int m=0; m<missing_children_IDs.size(); m++)
   {
      cout << "m = " << m << " missing_children_ID = " 
           << missing_children_IDs[m] << endl;
   }
   return parent_graph_ptr;
}

// ---------------------------------------------------------------------
// Member function compute_parent_graph_edges() loops over all nodes
// within input *parent_graph_ptr.  For each parent, node this method
// extracts its children node IDs.  For each child node, it extracts
// all edges to sibling neighbors.  For siblings who have different
// parents, this method adds the child-child edge weights to the
// corresponding parent-parent edge weight.  After all sibling
// children nodes have been visited, a new edge is added to the parent
// graph with the integrated edge weight.

void graph_hierarchy::compute_parent_graph_edges(
   graph* parent_graph_ptr,graph* child_graph_ptr)
{
//   cout << "inside graph_hierarchy::compute_parent_graph_edges()" << endl;

   unsigned int n_parents=parent_graph_ptr->get_n_nodes();
//   cout << "n_parents = " << n_parents << endl;
   
   for (unsigned int n=0; n<n_parents; n++)
   {
      node* parent_node_ptr=parent_graph_ptr->get_ordered_node_ptr(n);
      int parent_node_ID=parent_node_ptr->get_ID();

// PARENT_EDGE_WEIGHTS_MAP:  

//    int key = neighbor parent node ID
//    double value = parent-neighbor parent edge weight

      typedef map<int,double> PARENT_EDGE_WEIGHTS_MAP;
      PARENT_EDGE_WEIGHTS_MAP parent_edge_weights_map;

      vector<int> children_node_IDs=parent_node_ptr->get_children_node_IDs();
//      cout << "children_node_IDs.size() = " << children_node_IDs.size()
//           << endl;
      
      for (unsigned int i=0; i<children_node_IDs.size(); i++)
      {
         node* child_node_ptr=child_graph_ptr->get_node_ptr(
            children_node_IDs[i]);
         if (child_node_ptr==NULL) continue;
         
         vector<graph_edge*> child_graph_edge_ptrs=
            child_node_ptr->get_graph_edge_ptrs();
         for (unsigned int e=0; e<child_graph_edge_ptrs.size(); e++)
         {
            graph_edge* child_edge_ptr=child_graph_edge_ptrs[e];

            int child_neighbor_ID=child_node_ptr->get_neighbor_node_ID(
               child_edge_ptr);
            node* child_neighbor_node_ptr=child_graph_ptr->
               get_node_ptr(child_neighbor_ID);
            if (child_neighbor_node_ptr==NULL) continue;
            
            int parent_neighbor_ID=child_neighbor_node_ptr->get_parent_ID();
            if (parent_neighbor_ID==parent_node_ID) continue;
            if (parent_neighbor_ID < 0) continue;

            double child_weight=child_edge_ptr->get_weight();

            PARENT_EDGE_WEIGHTS_MAP::iterator iter=
               parent_edge_weights_map.find(parent_neighbor_ID);      
            if (iter==parent_edge_weights_map.end())
            {
               parent_edge_weights_map[parent_neighbor_ID]=child_weight;
            }
            else
            {
               parent_edge_weights_map[parent_neighbor_ID]=
                  parent_edge_weights_map[parent_neighbor_ID]+child_weight;
            }
         } // loop over e index labeling edges of *curr_node_ptr
      } // loop over index i labeling children of current parent node

      for (PARENT_EDGE_WEIGHTS_MAP::iterator iter=
              parent_edge_weights_map.begin(); 
           iter != parent_edge_weights_map.end(); iter++)
      {
         int parent_neighbor_ID=iter->first;
         double parent_edge_weight=iter->second;
         parent_graph_ptr->add_graph_edge(
            parent_node_ID,parent_neighbor_ID,parent_edge_weight);
      }
   } // loop over index n labeling parent nodes
   
//   cout << "parent_graph_ptr->get_n_nodes() = "
//        << parent_graph_ptr->get_n_nodes() << endl;
//   cout << "parent_graph_ptr->get_n_edges() = " 
//        << parent_graph_ptr->get_n_graph_edges() << endl;
}

// ---------------------------------------------------------------------
// Member function perturb_parent_node_positions() repulsively
// separates all nodes within *parents_graph_ptr from any nearby
// neighbors in order to declump the parental graph.

// COMMENT NEEDS WORK AS OF 5/2/10

void graph_hierarchy::perturb_parent_node_positions(
   graph* cluster_graph_ptr,graph* parents_graph_ptr,graph* child_graph_ptr)
{
   cout << "inside graph_hierarchy::perturb_parent_node_positions()" << endl;
//   cout << "parents_graph_ptr = " << parents_graph_ptr
//        << " child_graph_ptr = " << child_graph_ptr << endl;

   unsigned int n_iters=50;
//   cout << "Enter number of attractive/repulsive iterations to perform:" 
//        << endl;
//   cin >> n_iters;
   double max_separation_distance=10;

   double min_separation_frac=0.25;
//   cout << "Enter minimum separation fraction:" << endl;
//   cin >> min_separation_frac;
   
   for (unsigned int iter=0; iter<n_iters; iter++)
   { 
      parents_graph_ptr->generate_nodes_kdtree();
      double median_separation_distance=parents_graph_ptr->
         median_node_separation_distance();
      cout << "iter = " << iter << " median_separation_distance = "
           << median_separation_distance << endl;
      if (median_separation_distance > max_separation_distance) break;
      parents_graph_ptr->repulsive_force(
         median_separation_distance,min_separation_frac);
   } // loop over iter index
   
   graphfunc::adjust_child_node_posns_from_cluster_COMs(
      cluster_graph_ptr,child_graph_ptr,parents_graph_ptr);
   
   prob_distribution edgeweight_prob=child_graph_ptr->
      compute_edgeweight_distribution();
   
   n_iters=10;
//   cout << "Enter number of attrative/repulsive iterations to perform:" 
//        << endl;
//   cin >> n_iters;

   min_separation_frac=0.30;
//   cout << "Enter min_separation_frac:" << endl;
//   cin >> min_separation_frac;

   for (unsigned int iter=0; iter<n_iters; iter++)
   {
      double median_separation_distance=child_graph_ptr->
         median_node_separation_distance();
      cout << "iter = " << iter
           << " median_separation_distance = "
           << median_separation_distance << endl;

      child_graph_ptr->repulsive_force_between_neighbor_nodes(
         median_separation_distance,min_separation_frac);
      child_graph_ptr->attractive_force(edgeweight_prob);
      graphfunc::strengthen_clusters_in_layout(
         cluster_graph_ptr,child_graph_ptr);
   } // loop over iter index

   graphfunc::strengthen_clusters_in_layout(
      cluster_graph_ptr,child_graph_ptr);

// After perturbing original FM**3 layout, recompute layout for
// *parents_graph_ptr:

   graphfunc::set_parent_node_positions_based_on_children(
      parents_graph_ptr,child_graph_ptr);
}

// ---------------------------------------------------------------------
// Member function color_graph_levels() loops over each level within
// the current graph hierarchy starting from the top of the pyramid.
// Top level nodes are assigned pure hues.  Next-to-top level nodes
// have their hues varied slightly from their ancestors' as well as
// their intensities randomly altered.  All lower level nodes inherit
// their parents' colorings.

void graph_hierarchy::color_graph_levels()
{
   if (get_n_levels() < 3) return;
   
//   cout << "inside graph_hierarchy::color_graph_levels()" << endl;
   int n_levels = int(get_n_levels());
   for (int l=n_levels-1; l>=0; l--)
   {
      if (l==n_levels-1 && l >= 2)
      {
         get_graph_ptr(l)->assign_grandparent_node_colors();
      }
      else if (l==n_levels-2 && l >= 1)
      {
         get_graph_ptr(n_levels-2)->assign_parent_node_colors(
            get_graph_ptr(get_n_levels()-1));
      }
      else
      {
         get_graph_ptr(l)->inherit_parent_colors(get_graph_ptr(l+1));
      }
   }
}

// ---------------------------------------------------------------------
// Member function build_hierarchy() takes in the number of levels to
// generate within the current graph hierarchy.  It first parses
// SIFT matching information read in from the specified
// edgelist_filename and creates the level-0 ('children') graph.  It
// then loops over all parental levels within the hierarchy.  This
// method reads in level_0n_clusters.dat and uses it to cluster the
// parental levels.  It also computes and exports
// level_n-1n_clusters.dat.  Parental graph edges are calculated, and
// their node positions are based upon their childrens'.  Parent node
// sizes are scaled to (logarithmically?) reflect their number of
// children.

void graph_hierarchy::build_hierarchy(
   string edgelist_filename,unsigned int n_levels,int zeroth_datum_ID,
   string graphs_subdir,unsigned int n_connected_components,
   unsigned int connected_component,
   int max_child_node_ID,COMPONENTS_LEVELS_MAP& components_levels_map)
{
//   cout << "inside graph_hierarchy::build_hierarchy()" << endl;

// Parse SIFT matching information and generate level-0 graph:

   int graph_ID=levelzero_graph_ID+get_n_graphs();
   unsigned int level=0;
   cout << "Generating level 0 graph:" << endl;
   graph* child_graph_ptr=graphfunc::generate_graph_from_edgelist(
      edgelist_filename,graph_ID,level,zeroth_datum_ID,
      &global_graph_edge_counter);
   add_graph_ptr(child_graph_ptr);

   string connected_component_label="_C"+
      stringfunc::number_to_string(connected_component);
   string base_graph_layout_filename=
      graphs_subdir+"graph_XY_coords"+connected_component_label+".fm3_layout";
   string modified_base_graph_layout_filename=
      graphs_subdir+"graph_XY_coords"+connected_component_label+
      ".modified_fm3_layout";

   if (filefunc::fileexist(modified_base_graph_layout_filename))
   {
      base_graph_layout_filename=modified_base_graph_layout_filename;
   }
   
   child_graph_ptr->read_nodes_layout(base_graph_layout_filename);

// Loop over parent graphs starts here:

   string clusters_subdir=graphs_subdir;
   for (unsigned int l=0; l<n_levels-1; l++)
   {
      level++;

      cout << "Processing level = " << level 
           << " for connected graph component " << connected_component
           << endl;

      int parent_graph_ID=levelzero_graph_ID+get_n_graphs();
      child_graph_ptr->set_parent_identity(parent_graph_ID);

      graph* parent_graph_ptr=new graph(parent_graph_ID,level);
      parent_graph_ptr->set_graph_edge_counter_ptr(&global_graph_edge_counter);
      add_graph_ptr(parent_graph_ptr);

      string input_cluster_filename=clusters_subdir+
         "level_0"+stringfunc::number_to_string(level)+"_clusters"
         +connected_component_label+".dat";

//      cout << "l = " << l
//           << " input_cluster_filename = " << input_cluster_filename
//           << endl;

      if (l > 1)
      {
         input_cluster_filename=clusters_subdir+
            "level_"+stringfunc::number_to_string(l-1)+
            stringfunc::number_to_string(l+1)+"_clusters"+
            connected_component_label+".dat";
      }
//      cout << "l = " << l 
//           << " input_cluster_filename = " << input_cluster_filename
//           << endl;

      int min_child_node_ID=child_graph_ptr->get_min_node_ID();
      int min_parent_node_ID=max_child_node_ID+1;

// Make sure min_parent_node_ID is unique for each connected graph
// component at each graph level!

      for (unsigned int lprime=1; lprime<level; lprime++)
      {
         for (unsigned int cprime=0; cprime<n_connected_components; cprime++)
         {
            pair<int,int> p(cprime,lprime);

            COMPONENTS_LEVELS_MAP::iterator iter=components_levels_map.find(p);
            int k_clusters=iter->second.first;
            min_parent_node_ID += k_clusters;
         } // loop over index cprime labeling connected graph components
      } // loop over index lprime labeling levels below current level

      for (unsigned int cprime=0; cprime<connected_component; cprime++)
      {
         pair<int,int> p(cprime,level);
         COMPONENTS_LEVELS_MAP::iterator iter=
            components_levels_map.find(p);
         int k_clusters=iter->second.first;
         min_parent_node_ID += k_clusters;
      } // loop over index cprime labeling connected graph components
//      cout << "min_parent_node_ID = " << min_parent_node_ID << endl;
//      outputfunc::enter_continue_char();

      parent_graph_ptr->read_cluster_info(
         input_cluster_filename,min_parent_node_ID);
      graph* cluster_graph_ptr=parent_graph_ptr;

      if (l > 0)
      {

// Convert level_03_clusters.dat into level_13_clusters.dat,  
// or level_04_clusters.dat into level_24_clusters.dat, etc

         int grandparent_graph_ID=parent_graph_ID+1;
         graph* grandparent_graph_ptr=new graph(grandparent_graph_ID,level+1);

         string input_grandcluster_filename=clusters_subdir+
            "level_0"+stringfunc::number_to_string(level+1)+"_clusters"+
            connected_component_label+".dat";
//         cout << "input_grandcluster_filename = " 
//              << input_grandcluster_filename << endl;

         grandparent_graph_ptr->read_cluster_info(
            input_grandcluster_filename,min_parent_node_ID);

         graphfunc::hierarchical_grandparents_clustering(
            child_graph_ptr,grandparent_graph_ptr);
         child_graph_ptr->recompute_clusters_map_from_graph_nodes();

         string output_grandcluster_filename=clusters_subdir+
            "level_"+stringfunc::number_to_string(l)
            +stringfunc::number_to_string(l+2)+"_clusters"+
            connected_component_label+".dat";
//         cout << "l = " << l
//              << " output_grandcluster_filename = " 
//              << output_grandcluster_filename << endl;
         child_graph_ptr->export_cluster_info(output_grandcluster_filename);
         delete grandparent_graph_ptr;

// Reset clusters map within *child_graph_ptr back to previous form
// before hierarchical grandparent clustering was performed on
// *child_graph_ptr:

         string prev_cluster_filename=clusters_subdir+
            "level_"+stringfunc::number_to_string(l-1)
            +stringfunc::number_to_string(l)+"_clusters"+
            connected_component_label+".dat";
//         cout << "prev_cluster_filename = " << prev_cluster_filename << endl;
         child_graph_ptr->read_cluster_info(
            prev_cluster_filename,min_child_node_ID);

// Convert level_02_clusters.dat into level_12_clusters.dat, 
// or level_13_clusters.dat into level_23_clusters.dat, etc.

         graphfunc::hierarchical_grandparents_clustering(
            child_graph_ptr,parent_graph_ptr);
         child_graph_ptr->recompute_clusters_map_from_graph_nodes();

         string output_cluster_filename=clusters_subdir+
            "level_"+stringfunc::number_to_string(l)
            +stringfunc::number_to_string(l+1)+"_clusters"+
            connected_component_label+".dat";
//         cout << "l = " << l
//              << " output_cluster_filename = " << output_cluster_filename
//              << endl;
         child_graph_ptr->export_cluster_info(output_cluster_filename);

//         cout << "min_parent_node_ID = " << min_parent_node_ID << endl;
         parent_graph_ptr->read_cluster_info(
            output_cluster_filename,min_parent_node_ID);
      }

      graphfunc::generate_parent_graph_nodes_from_clusters(
         cluster_graph_ptr,parent_graph_ptr);
      graphfunc::propagate_cluster_IDs_to_input_graph_children(
         cluster_graph_ptr,child_graph_ptr);
      compute_parent_graph_edges(parent_graph_ptr,child_graph_ptr);
      graphfunc::set_parent_node_positions_based_on_children(
         parent_graph_ptr,child_graph_ptr);

/*
// As of 5/4/10, the following call to perturb_parent_node_posns()
// works OK for the MIT2317 but not for the MIT20K nor the
// winter/summer MIT sets...

      if (l==0)
      {
         cout << "perturbing parent node posns()" << endl;
         perturb_parent_node_positions(
            cluster_graph_ptr,parent_graph_ptr,child_graph_ptr);
      }
*/

      parent_graph_ptr->resize_nodes_based_on_n_children();
      child_graph_ptr=parent_graph_ptr;

   } // loop over index l labeling levels

   color_graph_levels();
}

// ---------------------------------------------------------------------
// Member function destroy_hierarchy() deletes all dynamically
// instantiated graphs within the current hierarchy and clears member
// STL map graphs_map.

void graph_hierarchy::destroy_hierarchy()
{
//   cout << "inside graph_hierarchy::destroy_hierarchy()" << endl;
   
   for (unsigned int l=0; l<get_n_levels(); l++)
   {
      delete get_graph_ptr(l);
   } // loop over index l labeling graph levels
   graphs_map.clear();

//   cout << "n_graphs = " << get_n_graphs() << endl;
}

/*
// ---------------------------------------------------------------------
// Member function output_JSON_files()

void graph_hierarchy::output_JSON_files(int n_levels,string bundler_IO_subdir)
{
   cout << "inside graph_hierarchy::output_JSON_files()" << endl;

// In order to run Michael Yee's GraphExplorer, we need to copy output
// JSON files into the appropriate subdirectory of
// apache-tomcat/webapps/photo/graphs:

   string tomcat_subdir="/usr/local/apache-tomcat/webapps/photo/graphs/";
   tomcat_subdir += filefunc::getbasename(bundler_IO_subdir);
   filefunc::dircreate(tomcat_subdir);

// Write all graphs within hierarchy to output JSON files:

   for (unsigned int l=0; l<n_levels; l++)
   {
      string json_filename=bundler_IO_subdir+
         "graph_level_"+stringfunc::number_to_string(l)+".json";
      cout << "json_filename = " << json_filename << endl;
      get_graph_ptr(l)->write_graph_json_file(json_filename);
      string unix_cmd="cp "+json_filename+" "+tomcat_subdir;
//   cout << "unix_cmd = " << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);
   }
}
*/

// ==========================================================================
// SQL command generation member functions
// ==========================================================================

// Member function write_SQL_insert_node_and_link_commands()

void graph_hierarchy::write_SQL_insert_node_and_link_commands(
   string SQL_subdir,int connected_component,
   int minimal_edge_weights_threshold,const twovector& gxgy_offset,
   vector<int>& n_total_nodes,vector<int>& n_total_links)
{
//   cout << "inside graph_hierarchy::write_SQL_insert_node_and_link_commands()"
//        << endl;
//   cout << "connected_component = " << connected_component << endl;
//   cout << "gxgy_offset = " << gxgy_offset << endl;

   string banner;
   ofstream SQL_node_stream,SQL_link_stream;
   for (unsigned int l=0; l<get_n_levels(); l++)
   {
      graph* graph_ptr=get_graph_ptr(l);
      graph_ptr->set_gxgy_offset(gxgy_offset);

      string connected_component_label="_C"+
         stringfunc::number_to_string(connected_component);

      string SQL_nodes_filename=SQL_subdir+
         "insert_level_"+stringfunc::number_to_string(l)+"_nodes"+
         connected_component_label+".sql";
      int n_curr_nodes=graph_ptr->write_SQL_insert_node_commands(
         get_ID(),connected_component,SQL_nodes_filename);
//      cout << "n_curr_nodes = " << n_curr_nodes << endl;
      n_total_nodes[l]=n_total_nodes[l]+n_curr_nodes;

      string SQL_links_filename=SQL_subdir+
         "insert_level_"+stringfunc::number_to_string(l)+"_links"+
         connected_component_label+".sql";
      int n_curr_links=graph_ptr->write_SQL_insert_link_commands(
         get_ID(),SQL_links_filename,minimal_edge_weights_threshold);
      n_total_links[l]=n_total_links[l]+n_curr_links;

      banner="Wrote "+SQL_nodes_filename;
      outputfunc::write_banner(banner);
      banner="Wrote "+SQL_links_filename;
//      outputfunc::write_banner(banner);
   } // loop over index l labeling graph_hierarchy level
}

// ---------------------------------------------------------------------
// Member function write_SQL_insert_graph_commands() takes in STL
// vectors containing total numbers of (disconnected) nodes and links
// as a function of level in the current graph hierarchy.  It exports
// this metadata to a SQL file which may subsequently be imported into
// the graphs table of the IMAGERY database.

void graph_hierarchy::write_SQL_insert_graph_commands(
   string SQL_subdir,const vector<int>& n_total_nodes,
   const vector<int>& n_total_links)
{
//   cout << "inside graph_hierarchy::write_SQL_insert_graph_commands()"
//        << endl;

   ofstream SQL_node_stream,SQL_link_stream;
   for (unsigned int l=0; l<get_n_levels(); l++)
   {
      graph* graph_ptr=get_graph_ptr(l);

      string SQL_graph_filename=SQL_subdir+
         "insert_level_"+stringfunc::number_to_string(l)+"_graph.sql";
      ofstream SQL_graph_stream;
      filefunc::openfile(SQL_graph_filename,SQL_graph_stream);
      
      string SQL_command=graphdbfunc::generate_insert_graph_SQL_command(
         get_ID(),graph_ptr->get_ID(),graph_ptr->get_level(),
         graph_ptr->get_parent_identity(),n_total_nodes[l],n_total_links[l]);
      SQL_graph_stream << SQL_command << endl;
      
      filefunc::closefile(SQL_graph_filename,SQL_graph_stream);

      string banner="Wrote "+SQL_graph_filename;
      outputfunc::write_banner(banner);

   } // loop over index l labeling graph_hierarchy level
}

void graph_hierarchy::write_SQL_update_graph_commands(
   string SQL_subdir,const vector<int>& n_total_nodes,
   const vector<int>& n_total_links)
{
//   cout << "inside graph_hierarchy::write_SQL_update_graph_commands()"
//        << endl;

   ofstream SQL_node_stream,SQL_link_stream;
   for (unsigned int l=0; l<get_n_levels(); l++)
   {
      graph* graph_ptr=get_graph_ptr(l);

      string SQL_graph_filename=SQL_subdir+
         "update_level_"+stringfunc::number_to_string(l)+"_graph.sql";
      ofstream SQL_graph_stream;
      filefunc::openfile(SQL_graph_filename,SQL_graph_stream);
      
      string SQL_command=graphdbfunc::generate_update_graph_SQL_command(
         get_ID(),graph_ptr->get_ID(),n_total_nodes[l],n_total_links[l]);
      SQL_graph_stream << SQL_command << endl;
      
      filefunc::closefile(SQL_graph_filename,SQL_graph_stream);

      string banner="Wrote "+SQL_graph_filename;
      outputfunc::write_banner(banner);

   } // loop over index l labeling graph_hierarchy level
}

// ---------------------------------------------------------------------
// Member function concatenate_SQL_insert_files() merges all SQL files
// containing insertion and/or update commands for graphs, nodes and
// links over various graph levels and connected components into
// single SQL files.

void graph_hierarchy::concatenate_SQL_insert_and_update_files(
   string graphs_subdir,unsigned int n_levels,int graph_component_ID,
   unsigned int n_connected_components)
{
   cout << "inside graph_hierarchy::concatenate_SQL_insert_and_update_files()" 
        << endl;

   string output_filename;
   ofstream outstream;

// Concatenate insert graph SQL files:

   output_filename=graphs_subdir+"insert_all_graphs.sql";
   filefunc::openfile(output_filename,outstream);

   for (unsigned int l=0; l<n_levels; l++)
   {
      string SQL_graph_filename=graphs_subdir+
         "insert_level_"+stringfunc::number_to_string(l)+"_graph.sql";
      filefunc::ReadInfile(SQL_graph_filename);
      for (unsigned int i=0; i<filefunc::text_line.size(); i++)
      {
         outstream << filefunc::text_line[i] << endl;
      }
   } // loop over index l labeling graph levels
   filefunc::closefile(output_filename,outstream);

   string banner="All graph SQL insertion commands written to "+
      output_filename;
   outputfunc::write_big_banner(banner);

// Concatenate update graph SQL files:

   output_filename=graphs_subdir+"update_all_graphs.sql";
   filefunc::openfile(output_filename,outstream);

   for (unsigned int l=0; l<n_levels; l++)
   {
      string SQL_graph_filename=graphs_subdir+
         "update_level_"+stringfunc::number_to_string(l)+"_graph.sql";
      filefunc::ReadInfile(SQL_graph_filename);
      for (unsigned int i=0; i<filefunc::text_line.size(); i++)
      {
         outstream << filefunc::text_line[i] << endl;
      }
   } // loop over index l labeling graph levels
   filefunc::closefile(output_filename,outstream);

   banner="All graph SQL update commands written to "+output_filename;
   outputfunc::write_big_banner(banner);

// Concatenate insert node SQL files:

   output_filename=graphs_subdir+"insert_all_nodes.sql";
   filefunc::openfile(output_filename,outstream);

   for (unsigned int l=0; l<n_levels; l++)
   {
      for (unsigned int c=0; c<n_connected_components; c++)
      {
         string SQL_node_filename=graphs_subdir+
            "insert_level_"+stringfunc::number_to_string(l)+"_nodes_C"+
            stringfunc::number_to_string(graph_component_ID+c)+".sql";
         filefunc::ReadInfile(SQL_node_filename);
         for (unsigned int i=0; i<filefunc::text_line.size(); i++)
         {
            outstream << filefunc::text_line[i] << endl;
         }
      } // loop over index c labeling connected graph components
   } // loop over index l labeling graph levels
   filefunc::closefile(output_filename,outstream);

   banner="All node SQL insertion commands written to "+output_filename;
   outputfunc::write_big_banner(banner);

// Concatenate insert link SQL files:

   output_filename=graphs_subdir+"insert_all_links.sql";
   filefunc::openfile(output_filename,outstream);

   for (unsigned int l=0; l<n_levels; l++)
   {
      for (unsigned int c=0; c<n_connected_components; c++)
      {
         string SQL_link_filename=graphs_subdir+
            "insert_level_"+stringfunc::number_to_string(l)+"_links_C"+
            stringfunc::number_to_string(graph_component_ID+c)+".sql";
         filefunc::ReadInfile(SQL_link_filename);
         for (unsigned int i=0; i<filefunc::text_line.size(); i++)
         {
            outstream << filefunc::text_line[i] << endl;
         }
      } // loop over index c labeling connected graph components
   } // loop over index l labeling graph levels
   filefunc::closefile(output_filename,outstream);

   banner="All link SQL insertion commands written to "+output_filename;
   outputfunc::write_big_banner(banner);
}

// ---------------------------------------------------------------------
// Member function
// write_SQL_insert_connected_component_annotation_commands()
// populates entries in the graph_annotations table of the IMAGERY
// database connected component information.

void graph_hierarchy::write_SQL_insert_connected_component_annotation_commands(
   string SQL_subdir,COMPONENTS_LEVELS_MAP& components_levels_map)
{
//   cout << "inside graph_hierarchy::write_SQL_insert_connected_component_annotation_commands()" << endl;

   string SQL_cc_annotations_filename=SQL_subdir+
      "insert_cc_annotations.sql";
   ofstream SQL_cc_annotation_stream;
   filefunc::openfile(SQL_cc_annotations_filename,SQL_cc_annotation_stream);

   for (COMPONENTS_LEVELS_MAP::iterator iter=
           components_levels_map.begin(); iter != 
           components_levels_map.end(); iter++)
   {
      pair<int,int> p=iter->first;
      int level=p.second;
      int n_nodes=iter->second.first;
      twovector gxgy_offset=iter->second.second;
      string cc_label=iter->second.third;
      vector<string> topic_labels=iter->second.fourth;

      int layout=0;	// SIFT graph display
      int graph_ID=level;
      string color="white";

//      cout << "Connected component ID = " << connected_component_ID
//           << " level = " << level
//           << " n_nodes = " << n_nodes << endl;

      double gx=0.5*1+gxgy_offset.get(0);
      double gy=-0.5+gxgy_offset.get(1);

      double annotation_size=1;
      string SQL_command=
         graphdbfunc::generate_insert_graph_annotation_SQL_command(
            get_ID(),graph_ID,level,layout,gx,gy,cc_label,
            color,annotation_size);
      SQL_cc_annotation_stream << SQL_command << endl;

      gy -= 0.25;
      cc_label=stringfunc::number_to_string(n_nodes)+" nodes";
      SQL_command=graphdbfunc::generate_insert_graph_annotation_SQL_command(
         get_ID(),graph_ID,level,layout,gx,gy,cc_label,color,annotation_size);
      SQL_cc_annotation_stream << SQL_command << endl;

      for (unsigned int t=0; t<topic_labels.size(); t++)
      {
         double gx=gxgy_offset.get(0)+0.5;
         double gy=gxgy_offset.get(1)+2.5-t*0.3;

         SQL_command=graphdbfunc::generate_insert_graph_annotation_SQL_command(
            get_ID(),graph_ID,level,layout,gx,gy,topic_labels[t],
            color,annotation_size);
         SQL_cc_annotation_stream << SQL_command << endl;
      }

   } // iterator loop
   
   filefunc::closefile(SQL_cc_annotations_filename,SQL_cc_annotation_stream);
      
   string banner="Wrote "+SQL_cc_annotations_filename;
   outputfunc::write_big_banner(banner);
}
