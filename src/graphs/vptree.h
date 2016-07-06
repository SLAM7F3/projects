// ==========================================================================
// Header file for Vantage Point vptree class.  This data structure is
// based upon the paper "Data Structures and Algorithms for Nearest
// Neighbor Search in General Metric Spaces" by Peter Yianilos.  See
// also "What is a good nearest neighbors algorithm for finding similar
// patches in images" by N. Jumar, L. Zhang and S. Nayar.
// ==========================================================================
// Last modified on 3/20/12; 9/1/12; 4/29/13; 5/31/13
// ==========================================================================

#ifndef VPTREE_H
#define VPTREE_H

#include <queue>
#include <vector>
#include "datastructures/BinaryTree.h"
#include "datastructures/descriptor.h"
#include "numrec/nrfuncs.h"
#include "graphs/samet_comparison.h"

class descriptor;

class vptree
{

  public:

   struct node_payload
   {
         descriptor* metric_space_element_ptr;
         double mu;
         double left_min_dist,left_max_dist,right_min_dist,right_max_dist;
   };

   typedef node_payload DATA;
   typedef BinaryTreeNode<DATA> BTreeNode;
   typedef BinaryTree<DATA> BTree;

   typedef std::pair<descriptor*,descriptor*> descriptor_pair;

   vptree();
   vptree(const vptree& v);
   ~vptree();
   vptree& operator= (const vptree& v);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const vptree& v);

// Set and get methods:

   void set_hamming_distance_flag(bool flag);
   void set_KL_distance_flag(bool flag);
   void set_sqrd_Euclidean_distance_flag(bool flag);
   double get_tau() const;

// Vantage point tree construction methods:

   void construct_tree(const std::vector<descriptor*>& metric_space_elements);
   void construct_tree(const std::vector<descriptor_pair>& feature_pairs);

   BTreeNode* build_vp_tree(
      int curr_level,const std::vector<descriptor*>& metric_space_elements,
      int& prev_used_ID);
   descriptor* select_vp(const std::vector<descriptor*>& metric_space_elements);
   std::vector<descriptor*> randomly_select_metric_space_elements(
      const std::vector<descriptor*>& metric_space_elements);
   descriptor* select_random_vp(
      const std::vector<descriptor*>& metric_space_elements);
   descriptor* select_centroid_vp(
      const std::vector<descriptor*>& metric_space_elements);

// Vantage point tree distance methods:

   double distance_between_elements(
      descriptor* element1_ptr,descriptor* element2_ptr);
   double Euclidean_distance_between_elements(
      descriptor* element1_ptr,descriptor* element2_ptr);
   double sqrd_Euclidean_distance_between_elements(
      descriptor* element1_ptr,descriptor* element2_ptr);
   double hamming_distance_between_elements(
      descriptor* element1_ptr,descriptor* element2_ptr);
   double KL_distance_between_elements(
      descriptor* element1_ptr,descriptor* element2_ptr);
   void compute_extremal_left_subspace_distances(
      BTreeNode* BinaryTreeNode_ptr,double& mu_min,double& mu_max);
   void compute_extremal_right_subspace_distances(
      BTreeNode* BinaryTreeNode_ptr,double& mu_min,double& mu_max);
   void compute_all_extremal_subspace_distances();
   double median_separation_distance(
      descriptor* p_ptr,const std::vector<descriptor*>& metric_space_elements);
   double median_separation_distance(
      descriptor* p_ptr,const std::vector<descriptor*>& metric_space_elements,
      std::vector<double>& distances);
   void compute_distances_to_nodes(
      descriptor* query_element_ptr,
      const std::vector<int>& nearest_neighbor_node_IDs,
      std::vector<double>& query_to_neighbor_distances,
      std::vector<descriptor*>& metric_space_element_ptrs);
      
// Vantage point tree display methods:

   void print_vp_tree();
   std::vector<std::string> extract_vp_node_info(BTreeNode* node_ptr);

// Vantage point tree search methods:

   descriptor* find_closest_node(descriptor* query_element_ptr);
   void search_vp_tree(
      BTreeNode* BinaryTreeNode_ptr,descriptor* query_element_ptr);
   void incrementally_find_nearest_nodes(
      int k,descriptor* query_element_ptr,
      std::vector<int>& nearest_neighbor_node_IDs,
      std::vector<double>& query_to_neighbor_distances,
      std::vector<descriptor*>& metric_space_element_ptrs);

  private: 

   bool hamming_distance_flag,KL_distance_flag,sqrd_Euclidean_distance_flag;
   int best_node_ID;
   double tau;
   std::vector<double> log_values;
   BTree* BinaryTree_ptr;
   std::priority_queue<threevector,std::vector<threevector>,samet_comparison>* 
      search_queue_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const vptree& v);
   void store_logarithm_values();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void vptree::set_hamming_distance_flag(bool flag)
{
   hamming_distance_flag=flag;
}

inline void vptree::set_KL_distance_flag(bool flag)
{
   KL_distance_flag=flag;
}

inline void vptree::set_sqrd_Euclidean_distance_flag(bool flag)
{
   sqrd_Euclidean_distance_flag=flag;
}

inline double vptree::get_tau() const
{
   return tau;
}

// --------------------------------------------------------------------------
// Method select_random_vp() 

inline descriptor* vptree::select_random_vp(
   const std::vector<descriptor*>& metric_space_elements)
{
   return metric_space_elements[
      metric_space_elements.size()*nrfunc::ran1()];
}

// --------------------------------------------------------------------------
inline double vptree::sqrd_Euclidean_distance_between_elements(
   descriptor* element1_ptr,descriptor* element2_ptr)
{
   return element1_ptr->sqrd_distance_to_another_descriptor(element2_ptr);
}


#endif  // vptree.h
