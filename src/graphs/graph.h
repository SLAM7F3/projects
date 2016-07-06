// ==========================================================================
// Header file for graph class
// ==========================================================================
// Last modified on 6/26/12; 6/4/13; 7/22/13; 4/3/14; 4/5/14
// ==========================================================================

#ifndef GRAPH_H
#define GRAPH_H

#include <map>
#include <set>
#include "color/colorfuncs.h"
#include "kdtree/kdtreefuncs.h"
#include "math/prob_distribution.h"

class graph_edge;
class genmatrix;
class node;

class graph
{

  public:

   typedef std::map<int,node*> NODES_MAP;
//   independent int var = node ID
//   dependent node* var = node pointer

   typedef std::map<int,graph_edge*> GRAPH_EDGES_MAP;
   typedef std::map<std::pair<int,int>,int> NODES_EDGE_MAP;
   typedef std::map<int,std::vector<int> > CLUSTERS_MAP;

   graph(int ID=0,int level=0);
   graph(const graph& g);
   ~graph();
   graph& operator= (const graph& g);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const graph& g);

// Set and get methods:

   void set_ID(int id);
   int get_ID() const;
   void set_level(int l);
   int get_level() const;
   void set_parent_identity(int id);
   int get_parent_identity() const;
   void set_graph_edge_counter_ptr(int* counter_ptr);

   int get_n_children() const;
   void pushback_child_ID(int id);
   std::vector<int>& get_child_IDs();
   const std::vector<int>& get_child_IDs() const;

   int get_n_distinct_colors() const;
   int get_n_clusters() const;
   CLUSTERS_MAP& get_clusters_map();
   const CLUSTERS_MAP& get_clusters_map() const;

   genmatrix* get_adjacency_matrix_ptr();
   const genmatrix* get_adjacency_matrix_ptr() const;

   void set_gxgy_offset(const twovector& offset);

// Node manipulation member functions:

   unsigned int get_n_nodes() const;
   void add_node(node* node_ptr);
   void erase_node(node* node_ptr);
   void delete_node(node* node_ptr);
   void purge_nodes();

   node* get_node_ptr(int node_ID);
   const node* get_node_ptr(int node_ID) const;
   node* get_ordered_node_ptr(int p);
   const node* get_ordered_node_ptr(int p) const;

   void reorder_nonnull_ptr_nodes();
   void reset_node_order(const std::vector<int>& ordered_node_ID);
   int get_node_ID_given_order(int p) const;
   int get_node_order_given_ID(int ID) const;

   bool node_in_graph(int ID) const;
   int get_first_node_ID() const;
   int get_max_node_ID() const;
   int get_min_node_ID() const;
   int get_max_edge_ID() const;
   std::vector<int> get_adjacent_edge_IDs(const std::vector<int>& node_IDs);

// Graph_Edge manipulation member functions:

   unsigned int get_n_graph_edges() const;
   graph_edge* add_graph_edge(int node1_ID,int node2_ID,double weight=1.0);
   graph_edge* add_graph_edge(node* n1_ptr,node* n2_ptr,double weight=1.0);
   void delete_graph_edge(graph_edge* graph_edge_ptr);
   void purge_graph_edges();

   graph_edge* get_graph_edge_ptr(int graph_edge_ID);
   const graph_edge* get_graph_edge_ptr(int graph_edge_ID) const;
   graph_edge* get_ordered_graph_edge_ptr(int p);
   const graph_edge* get_ordered_graph_edge_ptr(int p) const;

   bool graph_edge_in_graph(int graph_edge_ID) const;
   int get_edge_ID(int node1_ID,int node2_ID);
   graph_edge* get_edge_ptr(const node* node1_ptr,const node* node2_ptr);
   graph_edge* get_edge_ptr(int node1_ID,int node2_ID);

   void remove_graph_edge(graph_edge* graph_edge_ptr);
   double minimal_graph_edge_weight();
   double maximal_graph_edge_weight();

// Graph generation member functions:

   genmatrix* compute_adjacency_matrix();
   graph* generate_graph_from_specified_adjacency_matrix(
      genmatrix* adj_matrix_ptr);
   graph* create_clone();
   
   void generate_graph_nodes_from_clusters_map();
   void recompute_clusters_map_from_graph_nodes();

// GraphML/JSON output member functions:

   void compute_edge_weights_distribution(
      double minimal_edge_weight_threshold=0);
   colorfunc::RGB compute_edge_color(int n_SIFT_matches);
   void write_graph_json_file(std::string json_filename);
   void write_json_file(const std::string& value);
   void write_json_file(std::string json_filename,const std::string& value);

