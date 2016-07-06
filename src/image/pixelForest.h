// ==========================================================================
// Header file for pixelForest class
// ==========================================================================
// Last modified on 7/22/12
// ==========================================================================

#ifndef PIXELFOREST_H
#define PIXELFOREST_H

#include <vector>
#include "image/pixelNode.h"


class pixelForest
{

  public:

   typedef std::map<int,pixelNode* > PIXELNODES_MAP;

// Independent var: pixelNode integer ID
// Dependent var: pointer to pixelNode


   pixelForest(int xdim,int ydim);
   pixelForest(const pixelForest& pF);
   ~pixelForest();
   void purge_pixelNodes();

   pixelForest& operator= (const pixelForest& pF);
   friend std::ostream& operator<< 
      (std::ostream& outstream,pixelForest& pF);

// Set and get methods:

   int get_n_trees() const;
   int get_n_pixelNodes() const;
   void compute_four_neighbor_pixel_IDs(int px,int py);
   std::vector<int>& get_four_neighbor_pixel_IDs();
   const std::vector<int>& get_four_neighbor_pixel_IDs() const;
   

// RootNode generation & iteration member functions:

   void add_rootNode_ptr(pixelNode* rootNode_ptr);
   pixelNode* reset_rootNodes_map_ptr();
   pixelNode* get_next_rootNode_ptr();

// PixelNode retrieval member functions:

   void add_pixelNode_ptr(pixelNode* pixelNode_ptr);
   pixelNode* get_pixelNode_ptr(int ID);
   pixelNode* reset_pixelNodes_map_ptr();
   pixelNode* get_next_pixelNode_ptr();

// Recursive tree member functions:

   int recursively_count_nodes(pixelNode* rootNode_ptr);
   void recursively_count_nodes(pixelNode* input_pixelNode_ptr,int& n_nodes);
   void erase_rootNode(pixelNode* rootNode_ptr);
   void recursively_destroy_pixelNodes(pixelNode* rootNode_ptr);
   void recursively_destroy_pixel_nodes(pixelNode* input_pixelNode_ptr);
   void recursively_print_pixelNodes(
      std::ostream& outstream,pixelNode* input_pixelNode_ptr);

// Union-find algorithm member functions:

   void MakeSet(pixelNode* pixelnode_ptr);
   pixelNode* Find(pixelNode* pixelnode_ptr);
   pixelNode* Link(pixelNode* pixelnode1_ptr,pixelNode* pixelnode2_ptr);

  private: 

   int xdim,ydim;
   std::vector<int> four_neighbor_pixel_IDs;

   PIXELNODES_MAP* pixelNodes_map_ptr;
   PIXELNODES_MAP::iterator pixelNode_iter;

   PIXELNODES_MAP* rootNodes_map_ptr;
   PIXELNODES_MAP::iterator rootNode_iter;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const pixelForest& pl);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline std::vector<int>& pixelForest::get_four_neighbor_pixel_IDs()
{
   return four_neighbor_pixel_IDs;
}
inline const std::vector<int>& pixelForest::get_four_neighbor_pixel_IDs() const
{
   return four_neighbor_pixel_IDs;
}

#endif  // pixelForest.h
