// ==========================================================================
// SITE base class member function definitions
// ==========================================================================
// Last modified on 3/16/05
// ==========================================================================

#include <iostream>
#include "datastructures/Linkedlist.h"
#include "network/Site.h"

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

template <class T> void Site<T>::allocate_member_objects()
{
}		       

template <class T> void Site<T>::initialize_member_objects()
{
   n_neighbors=0;
   RHS_neighbor=LHS_neighbor=-1;
   netlink_list_ptr=NULL;
}		       

template <class T> Site<T>::Site()
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

template <class T> Site<T>::Site(T& d) : data(d)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

// Copy constructor:

template <class T> Site<T>::Site(const Site<T>& s)
{
   initialize_member_objects();
   allocate_member_objects();
   docopy(s);
}

template <class T> Site<T>::~Site()
{
   delete netlink_list_ptr;
   netlink_list_ptr=NULL;
}

// ---------------------------------------------------------------------
template <class T> void Site<T>::docopy(const Site<T>& s)
{
   n_neighbors=s.n_neighbors;
   RHS_neighbor=s.RHS_neighbor;
   LHS_neighbor=s.LHS_neighbor;
   data=s.data;
   if (s.netlink_list_ptr != NULL)
   {
      if (netlink_list_ptr == NULL) netlink_list_ptr=new Linkedlist<netlink>;
      *netlink_list_ptr=*(s.netlink_list_ptr);
   }
}

// ---------------------------------------------------------------------
// Overload = operator:

template <class T> Site<T>& Site<T>::operator= (const Site<T>& s)
{
   if (this==&s) return *this;
   docopy(s);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

template <class T> std::ostream& operator<< 
(std::ostream& outstream,const Site<T>& s)
{
   outstream << std::endl;
//   outstream << "Site ID = " << s.data->get_ID() << std::endl;
//   outstream << "*data = " << *(s.data) << std::endl;
   outstream << "n_neighbors = " << s.n_neighbors << std::endl;
   if (s.RHS_neighbor > -1)
   {
      outstream << "RHS_neighbor = " << s.RHS_neighbor << std::endl;
   }
   if (s.LHS_neighbor > -1)
   {
      outstream << "LHS_neighbor = " << s.LHS_neighbor << std::endl;
   }
   
   if (s.netlink_list_ptr==NULL)
   {
      outstream << "Site has no neighbors" << std::endl;
   }
   else
   {
      outstream << "Netlink list:" << std::endl;
      outstream << *(s.netlink_list_ptr) << std::endl;
   }

   return outstream;
}

// ---------------------------------------------------------------------
// Member function display_dereferenced_data is intended to be used to
// display the contents of sites containing pointers to data.  This
// method dereferences and prints out the data to output sitestream.

template <class T> void Site<T>::display_dereferenced_data(
   std::ostream& sitestream) const
{
   sitestream << *data << std::endl;
}

// ---------------------------------------------------------------------
// Member function get_netlink takes in the ID for some site neighbor
// and returns a pointer to the netlink for that neighbor if it
// exists.  Otherwise, this method returns NULL.

template <class T> netlink* Site<T>::get_netlink_ptr(int neighbor_ID)
{
   if (netlink_list_ptr != NULL)
   {
      for (Mynode<netlink>* currnode_ptr=netlink_list_ptr->get_start_ptr();
           currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
      {
         netlink curr_link=currnode_ptr->get_data();
         if (curr_link.get_ID()==neighbor_ID)
         {
            return &(currnode_ptr->get_data());
         }
      } // loop over nodes in netlink list
   }
   return NULL;
}

// ---------------------------------------------------------------------
// Member function get_neighbors returns an STL vector containing the
// integer IDs of all neighbors to the current Site object.

template <class T> std::vector<int> Site<T>::get_neighbors() const
{
   std::vector<int> neighbors;
   if (netlink_list_ptr != NULL)
   {
      for (Mynode<netlink>* currnode_ptr=netlink_list_ptr->get_start_ptr();
           currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
      {
         neighbors.push_back(currnode_ptr->get_data().get_ID());
      } 
   }
   return neighbors;
}

// ---------------------------------------------------------------------
// Utility member function display_neighbor_list prints out the
// contents of the netlink list.  This method was written primarily
// for debugging purposes.

template <class T> void Site<T>::display_neighbor_list()
{
   if (netlink_list_ptr != NULL)
   {
      for (Mynode<netlink>* currnode_ptr=netlink_list_ptr->get_start_ptr();
           currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
      {
         netlink curr_link=currnode_ptr->get_data();
         std::cout << "     Neighbor ID = " << curr_link.get_ID() 
                   << std::endl;
      } // loop over nodes in netlink list
   }
   else
   {
      std::cout << "Netlink list contains no neighbors" << std::endl;
   }
}