// Node properties I/O member functions:

   void export_edgelist(
      std::string edgelist_filename,double edgeweight_threshold);
   void export_edgelist(
      genmatrix* adjacency_matrix_ptr,std::string edgelist_filename,
      double edgeweight_threshold);
   void read_nodes_layout(std::string layout_filename);
   void read_cluster_info(std::string cluster_filename);
   void read_cluster_info(std::string cluster_filename,int min_parent_ID);
   void export_cluster_info(std::string output_cluster_filename);

// Node coloring member functions:

   void output_DIMACS_edgelist_file(std::string DIMACS_edgelist_filename);
   void run_Arbore_graph_coloring(std::string DIMACS_edgelist_filename);
   int read_Arbore_vertex_coloring(std::string vertex_coloring_filename);
   void compute_cluster_color(node* node_ptr);

   colorfunc::RGB compute_node_cluster_color(node* node_ptr,int parent_ID);
   colorfunc::RGB compute_node_centrality_color(node* node_ptr);

   void assign_grandparent_node_colors();
   bool assign_grandparent_node_colors(int prime_factor,int ID_offset);
   void assign_parent_node_colors(graph* grandparents_graph_ptr);

   void compute_ID_based_node_colors();

// Graph properties member functions:

   double compute_median_degree();
   prob_distribution compute_edgeweight_distribution();
   twovector node_COM();
   twovector node_std_dev_from_COM();
   void generate_nodes_kdtree();
   void find_nearby_nodes(
      node* curr_node_ptr,double radius,std::vector<node*>& nearby_node_ptrs);

// Graph layout member functions:

   void attractive_force(prob_distribution& edgeweight_prob);
   double median_node_separation_distance();
   void repulsive_force(double median_separation_distance,
                        double min_separation_frac);
   void repulsive_force_between_neighbor_nodes(
      double median_separation_distance,double min_separation_frac);
   void redistribute_nodes_in_angle();
   void sink_heavy_degree_nodes(double median_radius,double median_degree);

// Graph parent/child member functions:

   node* max_degree_child_node(int parent_ID,graph* child_graph_ptr);
   void transfer_children_IDs_to_parent_nodes(graph* parent_graph_ptr);
   void transfer_parent_IDs_to_children_nodes();
   void propagate_cluster_IDs_to_input_graph_children(
      graph* cluster_graph_ptr,graph* child_graph_ptr);

   void inherit_cluster_colors(graph* parent_graph_ptr);
   void inherit_parent_colors(graph* parent_graph_ptr);
   void resize_nodes_based_on_n_children(double prefactor=1.0);
   void compute_node_positions_weighted_by_relative_size(
      graph* cluster_graph_ptr,graph* child_graph_ptr,
      graph* parents_graph_ptr);
   void hierarchical_grandparents_clustering(
      graph* base_graph_ptr,graph* grandparents_graph_ptr);

   void populate_parent_graph_nodes_from_clusters(
      graph* cluster_graph_ptr,graph* parent_graph_ptr);

// Graph path finding member functions:

   void compute_Dijkstra_edge_weights(node* initial_node_ptr);
   void compute_neighbor_node_distances_from_start(node* curr_node_ptr);
   node* next_node_to_visit();

   std::vector<int> node_separation_degree(int initial_node_ID);
   genmatrix* girvan_newman_dendogram();
   std::vector<int> compute_Astar_path(int start_node_ID,int stop_node_ID);

// Centrality member functions:

   void read_node_centrality_info(std::string centrality_filename);   
   graph_edge* edge_with_minimal_centrality();
   void compute_node_centralities();

