// =======================================================================
// Connected components computation for MIT 30K photo set
// =======================================================================
// Last updated on 8/30/09
// =======================================================================

#include <boost/config.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <algorithm>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "general/filefuncs.h"
#include "templates/mytemplates.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"


int main(int , char* []) 
{
   using namespace boost; 
   using std::cout;
   using std::endl;
   using std::ofstream;
   using std::sort;
   using std::string;
   using std::vector;

   typedef adjacency_list <vecS, vecS, undirectedS> Graph;
   Graph G;
   
   string filename="MIT.pairs";
   filefunc::ReadInfile(filename);

   vector<int> first_node_ID,second_node_ID,nmatches;

   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> curr_triple=
         stringfunc::string_to_numbers(filefunc::text_line[i]);
      first_node_ID.push_back(curr_triple[0]);
      second_node_ID.push_back(curr_triple[1]);
      nmatches.push_back(curr_triple[2]);


//      cout << "first_node = " << first_node 
//           << " second_node = " << second_node
//           << " n_matches = " << n_matches << endl;
      add_edge(first_node_ID.back(),second_node_ID.back(),G);
   }

   cout << "first_node_ID.size() = " << first_node_ID.size()
        << " second_node_ID.size() = " << second_node_ID.size()
        << " nmatches.size() = " << nmatches.size() << endl;

   vector<int> component(num_vertices(G));
   int num = connected_components(G, &component[0]);
   cout << "number connected components = " << num << endl;
   cout << "component.size() = " << component.size() << endl;
   outputfunc::enter_continue_char();   


   vector<int>::size_type i;
   cout << "Total number of components: " << num << endl;
   int n0=0;
   int n1=0;
   for (i = 0; i != component.size(); ++i)
   {
      cout << "Vertex " << i << " is in component " << component[i] << endl;
      if (component[i]==0)
      {
         n0++;
      }
      else if (component[i]==1)
      {
         n1++;
      }
   }


   ofstream outstream;
   string output_filename="MIT_connected.pairs";
   filefunc::openfile(output_filename,outstream);
   for (int i=0; i<first_node_ID.size(); i++)
   {
      int id1=first_node_ID[i];
      int id2=second_node_ID[i];
      int m=nmatches[i];
      
      if (component[id1]==0 && component[id2]==0)
      {
         outstream << first_node_ID[i] << " "
                   << second_node_ID[i] << " "
                   << nmatches[i] << endl;
      }
      else if (component[id1] == 0 && component[id2] != 0)
      {
         cout << "Error!: component[id1] = " << component[id1]
              << " component[id2] = " << component[id2] << endl;
         cout << "id1 = " << id1 << " id2 = " << id2 << endl;
      }
      else if (component[id1] != 0 && component[id2] == 0)
      {
         cout << "Error!: component[id1] = " << component[id1]
              << " component[id2] = " << component[id2] << endl;
         cout << "id1 = " << id1 << " id2 = " << id2 << endl;
      }
   }
   filefunc::closefile(output_filename,outstream);


   cout << "n0 = " << n0 << " n1 = " << n1 << endl;
   sort(component.begin(),component.end());
   templatefunc::printVector(component);
   vector<int> n_members;

   int prev_c=0;
   int n_counter=0;
   for (int i=0; i<component.size(); i++)
   {
      int curr_c=component[i];
      if (curr_c==prev_c)
      {
         n_counter++;
      }
      else
      {
         n_members.push_back(n_counter);
         n_counter=0;
         prev_c=curr_c;
      }
   } // loop over index i
  
   sort(n_members.begin(),n_members.end());


   string sorted_components_filename="sorted_components.dat";

   filefunc::openfile(sorted_components_filename,outstream);
   int n_total_nodes=0;
   for (int i=0; i<n_members.size(); i++)
   {
      outstream << i << " " << n_members[i] << endl;
      n_total_nodes += n_members[i];
   }
   filefunc::closefile(sorted_components_filename,outstream);

   cout << "n_total_nodes = " << n_total_nodes << endl;
   
}

