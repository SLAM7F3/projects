// ==========================================================================
// GRAPHFUNCS stand-alone methods
// ==========================================================================
// Last modified on 1/31/11; 2/23/12; 8/27/12; 4/5/14
// ==========================================================================

#include <iomanip>
#include <iostream>
#include <map>
#include <math.h>
#include <vector>
#include "general/filefuncs.h"
#include "graphs/graph.h"
#include "graphs/graph_edge.h"
#include "graphs/graphfuncs.h"
#include "graphs/graph_hierarchy.h"
#include "graphs/node.h"
#include "postgres/gis_database.h"
#include "general/stringfuncs.h"


using std::cout;
using std::cin;
using std::endl;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

namespace graphfunc
{

   void initialize_findunion_structures(unsigned int n,int id[],int size[])
   {
      for (unsigned int i=0; i<n; i++)
      {
         id[i]=i;
         size[i]=1;
      }
   }
   
// ---------------------------------------------------------------------
// Methods quickfind, quickunion, weightedunion and halveunion come
// from chapter 1 of "Algorithms in C", 3rd edition by Robert
// Sedgewick (QA 76.73.C15 S43 1998).

// Method quickfind takes in an input pair (p,q) which is interpreted
// to mean "connect object p to object q".  It maintains an array id[]
// that has an entry from each object, with the property that
// id[p]=id[q] iff p and q are connected.  

   void quickfind(int p,int q,unsigned int n,int id[])
   {
      if (id[p] != id[q])
      {
         int t=id[p];
         for (unsigned int i=0; i<n; i++)
         {
            if (id[i]==t) id[i]=id[q];
         }
      }
   }

// Method quickunion performs the same operation as quickfind.  But it
// does less computation for the union operation at the expense of
// more computation for the find operation.

   void quickunion(int p,int q,int id[])
   {
      int i,j;
      for (i=p; i != id[i]; i=id[i]);
      for (j=q; j != id[j]; j=id[j]);

// Next line implements the union operation:

      if (i != j) id[i]=j;
   }

// Method weightedunion is a modification of quickunion.  It keeps an
// additional array size for the purpose of maintaining, for each
// object with id[i]==i, the number of nodes in the associated tree.
// The union operation can link the smaller  of the 2 specified trees
// to the larger.  It thereby prevents the growth of long paths in the
// trees.

   void weightedunion(int p,int q,int n,int id[],int size[])
   {
      int i,j;
      for (i=p; i != id[i]; i=id[i]);
      for (j=q; j != id[j]; j=id[j]);
      if (i != j) 
      {
         if (size[i] < size[j])
         {
               
            id[i]=j;
            size[j] += size[i];
         }
         else
         {
            id[j]=i;
            size[i] += size[j];
         }
      }
   }
   
// Method halveunion replaces the for loops in weightedunion and
// thereby halves the length of any path that we traverse.  The net
// result of this change is that the trees become almost completely
// flat after a long sequence of operations.  

   void halveunion(int p,int q,int n,int id[],int size[])
   {
      int i,j;
      for (i=p; i != id[i]; i=id[i])
      {
         id[i]=id[id[i]];
      }
      for (j=q; j != id[j]; j=id[j])
      {
         id[j]=id[id[j]];
      }
         
      if (i != j) 
      {
         if (size[i] < size[j])
         {
               
            id[i]=j;
            size[j] += size[i];
         }
         else
         {
            id[j]=i;
            size[i] += size[j];
         }
      }
   }

// ---------------------------------------------------------------------
// Method tree_root takes in "leaf" integer p which belongs to a tree
// containing n leaves.  It returns the root leaf for this particular
// input integer.  If two leaves have the same root leaf, they are
// connected.

   int tree_root(int p,int n,int id[])
   {
      while (true)
      {
         if (p==id[p])
         {
            return p;
         }
         else
         {
            p=id[p];
         }
      }
   }

// ---------------------------------------------------------------------
// Boolean method equivalent_leaves returns true if two leaves' root
// are equal.

   bool equivalent_leaves(int p,int q,int n,int id[])
   {
      return (tree_root(p,n,id)==tree_root(q,n,id));
   }

// ==========================================================================
// Graph generation methods
// ==========================================================================

// Method max_node_ID_from_edgelist() takes in an edgelist assumed to
// come from Noah Snavely's bundler codes.  It performs a brute force
// scan of the edge list and returns the maximum node ID value.

   int max_node_ID_from_edgelist(string edgelist_filename)
   {
      cout << "inside graphfunc::max_node_ID_from_edgelist()" << endl;
      cout << "edgelist_filename = " << edgelist_filename << endl;

      int max_node_ID=-1;
      filefunc::ReadInfile(edgelist_filename);
      for (unsigned int i=0; i<filefunc::text_line.size(); i++)
      {
         vector<double> curr_row_entries=
            stringfunc::string_to_numbers(filefunc::text_line[i]);
         int node_ID_2=basic_math::round(curr_row_entries[1]);
         max_node_ID=basic_math::max(max_node_ID,node_ID_2);
      } // loop over index i labeling lines in edgelist_filename
      cout << endl;

      cout << "max_node_ID = " << max_node_ID << endl;
      return max_node_ID;
   }

// ---------------------------------------------------------------------
// Method generate_graph_from_edgelist() takes in an edgelist assumed
// to come from Noah Snavely's bundler codes.  It instantiates a new
// graph and fills its nodes and edges based upon the entries within
// the input file.

   graph* generate_graph_from_edgelist(
      string edgelist_filename,int graph_ID,int level,int zeroth_datum_ID)
   {
      return generate_graph_from_edgelist(
         edgelist_filename,graph_ID,level,zeroth_datum_ID,NULL);
   }

