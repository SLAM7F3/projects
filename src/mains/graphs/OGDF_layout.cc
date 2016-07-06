// =========================================================================
// Program OGDF_LAYOUT parses an input graph edgelist text file.  It
// queries the user to set a minimum edge weight threshold.  [By
// convention, maximum edge weight equals 100 while mininum edge
// weight equals 0.]

// It next calls a graphical layout algorithm within the C++ Open
// Graph Drawing Framework (OGDF) library.  As of 9/1/09, we have
// found that only the Fast Multipole Multilevel layout algorithm can
// handle the 25K connected graph coming from the July 2009 MIT photo
// collect.  This program outputs the OGDF layout as a Graph Modeling
// Language (GML) text file.
// =========================================================================
// Last updated on 1/8/14; 11/28/15; 11/29/15; 11/30/15
// =========================================================================

#include <algorithm>
#include <map>
#include <vector>
#include <string>
#include <ogdf/misclayout/BalloonLayout.h>
#include <ogdf/internal/planarity/ConnectedSubgraph.h>
#include <ogdf/energybased/DavidsonHarelLayout.h>
#include <ogdf/layered/DfsAcyclicSubgraph.h>
#include <ogdf/basic/EdgeArray.h>
#include <ogdf/fileformats/GraphIO.h>
#include <ogdf/energybased/FMMMLayout.h>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/graph_generators.h>
#include <ogdf/basic/NodeArray.h>
#include <ogdf/energybased/SpringEmbedderFRExact.h>
#include <ogdf/layered/SugiyamaLayout.h>

#include "color/colorfuncs.h"
#include "general/filefuncs.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "passes/PassesGroup.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "templates/mytemplates.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using namespace ogdf;
   using std::cin;
   using std::cout;
   using std::endl;
   using std::map;
   using std::string;
   using std::vector;

   std::set_new_handler(sysfunc::out_of_memory);
   cout.precision(15);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);

   string bundle_filename=passes_group.get_bundle_filename();
//   cout << " bundle_filename = " << bundle_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(bundle_filename);
//   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string graphs_subdir=bundler_IO_subdir+"graphs/";

// Pass current graph component ID as input parameter:

   int graph_component_ID=passes_group.get_graph_component_ID();
   cout << "graph_component_ID = " << graph_component_ID << endl;

   double min_edge_weight=-1;
   cout << endl;
   cout << "Enter edge weight threshold.  All input edges with weights " 
        << endl;
   cout << "below this threshold will be ignored:" << endl;
   cin >> min_edge_weight;
   cout << endl;

// Read in graph edge list:
   
   string edgelist_filename=graphs_subdir+"edgelist.dat";
   cout << "edgelist_filename = " << edgelist_filename << endl;
   filefunc::ReadInfile(edgelist_filename);
   cout << "Number of input graph edges = " << filefunc::text_line.size() 
        << endl;

/*
// Compute probability distribution for all edge weights:

   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=
         stringfunc::string_to_numbers(filefunc::text_line[i]);
//      double curr_edge_weight=column_values[2];
      edge_weight.push_back(column_values[2]);
   }
   prob_distribution edge_probs(edge_weight,100);
   bool gzip_flag=false;
   edge_probs.writeprobdists(gzip_flag);
   string banner="Exported edge weight probability distribution";
   outputfunc::write_big_banner(banner);
//   outputfunc::enter_continue_char();
*/

// Determine maximum edge weight value:

   double max_edge_weight = 0;
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=
         stringfunc::string_to_numbers(filefunc::text_line[i]);
      double curr_edge_weight=column_values[2];
      max_edge_weight = basic_math::max(curr_edge_weight, max_edge_weight);
   }

// Renormalize all edge weights to range from 10 to 100:
   
   double min_renormalized_edge_weight = 10;
   double max_renormalized_edge_weight = 100;
   vector<int> edge_weight;
   vector<int> first_node_ID,second_node_ID;
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=
         stringfunc::string_to_numbers(filefunc::text_line[i]);
      double curr_edge_weight=column_values[2];
      if (curr_edge_weight < min_edge_weight) continue;

      double renormalized_edge_weight = min_renormalized_edge_weight + 
         (max_renormalized_edge_weight - min_renormalized_edge_weight) * 
         (curr_edge_weight - min_edge_weight) / 
         (max_edge_weight - min_edge_weight);

      first_node_ID.push_back(column_values[0]);
      second_node_ID.push_back(column_values[1]);
      edge_weight.push_back(renormalized_edge_weight);
   }
   cout << "Number of edges with weights exceeding specified threshold = "
        << edge_weight.size() << endl << endl;