// SQL output member functions:

   int write_SQL_insert_node_commands(
      int graph_hierarchy_ID,int connected_component,
      std::string SQL_node_filename);
   bool output_node_to_SQL(
      int graph_hierarchy_ID,int connected_component,
      node* node_ptr,std::string& insert_command);

   int write_SQL_insert_link_commands(
      int graph_hierarchy_ID,std::string SQL_link_filename,
      int minimal_edge_weights_threshold);
   std::string output_link_to_SQL(
      int link_ID,int graph_hierarchy_ID,
      node* node1_ptr,node* node2_ptr,double weight,
      const colorfunc::RGB& edge_RGB);

  protected:

   NODES_MAP nodes_map;
   GRAPH_EDGES_MAP graph_edges_map;
   NODES_EDGE_MAP nodes_edge_map;
   CLUSTERS_MAP clusters_map;
   std::vector<int> edge_weights_threshold;


   std::string indent_spaces(unsigned int n_indent);
   std::string output_GraphML_key_value_pair(
      int n_indent,std::string key,std::string value,
      bool terminal_comma_flag=true);
   std::string output_node_GraphML(
      int n_indent,node* node_ptr,bool terminal_node_flag);
   std::string output_data_GraphML(
      int n_indent,double edge_weights,double r,double g,double b,
      double relative_size);
   std::string output_data_GraphML(
      int n_indent,std::string data_type,int time_stamp,
      double edge_weights,int parent_ID,const std::vector<int>& children_IDs,
      double gx,double gy,double r,double g,double b,double relative_size);
   std::string output_edge_GraphML(
      int n_indent,int i,int j,double edge_weights,double r,double g,double b,
      double relative_size,bool terminal_edge_flag);
   node* max_relative_size_node_in_cluster(int parent_ID);

  private: 

   int ID,level,parent_identity,n_distinct_colors;
   int graph_edge_counter;
   int* graph_edge_counter_ptr;
   twovector gxgy_offset;
   std::vector<int> child_IDs;
   std::vector<int> node_order,graph_edge_order;
   genmatrix* adjacency_matrix_ptr;

   KDTree::KDTree<2, threevector>* nodes_kdtree_ptr;
   
   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const graph& g);

   void add_graph_edge(graph_edge* graph_edge_ptr);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void graph::set_ID(int id)
{
   ID=id;
}

inline int graph::get_ID() const
{
   return ID;
}

inline void graph::set_level(int l)
{
   level=l;
}

inline int graph::get_level() const
{
   return level;
}

// ---------------------------------------------------------------------
inline void graph::set_parent_identity(int id)
{
   parent_identity=id;
}

inline int graph::get_parent_identity() const
{
   return parent_identity;
}

inline void graph::set_graph_edge_counter_ptr(int* counter_ptr)
{
   graph_edge_counter_ptr=counter_ptr;
}

inline int graph::get_n_children() const
{
   return child_IDs.size();
}

inline void graph::pushback_child_ID(int id)
{
   child_IDs.push_back(id);
}

inline std::vector<int>& graph::get_child_IDs() 
{
   return child_IDs;
}

inline const std::vector<int>& graph::get_child_IDs() const
{
   return child_IDs;
}

// ---------------------------------------------------------------------
inline int graph::get_n_distinct_colors() const
{
   return n_distinct_colors;
}

// ---------------------------------------------------------------------
inline int graph::get_n_clusters() const
{
   return clusters_map.size();
}

// ---------------------------------------------------------------------
inline unsigned int graph::get_n_nodes() const
{
   return nodes_map.size();
}

inline unsigned int graph::get_n_graph_edges() const
{
   return graph_edges_map.size();
}

// ---------------------------------------------------------------------
inline int graph::get_node_ID_given_order(int p) const
{
   return node_order[p];
}

// ---------------------------------------------------------------------
inline int graph::get_node_order_given_ID(int ID) const
{
   for (int p=0; p<int(node_order.size()); p++)
   {
      if (node_order[p]==ID) return p;
   }
   return -1;
}

// ---------------------------------------------------------------------
inline genmatrix* graph::get_adjacency_matrix_ptr()
{
   return adjacency_matrix_ptr;
}

inline const genmatrix* graph::get_adjacency_matrix_ptr() const
{
   return adjacency_matrix_ptr;
}

// ---------------------------------------------------------------------
inline graph::CLUSTERS_MAP& graph::get_clusters_map()
{
   return clusters_map;
}

inline const graph::CLUSTERS_MAP& graph::get_clusters_map() const
{
   return clusters_map;
}

// ---------------------------------------------------------------------
inline void graph::set_gxgy_offset(const twovector& offset)
{
   gxgy_offset=offset;
}

// ---------------------------------------------------------------------
// Member function get_node_ptr() returns the node pointer
// corresponding to input ID.

inline node* graph::get_node_ptr(int node_ID)
{
   NODES_MAP::iterator iter=nodes_map.find(node_ID);      
   if (iter == nodes_map.end())
   {
      return NULL;
   }
   else
   {
      return iter->second;
   }
}

inline const node* graph::get_node_ptr(int node_ID) const
{
   NODES_MAP::const_iterator iter=nodes_map.find(node_ID);      
   if (iter == nodes_map.end())
   {
      return NULL;
   }
   else
   {
      return iter->second;
   }
}

#endif  // graph.h