   graph* generate_graph_from_edgelist(
      string edgelist_filename,int graph_ID,int level,
      int zeroth_datum_ID,int* graph_edge_counter_ptr)
   {
//         cout << "inside graphfunc::generate_graph_from_edgelist()" << endl;
//         cout << "edgelist_filename = " << edgelist_filename << endl;
//         cout << "graph_ID = " << graph_ID << " level = " << level << endl;

      graph* graph_ptr=new graph(graph_ID,level);
      if (graph_edge_counter_ptr != NULL)
      {
         graph_ptr->set_graph_edge_counter_ptr(graph_edge_counter_ptr);
      }

      filefunc::ReadInfile(edgelist_filename);
      for (unsigned int i=0; i<filefunc::text_line.size(); i++)
      {
//            if (i%10000==0) cout << i << " " << flush;

         vector<double> curr_row_entries=
            stringfunc::string_to_numbers(filefunc::text_line[i]);
         int node_ID_1=basic_math::round(curr_row_entries[0]);
         int node_ID_2=basic_math::round(curr_row_entries[1]);
         double weight=curr_row_entries[2];

         if (!graph_ptr->node_in_graph(node_ID_1))
         {
            node* node_ptr=new node(node_ID_1,level);
            node_ptr->set_data_ID(node_ptr->get_data_ID()+
                                  zeroth_datum_ID);
            graph_ptr->add_node(node_ptr);
         }

         if (!graph_ptr->node_in_graph(node_ID_2))
         {
            node* node_ptr=new node(node_ID_2,level);
            node_ptr->set_data_ID(node_ptr->get_data_ID()+
                                  zeroth_datum_ID);
            graph_ptr->add_node(node_ptr);
         }

         if (node_ID_1 != node_ID_2 && weight >= 0)
         {
            graph_ptr->add_graph_edge(node_ID_1,node_ID_2,weight);
         }

//      cout << "node_ID1 = " << node_ID_1
//           << " node_ID2 = " << node_ID_2
//           << " weight = " << weight << endl;

      } // loop over index i labeling lines in edgelist_filename
      cout << endl;

//         cout << "graph_ptr->get_n_nodes() = " << graph_ptr->get_n_nodes() 
//              << endl;
//         cout << "graph_ptr->get_max_node_ID() = "
//              << graph_ptr->get_max_node_ID() << endl;

      graph_ptr->reorder_nonnull_ptr_nodes();

      return graph_ptr;
   }

// ---------------------------------------------------------------------
// Method generate_node_from_database_info() 

   node* generate_node_from_database_info(
      int node_ID,unsigned int node_level,int data_ID,int parent_ID,
      double node_relative_size,colorfunc::RGB node_RGB,
      const twovector& gxgy)
   {
//         cout << "inside graphfunc::generate_node_from_database_info()" 
//              << endl;

      node* node_ptr=new node(node_ID,node_level);
      node_ptr->set_data_ID(data_ID);
      node_ptr->set_parent_ID(parent_ID);
      node_ptr->set_relative_size(node_relative_size);
      node_ptr->set_node_RGB(node_RGB);
      node_ptr->set_posn(gxgy);
      return node_ptr;
   }

// ---------------------------------------------------------------------
// Method generate_graph_hierarchy_from_database() takes in
// *postgis_database_ptr which is assumed to contain a "data_network"
// database with "node" and "link" tables.  It extracts all rows and
// columns from these database tables.  This method dynamically
// generates a graph_hierarchy and populates it with graphs
// reconstructed from the database information.