// Instantiate OGDF graph G:

   Graph G;

// Instantiate photo_map which holds node_ID vs OGDF node information:

   typedef std::map<int,node> PHOTO_MAP;
   PHOTO_MAP photo_map;

   node node1,node2;
   int n_edges=first_node_ID.size();
   for (int e=0; e<n_edges; e++)
   {
      int curr_node_ID=first_node_ID[e];
      PHOTO_MAP::iterator iter1=photo_map.find(curr_node_ID);
      if (iter1==photo_map.end())
      {
         node1=G.newNode(curr_node_ID);
         photo_map[curr_node_ID]=node1;
      }
      else
      {
         node1=iter1->second;
      }
      
      curr_node_ID=second_node_ID[e];
      PHOTO_MAP::iterator iter2=photo_map.find(curr_node_ID);
      if (iter2==photo_map.end())
      {
         node2=G.newNode(curr_node_ID);
         photo_map[curr_node_ID]=node2;
      }
      else
      {
         node2=iter2->second;
      }
      
      G.newEdge(node1,node2);
   } // loop over index e labeling edges
   cout << "photo_map.size() = " << photo_map.size() << endl;
//   outputfunc::enter_continue_char();

// Extract connected components from graph G:

   vector<int> connected_component_sizes;
   vector<Graph> connected_components;
   vector<vector<int> > connected_component_node_IDs;

   node curr_node;
   vector<bool> node_visited_flag;
   vector<int> node_ID_given_index;
   forall_nodes(curr_node,G)
   {
      int curr_node_ID=curr_node->index();
      node_visited_flag.push_back(false);
      node_ID_given_index.push_back(curr_node_ID);
   }

// Generate copy graph where nodes are indexed by counter which ranges
// from 0 to n_nodes-1 rather than by integer node IDs:

   Graph Gcopy(G);

   node copy_node;
   vector<node> node_given_index;
   forall_nodes(copy_node,Gcopy)
   {
//      int copy_node_index=copy_node->index();
      node_given_index.push_back(copy_node);
//      cout << "Copy node index = " << copy_node->index()
//           << " copy node ID = " << node_ID_given_index[copy_node_index]
//           << endl;
   }

   int iteration=0;
   int n_nodes=Gcopy.numberOfNodes();
   while (n_nodes > 0)
   {
      cout << "================================================" << endl;
      cout << "In iteration = " << iteration << ", graph G has n_nodes = " 
           << n_nodes << endl;

// Search for first node which has not already been tagged as
// belonging to some connected component:

      int index=0;
      while (node_visited_flag[index])
      {
         index++;
      }
//      cout << "index = " << index << endl;
//      int node_ID=node_ID_given_index[index];
      node first_node=node_given_index[index];
//      cout << "node ID = " << node_ID
//           << " first_node->index() = " << first_node->index() 
//          << endl;

// Given the first node within a new connected component, recursively
// find all of its neighbors within the connected component:

      Graph SG;
      NodeArray<int> SG_NodeID;
      NodeArray<node> subgraph_nodes;
      ogdf::ConnectedSubgraph<int>::call(Gcopy,SG,first_node,subgraph_nodes);

      int n_subgraph_nodes=SG.numberOfNodes();
//      cout << "SG.num_nodes = " << n_subgraph_nodes << endl;
      connected_component_sizes.push_back(n_subgraph_nodes);
      connected_components.push_back(SG);

      vector<int> curr_component_node_IDs;
      for (int n=0; n<n_subgraph_nodes; n++)
      {
         node curr_node=subgraph_nodes[n];
         int curr_node_index=curr_node->index();
         int curr_node_ID=node_ID_given_index[curr_node_index];
         curr_component_node_IDs.push_back(curr_node_ID);
         node_visited_flag[curr_node_index]=true;
//         cout << "subgraph node ID = " << curr_node_ID << endl;
      }
      connected_component_node_IDs.push_back(curr_component_node_IDs);

      n_nodes -= n_subgraph_nodes;
      iteration++;
   } // n_nodes > 0 while loop

