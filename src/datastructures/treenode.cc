// ==========================================================================
// Templatized treenode class member function definitions
// ==========================================================================
// Last modified on 7/7/12; 7/16/12; 7/18/12
// ==========================================================================

#include "templates/mytemplates.h"

// ==========================================================================
// Initialization, constructor and destructor methods:
// ==========================================================================

template <class T> void treenode<T>::allocate_member_objects()
{
   child_nodes_map_ptr=new TREENODES_MAP;
}

template <class T> void treenode<T>::initialize_member_objects()
{
   previously_visited_flag=false;
   ID=level=rank=-1;
   parent_node_ptr=NULL;
   data_ptr=NULL;
}

template <class T> treenode<T>::treenode()
{
   allocate_member_objects();
   initialize_member_objects();
}

template <class T> treenode<T>::treenode(int id,int level)
{
   allocate_member_objects();
   initialize_member_objects();
   ID=id;
   this->level=level;
}

// Copy constructor:

template <class T> treenode<T>::treenode(const treenode<T>& node)
{
   allocate_member_objects();
   initialize_member_objects();
   docopy(node);
}

template <class T> treenode<T>::~treenode()
{

// As of 7/4/12, we assume the data_ptr member is a pointer to a
// dynamically instantiated object.  So when we destroy this treenode,
// we call the destructor for *data_ptr:

   delete data_ptr;

   delete child_nodes_map_ptr;
   parent_node_ptr=NULL;
}

// ---------------------------------------------------------------------
template <class T> void treenode<T>::docopy(const treenode<T>& node)
{
   ID=node.ID;
   level=node.level;
   rank=node.rank;
   data_ptr=node.data_ptr;
}

// Overload = operator:

template <class T> treenode<T>& treenode<T>::operator= 
(const treenode<T>& n)
{
   if (this==&n) return *this;
   docopy(n);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

template <class T> std::ostream& operator<< 
(std::ostream& outstream,treenode<T>& node)
{
   outstream << std::endl;
   outstream << "treenode ID = " << node.ID << std::endl;
   outstream << "level = " << node.level << std::endl;
   outstream << "rank = " << node.rank << std::endl;
   if (node.get_previously_visited_flag())
   {
      outstream << "Node previously visited" << std::endl;
   }
   outstream << "data_ptr = " << node.get_data_ptr() << std::endl;
//   outstream << "*data_ptr = " << (*node.get_data_ptr()) << std::endl;
   if (node.get_parent_node_ptr() != NULL)
   {
      outstream << "parent node ID = " << node.get_parent_node_ptr()->get_ID()
                << std::endl;
   }
   if (node.is_leaf_flag())
   {
      outstream << "Current node is a leaf" << std::endl;
   }
   
   if (node.getNumChildren() > 0)
   {
      outstream << "children node IDs:" << std::endl;
      std::vector<treenode<T>* > ChildrenPtrs=node.getChildrenNodePtrs();
      for (int c=0; c<ChildrenPtrs.size(); c++)
      {
         std::cout << ChildrenPtrs[c]->get_ID() << " ";
      }
      std::cout << std::endl;
   }

   return outstream;
}

// ==========================================================================
// Parent member functions:
// ==========================================================================

template <class T> inline treenode<T>* treenode<T>::get_parent_node_ptr() 
{
   return parent_node_ptr;
}

// ---------------------------------------------------------------------
template <class T> inline const treenode<T>* treenode<T>::get_parent_node_ptr()  const
{
   return parent_node_ptr;
}

// ---------------------------------------------------------------------
template <class T> void treenode<T>::set_parent_node_ptr(
   treenode<T>* parent_node_ptr)
{
//   std::cout << "inside treenode::set_parent_node_ptr()" << std::endl;
//   std::cout << "this->ID = " << get_ID()
//             << " parent_node_ptr->get_ID() = " 
//             << parent_node_ptr->get_ID() << std::endl;
   this->parent_node_ptr=parent_node_ptr;
   parent_node_ptr->addChild(this);
}

// ==========================================================================
// Children member functions:
// ==========================================================================

template <class T> bool treenode<T>::isChild(treenode<T>* child_node_ptr)
{
   int child_ID=child_node_ptr->get_ID();
   child_treenode_iter=child_nodes_map_ptr->find(child_ID);
   if (child_treenode_iter==child_nodes_map_ptr->end())
   {
      return false;
   }
   else
   {
      return true;
   }
}

// ---------------------------------------------------------------------
template <class T> void treenode<T>::addChild(treenode<T>* child_node_ptr)
{
//   std::cout << "inside treenode::addChild()" << std::endl;
//   std::cout << "curr level = " << get_level() << std::endl;
   int child_ID=child_node_ptr->get_ID();
//   std::cout << "this->ID = " << get_ID()
//             << " child_ID = " << child_ID << std::endl;

   if (isChild(child_node_ptr))
   {
//      std::cout << "Warning in treenode::addChild()" << std::endl;
//      std::cout << "Node with ID = " << child_ID
//                << " is already a child of node with ID = " << get_ID()
//                << std::endl;
      return;
   }
   (*child_nodes_map_ptr)[child_ID]=child_node_ptr;
   child_node_ptr->set_parent_node_ptr(this);
}

// ---------------------------------------------------------------------
template <class T> void treenode<T>::deleteChild(treenode<T>* child_node_ptr)
{
//   std::cout << "inside treenode::deleteChild()" << std::endl;
   int child_ID=child_node_ptr->get_ID();

   child_treenode_iter=child_nodes_map_ptr->find(child_ID);
   if (child_treenode_iter==child_nodes_map_ptr->end())
   {
      return;
   }
   else
   {
      child_nodes_map_ptr->erase(child_treenode_iter);
   }
}

// ---------------------------------------------------------------------
template <class T> int treenode<T>::getNumChildren() const
{
   return child_nodes_map_ptr->size();
}

// ---------------------------------------------------------------------
// Member function is_leaf_flag() returns true if the current treenode
// has no children.

template <class T> bool treenode<T>::is_leaf_flag() const
{
   if (child_nodes_map_ptr->size() > 0)
   {
      return false;
   }
   else
   {
      return true;
   }
}

// ---------------------------------------------------------------------
// Member function get_n_siblings() returns the number of children
// for the current node's parent.

template <class T> int treenode<T>::get_n_siblings() const
{
   const treenode<T>* parent_node_ptr=get_parent_node_ptr();
   if (parent_node_ptr==NULL) return -1;
   
   int n_siblings=parent_node_ptr->getNumChildren()-1;
   return n_siblings;
}

// ---------------------------------------------------------------------
// Member function getChildrenNotePtrs() is generally deprecated.  Use
// reset_child_treenode_ptr() and get_next_child_treenode_ptr()
// instead.

template <class T> std::vector<treenode<T>* > 
treenode<T>::getChildrenNodePtrs()
{
   std::vector<treenode<T>* > ChildrenNodePtrs;
   
   for (child_treenode_iter=child_nodes_map_ptr->begin(); 
        child_treenode_iter != child_nodes_map_ptr->end(); 
        child_treenode_iter++)
   {
      ChildrenNodePtrs.push_back(child_treenode_iter->second);
   }
   return ChildrenNodePtrs;
}

// ---------------------------------------------------------------------
template <class T> treenode<T>* treenode<T>::reset_child_treenode_ptr()
{
   child_treenode_iter=child_nodes_map_ptr->begin();
   if (child_treenode_iter==child_nodes_map_ptr->end())
   {
      return NULL;
   }
   else
   {
      return child_treenode_iter->second;
   }
}

// ---------------------------------------------------------------------
template <class T> treenode<T>* treenode<T>::get_next_child_treenode_ptr()
{
   child_treenode_iter++;
   if (child_treenode_iter==child_nodes_map_ptr->end())
   {
      return NULL;
   }
   else
   {
      return child_treenode_iter->second;
   }
}

// ---------------------------------------------------------------------
template <class T> treenode<T>* treenode<T>::get_rootnode_ptr()
{
   treenode<T>* parent_node_ptr=get_parent_node_ptr();
   if (parent_node_ptr==NULL || parent_node_ptr==this) 
   {
      return this;
   }
   else
   {
      return parent_node_ptr->get_rootnode_ptr();
   }
}

