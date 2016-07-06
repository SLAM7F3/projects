// ==========================================================================
// Header file for graphdbfunc namespace
// ==========================================================================
// Last modified on 4/5/14; 11/15/15; 12/1/15; 1/18/16
// ==========================================================================

#ifndef GRAPHDBFUNCS_H
#define GRAPHDBFUNCS_H

#include <map>
#include <string>
#include <vector>
#include "color/colorfuncs.h"

class fourvector;
class gis_database;
class graph;
class graph_hierarchy;
class twovector;

namespace graphdbfunc
{

   typedef std::map<int,int> NODE_DATA_IDS_MAP;
   typedef std::map<int,std::string> NODE_IDS_LABELS_MAP;

// Database metadata insertion methods

   std::string generate_insert_node_SQL_command(
      int graph_hierarchy_ID,int graph_ID,int level,int connected_component,
      int node_ID,int parent_node_ID,int data_ID,
      const colorfunc::RGB& node_RGB,
      double relative_size,double gx,double gy);
   std::string generate_insert_link_SQL_command(
      int graph_hierarchy_ID,int graph_ID,int level,int link_ID,
      int source_node_ID,int target_node_ID,
      bool directed_link_flag,double weight,const colorfunc::RGB& link_RGB,
      double relative_size);
   std::string generate_insert_graph_SQL_command(
      int graph_hierarchy_ID,int graph_ID,int level,int parent_graph_ID,
      int n_nodes,int n_links);
   std::string generate_update_graph_SQL_command(
      int graph_hierarchy_ID,int graph_ID,int n_nodes,int n_links);

   std::string update_graph_hierarchy_SQL_command(
      int graph_hierarchy_ID,int n_grpahs,int n_levels,
      int n_connected_components);
   bool update_hierarchy_table(
      gis_database* gis_database_ptr,int hierarchy_ID,
      std::string hierarchy_name,int n_graphs,int n_levels,
      int n_connected_components);
   std::string generate_insert_graph_hierarchy_SQL_command(
      int graph_hierarchy_ID,std::string hierarchy_name,int n_graphs,
      int n_levels,int n_connected_components);

   bool update_nodes_table(
      gis_database* gis_database_ptr,int hierarchy_ID,int graph_ID,
      int node_ID,double gx2,double gy2);
   bool update_nodes_table(
      gis_database* gis_database_ptr,int hierarchy_ID,int graph_ID,
      int node_ID,const colorfunc::RGB& node_RGB);
   std::string generate_update_node_SQL_command(
      int hierarchy_ID,int graph_ID,int node_ID,double gx2,double gy2);
   std::string generate_update_node_SQL_command(
      int hierarchy_ID,int graph_ID,int node_ID,
      const colorfunc::RGB& node_RGB);

// Graph hierarchy metadata retrieval methods

   std::string generate_retrieve_hierarchy_IDs_SQL_command();
   bool retrieve_and_display_hierarchy_IDs_from_database(
      gis_database* gis_database_ptr,
      std::vector<int>& hierarchy_IDs,
      std::vector<std::string>& hierarchy_descriptions);
   bool retrieve_hierarchy_IDs_from_database(
      gis_database* gis_database_ptr,
      std::vector<int>& hierarchy_IDs,
      std::vector<std::string>& hierarchy_descriptions);

   bool retrieve_node_ID_for_particular_hierarchy_datum(
      gis_database* gis_database_ptr,
      int hierarchy_ID,int graph_ID,int datum_ID,int& node_ID);

   int retrieve_n_levels_for_particular_graph_hierarchy(
      gis_database* gis_database_ptr,int graph_hierarchy_ID);

   NODE_DATA_IDS_MAP* retrieve_node_data_IDs_for_particular_graph_hierarchy(
      gis_database* gis_database_ptr,int graph_hierarchy_ID);

   int retrieve_image_ID_for_particular_graph_hierarchy_and_node(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,int node_ID);

   int retrieve_datum_ID_for_particular_graph_hierarchy_and_node(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,int node_ID);

   std::vector<int> retrieve_node_IDs_for_particular_graph_hierarchy_and_datum(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,int datum_ID);

   std::string generate_retrieve_hierarchies_labels_SQL_command();
   bool retrieve_hierarchies_levels_from_database(
      gis_database* gis_database_ptr,
      std::vector<int>& hierarchy_IDs,
      std::vector<std::string>& hierarchy_labels,
      std::vector<int>& graph_IDs,std::vector<int>& graph_levels);