   graph_hierarchy* generate_graph_hierarchy_from_database(
      gis_database* gis_database_ptr)
   {

// First read contents of node table from photo database:

      string curr_select_command = 
         "SELECT id,level,data_id,parent_id,relative_size,rgb_color, ";
      curr_select_command += 
         "x(gxgy_posn) as gx,y(gxgy_posn) as gy from node";
      Genarray<string>* field_array_ptr=
         gis_database_ptr->select_data(curr_select_command);

      colorfunc::RGB node_RGB;   
      vector<unsigned int> node_level;
      vector<int> node_ID,data_ID,parent_ID;
      vector<double> node_relative_size;
      vector<colorfunc::RGB> node_RGB_coords;
      vector<twovector> gxgy;

      for (unsigned int i=0; i<field_array_ptr->get_mdim(); i++)
      {
         int curr_ID=
            stringfunc::string_to_number(field_array_ptr->get(i,0));
         int curr_level=
            stringfunc::string_to_number(field_array_ptr->get(i,1));
         int curr_data_ID=
            stringfunc::string_to_number(field_array_ptr->get(i,2));   
         int curr_parent_ID=
            stringfunc::string_to_number(field_array_ptr->get(i,3));
         double curr_relative_size=stringfunc::string_to_number(
            field_array_ptr->get(i,4));

         string RGB_str=field_array_ptr->get(i,5);
         vector<double> RGB_values=
            stringfunc::string_to_numbers(RGB_str,",");
         node_RGB.first=RGB_values[0]/255.0;
         node_RGB.second=RGB_values[1]/255.0;
         node_RGB.third=RGB_values[2]/255.0;

         double curr_gx=stringfunc::string_to_number(
            field_array_ptr->get(i,6));
         double curr_gy=stringfunc::string_to_number(
            field_array_ptr->get(i,7));

         node_ID.push_back(curr_ID);
         node_level.push_back(curr_level);
         data_ID.push_back(curr_data_ID);
         parent_ID.push_back(curr_parent_ID);
         node_relative_size.push_back(curr_relative_size);
         node_RGB_coords.push_back(node_RGB);
         gxgy.push_back(twovector(curr_gx,curr_gy));

/*
  cout << "ID = " << node_ID.back()
  << " level = " << node_level.back()
  << " data_ID = " << data_ID.back()
  << " rel_size = " << node_relative_size.back() << endl;
  cout << "  R = " << node_RGB.first 
  << " G = " << node_RGB.second
  << " B = " << node_RGB.third 
  << " gx = " << gxgy.back().get(0)
  << " gy = " << gxgy.back().get(1) << endl;
*/

      } // loop over index i labeling rows in *field_array_ptr

// Next read contents of link table from photo database:

      curr_select_command = 
         "SELECT id,level,source_id,target_id,weight,relative_size,rgb_color ";
      curr_select_command += "from link";
      field_array_ptr=gis_database_ptr->select_data(curr_select_command);

      colorfunc::RGB edge_RGB;   
      vector<unsigned int> edge_level;
      vector<int> edge_ID,source_ID,target_ID;
      vector<double> weight,edge_relative_size;
      vector<colorfunc::RGB> edge_RGB_coords;

      for (unsigned int i=0; i<field_array_ptr->get_mdim(); i++)
      {
         int curr_edge_ID=
            stringfunc::string_to_number(field_array_ptr->get(i,0));
         unsigned int curr_edge_level=
            stringfunc::string_to_number(field_array_ptr->get(i,1));
         int curr_source_ID=
            stringfunc::string_to_number(field_array_ptr->get(i,2));   
         int curr_target_ID=
            stringfunc::string_to_number(field_array_ptr->get(i,3));
         double curr_weight=stringfunc::string_to_number(
            field_array_ptr->get(i,4));
         double curr_relative_size=stringfunc::string_to_number(
            field_array_ptr->get(i,5));

         string RGB_str=field_array_ptr->get(i,6);
         vector<double> RGB_values=stringfunc::string_to_numbers(
            RGB_str,",");
         edge_RGB.first=RGB_values[0]/255.0;
         edge_RGB.second=RGB_values[1]/255.0;
         edge_RGB.third=RGB_values[2]/255.0;

         edge_ID.push_back(curr_edge_ID);
         edge_level.push_back(curr_edge_level);
         source_ID.push_back(curr_source_ID);
         target_ID.push_back(curr_target_ID);
         weight.push_back(curr_weight);
         edge_relative_size.push_back(curr_relative_size);
         edge_RGB_coords.push_back(edge_RGB);

/*
  if (edge_ID.back() %1000 != 0) continue;
  cout << "Edge ID = " << edge_ID.back()
  << " level = " << edge_level.back()
  << " source_ID = " << source_ID.back()
  << " target_ID = " << target_ID.back()
  << " weight = " << weight.back()
  << " rel_size = " << edge_relative_size.back() << endl;
  cout << "  R = " << edge_RGB.first 
  << " G = " << edge_RGB.second
  << " B = " << edge_RGB.third << endl;
*/
            
      } // loop over index i labeling rows in *field_array_ptr

// Construct graph hierarchy from node and edge information retrieved
// from photo database:

      graph_hierarchy* graphs_pyramid_ptr=new graph_hierarchy;
      unsigned int n_levels=1;
   
      for (unsigned int i=0; i<node_level.size(); i++)
      {
         n_levels = basic_math::max(n_levels,node_level[i]+1);
      }
      cout << "n_levels = " << n_levels << endl;

      int graph_ID=0;
      for (unsigned int l=0; l<n_levels; l++)
      {
         graph* graph_ptr=new graph(graph_ID,l);
         graphs_pyramid_ptr->add_graph_ptr(graph_ptr);
      } // loop over index l labeling graph levels
   
      cout << "Reconstructing graph nodes from database information" 
           << endl;
      for (unsigned int i=0; i<node_ID.size(); i++)
      {
         node* node_ptr=graphfunc::generate_node_from_database_info(
            node_ID[i],node_level[i],data_ID[i],parent_ID[i],
            node_relative_size[i],node_RGB_coords[i],gxgy[i]);
      
         graph* graph_ptr=graphs_pyramid_ptr->get_graph_ptr(node_level[i]);
         if (!graph_ptr->node_in_graph(node_ID[i]))
         {
            graph_ptr->add_node(node_ptr);
         }
      } // loop over index i labeling graph nodes

      cout << "Reconstructing graph edges from database information" 
           << endl;
      for (unsigned int l=0; l<n_levels; l++)
      {
         graph* graph_ptr=graphs_pyramid_ptr->get_graph_ptr(l);
         for (unsigned int e=0; e<edge_ID.size(); e++)
         {
            if (edge_level[e] != l) continue;
         
            graph_edge* edge_ptr=
               graph_ptr->add_graph_edge(
                  source_ID[e],target_ID[e],weight[e]);
            edge_ptr->set_ID(edge_ID[e]);
            edge_ptr->set_relative_size(edge_relative_size[e]);
            edge_ptr->set_edge_RGB(edge_RGB_coords[e]);
         } // loop over index e labeling graph edges
         cout << "level = " << l
              << " n_nodes = " << graph_ptr->get_n_nodes() 
              << " n_edges = " << graph_ptr->get_n_graph_edges() << endl;
      } // loop over index l labeling graph levels
         
      return graphs_pyramid_ptr;

   }
      
// ==========================================================================
// MCL output manipulation methods
// ==========================================================================

// Method extract_hierarchy_cluster_sizes() takes in a "cone"
// clustering hierarchy file generated by MCLCM.  It scans this file
// for the "dimensions" keyword followed by the sizes of each
// hierarchical cluster.  This method returns an STL vector containing
// level values and cluster sizes for the MCLCM clusters which are
// sequentially separated by cluster_ratio and exceed
// min_cluster_size.

