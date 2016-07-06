// ==========================================================================
// Program COUNT_CCS imports a user-specified edge list for some SIFT,
// GIST, color, caffe_feature, etc graph.  It also queries the user to
// enter an edge weight threshold.  COUNT_CCS scans through the edge
// list and enters each node into a map_unionfind data structure.
// Nodes connected by an edge are linked within the map_unionfind.
// COUNT_CCS exports a list of connected components sorted by the
// number of nodes they contain to an output text file.

// 				./count_ccs

// ==========================================================================
// Last updated on 8/15/13; 11/26/15; 11/27/15; 11/28/15
// ==========================================================================

#include  <algorithm>
#include  <iostream>
#include  <map>
#include  <string>
#include  <vector>

#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "datastructures/map_unionfind.h"
#include "passes/PassesGroup.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);

   string image_list_filename=passes_group.get_image_list_filename();
   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string graphs_subdir = bundler_IO_subdir+"graphs/";

   string graph_edgelist_filename = graphs_subdir+"edgelist.dat";
//   cout << "Enter full path for input graph edge list file:" << endl;
//   cin >> graph_edgelist_filename;

   double edgeweight_threshold=0;
   cout << endl;
   cout << "Enter edge weight threshold:" << endl;
   cin >> edgeweight_threshold;

   timefunc::initialize_timeofday_clock(); 

//   string graph_edgelist_filename="../gist/gist_edgelist.dat";
//   string color_hist_subdir=
//      "/data/ImageEngine/tidmarsh/color_histograms/";
//   string graph_edgelist_filename=
//      color_hist_subdir+"image_gist_colors_edgelist.dat";

   filefunc::ReadInfile(graph_edgelist_filename);

   int n_lines = filefunc::text_line.size();
   cout << "Number of imported node pairs + edge weights = " 
        << n_lines << endl;
   outputfunc::print_elapsed_time();

// Enter all node IDs into *map_unionfind_ptr:

   map_unionfind* map_unionfind_ptr=new map_unionfind();
   for (int i=0; i<n_lines; i++)
   {
      outputfunc::update_progress_fraction(i, 0.1 * n_lines, n_lines);
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      int node1_ID=column_values[0];
      int node2_ID=column_values[1];

      if (!map_unionfind_ptr->ElementOf(DUPLE(node1_ID,0)))
      {
         map_unionfind_ptr->MakeSet(DUPLE(node1_ID,0));
      }

      if (!map_unionfind_ptr->ElementOf(DUPLE(node2_ID,0)))
      {
         map_unionfind_ptr->MakeSet(DUPLE(node2_ID,0));
      }
   }
   cout << endl;
   cout << "Number of nodes in map_unionfind = "
        << map_unionfind_ptr->get_n_nodes() << endl;
   outputfunc::print_elapsed_time();

   for (int i=0; i<n_lines; i++)
   {
      outputfunc::update_progress_fraction(i, 0.1 * n_lines, n_lines);
//      cout << "i = " << i << " filefunc::text_line[i] = "
//           << filefunc::text_line[i] << endl;

      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      int node1_ID=column_values[0];
      int node2_ID=column_values[1];
      double edgeweight=column_values[2];

      if (edgeweight < edgeweight_threshold) continue;

//      cout << "node1_ID = " << node1_ID 
//           << " node2_ID = " << node2_ID
//           << " edge weight = " << edgeweight << endl;
//      outputfunc::enter_continue_char();

      map_unionfind_ptr->Link(DUPLE(node1_ID,0),DUPLE(node2_ID,0));
   } // loop over index i labeling lines within input edge list file
   cout << endl;
   outputfunc::print_elapsed_time();

   typedef map<int,int> NODE_ROOT_MAP;
// Independent int = root_ID
// Dependent int = number of nodes associated with root_ID

   NODE_ROOT_MAP node_root_map;
   NODE_ROOT_MAP::iterator node_root_iter;

   map_unionfind::NODES_MAP* nodes_map_ptr=map_unionfind_ptr->
      get_nodes_map_ptr();
   for (map_unionfind::NODES_MAP::iterator iter=
           nodes_map_ptr->begin(); iter != nodes_map_ptr->end(); iter++)
   {
      DUPLE node_ID=iter->first;
//      cout << "node_ID = " << node_ID << endl;
      int root_ID=map_unionfind_ptr->Find(node_ID).first;
//      cout << "root_ID = " << root_ID << endl;

      node_root_iter=node_root_map.find(root_ID);
      if (node_root_iter==node_root_map.end())
      {
         node_root_map[root_ID]=1;
      }
      else
      {
         node_root_iter->second=node_root_iter->second+1;
      }
   } // loop over iter index 

   vector<int> cc_sizes;
   for (node_root_iter=node_root_map.begin(); node_root_iter !=
           node_root_map.end(); node_root_iter++)
   {
      cc_sizes.push_back(node_root_iter->second);
   }
   
   std::sort(cc_sizes.begin(), cc_sizes.end(), std::greater<int>());

   string ccs_filename=graphs_subdir+"cc_sizes.dat";
   ofstream cc_stream;
   filefunc::openfile(ccs_filename,cc_stream);

   int cc_size_sum=0;
   for (unsigned int cc=0; cc<cc_sizes.size(); cc++)
   {
//      cout << "CC_ID = " << cc
//           << " CC_SIZE = " << cc_sizes[cc] << endl;
      cc_stream << "CC_ID = " << cc
                << " CC_SIZE = " << cc_sizes[cc] << endl;
      cc_size_sum += cc_sizes[cc];
   }
   cc_stream << endl;
   cc_stream << "CC_SIZES sum = " << cc_size_sum << endl;
   cc_stream << "Edge weight threshold = " << edgeweight_threshold << endl;

   filefunc::closefile(ccs_filename,cc_stream);
   string banner="Exported connected component sizes to "+ccs_filename;
   outputfunc::write_big_banner(banner);

   delete map_unionfind_ptr;

   outputfunc::print_elapsed_time();
}

