// ==========================================================================
// KDTREEFUNCS stand-alone methods
// ==========================================================================
// Last modified on 2/10/08; 6/30/08; 12/4/10
// ==========================================================================

#include "math/constant_vectors.h"
#include "kdtree/kdtreefuncs.h"
#include "math/mathfuncs.h"
#include "templates/mytemplates.h"

using std::cin;
using std::cout;
using std::endl;
using std::vector;

namespace kdtreefunc
{

// ==========================================================================
// Planar segmentation methods
// ==========================================================================

// Method quadtree_midpoint takes in integer l which labels the
// KDtree's level as well as integer i.  As index i ranges from 0 to
// 2**l-1, this method returns points within the square -1 <= x,y <=
// +1 that evenly segment the square into 2**l regions.

   threevector quadtree_midpoint(int l,int i,double theta)
      {
         double cos_theta=cos(theta);
         double sin_theta=sin(theta);
         if (l==0)
         {
            return threevector(0,0);
         }
         else
         {
            int k=modulo(l+1,2);	
            int j=(l-k-1)/2;

            int sgn=1;
            if (i%2==0) sgn=-1;

            threevector new_offset;
            if (k==0)
            {
               new_offset=cos_theta*x_hat+sin_theta*y_hat;
            }
            else if (k==1)
            {
               new_offset=-sin_theta*x_hat+cos_theta*y_hat;
            }
            new_offset *= sgn;
            new_offset /= mathfunc::power_of_two(j+1);
            return quadtree_midpoint(l-1,i/2)+new_offset;
         }
      }

// ==========================================================================
// Closest node methods
// ==========================================================================

// Method sorted_nodes_within_range takes in an existing
// KDtree<2,threevector>.  It also takes in some 2D position along with a
// search radius.  This method fills output STL vector node with those
// KDtree sites which lie within r < radius of xy_posn.  The output
// nodes are arranged in increasing distance from the input position.

   void sorted_nodes_within_range(
      KDTree::KDTree<1, twovector>* kdtree_ptr,
      const twovector& x_posn,double rho,vector<twovector>& node)
      {
         kdtree_ptr->find_within_range(x_posn,rho,std::back_inserter(node));

         const int __K=1;
         vector<double> sqrd_distance;
         for (int i=0; i<int(node.size()); i++)
         {
            double curr_sqrd_dist=0;
            for (int k=0; k<__K; k++)
            {
               curr_sqrd_dist += sqr(node[i].get(k)-x_posn.get(k));
            }
            sqrd_distance.push_back(curr_sqrd_dist);
         } // loop over index i labeling nearby features
      
         templatefunc::Quicksort(sqrd_distance,node);
      }

   void sorted_nodes_within_range(
      KDTree::KDTree<2, threevector>* kdtree_ptr,
      const threevector& xy_posn,double rho,vector<threevector>& node)
      {
         kdtree_ptr->find_within_range(xy_posn,rho,std::back_inserter(node));

         const int __K=2;
         vector<double> sqrd_distance;
         for (int i=0; i<int(node.size()); i++)
         {
            double curr_sqrd_dist=0;
            for (int k=0; k<__K; k++)
            {
               curr_sqrd_dist += sqr(node[i].get(k)-xy_posn.get(k));
            }
            sqrd_distance.push_back(curr_sqrd_dist);
         } // loop over index i labeling nearby features
      
         templatefunc::Quicksort(sqrd_distance,node);
      }

   void sorted_nodes_within_range(
      KDTree::KDTree<3, fourvector>* kdtree_ptr,
      const fourvector& xyz_posn,double rho,vector<fourvector>& node,
      int __K)
      {
         kdtree_ptr->find_within_range(
            xyz_posn,rho,std::back_inserter(node),
            3-__K);

//         const int __K=3;
         vector<double> sqrd_distance;
         for (int i=0; i<int(node.size()); i++)
         {
            double curr_sqrd_dist=0;
            for (int k=0; k<__K; k++)
            {
               curr_sqrd_dist += sqr(node[i].get(k)-xyz_posn.get(k));
            }
            sqrd_distance.push_back(curr_sqrd_dist);
         } // loop over index i labeling nearby features
      
         templatefunc::Quicksort(sqrd_distance,node);
      }

// --------------------------------------------------------------------------
// Method find_closest_nodes takes in an existing
// KDtree<2,threevector>, some 2D position along with an initial
// search radius rho.  This method fills output STL vector
// closest_node with the n_closest_nodes KDtree sites that lie closest
// to xy_posn.  The output nodes are arranged in increasing distance
// from the input position.

