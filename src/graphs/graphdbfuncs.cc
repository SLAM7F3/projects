// ==========================================================================
// Graphdbfuncs namespace method definitions
// ==========================================================================
// Last modified on 5/27/14; 11/15/15; 12/1/15; 1/18/16; 8/19/16
// ==========================================================================

#include <iostream>
#include <map>
#include "graphs/cppJSON.h"
#include "general/filefuncs.h"
#include "math/fourvector.h"
#include "postgres/gis_database.h"
#include "graphs/graph.h"
#include "graphs/graphdbfuncs.h"
#include "graphs/graph_edge.h"
#include "graphs/graph_hierarchy.h"
#include "graphs/node.h"
#include "general/stringfuncs.h"
#include "math/twovector.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;

namespace graphdbfunc
{
   graph_hierarchy* graph_hierarchy_ptr=NULL;

// ==========================================================================
// Database metadata insertion methods
// ==========================================================================

// Method generate_insert_node_SQL_command() takes in metadata
// associated with a single node.  It generates and returns a string
// containing a SQL insert command needed to populate a row within the
// nodes table of the IMAGERY database.

   string generate_insert_node_SQL_command(
      int graph_hierarchy_ID,int graph_ID,int level,int connected_component,
      int node_ID,int parent_node_ID,int data_ID,
      const colorfunc::RGB& node_RGB,double relative_size,double gx,double gy)
   {
//      cout << "inside graphdbfunc::generate_insert_node_SQL_command()" << endl;

      string SQL_command="insert into nodes ";
      SQL_command += "(graph_hierarchy_ID,graph_ID,connected_component_ID,";
      SQL_command += "node_ID,parent_node_ID,data_ID,";
      SQL_command += "color,relative_size,gx,gy) ";
      SQL_command += "values( ";
      SQL_command += stringfunc::number_to_string(graph_hierarchy_ID)+",";
      SQL_command += stringfunc::number_to_string(graph_ID)+",";
      SQL_command += stringfunc::number_to_string(connected_component)+",";
      SQL_command += stringfunc::number_to_string(node_ID)+",";
      SQL_command += stringfunc::number_to_string(parent_node_ID)+",";
      SQL_command += stringfunc::number_to_string(data_ID)+",";
      SQL_command += "'"+colorfunc::RGB_to_RRGGBB_hex(node_RGB)+"',";
      SQL_command += stringfunc::number_to_string(relative_size)+",";
      SQL_command += stringfunc::number_to_string(gx)+",";
      SQL_command += stringfunc::number_to_string(gy);
      SQL_command += ");";
//      cout << SQL_command << endl;
      return SQL_command;
   }

// ---------------------------------------------------------------------
// Method generate_insert_link_SQL_command() takes in metadata
// associated with a single link.  It generates and returns a string
// containing a SQL insert command needed to populate a row within the
// links table of the IMAGERY database.

   string generate_insert_link_SQL_command(
      int graph_hierarchy_ID,int graph_ID,int level,int link_ID,
      int source_node_ID,int target_node_ID,
      bool directed_link_flag,double weight,const colorfunc::RGB& link_RGB,
      double relative_size)
   {
//      cout << "inside graphdbfunc::generate_insert_link_SQL_command()" << endl;

      string SQL_command="insert into links ";
      SQL_command += "(graph_hierarchy_ID,graph_ID,link_ID,";
      SQL_command += "source_node_ID,target_node_ID,";
      SQL_command += "directed,weight,color,relative_size) ";
      SQL_command += "values( ";
      SQL_command += stringfunc::number_to_string(graph_hierarchy_ID)+",";
      SQL_command += stringfunc::number_to_string(graph_ID)+",";
      SQL_command += stringfunc::number_to_string(link_ID)+",";
      SQL_command += stringfunc::number_to_string(source_node_ID)+",";
      SQL_command += stringfunc::number_to_string(target_node_ID)+",";
      if (directed_link_flag)
      {
         SQL_command += "TRUE,";
      }
      else
      {
         SQL_command += "FALSE,";
      }
      SQL_command += stringfunc::number_to_string(weight)+",";
      SQL_command += "'"+colorfunc::RGB_to_RRGGBB_hex(link_RGB)+"',";
      SQL_command += stringfunc::number_to_string(relative_size)+");";

//      cout << SQL_command << endl;
      return SQL_command;
   }

// ---------------------------------------------------------------------
// Method generate_insert_graph_SQL_command() takes in metadata
// associated with a single graph.  It generates and returns a string
// containing a SQL insert command needed to populate a row within the
// graphs table of the IMAGERY database.

   string generate_insert_graph_SQL_command(
      int graph_hierarchy_ID,int graph_ID,int level,int parent_graph_ID,
      int n_nodes,int n_links)
   {
//      cout << "inside graphdbfunc::generate_insert_graph_SQL_command()" 
//           << endl;

      string SQL_command="insert into graphs ";
      SQL_command += "(graph_hierarchy_ID,graph_ID,level,parent_graph_ID,";
      SQL_command += "n_nodes,n_links) ";
      SQL_command += "values( ";
      SQL_command += stringfunc::number_to_string(graph_hierarchy_ID)+",";
      SQL_command += stringfunc::number_to_string(graph_ID)+",";
      SQL_command += stringfunc::number_to_string(level)+",";
      SQL_command += stringfunc::number_to_string(parent_graph_ID)+",";
      SQL_command += stringfunc::number_to_string(n_nodes)+",";
      SQL_command += stringfunc::number_to_string(n_links)+");";

//      cout << SQL_command << endl;
      return SQL_command;
   }

// ---------------------------------------------------------------------
// Method generate_update_graph_SQL_command() takes in metadata
// associated with a single graph.  It generates and returns a string
// containing a SQL update command needed to modify the number of
// nodes and links within a particular row in the graphs table of the
// IMAGERY database.

   string generate_update_graph_SQL_command(
      int graph_hierarchy_ID,int graph_ID,int n_nodes,int n_links)
   {
//      cout << "inside graphdbfunc::generate_update_graph_SQL_command()" 
//           << endl;

      string SQL_cmd="UPDATE graphs ";
      SQL_cmd += "SET n_nodes="+stringfunc::number_to_string(n_nodes)+",";
      SQL_cmd += "n_links="+stringfunc::number_to_string(n_links);
      SQL_cmd += " WHERE graph_hierarchy_ID="+stringfunc::number_to_string(
         graph_hierarchy_ID);
      SQL_cmd += " AND graph_ID="+stringfunc::number_to_string(graph_ID);
      return SQL_cmd;
   }

// ---------------------------------------------------------------------
// Method update_hierarchy_table() 

   bool update_hierarchy_table(
      gis_database* gis_database_ptr,int hierarchy_ID,string hierarchy_name,
      int n_graphs,int n_levels,int n_connected_components)

   {
      string SQL_cmd=generate_insert_graph_hierarchy_SQL_command(
         hierarchy_ID,hierarchy_name,n_graphs,n_levels,n_connected_components);
    
      vector<string> insert_commands;
      insert_commands.push_back(SQL_cmd);
      gis_database_ptr->set_SQL_commands(insert_commands);
      return gis_database_ptr->execute_SQL_commands();
   }

// ---------------------------------------------------------------------
// Method generate_insert_graph_hierarchy_SQL_command() 

   string generate_insert_graph_hierarchy_SQL_command(
      int graph_hierarchy_ID,string hierarchy_name,int n_graphs,int n_levels,
      int n_connected_components)
   {
      string SQL_cmd="INSERT into graph_hierarchies ";
      SQL_cmd += "(graph_hierarchy_ID,name,n_graphs,n_levels,n_connected_components) ";
      SQL_cmd += "values( ";
      SQL_cmd += stringfunc::number_to_string(graph_hierarchy_ID)+",";
      SQL_cmd += "'"+hierarchy_name+"',";
      SQL_cmd += stringfunc::number_to_string(n_graphs)+",";
      SQL_cmd += stringfunc::number_to_string(n_levels)+",";
      SQL_cmd += stringfunc::number_to_string(n_connected_components);
      SQL_cmd += ");";
      return SQL_cmd;
   }

// ---------------------------------------------------------------------
// Method update_nodes_table() 

   bool update_nodes_table(
      gis_database* gis_database_ptr,int hierarchy_ID,int graph_ID,
      int node_ID,double gx2,double gy2)
   {
/*      
      string SQL_cmd="UPDATE nodes ";
      SQL_cmd += "SET gx2="+stringfunc::number_to_string(gx2)+",";
      SQL_cmd += "gy2="+stringfunc::number_to_string(gy2);
      SQL_cmd += " WHERE graph_hierarchy_ID="+stringfunc::number_to_string(
         hierarchy_ID);
      SQL_cmd += " AND graph_ID="+stringfunc::number_to_string(graph_ID);
      SQL_cmd += " AND node_ID="+stringfunc::number_to_string(node_ID);
*/

      string SQL_cmd = generate_update_node_SQL_command(
         hierarchy_ID, graph_ID, node_ID, gx2, gy2);
      vector<string> update_commands;
      update_commands.push_back(SQL_cmd);

      gis_database_ptr->set_SQL_commands(update_commands);
      return gis_database_ptr->execute_SQL_commands();
   }

   bool update_nodes_table(
      gis_database* gis_database_ptr,int hierarchy_ID,int graph_ID,
      int node_ID,const colorfunc::RGB& node_RGB)
   {
      string SQL_cmd = generate_update_node_SQL_command(
         hierarchy_ID, graph_ID, node_ID, node_RGB);
      vector<string> update_commands;
      update_commands.push_back(SQL_cmd);

      gis_database_ptr->set_SQL_commands(update_commands);
      return gis_database_ptr->execute_SQL_commands();
   }

// ---------------------------------------------------------------------
// Method generate_update_node_SQL_command()

   string generate_update_node_SQL_command(
      int hierarchy_ID,int graph_ID,int node_ID,double gx2,double gy2)
   {
      string SQL_cmd="UPDATE nodes ";
      SQL_cmd += "SET gx2="+stringfunc::number_to_string(gx2)+",";
      SQL_cmd += "gy2="+stringfunc::number_to_string(gy2);
      SQL_cmd += " WHERE graph_hierarchy_ID="+stringfunc::number_to_string(
         hierarchy_ID);
      SQL_cmd += " AND graph_ID="+stringfunc::number_to_string(graph_ID);
      SQL_cmd += " AND node_ID="+stringfunc::number_to_string(node_ID);
      return SQL_cmd;
   }
   
// ---------------------------------------------------------------------
// This overloaded version of generate_update_node_SQL_command()
// resets the hex_color for the specified input node:

   string generate_update_node_SQL_command(
      int hierarchy_ID,int graph_ID,int node_ID,
      const colorfunc::RGB& node_RGB)
   {
      string SQL_cmd="UPDATE nodes ";
      SQL_cmd += "SET color='"+colorfunc::RGB_to_RRGGBB_hex(node_RGB)+"'";
      SQL_cmd += " WHERE graph_hierarchy_ID="+stringfunc::number_to_string(
         hierarchy_ID);
      SQL_cmd += " AND graph_ID="+stringfunc::number_to_string(graph_ID);
      SQL_cmd += " AND node_ID="+stringfunc::number_to_string(node_ID);
      return SQL_cmd;
   }
// ==========================================================================
// Graph hierarchy metadata retrieval methods
// ==========================================================================

// Member function generate_retrieve_hierarchy_IDs_SQL_command()

