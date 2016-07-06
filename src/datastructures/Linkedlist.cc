// ==========================================================================
// Templatized Linkedlist class member function definitions
// ==========================================================================
// Last modified on 9/23/05; 6/14/06
// ==========================================================================

#include "datastructures/Mynode.h"
#include "templates/mytemplates.h"

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

template <class T> void Linkedlist<T>::initialize_member_objects() 
{
   n_nodes=0;
   unique_ID_counter=0;
   start_ptr=NULL;
   stop_ptr=NULL;
}

template <class T> void Linkedlist<T>::allocate_member_objects() 
{
}

template <class T> Linkedlist<T>::Linkedlist()
{
   initialize_member_objects();
}

// Copy constructor:

template <class T> Linkedlist<T>::Linkedlist(const Linkedlist<T>& l)
{
   initialize_member_objects();
   allocate_member_objects();
   docopy(l);
}

template <class T> void Linkedlist<T>::docopy(const Linkedlist<T>& l)
{

// Generate new set of nodes whose contents are filled up with
// incoming nodes' values:

   Mynode<T>* l_currnode_ptr=l.start_ptr;
   while (l_currnode_ptr != NULL)
   {
      append_node(l_currnode_ptr->get_data());
      stop_ptr->set_ID(l_currnode_ptr->get_ID());
      stop_ptr->set_order(l_currnode_ptr->get_order());
      l_currnode_ptr=l_currnode_ptr->get_nextptr();
   }
}	

// Overload = operator:

template <class T> Linkedlist<T>& Linkedlist<T>::operator= 
(const Linkedlist<T>& l)
{
   if (this==&l) return *this;
   docopy(l);
   return *this;
}

// ---------------------------------------------------------------------
// On 6/11/01, we learned from James Wanken that we must explicitly
// delete any object which has been dynamically allocated.  In
// particular, we need to traverse the linked list in the destructor
// and explicitly delete each of the nodes before a linked list goes
// out of scope:

template <class T> void Linkedlist<T>::purge_all_nodes(bool destructor_flag)
{
//   std::cout << "inside Linkedlist::purge_all_nodes()" << std::endl;
   if (n_nodes > 0 && start_ptr != NULL)
   {
      Mynode<T>* currnode_ptr=start_ptr;
      while (currnode_ptr->get_nextptr() != NULL)
      {
         currnode_ptr=currnode_ptr->get_nextptr();
         delete currnode_ptr->get_prevptr();
      }
      delete currnode_ptr;
   }
   if (!destructor_flag)
   {
      n_nodes=0;
      start_ptr=NULL;
      stop_ptr=NULL;
   }
}

template <class T> Linkedlist<T>::~Linkedlist()
{
//   std::cout << "inside Linkedlist destructor" << std::endl;
   purge_all_nodes(true);
}

// ---------------------------------------------------------------------
// Overload << operator:

template <class T> std::ostream& operator<< 
(std::ostream& outstream,const Linkedlist<T>& l)
{
   Mynode<T>* currnode_ptr;

   outstream << std::endl;
   outstream << "n_nodes = " << l.n_nodes << std::endl;
   outstream << std::endl;

   if (l.n_nodes > 0)
   {
      for (currnode_ptr=l.start_ptr; currnode_ptr != NULL;
           currnode_ptr=currnode_ptr->get_nextptr())
      {
         outstream << *currnode_ptr << std::endl;
      }
   }
   return outstream;
}

// ==========================================================================
// Data retrieval member functions
// ==========================================================================

// Member function data_in_list traverses through the list and
// compares each node's data with the input data.  If a match is
// found, this method returns a pointer to the node containing the
// matching data.  Otherwise, it returns NULL.

template <class T> Mynode<T>* Linkedlist<T>::data_in_list(const T& data) const
{
   for (Mynode<T>* currnode_ptr=start_ptr; currnode_ptr != NULL;
        currnode_ptr=currnode_ptr->get_nextptr())
   {
      if (data==currnode_ptr->get_data()) return currnode_ptr;
   }
   return NULL;
}

