// ==========================================================================
// Header file for templatized network class
// ==========================================================================
// Last modified on 6/23/07; 7/9/07; 5/24/08; 5/26/10; 3/23/14
// ==========================================================================

#ifndef T_NETWORK_H
#define T_NETWORK_H

#include <set>
#include <vector>
#include "datastructures/Hashtable.h"
#include "datastructures/Mynode.h"
#include "math/threevector.h"
#include "network/Site.h"
#include "datastructures/Triple.h"

typedef Triple<std::pair<int,int>,std::pair<int,int>,threevector> 
intersection_triple;

template <class T_ptr>
class Network
{

  public:
   
// Initialization, constructor and destructor functions:

   Network(void);
   Network(int hashtable_capacity);
   Network(const Network<T_ptr>& h);
   virtual ~Network();
   Network<T_ptr>& operator= (Network<T_ptr>& h);

   template <class T1_ptr>
   friend std::ostream& operator<< 
      (std::ostream& outstream,Network<T1_ptr>& n);

// Set & get member functions:

   void set_max_key_value(int max_value);
   int get_max_key_value() const;

   unsigned int size() const;

   Hashtable< Site<T_ptr> >* get_sites_hashtable_ptr();
   const Hashtable< Site<T_ptr> >* get_sites_hashtable_ptr() const;
   Linkedlist<int>* get_entries_list_ptr(); 
   const Linkedlist<int>* get_entries_list_ptr() const; 

   Site<T_ptr>* get_site_ptr(int r);
   const Site<T_ptr>* get_site_ptr(int r) const;

   Site<T_ptr>* get_site_ptr(
      int r,Hashtable< Site<T_ptr> >* curr_sites_hashtable_ptr);
   const Site<T_ptr>* get_site_ptr(
      int r,Hashtable< Site<T_ptr> >* curr_sites_hashtable_ptr) const;

   T_ptr get_site_data_ptr(int r);
   const T_ptr get_site_data_ptr(int r) const;
   T_ptr get_site_data_ptr(
      int r,Hashtable< Site<T_ptr> >* curr_sites_hashtable_ptr);
   const T_ptr get_site_data_ptr(
      int r,Hashtable< Site<T_ptr> >* curr_sites_hashtable_ptr) const;
   netlink* get_netlink_ptr(int r,int q);

// Network generation methods:

   Site<T_ptr>* insert_site(int key,Site<T_ptr> curr_site);
   void append_new_sites(Linkedlist<std::pair<T_ptr,std::vector<int> > >* 
                         new_sites_list_ptr);
   bool neighboring_site(int r,int q);
   void merge_nearby_sites(int r,int q);
   void add_to_neighbor_list(int r,int q);
   void add_symmetric_link(int r,int q);
   void delete_from_neighbor_list(int r,int q);
   void delete_from_neighbor_list(
      int r,int q,Hashtable< Site<T_ptr> >* curr_sites_hashtable_ptr);
   void delete_symmetric_link(int r,int q);
   void delete_symmetric_link(
      int r,int q,Hashtable< Site<T_ptr> >* curr_sites_hashtable_ptr);
   void delete_all_neighbor_links();
   void delete_neighbor_links(int r);
   void delete_neighbor_links(
      int r,Hashtable< Site<T_ptr> >* curr_sites_hashtable_ptr);
   void delete_single_site(int r);
   void delete_single_site(
      int r,Hashtable< Site<T_ptr> >* curr_sites_hashtable_ptr);
   void delete_sites(Linkedlist<int> const *sites_to_delete_list_ptr);
   Site<T_ptr>* insert_site_along_link(Site<T_ptr> new_site,int l1,int l2);
   Site<T_ptr>* insert_site_along_link(
      int key,Site<T_ptr> new_site,int l1,int l2);
   void delete_site_along_link(int key,int l1,int l2);
   bool overlapping_network(Network<T_ptr> const *network2_ptr) const;

// Network neighbor methods:

   void sort_all_site_neighbors();
   void sort_neighbors(int r);
   std::vector<threevector> rel_neighbor_displacements(int r);
   std::vector<double> angles_between_neighboring_links(
      int r,const threevector& r_posn);
   void rearrange_into_RH_cycle(
      const threevector& origin,std::vector<int>& node_label) const;

// Network triangle methods:

   std::vector<Triple<int,int,int> > find_all_triangles();
   void find_site_triangles(
      int r,std::vector<Triple<int,int,int> >& Triangles);

// Network site search methods:

   std::vector<int> generate_npole_list(int n) const;
   Linkedlist<int>* monopole_strand_members(int r_monopole) const;
   int search_for_element(const threevector& posn);
   int search_for_nearby_element(
      const double distance_to_point,const threevector& posn);
   int neighbor_on_left(int r,int r_prev,const threevector& r_hat,
                        bool backtracking_allowed=false) const;
   int neighbor_on_right(int r,int r_prev,const threevector& r_hat,
                         bool backtracking_allowed=false) const;
   int bottom_right_site() const;
   int bottom_right_site_RHS_neighbor() const;

   std::vector<int>* right_hand_loop(
      int r,threevector& e_hat,bool backtracking_allowed=false) const;

   std::vector<std::pair<int,int> > find_all_netlinks() const;
   std::vector<intersection_triple> search_for_intersecting_netlinks() const;
   Site<T_ptr>* insert_site_at_netlink_intersection(
      Site<T_ptr> new_site,intersection_triple& netlink_intersection);
   void merge_close_sites_and_links(double min_distance);

// Network path finding methods:

   std::vector<int> find_nearest_neighbors(int r);
   Linkedlist<int>* generate_leveled_site_neighbor_list(int r,int level);
   std::vector<int> new_generate_leveled_site_neighbor_list(
      int r,int level,Hashtable< Site<T_ptr> >* curr_sites_hashtable_ptr);
   Linkedlist<int>* generate_leveled_site_neighbor_list(
      int r,int level,Hashtable< Site<T_ptr> >* curr_sites_hashtable_ptr);
   int n_links_between_sites(int r,int q);
   Linkedlist<int>* shortest_path_between_sites(
      int r,int q,bool search_for_loops=false);
   Linkedlist<int>* shortest_path_between_sites(
      int r,int q,Hashtable< Site<T_ptr> >* curr_sites_hashtable_ptr,
      bool search_for_loops=false);
   Linkedlist<int>* shortest_path_between_sites_thru_diminished_network(
      int r,int q,int l1,int l2);
   std::vector<int>* find_shortest_loop(int r);
   std::vector<int>* find_shortest_right_hand_loop(int r);

  private: 

// We include integer max_key_value so that assigning
// key=max_key_value+1 is guaranteed to yield a key which has never
// been accessed before.  This is helpful if the network has had sites
// added and subsequently deleted.

   int max_key_value;

// Hashtable *sites_hashtable_ptr holds network site information:

   Hashtable< Site<T_ptr> >* sites_hashtable_ptr;

// Linked list *entries_list_ptr stores the IDs for entries within the
// network in a monotonic increasing order:

   Linkedlist<int>* entries_list_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const Network<T_ptr>& h);

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

template <class T_ptr> inline void Network<T_ptr>::set_max_key_value(
   int max_value) 
{
   max_key_value=max_value;
}

template <class T_ptr> inline int Network<T_ptr>::get_max_key_value() const
{
   return max_key_value;
}

template <class T_ptr> inline unsigned int Network<T_ptr>::size() const
{
   if (sites_hashtable_ptr != NULL)
   {
      return sites_hashtable_ptr->size();
   }
   else
   {
      return 0;
   }
}

// ---------------------------------------------------------------------
template <class T_ptr> inline Hashtable< Site<T_ptr> >* Network<T_ptr>::
get_sites_hashtable_ptr()
{
   return sites_hashtable_ptr;
}

template <class T_ptr> inline const Hashtable< Site<T_ptr> >* Network<T_ptr>::
get_sites_hashtable_ptr() const
{
   return sites_hashtable_ptr;
}

// ---------------------------------------------------------------------
template <class T_ptr> inline Linkedlist<int>* Network<T_ptr>::
get_entries_list_ptr()
{
   return entries_list_ptr;
}

template <class T_ptr> inline const Linkedlist<int>* Network<T_ptr>::
get_entries_list_ptr() const
{
   return entries_list_ptr;
}

// ---------------------------------------------------------------------
template <class T_ptr> inline Site<T_ptr>* Network<T_ptr>::get_site_ptr(int r)
{
   return get_site_ptr(r,sites_hashtable_ptr);
}

template <class T_ptr> inline const Site<T_ptr>* Network<T_ptr>::get_site_ptr(
   int r) const
{
   return get_site_ptr(r,sites_hashtable_ptr);
}

template <class T_ptr> inline Site<T_ptr>* Network<T_ptr>::get_site_ptr(
   int r,Hashtable< Site<T_ptr> >* curr_sites_hashtable_ptr) 
{
   Mynode< Site<T_ptr> >* currnode_ptr=curr_sites_hashtable_ptr->
      retrieve_key(r);
   if (currnode_ptr != NULL)
   {
      return &(currnode_ptr->get_data());
   }
   else
   {
      return NULL;
   }
}

template <class T_ptr> inline const Site<T_ptr>* Network<T_ptr>::get_site_ptr(
   int r,Hashtable< Site<T_ptr> >* curr_sites_hashtable_ptr) const
{
   Mynode< Site<T_ptr> > const *currnode_ptr=curr_sites_hashtable_ptr->
      retrieve_key(r);
   if (currnode_ptr != NULL)
   {
      return &(currnode_ptr->get_data());
   }
   else
   {
      return NULL;
   }
}

// ---------------------------------------------------------------------
template <class T_ptr> inline T_ptr Network<T_ptr>::get_site_data_ptr(int r)
{
   return get_site_data_ptr(r,sites_hashtable_ptr);
}

template <class T_ptr> inline const T_ptr Network<T_ptr>::get_site_data_ptr(
   int r) const
{
   return get_site_data_ptr(r,sites_hashtable_ptr);
}

template <class T_ptr> inline T_ptr Network<T_ptr>::get_site_data_ptr(
   int r,Hashtable< Site<T_ptr> >* curr_sites_hashtable_ptr)
{
   Mynode< Site<T_ptr> >* currnode_ptr=curr_sites_hashtable_ptr->
      retrieve_key(r);
   if (currnode_ptr != NULL)
   {
      return (currnode_ptr->get_data()).get_data();
   }
   else
   {
      return NULL;
   }
}


template <class T_ptr> inline const T_ptr Network<T_ptr>::get_site_data_ptr(
   int r,Hashtable< Site<T_ptr> >* curr_sites_hashtable_ptr) const
{
   Mynode< Site<T_ptr> >* currnode_ptr=curr_sites_hashtable_ptr->
      retrieve_key(r);
   if (currnode_ptr != NULL)
   {
      return (currnode_ptr->get_data()).get_data();
   }
   else
   {
      return NULL;
   }
}

// ---------------------------------------------------------------------
template <class T_ptr> inline netlink* Network<T_ptr>::get_netlink_ptr(
   int r,int q)
{
   Site<T_ptr>* curr_site_ptr=get_site_ptr(r);
   return curr_site_ptr->get_netlink_ptr(q);
}

#include "Network.cc"

#endif  // T_Network.h





