// ==========================================================================
// Templatized Tree class member function definitions
// ==========================================================================
// Last modified on 11/9/06; 11/13/06; 12/4/10; 8/15/15
// ==========================================================================


#include "math/basic_math.h"

// Initialization, constructor and destructor methods:

template <class T> void Tree<T>::allocate_member_objects() 
{
}

template <class T> void Tree<T>::initialize_member_objects() 
{
   generate_root_node();
}

template <class T> Treenode<T>* Tree<T>::generate_root_node() 
{
   int root_level=0;
   int root_child_index=0;
   root_ptr=new Treenode<T>(
      get_next_unused_ID(),root_level,root_child_index);
   root_ptr->get_total_indices().push_back(0);
//   std::cout << "*root_ptr = " << *root_ptr << std::endl;

   threevector root_posn(0,0,0);
   root_ptr->set_posn(root_posn);

   Triple<int,std::vector<int>,Treenode<T>*> root_triple(
      root_ptr->get_ID(),root_ptr->get_total_indices(),root_ptr);
   tree_nodes.push_back(root_triple);

   return root_ptr;
}

template <class T> Tree<T>::Tree()
{
   allocate_member_objects();
   initialize_member_objects();
}

// Copy constructor:

template <class T> Tree<T>::Tree(const Tree<T>& t)
{
   allocate_member_objects();
   initialize_member_objects();
   docopy(t);
}

template <class T> void Tree<T>::docopy(const Tree<T>& t)
{
}	

// Overload = operator:

template <class T> Tree<T>& Tree<T>::operator= (const Tree<T>& t)
{
   if (this==&t) return *this;
   docopy(t);
   return *this;
}

template <class T> Tree<T>::~Tree()
{
   delete root_ptr;
//   std::cout << "inside Tree destructor" << std::endl;
//   purge_nodes(true);
}

// ---------------------------------------------------------------------
// Overload << operator:

template <class T> std::ostream& operator<< 
(std::ostream& outstream,const Tree<T>& t)
{
   Treenode<T>* currnode_ptr;

   outstream << std::endl;
   outstream << "n_nodes = " << t.size() << std::endl;
   outstream << std::endl;

   if (t.size() > 0)
   {
//      for (currnode_ptr=l.start_ptr; currnode_ptr != NULL;
//           currnode_ptr=currnode_ptr->get_nextptr())
//      {
//         outstream << *currnode_ptr << std::endl;
//      }
   }
   return outstream;
}

// ==========================================================================
// Data retrieval & insertion member functions
// ==========================================================================

// Member function get_next_unused_ID loops over all nodes within this
// tree.  It returns the smallest integer which does not correspond to
// any Treenode within the tree.

template<class T> int Tree<T>::get_next_unused_ID() const
{
   bool ID_already_exists;
   int next_ID=0;
   do
   {
      ID_already_exists=false;
      for (int i=0; i<size(); i++)
      {
         if (tree_nodes[i].first==next_ID)
         {
            next_ID++;
            ID_already_exists=true;
         }
      }
   }
   while (ID_already_exists);
   return next_ID;
}

// --------------------------------------------------------------------------
// Member function get_Treenode_ptr(int n) returns the nth Treenode
// located within member STL vector nodes:

template <class T> Treenode<T>* Tree<T>::get_Treenode_ptr(int n) 
{
   if (n >= 0 && n < size())
   {
      return tree_nodes[n].third;
   }
   else
   {
      return NULL;
   }
}
   
// --------------------------------------------------------------------------
template <class T> Treenode<T>* Tree<T>::get_ID_labeled_Treenode_ptr(int ID)
{
   for (int n=0; n<size(); n++)
   {
      if (tree_nodes[n].first==ID)
      {
         return tree_nodes[n].third;
      }
   }
   return NULL;
}

// --------------------------------------------------------------------------
template <class T> Treenode<T>* 
Tree<T>::get_total_indices_labeled_Treenode_ptr(
   const std::vector<int>& tot_indices)
{
   for (int n=0; n<size(); n++)
   {
      if (tree_nodes[n].second==tot_indices)
      {
         return tree_nodes[n].third;
      }
   }
   return NULL;
}

// --------------------------------------------------------------------------
template <class T> int Tree<T>::get_ID(const std::vector<int>& tot_indices) 
{
   Treenode<T>* currnode_ptr=get_total_indices_labeled_Treenode_ptr(
      tot_indices);
   if (currnode_ptr==NULL)
   {
      std::cout << "Error in Tree::get_ID(tot_indices)" << std::endl;
      std::cout << "currnode_ptr = NULL!" << std::endl;
      return -1;
   }
   else
   {
      return currnode_ptr->get_ID();
   }
}


// ---------------------------------------------------------------------
template <class T> Treenode<T>* Tree<T>::addChild(int parent_id)
{
//   std::cout << "inside Tree::addChild()" << std::endl;
   
   Treenode<T>* parent_node_ptr=get_ID_labeled_Treenode_ptr(parent_id);

   int child_id=get_next_unused_ID();
   Treenode<T>* child_node_ptr=parent_node_ptr->addChild(child_id);

   Triple<int,std::vector<int>,Treenode<T>* > new_tree_node(
      child_id,child_node_ptr->get_total_indices(),child_node_ptr);
   tree_nodes.push_back(new_tree_node);

//   std::cout << "size() = " << size() << std::endl;
//   std::cout << "get_n_levels() = " << get_n_levels() << std::endl;

//   std::cout << "*parent_node_ptr = " << *parent_node_ptr << std::endl;
//   std::cout << "*child_node_ptr = " << *child_node_ptr << std::endl;
   
   return child_node_ptr;
}

