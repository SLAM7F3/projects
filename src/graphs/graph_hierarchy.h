// ==========================================================================
// Header file for Graph_Hierarchy class
// ==========================================================================
// Last modified on 5/26/13; 7/22/13; 4/3/14; 4/5/14
// ==========================================================================

#ifndef GRAPH_HIERARCHY_H
#define GRAPH_HIERARCHY_H

#include <algorithm>
#include <map>
#include <string>
#include "color/colorfuncs.h"
#include "datastructures/Quadruple.h"
#include "math/twovector.h"

class graph;
class node;

class graph_hierarchy
{

  public:

   typedef std::map<int,graph*> GRAPHS_MAP;

   typedef Quadruple<int,twovector,std::string,std::vector<std::string> > 
      NNODES_POSN_LABELS;
   typedef std::map<std::pair<int,int>,NNODES_POSN_LABELS> 
      COMPONENTS_LEVELS_MAP;
//    Independent var = connected component ID,level
//    Dependent vars = n_nodes, (gx,gy), cc label, topic labels

   graph_hierarchy(int ID=0,int levelzero_graph_ID=0);
   graph_hierarchy(const graph_hierarchy& g);
   ~graph_hierarchy();
   graph_hierarchy& operator= (const graph_hierarchy& g);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const graph_hierarchy& g);

// Set and get member functions:

   void set_ID(int id);
   int get_ID() const;
   void set_label(std::string description);
   std::string get_label() const;
   unsigned int get_n_levels() const;
   unsigned int get_n_graphs() const;
   void add_graph_ptr(graph* graph_ptr);
   graph* get_graph_ptr(int level);
   const graph* get_graph_ptr(int level) const;

// Children retrieval member functions:

   std::vector<node*> get_children_node_ptrs(node* curr_node_ptr);
   node* get_parent_node_ptr(node* curr_node_ptr);
   std::vector<node*> get_ancestor_node_ptrs(node* start_node_ptr);
   int source_node_ID(node* start_node_ptr);
   void compute_all_ancestors();
   std::vector<node*> get_descendant_node_ptrs(
      node* curr_node_ptr,int descendant_level);
   void export_descendants(int start_level,int stop_level,std::string subdir);
   
   void rename_parental_cluster_labels(int max_child_node_ID);
   void compute_order_of_magnitude_clusters(
      const std::vector<twovector>& selected_mclcm_clusters);
   void compute_descendants(int start_level,int stop_level,
                            std::string level_filename);

// Hierarchy building member functions:

   graph* generate_parent_graph_from_MCLCM_cluster_info(
      std::string cluster_filename,int parent_graph_ID,int parent_level,
      graph* child_graph_ptr);
   void generate_child_graph_from_MCLCM_cluster_info(
      std::string cluster_filename);
   graph* generate_parent_graph(
      int parent_graph_ID,int parent_level,graph* child_graph_ptr);

   void compute_parent_graph_edges(
      graph* parent_graph_ptr,graph* child_graph_ptr);
   void perturb_parent_node_positions(
      graph* cluster_graph_ptr,graph* parents_graph_ptr,
      graph* child_graph_ptr);
   void color_graph_levels();
   void build_hierarchy(
      std::string edgelist_filename,unsigned int n_levels,int zeroth_datum_ID,
      std::string graphs_subdir,unsigned int n_connected_components,
      unsigned int connected_component,int max_child_node_ID,
      COMPONENTS_LEVELS_MAP& components_levels_map);

   void destroy_hierarchy();
//    void output_JSON_files(int n_levels,std::string bundler_IO_subdir);

// SQL commands member functions:

   void write_SQL_insert_node_and_link_commands(
      std::string SQL_subdir,int connected_component,
      int minimal_edge_weights_threshold,const twovector& gxgy_offset,
      std::vector<int>& n_total_nodes,std::vector<int>& n_total_links);
   void write_SQL_insert_graph_commands(
      std::string SQL_subdir,const std::vector<int>& n_total_nodes,
      const std::vector<int>& n_total_links);
   void write_SQL_update_graph_commands(
      std::string SQL_subdir,const std::vector<int>& n_total_nodes,
      const std::vector<int>& n_total_links);
   void concatenate_SQL_insert_and_update_files(
      std::string graphs_subdir,unsigned int n_levels,
      int graph_component_ID,unsigned int n_connected_components);
   void write_SQL_insert_connected_component_annotation_commands(
      std::string SQL_subdir,
      COMPONENTS_LEVELS_MAP& components_levels_map);

  private: 

   std::string label;
   int ID,global_graph_edge_counter,levelzero_graph_ID;
   GRAPHS_MAP graphs_map;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const graph_hierarchy& g);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void graph_hierarchy::set_ID(int id)
{
   ID=id;
}

inline int graph_hierarchy::get_ID() const
{
   return ID;
}

inline void graph_hierarchy::set_label(std::string description)
{
   label=description;
}

inline std::string graph_hierarchy::get_label() const
{
   return label;
}

// Note added on 1/31/2001: In general graphs_map.size() could be
// larger than n_levels ...

inline unsigned int graph_hierarchy::get_n_levels() const
{
   return graphs_map.size();
}

inline unsigned int graph_hierarchy::get_n_graphs() const
{
   return graphs_map.size();
}


#endif  // graph_hierarchy.h
