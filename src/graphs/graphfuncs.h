// =========================================================================
// Header file for stand-alone graph theory functions.
// =========================================================================
// Last modified on 6/6/10; 12/1/10; 1/31/11; 4/5/14
// =========================================================================

#ifndef GRAPHFUNCS_H
#define GRAPHFUNCS_H

#include <string>
#include "color/colorfuncs.h"
#include "math/twovector.h"

class gis_database;
class graph;
class graph_hierarchy;
class node;

namespace graphfunc
{
   void initialize_findunion_structures(unsigned int n,int id[],int size[]);
   void quickfind(int p,int q,unsigned int n,int id[]);
   void quickunion(int p,int q,int id[]);
   void weightedunion(int p,int q,int n,int id[],int size[]);
   void halveunion(int p,int q,int n,int id[],int size[]);

   int tree_root(int p,int n,int id[]);
   bool equivalent_leaves(int p,int q,int n,int id[]);

// Graph generation methods

   int max_node_ID_from_edgelist(std::string edgelist_filename);
   graph* generate_graph_from_edgelist(
      std::string edgelist_filename,int graph_ID,int level,
      int zeroth_datum_ID=0);
   graph* generate_graph_from_edgelist(
      std::string edgelist_filename,int graph_ID,int level,
      int zeroth_datum_ID,int* graph_edge_counter_ptr);
   node* generate_node_from_database_info(
      int node_ID,unsigned int node_level,int data_ID,int parent_ID,
      double node_relative_size,colorfunc::RGB node_RGB,
      const twovector& gxgy);
   graph_hierarchy* generate_graph_hierarchy_from_database(
      gis_database* gis_database_ptr);

// MCL output manipulation methods

   std::vector<twovector> extract_hierarchy_cluster_sizes(
      std::string mclcm_cone_filename);
   void parse_mcl_cone_file(
      std::string mclcm_cone_filename,graph_hierarchy* graphs_pyramid_ptr,
      graph* edgelist_graph_ptr);
   void extract_nodes_from_mcl_cone_file(
      int level,std::string curr_substring,
      std::vector<int>& children_node_IDs,node* curr_node_ptr,
      graph* lowest_graph_ptr,graph* edgelist_graph_ptr);

// Clustering methods

   twovector compute_cluster_COM(
      graph* cluster_graph_ptr,graph* child_graph_ptr,
      int parent_ID,bool take_medians_flag=true);
   double median_cluster_radius(
      graph* cluster_graph_ptr,graph* child_graph_ptr,int parent_ID);
   void adjust_child_node_posns_from_cluster_COMs(
      graph* cluster_graph_ptr,graph* child_graph_ptr,
      graph* parent_graph_ptr);
   void strengthen_clusters_in_layout(
      graph* cluster_graph_ptr,graph* child_graph_ptr);
   void angularly_redistribute_cluster_nodes(
      graph* cluster_graph_ptr,graph* child_graph_ptr);

// Cluster information propagation methods

   void propagate_cluster_IDs_to_input_graph_children(
      graph* cluster_graph_ptr,graph* child_graph_ptr);
   void generate_parent_graph_nodes_from_clusters(
      graph* cluster_graph_ptr,graph* parent_graph_ptr);
   void set_parent_node_positions_based_on_children(
      graph* parent_graph_ptr,graph* child_graph_ptr);
   void hierarchical_grandparents_clustering(
      graph* parent_graph_ptr,graph* grandparents_graph_ptr);

// Graph path methods:

   void print_shortest_path(node* destination_node_ptr);
   
}

#endif // graphs/graphfuncs.h




