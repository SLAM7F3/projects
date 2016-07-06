// ==========================================================================
// Templatized Treenode class member function definitions
// ==========================================================================
// Last modified on 11/7/06
// ==========================================================================

#include "templates/mytemplates.h"

// ==========================================================================
// Initialization, constructor and destructor methods:
// ==========================================================================

template <class T> void Treenode<T>::allocate_member_objects()
{
   Parents_list_ptr=new Linkedlist< Treenode<T>* >;
   Children_list_ptr=new Linkedlist< Treenode<T>* >;
}

template <class T> void Treenode<T>::initialize_member_objects()
{
   ID=level=column=-1;
   offspring_counter=0;
}

template <class T> Treenode<T>::Treenode()
{
   allocate_member_objects();
   initialize_member_objects();
}

template <class T> Treenode<T>::Treenode(int id,int l,int cc)
{
   allocate_member_objects();
   initialize_member_objects();
   ID=id;
   level=l;
   own_child_index=cc;
}

// Copy constructor:

template <class T> Treenode<T>::Treenode(const Treenode<T>& node)
{
   allocate_member_objects();
   initialize_member_objects();
   docopy(node);
}

template <class T> Treenode<T>::~Treenode()
{
   delete Parents_list_ptr;
   delete Children_list_ptr;
}

// ---------------------------------------------------------------------
template <class T> void Treenode<T>::docopy(const Treenode<T>& node)
{
   ID=node.ID;
   data=node.data;
}

// Overload = operator:

template <class T> Treenode<T>& Treenode<T>::operator= 
(const Treenode<T>& n)
{
   if (this==&n) return *this;
   docopy(n);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

template <class T> std::ostream& operator<< 
(std::ostream& outstream,const Treenode<T>& node)
{
   outstream << std::endl;
   outstream << "ID = " << node.ID << std::endl;
   outstream << "level = " << node.level << std::endl;
   outstream << "column = " << node.column << std::endl;
   outstream << "own child index = " << node.own_child_index << std::endl;
   outstream << "Total indices = " << std::endl;
   templatefunc::printVector(node.get_total_indices());
   outstream << "data = " << node.get_data() << std::endl;
   outstream << "*Parents_list_ptr = " << *(node.Parents_list_ptr) 
             << std::endl;
   outstream << "*Children_list_ptr = " << *(node.Children_list_ptr) 
             << std::endl;
   return outstream;
}

// ==========================================================================
// Parents & children manipulation member functions:
// ==========================================================================

template <class T> int Treenode<T>::getNumParents() const
{
   return Parents_list_ptr->size();
}

// ---------------------------------------------------------------------
template <class T> int Treenode<T>::getNumChildren() const
{
   return Children_list_ptr->size();
}

// ---------------------------------------------------------------------
template <class T> Treenode<T>* Treenode<T>::addChild(int id)
{
   int child_level=get_level()+1;
   Treenode<T>* child_node_ptr=new Treenode<T>(
      id,child_level,offspring_counter);

// Assign parent's total indices to new child's total indices.  Then
// append offspring_counter child's total indices:

   for (int unsigned i=0; i<total_indices.size(); i++)
   {
      child_node_ptr->get_total_indices().push_back(total_indices[i]);
   }
   child_node_ptr->get_total_indices().push_back(offspring_counter);
   

   Children_list_ptr->append_node(child_node_ptr);
   child_node_ptr->get_Parents_list_ptr()->append_node(this);

   offspring_counter++;
   return child_node_ptr;
}