// ---------------------------------------------------------------------
// Note: Levels within Tree<T> range from 0 up to and including
// get_n_levels()

template <class T> int Tree<T>::get_n_levels() 
{
   int max_level=-1;
   for (int i=0; i<size(); i++)
   {
      max_level=basic_math::max(max_level,get_Treenode_ptr(i)->get_level());
   }
   return max_level;
}

// ---------------------------------------------------------------------
template <class T> std::vector<Treenode<T>* > 
Tree<T>::retrieve_nodes_on_level(int level)
{
   std::vector<Treenode<T>* > nodes_on_level;
   for (int i=0; i<size(); i++)
   {
      if (get_Treenode_ptr(i)->get_level()==level)
      {
         nodes_on_level.push_back(get_Treenode_ptr(i));
      }
   }
   return nodes_on_level;
}

// ---------------------------------------------------------------------
template <class T> int Tree<T>::number_nodes_on_level(int level)
{
   return retrieve_nodes_on_level(level).size();
}

// ---------------------------------------------------------------------
// Member function compute_columns_for_nodes_on_level first retrieves
// all nodes on the input level.  It next forms an STL vector of each
// node's total_index sequence.  After sorting the sequence, this
// method assigns unique column values to each Treenode on the level
// based upon its sorted position.  The column values range from 0 to
// number_nodes_on_level(level)-1.

template <class T> void Tree<T>::compute_columns_for_nodes_on_level(int level)
{
   std::vector<Treenode<T>* > nodes_on_level=retrieve_nodes_on_level(level);
   std::vector<std::vector<int> > total_indices_sequences;

   for (unsigned int n=0; n<nodes_on_level.size(); n++)
   {
      Treenode<T>* currnode_ptr=nodes_on_level[n];
      std::vector<int> curr_total_indices;
      for (int unsigned i=0; i<currnode_ptr->get_total_indices().size(); i++)
      {
         curr_total_indices.push_back(
            currnode_ptr->get_total_indices().at(i));
      }

//      std::cout << "n = " << n << " curr_total_indices = " << std::endl;
//      templatefunc::printVector(curr_total_indices);

      total_indices_sequences.push_back(curr_total_indices);

   } // loop over index n labeling nodes on current level

   templatefunc::Quicksort(total_indices_sequences,nodes_on_level);

   for (unsigned int n=0; n<nodes_on_level.size(); n++)
   {
      Treenode<T>* currnode_ptr=nodes_on_level[n];
      currnode_ptr->set_column(n);
   }
}

// ---------------------------------------------------------------------
// Member function get_node retrieves the Treenode located on the
// input level and within the specified column.  If no node
// corresponding to these input parameters is found within the tree,
// this method returns NULL.

template <class T> Treenode<T>* Tree<T>::get_node(int level,int column)
{
//   std::cout << "inside Tree::get_node(l,c)" << std::endl;
   
   std::vector<Treenode<T>* > nodes_on_level=
      retrieve_nodes_on_level(level);
   for (unsigned int n=0; n<nodes_on_level.size(); n++)
   {
      Treenode<T>* currnode_ptr=nodes_on_level[n];

//      std::cout << "n = " << n 
//                << " nodes_on_level.size() = " << nodes_on_level.size() 
//                << " currnode_ptr = " << currnode_ptr 
//                << " column = " << currnode_ptr->get_column()
//                << std::endl;

      if (currnode_ptr->get_column()==column) return currnode_ptr;
   } // loop over index n labeling nodes on level
   return NULL;
}

// ---------------------------------------------------------------------
// This overloaded version of addChild takes in the vector<int>
// indices labels for some Treenode to be inserted into the Tree.  It
// first checks whether the Treenode already exists within the tree.
// If not, it next checks whether the node's parent exists.  If the
// parent does exist, this method adds the child to the parent.  If
// the parent does not exist, this method recursively calls itself,
// instantiates the parent node and then adds the child to the parent.

template <class T> Treenode<T>* Tree<T>::addChild(
   const std::vector<int>& tot_indices)
{
   Treenode<T>* childnode_ptr=
      get_total_indices_labeled_Treenode_ptr(tot_indices);

// First check whether childnode to be inserted into tree already
// exists.  If so, do nothing:

   if (childnode_ptr==NULL)
   {

// Next check whether child's parent already exists in tree:

      std::vector<int> parent_indices;
      for (unsigned int i=0; i<tot_indices.size()-1; i++)
      {
         parent_indices.push_back(tot_indices[i]);
      }
      
      Treenode<T>* parentnode_ptr=
         get_total_indices_labeled_Treenode_ptr(parent_indices);
      
// If no parent node is found, recursively call this method and create
// it:

      if (parentnode_ptr==NULL)
      {
         parentnode_ptr=addChild(parent_indices);
      }
      
      childnode_ptr=addChild(parentnode_ptr->get_ID());

   } // childnode_ptr == NULL conditional
   return childnode_ptr;
}

// ---------------------------------------------------------------------
// Member function purge_nodes loops over all treenodes as a function
// of level and column.  It deletes each treenode and clears the
// tree_nodes vector member object.

template<class T> void Tree<T>::purge_nodes() 
{
//   std::cout << "inside Tree::purge_nodes()" << std::endl;
   
   for (int l=get_n_levels(); l>=0; l--)
   {
      compute_columns_for_nodes_on_level(l);
      for (int c=0; c<number_nodes_on_level(l); c++)
      {
         Treenode<T>* curr_treenode_ptr=get_node(l,c);
//         std::cout << "l = " << l << " c = " << c
//                   << " treenode_ptr = " << curr_treenode_ptr << std::endl;
         delete curr_treenode_ptr;
      } // loop over index c labeling columns
   } // loop over index l labeling levels

   tree_nodes.clear();
}