   string generate_retrieve_hierarchy_IDs_SQL_command()
   {
//      cout << "inside graphdbfunc::generate_retrieve_hierarchy_IDs_SQL_command()" 
//           << endl;

      string SQL_command="SELECT graph_hierarchy_ID,name from graph_hierarchies";
      return SQL_command;
   }

// ---------------------------------------------------------------------   
   bool retrieve_and_display_hierarchy_IDs_from_database(
      gis_database* gis_database_ptr,
      vector<int>& hierarchy_IDs,vector<string>& hierarchy_descriptions)
   {
      bool flag=retrieve_hierarchy_IDs_from_database(
         gis_database_ptr,hierarchy_IDs,hierarchy_descriptions);
      
      cout << "Existing graph hierarchies within IMAGERY database:" << endl;
      cout << endl;
      for (unsigned int h=0; h<hierarchy_IDs.size(); h++)
      {
         cout << "Hierarchy ID = " << hierarchy_IDs[h]
              << "  hierarchy description = " << hierarchy_descriptions[h] 
              << endl;
      }
      return flag;
   }

// ---------------------------------------------------------------------   
   bool retrieve_hierarchy_IDs_from_database(
      gis_database* gis_database_ptr,
      vector<int>& hierarchy_IDs,vector<string>& hierarchy_descriptions)
   {
//      cout << "inside graphdbfunc::retrieve_hierarchy_IDs_from_database()" 
//           << endl;

      string curr_select_cmd=generate_retrieve_hierarchy_IDs_SQL_command();
//      cout << "curr_select_cmd = " << curr_select_cmd << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);
      if (field_array_ptr==NULL) return false;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      for (unsigned int m=0; m<field_array_ptr->get_mdim(); m++)
      {
         int curr_hierarchy_ID=
            stringfunc::string_to_number(field_array_ptr->get(m,0));
         hierarchy_IDs.push_back(curr_hierarchy_ID);
         hierarchy_descriptions.push_back(field_array_ptr->get(m,1));
      } // loop over index m labeling rows

      return true;
   }

// ---------------------------------------------------------------------   
// Method retrieve_node_ID_for_particular_hierarchy_datum()
// takes in the hierarchy, graph and datum IDs for some entry
// in the nodes table of the IMAGERY database.  (Recall that a photo
// with a unique datum ID may appear in different hierarchies and in
// multiple levels of a graph hierarchy.) It finds and returns the ID
// for the unique node to which the input datum belongs.

   bool retrieve_node_ID_for_particular_hierarchy_datum(
      gis_database* gis_database_ptr,
      int hierarchy_ID,int graph_ID,int datum_ID,int& node_ID)
   {
//      cout << "inside graphdbfunc::retrieve_node_ID_for_particular_hierarchy_datum()" << endl;
//      cout << "hierarchy_ID = " << hierarchy_ID << endl;
//      cout << "graph_ID = " << graph_ID << endl;
//      cout << "datum_ID = " << datum_ID << endl;

      node_ID=-1;
      
      string curr_select_cmd="SELECT node_id from nodes ";
      curr_select_cmd += "WHERE graph_hierarchy_ID="+
         stringfunc::number_to_string(hierarchy_ID);
      curr_select_cmd += " AND graph_ID="+
         stringfunc::number_to_string(graph_ID);
      curr_select_cmd += " AND data_id="+
         stringfunc::number_to_string(datum_ID);
//      cout << "curr_select_cmd = " << curr_select_cmd << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);
      if (field_array_ptr==NULL) 
      {
//         cout << "ERROR! FIELD_ARRAY_PTR=NULL!!!" << endl;
         return false;
      }
      
      node_ID=stringfunc::string_to_number(field_array_ptr->get(0,0));

//      cout << "At end of graphdbfunc::retrieve_node_ID_for_particular_hierarchy_datum()"
//           << endl;
//      cout << "input: graph_hierarchy_ID = " << hierarchy_ID << endl;
//      cout << "input: graph_ID = " << graph_ID << endl;
//      cout << "input: datum_ID = " << datum_ID << endl;
//      cout << "output: node_ID = " << node_ID << endl;

      return true;
//      outputfunc::enter_continue_char();
   }

// ---------------------------------------------------------------------   
// Method retrieve_n_levels_for_particular_graph_hierarchy()
// takes in the hierarchy ID for some entry in the graph_hierarchies
// table of the IMAGERY databse database.  It returns the number of
// levels within the specified graph hierarchy.

   int retrieve_n_levels_for_particular_graph_hierarchy(
      gis_database* gis_database_ptr,int graph_hierarchy_ID)
   {
//      cout << "inside graphdbfunc::retrieve_n_levels_for_particular_graph_hierarchy()" 
//           << endl;

      string curr_select_cmd="select n_levels from graph_hierarchies ";
      curr_select_cmd += "where graph_hierarchy_id="+
         stringfunc::number_to_string(graph_hierarchy_ID);
      
//      cout << "curr_select_cmd = " << curr_select_cmd << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);
      if (field_array_ptr==NULL) return -1;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;
      int n_levels=
         stringfunc::string_to_number(field_array_ptr->get(0,0));         
      return n_levels;
   }

// ---------------------------------------------------------------------   
// Method retrieve_node_data_IDs_for_particular_graph_hierarchy()
// takes in the graph_hierarchy ID for some entries in the
// nodes table of the IMAGERY database.  It returns an STL map
// containing node and data ID pairs.

   NODE_DATA_IDS_MAP* retrieve_node_data_IDs_for_particular_graph_hierarchy(
      gis_database* gis_database_ptr,int graph_hierarchy_ID)
   {
//      cout << "inside graphdbfunc::retrieve_node_data_IDs_for_particular_graph_hierarchy()" 
//           << endl;

      string curr_select_cmd="select node_id,data_id from nodes ";
      curr_select_cmd += "where graph_hierarchy_id="+
         stringfunc::number_to_string(graph_hierarchy_ID);
      
//      cout << "curr_select_cmd = " << curr_select_cmd << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);
      if (field_array_ptr==NULL) return NULL;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      NODE_DATA_IDS_MAP* node_data_ids_map_ptr=new NODE_DATA_IDS_MAP;
      for (unsigned int i=0; i<field_array_ptr->get_mdim(); i++)
      {
         int curr_node_ID=
            stringfunc::string_to_number(field_array_ptr->get(i,0));         
         int curr_data_ID=
            stringfunc::string_to_number(field_array_ptr->get(i,1));         
         (*node_data_ids_map_ptr)[curr_node_ID]=curr_data_ID;
      }

//      cout << "node_data_ids_map_ptr->size() = "
//           << node_data_ids_map_ptr->size() << endl;
      
      return node_data_ids_map_ptr;
   }

// ---------------------------------------------------------------------   
// Method retrieve_datum_ID_for_particular_graph_hierarchy_and_node()
// takes in the graph_hierarchy and node IDs for some entry in the
// nodes table of the IMAGERY database.  It finds and returns the datum ID
// to which the input datum belongs.

   int retrieve_datum_ID_for_particular_graph_hierarchy_and_node(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,int node_ID)
   {
//      cout << "inside graphdbfunc::retrieve_datum_ID_for_particular_graph_hierarchy_and_node()" 
//           << endl;

      int data_ID=-1;
      
      string curr_select_cmd="select data_id from nodes ";
      curr_select_cmd += "where graph_hierarchy_id="+
         stringfunc::number_to_string(graph_hierarchy_ID);
      curr_select_cmd += " and node_id="+
         stringfunc::number_to_string(node_ID);
      
//      cout << "curr_select_cmd = " << curr_select_cmd << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);
      if (field_array_ptr==NULL) return data_ID;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      data_ID=stringfunc::string_to_number(field_array_ptr->get(0,0));
//      cout << "Retrieved data_ID = " << data_ID << endl;
      return data_ID;
   }

// ---------------------------------------------------------------------   
// Method retrieve_node_IDs_for_particular_graph_hierarchy_and_datum()
// takes in the graph_hierarchy and datum IDs for some entries in the
// nodes table of the IMAGERY database.  It finds and returns the IDs for
// all nodes to which the input datum belongs.

   vector<int> retrieve_node_IDs_for_particular_graph_hierarchy_and_datum(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,int datum_ID)
   {
//      cout << "inside graphdbfunc::retrieve_node_IDs_for_particular_graph_heirarchy_and_datum()" 
//           << endl;

      vector<int> node_IDs;
      
      string curr_select_cmd="select node_id from nodes ";
      curr_select_cmd += "where graph_hierarchy_id="+
         stringfunc::number_to_string(graph_hierarchy_ID);
      curr_select_cmd += " and data_id="+
         stringfunc::number_to_string(datum_ID);
      curr_select_cmd += " order by node_id";
      
//      cout << "curr_select_cmd = " << curr_select_cmd << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);
      if (field_array_ptr==NULL) return node_IDs;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      for (unsigned int m=0; m<field_array_ptr->get_mdim(); m++)
      {
         int curr_node_ID=stringfunc::string_to_number(
            field_array_ptr->get(m,0));
         node_IDs.push_back(curr_node_ID);
      }

      return node_IDs;
   }

// ---------------------------------------------------------------------
// Method generate_retrieve_hierarchies_labels_SQL_command()

   string generate_retrieve_hierarchies_labels_SQL_command()
   {
//      cout << "inside graphdbfunc::generate_retrieve_hierarchies_labels_SQL_command()" << endl;

      string SQL_command="SELECT h.graph_hierarchy_id AS hierarchy_id,";
      SQL_command += "h.name AS hierarchy_label,g.id AS graph_id,";
      SQL_command += "g.level AS graph_level ";
      SQL_command += "FROM graphs AS g, graph_hierarchies AS h ";
      SQL_command += "WHERE h.graph_hierarchy_id=g.graph_hierarchy_id ";
      SQL_command += "ORDER by graph_id";
//      cout << SQL_command << endl;
      return SQL_command;
   }

// ---------------------------------------------------------------------   
   bool retrieve_hierarchies_levels_from_database(
      gis_database* gis_database_ptr,
      vector<int>& hierarchy_IDs,vector<string>& hierarchy_labels,
      vector<int>& graph_IDs,vector<int>& graph_levels)
   {
//      cout << "inside graphdbfunc::retrieve_hierarchies_levels_from_database()" 
//           << endl;

      string curr_select_cmd=
         generate_retrieve_hierarchies_labels_SQL_command();
//      cout << "curr_select_cmd = " << curr_select_cmd << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;
      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);
      if (field_array_ptr==NULL) return false;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      for (unsigned int m=0; m<field_array_ptr->get_mdim(); m++)
      {
         int curr_hierarchy_ID=
            stringfunc::string_to_number(field_array_ptr->get(m,0));
         hierarchy_IDs.push_back(curr_hierarchy_ID);
         hierarchy_labels.push_back(field_array_ptr->get(m,1));
         int curr_graph_ID=
            stringfunc::string_to_number(field_array_ptr->get(m,2));
         graph_IDs.push_back(curr_graph_ID);
         int curr_graph_level=
            stringfunc::string_to_number(field_array_ptr->get(m,3));

//         cout << "m = " << m 
//              << " hierarchy_ID = " << curr_hierarchy_ID
//              << " graph_ID = " << curr_graph_ID
//              << " graph level = " << curr_graph_level << endl;
         
         graph_levels.push_back(curr_graph_level);
      } // loop over index m labeling rows

      return true;
   }

