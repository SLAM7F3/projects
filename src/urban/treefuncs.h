// ==========================================================================
// Header file for TREEFUNCS namespace
// ==========================================================================
// Last modified on 4/17/05; 7/29/06
// ==========================================================================

#ifndef TREEFUNCS_H
#define TREEFUNCS_H

#include <iostream>
#include <string>
#include "kdtree/kdtree.h"
#include "network/Network.h"
#include "math/threevector.h"

class tree_cluster;

namespace treefunc
{

// Network *trees_network_ptr holds trees cluster network information:

   extern Network<tree_cluster*>* trees_network_ptr;

// ---------------------------------------------------------------------

   Network<tree_cluster*>* generate_trees_network(
      std::string imagedir,double min_footprint_area,
      twoDarray const *ztwoDarray_ptr,
      twoDarray const *features_twoDarray_ptr);
   void delete_trees_network(Network<tree_cluster*>* curr_trees_network_ptr);
   void draw_tree_cluster_pixels(
      Network<tree_cluster*> const *curr_trees_network_ptr,
      twoDarray const *ztwoDarray_ptr,twoDarray* ztree_twoDarray_ptr,
      twoDarray* ftree_twoDarray_ptr);
   KDTree::KDTree<3, threevector>* generate_tree_network_kdtree(
      Network<tree_cluster*> const *curr_tree_network_ptr);
   void tree_points_near_input_location(
      const threevector& xy_pnt,double rho,
      KDTree::KDTree<3, threevector> const *kdtree_ptr,
      std::vector<threevector>& nearby_tree_points);

// Trees network text input/output methods:

   void output_trees_network_to_textfile(
      Network<tree_cluster*> const *trees_network_ptr,
      std::string output_filename);
   void output_treecluster(
      Network<tree_cluster*> const *trees_network_ptr,int r,
      std::ofstream& outstream);
   Network<tree_cluster*>* readin_trees_network_from_textfile(
      std::string input_filename,bool vertices_lie_in_plane);

} // treefunc namespace

// ==========================================================================
// Inlined methods:
// ==========================================================================

#endif // treefuncs.h



