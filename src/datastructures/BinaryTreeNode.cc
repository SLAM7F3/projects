// ==========================================================================
// Templatized BinaryTreeNode class member function definitions
// ==========================================================================
// Last modified on 3/16/12; 3/17/12
// ==========================================================================

#include "templates/mytemplates.h"

// ==========================================================================
// Initialization, constructor and destructor methods:
// ==========================================================================

template <class T> void BinaryTreeNode<T>::allocate_member_objects()
{
}

template <class T> void BinaryTreeNode<T>::initialize_member_objects()
{
   ID=level=parent_ID=-1;
   n_descendants=0;
   gx=gy=-1;
   LeftChild_ptr=NULL;
   RightChild_ptr=NULL;
}

template <class T> BinaryTreeNode<T>::BinaryTreeNode()
{
   allocate_member_objects();
   initialize_member_objects();
}

template <class T> BinaryTreeNode<T>::BinaryTreeNode(int id,int l,int p_id)
{
   allocate_member_objects();
   initialize_member_objects();
   ID=id;
   level=l;
   parent_ID=p_id;
}

// Copy constructor:

template <class T> BinaryTreeNode<T>::BinaryTreeNode(
   const BinaryTreeNode<T>& node)
{
   allocate_member_objects();
   initialize_member_objects();
   docopy(node);
}

template <class T> BinaryTreeNode<T>::~BinaryTreeNode()
{
}

// ---------------------------------------------------------------------
template <class T> void BinaryTreeNode<T>::docopy(
   const BinaryTreeNode<T>& node)
{
   ID=node.ID;
   level=node.level;
   n_descendants=node.n_descendants;
   data=node.data;
}

// Overload = operator:

template <class T> BinaryTreeNode<T>& BinaryTreeNode<T>::operator= 
(const BinaryTreeNode<T>& n)
{
   if (this==&n) return *this;
   docopy(n);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

template <class T> std::ostream& operator<< 
(std::ostream& outstream,const BinaryTreeNode<T>& node)
{
   outstream << std::endl;
   outstream << "ID = " << node.ID << std::endl;
   outstream << "level = " << node.level << std::endl;
//   outstream << "n_descendants = " << node.n_descendants << std::endl;
   outstream << "gx = " << node.gx << " gy = " << node.gy << std::endl;

   if (node.LeftChild_ptr != NULL)
   {
      outstream << "Left Child ID = " << node.LeftChild_ptr->get_ID() 
                << std::endl;
   }
   if (node.RightChild_ptr != NULL)
   {
      outstream << "Right Child ID = " << node.RightChild_ptr->get_ID() 
                << std::endl;
   }

//   outstream << "data = " << node.get_data() << std::endl;
   
   return outstream;
}

// ==========================================================================
// Child node generation member functions:
// ==========================================================================

template <class T> BinaryTreeNode<T>* BinaryTreeNode<T>::GenerateLeftChild(
   int id)
{
//   std::cout << "inside BinaryTreenode::GenerateLeftChild(), child ID = "
//             << id << std::endl;

   int child_level=get_level()+1;
   int parent_ID=get_ID();
//   std::cout << "current parent ID = " << parent_ID << std::endl;

   LeftChild_ptr=new BinaryTreeNode<T>(id,child_level,parent_ID);
   return LeftChild_ptr;
}

// ---------------------------------------------------------------------
template <class T> BinaryTreeNode<T>* BinaryTreeNode<T>::GenerateRightChild(
   int id)
{
//   std::cout << "inside BinaryTreenode::GenerateRightChild(), child ID = "
//             << id << std::endl;

   int child_level=get_level()+1;
   int parent_ID=get_ID();
//   std::cout << "current parent ID = " << parent_ID << std::endl;

   RightChild_ptr=new BinaryTreeNode<T>(id,child_level,parent_ID);
   return RightChild_ptr;
}

// ---------------------------------------------------------------------
// Member function GetDescendantsAndSelf() returns an STL vector
// containing pointers to all descendant nodes of the current
// BinaryTreeNode.  The final pointer within the STL vector points to
// the original BinaryTreeNode with which this recursive function was
// called.

template <class T> void BinaryTreeNode<T>::GetDescendantsAndSelf(
   std::vector<BinaryTreeNode<T>*>& DescendantNode_ptrs)
{
//   std::cout << "inside BinaryTreenode::GetDescendants(), curr node ID = "
//             << get_ID() << std::endl;

   BinaryTreeNode<T>* LeftChild_ptr=get_LeftChild_ptr();
   if (LeftChild_ptr != NULL)
   {
      LeftChild_ptr->GetDescendantsAndSelf(DescendantNode_ptrs);
   }
   BinaryTreeNode<T>* RightChild_ptr=get_RightChild_ptr();
   if (RightChild_ptr != NULL)
   {
      RightChild_ptr->GetDescendantsAndSelf(DescendantNode_ptrs);
   }
   DescendantNode_ptrs.push_back(this);
}

   
