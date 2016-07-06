// =========================================================================
// Header file for stand-alone K-D tree functions
// =========================================================================
// Last modified on 2/20/06; 6/18/06; 7/29/06; 2/10/08; 6/30/08
// =========================================================================

#ifndef KDTREEFUNCS_H
#define KDTREEFUNCS_H

#include <iostream>
#include <vector>
#include "datastructures/Databin.h"
#include "math/fourvector.h"
#include "kdtree/kdtree.h"
#include "math/threevector.h"

namespace kdtreefunc
{

// Planar segmentation methods:

   threevector quadtree_midpoint(int l,int i,double theta=0);
   template <class T> inline void generate_quadtree(
      int nlevels,KDTree::KDTree<2,Databin<T> >* kdtree_ptr,double theta=0);

// Closest node methods:

   void sorted_nodes_within_range(
      KDTree::KDTree<1, twovector>* kdtree_ptr,
      const twovector& x_posn,double rho,std::vector<twovector>& node);
   void sorted_nodes_within_range(
      KDTree::KDTree<2, threevector>* kdtree_ptr,
      const threevector& xy_posn,double rho,std::vector<threevector>& node);
   void sorted_nodes_within_range(
      KDTree::KDTree<3, fourvector>* kdtree_ptr,
      const fourvector& xyz_posn,double rho,std::vector<fourvector>& node,
      int __K=3);

   void find_closest_nodes(
      KDTree::KDTree<1, twovector>* kdtree_ptr,const twovector& x_posn,
      double rho,unsigned int n_closest_nodes,
      std::vector<twovector>& closest_node,int n_max_search_iters=20);
   void find_closest_nodes(
      KDTree::KDTree<2, threevector>* kdtree_ptr,const threevector& xy_posn,
      double rho,unsigned int n_closest_nodes,
      std::vector<threevector>& closest_node,int n_max_search_iters=20);
   void find_closest_nodes(
      KDTree::KDTree<3, fourvector>* kdtree_ptr,const fourvector& xyz_posn,
      double rho,unsigned int n_closest_nodes,
      std::vector<fourvector>& closest_node,int n_max_search_iters=20,
      int __K=3);

// ==========================================================================
// Templatized methods:
// ==========================================================================

// Methods generate_1D_kdtree, generate_2D_kdtree and
// generate_3D_kdtree take in an STL vector V of some vector type
// objects.  They return pointers to dynamically generated KDtrees
// that contain the input spatial information.

   template <class T> KDTree::KDTree<1,T>* generate_1D_kdtree(
      std::vector<T>& V)
      {
         KDTree::KDTree<1, T>* kdtree_ptr=new KDTree::KDTree<1,T>;

         for (unsigned int n=0; n<V.size(); n++)
         {
            kdtree_ptr->insert(V[n]);
         }

         kdtree_ptr->optimize();
         return kdtree_ptr;
      }

   template <class T> KDTree::KDTree<2,T>* generate_2D_kdtree(
      std::vector<T>& V)
      {
         KDTree::KDTree<2, T>* kdtree_ptr=new KDTree::KDTree<2,T>;

         for (unsigned int n=0; n<V.size(); n++)
         {
            kdtree_ptr->insert(V[n]);
         }

         kdtree_ptr->optimize();
         return kdtree_ptr;
      }

   template <class T> KDTree::KDTree<3,T>* generate_3D_kdtree(
      std::vector<T>& V)
      {
         KDTree::KDTree<3, T>* kdtree_ptr=new KDTree::KDTree<3,T>;

         for (unsigned int n=0; n<V.size(); n++)
         {
            kdtree_ptr->insert(V[n]);
         }

         kdtree_ptr->optimize();
         return kdtree_ptr;
      }

// As of 6/18/06, we unfortunately cannot get the following method
// which takes dimension K as an input parameter to correctly
// compile...

/*
   template <size_t const __K, class T> 
      KDTree::KDTree<__K,T>* generate_kdtree(int K,std::vector<T>& V)
      {
         if (K==2)
         {
            return generate_2D_kdtree(V);
         }
         else if (K==3)
         {
            return generate_3D_kdtree(V);
         }
         else
         {
            std::cout << "Error in kdtreefunc::generate_kdtree() !" 
                      << std::endl;
            std::cout << "K = " << K << " lies outside implemented range"
                      << std::endl;
            exit(-1);
         }
      }
*/

// --------------------------------------------------------------------------
// Method generate_quadtree fills a quad tree with nodes whose
// independent variables range over the square -1 <= X,Y <= 1.  The
// quad tree information is returned within the input KDTree data
// structure.
   
   template <class T> inline void generate_quadtree(
      int nlevels,KDTree::KDTree<2,Databin<T> >* kdtree_ptr,double theta)
      {

// First generate an STL vector filled with quadtree bins for KDtree
// levels 0 through nlevels-1.  The bins' centers evenly partition the
// square -1 <= x,y, <= 1:

         std::vector<Databin<T> > V;
         for (int l=0; l<=nlevels; l++)
         {
            for (int i=0; i<mathfunc::power_of_two(l); i++)
            {
               Databin<T> curr_bin(V.size(),quadtree_midpoint(l,i,theta));
               V.push_back(curr_bin);
               std::cout << "Quadtree level = " << l 
                         << " Midpoint counter = " << i 
                         << " Midpoint = " << curr_bin << std::endl;
            } // loop over index i labeling KDnodes within level l
         } // loop over index l labeling KDTree level

         for (unsigned int n=0; n<V.size(); n++)
         {
//            std::cout << "n = " << n << " V[n] = " << V[n] << std::endl;
            KDTree::_Iterator<Databin<T>, const Databin<T>&, 
               const Databin<T>*> curr_iterator=kdtree_ptr->insert(V[n]);
//            std::cout << "curr_iterator.get_node_ptr() = " 
//                      << curr_iterator.get_node_ptr() << std::endl;
//            Databin<T> curr_data_point(*curr_iterator);
//            cout << "curr_data_point = " << curr_data_point << endl;
         }
//         std::cout << "Quadtree = " << *kdtree_ptr << std::endl;
      }

} // kdtreefunc namespace

#endif // kdtreefuncs.h