   vector<twovector> extract_hierarchy_cluster_sizes(
      string mclcm_cone_filename)
   {
      filefunc::ReadInfile(mclcm_cone_filename);

      vector<int> cluster_size;
      for (unsigned int i=0; i<filefunc::text_line.size(); i++)
      {
         vector<string> substrings=
            stringfunc::decompose_string_into_substrings(
               filefunc::text_line[i]);
         if (substrings[0]=="dimensions")
         {
            vector<string> subsubstrings=
               stringfunc::decompose_string_into_substrings(
                  substrings[1],"x");
            int curr_cluster_size=stringfunc::string_to_number(
               subsubstrings[0]);
            cluster_size.push_back(curr_cluster_size);
         }
      } // loop over index i labeling lines within input file

      vector<twovector> selected_mclcm_clusters;
      int prev_cluster_size=10*cluster_size[0];

      for (unsigned int j=0; j<cluster_size.size(); j++)
      {
         cout << "Cluster j = " << j << " size = " << cluster_size[j]
              << endl;

         const int min_cluster_size=10;
         const double cluster_ratio=4.5;
         if (double(prev_cluster_size)/double(cluster_size[j]) > 
             cluster_ratio && cluster_size[j] >= min_cluster_size)
         {
            selected_mclcm_clusters.push_back(
               twovector(j,cluster_size[j]));
            prev_cluster_size=cluster_size[j];
         }
      } // loop over index j labeling hierarchical MCLCM clusters


      for (unsigned int k=0; k<selected_mclcm_clusters.size(); k++)
      {
         twovector curr_selected_mclcm_clusters(
            selected_mclcm_clusters[k]);
         cout << "Final MCLCM cluster level = " 
              << curr_selected_mclcm_clusters.get(0)
              << " n_clusters = " 
              << curr_selected_mclcm_clusters.get(1) << endl;
      } // loop over index k 

      return selected_mclcm_clusters;
   }

// ---------------------------------------------------------------------
// Method parse_mcl_cone_file() parses the input "cone" graph
// hierarchy file generated by MCLCM.  It instantiates a graph object
// for each level within the hierarchy.  This method generates nodes
// within level l and stores the IDs of their children nodes within
// level l-1.

   void parse_mcl_cone_file(
      string mclcm_cone_filename,graph_hierarchy* graphs_pyramid_ptr,
      graph* edgelist_graph_ptr)
   {
//         cout << "inside parse_mcl_cone_file()" << endl;
      filefunc::ReadInfile(mclcm_cone_filename);

      bool cluster_start_flag=false;
//      bool graph_start_flag=false;
      int i_next_line=-1;
      int level=0;
      node* curr_node_ptr=NULL;
      graph* lowest_graph_ptr=NULL;
      graph* curr_graph_ptr=NULL;
      vector<int> children_node_IDs;

      for (unsigned int i=0; i<filefunc::text_line.size(); i++)
      {
         vector<string> substrings=
            stringfunc::decompose_string_into_substrings(
               filefunc::text_line[i]);
            
         if (cluster_start_flag && stringfunc::is_number(substrings[0]))
         {
            int curr_cluster_ID=stringfunc::string_to_number(
               substrings[0]);
//               cout << "level = " << level 
//                    << " curr_cluster_ID = " << curr_cluster_ID << endl;
            cluster_start_flag=false;
            curr_node_ptr=new node(curr_cluster_ID,level);
            curr_graph_ptr->add_node(curr_node_ptr);

            for (unsigned int j=1; j<substrings.size(); j++)
            {
               extract_nodes_from_mcl_cone_file(
                  level,substrings[j],children_node_IDs,
                  curr_node_ptr,lowest_graph_ptr,edgelist_graph_ptr);
            } // loop over index j
            i_next_line=i+1;
         }

         if (substrings[0]=="begin")
         {
            int ID=0;
            if (level==0)
            {
               lowest_graph_ptr=new graph(ID,level);
               graphs_pyramid_ptr->add_graph_ptr(lowest_graph_ptr);
            }
            level++;

            curr_graph_ptr=new graph(ID,level);
            graphs_pyramid_ptr->add_graph_ptr(curr_graph_ptr);
//            graph_start_flag=true;
            cluster_start_flag=true;
         }

         if (int(i)==i_next_line)
         {
            for (unsigned int j=0; j<substrings.size(); j++)
            {
               extract_nodes_from_mcl_cone_file(
                  level,substrings[j],children_node_IDs,
                  curr_node_ptr,lowest_graph_ptr,edgelist_graph_ptr);
            } // loop over j index
            i_next_line++;
         }

         if (substrings[substrings.size()-1]=="$")
         {
            cluster_start_flag=true;
            i_next_line=-1;
            curr_node_ptr->set_children_node_IDs(children_node_IDs);

            graph* child_graph_ptr=graphs_pyramid_ptr->
               get_graph_ptr(level-1);
            if (child_graph_ptr != NULL)
            {
               for (unsigned int k=0; k<children_node_IDs.size(); k++)
               {
                  int curr_child_node_ID=children_node_IDs[k];
//                     cout << "curr_child_node_ID = "
//                          << curr_child_node_ID << endl;
                  node* child_node_ptr=child_graph_ptr->get_node_ptr(
                     curr_child_node_ID);
//                     cout << "child_node_ptr = "
//                          << child_node_ptr << endl;
                  child_node_ptr->set_parent_ID(curr_node_ptr->get_ID());
               } // loop over index k
            } // child_graph_ptr != NULL conditional

            children_node_IDs.clear();
         } 

         if (substrings[0]==")")
         {
//            graph_start_flag=false;
         }

      } // loop over index i labeling lines within input file

      string outputfilename="graphs.out";
      ofstream outstream;
      filefunc::openfile(outputfilename,outstream);
      outstream << *graphs_pyramid_ptr << endl;
      filefunc::closefile(outputfilename,outstream);

      graphs_pyramid_ptr->compute_all_ancestors();

/*
  while(true)
  {
  int start_level,stop_level;
  cout << "Enter start level:" << endl;
  cin >> start_level;
  cout << "Enter stop level:" << endl;
  cin >> stop_level;
  graphs_pyramid_ptr->compute_descendants(start_level,stop_level);
  }
*/
   }
   
// ---------------------------------------------------------------------
// Method extract_nodes_from_mcl_cone_file() is a utility function
// which converts curr_substring to an integer and appends it onto
// children_node_IDs.  If input level==1, this method also
// instantiates a new node member of *lowest_graph_ptr and sets its
// parents ID to curr_node_ptr->get_ID().