// Sort connected components into descending order in terms of their sizes:

   templatefunc::Quicksort_descending(
      connected_component_sizes,connected_components,
      connected_component_node_IDs);

// Export text file containing connected graph component information:

   int n_nodes_integral=0;
   string connected_filename=graphs_subdir+"connected_SIFT_components.dat";
   ofstream connected_stream;
   filefunc::openfile(connected_filename,connected_stream);

   connected_stream << "Minimum edge weight = " 
                    << stringfunc::number_to_string(min_edge_weight)
                    << endl << endl;
   for (int c=connected_component_sizes.size()-1; c >=0; c--)
   {
      cout << "Connected component #" << c 
           << " has " << connected_component_sizes[c] << " nodes" 
           << endl;
      connected_stream << "Connected component # " << c 
                       << " has " << connected_component_sizes[c]
                       << " nodes" << endl;
      n_nodes_integral += connected_component_sizes[c];
   }
   connected_stream << endl;
   connected_stream << "n_nodes_integral = " << n_nodes_integral << endl;
   filefunc::closefile(connected_filename,connected_stream);
   cout << "n_nodes_integral = " << n_nodes_integral << endl;

// Query user to enter smallest connected component to retain:

   cout << "Enter minimum number of nodes allowed within smallest connected component:" << endl;

   int min_n_connected_nodes=1;
//   int min_n_connected_nodes=500;
   cin >> min_n_connected_nodes;
   int n_connected_components_counter=0;

   for (unsigned int c=0; c<connected_component_sizes.size(); c++)
   {
      if (connected_component_sizes[c] >= min_n_connected_nodes)
         n_connected_components_counter++;
   }

   cout << n_connected_components_counter 
        << " connected components contain more than "
        << min_n_connected_nodes << " nodes" << endl;

// Write out IDs for nodes belonging to distinct connected graph
// components:

   for (int connected_component_counter=0; 
        connected_component_counter<n_connected_components_counter; 
        connected_component_counter++)
   {
      vector<int> curr_connected_component_node_IDs=
         connected_component_node_IDs[connected_component_counter];
      std::sort(curr_connected_component_node_IDs.begin(),
      curr_connected_component_node_IDs.end());

      string connected_component_label="_C"+stringfunc::number_to_string(
         graph_component_ID+connected_component_counter);
      
      string connected_nodes_filename=graphs_subdir+
         "connected_nodes"+connected_component_label+".dat";
      ofstream nodes_stream;
      filefunc::openfile(connected_nodes_filename,nodes_stream);
      for (unsigned int j=0; j<curr_connected_component_node_IDs.size(); j++)
      {
         nodes_stream << curr_connected_component_node_IDs[j] << "  " << flush;
      }
      nodes_stream << endl;
      filefunc::closefile(connected_nodes_filename,nodes_stream);

//   bool retrieve_photo_ID_URL_given_node_ID(
//      gis_database* gis_database_ptr,int hierarchy_ID,int node_ID,
//      int& photo_ID,std::string& photo_URL,std::string& thumbnail_URL);


   } // loop over connected_component_counter