// ---------------------------------------------------------------------   
// Method get_graph_ID_given_level() takes in the graph_hierarchy and
// level for some entry in the graphs table of the IMAGERY database.  It
// finds and returns the corresponding graph ID.

   int get_graph_ID_given_level(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,int level)
   {
//      cout << "inside graphdbfunc::get_graph_ID_given_level()" 
//           << endl;

      int graph_ID=-1;
      
      string curr_select_cmd="select graph_id from graphs ";
      curr_select_cmd += "where graph_hierarchy_id="+
         stringfunc::number_to_string(graph_hierarchy_ID);
      curr_select_cmd += " and level="+stringfunc::number_to_string(level);
      
//      cout << "curr_select_cmd = " << curr_select_cmd << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);
      if (field_array_ptr==NULL) return graph_ID;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      graph_ID=stringfunc::string_to_number(field_array_ptr->get(0,0));
      return graph_ID;
   }

// ---------------------------------------------------------------------   
// Method get_graph_level_given_ID() takes in the graph_hierarchy and
// ID for some entry in the graphs table of the IMAGERY database.  It finds
// and returns the corresponding graph level.

   int get_graph_level_given_ID(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,int graph_ID)
   {
//      cout << "inside graphdbfunc::get_graph_level_given_ID()" 
//           << endl;

      int graph_level=-1;
      
      string curr_select_cmd="select level from graphs ";
      curr_select_cmd += "where graph_hierarchy_id="+
         stringfunc::number_to_string(graph_hierarchy_ID);
      curr_select_cmd += " and graph_id="
         +stringfunc::number_to_string(graph_ID);
      
//      cout << "curr_select_cmd = " << curr_select_cmd << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);
      if (field_array_ptr==NULL) return graph_level;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      graph_level=stringfunc::string_to_number(field_array_ptr->get(0,0));
      return graph_level;
   }

// ---------------------------------------------------------------------   
// Method get_graph_level_given_node_ID() takes in the graph_hierarchy and
// ID for some node in the graphs table of the IMAGERY database.  It finds
// and returns the corresponding graph level.

   int get_graph_level_given_node_ID(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,int node_ID)
   {
//      cout << "inside graphdbfunc::get_graph_level_given_node_ID()" 
//           << endl;

      int graph_level=-1;
      
      string curr_select_cmd="select graph_ID from nodes ";
      curr_select_cmd += "where graph_hierarchy_id="+
         stringfunc::number_to_string(graph_hierarchy_ID);
      curr_select_cmd += " and node_id="+stringfunc::number_to_string(
         node_ID);
      
//      cout << "curr_select_cmd = " << curr_select_cmd << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);
      if (field_array_ptr==NULL) return graph_level;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      int graph_ID=stringfunc::string_to_number(field_array_ptr->get(0,0));
      graph_level=get_graph_level_given_ID(
         gis_database_ptr,graph_hierarchy_ID,graph_ID);

      return graph_level;
   }

// ---------------------------------------------------------------------   
// Method get_zeroth_node_given_level() takes in the graph_hierarchy and
// level for some entry in the graphs table of the IMAGERY database.  It
// finds and returns the ID for the "zeroth" node corresponding to the
// input graph level.

   int get_zeroth_node_given_level(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,int level)
   {
//      cout << "inside graphdbfunc::get_zeroth_node_given_level()" << endl;
//      cout << "level = " << level << " graph_hierarchy_ID = "
//           << graph_hierarchy_ID << endl;

      int zeroth_node_ID=-1;

/*
// As of 2/13/12, graph_ID = level for all entries within images table
// of IMAGERY database

      int graph_ID=get_graph_ID_given_level(
         gis_database_ptr,graph_hierarchy_ID,level);
      if (graph_ID < 0) return zeroth_node_ID;
      cout << "Graph_ID = " << graph_ID << endl;

      if (graph_ID != level)
      {
         cout << "Error in graphdbfunc::get_zeroth_node_given_level()"
              << endl;
         cout << "level = " << level << " graph_ID = " << graph_ID
              << " should be EQUAL!" << endl;
         exit(-1);
      }
*/

      string curr_select_cmd="select node_id from nodes ";
      curr_select_cmd += "where graph_hierarchy_id="+
         stringfunc::number_to_string(graph_hierarchy_ID);
      curr_select_cmd += " AND graph_id="+
         stringfunc::number_to_string(level);
      curr_select_cmd += " order by node_id";
      
//      cout << "curr_select_cmd = " << curr_select_cmd << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);
      if (field_array_ptr==NULL) return zeroth_node_ID;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      zeroth_node_ID=stringfunc::string_to_number(field_array_ptr->get(0,0));
//      cout << "zeroth_node_ID = " << zeroth_node_ID << endl;
      return zeroth_node_ID;
   }

// ---------------------------------------------------------------------
// Method get_graph_ID_given_node_and_hierarchy_IDs() takes in a node
// ID along with its corresponding graph hierarchy ID.  It returns the
// unique graph ID retrieved from the nodes table in the IMAGERY database.

   int get_graph_ID_given_node_and_hierarchy_IDs(
      gis_database* gis_database_ptr,int node_ID,int hierarchy_ID)
   {
//      cout << "inside graphdbfunc::get_graph_ID_given_node_and_hierarchy_IDs()"
//           << endl;

      string curr_select_cmd="select graph_ID from nodes where node_id="+
         stringfunc::number_to_string(node_ID)+" and graph_hierarchy_id="+
         stringfunc::number_to_string(hierarchy_ID);

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);
      if (field_array_ptr==NULL) return -1;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      int curr_graph_ID=
         stringfunc::string_to_number(field_array_ptr->get(0,0));
      return curr_graph_ID;
   }

// ==========================================================================
// Graph database metadata retrieval methods
// ==========================================================================

// Method retrieve_level_for_particular_graph() takes in the ID for
// some entry in the graphs table of the IMAGERY database.  It finds and
// returns the level within graph_hierarchy to which the specified
// graph belongs.

   int retrieve_level_for_particular_graph(
      gis_database* gis_database_ptr,int graph_ID)
   {
//      cout << "inside graphdbfunc::retrieve_level_for_particular_graph()" 
//           << endl;
//      cout << "graph_ID = " << graph_ID << endl;

      int graph_level=-1;
      
      string curr_select_cmd="select level from graphs ";
      curr_select_cmd += "where graph_id="+
         stringfunc::number_to_string(graph_ID);
      
//      cout << "curr_select_cmd = " << curr_select_cmd << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);
      if (field_array_ptr==NULL) return graph_level;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      graph_level=stringfunc::string_to_number(field_array_ptr->get(0,0));
//      cout << "At end of retrieve_level_for_particular_graph(), graph_level = "
//           << graph_level << endl;
      return graph_level;
   }

// ---------------------------------------------------------------------   
// Method retrieve_all_graph_levels_for_particular_hierarchy() takes
// in the ID for some particular graph hierarchy.  It returns an STL
// vector containing pairs of graph IDs vs levels.

   vector<twovector> retrieve_all_graph_levels_for_particular_hierarchy(
      gis_database* gis_database_ptr,int hierarchy_ID)
   {
//      cout << "inside graphdbfunc::retrieve_all_graph_levels_for_particular_hierarchy()" 
//           << endl;
//      cout << "hierarchy_ID = " << hierarchy_ID << endl;

      vector<twovector> graphID_graphLevel;

      string curr_select_cmd="select graph_id,level from graphs ";
      curr_select_cmd += "where graph_hierarchy_id="+
         stringfunc::number_to_string(hierarchy_ID);
      curr_select_cmd += " order by graph_id";
//      cout << "curr_select_cmd = " << curr_select_cmd << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);
      if (field_array_ptr==NULL) return graphID_graphLevel;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;
      unsigned int mdim=field_array_ptr->get_mdim();
      for (unsigned int m=0; m<mdim; m++)
      {
         int graphID=stringfunc::string_to_number(field_array_ptr->get(m,0));
         int graphLevel=stringfunc::string_to_number(
            field_array_ptr->get(m,1));
//         cout << "graphID = " << graphID 
//              << " graphLevel = " << graphLevel << endl;
         graphID_graphLevel.push_back(twovector(graphID,graphLevel));
      }
      
      return graphID_graphLevel;
   }

// ---------------------------------------------------------------------   
// Member function generate_retrieve_graph_hierarchy_SQL_command()

   string generate_retrieve_graph_hierarchy_SQL_command(int hierarchy_ID)
   {
//      cout << "inside graphdbfunc::generate_retrieve_graph_hierarchy_SQL_command()" 
//           << endl;

      string SQL_command=
         "SELECT name,n_graphs,n_levels,n_connected_components ";
      SQL_command += "from graph_hierarchies ";
      SQL_command += "WHERE graph_hierarchy_ID="+stringfunc::number_to_string(
         hierarchy_ID);
//      cout << "SQL_command = " << SQL_command << endl;
      return SQL_command;
   }

// ---------------------------------------------------------------------   
   void retrieve_graph_hierarchy_metadata_from_database(
      gis_database* gis_database_ptr,int hierarchy_ID,
      string& description,unsigned int& n_graphs,unsigned int& n_levels,
      unsigned int& n_connected_components)
   {
//      cout << "inside graphdbfunc::retrieve_graph_hierarchy_metadata_from_database()" 
//           << endl;

      string curr_select_cmd=generate_retrieve_graph_hierarchy_SQL_command(
         hierarchy_ID);
//      cout << "curr_select_cmd = " << curr_select_cmd << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);
      if (field_array_ptr==NULL) return;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      description="\""+field_array_ptr->get(0,0)+"\"";
      n_graphs=stringfunc::string_to_number(field_array_ptr->get(0,1));
      n_levels=stringfunc::string_to_number(field_array_ptr->get(0,2));
      n_connected_components=
         stringfunc::string_to_number(field_array_ptr->get(0,3));

//      cout << "description = " << description << endl;
//      cout << "n_graphs = " << n_graphs 
//           << " n_levels = " << n_levels 
//           << " n_connected_components = " << n_connected_components
//           << endl;
   }

// ---------------------------------------------------------------------   
// Member function generate_retrieve_graphs_SQL_command()

   string generate_retrieve_graphs_SQL_command(int hierarchy_ID)
   {
//      cout << "inside graphdbfunc::generate_retrieve_graphs_SQL_command()" 
//           << endl;

      string SQL_command="SELECT graph_ID,level,parent_graph_ID,n_nodes,n_links ";
      SQL_command += " from graphs ";
      SQL_command += " WHERE graph_hierarchy_ID="+stringfunc::number_to_string(
         hierarchy_ID);
      SQL_command += " ORDER by graph_ID";
//      cout << "SQL_command = " << SQL_command << endl;
      return SQL_command;
   }

// ---------------------------------------------------------------------   
   void retrieve_graphs_metadata_from_database(
      gis_database* gis_database_ptr,int hierarchy_ID,unsigned int n_graphs,
      vector<int>& graph_ID,vector<int>& graph_level,
      vector<int>& parent_graph_ID, vector<int>& n_nodes,vector<int>& n_links)
   {
//      cout << "inside graphdbfunc::retrieve_graphs_metadata_from_database()" 
//           << endl;

      string curr_select_cmd=generate_retrieve_graphs_SQL_command(
         hierarchy_ID);
//      cout << "curr_select_cmd = " << curr_select_cmd << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);
      if (field_array_ptr==NULL) return;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;
//      cout << "n_graphs = " << n_graphs << endl;

      for (unsigned int g=0; g<n_graphs; g++)
      {
         graph_ID.push_back(
            stringfunc::string_to_number(field_array_ptr->get(g,0)));
         graph_level.push_back(
            stringfunc::string_to_number(field_array_ptr->get(g,1)));
         parent_graph_ID.push_back(
            stringfunc::string_to_number(field_array_ptr->get(g,2)));
         n_nodes.push_back(
            stringfunc::string_to_number(field_array_ptr->get(g,3)));
         n_links.push_back(
            stringfunc::string_to_number(field_array_ptr->get(g,4)));

//         cout << "g = " << g
//              << " graph_ID = " << graph_ID.back()
//              << " graph_level = " << graph_level.back()
//              << " parent_graph_ID = " << parent_graph_ID.back()
//              << " n_nodes = " << n_nodes.back()
//              << " n_links = " << n_links.back()
//              << endl;
      } // loop over index g labeling graphs within graph_hierarchy
   }