   int get_graph_ID_given_level(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,int level);
   int get_graph_level_given_ID(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,int graph_ID);
   int get_graph_level_given_node_ID(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,int node_ID);
   int get_zeroth_node_given_level(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,int level);
   int get_graph_ID_given_node_and_hierarchy_IDs(
      gis_database* gis_database_ptr,int node_ID,int hierarchy_ID);

// Graph database metadata retrieval methods

   int retrieve_level_for_particular_graph(
      gis_database* gis_database_ptr,int graph_ID);
   std::string generate_retrieve_graph_hierarchy_SQL_command(int hierarchy_ID);
   void retrieve_graph_hierarchy_metadata_from_database(
      gis_database* gis_database_ptr,int hierarchy_ID,
      std::string& description,unsigned int& n_graphs,unsigned int& n_levels,
      unsigned int& n_connected_components);

   std::string generate_retrieve_graphs_SQL_command(int hierarchy_ID);

   void retrieve_graphs_metadata_from_database(
      gis_database* gis_database_ptr,int hierarchy_ID,
      unsigned int n_graphs,
      std::vector<int>& graph_ID,std::vector<int>& graph_level,
      std::vector<int>& parent_graph_ID, std::vector<int>& n_nodes,
      std::vector<int>& n_links);

   std::string generate_retrieve_particular_graph_SQL_command(
      int hierarchy_ID,int graph_ID);
   bool retrieve_particular_graph_metadata_from_database(
      gis_database* gis_database_ptr,int hierarchy_ID,int graph_ID,
      int& level,int& parent_graph_ID,int& n_nodes,int& n_links);

   std::string generate_retrieve_nodes_SQL_command(
      int graph_hierarchy_ID,int graph_ID);
   void retrieve_nodes_metadata_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,int graph_ID,
      std::vector<int>& node_ID,
      std::vector<int>& connected_component_node_ID,
      std::vector<int>& parent_node_ID,
      std::vector<int>& data_ID,std::vector<std::string>& color,
      std::vector<double>& relative_size,
      std::vector<twovector>& gxgy,std::vector<twovector>& gxgy2,
      std::vector<std::string>& label);
   std::string generate_retrieve_links_SQL_command(
      int graph_hierarchy_ID,int graph_ID);
   void retrieve_links_metadata_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,int graph_ID,
      std::vector<int>& link_ID,std::vector<int>& source_node_ID,
      std::vector<int>& target_node_ID,std::vector<bool>& directed,
      std::vector<double>& weight,std::vector<std::string>& color,
      std::vector<double>& relative_size);

   graph* reconstruct_graph_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,
      int graph_ID,int level);
   graph_hierarchy* reconstruct_graph_hierarchy_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID);

// Graphs from JSON file methods

   graph_hierarchy* reconstruct_graph_hierarchy_from_JSON_file(
      std::string json_filename,int graph_hierarchy_ID,
      unsigned int& n_graphs,unsigned int& n_levels,
      unsigned int& n_connected_components,std::string& description,
      std::vector<int>& graph_ID,std::vector<int>& graph_level,
      std::vector<int>& parent_graph_ID,
      std::vector<int>& n_nodes,std::vector<int>& n_links);
   void count_nodes_links_in_JSON_file(
      std::string json_filename,std::vector<int>& n_nodes,
      std::vector<int>& n_links);

// Sibling search methods

   std::string generate_retrieve_sibling_node_IDs_SQL_command(
      int graph_hierarchy_ID,int parent_node_ID);
   std::vector<int> retrieve_sibling_node_IDs_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,int node_ID);
   std::vector<int> retrieve_sibling_data_IDs_from_database(
      gis_database* gis_database_ptr,int hierarchy_ID,int graph_level,
      int datum_ID);
   int get_n_siblings_from_database(
      gis_database* gis_database_ptr,int hierarchy_ID,int graph_level,
      int datum_ID);

   std::vector<int> retrieve_cousin_node_IDs_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,int node_ID);
   std::vector<int> retrieve_cousin_node_IDs_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,
      int graph_ID,int datum_ID);
   std::vector<int> retrieve_cousin_data_IDs_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,
      int graph_ID,int datum_ID);

