// ==========================================================================
// Header file for templatized BinaryTreeNode class
// ==========================================================================
// Last modified on 3/16/12; 3/17/12
// ==========================================================================

#ifndef T_BINARYTREENODE_H
#define T_BINARYTREENODE_H

#include <list>
#include "pool/objpool.h"
#include "math/threevector.h"

template <class T>
class BinaryTreeNode: public ObjectPool< BinaryTreeNode<T> >
{

  public:

// Initialization, constructor and destructor functions:

   BinaryTreeNode();
   BinaryTreeNode(int id,int l,int p_id);
   BinaryTreeNode(const BinaryTreeNode<T>& node);
   virtual ~BinaryTreeNode();
   BinaryTreeNode<T>& operator= (const BinaryTreeNode<T>& node);

   template <class T1>
   friend std::ostream& operator<< 
      (std::ostream& outstream,const BinaryTreeNode<T1>& node);

// Set and get member functions:
   
   void set_ID(int id);
   int get_ID() const;
   void set_parent_ID(int id);
   int get_parent_ID() const;
   void set_level(int l);
   int get_level() const;
   void set_n_descendants(int n);
   int get_n_descendants() const;

   void set_gx(double gx);
   double get_gx() const;
   void set_gy(double gy);
   double get_gy() const;

   void set_data(T const & d);
   T& get_data(); 
   const T& get_data() const; 

   void set_LeftChild_ptr(BinaryTreeNode<T>* node_ptr);
   BinaryTreeNode<T>* get_LeftChild_ptr();
   const BinaryTreeNode<T>* get_LeftChild_ptr() const;

   void set_RightChild_ptr(BinaryTreeNode<T>* node_ptr);
   BinaryTreeNode<T>* get_RightChild_ptr();
   const BinaryTreeNode<T>* get_RightChild_ptr() const;

// Children node generation member functions:

   BinaryTreeNode<T>* GenerateLeftChild(int id);
   BinaryTreeNode<T>* GenerateRightChild(int id);

   void GetDescendantsAndSelf(
      std::vector<BinaryTreeNode<T>*>& DescendantNode_ptrs);

  private:

   int ID,level,parent_ID,n_descendants;
   double gx,gy;	// Node coords which range from 0 to 1
   T data;		// Arbitrary data object

   BinaryTreeNode<T>* LeftChild_ptr;
   BinaryTreeNode<T>* RightChild_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const BinaryTreeNode<T>& node);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

template <class T> inline void BinaryTreeNode<T>::set_ID(int id)
{
   ID=id;
}

template <class T> inline int BinaryTreeNode<T>::get_ID() const
{
   return ID;
}

template <class T> inline void BinaryTreeNode<T>::set_parent_ID(int id)
{
   parent_ID=id;
}

template <class T> inline int BinaryTreeNode<T>::get_parent_ID() const
{
   return parent_ID;
}

template <class T> inline void BinaryTreeNode<T>::set_level(int l)
{
   level=l;
}

template <class T> inline int BinaryTreeNode<T>::get_level() const
{
   return level;
}

template <class T> inline void BinaryTreeNode<T>::set_n_descendants(int n)
{
   n_descendants=n;
}

template <class T> inline int BinaryTreeNode<T>::get_n_descendants() const
{
   return n_descendants;
}

template <class T> inline void BinaryTreeNode<T>::set_gx(double gx)
{
   this->gx=gx;
}

template <class T> inline double BinaryTreeNode<T>::get_gx() const
{
   return gx;
}

template <class T> inline void BinaryTreeNode<T>::set_gy(double gy)
{
   this->gy=gy;
}

template <class T> inline double BinaryTreeNode<T>::get_gy() const
{
   return gy;
}

template <class T> inline void BinaryTreeNode<T>::set_data(T const & d)
{
   data=d;
}

template <class T> inline T& BinaryTreeNode<T>::get_data() 
{
   return data;
}

template <class T> inline const T& BinaryTreeNode<T>::get_data() const
{
   return data;
}

template <class T> 
inline void BinaryTreeNode<T>::set_LeftChild_ptr(BinaryTreeNode<T>* node_ptr)
{
   LeftChild_ptr=node_ptr;
}

template <class T> inline BinaryTreeNode<T>* 
BinaryTreeNode<T>::get_LeftChild_ptr()
{
   return LeftChild_ptr;
}

template <class T> inline const BinaryTreeNode<T>* 
BinaryTreeNode<T>::get_LeftChild_ptr() const
{
   return LeftChild_ptr;
}

template <class T> 
inline void BinaryTreeNode<T>::set_RightChild_ptr(BinaryTreeNode<T>* node_ptr)
{
   RightChild_ptr=node_ptr;
}

template <class T> inline BinaryTreeNode<T>* 
BinaryTreeNode<T>::get_RightChild_ptr()
{
   return RightChild_ptr;
}

template <class T> inline const BinaryTreeNode<T>* 
BinaryTreeNode<T>::get_RightChild_ptr() const
{
   return RightChild_ptr;
}


#include "datastructures/BinaryTreeNode.cc"

#endif  // T_datastructures/BinaryTreeNode.h