// ---------------------------------------------------------------------   
// Member function generate_retrieve_particular_graph_SQL_command()

   string generate_retrieve_particular_graph_SQL_command(
      int hierarchy_ID,int graph_ID)
   {
//      cout << "inside graphdbfunc::generate_retrieve_particular_graph_SQL_command()" 
//           << endl;

      string SQL_command="SELECT level,parent_graph_ID,n_nodes,n_links ";
      SQL_command += " from graphs ";
      SQL_command += "WHERE graph_hierarchy_ID="+stringfunc::number_to_string(
         hierarchy_ID);
      SQL_command += " AND graph_ID="+stringfunc::number_to_string(
         graph_ID);
//      cout << "SQL_command = " << SQL_command << endl;
      return SQL_command;
   }

// ---------------------------------------------------------------------   
// Method retrieve_particular_graph_metadata_from_database()

   bool retrieve_particular_graph_metadata_from_database(
      gis_database* gis_database_ptr,int hierarchy_ID,int graph_ID,
      int& level,int& parent_graph_ID,int& n_nodes,int& n_links)
   {
//      cout << "inside graphdbfunc::retrieve_particular_graph_metadata_from_database()" 
//           << endl;

      string curr_select_cmd=generate_retrieve_particular_graph_SQL_command(
         hierarchy_ID,graph_ID);
//      cout << "curr_select_cmd = " << curr_select_cmd << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);
      if (field_array_ptr==NULL) return false;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      level=stringfunc::string_to_number(field_array_ptr->get(0,0));
      parent_graph_ID=stringfunc::string_to_number(field_array_ptr->get(0,1));
      n_nodes=stringfunc::string_to_number(field_array_ptr->get(0,2));
      n_links=stringfunc::string_to_number(field_array_ptr->get(0,3));
      return true;
   }

// ---------------------------------------------------------------------   
// Method generate_retrieve_nodes_SQL_command()

   string generate_retrieve_nodes_SQL_command(
      int graph_hierarchy_ID,int graph_ID)
   {
//      cout << "inside graphdbfunc::generate_retrieve_nodes_SQL_command()" 
//           << endl;

      string SQL_command=
         "SELECT node_ID,connected_component_ID,parent_node_ID,data_ID,";
      SQL_command += " color,relative_size,gx,gy,gx2,gy2,label from nodes";
      SQL_command += " WHERE graph_ID="+stringfunc::number_to_string(
         graph_ID);
      SQL_command += " AND graph_hierarchy_ID="+stringfunc::number_to_string(
         graph_hierarchy_ID);
      SQL_command += " ORDER by node_id";
//      cout << "SQL_command = " << SQL_command << endl;
      return SQL_command;
   }

// ---------------------------------------------------------------------   
// Method retrieve_nodes_metadata_from_database()

   void retrieve_nodes_metadata_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,int graph_ID,
      vector<int>& node_ID,vector<int>& connected_component_ID,
      vector<int>& parent_node_ID,
      vector<int>& data_ID,vector<string>& color,vector<double>& relative_size,
      vector<twovector>& gxgy,vector<twovector>& gxgy2,vector<string>& label)
   {
//      cout << "inside graphdbfunc::retrieve_nodes_metadata_from_database()" 
//           << endl;

      string curr_select_cmd=generate_retrieve_nodes_SQL_command(
         graph_hierarchy_ID,graph_ID);
//      cout << "curr_select_cmd = " << curr_select_cmd << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);
      if (field_array_ptr==NULL) return;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      unsigned int mdim=field_array_ptr->get_mdim();
      for (unsigned int n=0; n<mdim; n++)
      {
         node_ID.push_back(
            stringfunc::string_to_number(field_array_ptr->get(n,0)));
         connected_component_ID.push_back(
            stringfunc::string_to_number(field_array_ptr->get(n,1))); 
         parent_node_ID.push_back(
            stringfunc::string_to_number(field_array_ptr->get(n,2))); 
         data_ID.push_back(
            stringfunc::string_to_number(field_array_ptr->get(n,3)));
         color.push_back(field_array_ptr->get(n,4));
         relative_size.push_back(
            stringfunc::string_to_number(field_array_ptr->get(n,5)));
         gxgy.push_back(twovector(
            stringfunc::string_to_number(field_array_ptr->get(n,6)),
            stringfunc::string_to_number(field_array_ptr->get(n,7))));

         twovector curr_gxgy2(NEGATIVEINFINITY,NEGATIVEINFINITY);
         if (stringfunc::is_number(field_array_ptr->get(n,8)) &&
         stringfunc::is_number(field_array_ptr->get(n,9)))
         {
            curr_gxgy2=twovector(
               stringfunc::string_to_number(field_array_ptr->get(n,8)),
               stringfunc::string_to_number(field_array_ptr->get(n,9)));
         }
         gxgy2.push_back(curr_gxgy2);
         label.push_back(field_array_ptr->get(n,10));

//         cout << "n = " << n
//              << " node_ID = " << node_ID.back()
//              << " data_ID = " << data_ID.back() 
//              << " label = " << label.back() 
//              << endl;
      } // loop over index n labeling nodes within graph
   }

// ---------------------------------------------------------------------   
// Method generate_retrieve_links_SQL_command()

   string generate_retrieve_links_SQL_command(
      int graph_hierarchy_ID,int graph_ID)
   {
//      cout << "inside graphdbfunc::generate_retrieve_links_SQL_command()" 
//           << endl;

      string SQL_command="SELECT link_ID,source_node_ID,target_node_ID,";
      SQL_command += " directed,weight,color,relative_size from links ";
      SQL_command += " WHERE graph_ID="+stringfunc::number_to_string(
         graph_ID);
      SQL_command += " AND graph_hierarchy_ID="+stringfunc::number_to_string(
         graph_hierarchy_ID);
      SQL_command += " ORDER by link_id";
//      cout << "SQL_command = " << SQL_command << endl;
      return SQL_command;
   }

// ---------------------------------------------------------------------   
// Method retrieve_links_metadata_from_database()

   void retrieve_links_metadata_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,int graph_ID,
      vector<int>& link_ID,vector<int>& source_node_ID,
      vector<int>& target_node_ID,vector<bool>& directed,
      vector<double>& weight,vector<string>& color,
      vector<double>& relative_size)
   {
//      cout << "inside graphdbfunc::retrieve_links_metadata_from_database()" 
//           << endl;

      string curr_select_cmd=generate_retrieve_links_SQL_command(
         graph_hierarchy_ID,graph_ID);
//      cout << "curr_select_cmd = " << curr_select_cmd << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);
      if (field_array_ptr==NULL) return;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      unsigned int mdim=field_array_ptr->get_mdim();
      for (unsigned int n=0; n<mdim; n++)
      {
         link_ID.push_back(
            stringfunc::string_to_number(field_array_ptr->get(n,0)));
         source_node_ID.push_back(
            stringfunc::string_to_number(field_array_ptr->get(n,1)));
         target_node_ID.push_back(
            stringfunc::string_to_number(field_array_ptr->get(n,2)));
         bool directed_flag=true;
         if (field_array_ptr->get(n,3)=="f" ||
             field_array_ptr->get(n,3)=="F") directed_flag=false;

         directed.push_back(directed_flag);
         weight.push_back(
            stringfunc::string_to_number(field_array_ptr->get(n,4)));
         color.push_back(field_array_ptr->get(n,5));
         relative_size.push_back(
            stringfunc::string_to_number(field_array_ptr->get(n,6)));
      } // loop over index n labeling links within graph
   }

// ---------------------------------------------------------------------   
// Method reconstruct_graph_from_database()

   graph* reconstruct_graph_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,int graph_ID,
      int level)
   {
//      cout << "inside graphdbfunc::reconstruct_graph_from_database()" << endl;
//      cout << "graph_ID = " << graph_ID << " level = " << level << endl;
      graph* graph_ptr=new graph(graph_ID,level);

// Recover information needed to reconstruct all nodes from database:

      vector<int> node_ID,connected_component_ID,parent_node_ID,data_ID;
      vector<double> node_rel_size;
      vector<string> node_color,label;
      vector<twovector> gxgy,gxgy2;
      retrieve_nodes_metadata_from_database(
         gis_database_ptr,graph_hierarchy_ID,graph_ID,
         node_ID,connected_component_ID,parent_node_ID,data_ID,
         node_color,node_rel_size,gxgy,gxgy2,label);

      for (unsigned int n=0; n<node_ID.size(); n++)
      {
//         cout << "n = " << n << " node_ID = " << node_ID[n] << endl;
         node* node_ptr=new node(node_ID[n],level);
         node_ptr->set_parent_ID(parent_node_ID[n]);
         node_ptr->set_data_ID(data_ID[n]);
         node_ptr->set_hex_color(node_color[n]);
         node_ptr->set_relative_size(node_rel_size[n]);
         node_ptr->set_Uposn(gxgy[n].get(0));
         node_ptr->set_Vposn(gxgy[n].get(1));
         node_ptr->set_U2posn(gxgy2[n].get(0));
         node_ptr->set_V2posn(gxgy2[n].get(1));
         node_ptr->set_label(label[n]);
//         cout << "label = " << label[n] << endl;
         
         graph_ptr->add_node(node_ptr);
      }
      
// Recover information needed to reconstruct all links from database:

      vector<bool> directed;
      vector<int> link_ID,source_node_ID,target_node_ID;
      vector<double> weight,link_rel_size;
      vector<string> link_color;
      retrieve_links_metadata_from_database(
         gis_database_ptr,graph_hierarchy_ID,graph_ID,
         link_ID,source_node_ID,target_node_ID,
         directed,weight,link_color,link_rel_size);

      for (unsigned int l=0; l<link_ID.size(); l++)
      {
//         cout << "l = " << l << " link_ID = " << link_ID[l] << endl;
//         cout << " source_node = " << source_node_ID[l]
//              << " target_node = " << target_node_ID[l]
//              << " weight = " << weight[l] 
//              << endl;
         
         graph_edge* graph_edge_ptr=graph_ptr->add_graph_edge(
            source_node_ID[l],target_node_ID[l],weight[l]);
         graph_edge_ptr->set_directed_flag(directed[l]);
         graph_edge_ptr->set_hex_color(link_color[l]);
         graph_edge_ptr->set_relative_size(link_rel_size[l]);
      }
      
      return graph_ptr;
   }

// ---------------------------------------------------------------------   
// Method reconstruct_graph_hierarchy_from_database()

   graph_hierarchy* reconstruct_graph_hierarchy_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID)
   {
//      cout << "inside graphdbfunc::reconstruct_graph_hierarchy_from_database()"
//           << endl;

      delete graph_hierarchy_ptr;
      graph_hierarchy_ptr=new graph_hierarchy(graph_hierarchy_ID);

      unsigned int n_graphs=0;
      unsigned int n_levels,n_connected_components;
      string description;
      retrieve_graph_hierarchy_metadata_from_database(
         gis_database_ptr,graph_hierarchy_ID,description,n_graphs,n_levels,
         n_connected_components);
      graph_hierarchy_ptr->set_label(description);

      vector<int> graph_ID,graph_level,parent_graph_ID,n_nodes,n_links;
      retrieve_graphs_metadata_from_database(
         gis_database_ptr,graph_hierarchy_ID,n_graphs,
         graph_ID,graph_level,parent_graph_ID,n_nodes,n_links);

      for (unsigned int g=0; g<n_graphs; g++)
      {
         graph* curr_graph_ptr=reconstruct_graph_from_database(
            gis_database_ptr,graph_hierarchy_ID,graph_ID[g],graph_level[g]);
         graph_hierarchy_ptr->add_graph_ptr(curr_graph_ptr);
      }

      return graph_hierarchy_ptr;
   }