// Children search methods

   std::vector<int> retrieve_children_node_IDs_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,int node_ID);
   std::vector<int> retrieve_children_data_IDs_from_database(
      gis_database* gis_database_ptr,int hierarchy_ID,int graph_level,
      int datum_ID);
   int get_n_children_from_database(
      gis_database* gis_database_ptr,int hierarchy_ID,int graph_level,
      int datum_ID);
   std::vector<int> retrieve_grandchildren_node_IDs_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,
      int grandparent_node_ID);

// Parent search methods

   std::string generate_retrieve_parent_node_ID_SQL_command(
      int graph_hierarchy_ID,int node_ID);
   int retrieve_parent_node_ID_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,int node_ID);
   int retrieve_parent_datum_ID_from_database(
      gis_database* gis_database_ptr,int hierarchy_ID,int graph_level,
      int datum_ID);
   int retrieve_grandparent_node_ID_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,int node_ID);

// Node label methods
   
   std::string generate_update_node_label_SQL_command(
      int graph_hierarchy_ID,int node_ID,std::string label);
   bool update_node_label_in_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,int node_ID,
      std::string label);
   bool retrieve_node_label_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,int node_ID,
      std::string& label);
   NODE_IDS_LABELS_MAP* retrieve_node_labels_from_database(
      gis_database* gis_database_ptr,int graph_ID);

// Connected graph component methods

   bool insert_connected_component(
      gis_database* gis_database_ptr,int hierarchy_ID,int graph_ID,int level,
      int connected_component_ID,int n_nodes,int n_links);
   std::string generate_insert_connected_component_SQL_command(
      int hierarchy_ID,int graph_ID,int level,int connected_component_ID,
      int n_nodes,int n_links);
   std::string generate_update_connected_component_SQL_command(
      int hierarchy_ID,int graph_ID,int connected_component_ID,
      int n_nodes,int n_links);
   bool insert_connected_component(
      gis_database* gis_database_ptr,int hierarchy_ID,int graph_ID,int level,
      int connected_component_ID,int cc_row,int cc_column,
      std::string cc_label);

   int get_n_connected_components(
      gis_database* gis_database_ptr,int hierarchy_ID);
   bool retrieve_connected_component_info_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,
      std::vector<int>& graph_IDs,std::vector<int>& levels,
      std::vector<int>& connected_component_IDs,std::vector<int>& n_nodes);
   int get_n_nodes_for_connected_component(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,
      int graph_ID,int connected_component_ID);
   bool retrieve_connected_component_nodes_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,
      int graph_ID,int connected_component_ID,std::vector<int>& node_IDs);
   int get_link_weight(
      gis_database* gis_database_ptr,int hierarchy_ID,int graph_ID,
      int source_node_ID,int target_node_ID);
   int get_connected_component(
      gis_database* gis_database_ptr,int hierarchy_ID,int node_ID);
   bool retrieve_node_connected_component_IDs_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,
      int graph_ID,std::vector<int>& node_IDs,std::vector<int>& cc_IDs);
   bool update_connected_component_info(
      gis_database* gis_database_ptr,
      int graph_hierarchy_ID,int graph_ID,int connected_component_ID,
      int cc_row,int cc_column,std::string label);
   bool retrieve_connected_component_posn_labels_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,
      int graph_ID,int connected_component_ID,
      double& cc_row,double& cc_column,std::string& cc_label,
      std::vector<std::string>& topic_labels);
   bool update_connected_component_topic(
      gis_database* gis_database_ptr,int hierarchy_ID,int graph_ID,
      int connected_component_ID,int topic_ID,std::string topic);

// Graph annotation methods

   bool retrieve_graph_annotations_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,int graph_ID,
      std::vector<int>& layouts,
      std::vector<double>& gxs,std::vector<double>& gys,
      std::vector<std::string>& labels,std::vector<std::string>& colors,
      std::vector<double>& sizes);
   std::string generate_insert_graph_annotation_SQL_command(
      int hierarchy_ID,int graph_ID,int level,int layout,double gx,double gy,
      std::string label,std::string color,double annotation_size);

// Graph geometry methods

   bool retrieve_graph_geometries_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,
      std::vector<std::string>& geom_labels,
      std::vector<std::string>& geom_colors,
      std::vector<std::string>& the_geoms);
   bool retrieve_graph_polygons_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,
      std::vector<std::string>& polygon_labels,
      std::vector<fourvector>& polygon_RGBA,
      std::vector< std::vector<twovector> >& polygon_vertices);
      
// ==========================================================================
// Inlined methods:
// ==========================================================================

} // graphdbfunc namespace

#endif // graphdbfuncs.h