// ---------------------------------------------------------------------
// Loop over largest connected graph components starts here
// ---------------------------------------------------------------------

   for (int connected_component_counter=0; 
        connected_component_counter<n_connected_components_counter; 
        connected_component_counter++)
   {
      string connected_component_label="_C"+stringfunc::number_to_string(
         graph_component_ID+connected_component_counter);
      Graph G_connected=connected_components[connected_component_counter];

      string banner="Connected component #"+
         stringfunc::number_to_string(connected_component_counter)
         +" contains "+
         stringfunc::number_to_string(G_connected.numberOfNodes())+" nodes";
      outputfunc::write_big_banner(banner);
//      outputfunc::enter_continue_char();

/*
  List<edge> edgelist;
  G_connected.allEdges(edgelist);
  n_edges=edgelist.size();
  cout << "n_connected graph edges = " << n_edges << endl;

  for (ListIterator<edge> itr=edgelist.begin();
  itr != edgelist.end(); ++itr)
  {
  edge curr_edge=*itr;
  cout << "source node = " << curr_edge->source()
  << " target node = " << curr_edge->target()
  << endl;
  }
*/

      GraphAttributes GA(
         G_connected,
         GraphAttributes::nodeGraphics | GraphAttributes::edgeGraphics |
         GraphAttributes::nodeLabel | GraphAttributes::nodeTemplate |
         GraphAttributes::nodeStyle | GraphAttributes::edgeStyle);

      typedef std::map<int,int> CURR_CONNECTED_NODES_MAP;
      CURR_CONNECTED_NODES_MAP curr_connected_nodes_map;

      int i=0;
      forall_nodes(curr_node,G_connected)
      {
         int curr_node_ID=
            (connected_component_node_IDs[connected_component_counter])[i++];
         cout << "Connected component #" << connected_component_counter 
              << " node ID = " << curr_node_ID << endl;
         string label=stringfunc::number_to_string(curr_node_ID);

         GA.label(curr_node)=label;
         GA.width(curr_node)=GA.height(curr_node)=10.0;

         curr_connected_nodes_map[curr_node_ID]=1;
      }

// Write out edgelist for G_connected:

      string connected_edge_list_filename=
         graphs_subdir+"connected_edgelist"+connected_component_label+".dat";
      ofstream outstream;
      filefunc::openfile(connected_edge_list_filename,outstream);
      outstream << "# NodeID  NodeID'  Edge weight" << endl << endl;

      for (unsigned int i=0; i<first_node_ID.size(); i++)
      {
         int curr_node_ID=first_node_ID[i];
         CURR_CONNECTED_NODES_MAP::iterator iter1=
            curr_connected_nodes_map.find(curr_node_ID);
         if (iter1==curr_connected_nodes_map.end()) continue;

         outstream << first_node_ID[i] << "  "
                   << second_node_ID[i] << "  "
                   << edge_weight[i] << endl;
      }
      filefunc::closefile(connected_edge_list_filename,outstream);

// Fast Multipole Multilevel Layout algorithm

      banner="Computing Fast Multiple Multilevel graph layout...";
      outputfunc::write_banner(banner);

      FMMMLayout fmmm;
      fmmm.useHighLevelOptions(true);
//   fmmm.unitEdgeLength(15.0); 
      fmmm.unitEdgeLength(150.0); 
      fmmm.newInitialPlacement(true);
      fmmm.qualityVersusSpeed(FMMMLayout::qvsGorgeousAndEfficient);

// On 12/30/09, we experimented with varying several different FMMM
// parameters in an attempt to generate a better layout for Noah's 2K
// MIT reconstructed graph.  We empirically found that setting the
// resizeDrawing flag to true and the resizingScalar value to 300 had
// the only noticeable beneficial impact upon the final graph layout:

//   fmmm.forceModel(FMMMLayout::fmEades);
//   fmmm.forceModel(FMMMLayout::fmFruchtermanReingold);

//   fmmm.springStrength(0.01);
//   fmmm.repForcesStrength(100);
//   fmmm.postSpringStrength(0.01);
//   fmmm.postStrengthOfRepForces(100);
      fmmm.minGraphSize(300);
      fmmm.resizeDrawing(true);
//   fmmm.resizingScalar(3.0);
//   fmmm.resizingScalar(30.0);
//   fmmm.resizingScalar(10.0);
      fmmm.resizingScalar(300.0);

      cout << "fmmm.forceModel() = " << fmmm.forceModel() << endl;
      cout << "fmmm.springStrength() = " << fmmm.springStrength() << endl;
      cout << "fmmm.repForcesStrength() = " << fmmm.repForcesStrength() 
           << endl;
      cout << "minDistCC = " << fmmm.minDistCC() << endl;
      cout << "postSpringStrength = " << fmmm.postSpringStrength() << endl;
      cout << "postStrengthOfRepForces() = "
           << fmmm.postStrengthOfRepForces() << endl;
      cout << "minGraphSize() = " << fmmm.minGraphSize() << endl;
      cout << "coolTemperature() = " << fmmm.coolTemperature() << endl;
      cout << "coolValue() = " << fmmm.coolValue() << endl;
      cout << "resizingScalar() = " << fmmm.resizingScalar() << endl;

      fmmm.call(GA);
   
      string gml_filename=graphs_subdir+"graph_layout"+
         connected_component_label+".gml";
      GraphIO::writeGML(GA, gml_filename.c_str());

      banner="Wrote out "+gml_filename;
      outputfunc::write_banner(banner);
       
   } // loop over iter index labeling large connected graph components

}