// ==========================================================================
// Graphs from JSON file methods
// ==========================================================================

// Method reconstruct_graph_hierarchy_from_JSON_file()

   graph_hierarchy* reconstruct_graph_hierarchy_from_JSON_file(
      string json_filename,int graph_hierarchy_ID,
      unsigned int& n_graphs,unsigned int& n_levels,
      unsigned int& n_connected_components,string& description,
      vector<int>& graph_ID,vector<int>& graph_level,
      vector<int>& parent_graph_ID,
      vector<int>& n_nodes,vector<int>& n_links)
   {
//      cout << "inside graphdbfunc::reconstruct_graph_hierarchy_from_JSON_file()"
//           << endl;

      delete graph_hierarchy_ptr;
      graph_hierarchy_ptr=new graph_hierarchy(graph_hierarchy_ID);

      n_graphs=1;
      n_levels=1;
      n_connected_components=1;
      description="\"Neural Net\"";
      graph_hierarchy_ptr->set_label(description);

      graph_ID.push_back(0);
      graph_level.push_back(0);
      parent_graph_ID.push_back(-1);
      for (unsigned int g=0; g<n_graphs; g++)
      {
         graph* curr_graph_ptr=new graph(graph_ID[g],graph_level[g]);
         graph_hierarchy_ptr->add_graph_ptr(curr_graph_ptr);
      }

      count_nodes_links_in_JSON_file(json_filename,n_nodes,n_links);

      return graph_hierarchy_ptr;
   }

// ---------------------------------------------------------------------   
// Method count_nodes_links_in_JSON_file()

   void count_nodes_links_in_JSON_file(
      string json_filename,vector<int>& n_nodes,vector<int>& n_links)
   {
//      cout << "inside graphdbfunc::count_nodes_links_in_JSON_file()" << endl;

      cppJSON* cppJSON_ptr=new cppJSON();
      string json_string=get_JSON_string_from_JSON_file(json_filename);
      cJSON* root_ptr=cppJSON_ptr->parse_json(json_string);
      cppJSON_ptr->generate_JSON_tree();
      cppJSON_ptr->extract_key_value_pairs(root_ptr);

      unsigned int n_JSON_objects=cppJSON_ptr->get_n_objects();

      int number_nodes=0;
      int number_links=0;
      for (unsigned int n=0; n<n_JSON_objects; n++)
      {
         vector<cppJSON::KEY_VALUE_PAIR> key_value_pairs=
            cppJSON_ptr->get_object_key_value_pairs(n);
         for (unsigned int k=0; k<key_value_pairs.size(); k++)
         {
            cppJSON::KEY_VALUE_PAIR curr_key_value_pair=key_value_pairs[k];

            string key=curr_key_value_pair.first;
            string value=curr_key_value_pair.second;
            if (key=="type" && value=="\"NODE\"") number_nodes++;
            if (key=="source") number_links++;
         }
      } // loop over index n labeling JSON objects

      delete cppJSON_ptr;

      n_nodes.push_back(number_nodes);
      n_links.push_back(number_links);

//      cout << "number_nodes = " << number_nodes
//           << " number_links = " << number_links << endl;
   }

// ---------------------------------------------------------------------   
// Method function get_JSON_string_from_JSON_file()

   string get_JSON_string_from_JSON_file(string json_filename)
   {
      filefunc::ReadInfile(json_filename);
      string json_string;
      for (unsigned int i=0; i<filefunc::text_line.size(); i++)
      {
         json_string += filefunc::text_line[i];
      }
//   cout << "json_string = " << json_string << endl;
      return json_string;
   }

// ==========================================================================
// Sibling search methods
// ==========================================================================

// Method generate_retrieve_sibling_node_IDs_SQL_command() takes in a
// graph_hierarchy_ID along with a parent_node_ID.  It returns the SQL
// string needed to retrieve all children nodes who share the same
// parent node.

   string generate_retrieve_sibling_node_IDs_SQL_command(
      int graph_hierarchy_ID,int parent_node_ID)
   {
//      cout << "inside graphdbfunc::generate_retrieve_sibling_node_IDs_SQL_command()" 
//           << endl;
//      cout << "parent_node_ID = " << parent_node_ID << endl;

      string SQL_command="SELECT node_ID from nodes";
      SQL_command += " WHERE graph_hierarchy_ID="
         +stringfunc::number_to_string(graph_hierarchy_ID);
      SQL_command += " AND parent_node_ID="+stringfunc::number_to_string(
         parent_node_ID);
      SQL_command += " ORDER by node_id";
//      cout << "SQL_command = " << SQL_command << endl;
      return SQL_command;
   }

// ---------------------------------------------------------------------   
// Method retrieve_sibling_node_IDs_from_database() takes in the IDs
// for a graph_hierarchy and a node.  This method first retrieves the
// parent ID for the input node.  It then issues a SQL call to
// *gis_database_ptr for all nodes with the same parent ID.  This
// method returns an STL vector containing the IDs of all nodes with
// the same parent.

   vector<int> retrieve_sibling_node_IDs_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,int node_ID)
   {
//      cout << "inside graphdbfunc::retrieve_sibling_node_IDs_from_database()"
//           << endl;
//      cout << "hierarchy_ID = " << graph_hierarchy_ID << endl;
//      cout << "node_ID = " << node_ID << endl;

      int parent_node_ID=retrieve_parent_node_ID_from_database(
         gis_database_ptr,graph_hierarchy_ID,node_ID);
//      cout << "parent_node_ID = " << parent_node_ID << endl;
      
      string curr_select_cmd=generate_retrieve_sibling_node_IDs_SQL_command(
         graph_hierarchy_ID,parent_node_ID);
//      cout << "curr_select_cmd = " << curr_select_cmd << endl;
      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);

      vector<int> sibling_node_IDs;
      if (field_array_ptr==NULL) return sibling_node_IDs;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      for (unsigned int m=0; m<field_array_ptr->get_mdim(); m++)
      {
         int sibling_ID=stringfunc::string_to_number(
            field_array_ptr->get(m,0));
         sibling_node_IDs.push_back(sibling_ID);
      } // loop over index m 

      return sibling_node_IDs;
   }

// ---------------------------------------------------------------------   
// Method retrieve_sibling_data_IDs_from_database() takes in the graph
// level and ID for some particular datum.  It first recovers the
// graph hierarchy and node IDs corresponding to the input datum.
// This method next fetches sibling node IDs.  Finally, it converts
// the sibling node IDs into sibling data IDs.

   vector<int> retrieve_sibling_data_IDs_from_database(
      gis_database* gis_database_ptr,int hierarchy_ID,int graph_level,
      int datum_ID)
   {
//      cout << "inside graphdbfunc::retrieve_sibling_data_IDs_from_database()"
//           << endl;
//      cout << "Hierarchy_ID = " << hierarchy_ID 
//           << " graph_level = " << graph_level
//           << " datum_ID = " << datum_ID << endl;

      vector<int> sibling_data_IDs;

      int node_ID=-1;
      graphdbfunc::retrieve_node_ID_for_particular_hierarchy_datum(
         gis_database_ptr,hierarchy_ID,graph_level,datum_ID,node_ID);
//      cout << " node_ID = " << node_ID << endl;

      if (node_ID==-1)
      {
         cout << "Error in graphdbfunc::retrieve_sibling_data_IDs_from_database()" << endl;
         cout << " node_ID = " << node_ID << endl;
         return sibling_data_IDs;
      }

      vector<int> sibling_node_IDs=
         graphdbfunc::retrieve_sibling_node_IDs_from_database(
            gis_database_ptr,hierarchy_ID,node_ID);

      NODE_DATA_IDS_MAP* node_data_ids_map_ptr=
         retrieve_node_data_IDs_for_particular_graph_hierarchy(
            gis_database_ptr,hierarchy_ID);

// Note added on Fri, 7/22/2011 at 6:46 pm: This next loop used to be very
// slow for MIT32K level 2 !!!  But as of Sat July 23, 2011 we believe
// that it now executes much faster...

      for (unsigned int s=0; s<sibling_node_IDs.size(); s++)
      {
         NODE_DATA_IDS_MAP::iterator iter=node_data_ids_map_ptr->find(
            sibling_node_IDs[s]);
         sibling_data_IDs.push_back(iter->second);
//         cout << "s = " << s 
//              << " sibling_datum_ID = " << sibling_data_IDs.back()
//              << endl;
      }

      delete node_data_ids_map_ptr;
      return sibling_data_IDs;
   }

// ---------------------------------------------------------------------   
// Method get_n_siblings_from_database()

   int get_n_siblings_from_database(
      gis_database* gis_database_ptr,int hierarchy_ID,int graph_level,
      int datum_ID)
   {
//      cout << "inside graphdbfunc::get_n_siblings_from_database()" << endl;
//      cout << "graph_level = " << graph_level
//           << " datum_ID = " << datum_ID << endl;

      if (graph_level < 0 || datum_ID < 0)
      {
         cout << "Error in graphdbfunc::get_n_siblings_from_database()" 
              << endl;
         cout << "graph_level = " << graph_level 
              << " datum_ID = " << datum_ID << endl;
         exit(-1);
      }
      
      int n_siblings=retrieve_sibling_data_IDs_from_database(
         gis_database_ptr,hierarchy_ID,graph_level,datum_ID).size();
//      cout << "n_siblings = " << n_siblings << endl << endl;
      return n_siblings;
   }

// ---------------------------------------------------------------------   
// Method retrieve_cousin_node_IDs_from_database() takes in the IDs
// for a graph_hierarchy, graph and particular datum.  This method
// first the corresponding node ID.  It next retrieves the
// grandparent ID for the node.  It then issues a SQL call to
// *gis_database_ptr for all nodes with the same grandparent.
// This method returns an STL vector containing the grandchildren IDs
// of all nodes with the same grandparent.

   vector<int> retrieve_cousin_node_IDs_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,int node_ID)
   {
//      cout << "inside graphdbfunc::retrieve_cousin_node_IDs_from_database()"
//           << endl;
//      cout << "hierarchy_ID = " << graph_hierarchy_ID << endl;
 //     cout << "node_ID = " << node_ID << endl;

      int grandparent_node_ID=retrieve_grandparent_node_ID_from_database(
         gis_database_ptr,graph_hierarchy_ID,node_ID);
//      cout << "grandparent_node_ID = " << grandparent_node_ID
//           << endl;
      
      return retrieve_grandchildren_node_IDs_from_database(
         gis_database_ptr,graph_hierarchy_ID,grandparent_node_ID);
   }