   void extract_nodes_from_mcl_cone_file(
      int level,string curr_substring,vector<int>& children_node_IDs,
      node* curr_node_ptr,graph* lowest_graph_ptr,graph* edgelist_graph_ptr)
   {
//         cout << "inside graphfunc::extract_nodes_from_mcl_cone_file()"
//              << endl;
      if (!stringfunc::is_number(curr_substring)) 
      {
         return;
      }

      int curr_child_ID=stringfunc::string_to_number(curr_substring);
//         cout << "curr_child_ID = " << curr_child_ID << endl;
            
      if (level==1)
      {
         curr_child_ID=edgelist_graph_ptr->get_ordered_node_ptr(
            curr_child_ID)->get_ID();
//            cout << "Revised curr_child_ID = " << curr_child_ID << endl;
         node* lowest_node_ptr=new node(curr_child_ID,0);
         lowest_node_ptr->set_parent_ID(curr_node_ptr->get_ID());
         lowest_graph_ptr->add_node(lowest_node_ptr);
      }

      children_node_IDs.push_back(curr_child_ID);
   }

// ==========================================================================
// Clustering methods
// ==========================================================================

// Method compute_cluster_COM() takes in the ID for some cluster of
// nodes within input *cluster_graph_ptr.  After looping over all
// nodes within the specified cluster, this method computes and
// returns the cluster's COM.  If input take_medians_flag==false, this
// method returns the average of the cluster constituents' (U,V)
// coordinates weighted by their relative size.

   twovector compute_cluster_COM(
      graph* cluster_graph_ptr,graph* child_graph_ptr,
      int parent_ID,bool take_medians_flag)
   {
//   cout << "inside graph::compute_cluster_COM()" << endl;
//   cout << "parent_ID = " << parent_ID << endl;
   
      vector<int> clustered_node_IDs=
         cluster_graph_ptr->get_clusters_map().at(parent_ID);
      vector<double> gx,gy,gx_weighted,gy_weighted,denom;
      for (unsigned int i=0; i<clustered_node_IDs.size(); i++)
      {
         node* child_node_ptr=child_graph_ptr->get_node_ptr(
            clustered_node_IDs[i]);
         double relative_size=child_node_ptr->get_relative_size();
      
         gx.push_back(child_node_ptr->get_Uposn());
         gy.push_back(child_node_ptr->get_Vposn());
         gx_weighted.push_back(relative_size*child_node_ptr->get_Uposn());
         gy_weighted.push_back(relative_size*child_node_ptr->get_Vposn());
         denom.push_back(relative_size);
      } // loop over index i labeling nodes within current cluster

      double gx_avg,gy_avg;
      if (take_medians_flag)
      {
         gx_avg=mathfunc::median_value(gx);
         gy_avg=mathfunc::median_value(gy);
      }
      else
      {
         gx_avg=mathfunc::mean(gx_weighted)/mathfunc::mean(denom);
         gy_avg=mathfunc::mean(gy_weighted)/mathfunc::mean(denom);
      }
      twovector cluster_COM(gx_avg,gy_avg);
   
      return cluster_COM;
   }

// ---------------------------------------------------------------------
// Method median_cluster_radius(int cluster_radius) computes the
// radial distance of each constituent node from the center-of-mass of
// the input specified cluster.  This method then returns the median
// of all the constituent nodes' radial distances.

   double median_cluster_radius(
      graph* cluster_graph_ptr,graph* child_graph_ptr,int parent_ID)
   {
//   cout << "inside graphfunc::median_cluster_radius()" << endl;
      twovector cluster_COM=graphfunc::compute_cluster_COM(
         cluster_graph_ptr,child_graph_ptr,parent_ID);
      vector<int> clustered_node_IDs=
         cluster_graph_ptr->get_clusters_map().at(parent_ID);
 
// Compute radius of each node from its cluster_COM:

      vector<double> radii;
      for (unsigned int i=0; i<clustered_node_IDs.size(); i++)
      {
         node* child_node_ptr=child_graph_ptr->get_node_ptr(
            clustered_node_IDs[i]);
         twovector curr_posn(child_node_ptr->get_posn());
         twovector rel_curr_posn(curr_posn-cluster_COM);
         radii.push_back(rel_curr_posn.magnitude());
      } // loop over index i labeling nodes within current cluster

//      double mu_cluster_radius=mathfunc::mean(radii);
//      double sigma_cluster_radius=mathfunc::std_dev(radii);
      double median_cluster_radius=mathfunc::median_value(radii);

//   cout << "Mean radius for cluster = " << mu_cluster_radius 
//        << " +/- " << sigma_cluster_radius << endl;
//   cout << "Median radius for cluster= " << median_cluster_radius << endl;

      return median_cluster_radius;
   }

// ---------------------------------------------------------------------
// Method adjust_node_posns_from_cluster_COMs() loops over all
// clusters within *cluster_graph_ptr.  It computes the displacement
// between the cluster's original center-of-mass and its new,
// perturbed center-of-mass.  This method adds this displacement to
// all of the cluster's constituent nodes.

