// ==========================================================================
// TERRAINFUNCS stand-alone methods
// ==========================================================================
// Last modified on 12/1/10; 12/2/10; 12/5/10; 5/23/11
// ==========================================================================

#include <iostream>
#include "graphs/graph.h"
#include "graphs/node.h"
#include "image/terrainfuncs.h"
#include "image/TwoDarray.h"

using std::cout;
using std::endl;
using std::flush;
using std::pair;
using std::vector;

namespace terrainfunc
{

// Method px_py_to_node_ID returns the single integer ID for a DTED graph
// node corresponding height map *ztwoDarray_ptr.

   int px_py_to_node_ID(int mdim,int px,int py)
   {
      int node_ID=py*mdim+px;
      return node_ID;
   }

// Method node_ID_to_px_py returns the pair of integers (px,py)
// corresponding to input node_ID for a DTED graph.

   pair<int,int> node_ID_to_px_py(int mdim,int node_ID)
   {
      int py=node_ID/mdim;
      int px=node_ID%mdim;
      return pair<int,int>(px,py);
   }
   
// --------------------------------------------------------------------------
// Method compute_shortest_path() takes in some destination node
// within a DTED graph for which a Dijkstra field is assumed to have
// already been calculated.  This method traces the shortest path back
// to the initial node used to compute the Dijkstra DTED field.  It
// then fills and returns an STL vector with threevectors
// corresponding to waypoints along the shortest Dijkstra path.

   vector<threevector> compute_shortest_path(
      node* destination_node_ptr,const twoDarray* ztwoDarray_ptr)
   {
      vector<threevector> waypoints;

      node* previous_node_ptr = destination_node_ptr;
      cout << "Distance from starting node: " 
           << destination_node_ptr->get_distance_from_start() << endl;
      while (previous_node_ptr != NULL)
      {
         int node_ID=previous_node_ptr->get_ID();
         pair<int,int> p=node_ID_to_px_py(ztwoDarray_ptr->get_mdim(),node_ID);
         
         int px=p.first;
         int py=p.second;
         double x,y;
         ztwoDarray_ptr->pixel_to_point(px,py,x,y);
         double z=ztwoDarray_ptr->get(px,py);

         waypoints.push_back(threevector(x,y,z));

         cout.precision(8);
         cout << "Node=" << node_ID
              << " px=" << px << " py=" << py 
              << " x=" << x << " y=" << y << " z=" << z << endl;

         previous_node_ptr = previous_node_ptr->get_path_predecessor_ptr();
      }
      cout << endl;
      return waypoints;
   }

// --------------------------------------------------------------------------
// Method generate_Dijkstra_DTED_graph() takes in *ztwoDarray_ptr
// which is assumed to correspond to a DTED height map.  It
// dynamically generates a DTED graph object and instantiates nodes
// corresponding to a gridded subset of pixels in *ztwoDarray_ptr.
// For each DTED graph node, this method computes a cost to move to
// each neighboring lattice node based upon a cost function involving
// parameter alpha.  The dynamic DTED graph with its nodes and edges
// is returned by this method.

   graph* generate_Dijkstra_DTED_graph(
      const twoDarray* ztwoDarray_ptr,double alpha,int skip)
   {
      cout << "inside terrainfunc::generate_Dijkstra_DTED_graph()" << endl;
//      cout << "*ztwoDarray_ptr = " << *ztwoDarray_ptr << endl;

      graph* graph_ptr=new graph();

      int mdim=ztwoDarray_ptr->get_mdim();
      int ndim=ztwoDarray_ptr->get_ndim();

// First instantiate nodes within graph corresponding to some subset
// of pixels in *ztwoDarray_ptr:

      for (int px=0; px<mdim; px += skip)
      {
         if (px%100==0) cout << px/100 << " " << flush;
         for (int py=0; py<ndim; py += skip)
         {
            int node_ID=px_py_to_node_ID(mdim,px,py);
            node* node_ptr=new node(node_ID);
            graph_ptr->add_node(node_ptr);
         } // loop over py index
      } // loop over px index
      cout << endl;

      cout << "N_nodes = " << graph_ptr->get_n_nodes() << endl;
      
// Next loop over all pixels not located on outermost perimeter of
// *ztwoDarray_ptr.  

      double x,y,neighbor_x,neighbor_y;
      for (int px=skip; px<mdim-skip; px += skip)
      {
         if (px%100==0) cout << px/100 << " " << flush;
         for (int py=skip; py<ndim-skip; py += skip)
         {
            int node_ID=px_py_to_node_ID(mdim,px,py);

            ztwoDarray_ptr->pixel_to_point(px,py,x,y);
            double z=ztwoDarray_ptr->get(px,py);

            for (int j=-1; j<=1; j++)
            {
               int qx=px+j*skip;
               for (int k=-1; k<=1; k++)
               {
                  if (j==0 && k==0) continue;
                  int qy=py+k*skip;
                  int neighbor_node_ID=px_py_to_node_ID(mdim,qx,qy);

// Set edge weight between node and neighbor_node using functional
// form suggested in eqn 2 of "Least-cost paths in mountainous
// terrain" Computers & Geosciences 30 (2004) 203-209 by W.G. Rees:

                  ztwoDarray_ptr->pixel_to_point(qx,qy,neighbor_x,neighbor_y);

//                  cout << "x = " << x << " neighbor_x = " << neighbor_x
//                       << endl;
//                  cout << "y = " << y << " neighbor_y = " << neighbor_y 
//                       << endl;
                  double neighbor_z=ztwoDarray_ptr->get(qx,qy);
                  double ds=sqrt(sqr(neighbor_x-x)+sqr(neighbor_y-y));
//                  cout << "ds = " << ds << endl;
                  double dz=neighbor_z-z;
                  double step_cost=ds+alpha*sqr(dz)/ds;

//                  node* neighbor_node_ptr=graph_ptr->get_node_ptr(
//                     neighbor_node_ID);

//                  cout << "node_ID = " << node_ID
//                       << " neighbor ID = " << neighbor_node_ID
//                       << " step cost = " << step_cost << endl;

                  graph_ptr->add_graph_edge(
                     node_ID,neighbor_node_ID,step_cost);
                  
               } // loop over index k
            } // loop over index j
         } // loop over py index
      } // loop over px index
      cout << endl;

      cout << "N_edges = " << graph_ptr->get_n_graph_edges() << endl;

      return graph_ptr;
   }
   
   
} // terrainfunc namespace