// ---------------------------------------------------------------------      
   vector<int> retrieve_cousin_node_IDs_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,
      int graph_ID,int datum_ID)
   {
//      cout << "inside graphdbfunc::retrieve_cousin_node_IDs_from_database()"
//           << endl;
//      cout << "hierarchy_ID = " << graph_hierarchy_ID << endl;
//      cout << "graph_ID = " << graph_ID << endl;
//      cout << "datum_ID = " << datum_ID << endl;

      int node_ID;
      retrieve_node_ID_for_particular_hierarchy_datum(
         gis_database_ptr,graph_hierarchy_ID,graph_ID,datum_ID,node_ID);
//      cout << "node_ID = " << node_ID << endl;

      return retrieve_cousin_node_IDs_from_database(
         gis_database_ptr,graph_hierarchy_ID,node_ID);
   }

// ---------------------------------------------------------------------   
   vector<int> retrieve_cousin_data_IDs_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,
      int graph_ID,int datum_ID)
   {
//      cout << "inside graphdbfunc::retrieve_cousin_data_IDs_from_database()"
//           << endl;
      
      vector<int> cousin_node_IDs=retrieve_cousin_node_IDs_from_database(
         gis_database_ptr,graph_hierarchy_ID,graph_ID,datum_ID);

      NODE_DATA_IDS_MAP* node_data_ids_map_ptr=
         retrieve_node_data_IDs_for_particular_graph_hierarchy(
            gis_database_ptr,graph_hierarchy_ID);

      vector<int> cousin_data_IDs;
      for (unsigned int s=0; s<cousin_node_IDs.size(); s++)
      {
         NODE_DATA_IDS_MAP::iterator iter=node_data_ids_map_ptr->find(
            cousin_node_IDs[s]);
         cousin_data_IDs.push_back(iter->second);
      }
      delete node_data_ids_map_ptr;

      return cousin_data_IDs;
   }

// ==========================================================================
// Children search methods
// ==========================================================================

// Method retrieve_children_node_IDs_from_database() takes in the IDs
// for a graph_hierarchy and a node.  It then issues a SQL call to
// *gis_database_ptr for all nodes whose parent has the input node_ID.
// This method returns an STL vector containing the IDs of all nodes
// with the same parent.

   vector<int> retrieve_children_node_IDs_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,int node_ID)
   {
//      cout << "inside graphdbfunc::retrieve_children_node_IDs_from_database()"
//           << endl;
//      cout << "hierarchy_ID = " << graph_hierarchy_ID << endl;
//      cout << "node_ID = " << node_ID << endl;

      string curr_select_cmd=generate_retrieve_sibling_node_IDs_SQL_command(
         graph_hierarchy_ID,node_ID);
//      cout << "curr_select_cmd = " << curr_select_cmd << endl;
      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);

      vector<int> children_node_IDs;
      if (field_array_ptr==NULL) return children_node_IDs;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      for (unsigned int m=0; m<field_array_ptr->get_mdim(); m++)
      {
         int child_ID=stringfunc::string_to_number(
            field_array_ptr->get(m,0));
         children_node_IDs.push_back(child_ID);
      } // loop over index m 

      return children_node_IDs;
   }

// ---------------------------------------------------------------------   
// Method retrieve_children_data_IDs_from_database() takes in the graph
// level and ID for some particular datum.  It first recovers the
// graph hierarchy and node IDs corresponding to the input datum.
// This method next fetches children node IDs.  Finally, it converts
// the children node IDs into children data IDs.

   vector<int> retrieve_children_data_IDs_from_database(
      gis_database* gis_database_ptr,int hierarchy_ID,int graph_level,
      int datum_ID)
   {
//      cout << "inside graphdbfunc::retrieve_children_data_IDs_from_database()"
//           << endl;
//      cout << "hierarchy_ID = " << hierarchy_ID << endl;
//      cout << "graph_level = " << graph_level
//           << " datum_ID = " << datum_ID << endl;

      vector<int> children_data_IDs;

      int node_ID=-1;
      graphdbfunc::retrieve_node_ID_for_particular_hierarchy_datum(
         gis_database_ptr,hierarchy_ID,graph_level,datum_ID,node_ID);

      if (node_ID==-1)
      {
         cout << "Error in graphdbfunc::retrieve_children_data_IDs_from_database()" << endl;
         cout << " node_ID = " << node_ID << endl;
         return children_data_IDs;
      }
//       cout << " node_ID = " << node_ID << endl;

      vector<int> children_node_IDs=
         graphdbfunc::retrieve_children_node_IDs_from_database(
            gis_database_ptr,hierarchy_ID,node_ID);

      NODE_DATA_IDS_MAP* node_data_ids_map_ptr=
         retrieve_node_data_IDs_for_particular_graph_hierarchy(
            gis_database_ptr,hierarchy_ID);

      for (unsigned int s=0; s<children_node_IDs.size(); s++)
      {
         NODE_DATA_IDS_MAP::iterator iter=node_data_ids_map_ptr->find(
            children_node_IDs[s]);
         children_data_IDs.push_back(iter->second);
      }
      delete node_data_ids_map_ptr;

      return children_data_IDs;
   }

// ---------------------------------------------------------------------   
// Method get_n_children_from_database()

   int get_n_children_from_database(
      gis_database* gis_database_ptr,int hierarchy_ID,int graph_level,
      int datum_ID)
   {
      int n_children=retrieve_children_data_IDs_from_database(
         gis_database_ptr,hierarchy_ID,graph_level,datum_ID).size();
//      cout << "n_children = " << n_children << endl;
      return n_children;
   }

// ---------------------------------------------------------------------
// Method retrieve_grandchildren_node_IDs_from_database() 

   vector<int> retrieve_grandchildren_node_IDs_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,
      int grandparent_node_ID)
   {
//      cout << "inside graphdbfunc::retrieve_grandchildren_node_IDs_from_database()"
//           << endl;

      string SQL_cmd="WITH parent_nodes AS ( ";
      SQL_cmd += "SELECT node_id FROM nodes WHERE parent_node_id="+
         stringfunc::number_to_string(grandparent_node_ID)+") ";
      SQL_cmd += "SELECT node_id FROM nodes ";
      SQL_cmd += "WHERE parent_node_ID IN (SELECT node_id from parent_nodes) ";
      SQL_cmd += "AND graph_hierarchy_ID="+stringfunc::number_to_string(
         graph_hierarchy_ID);
      SQL_cmd += " ORDER BY node_id";

//      cout << "SQL_cmd = " << SQL_cmd << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      vector<int> grandchildren_node_IDs;
      for (unsigned int m=0; m<field_array_ptr->get_mdim(); m++)
      {
         int grandchild_node_ID=stringfunc::string_to_number(
            field_array_ptr->get(m,0));
         grandchildren_node_IDs.push_back(grandchild_node_ID);
      } // loop over index m 
      return grandchildren_node_IDs;
   }

// ==========================================================================
// Parent search methods
// ==========================================================================

// Method generate_retrieve_parent_node_ID_SQL_command()

   string generate_retrieve_parent_node_ID_SQL_command(
      int graph_hierarchy_ID,int node_ID)
   {
//      cout << "inside graphdbfunc::generate_retrieve_parent_node_ID_SQL_command()" 
//           << endl;

      string SQL_command="SELECT parent_node_ID from nodes";
      SQL_command += " WHERE graph_hierarchy_ID="+
         stringfunc::number_to_string(graph_hierarchy_ID);
      SQL_command += " AND node_ID="+
         stringfunc::number_to_string(node_ID);
//      cout << "SQL_command = " << SQL_command << endl;

      return SQL_command;
   }

// ---------------------------------------------------------------------   
// Method retrieve_parent_node_ID_from_database() takes in the IDs
// for a graph_hierarchy and a node.  It retrieves and returns the
// parent ID for the input node.  

   int retrieve_parent_node_ID_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,int node_ID)
   {
//      cout << "inside graphdbfunc::retrieve_parent_node_ID_from_database()"
//           << endl;

      string curr_select_cmd=generate_retrieve_parent_node_ID_SQL_command(
         graph_hierarchy_ID,node_ID);

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);
      if (field_array_ptr==NULL) return -1;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      int parent_node_ID=stringfunc::string_to_number(
            field_array_ptr->get(0,0));

//      cout << "graph_hierarchy_ID = " << graph_hierarchy_ID << endl;
//      cout << "node_ID = " << node_ID << endl;
//      cout << "parent_node_ID = " << parent_node_ID << endl;

      return parent_node_ID;
   }

// ---------------------------------------------------------------------   
// Method retrieve_parent_datum_IDs_from_database() takes in the graph
// level and ID for some particular datum.  It first recovers the
// graph hierarchy and node ID corresponding to the input datum.
// This method next fetches the parent node's ID.  Finally, it
// converts the parent node ID into a parent datum ID.

   int retrieve_parent_datum_ID_from_database(
      gis_database* gis_database_ptr,int hierarchy_ID,int graph_level,
      int datum_ID)
   {
//      cout << "inside graphdbfunc::retrieve_parent_datum_ID_from_database()"
//           << endl;
//      cout << "datum_ID = " << datum_ID << endl;
      
      int node_ID=-1;
      graphdbfunc::retrieve_node_ID_for_particular_hierarchy_datum(
         gis_database_ptr,hierarchy_ID,graph_level,datum_ID,node_ID);

      if (node_ID==-1)
      {
         cout << "Error in graphdbfunc::retrieve_parent_datum_ID_from_database()" << endl;
         cout << " node_ID = " << node_ID << endl;
         return -1;
      }

//      cout << " node_ID = " << node_ID << endl;

      int parent_node_ID=retrieve_parent_node_ID_from_database(
         gis_database_ptr,hierarchy_ID,node_ID);

      NODE_DATA_IDS_MAP* node_data_ids_map_ptr=
         retrieve_node_data_IDs_for_particular_graph_hierarchy(
            gis_database_ptr,hierarchy_ID);

      NODE_DATA_IDS_MAP::iterator iter=node_data_ids_map_ptr->find(
         parent_node_ID);

      int parent_datum_ID=iter->second;
      return parent_datum_ID;
   }

// ---------------------------------------------------------------------   
// Method retrieve_grandparent_node_ID_from_database() takes in the
// IDs for a graph_hierarchy and a node.  It retrieves and returns the
// grandparent ID for the input node.

   int retrieve_grandparent_node_ID_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,int node_ID)
   {
//      cout << "inside graphdbfunc::retrieve_grandparent_node_ID_from_database()"
//           << endl;
      int parent_node_ID=retrieve_parent_node_ID_from_database(
         gis_database_ptr,graph_hierarchy_ID,node_ID);
//      cout << "parent_node_ID = " << parent_node_ID << endl;
      
      int grandparent_node_ID=retrieve_parent_node_ID_from_database(
         gis_database_ptr,graph_hierarchy_ID,parent_node_ID);
//      cout << "grandparent_node_ID = " << grandparent_node_ID << endl;
      return grandparent_node_ID;
   }

// ==========================================================================
// Node label methods
// ==========================================================================

// Method generate_update_node_label_SQL_command() 

   string generate_update_node_label_SQL_command(
      int graph_hierarchy_ID,int node_ID,string label)
   {
//      cout << "inside graphdbfunc::generate_update_node_label_SQL_command()" 
//           << endl;

      string SQL_command="update nodes set label='";
      SQL_command += label+"' where graph_hierarchy_ID=";
      SQL_command += stringfunc::number_to_string(graph_hierarchy_ID);
      SQL_command += " AND node_ID=";
      SQL_command += stringfunc::number_to_string(node_ID);
      SQL_command += ";";
      
//      cout << SQL_command << endl;
      return SQL_command;
   }

// ---------------------------------------------------------------------
   bool update_node_label_in_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,int node_ID,
      string label)
   {
      vector<string> update_commands;
      update_commands.push_back(
         generate_update_node_label_SQL_command(graph_hierarchy_ID,
         node_ID,label));

      gis_database_ptr->set_SQL_commands(update_commands);
      bool exec_flag=gis_database_ptr->execute_SQL_commands();
      exec_flag=true;
      return exec_flag;
   }