   void adjust_child_node_posns_from_cluster_COMs(
      graph* cluster_graph_ptr,graph* child_graph_ptr,
      graph* parent_graph_ptr)
   {
      cout << "inside graphfunc::adjust_node_posns_from_cluster_COMs()" 
           << endl;

      for (graph::CLUSTERS_MAP::iterator itr=cluster_graph_ptr->
              get_clusters_map().begin(); 
           itr != cluster_graph_ptr->get_clusters_map().end(); ++itr)
      {
         int parent_node_ID=itr->first;
         twovector init_cluster_COM=graphfunc::compute_cluster_COM(
            cluster_graph_ptr,child_graph_ptr,parent_node_ID);

         node* parent_node_ptr=parent_graph_ptr->get_node_ptr(
            parent_node_ID);
//      cout << "parent_node_ID = " << parent_node_ID
//           << " n_neighbors = " << parent_node_ptr->get_n_neighbors()
//           << endl;
      
         twovector final_cluster_COM(parent_node_ptr->get_posn());
         twovector cluster_COM_displacement=
            final_cluster_COM-init_cluster_COM;

         vector<int> clustered_node_IDs=
            cluster_graph_ptr->get_clusters_map().at(parent_node_ID);
         for (unsigned int i=0; i<clustered_node_IDs.size(); i++)
         {
            node* child_node_ptr=child_graph_ptr->get_node_ptr(
               clustered_node_IDs[i]);
            twovector init_node_posn(
               child_node_ptr->get_Uposn(),child_node_ptr->get_Vposn());
            twovector final_node_posn=
               init_node_posn+cluster_COM_displacement;
            child_node_ptr->set_posn(final_node_posn);
         } // loop over index i labeling child nodes in current cluster
      } // iterator itr index
   }

// ---------------------------------------------------------------------
// Method strengthen_clusters_in_layout() loops over all clusters in
// *cluster_graph_ptr.  For each cluster, it computes a center-of-mass
// as well as median radial distance for the cluster's constituent
// nodes.  For any constituent whose radius exceeds the median cluster
// radius, the over-radial distance is expontentially diminished.

   void strengthen_clusters_in_layout(
      graph* cluster_graph_ptr,graph* child_graph_ptr)
   {
//         cout << "inside graphfunc::strengthen_clusters_in_layout()" << endl;
      const unsigned int min_n_nodes_in_cluster=3;

      for (graph::CLUSTERS_MAP::iterator itr=
              cluster_graph_ptr->get_clusters_map().begin();
           itr != cluster_graph_ptr->get_clusters_map().end(); ++itr)
      {
         int parent_node_ID=itr->first;
         vector<int> clustered_node_IDs=
            cluster_graph_ptr->get_clusters_map().at(parent_node_ID);
//            cout << "clustered_node_IDs.size() = "
//                 << clustered_node_IDs.size() << endl;

// Ignore any "cluster" which contains fewer nodes than
// min_n_nodes_in_cluster:

         if (clustered_node_IDs.size() < min_n_nodes_in_cluster) continue;

         twovector cluster_COM=graphfunc::compute_cluster_COM(
            cluster_graph_ptr,child_graph_ptr,parent_node_ID);
//            cout << "cluster_COM = " << cluster_COM << endl;

         double r_avg=graphfunc::median_cluster_radius(
            cluster_graph_ptr,child_graph_ptr,parent_node_ID);
//            cout << "r_avg = " << r_avg << endl;

// Contract radius of a node if it exceeds the average cluster radius:

         for (unsigned int i=0; i<clustered_node_IDs.size(); i++)
         {
            node* child_node_ptr=child_graph_ptr->get_node_ptr(
               clustered_node_IDs[i]);
            twovector curr_posn(child_node_ptr->get_posn());
            twovector rel_curr_posn(curr_posn-cluster_COM);
            double curr_radius=rel_curr_posn.magnitude();
            double frac=curr_radius/r_avg - 1;

            if (frac <= 0) continue;

            double delta_radius=curr_radius-r_avg;
            const double alpha=1;
            delta_radius = exp(-alpha*frac)*delta_radius;
            curr_radius=r_avg+delta_radius;

            curr_posn=cluster_COM+curr_radius*rel_curr_posn.unitvector();
            child_node_ptr->set_posn(curr_posn);
         } // loop over index i labeling nodes within current cluster
      } // iterator itr loop
   }

// ---------------------------------------------------------------------
// Method angularly_redistribute_cluster_nodes() loops over all
// clusters in *cluster_graph_ptr.  For each cluster, this method
// evenly redistributes its constituent nodes in angle while
// preserving their cluster radii.  In Feb 2010, we experimented with
// this approach to spreading apart densely packed nodes in the 2.3K
// MIT photo set.  But we eventually abandoned this idea.

