
#include <map>
#include <vector>
#include <string>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/graph_generators.h>
#include <ogdf/layered/DfsAcyclicSubgraph.h>
#include <ogdf/energybased/FMMMLayout.h>

#include "general/filefuncs.h"
#include "general/stringfuncs.h"


using namespace ogdf;
using std::cin;
using std::cout;
using std::endl;
using std::map;
using std::string;
using std::vector;

int main()
{

// Read in Noah's edge list:
   
   string filename="test1.pairs";
   cout << "Enter name of Noah's input edge list file:" << endl;
   cin >> filename;
   filefunc::ReadInfile(filename);

   vector<int> first_node_ID,second_node_ID;
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> FS=stringfunc::string_to_numbers(filefunc::text_line[i]);
      first_node_ID.push_back(FS[0]);
      second_node_ID.push_back(FS[1]);
   }

// Instantiate OGDF graph and photo_map:

   typedef std::map<int,node> PHOTO_MAP;
   PHOTO_MAP photo_map;

   Graph G;
   GraphAttributes GA(G);
   
   node node1,node2;
   int n_edges=first_node_ID.size();
   for (int e=0; e<n_edges; e++)
   {
      int curr_ID=first_node_ID[e];
      PHOTO_MAP::iterator iter1=photo_map.find(curr_ID);
      if (iter1==photo_map.end())
      {
         node1=photo_map[curr_ID]=G.newNode();
      }
      else
      {
         node1=iter1->second;
      }
      
      curr_ID=second_node_ID[e];
      PHOTO_MAP::iterator iter2=photo_map.find(curr_ID);
      if (iter2==photo_map.end())
      {
         node2=photo_map[curr_ID]=G.newNode();
      }
      else
      {
         node2=iter2->second;
      }
      
      G.newEdge(node1,node2);
   }

   cout << "photo_map.size() = " << photo_map.size() << endl;

   node v;
   forall_nodes(v,G)
      GA.width(v) = GA.height(v) = 10.0;
 
   FMMMLayout fmmm;
 
   fmmm.useHighLevelOptions(true);
   fmmm.unitEdgeLength(15.0); 
   fmmm.newInitialPlacement(true);
   fmmm.qualityVersusSpeed(FMMMLayout::qvsGorgeousAndEfficient);
 
   fmmm.call(GA);
   GA.writeGML("new-layout.gml"); 
}