// ---------------------------------------------------------------------
// Member function get_data takes in integer n which labels the
// location within the linked list from where data is desired.  This
// method executes a brute force loop over the list and returns the
// data within the nth node.

// Note added on 7/18/05: We might want to use the
// convert_list_to_vector method in the future within this member
// function...

template <class T> T Linkedlist<T>::get_data(int n) const
{
//   std::cout << "inside Linkedlist::get_data(), n = " << n
//             << " n_nodes = " << n_nodes << std::endl;
   
   if (n >= n_nodes)
   {
      std::cout << "Error in Linkedlist::get_data()!" << std::endl;
      std::cout << "n = " << n << " n_nodes = " << n_nodes << std::endl;
      std::cout << "Exiting now..." << std::endl;
      exit(-1);
   }
   
   int counter=0;
   for (Mynode<T>* currnode_ptr=start_ptr; currnode_ptr != NULL; 
        currnode_ptr=currnode_ptr->get_nextptr())
   {
      if (counter==n)
      {
         return currnode_ptr->get_data();
      }
      else
      {
         counter++;
      }
   }

   std::cout << "At end of Linkedlist::get_data()" << std::endl;
   std::cout << "Should not have reached this point...exiting..." 
             << std::endl;
   exit(-1);
}

// ==========================================================================
// Node manipulation member functions
// ==========================================================================

// Member function insert_node_into_list inserts a new node into the
// linked list between nodes posn and posn+1.  In order to add a node
// to the very beginning of the list, call this member function with
// posn=-1.  To add a node to the very end of the list, call this
// method with posn = n_nodes.

template <class T> void Linkedlist<T>::insert_node_into_list(
   const int posn,const double ord,Mynode<T>* newnode_ptr)
{

// As of 9/23/05, we no longer set the new node's ID equal to n_nodes,
// for it may not be unique if node deletion occurs.  Instead, we set
// every new node's ID equal to a counter value which monotonically
// increases over the Linkedlist's life:

   newnode_ptr->set_ID(unique_ID_counter++);

   newnode_ptr->set_order(ord);
        
// If linked list is empty, simply create a first node in the list:

   if (n_nodes==0)
   {
      start_ptr=newnode_ptr;
      stop_ptr=start_ptr;
      start_ptr->set_nextptr(NULL);
      start_ptr->set_prevptr(NULL);
   }
   else
   {
      if (posn==n_nodes)
      {
         newnode_ptr->set_nextptr(NULL);
         newnode_ptr->set_prevptr(stop_ptr);
         stop_ptr->set_nextptr(newnode_ptr);
         stop_ptr=newnode_ptr;
      }
      else if (posn==-1)
      {
         newnode_ptr->set_nextptr(start_ptr);
         newnode_ptr->set_prevptr(NULL);
         start_ptr->set_prevptr(newnode_ptr);
         start_ptr=newnode_ptr;
      }
      else
      {

// Traverse through first posn nodes within the linked list.
      
         int i=0;
         Mynode<T>* currnode_ptr=start_ptr;
         while (i<posn)
         {
            currnode_ptr=currnode_ptr->get_nextptr();
            i++;
         }
         Mynode<T>* nextnode_ptr=currnode_ptr->get_nextptr();

// Perform pointer surgery: 

         newnode_ptr->set_nextptr(nextnode_ptr);
         newnode_ptr->set_prevptr(currnode_ptr);
         currnode_ptr->set_nextptr(newnode_ptr);
         nextnode_ptr->set_prevptr(newnode_ptr);
      }
   } // n_nodes==0 conditional
   n_nodes++;
}

// ---------------------------------------------------------------------
// Member function get_node returns a pointer to the node labeled by
// linked list position posn:

template <class T> Mynode<T>* Linkedlist<T>::get_node(int posn) const
{
   Mynode<T>* currnode_ptr=NULL;

   if (posn >= 0 && posn < n_nodes)
   {
      int currposn=0;
      currnode_ptr=start_ptr;
      while (currposn < posn)
      {
         currnode_ptr=currnode_ptr->get_nextptr();
         currposn++;
      }
   }
   return currnode_ptr;
}

// ---------------------------------------------------------------------
// Member function get_ID_node returns a pointer to the node whose ID
// equals the input parameter:

template <class T> Mynode<T>* Linkedlist<T>::get_ID_node(int ID) const
{
   for (Mynode<T>* currnode_ptr=start_ptr; currnode_ptr != NULL;
        currnode_ptr=currnode_ptr->get_nextptr())
   {
      if (currnode_ptr->get_ID()==ID) return currnode_ptr;
   }
   return NULL;
}

// ---------------------------------------------------------------------
// Member function delete_node removes the *node_ptr Mynode<T> from the
// current linkedlist object.  It performs the necessary pointer
// surgery on the nodes before and after the deleted node.

template <class T> void Linkedlist<T>::delete_node(Mynode<T>* node_ptr)
{
   if (node_ptr != NULL)
   {
      Mynode<T>* currnode_ptr=node_ptr;
      Mynode<T> *prevnode_ptr,*nextnode_ptr;

// If linked list is empty, print error message:

      if (n_nodes==0)
      {
         std::cout << "Error inside Linkedlist<T>::delete_node() !" 
                   << std::endl;
         std::cout << "Cannot delete node since number of nodes = 0" 
                   << std::endl;
      }
      else if (n_nodes==1)
      {
         delete currnode_ptr;
         start_ptr=NULL;
         stop_ptr=NULL;
         n_nodes=0;
      }
      else
      {
         if (node_ptr==start_ptr)
         {
            currnode_ptr=currnode_ptr->get_nextptr();
            prevnode_ptr=currnode_ptr->get_prevptr();
            delete prevnode_ptr;
            currnode_ptr->set_prevptr(NULL);
            start_ptr=currnode_ptr;
         }
         else if (node_ptr==stop_ptr)
         {
            currnode_ptr=currnode_ptr->get_prevptr();
            nextnode_ptr=currnode_ptr->get_nextptr();
            delete nextnode_ptr;
            currnode_ptr->set_nextptr(NULL);
            stop_ptr=currnode_ptr;
         }
         else
         {
            prevnode_ptr=currnode_ptr->get_prevptr();
            nextnode_ptr=currnode_ptr->get_nextptr();

// Perform pointer surgery.  
         
            prevnode_ptr->set_nextptr(nextnode_ptr);
            nextnode_ptr->set_prevptr(prevnode_ptr);
            delete currnode_ptr;
         }
         n_nodes--;
      }  // n_nodes==0 conditional
   } // node_ptr != NULL conditional
}

// ---------------------------------------------------------------------
// Member function sort_nodes takes in an ordered array of node
// pointers.  It rearranges the links in the chain so that the list's
// order matches that of the ordered array:

template <class T> void Linkedlist<T>::sort_nodes(Mynode<T>* node_ptr[])
{
   start_ptr=node_ptr[0];
   stop_ptr=node_ptr[n_nodes-1];
   start_ptr->set_prevptr(NULL);
   start_ptr->set_nextptr(node_ptr[1]);

   Mynode<T>* currnode_ptr=start_ptr->get_nextptr();
   int n=1;
   while (n <= n_nodes-2)
   {
      currnode_ptr->set_prevptr(node_ptr[n-1]);
      currnode_ptr->set_nextptr(node_ptr[n+1]);
      currnode_ptr=currnode_ptr->get_nextptr();
      n++;
   }
   stop_ptr->set_prevptr(node_ptr[n_nodes-2]);
   stop_ptr->set_nextptr(NULL);
}

// ---------------------------------------------------------------------
// Member function sort_nodes takes in an array var[] whose size and
// ordering is assumed to match those of the current linked list.
// This method rearranges the array's values so that they are either
// monotonically increasing or decreasing (depending upon input
// boolean parameter decreasing_func).  It makes the corresponding
// changes to the linked list's nodes as well.