   void angularly_redistribute_cluster_nodes(
      graph* cluster_graph_ptr,graph* child_graph_ptr)
   {
//   cout << "inside graphfunc::angularly_redistribute_cluster_nodes()" << endl;

      for (graph::CLUSTERS_MAP::iterator itr=
              cluster_graph_ptr->get_clusters_map().begin();
           itr != cluster_graph_ptr->get_clusters_map().end(); ++itr)
      {
         int parent_node_ID=itr->first;
         vector<int> clustered_node_IDs=cluster_graph_ptr->
            get_clusters_map().at(parent_node_ID);
         twovector cluster_COM=graphfunc::compute_cluster_COM(
            cluster_graph_ptr,child_graph_ptr,parent_node_ID);
   
// Compute angle of each node about its cluster's COM:

         vector<double> theta;
         vector<node*> node_ptrs;
         for (unsigned int i=0; i<clustered_node_IDs.size(); i++)
         {
            node* curr_node_ptr=child_graph_ptr->get_node_ptr(
               clustered_node_IDs[i]);
            twovector curr_posn(curr_node_ptr->get_posn());
            twovector rel_curr_posn(curr_posn-cluster_COM);
            double curr_theta=atan2(rel_curr_posn.get(1),
                                    rel_curr_posn.get(0));
            theta.push_back(curr_theta);
            node_ptrs.push_back(curr_node_ptr);
         } // loop over index i labeling nodes within current cluster

         templatefunc::Quicksort(theta,node_ptrs);
         double delta_theta=2*PI/theta.size();

// Evenly redistribute angles of all nodes about their cluster_COM:

         for (unsigned int i=0; i<clustered_node_IDs.size(); i++)
         {
            node* curr_node_ptr=node_ptrs[i];
            double modified_theta=theta[0]+i*delta_theta;
         
            twovector curr_posn(curr_node_ptr->get_posn());
            twovector rel_curr_posn(curr_posn-cluster_COM);
            double curr_radius=rel_curr_posn.magnitude();
            twovector r_hat(cos(modified_theta),sin(modified_theta));
         
            curr_posn=cluster_COM+curr_radius*r_hat;
            curr_node_ptr->set_posn(curr_posn);
         } // loop over index i labeling nodes within current cluster
      } // iterator itr index
   }

// ==========================================================================
// Cluster information propagation methods
// ==========================================================================

// Method propagate_cluster_IDs_to_input_graph_children() acts on
// *cluster_graph_ptr whose clusters_map is assumed to have been
// pre-filled with parent cluster information.  Iterating over each
// cluster within the member STL map, this method sets the child
// nodes' parent ID equal to the cluster ID.

   void propagate_cluster_IDs_to_input_graph_children(
      graph* cluster_graph_ptr,graph* child_graph_ptr)
   {
//         cout << "inside graphfunc::propagate_cluster_IDs_to_input_graph_children()" 
//              << endl;

      for (graph::CLUSTERS_MAP::iterator itr=
              cluster_graph_ptr->get_clusters_map().begin();
           itr != cluster_graph_ptr->get_clusters_map().end(); ++itr)
      {
         int cluster_node_ID=itr->first;
         vector<int> children_node_IDs=itr->second;

         for (unsigned int c=0; c<children_node_IDs.size(); c++)
         {
            node* child_node_ptr=child_graph_ptr->get_node_ptr(
               children_node_IDs[c]);
            if (child_node_ptr==NULL)
            {
               cout << "child_node_ptr = " << child_node_ptr << endl;
               cout << "c = " << c
                    << " children_node_IDs[c] = " << children_node_IDs[c]
                    << endl;
               cout << "cluster_node_ID = " << cluster_node_ID << endl;
            }

            child_node_ptr->set_parent_ID(cluster_node_ID);
         } // loop over index c
      } // iterator itr loop
   }

// ---------------------------------------------------------------------
// Method generate_parent_graph_nodes_from_clusters() extracts the
// node ID for parents from the clusters map within input
// *cluster_graph_ptr.  A new parent node is added to the parent graph
// for each cluster map entry, and children node IDs are assigned to
// the new parent based upon the remaining entries in each line of the
// input file.

   void generate_parent_graph_nodes_from_clusters(
      graph* cluster_graph_ptr,graph* parent_graph_ptr)
   {
//         cout << "inside graphfunc::generate_parent_graph_nodes_from_clusters()" 
//              << endl;
   
      for (graph::CLUSTERS_MAP::iterator itr=
              cluster_graph_ptr->get_clusters_map().begin();
           itr != cluster_graph_ptr->get_clusters_map().end(); ++itr)
      {
         int parent_node_ID=itr->first;
         int parent_level=parent_graph_ptr->get_level();
         node* parent_node_ptr=new node(parent_node_ID,parent_level);
         parent_graph_ptr->add_node(parent_node_ptr);

         vector<int> children_node_IDs=itr->second;
         parent_node_ptr->set_children_node_IDs(children_node_IDs);
      } // iterator itr loop
   }

// ---------------------------------------------------------------------
// Method set_parent_node_positions_based_on_children() calculates
// graph coordinates for all nodes within input *parent_graph_ptr
// based upon averages of their children node's (gx,gy) coordinates.
// It also identifies a representative child for each node within
// *parent_graph_ptr based upon maximal degree.  The ID for the
// representative child node is stored within the parent node's
// representative_child_ID member.  The representative child node's
// data ID is also assigned as the parent's data ID.