   void find_closest_nodes(
      KDTree::KDTree<1, twovector>* kdtree_ptr,const twovector& x_posn,
      double rho,unsigned int n_closest_nodes,
      vector<twovector>& closest_node,int n_max_search_iters)
      {
         vector<twovector> nearby_node;

         int search_iter=0;
         while (nearby_node.size() < n_closest_nodes &&
                nearby_node.size() < kdtree_ptr->size() &&
                search_iter < n_max_search_iters)
         {
            nearby_node.clear();
            sorted_nodes_within_range(
               kdtree_ptr,x_posn,rho,nearby_node);
            if (nearby_node.size() >= n_closest_nodes ||
                nearby_node.size()==kdtree_ptr->size())
            {
              int n_max=basic_math::min(n_closest_nodes,
                            static_cast<unsigned int>(kdtree_ptr->size()));
                                                                      
               for (int n=0; n<n_max; n++)
               {
                  closest_node.push_back(nearby_node[n]);
               }
            }
            else
            {
               rho *= 2.0;
            }
            search_iter++;
         }
      }

   void find_closest_nodes(
      KDTree::KDTree<2, threevector>* kdtree_ptr,const threevector& xy_posn,
      double rho,unsigned int n_closest_nodes,
      vector<threevector>& closest_node,int n_max_search_iters)
      {
         vector<threevector> nearby_node;

         int search_iter=0;
         while (nearby_node.size() < n_closest_nodes &&
                nearby_node.size() < kdtree_ptr->size() &&
                search_iter < n_max_search_iters)
         {
            nearby_node.clear();
            sorted_nodes_within_range(
               kdtree_ptr,xy_posn,rho,nearby_node);
            if (nearby_node.size() >= n_closest_nodes ||
                nearby_node.size()==kdtree_ptr->size())
            {
              int n_max=basic_math::min(n_closest_nodes,
                            static_cast<unsigned int>(kdtree_ptr->size()));
                                                                      
               for (int n=0; n<n_max; n++)
               {
                  closest_node.push_back(nearby_node[n]);
               }
            }
            else
            {
               rho *= 2.0;
            }
            search_iter++;
         }
      }

   void find_closest_nodes(
      KDTree::KDTree<3, fourvector>* kdtree_ptr,const fourvector& xyz_posn,
      double rho,unsigned int n_closest_nodes,
      vector<fourvector>& closest_node,int n_max_search_iters,
      int __K)
      {
//         cout << "inside kdtreefunc::find_closest_nodes()" << endl;
//         cout << "n_closest_nodes = " << n_closest_nodes << endl;
//         cout << "n_max_search_iters = " << n_max_search_iters << endl;
//         cout << "__K = " << __K << endl;

         vector<fourvector> nearby_node;

         int search_iter=0;
         while (nearby_node.size() < n_closest_nodes &&
                nearby_node.size() < kdtree_ptr->size() &&
                search_iter < n_max_search_iters)
         {
            nearby_node.clear();
            sorted_nodes_within_range(
               kdtree_ptr,xyz_posn,rho,nearby_node,__K);
            if (nearby_node.size() >= n_closest_nodes ||
                nearby_node.size()==kdtree_ptr->size())
            {
               int n_max=basic_math::min(n_closest_nodes,
                             static_cast<unsigned int>(kdtree_ptr->size()));
               for (int n=0; n<n_max; n++)
               {
                  closest_node.push_back(nearby_node[n]);
               }
            }
            else
            {
               rho *= 2.0;
            }
            search_iter++;
         }

//         cout << "closest_node.size() = " << closest_node.size() << endl;
      }
   
} // kdtreefunc namespace