// ---------------------------------------------------------------------
// As of 7/27/11, method retrieve_node_label_from_database() is
// deprecated.  It is very inefficient to pull captions from
// *gis_database_ptr one at a time.  Instead, we use method
// retrieve_node_labels_from_database() which retrieves an entire
// graph's worth of captions with a single SQL call.

   bool retrieve_node_label_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,int node_ID,
      string& label)
   {
//      cout << "inside graphdbfunc::retrieve_node_label_from_database()"
//           << endl;
      string SQL_command="SELECT label from nodes ";
      SQL_command += "WHERE graph_hierarchy_ID="+stringfunc::number_to_string(
         graph_hierarchy_ID);
      SQL_command += " AND node_ID="+stringfunc::number_to_string(node_ID)+";";

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(SQL_command);
      if (field_array_ptr==NULL) return false;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      label=field_array_ptr->get(0,0);
      return true;
   }

// ---------------------------------------------------------------------
// Method retrieve_node_labels_from_database() takes in the ID for
// some particular graph.  It returns an STL map containing the
// graph's captions versus its nodes IDs.

   NODE_IDS_LABELS_MAP* retrieve_node_labels_from_database(
      gis_database* gis_database_ptr,int graph_ID)
   {
//      cout << "inside graphdbfunc::retrieve_node_labels_from_database()"
//           << endl;
      string SQL_command="SELECT node_id,label from nodes ";
      SQL_command += "WHERE graph_ID="+stringfunc::number_to_string(
         graph_ID)+";";

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(SQL_command);
      if (field_array_ptr==NULL) return NULL;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      NODE_IDS_LABELS_MAP* node_ids_labels_map_ptr=
         new NODE_IDS_LABELS_MAP;

      for (unsigned int m=0; m<field_array_ptr->get_mdim(); m++)
      {
         int curr_node_ID=stringfunc::string_to_number(
            field_array_ptr->get(m,0));
         string curr_label=field_array_ptr->get(m,1);
         (*node_ids_labels_map_ptr)[curr_node_ID]=curr_label;
      }

//      cout << "node_ids_labels_map_ptr->size() = "
//           << node_ids_labels_map_ptr->size() << endl;

      return node_ids_labels_map_ptr;
   }
   
// ==========================================================================
// Connected graph component methods
// ==========================================================================

// Method insert_connected_component() 

   bool insert_connected_component(
      gis_database* gis_database_ptr,int hierarchy_ID,int graph_ID,int level,
      int connected_component_ID,int n_nodes,int n_links)
   {
      string SQL_cmd=generate_insert_connected_component_SQL_command(
         hierarchy_ID,graph_ID,level,connected_component_ID,n_nodes,n_links);
    
      vector<string> insert_commands;
      insert_commands.push_back(SQL_cmd);
      gis_database_ptr->set_SQL_commands(insert_commands);
      return gis_database_ptr->execute_SQL_commands();
   }

// ---------------------------------------------------------------------
// Method generate_insert_connected_component_SQL_command() takes in
// metadata associated with some connected component of some
// hierarchy.  It generates and returns a string containing a SQL
// insert command needed to populate a row within the
// connected_components table of the IMAGERY database.

   string generate_insert_connected_component_SQL_command(
      int graph_hierarchy_ID,int graph_ID,int level,int connected_component_ID,
      int n_nodes,int n_links)
   {
//      cout << "inside graphdbfunc::generate_insert_connected_component_SQL_command()" 
//           << endl;

      string SQL_command="insert into connected_components ";
      SQL_command += "(graph_hierarchy_ID,graph_ID,level,connected_component_ID,";
      SQL_command += "n_nodes,n_links) ";
      SQL_command += "values( ";
      SQL_command += stringfunc::number_to_string(graph_hierarchy_ID)+",";
      SQL_command += stringfunc::number_to_string(graph_ID)+",";
      SQL_command += stringfunc::number_to_string(level)+",";
      SQL_command += stringfunc::number_to_string(connected_component_ID)+",";
      SQL_command += stringfunc::number_to_string(n_nodes)+",";
      SQL_command += stringfunc::number_to_string(n_links)+");";

//      cout << SQL_command << endl;
      return SQL_command;
   }

// ---------------------------------------------------------------------
   string generate_update_connected_component_SQL_command(
      int hierarchy_ID,int graph_ID,int connected_component_ID,
      int n_nodes,int n_links)
   {
//      cout << "inside graphdbfunc::generate_update_connected_component_SQL_command()" 
//           << endl;

      string SQL_cmd="UPDATE connected_components ";
      SQL_cmd += "SET n_nodes="+stringfunc::number_to_string(n_nodes)+",";
      SQL_cmd += "n_links="+stringfunc::number_to_string(n_links);
      SQL_cmd += " WHERE graph_hierarchy_ID="+stringfunc::number_to_string(
         hierarchy_ID);
      SQL_cmd += " AND graph_ID="+stringfunc::number_to_string(graph_ID);
      SQL_cmd += " AND connected_component_ID="+
         stringfunc::number_to_string(connected_component_ID);

      return SQL_cmd;
   }

// ---------------------------------------------------------------------
// Method insert_connected_component() 

   bool insert_connected_component(
      gis_database* gis_database_ptr,int hierarchy_ID,int graph_ID,int level,
      int connected_component_ID,int cc_row,int cc_column,string cc_label)
   {
      string SQL_command="insert into connected_components ";
      SQL_command += "(graph_hierarchy_ID,graph_ID,level,connected_component_ID,";
      SQL_command += "cc_row,cc_column,label) ";
      SQL_command += "values( ";
      SQL_command += stringfunc::number_to_string(hierarchy_ID)+",";
      SQL_command += stringfunc::number_to_string(graph_ID)+",";
      SQL_command += stringfunc::number_to_string(level)+",";
      SQL_command += stringfunc::number_to_string(connected_component_ID)+",";
      SQL_command += stringfunc::number_to_string(cc_row)+",";
      SQL_command += stringfunc::number_to_string(cc_column)+",";
      SQL_command += "'"+cc_label+"') ;";
    
      vector<string> insert_commands;
      insert_commands.push_back(SQL_command);
      gis_database_ptr->set_SQL_commands(insert_commands);
      return gis_database_ptr->execute_SQL_commands();
   }

// ---------------------------------------------------------------------
   int get_n_connected_components(
      gis_database* gis_database_ptr,int hierarchy_ID)
   {
      string select_command=
         "SELECT n_connected_components FROM graph_hierarchies ";
      select_command += " WHERE graph_hierarchy_id="+
         stringfunc::number_to_string(hierarchy_ID);
//      cout << "select_command = " << select_command << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(select_command);
      if (field_array_ptr==NULL) return 0;

      int n_connected_components=stringfunc::string_to_number(
         field_array_ptr->get(0,0));
      return n_connected_components;
   }

// ---------------------------------------------------------------------
// Method retrieve_connected_component_info_from_database()

   bool retrieve_connected_component_info_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,
      vector<int>& graph_IDs,vector<int>& levels,
      vector<int>& connected_component_IDs,vector<int>& n_nodes)
   {
      cout << "inside graphdbfunc::retrieve_connected_component_info_from_database()" << endl;
      cout << "graph_hierarchy_ID = " << graph_hierarchy_ID << endl;

      string SQL_cmd="SELECT graph_id,level,connected_component_id,n_nodes ";
      SQL_cmd += "FROM connected_components ";
      SQL_cmd += "WHERE graph_hierarchy_ID="+stringfunc::number_to_string(
         graph_hierarchy_ID);
      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);

      if (field_array_ptr==NULL) return false;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      for (unsigned int m=0; m<field_array_ptr->get_mdim(); m++)
      {
         int graph_ID=stringfunc::string_to_number(field_array_ptr->get(m,0));
         int level=stringfunc::string_to_number(field_array_ptr->get(m,1));
         int connected_component_ID=
            stringfunc::string_to_number(field_array_ptr->get(m,2));
         int curr_n_nodes=stringfunc::string_to_number(
            field_array_ptr->get(m,3));

         graph_IDs.push_back(graph_ID);
         levels.push_back(level);
         connected_component_IDs.push_back(connected_component_ID);
         n_nodes.push_back(curr_n_nodes);
      } // loop over index m 

      return true;
   }

// ---------------------------------------------------------------------
// Method get_n_nodes_for_connected_component()

   int get_n_nodes_for_connected_component(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,
      int graph_ID,int connected_component_ID)
   {
//      cout << "inside graphdbfunc::get_n_nodes_for_connected_component()" 
//           << endl;
//      cout << "graph_hierarchy_ID = " << graph_hierarchy_ID << endl;

      string SQL_cmd="SELECT n_nodes FROM connected_components ";
      SQL_cmd += "WHERE graph_hierarchy_ID="+stringfunc::number_to_string(
         graph_hierarchy_ID);
      SQL_cmd += " AND graph_ID="+stringfunc::number_to_string(
         graph_ID);
      SQL_cmd += " AND connected_component_ID="+stringfunc::number_to_string(
         connected_component_ID);

      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);
      if (field_array_ptr==NULL) return 0;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      int n_nodes=stringfunc::string_to_number(field_array_ptr->get(0,0));
      return n_nodes;
   }

// ---------------------------------------------------------------------
// Method retrieve_connected_component_nodes_from_database()

   bool retrieve_connected_component_nodes_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,
      int graph_ID,int connected_component_ID,vector<int>& node_IDs)
   {
      cout << "inside graphdbfunc::retrieve_connected_component_nodes_from_database()" << endl;
      cout << "graph_hierarchy_ID = " << graph_hierarchy_ID << endl;

      string SQL_cmd="SELECT node_ID FROM nodes ";
      SQL_cmd += "WHERE graph_hierarchy_ID="+stringfunc::number_to_string(
         graph_hierarchy_ID);
      SQL_cmd += " AND graph_ID="+stringfunc::number_to_string(
         graph_ID);
      SQL_cmd += " AND connected_component_ID="+stringfunc::number_to_string(
         connected_component_ID);

      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);

      if (field_array_ptr==NULL) return false;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      node_IDs.clear();
      for (unsigned int m=0; m<field_array_ptr->get_mdim(); m++)
      {
         int node_ID=stringfunc::string_to_number(field_array_ptr->get(m,0));
         node_IDs.push_back(node_ID);
      } // loop over index m 

      return true;
   }

// ---------------------------------------------------------------------
   int get_link_weight(
      gis_database* gis_database_ptr,int hierarchy_ID,int graph_ID,
      int source_node_ID,int target_node_ID)
   {
      string select_command=
         "SELECT weight FROM links ";
      select_command += " WHERE graph_hierarchy_id="+
         stringfunc::number_to_string(hierarchy_ID);
      select_command += " AND graph_ID="+
         stringfunc::number_to_string(graph_ID);
      select_command += " AND source_node_ID="+
         stringfunc::number_to_string(source_node_ID);
      select_command += " AND target_node_ID="+
         stringfunc::number_to_string(target_node_ID);

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(select_command);
      if (field_array_ptr==NULL) return 0;

      int links_weight=stringfunc::string_to_number(
         field_array_ptr->get(0,0));
      return links_weight;
   }