   void set_parent_node_positions_based_on_children(
      graph* parent_graph_ptr,graph* child_graph_ptr)
   {
//         cout << "inside graphfunc::set_parent_node_positions_based_on_children()" << endl;

      vector<double> gx,gy;
      for (unsigned int n=0; n<parent_graph_ptr->get_n_nodes(); n++)
      {
         node* parent_node_ptr=parent_graph_ptr->get_ordered_node_ptr(n);
         vector<int> child_node_IDs=parent_node_ptr->
            get_children_node_IDs();

         int representative_child_node_ID=-1;
         double max_degree=NEGATIVEINFINITY;

         gx.clear();
         gy.clear();
         for (unsigned int i=0; i<child_node_IDs.size(); i++)
         {
            int child_ID=child_node_IDs[i];
            node* child_node_ptr=child_graph_ptr->get_node_ptr(child_ID);
            if (child_node_ptr==NULL)
            {
               cout << "Trouble in graphfunc::set_parent_node_posns_based_on_children()" << endl;
               cout << "child_ID = " << child_ID << " child_node_ptr = NULL"
                    << endl;
               outputfunc::enter_continue_char();
               continue;
            }

            gx.push_back(child_node_ptr->get_Uposn());
            gy.push_back(child_node_ptr->get_Vposn());
            double curr_degree=child_node_ptr->get_degree();
            if (curr_degree > max_degree)
            {
               max_degree=curr_degree;
               representative_child_node_ID=child_ID;
            }
         } // loop over index i labeling nodes within current cluster
         double gx_avg=mathfunc::mean(gx);
         double gy_avg=mathfunc::mean(gy);

         parent_node_ptr->set_Uposn(gx_avg);
         parent_node_ptr->set_Vposn(gy_avg);

// Compute and store representative child node and data IDs within
// current parent node:

         parent_node_ptr->set_representative_child_ID(
            representative_child_node_ID);
         node* representative_child_node_ptr=child_graph_ptr->
            get_node_ptr(representative_child_node_ID);
         parent_node_ptr->set_data_ID(
            representative_child_node_ptr->get_data_ID());

      } // loop over index n labeling parent nodes
   }

// ---------------------------------------------------------------------
// Method function hierarchical_grandparents_clustering() is assumed
// to work on a current "parents" graph within input *parent_graph_ptr
// whose children nodes are known.  It takes in
// *grandparents_graph_ptr whose grandchildren nodes are also assumed
// to be known.  In general, we assume that the coarse and finer
// clustering used to form the grandparents and parents graph are not
// strictly hierarchical.  Candidate grandparent IDs flow to candidate
// parents via the common children base layer.  This method implements
// a "voting" approach to determine which parent nodes should actually
// be grouped together into a grandparent cluster based upon their
// common children.  Each parent node within the current graph is
// assigned a unique grandparent ancestor.

   void hierarchical_grandparents_clustering(
      graph* parent_graph_ptr,graph* grandparents_graph_ptr)
   {
//         cout << "inside graphfunc::hierarchical_grandparents_clustering()" 
//              << endl;

      typedef map<int,int> CHILD_PARENT_MAP;
      CHILD_PARENT_MAP child_parent_map;

// Loop over current parents map clusters.  Store parent cluster ID as
// a function of child node ID within child_parent_map:

      for (graph::CLUSTERS_MAP::iterator itr=
              parent_graph_ptr->get_clusters_map().begin();
           itr != parent_graph_ptr->get_clusters_map().end(); ++itr)
      {
         int parent_node_ID=itr->first;
         vector<int> children_node_IDs=itr->second;

//            cout << "parent_node_ID = " << parent_node_ID
//                 << " children_node_IDs.size() = "
//                 << children_node_IDs.size() << endl;

         for (unsigned int c=0; c<children_node_IDs.size(); c++)
         {
            int curr_child_node_ID=children_node_IDs[c];
//               cout << "c = " << c << " child node = " << curr_child_node_ID
//                    << endl;
            child_parent_map[curr_child_node_ID]=parent_node_ID;
         } // loop over index c labeling children in current parent cluster
      } // loop over parent clusters

      typedef map<int,vector<int> > PARENT_GRANDPARENTS_MAP;
      PARENT_GRANDPARENTS_MAP parent_grandparents_map;

      typedef map<int,int> GRANDPARENTS_FREQ_MAP;
      GRANDPARENTS_FREQ_MAP grandparents_freq_map;
   
// Loop over grand parents map clusters.  Extract each grandparent's
// grandchildren.  Look up each grand child's parent.  Identify
// grandparents with parents via the children:

      for (graph::CLUSTERS_MAP::iterator itr=
              grandparents_graph_ptr->get_clusters_map().begin();
           itr != grandparents_graph_ptr->get_clusters_map().end(); ++itr)
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

            if (curr_parent_ID < 0 || grandparent_node_ID < 0)
            {
               cout << "grandparent cluster ID = " << grandparent_node_ID
                    << " curr_parent_ID = " << curr_parent_ID << endl;
               cout << "curr_child_ID = " << curr_child_ID << endl;

               outputfunc::enter_continue_char();
            }

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

         } // loop over index c labeling children in grandparent cluster
//            cout << "----------------------------------------------" << endl;
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
               grandparents_freq_map[curr_grandparent_ID]=
                  freq_itr->second+1;
            }
            else
            {
               grandparents_freq_map[curr_grandparent_ID]=1;
            }
         } // loop over index c labeling candidate grandparent ID

//            cout << "parent_ID = " << parent_ID << endl;
//            cout << "Candidate grandparent IDs = " << endl;
//            templatefunc::printVector(candidate_grandparent_IDs);

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
//               cout << "Grandparent ID = " << freq_itr->first
//                    << " frequency = " << curr_freq << endl;
         }

//            cout << "parent_ID = " << parent_ID << endl;
//            cout << "unique_grandparent_ID = " << unique_grandparent_ID
//                 << endl;

         node* parent_node_ptr=parent_graph_ptr->get_node_ptr(parent_ID);
         parent_node_ptr->set_parent_ID(unique_grandparent_ID);

//            cout << "Parent ID = " << parent_node_ptr->get_ID()
//                 << " unique grandparent ID = " 
//                 << unique_grandparent_ID << endl;
//            cout << "----------------------------------------------" << endl;

      } // loop over parent IDs within parent_grandparents_map
   }

// ==========================================================================
// Graph path methods
// ==========================================================================
   
// Method print_shortest_path() takes in some destination node's
// pointer.  It prints the total distance of the destination from the
// starting node.  This method then prints out the node IDs for the
// shortest path from the destination back to the starting node.

   void print_shortest_path(node* destination_node_ptr)
   {
      node* previous_node_ptr = destination_node_ptr;
      cout << "Distance from starting node: " 
           << destination_node_ptr->get_distance_from_start() << endl;
      while (previous_node_ptr != NULL)
      {
         cout << "Node ID = " << previous_node_ptr->get_ID() << endl;
         previous_node_ptr = previous_node_ptr->get_path_predecessor_ptr();
      }
      cout << endl;
   }
   


} // graphfuncs namespace







