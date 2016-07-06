// ==========================================================================
// Templatized Binary Tree class member function definitions
// ==========================================================================
// Last modified on 3/16/12; 3/17/12; 3/18/12
// ==========================================================================

// Initialization, constructor and destructor methods:

template <class T> void BinaryTree<T>::allocate_member_objects() 
{
   BinaryTreeNodes_map_ptr=new BINARYTREENODES_MAP;
}

template <class T> void BinaryTree<T>::initialize_member_objects() 
{
}

template <class T> BinaryTree<T>::BinaryTree()
{
   allocate_member_objects();
   initialize_member_objects();
}

// Copy constructor:

template <class T> BinaryTree<T>::BinaryTree(const BinaryTree<T>& t)
{
   allocate_member_objects();
   initialize_member_objects();
   docopy(t);
}

template <class T> void BinaryTree<T>::docopy(const BinaryTree<T>& t)
{
}	

// Overload = operator:

template <class T> BinaryTree<T>& BinaryTree<T>::operator= (const BinaryTree<T>& t)
{
   if (this==&t) return *this;
   docopy(t);
   return *this;
}

template <class T> BinaryTree<T>::~BinaryTree()
{
//   std::cout << "inside BinaryTree destructor" << std::endl;
   purge_nodes();
   delete BinaryTreeNodes_map_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator:

template <class T> std::ostream& operator<< 
(std::ostream& outstream,const BinaryTree<T>& t)
{
   BinaryTreeNode<T>* currnode_ptr;

   outstream << std::endl;
   outstream << "n_nodes = " << t.size() << std::endl;
   outstream << "n_levels = " << t.get_n_levels() << std::endl;
   outstream << std::endl;

   std::vector<int> node_IDs;
   std::vector<BinaryTreeNode<T>*> node_ptrs;

   typename std::map<int,BinaryTreeNode<T>* >::iterator iter;
   for (iter=t.BinaryTreeNodes_map_ptr->begin(); 
        iter != t.BinaryTreeNodes_map_ptr->end(); iter++)
   {
      node_IDs.push_back(iter->first);
      node_ptrs.push_back(iter->second);
   }
   templatefunc::Quicksort(node_IDs,node_ptrs);
   
   for (int i=0; i<node_IDs.size(); i++)
   {
      outstream << "node " << *(node_ptrs[i]) << std::endl;
   }

   return outstream;
}

// ==========================================================================
// Tree information member functions
// ==========================================================================

template <class T> inline int BinaryTree<T>::size() const
{
   return BinaryTreeNodes_map_ptr->size();
}

// ---------------------------------------------------------------------
// Member function get_next_unused_ID()

template<class T> int BinaryTree<T>::get_next_unused_ID() 
{
   return size();
}

// ---------------------------------------------------------------------
// Levels start at l=0 and increase downwards in the binary tree.  But
// at a given level, the total number of levels within the tree = l+1.

template <class T> int BinaryTree<T>::get_n_levels() const
{
   int max_level=-1;
   typename BINARYTREENODES_MAP::iterator iter;
   for (iter=BinaryTreeNodes_map_ptr->begin(); 
        iter != BinaryTreeNodes_map_ptr->end(); iter++)
   {
      BinaryTreeNode<T>* node_ptr=iter->second;
      max_level=basic_math::max(max_level,node_ptr->get_level());
   }
   return max_level+1;
}

// ---------------------------------------------------------------------
template <class T> inline BinaryTreeNode<T>* BinaryTree<T>::get_root_ptr()
{
   return root_ptr;
}

// ==========================================================================
// Node generation & retrieval member functions
// ==========================================================================

template <class T> BinaryTreeNode<T>* BinaryTree<T>::generate_root_node()
{
   root_ptr=new BinaryTreeNode<T>(0,0,-1);
   root_ptr->set_gx(0.5);
   (*BinaryTreeNodes_map_ptr)[root_ptr->get_ID()]=root_ptr;
   return root_ptr;
}

// ---------------------------------------------------------------------
template <class T> BinaryTreeNode<T>* BinaryTree<T>::generate_root_node(
   const T& root_data)
{
   generate_root_node();
   root_ptr->set_data(root_data);
   return root_ptr;
}

// ---------------------------------------------------------------------
// Member function get_node_ptr() takes in the ID for some node within
// the binary tree.  If the node exists, its pointer is returned.
// Otherwise this method returns NULL.

template<class T> BinaryTreeNode<T>* BinaryTree<T>::get_node_ptr(int ID)
{
   typename BINARYTREENODES_MAP::iterator iter=
      BinaryTreeNodes_map_ptr->find(ID);
   if (iter==BinaryTreeNodes_map_ptr->end()) 
   {
      return NULL;
   }
   else
   {
      return iter->second;
   }
}

// ---------------------------------------------------------------------
// Member function get_parent_node_ptr() takes in the ID for some node
// within the binary tree.  It returns a pointer to the input node's parent.

template<class T> BinaryTreeNode<T>* BinaryTree<T>::get_parent_node_ptr(
   int child_ID)
{
   int parent_ID=get_node_ptr(child_ID)->get_parent_ID();
   return get_node_ptr(parent_ID);
}

// ==========================================================================
// Node insertion member functions
// ==========================================================================

template <class T> BinaryTreeNode<T>* BinaryTree<T>::addLeftChild(
   int parent_ID,T& data)
{
   BinaryTreeNode<T>* parent_node_ptr=get_node_ptr(parent_ID);
   int left_child_ID=get_next_unused_ID();
   return addLeftChild(parent_node_ptr,left_child_ID,data);
}

// ---------------------------------------------------------------------
template <class T> BinaryTreeNode<T>* BinaryTree<T>::addLeftChild(
   BinaryTreeNode<T>* parent_node_ptr,int left_child_ID,T& data)
{
//   std::cout << "inside BinaryTree::addLeftChild(), parent_node ID = "
//             << parent_node_ptr->get_ID() << std::endl;
//   std::cout << "left child ID = " << left_child_ID << std::endl;

   BinaryTreeNode<T>* left_child_node_ptr=parent_node_ptr->addLeftChild(
      left_child_ID);
   left_child_node_ptr->set_data(data);
   (*BinaryTreeNodes_map_ptr)[left_child_ID]=left_child_node_ptr;
   return left_child_node_ptr;
}

// ---------------------------------------------------------------------
template <class T> BinaryTreeNode<T>* BinaryTree<T>::addRightChild(
   int parent_ID,T& data)
{
//   std::cout << "inside BinaryTree::addRightChild()" << std::endl;
   
   BinaryTreeNode<T>* parent_node_ptr=get_node_ptr(parent_ID);
   int right_child_ID=get_next_unused_ID();
   return addRightChild(parent_node_ptr,right_child_ID,data);
}

// ---------------------------------------------------------------------
template <class T> BinaryTreeNode<T>* BinaryTree<T>::addRightChild(
   BinaryTreeNode<T>* parent_node_ptr,int right_child_ID,T& data)
{
//   std::cout << "inside BinaryTree::addRighttChild(), parent_node_ID = "
//             << parent_node_ptr->get_ID() << std::endl;
//   std::cout << "right child ID = " << right_child_ID << std::endl;

   BinaryTreeNode<T>* right_child_node_ptr=parent_node_ptr->addRightChild(
      right_child_ID);
   right_child_node_ptr->set_data(data);
   (*BinaryTreeNodes_map_ptr)[right_child_ID]=right_child_node_ptr;
   return right_child_node_ptr;
}

// ---------------------------------------------------------------------
// Member function addExistingNodeToTree() takes in parent and child
// nodes which have already been instantiated and related to one
// another at the node level.  It inserts the child node into the
// current Binary Tree.

template <class T> void BinaryTree<T>::addExistingNodeToTree(
   BinaryTreeNode<T>* parent_node_ptr,BinaryTreeNode<T>* child_node_ptr)
{
//   std::cout << "inside BinaryTree::addExistingNodeToTree(), parent_node_ID = "
//             << parent_node_ptr->get_ID() << std::endl;
//   std::cout << "child ID = " << child_node_ptr->get_ID() << std::endl;

   child_node_ptr->set_level(parent_node_ptr->get_level()+1);
   child_node_ptr->set_parent_ID(parent_node_ptr->get_ID());
   (*BinaryTreeNodes_map_ptr)[child_node_ptr->get_ID()]=child_node_ptr;
}

// ==========================================================================
// Node removal member functions
// ==========================================================================

// Member function purge_nodes iterates over all nodes store within
// *BinaryTreeNodes_map_ptr.  It explicitly deletes each node within
// the map.  

template<class T> void BinaryTree<T>::purge_nodes() 
{
//   std::cout << "inside BinaryTree::purge_nodes()" << std::endl;

   typename std::map<int,BinaryTreeNode<T>* >::iterator iter;
   for (iter=BinaryTreeNodes_map_ptr->begin(); 
        iter != BinaryTreeNodes_map_ptr->end(); iter++)
   {
      BinaryTreeNode<T>* BinaryTreeNode_ptr=iter->second;
      delete BinaryTreeNode_ptr;
   } // iterator loop over all Binary Tree nodes
}

// ==========================================================================
// Node placement member functions
// ==========================================================================

// Member function compute_all_gxgy_coords()

template<class T> void BinaryTree<T>::compute_all_gxgy_coords()
{
   double root_gy=1-0.5/get_n_levels();
   get_root_ptr()->set_gy(root_gy);
   compute_children_gxgy_coords(get_root_ptr());
}

// ---------------------------------------------------------------------
// Member function compute_children_gxgy_coords()

template<class T> void BinaryTree<T>::compute_children_gxgy_coords(
   BinaryTreeNode<T>* parent_node_ptr)
{
   double parent_gx=parent_node_ptr->get_gx();
   int child_level=parent_node_ptr->get_level()+1;
   double Delta=1.0/pow(2.0,child_level);

   double child_gy=1-(child_level+0.5)/get_n_levels();

   BinaryTreeNode<T>* LeftChild_ptr=parent_node_ptr->get_LeftChild_ptr();
   if (LeftChild_ptr != NULL)
   {
      LeftChild_ptr->set_gx(parent_gx-0.5*Delta);
      LeftChild_ptr->set_gy(child_gy);
      compute_children_gxgy_coords(LeftChild_ptr);
   }

   BinaryTreeNode<T>* RightChild_ptr=parent_node_ptr->get_RightChild_ptr();
   if (RightChild_ptr != NULL)
   {
      RightChild_ptr->set_gx(parent_gx+0.5*Delta);
      RightChild_ptr->set_gy(child_gy);
      compute_children_gxgy_coords(RightChild_ptr);
   }
}

// ---------------------------------------------------------------------
// Member function get_descendant_node_ptrs()

template<class T> std::vector<BinaryTreeNode<T>*> 
BinaryTree<T>::get_descendant_node_ptrs(int ID)
{
   BinaryTreeNode<T>* starting_node_ptr=get_node_ptr(ID);
   std::vector<BinaryTreeNode<T>*> DescendantsAndSelf;
   starting_node_ptr->GetDescendantsAndSelf(DescendantsAndSelf);
   
   std::vector<BinaryTreeNode<T>*> Descendants;
   for (int i=0; i<DescendantsAndSelf.size()-1; i++)
   {
      Descendants.push_back(DescendantsAndSelf[i]);
   }
   return Descendants;
}