template <class T> void Linkedlist<T>::sort_nodes(
   double var[],bool decreasing_func)
{
   if (n_nodes >= 2)
   {
      Mynode<T>* node_ptr[n_nodes];
   
      int counter=0;
      Mynode<T>* currnode_ptr=start_ptr;
      do 
      {
         node_ptr[counter++]=currnode_ptr;
         currnode_ptr=currnode_ptr->get_nextptr();
      } 
      while (currnode_ptr != NULL);

      if (decreasing_func)
      {
         Quicksort_descending(var,node_ptr,n_nodes);
      }
      else
      {
         Quicksort(var,node_ptr,n_nodes);
      }
      sort_nodes(node_ptr);
   } // n_nodes >=2 conditional
}

// ---------------------------------------------------------------------
// Member function concatenate takes in a Linkedlist and adds its
// nodes to the end of the current Linkedlist object:

template <class T> void Linkedlist<T>::concatenate(
   Linkedlist<T>* taillist_ptr)
{
   for (Mynode<T>* currnode_ptr=taillist_ptr->get_start_ptr();
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      append_node(currnode_ptr->get_data());
      stop_ptr->set_ID(currnode_ptr->get_ID());
      stop_ptr->set_order(currnode_ptr->get_order());
   }
}

// ---------------------------------------------------------------------
// Member function concatenate_wo_duplication takes in a Linkedlist
// and adds those nodes whose data entries differ from the ones in the
// current Linkedlist object to the end of the current Linkedlist:

template <class T> void Linkedlist<T>::concatenate_wo_duplication(
   Linkedlist<T>* taillist_ptr)
{
   for (Mynode<T>* currnode_ptr=taillist_ptr->get_start_ptr();
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      T curr_data=currnode_ptr->get_data();
      if (data_in_list(curr_data)==NULL) 
      {
         append_node(curr_data);
         stop_ptr->set_ID(currnode_ptr->get_ID());
         stop_ptr->set_order(currnode_ptr->get_order());
      }
   }
}

// ---------------------------------------------------------------------
// Member function convert_list_to_vector returns a dynamically
// generated vector which is filled up with the data from the current
// linked list object.

template <class T> std::vector<T>* Linkedlist<T>::convert_list_to_vector()
{
   std::vector<T>* vector_ptr=new std::vector<T>;
   vector_ptr->reserve(size());
   for (Mynode<T>* currnode_ptr=start_ptr; currnode_ptr != NULL;
        currnode_ptr=currnode_ptr->get_nextptr())
   {
      vector_ptr->push_back(currnode_ptr->get_data());
   }
   return vector_ptr;
}

// ---------------------------------------------------------------------
// Member function convert_vector_to_list first purges the current
// Linkedlist object.  It then appends the contents of input STL
// vector V to the current list object.

template <class T> void Linkedlist<T>::convert_vector_to_list(
   const std::vector<T>& V)
{
   purge_all_nodes();
   for (int i=0; i<V.size(); i++)
   {
      append_node(V[i]);
   }
}

// ---------------------------------------------------------------------
// Boolean member function overlapping_list loops over every node
// within the current linked list as well as in input list *list2_ptr.
// It returns true if the address of any node in the first list
// matches that in the second.  This method can be used to search for
// memory clobbers.

template <class T> bool Linkedlist<T>::overlapping_list(
   Linkedlist<T> const *list2_ptr) const
{
   bool overlap_flag=false;
   for (const Mynode<T>* currnode_ptr=start_ptr; currnode_ptr != NULL;
        currnode_ptr=currnode_ptr->get_nextptr())
   {
      for (const Mynode<T>* node2_ptr=list2_ptr->get_start_ptr(); 
           node2_ptr != NULL; node2_ptr=node2_ptr->get_nextptr())
      {
         if (currnode_ptr==node2_ptr) overlap_flag=true;
      }
   }
   return overlap_flag;
}