// ---------------------------------------------------------------------
   int get_connected_component(
      gis_database* gis_database_ptr,int hierarchy_ID,int node_ID)
   {
      string select_command=
         "SELECT connected_component_ID FROM nodes ";
      select_command += " WHERE graph_hierarchy_id="+
         stringfunc::number_to_string(hierarchy_ID);
      select_command += " AND node_id = "+
         stringfunc::number_to_string(node_ID);
//      cout << "select_command = " << select_command << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(select_command);
      if (field_array_ptr==NULL) return 0;

      int cc_ID=stringfunc::string_to_number(field_array_ptr->get(0,0));
      return cc_ID;
   }

// ---------------------------------------------------------------------
// Method retrieve_node_connected_component_IDs_from_database()

   bool retrieve_node_connected_component_IDs_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,
      int graph_ID,vector<int>& node_IDs,vector<int>& cc_IDs)
   {
      cout << "inside graphdbfunc::retrieve_node_connected_component_IDs_from_database()" << endl;
      cout << "graph_hierarchy_ID = " << graph_hierarchy_ID << endl;

      string SQL_cmd="SELECT node_ID,connected_component_ID FROM nodes ";
      SQL_cmd += "WHERE graph_hierarchy_ID="+stringfunc::number_to_string(
         graph_hierarchy_ID);
      SQL_cmd += " AND graph_ID="+stringfunc::number_to_string(
         graph_ID);

      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);

      if (field_array_ptr==NULL) return false;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      unsigned int mdim=field_array_ptr->get_mdim();
      node_IDs.reserve(mdim);
      cc_IDs.reserve(mdim);
      node_IDs.clear();
      cc_IDs.clear();
      for (unsigned int m=0; m<field_array_ptr->get_mdim(); m++)
      {
         int node_ID=stringfunc::string_to_number(field_array_ptr->get(m,0));
         int cc_ID=stringfunc::string_to_number(field_array_ptr->get(m,1));
         node_IDs.push_back(node_ID);
         cc_IDs.push_back(cc_ID);

      } // loop over index m 

      return true;
   }

// ---------------------------------------------------------------------
// Method update_connected_component_info()

   bool update_connected_component_info(
      gis_database* gis_database_ptr,int hierarchy_ID,int graph_ID,
      int connected_component_ID,int cc_row,int cc_column,string label)
   {
      string SQL_cmd="UPDATE connected_components ";
      SQL_cmd += "SET cc_row="+stringfunc::number_to_string(cc_row)+",";
      SQL_cmd += "cc_column="+stringfunc::number_to_string(cc_column)+",";
      SQL_cmd += "label='"+label+"'";
      SQL_cmd += " WHERE graph_hierarchy_ID="+stringfunc::number_to_string(
         hierarchy_ID);
      SQL_cmd += " AND graph_ID="+stringfunc::number_to_string(graph_ID);
      SQL_cmd += " AND connected_component_ID="+
         stringfunc::number_to_string(connected_component_ID);
    
      vector<string> insert_commands;
      insert_commands.push_back(SQL_cmd);
      gis_database_ptr->set_SQL_commands(insert_commands);
      return gis_database_ptr->execute_SQL_commands();
   }

// ---------------------------------------------------------------------
// Method retrieve_connected_component_posn_labels_from_database()

   bool retrieve_connected_component_posn_labels_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,
      int graph_ID,int connected_component_ID,
      double& cc_row,double& cc_column,string& cc_label,
      vector<string>& topic_labels)
   {
      cout << "inside graphdbfunc::retrieve_connected_component_posn_labels_from_database()" << endl;
      
      string SQL_cmd=
         "SELECT cc_row,cc_column,label,topic_0,topic_1,topic_2,topic_3,topic_4 ";
      SQL_cmd += "FROM connected_components ";
      SQL_cmd += "WHERE graph_hierarchy_ID="+stringfunc::number_to_string(
         graph_hierarchy_ID);
      SQL_cmd += " AND graph_ID="+stringfunc::number_to_string(
         graph_ID);
      SQL_cmd += " AND connected_component_ID="+stringfunc::number_to_string(
         connected_component_ID);
      cout << "SQL_cmd = " << SQL_cmd << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);
//      cout << "field_array_ptr = " << field_array_ptr << endl;
      
      if (field_array_ptr==NULL) return false;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      cc_row=stringfunc::string_to_number(field_array_ptr->get(0,0));
      cc_column=stringfunc::string_to_number(field_array_ptr->get(0,1));

      if (field_array_ptr->get(0,2) != "NULL")
         cc_label=field_array_ptr->get(0,2);

      topic_labels.clear();
      for (unsigned int c=3; c<8; c++)
      {
         string curr_topic=field_array_ptr->get(0,c);
         if (curr_topic != "NULL") topic_labels.push_back(curr_topic);
      }

      return true;
   }

// ---------------------------------------------------------------------
// Method update_connected_component_topic()

   bool update_connected_component_topic(
      gis_database* gis_database_ptr,int hierarchy_ID,int graph_ID,
      int connected_component_ID,int topic_ID,string topic)
   {
      if (topic_ID < 0 || topic_ID > 4) return false;
      
      string SQL_cmd="UPDATE connected_components ";
      SQL_cmd += "SET topic_"+stringfunc::number_to_string(topic_ID)+"='"
         +topic+"'";
      SQL_cmd += " WHERE graph_hierarchy_ID="+stringfunc::number_to_string(
         hierarchy_ID);
      SQL_cmd += " AND graph_ID="+stringfunc::number_to_string(graph_ID);
      SQL_cmd += " AND connected_component_ID="+
         stringfunc::number_to_string(connected_component_ID);
    
      vector<string> insert_commands;
      insert_commands.push_back(SQL_cmd);
      gis_database_ptr->set_SQL_commands(insert_commands);
      return gis_database_ptr->execute_SQL_commands();
   }

// ==========================================================================
// Graph annotation methods
// ==========================================================================

// Method retrieve_graph_annotations_from_database()

   bool retrieve_graph_annotations_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,int graph_ID,
      vector<int>& layouts,vector<double>& gxs,vector<double>& gys,
      vector<string>& labels,vector<string>& colors,vector<double>& sizes)
   {
//      cout << "inside graphdbfunc::retrieve_graph_annotations_from_database()"
//           << endl;
//      cout << "graph_hierarchy_ID = " << graph_hierarchy_ID << endl;

      string SQL_cmd="SELECT layout,gx,gy,label,color,size ";
      SQL_cmd += "FROM graph_annotations ";
      SQL_cmd += "WHERE graph_hierarchy_ID="+stringfunc::number_to_string(
         graph_hierarchy_ID);
      SQL_cmd += " AND graph_ID="+stringfunc::number_to_string(graph_ID);
      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);

      if (field_array_ptr==NULL) return false;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      for (unsigned int m=0; m<field_array_ptr->get_mdim(); m++)
      {
         int layout=stringfunc::string_to_number(field_array_ptr->get(m,0));
         double gx=stringfunc::string_to_number(field_array_ptr->get(m,1));
         double gy=stringfunc::string_to_number(field_array_ptr->get(m,2));
         string label=field_array_ptr->get(m,3);
         string color=field_array_ptr->get(m,4);
         double size=stringfunc::string_to_number(field_array_ptr->get(m,5));

         layouts.push_back(layout);
         gxs.push_back(gx);
         gys.push_back(gy);
         labels.push_back(label);
         colors.push_back(color);
         sizes.push_back(size);
      } // loop over index m 

      return true;
   }

// ---------------------------------------------------------------------
// Method generate_insert_graph_annotation_SQL_command() 

   string generate_insert_graph_annotation_SQL_command(
      int hierarchy_ID,int graph_ID,int level,int layout,double gx,double gy,
      string label,string color,double annotation_size)
   {
//      cout << "inside graphdbfunc::generate_insert_graph_annotation_SQL_command()" << endl;

      string SQL_command="insert into graph_annotations ";
      SQL_command += "(graph_hierarchy_ID,graph_ID,level,layout,gx,gy,";
      SQL_command += "label,color,size) ";
      SQL_command += "values( ";
      SQL_command += stringfunc::number_to_string(hierarchy_ID)+",";
      SQL_command += stringfunc::number_to_string(graph_ID)+",";
      SQL_command += stringfunc::number_to_string(level)+",";
      SQL_command += stringfunc::number_to_string(layout)+",";
      SQL_command += stringfunc::number_to_string(gx)+",";
      SQL_command += stringfunc::number_to_string(gy)+",";
      SQL_command += "'"+label+"',";
      SQL_command += "'"+color+"',";
      SQL_command += stringfunc::number_to_string(annotation_size)+");";

//      cout << SQL_command << endl;
      return SQL_command;
   }

// ==========================================================================
// Graph geometries methods
// ==========================================================================

// Method retrieve_graph_geometries_from_database()

   bool retrieve_graph_geometries_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,
      vector<string>& geom_labels,vector<string>& geom_colors,
      vector<string>& the_geoms)
   {
//      cout << "inside graphdbfunc::retrieve_graph_geometries_from_database()"
//           << endl;
//      cout << "graph_hierarchy_ID = " << graph_hierarchy_ID << endl;

      string SQL_cmd="SELECT geometry_label,ST_AsText(the_geom),color ";
      SQL_cmd += "FROM graph_geometries ";
      SQL_cmd += "WHERE graph_hierarchy_ID="+stringfunc::number_to_string(
         graph_hierarchy_ID);
      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);

      if (field_array_ptr==NULL) return false;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      for (unsigned int m=0; m<field_array_ptr->get_mdim(); m++)
      {
         string geom_label=field_array_ptr->get(m,0);
         string the_geom=field_array_ptr->get(m,1);
         string geom_color=field_array_ptr->get(m,2);

         geom_labels.push_back(geom_label);
         the_geoms.push_back(the_geom);
         geom_colors.push_back(geom_color);
      } // loop over index m 

      return true;
   }

// ---------------------------------------------------------------------
// Method retrieve_graph_polygons_from_database()

   bool retrieve_graph_polygons_from_database(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,
      vector<string>& polygon_labels,vector<fourvector>& polygon_RGBA,
      vector< vector<twovector> >& polygon_vertices)
   {
//      cout << "inside graphdbfunc::retrieve_graph_polygons_from_database()"
//           << endl;

      vector<string> geom_colors,the_geoms;
      graphdbfunc::retrieve_graph_geometries_from_database(
         gis_database_ptr,graph_hierarchy_ID,polygon_labels,
         geom_colors,the_geoms);

      bool polygons_retrieved_flag=false;
      for (unsigned int i=0; i<the_geoms.size(); i++)
      {
         string separator_chars="(),";
         vector<string> substrings=
            stringfunc::decompose_string_into_substrings(
               the_geoms[i],separator_chars);
         
         string geom_type=stringfunc::capitalize_word(substrings[0]);
         if (geom_type=="POLYGON")
         {
            double r,g,b,a;
            colorfunc::RRGGBBAA_hex_to_rgba(geom_colors[i],r,g,b,a);
            fourvector curr_RGBA(r,g,b,a);
//            cout << "curr_RGBA = " << curr_RGBA << endl;
            polygon_RGBA.push_back(curr_RGBA);

            vector<twovector> curr_polygon_vertices;
            for (unsigned int s=1; s<substrings.size(); s++)
            {
               vector<double> column_values=stringfunc::string_to_numbers(
                  substrings[s]);
               curr_polygon_vertices.push_back(twovector(
                  column_values[0],column_values[1]));
//               cout << "s = " << s 
//                    << " polygon vertex = " << curr_polygon_vertices.back() 
//                    << endl;
            } // loop over index s labeling substrings
            polygon_vertices.push_back(curr_polygon_vertices);
            polygons_retrieved_flag=true;
         } // geom_type conditional
      }
      return polygons_retrieved_flag;
   }
   
} // graphdbfunc namespace







