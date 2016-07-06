// ==========================================================================
// Header file for pixelNode class
// ==========================================================================
// Last modified on 7/22/12
// ==========================================================================

#ifndef PIXELNODE_H
#define PIXELNODE_H

#include <map>
#include <set>
#include <vector>
#include "pool/objpool.h"

class pixelNode: public ObjectPool< pixelNode >
{

  public:

   typedef std::map<int,pixelNode* > PIXELNODES_MAP;
// Indep var: pixelNode integer ID
// Depend var: pointer to pixelNode


   pixelNode();
   pixelNode(int px,int py,int ID);
   pixelNode(const pixelNode& pl);
   ~pixelNode();
   pixelNode& operator= (const pixelNode& pN);
   friend std::ostream& operator<< 
      (std::ostream& outstream,pixelNode& pN);

// Set and get methods:

   int get_px() const;
   int get_py() const;
   int get_ID() const;
   void set_intensity(double z);
   int get_intensity() const;
   void set_rank(int r);
   int get_rank() const;

// Parent member functions:

   void set_parent_node_ptr(pixelNode* parent_node_ptr);
   pixelNode* get_parent_node_ptr();
   const pixelNode* get_parent_node_ptr() const;

// Children member functions:

   bool isChild(pixelNode* child_node_ptr);
   void addChild(pixelNode* child_node_ptr);
   void deleteChild(pixelNode* child_node_ptr);
   int getNumChildren() const;

   PIXELNODES_MAP* get_child_nodes_map_ptr()
   {
      return child_nodes_map_ptr;
   }

   const PIXELNODES_MAP* get_child_nodes_map_ptr() const
   {
      return child_nodes_map_ptr;
   }

   pixelNode* reset_child_pixelNode_ptr();
   pixelNode* get_next_child_pixelNode_ptr();
   pixelNode* get_rootnode_ptr();

//   std::vector<int> get_four_neighbor_IDs();

  private: 

   int px,py,ID,rank;
   double intensity;
   pixelNode* parent_node_ptr;
   PIXELNODES_MAP* child_nodes_map_ptr;
   PIXELNODES_MAP::iterator child_pixelNode_iter;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const pixelNode& pl);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline int pixelNode::get_px() const
{
   return px;
}

inline int pixelNode::get_py() const
{
   return py;
}

inline int pixelNode::get_ID() const
{
   return ID;
}

inline void pixelNode::set_intensity(double z)
{
   intensity=z;
}

inline int pixelNode::get_intensity() const
{
   return intensity;
}

inline void pixelNode::set_rank(int r)
{
   rank=r;
}

inline int pixelNode::get_rank() const
{
   return rank;
}



#endif  // pixelNode.h
