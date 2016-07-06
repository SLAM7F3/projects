// Note added on 6/14/06: All methods involving geometry objects will
// have to be moved out of src/network and into src/geometry to avoid
// linker problems !!!

// ==========================================================================
// Templatized Network class member function definitions
// ==========================================================================
// Last modified on 6/23/07; 5/26/08; 12/4/10
// ==========================================================================

#include <algorithm>
#include <iostream>
#include "math/basic_math.h"
#include "math/constants.h"
#include "math/constant_vectors.h"
#include "geometry/geometry_funcs.h"
#include "geometry/linesegment.h"
#include "math/mathfuncs.h"
#include "templates/mytemplates.h"
#include "general/outputfuncs.h"

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:
// ---------------------------------------------------------------------

template <class T_ptr> void Network<T_ptr>::allocate_member_objects() 
{
}

template <class T_ptr> void Network<T_ptr>::initialize_member_objects() 
{
   max_key_value=-1;
   sites_hashtable_ptr=NULL;
   entries_list_ptr=NULL;
}

template <class T_ptr> Network<T_ptr>::Network(void)
{
   allocate_member_objects();
   initialize_member_objects();
}

template <class T_ptr> Network<T_ptr>::Network(int hashtable_capacity)
{
   allocate_member_objects();
   initialize_member_objects();
   sites_hashtable_ptr=new Hashtable< Site<T_ptr> >(hashtable_capacity);
   entries_list_ptr=new Linkedlist<int>;
}

// Copy constructor:

template <class T_ptr> Network<T_ptr>::Network(const Network<T_ptr>& n)
{
   allocate_member_objects();
   initialize_member_objects();
   docopy(n);
}

template <class T_ptr> Network<T_ptr>::~Network()
{

// Since the network class is NOT responsible for dynamically
// allocating objects and since it only stores pointers, we have
// followed Vadim's advice of not making the network class responsible
// for deleting the dynamic objects.  Instead, a call should be made
// to Networkfunc::delete_dynamically_allocated_objects() BEFORE this
// network destructor is called.

// In some cases, we may NOT want to destroy the dynamically allocated
// objects if they are being accessed by some other network, for
// instance.  In those cases,
// Networkfunc::delete_dynamically_allocated_objects() should only be
// called at the very end when all but the last network accessing the
// objects have already been destroyed...

//   std::cout << "inside Network destructor" << std::endl;

   delete sites_hashtable_ptr;

//   std::cout << "Before deleting entries_list_ptr" << std::endl;
   delete entries_list_ptr;

   sites_hashtable_ptr=NULL;
   entries_list_ptr=NULL;

//   std:: cout << "At end of Network destructor" << std::endl;
}

// ---------------------------------------------------------------------
template <class T_ptr> void Network<T_ptr>::docopy(const Network<T_ptr>& n)
{
   max_key_value=n.max_key_value;
   if (n.get_sites_hashtable_ptr() != NULL)
   {
      if (sites_hashtable_ptr==NULL)
      {
         sites_hashtable_ptr=new Hashtable< Site<T_ptr> >(
            *(n.get_sites_hashtable_ptr()));
      }
      else
      {
         *sites_hashtable_ptr=*(n.get_sites_hashtable_ptr());
      }
      if (entries_list_ptr==NULL)
      {
         entries_list_ptr=new Linkedlist<int>(*n.get_entries_list_ptr());
      }
      else
      {
         *entries_list_ptr=*n.get_entries_list_ptr();
      }
   }
}	

// ---------------------------------------------------------------------
// Overload = operator:

template <class T_ptr> Network<T_ptr>& Network<T_ptr>::operator= 
(Network<T_ptr>& n)
{
   if (this==&n) return *this;
   docopy(n);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

template <class T_ptr> std::ostream& operator<< 
(std::ostream& outstream,Network<T_ptr>& n)
{
   outstream << "max_key_value = " << n.max_key_value << std::endl;
   outstream << "nkeys in sites hashtable = "
             << n.get_sites_hashtable_ptr()->size() << std::endl;
   outstream << "n_nodes in entries list = "
             << n.get_entries_list_ptr()->size() << std::endl;
   outstream << std::endl;
   for (int r=0; r<=n.max_key_value; r++)
   {
      if (n.get_site_ptr(r) != NULL) 
      {
         outstream << "-----------------------------------------------------"
                   << std::endl;
         outstream << "r = " << r << " " 
                   << *(n.get_site_ptr(r)) << std::endl;
         outstream << "n.get_site_data_ptr(r) = " 
                   << n.get_site_data_ptr(r) << std::endl;
         outstream << "data = " << *(n.get_site_data_ptr(r)) << std::endl
                   << std::endl;
      }
   }
   return outstream;
}

// ---------------------------------------------------------------------
// Member function merge_close_sites_and_links computes the minimum
// distance between every network link and every network site which
// does not correspond to the link's endpoints r and q.  If the
// distance to some site p is less than input parameter min_distance,
// the link between r and q is dissolved.  New links between p & r and
// p & q are formed if they do not exist already within the network.

template <class T_ptr> void Network<T_ptr>::merge_close_sites_and_links(
   double min_distance)
{
   outputfunc::write_banner("Merging close sites and netlinks:");

   std::vector<std::pair<int,int> > netlinks=find_all_netlinks();
   for (unsigned int i=0; i<netlinks.size(); i++)
   {
      int r=netlinks[i].first;
      int q=netlinks[i].second;
      threevector r_posn(get_site_data_ptr(r)->get_posn());      
      threevector q_posn(get_site_data_ptr(q)->get_posn());      
      linesegment l(r_posn,q_posn);

      for (Mynode<int>* currnode_ptr=entries_list_ptr->get_start_ptr(); 
           currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
      {
         int p=currnode_ptr->get_data(); // network element number
         if (p != r && p != q)
         {
            threevector p_posn(get_site_data_ptr(p)->get_posn());      
            threevector closest_point_on_l;
            double distance=l.point_to_line_segment_distance(
               p_posn,closest_point_on_l);

            if (distance < min_distance)
            {
               std::cout << "Merging p = " << p 
                         << " with segment defined by r = " << r 
                         << " & q = " << q << std::endl;
               delete_symmetric_link(r,q);
               if (!neighboring_site(p,r)) add_symmetric_link(p,r);
               if (!neighboring_site(p,q)) add_symmetric_link(p,q);
            } // distance < min_distance conditional
         } // p != r and p != q conditional
      } // loop over nodes in *entries_list_ptr
   } // loop over index i labeling netlinks
}

// ==========================================================================
// Network path finding methods
// ==========================================================================

template <class T_ptr> Site<T_ptr>* Network<T_ptr>::insert_site(
   int key,Site<T_ptr> curr_site)
{
   Mynode< Site<T_ptr> >* currnode_ptr=sites_hashtable_ptr->
      insert_key(key,curr_site);
   entries_list_ptr->append_node(key);
   max_key_value=basic_math::max(key,max_key_value);
   return &(currnode_ptr->get_data());
}

// ---------------------------------------------------------------------
// Member function append_new_sites takes in a linked list of pairs of
// site object pointers and STL vectors of integer ID labels
// containing neighbor connectivity information.  For each node within
// the linked list, this method creates a new key equal to
// max_key_value+1, instantiates a new network site and inserts that
// site into the network.  This method enables users to make
// significant additions to networks after they have already been
// formed.

template <class T_ptr> void Network<T_ptr>::append_new_sites(
   Linkedlist<std::pair<T_ptr,std::vector<int> > >* new_sites_list_ptr)
{
   for (Mynode<std::pair<T_ptr,std::vector<int> > >* new_node_ptr=
           new_sites_list_ptr->get_start_ptr(); new_node_ptr != NULL; 
        new_node_ptr=new_node_ptr->get_nextptr())
   {
      std::pair<T_ptr,std::vector<int> > p=new_node_ptr->get_data();
      int key=max_key_value+1;
      insert_site(key,Site<T_ptr>(p.first));
      for (unsigned int i=0; i<p.second.size(); i++)
      {
         add_symmetric_link(key,p.second[i]);
      }
   }
}

// ---------------------------------------------------------------------
// Boolean method neighboring_site returns true if site q is listed as
// a neighbor in site r's *netlink_list_ptr linked list.

template <class T_ptr> bool Network<T_ptr>::neighboring_site(int r,int q)
{
   bool q_is_neighbor=false;
   Site<T_ptr>* curr_site_ptr=get_site_ptr(r);
   if (curr_site_ptr != NULL)
   {
      Linkedlist<netlink>* curr_netlink_list_ptr
         =curr_site_ptr->get_netlink_list_ptr();
      if (curr_netlink_list_ptr != NULL)
      {
         if (curr_netlink_list_ptr->data_in_list(netlink(q)) != NULL)
         {
            q_is_neighbor=true;
         }
      }
   }
   return q_is_neighbor;
}

// ---------------------------------------------------------------------
// Method merge_nearby_sites takes in two site IDs r and q whose
// positions are assumed to be physically close.  This method first
// deletes r from q from each other's neighbor lists (though r and q
// are not necessarily neighbors).  It next establishes new links
// between r and all of q's neighbors.  Finally, site q is deleted
// from the network.  Site r then effectively becomes the union of the
// original r and q sites.

template <class T_ptr> void Network<T_ptr>::merge_nearby_sites(int r,int q)
{
   delete_symmetric_link(r,q);

// Establish new link between site r and all of q's neighbors:

   Linkedlist<netlink>* q_netlink_list_ptr
      =get_site_ptr(q)->get_netlink_list_ptr();
   for (Mynode<netlink>* netlink_node_ptr=q_netlink_list_ptr->
           get_start_ptr(); netlink_node_ptr != NULL;
        netlink_node_ptr=netlink_node_ptr->get_nextptr())
   {
      int p=netlink_node_ptr->get_data().get_ID();
      add_symmetric_link(p,r);
   } // loop over *q_netlink_list_ptr

// Finally, delete site q from network:

   delete_single_site(q);
}
   
// ---------------------------------------------------------------------
// Method add_to_neighbor_list adds site q as a neighbor in site r's
// *netlink_list_ptr linked list if it does not already exist.

template <class T_ptr> void Network<T_ptr>::add_to_neighbor_list(int r,int q)
{
   Site<T_ptr>* curr_site_ptr=get_site_ptr(r);
   if (curr_site_ptr != NULL)
   {
      Linkedlist<netlink>* curr_netlink_list_ptr
         =curr_site_ptr->get_netlink_list_ptr();

// Dynamically generate new neighbor ID list if one does not already exist:

      if (curr_netlink_list_ptr==NULL)
      {
         curr_netlink_list_ptr=new Linkedlist<netlink>;
         curr_site_ptr->set_netlink_list_ptr(curr_netlink_list_ptr);
      }

      if (curr_netlink_list_ptr->data_in_list(netlink(q))==NULL)
      {
         curr_netlink_list_ptr->append_node(netlink(q));
         curr_site_ptr->increment_n_neighbors();
      }
   }
}

// ---------------------------------------------------------------------
// Method add_symmetric_link takes in ID labels r and q for two sites.
// It adds q to the *netlink_list_ptr linked list for site r, and
// vice-versa.

template <class T_ptr> void Network<T_ptr>::add_symmetric_link(int r,int q)
{
   add_to_neighbor_list(r,q);
   add_to_neighbor_list(q,r);
}

// ---------------------------------------------------------------------
// Member function delete_from_neighbor_list removes site q from the
// netlink list for site r.

template <class T_ptr> void Network<T_ptr>::delete_from_neighbor_list(
   int r,int q)
{
   delete_from_neighbor_list(r,q,sites_hashtable_ptr);
}

template <class T_ptr> void Network<T_ptr>::delete_from_neighbor_list(
   int r,int q,Hashtable< Site<T_ptr> >* curr_sites_hashtable_ptr)
{
   Site<T_ptr>* curr_site_ptr=get_site_ptr(r,curr_sites_hashtable_ptr);
   if (curr_site_ptr != NULL)
   {
      Linkedlist<netlink>* curr_netlink_list_ptr
         =curr_site_ptr->get_netlink_list_ptr();

      if (curr_netlink_list_ptr != NULL)
      {
         Mynode<netlink>* netlink_node_ptr=
            curr_netlink_list_ptr->data_in_list(netlink(q));
         if (netlink_node_ptr != NULL)
         {
            curr_netlink_list_ptr->delete_node(netlink_node_ptr);
            curr_site_ptr->decrement_n_neighbors();
         }
      }
   }
}

// ---------------------------------------------------------------------
// Member function delete_symmetric_link takes in ID labels r and q
// for two sites in the sites hashtable.  It deletes site q from the
// netlink list for site r, and vice-versa.

template <class T_ptr> void Network<T_ptr>::delete_symmetric_link(int r,int q)
{
   delete_symmetric_link(r,q,sites_hashtable_ptr);
}

template <class T_ptr> void Network<T_ptr>::delete_symmetric_link(
   int r,int q,Hashtable< Site<T_ptr> >* curr_sites_hashtable_ptr)
{
   delete_from_neighbor_list(r,q,curr_sites_hashtable_ptr);
   delete_from_neighbor_list(q,r,curr_sites_hashtable_ptr);
}

// ---------------------------------------------------------------------
// Member function delete_all_neighbor_links scans through the entire
// network and deletes every site's links to all of its neighbors.

template <class T_ptr> void Network<T_ptr>::delete_all_neighbor_links()
{
   for (Mynode<int>* currnode_ptr=entries_list_ptr->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int r=currnode_ptr->get_data(); // network element number
      delete_neighbor_links(r);
   }
}

// ---------------------------------------------------------------------
// Method function delete_neighbor_links deletes all links between
// site r and its neighbors.

template <class T_ptr> void Network<T_ptr>::delete_neighbor_links(int r)
{
   delete_neighbor_links(r,sites_hashtable_ptr);
}

template <class T_ptr> void Network<T_ptr>::delete_neighbor_links(
   int r,Hashtable< Site<T_ptr> >* curr_sites_hashtable_ptr)
{
   Site<T_ptr>* curr_site_ptr=get_site_ptr(r,curr_sites_hashtable_ptr);
   if (curr_site_ptr != NULL)
   {
      Linkedlist<netlink>* curr_netlink_list_ptr
         =curr_site_ptr->get_netlink_list_ptr();
      if (curr_netlink_list_ptr != NULL)
      {
         Mynode<netlink>* netlink_node_ptr=curr_netlink_list_ptr->
            get_start_ptr();
         while (netlink_node_ptr != NULL)
         {
            Mynode<netlink>* next_netlink_node_ptr=
               netlink_node_ptr->get_nextptr();
            int q=netlink_node_ptr->get_data().get_ID();
            delete_symmetric_link(r,q,curr_sites_hashtable_ptr);
            netlink_node_ptr=next_netlink_node_ptr;
         }
      } // curr_netlink_list_ptr != NULl conditional
   } // curr_site_ptr != NULL conditional
}

// ---------------------------------------------------------------------
// Member function delete_single_site deletes all previously generated
// dynamic links between site r and its nearest neighbors.  It then
// deletes site r itself from the network.

template <class T_ptr> void Network<T_ptr>::delete_single_site(int r)
{
   delete_single_site(r,sites_hashtable_ptr);
}

template <class T_ptr> void Network<T_ptr>::delete_single_site(
   int r,Hashtable< Site<T_ptr> >* curr_sites_hashtable_ptr)
{
   delete_neighbor_links(r,curr_sites_hashtable_ptr);
   
   Site<T_ptr>* curr_site_ptr=get_site_ptr(r,curr_sites_hashtable_ptr);
   if (curr_site_ptr != NULL)
   {
      curr_sites_hashtable_ptr->delete_key(r);
      entries_list_ptr->delete_node(entries_list_ptr->data_in_list(r));
   } // curr_site_ptr != NULL conditional
}

// ---------------------------------------------------------------------
// Member function delete_sites takes in a linked list of integer IDs
// for sites which are to be deleted from the network.  It calls
// delete_single_site() for each one of the nodes within the input
// linked list.  

template <class T_ptr> void Network<T_ptr>::delete_sites(
   Linkedlist<int> const *sites_to_delete_list_ptr)
{
   for (Mynode<int> const *currnode_ptr=sites_to_delete_list_ptr->
           get_start_ptr(); currnode_ptr != NULL; currnode_ptr=
           currnode_ptr->get_nextptr())
   {
      int r=currnode_ptr->get_data(); // site number
      delete_single_site(r);
   }
}

// ---------------------------------------------------------------------
// Member function insert_site_along_link takes in site IDs l1 and l2.
// If these two sites are network neighbors, this method inserts a new
// site into the network, forms links between the new site and l1 &
// l2, and dissolves the link between l1 and l2.

template <class T_ptr> Site<T_ptr>* Network<T_ptr>::insert_site_along_link(
   Site<T_ptr> new_site,int l1,int l2)
{
   return insert_site_along_link(max_key_value+1,new_site,l1,l2);
}

template <class T_ptr> Site<T_ptr>* Network<T_ptr>::insert_site_along_link(
   int key,Site<T_ptr> new_site,int l1,int l2)
{
   Site<T_ptr>* new_site_ptr=NULL;
   if (neighboring_site(l1,l2))
   {
      new_site_ptr=insert_site(key,new_site);
      new_site_ptr->get_data()->set_ID(key);
      add_symmetric_link(key,l1);
      add_symmetric_link(key,l2);
      delete_symmetric_link(l1,l2);
   }
   return new_site_ptr;
}

// ---------------------------------------------------------------------
// Member function delete_site_along_link

template <class T_ptr> void Network<T_ptr>::delete_site_along_link(
   int key,int l1,int l2)
{
   delete_single_site(key);
   add_symmetric_link(l2,l1);
}


// ---------------------------------------------------------------------
// Boolean member function overlapping_network returns true if either
// the hashtable or linked list part of input network *network2_ptr
// overlaps with their counterparts within the current network.  This
// method can be used to search for memory clobbers.

template <class T_ptr> bool Network<T_ptr>::overlapping_network(
   Network<T_ptr> const *network2_ptr) const
{
   bool overlap_flag=false;
   if (sites_hashtable_ptr->overlapping_table(
      network2_ptr->get_sites_hashtable_ptr())) overlap_flag=true;
   if (entries_list_ptr->overlapping_list(
      network2_ptr->get_entries_list_ptr())) overlap_flag=true;
   return overlap_flag;
}

// ==========================================================================
// Network neighbor methods
// ==========================================================================

// Member function sort_all_site_neighbors loops over all sites within
// the network.  It rearranges every site's list of neighbor links so
// that they all form clockwise cycles.

template <class T_ptr> void Network<T_ptr>::sort_all_site_neighbors()
{
   outputfunc::write_banner("Sorting all site neighbors:");
   for (Mynode<int>* currnode_ptr=entries_list_ptr->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int r=currnode_ptr->get_data(); // network element number
      sort_neighbors(r);
   }
}

// ---------------------------------------------------------------------
// Member function sort_neighbors takes in site ID r.  It computes the
// angles which all neighbors of r make relative to +x_hat.  This
// method subsequently sorts the angles so that they are rearranged in
// ascending order, and it makes the corresponding changes to
// *netlink_list_ptr for site r.

template <class T_ptr> void Network<T_ptr>::sort_neighbors(int r)
{
   Linkedlist<netlink>* netlink_list_ptr=get_site_ptr(r)->
      get_netlink_list_ptr();
   if (netlink_list_ptr != NULL)
   {
      double theta[netlink_list_ptr->size()];
   
      int i=0;
      threevector r_posn(get_site_data_ptr(r)->get_posn());
      for (Mynode<netlink>* netlink_node_ptr=netlink_list_ptr->
              get_start_ptr(); netlink_node_ptr != NULL;
           netlink_node_ptr=netlink_node_ptr->get_nextptr())
      {
         int q=netlink_node_ptr->get_data().get_ID();      
         threevector q_posn(get_site_data_ptr(q)->get_posn());
         threevector e_hat=(q_posn-r_posn).unitvector();
         theta[i]=mathfunc::angle_between_unitvectors(x_hat,e_hat);
         theta[i]=basic_math::phase_to_canonical_interval(theta[i],0,2*PI);
//         std::cout << "i = " << i << " q = " << q 
//                   << " theta = " << theta[i]*180/PI << std::endl;
         i++;
      } // loop over nodes in *netlink_list_ptr
      netlink_list_ptr->sort_nodes(theta,false);
   } // netlink_list_ptr != NULL conditional
}

// ---------------------------------------------------------------------
// Member function rel_neighbor_displacements takes in integer ID r
// for some site within the network.  It returns an STL vector
// containing displacement vectors for all neighbors measured relative
// to the site r.

/*
template <class T_ptr> std::vector<threevector> 
Network<T_ptr>::rel_neighbor_displacements(int r)
{
   std::vector<threevector> neighbor_displacements;

   Linkedlist<netlink>* netlink_list_ptr=get_site_ptr(r)->
      get_netlink_list_ptr();
   if (netlink_list_ptr != NULL)
   {
      threevector r_posn(get_site_data_ptr(r)->get_posn());      

      for (Mynode<netlink>* netlink_node_ptr=netlink_list_ptr->
              get_start_ptr(); netlink_node_ptr != NULL;
           netlink_node_ptr=netlink_node_ptr->get_nextptr())
      {
         int q=netlink_node_ptr->get_data().get_ID();      
         threevector q_posn(get_site_data_ptr(q)->get_posn());
         neighbor_displacements.push_back(q_posn-r_posn);
      } // loop over *netlink_list_ptr
   } // netlink_list_ptr != NULL conditional
   return neighbor_displacements;
}
*/

template <class T_ptr> std::vector<threevector> 
Network<T_ptr>::rel_neighbor_displacements(int r)
{
   std::vector<threevector> neighbor_displacements;

   Linkedlist<netlink>* netlink_list_ptr=get_site_ptr(r)->
      get_netlink_list_ptr();
   if (netlink_list_ptr != NULL)
   {
      threevector r_posn(get_site_data_ptr(r)->get_posn());      

      for (Mynode<netlink>* netlink_node_ptr=netlink_list_ptr->
              get_start_ptr(); netlink_node_ptr != NULL;
           netlink_node_ptr=netlink_node_ptr->get_nextptr())
      {
         int q=netlink_node_ptr->get_data().get_ID();      
         threevector q_posn(get_site_data_ptr(q)->get_posn());
         neighbor_displacements.push_back(q_posn-r_posn);
      } // loop over *netlink_list_ptr
   } // netlink_list_ptr != NULL conditional
   return neighbor_displacements;
}

// ---------------------------------------------------------------------
// Member function angles_between_neighboring_links takes in integer
// ID r for some site within the network.  It computes the angle theta
// of each neighboring site relative to site r.  This method then
// subtracts angles of adjacent neighbors to compute relative angle
// dtheta.  This relative angle is forced to range from 0 to 2*PI.  An
// STL vector containing dtheta values if returned by this method.

template <class T_ptr> std::vector<double> 
Network<T_ptr>::angles_between_neighboring_links(
   int r,const threevector& r_posn)
{
   std::vector<double> theta;
   std::vector<double> delta_theta;
   Linkedlist<netlink>* netlink_list_ptr=get_site_ptr(r)->
      get_netlink_list_ptr();
   if (netlink_list_ptr != NULL)
   {
      for (Mynode<netlink>* netlink_node_ptr=netlink_list_ptr->
              get_start_ptr(); netlink_node_ptr != NULL;
           netlink_node_ptr=netlink_node_ptr->get_nextptr())
      {
         int q=netlink_node_ptr->get_data().get_ID();      
         threevector q_posn(get_site_data_ptr(q)->get_posn());
         threevector e_hat=(q_posn-r_posn).unitvector();
         double curr_theta=mathfunc::angle_between_unitvectors(x_hat,e_hat);
         theta.push_back(basic_math::phase_to_canonical_interval(
            curr_theta,-PI,PI));
//         std::cout << "q = " << q << " theta = " 
//                   << theta.back()*180/PI << std::endl;
      } // loop over *netlink_list_ptr

      for (unsigned int i=0; i<theta.size(); i++)
      {
         double curr_theta=theta[i];
         double next_theta=theta[modulo(i+1,theta.size())];
         next_theta=basic_math::phase_to_canonical_interval(
            next_theta,curr_theta,curr_theta+2*PI);
         double dtheta=next_theta-curr_theta;

         delta_theta.push_back(basic_math::phase_to_canonical_interval(
            dtheta,-PI,PI));
//         std::cout << "i = " << i << " dtheta = " << dtheta*180/PI 
//                   << std::endl;
      }
   } // netlink_list_ptr != NULL conditional
   return delta_theta;
}

// ---------------------------------------------------------------------
// Member function rearrange_into_RH_cycle takes in an origin point
// along with an STL vector containing integer ID labels for some
// subset of network nodes.  This method computes the angles relative
// to +x_hat for every point in the sub-network measured wrt the
// origin.  It then sorts the points according to their angular
// values.  This method rearranges the entries within node_label so
// that they correspond to a right-handed cycle.

// Note: On 3/15/05, we realized that this method can fail if the
// network nodes lie along a concave polygon !!!

template <class T_ptr> void Network<T_ptr>::rearrange_into_RH_cycle(
   const threevector& origin,std::vector<int>& node_label) const
{
   int n_nodes=node_label.size();
   int label[n_nodes];
   double theta[n_nodes];
   
   for (int i=0; i<n_nodes; i++)
   {
      label[i]=node_label[i];
      threevector node_posn(get_site_data_ptr(label[i])->get_posn());
      threevector e_hat=(node_posn-origin).unitvector();
      theta[i]=mathfunc::angle_between_unitvectors(x_hat,e_hat);
      theta[i]=basic_math::phase_to_canonical_interval(theta[i],0,2*PI);
//         std::cout << "i = " << i << " r = " << r 
//                   << " theta = " << theta[i]*180/PI << std::endl;
   } // loop over node labels
   Quicksort(theta,label,n_nodes);

   node_label.clear();
   for (int i=0; i<n_nodes; i++)
   {
      node_label.push_back(label[i]);
   }
}

// ==========================================================================
// Network triangle methods
// ==========================================================================

// Member function find_all_triangles loops over every site within the
// network.  It searches for all nearest-neighbor triangles.  It
// returns an STL vector of Triples containing site IDs corresponding
// to such triangles.

template <class T_ptr> std::vector<Triple<int,int,int> > 
Network<T_ptr>::find_all_triangles()
{
//   outputfunc::write_banner("Computing all network triangles:");

   std::vector<Triple<int,int,int> > Triangles;
   for (Mynode<int>* currnode_ptr=entries_list_ptr->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int r=currnode_ptr->get_data(); // network element number
      find_site_triangles(r,Triangles);
   }
   return Triangles;
}

// ---------------------------------------------------------------------
// Member function find_site_triangles takes in ID r for some site
// within the network.  It systematically checks every pair (q,p) of
// the site's neighbors to see whether q and p are themselves
// neighbors.  If so, the triple (r,q,p) forms a triangle.  After
// ordering these triples, this method returns an STL vector of all
// unique ordered triples corresponding to nearest-neighbor network
// triangles. 

// We wrote this method in Dec 2005 to convert Delaunay edge
// information into a list of Delaunay triangles.

template <class T_ptr> void Network<T_ptr>::find_site_triangles(
   int r,std::vector<Triple<int,int,int> >& Triangles)
{
   Linkedlist<netlink>* netlink_list_ptr=get_site_ptr(r)->
      get_netlink_list_ptr();
   if (netlink_list_ptr != NULL)
   {
      for (Mynode<netlink>* netlink_node_ptr=netlink_list_ptr->
              get_start_ptr(); netlink_node_ptr != NULL;
           netlink_node_ptr=netlink_node_ptr->get_nextptr())
      {
         int q=netlink_node_ptr->get_data().get_ID();      

         for (Mynode<netlink>* next_netlink_node_ptr=netlink_node_ptr->
                 get_nextptr(); next_netlink_node_ptr != NULL;
              next_netlink_node_ptr=next_netlink_node_ptr->get_nextptr())
         {
            int p=next_netlink_node_ptr->get_data().get_ID();

//            std::cout << "Node r = " << r << " has neighbors q = " << q
//                      << " and p = " << p << std::endl;

            if (neighboring_site(p,q))
            {
//               std::cout << "p = " << p << " and q = " << q 
//                         << " are neighbors" << std::endl;
               
               std::vector<int> triangle_vertices;
               triangle_vertices.push_back(r);
               triangle_vertices.push_back(q);
               triangle_vertices.push_back(p);
               std::sort(triangle_vertices.begin(),triangle_vertices.end());

//               std::cout << triangle_vertices[0] << " "
//                         << triangle_vertices[1] << " "
//                         << triangle_vertices[2] << std::endl;

               Triple<int,int,int> curr_triangle(triangle_vertices[0],
                                                 triangle_vertices[1],
                                                 triangle_vertices[2]);

               bool triangle_previously_found=false;
               for (unsigned int i=0; i<Triangles.size(); i++)
               {
                  if (curr_triangle==Triangles[i]) 
                     triangle_previously_found=true;
               }
               if (!triangle_previously_found) Triangles.push_back(
                  curr_triangle);

//               std::cout << "r = " << r << ", q = " << q
//                         << " and p = " << p << " form triangle"
//                         << std::endl;

            }
         } // loop over "next" nodes in *netlink_list_ptr
      } // loop over nodes in *netlink_list_ptr
   } // netlink_list_ptr != NULL conditional
}

// ==========================================================================
// Network site search methods
// ==========================================================================

// Member function generate_npole_list returns an STL vector
// containing IDs for sites which have exactly n neighbors.

template <class T_ptr> std::vector<int> Network<T_ptr>::generate_npole_list(
   int n) const
{
   std::vector<int> npole_list;
   for (const Mynode<int>* currnode_ptr=entries_list_ptr->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int r=currnode_ptr->get_data(); // network element number
      if (get_site_ptr(r)->get_n_neighbors()==n)
      {
         npole_list.push_back(r);
      }
   } // loop over *entries_list_ptr
   return npole_list;
}

// ---------------------------------------------------------------------
// Member function monopole_strand_members takes in the ID label
// r_monopole for some site which is assumed to represent a genuine
// monopole.  It follows the path leading from this monopole site to a
// site which has at least 3 neighbors.  All of the sites between the
// monopole and the tri-neighbor site form a "monopole strand".  A
// dynamically generated STL vector containing the IDs for the sites
// within the monopole strand is returned by this method.

template <class T_ptr> Linkedlist<int>* 
Network<T_ptr>::monopole_strand_members(int r_monopole) const
{
   int r=r_monopole;
   int r_next=-1;
   Linkedlist<int>* strand_list_ptr=new Linkedlist<int>;

   bool found_strand_end=false;
   while (!found_strand_end)
   {
//      cout << "strand_list = " << *strand_list_ptr << endl;
//      outputfunc::enter_continue_char();

      Site<T_ptr> const *curr_site_ptr=get_site_ptr(r);
      int n_neighbors=curr_site_ptr->get_n_neighbors();
      std::vector<int> neighbors=curr_site_ptr->get_neighbors();  
//      std::cout << "site r = " << r << " has n_neighbors = " << n_neighbors 
//                << std::endl;

      if (n_neighbors==0)
      {
         std::cout << "n_neighbors = 0 in Linkedlist::monopole_strand_mem"
                   << std::endl;
         found_strand_end=true;
      }
      else if (n_neighbors==1)
      {
         r_next=neighbors[0];

// If entire network consists of a single strand, r_next will
// eventually coincide with an entry within *strand_list_ptr.  We must
// check for this possibility:

         if (strand_list_ptr->data_in_list(r_next)==NULL)
         {
            strand_list_ptr->append_node(r);
         }
         else
         {
            found_strand_end=true;
         }
      }
      else if (n_neighbors==2)
      {
         Mynode<int>* zeronode_ptr=
            strand_list_ptr->data_in_list(neighbors[0]);
         Mynode<int>* onenode_ptr=
            strand_list_ptr->data_in_list(neighbors[1]);
         if (zeronode_ptr==NULL && onenode_ptr==NULL)
         {
            std::cout << "Error in Network::monopole_strand_members()" 
                      << std::endl;
            std::cout << "Both zeronode and onenode = NULL" << std::endl;
            exit(-1);
         }
         else if (zeronode_ptr != NULL && onenode_ptr != NULL)
         {
            found_strand_end=true;
         }
         else if (zeronode_ptr != NULL && onenode_ptr==NULL)
         {
            r_next=neighbors[1];
            strand_list_ptr->append_node(r);
         }
         else if (zeronode_ptr == NULL && onenode_ptr != NULL)
         {
            r_next=neighbors[0];
            strand_list_ptr->append_node(r);
         }
      }
      else if (n_neighbors >= 3)
      {
         found_strand_end=true;
      }
      r=r_next;
//      std::cout << "r_next = " << r_next << std::endl;
   } // !found_strand_end while loop
   
   return strand_list_ptr;
}

// ---------------------------------------------------------------------
// Member function search_for_nearby_element takes in threevector posn.
// It performs a brute-force search over the network for any element
// which is located within distance_to_point of posn.  If such an
// element is found, this method returns its identifying integer
// label.

template <class T_ptr> int Network<T_ptr>::search_for_element(
   const threevector& posn)
{
   const double TINY=1E-4;
   return search_for_nearby_element(TINY,posn);
}

template <class T_ptr> int Network<T_ptr>::search_for_nearby_element(
   const double distance_to_point,const threevector& posn)
{
   int network_element_ID=-1;
   for (Mynode<int>* currnode_ptr=entries_list_ptr->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int r=currnode_ptr->get_data(); // network element number
      T_ptr curr_element_ptr=get_site_data_ptr(r);
      if ((curr_element_ptr->get_posn()-posn).magnitude() < distance_to_point)
      {
         network_element_ID=r;
         break;
      }
   } // loop over index r labeling roadpoint number
   return network_element_ID;
}

// ---------------------------------------------------------------------
// Member function neighbor_on_left takes in integer ID r for some
// element within the network as well as direction vector r_hat.
// r_hat is usually the direction vector from the preceding neighbor
// to the right of element r.  But it can also be an outward radial
// direction vector pointing from some interior origin point towards
// element r.  This method scans over every neighbor q of element r
// and effectively computes the line segment between r and q.  It
// subsequently computes the angle between the line segment and r_hat.
// This method returns the q value for which angle theta is maximal.
// The angular relationship between elements r and q obey a right-hand
// rule.

template <class T_ptr> int Network<T_ptr>::neighbor_on_left(
   int r,int r_prev,const threevector& r_hat,bool backtracking_allowed) const
{
   const Site<T_ptr>* curr_site_ptr=get_site_ptr(r);
   T_ptr curr_data_ptr=curr_site_ptr->get_data();
   const Linkedlist<netlink>* netlink_list_ptr=
      curr_site_ptr->get_netlink_list_ptr();   
//   std::cout << "netlink_list = " << *netlink_list_ptr << std::endl;
   
   int left_node_ID=-1;
   if (netlink_list_ptr != NULL)
   {
      double max_theta=NEGATIVEINFINITY;
      for (const Mynode<netlink>* currnode_ptr=
              netlink_list_ptr->get_start_ptr(); currnode_ptr != NULL; 
           currnode_ptr=currnode_ptr->get_nextptr())
      {
         netlink curr_link(currnode_ptr->get_data());
         int q=curr_link.get_ID();

// For loop searching purposes, we do not want the next neighbor to
// ever backtrack onto the previous neighbor.  So we implement the
// following conditional which guarantees that such backtracking is
// avoided:

         if (backtracking_allowed ||
             (!backtracking_allowed && q != r_prev))
         {
            T_ptr neighbor_data_ptr=get_site_data_ptr(q);
            threevector e_hat=(
               neighbor_data_ptr->get_posn()-curr_data_ptr->get_posn()).
               unitvector();
            double theta=mathfunc::angle_between_unitvectors(r_hat,e_hat);
          
// To avoid wrap-around problems, subtract 2*PI from theta values
// exceeding 179 degrees:

            if (theta*180/PI > 179) theta -= 2*PI;

            if (theta > max_theta)
            {
               max_theta=theta;
               left_node_ID=q;
            }

//         std::cout << "q = " << q << std::endl;
//         std::cout << "sgn_theta = " << sgn_theta << std::endl;
//         std::cout << "r_hat = " << r_hat << endl;
//         std::cout << "e_hat = " << e_hat << endl;
//         std::cout << "r_hat x e_hat = " << cross_product << std::endl;
//         std::cout << "theta = " << theta*180/PI << std::endl;
//         std::cout << "max_theta = " << max_theta*180/PI << std::endl;
//         std::cout << "left_node_ID = " << left_node_ID << std::endl;
         } // q != r_prev conditional
         
      } // loop over nodes in netlink list
   } // netlink_list_ptr != NULL conditional
//   std::cout << "left_node_ID = " << left_node_ID << std::endl;
   return left_node_ID;
}

// ---------------------------------------------------------------------
// Member function neighbor_on_right takes in site ID labels r and
// r_prev as well as the direction vector r_hat pointing from r_prev
// to r.  It loops over all of r's neighbors (not including r_prev is
// boolean backtracking_allowed flag == false) and computes their
// direction vectors relative to r.  This method chooses that neighbor
// whose direction vector's angle relative to r_hat is minimal.  It
// returns the ID label for that neighbor.

template <class T_ptr> int Network<T_ptr>::neighbor_on_right(
   int r,int r_prev,const threevector& r_hat,bool backtracking_allowed) const
{
   const Site<T_ptr>* curr_site_ptr=get_site_ptr(r);
   T_ptr curr_data_ptr=curr_site_ptr->get_data();
   const Linkedlist<netlink>* netlink_list_ptr=
      curr_site_ptr->get_netlink_list_ptr();   
   
   int right_node_ID=-1;
   if (netlink_list_ptr != NULL)
   {
      double min_theta=POSITIVEINFINITY;
      for (const Mynode<netlink>* currnode_ptr=netlink_list_ptr->
              get_start_ptr(); currnode_ptr != NULL; 
           currnode_ptr=currnode_ptr->get_nextptr())
      {
         netlink curr_link(currnode_ptr->get_data());
         int q=curr_link.get_ID();

// For loop searching purposes, we do not want the next neighbor to
// ever backtrack onto the previous neighbor.  So we implement the
// following conditional which guarantees that such backtracking is
// avoided:

         if (backtracking_allowed ||
             (!backtracking_allowed && q != r_prev))
         {
            T_ptr neighbor_data_ptr=get_site_data_ptr(q);
            threevector e_hat=(
               neighbor_data_ptr->get_posn()-curr_data_ptr->get_posn()).
               unitvector();
            double theta=mathfunc::angle_between_unitvectors(r_hat,e_hat);
          
// To avoid wrap-around problems, add 2*PI to theta values below -179
// degrees:

            if (theta*180/PI < -179) theta += 2*PI;

            if (theta < min_theta)
            {
               min_theta=theta;
               right_node_ID=q;
            }

//         std::cout << "q = " << q << std::endl;
//         std::cout << "sgn_theta = " << sgn_theta << std::endl;
//         std::cout << "r_hat = " << r_hat << endl;
//         std::cout << "e_hat = " << e_hat << endl;
//         std::cout << "r_hat x e_hat = " << cross_product << std::endl;
//         std::cout << "theta = " << theta*180/PI << std::endl;
//         std::cout << "min_theta = " << min_theta*180/PI << std::endl;
//         std::cout << "right_node_ID = " << right_node_ID << std::endl;
         } // q != r_prev conditional
         
      } // loop over nodes in netlink list
   } // netlink_list_ptr != NULL conditional
//   std::cout << "right_node_ID = " << right_node_ID << std::endl;
   return right_node_ID;
}

// ---------------------------------------------------------------------
// Member function bottom_right_site scans through all sites within
// the network and returns the ID for the one whose y value is most
// minimal.  If more than one site has this minimal y value, it
// returns the ID for the site whose x value is maximal.

template <class T_ptr> int Network<T_ptr>::bottom_right_site() const
{
   double ymin=POSITIVEINFINITY;
   double xmax=NEGATIVEINFINITY;
   threevector bottom_right_posn;

   int r_extreme=-1;
   for (Mynode<int>* currnode_ptr=entries_list_ptr->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int r=currnode_ptr->get_data(); // network element number
      threevector curr_posn(get_site_data_ptr(r)->get_posn());
      if (curr_posn.get(1) < ymin)
      {
         ymin=curr_posn.get(1);
         bottom_right_posn=curr_posn;
         r_extreme=r;
      }
      else if (nearly_equal(curr_posn.get(1),ymin))
      {
         if (curr_posn.get(0) > xmax)
         {
            xmax=curr_posn.get(0);
            bottom_right_posn=curr_posn;
            r_extreme=r;
         }
      }
   }
   return r_extreme;
}

// ---------------------------------------------------------------------
// Member function bottom_right_site_RHS_neighbor first obtains the ID
// for the site which is located as far to the bottom (and possibly as
// far to the right) as possible within the network.  It then computes
// the angles this site's neighbors make relative to the positive x
// axis.  It returns the ID of the site whose angle is minimal (and
// which therefore lies to the right of the bottom right site).

template <class T_ptr> int Network<T_ptr>::bottom_right_site_RHS_neighbor() 
   const
{
   int r=bottom_right_site();
   threevector r_posn(get_site_data_ptr(r)->get_posn());

   int RHS_neighbor=-1;
   double min_theta=POSITIVEINFINITY;
   const Linkedlist<netlink>* netlink_list_ptr=get_site_ptr(r)->
      get_netlink_list_ptr();
   for (const Mynode<netlink>* netlink_node_ptr=netlink_list_ptr->
           get_start_ptr(); netlink_node_ptr != NULL;
        netlink_node_ptr=netlink_node_ptr->get_nextptr())
   {
      int q=netlink_node_ptr->get_data().get_ID();      
      threevector q_posn(get_site_data_ptr(q)->get_posn());
      threevector e_hat=(q_posn-r_posn).unitvector();
      double theta=mathfunc::angle_between_unitvectors(x_hat,e_hat);
      if (theta < min_theta)
      {
         min_theta=theta;
         RHS_neighbor=q;
      }
   }
   return RHS_neighbor;
}

// ---------------------------------------------------------------------
// Member function right_hand_loop takes in integer ID r for some
// element within the network along with an initial direction vector
// e_hat which is assumed to point towards some neighbor of r.  This
// method traverses the network according to a right-hand rule.  If r
// belongs to a right-handed loop, this method keeps on traversing the
// network until it returns to its starting element.  A dynamically
// generated STL vector containing the integer IDs of all elements
// within the right-handed loop is returned by this method.

template <class T_ptr> std::vector<int>* Network<T_ptr>::right_hand_loop(
   int r,threevector& e_hat,bool backtracking_allowed) const
{
   std::vector<int>* loop_ID_ptr=new std::vector<int>;
   T_ptr curr_element_ptr=get_site_data_ptr(r);

   int q_prev=r;
   int q=r;
   int n_neighbors=0;
   const int max_neighbors=1000;
   while(n_neighbors==0 || (n_neighbors < max_neighbors && q != r))
   {
      loop_ID_ptr->push_back(q);
      n_neighbors++;

//      int q_new=neighbor_on_left(q,q_prev,e_hat,backtracking_allowed);
      int q_new=neighbor_on_right(q,q_prev,e_hat,backtracking_allowed);
      q_prev=q;
      q=q_new;
      T_ptr next_element_ptr=get_site_data_ptr(q);
      e_hat=linesegment(
         curr_element_ptr->get_posn(),next_element_ptr->get_posn()).
         get_ehat();
      curr_element_ptr=next_element_ptr;
//      std::cout << "n_neighbors = " << n_neighbors 
//                << " q = " << q << std::endl;
   } // while loop
   return loop_ID_ptr;
}

// ---------------------------------------------------------------------
// Member function find_all_netlinks loops over every network site
// labels r within *entries_list_ptr.  It finds all neighbor site
// labels q for each site r.  If q > r, this method saves the pair
// (r,q) within an STL vector.  The STL vector is returned at the end
// of this method.

template <class T_ptr> std::vector<std::pair<int,int> > 
Network<T_ptr>::find_all_netlinks() const
{
   std::vector<std::pair<int,int> > netlinks;
   for (Mynode<int>* currnode_ptr=entries_list_ptr->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int r=currnode_ptr->get_data(); // network element number
      const Site<T_ptr>* curr_site_ptr=get_site_ptr(r);

      if (curr_site_ptr != NULL)
      {
         const Linkedlist<netlink>* curr_netlink_list_ptr
            =curr_site_ptr->get_netlink_list_ptr();

         if (curr_netlink_list_ptr != NULL)
         {
            for (const Mynode<netlink>* netlink_node_ptr=
                    curr_netlink_list_ptr->get_start_ptr(); 
                 netlink_node_ptr != NULL;
                 netlink_node_ptr=netlink_node_ptr->get_nextptr())
            {
               int q=netlink_node_ptr->get_data().get_ID();
               if (q > r)
               {
                  netlinks.push_back(std::pair<int,int>(r,q));
//                  std::cout << "r = " << r << " and q = " << q 
//                            << " are linked" << std::endl;
               }
            }
         } // curr_netlink_list_ptr != NULL conditional
      } // curr_site_ptr != NULL conditional
   } // loop over *entries_list_ptr
   return netlinks;
}

// ---------------------------------------------------------------------
// Member function search_for_intersecting_netlinks performs a brute
// force search over a list of netlinks for any two which intersect.
// The fractional distance of the point of intersection is required to
// lie more than 1% away from both netlinks' endpoints.  All such
// netlink intersections are returned within an STL vector of triples
// containing (r,q), (r2,q2) and intersection point position information.

template <class T_ptr> std::vector<intersection_triple>
Network<T_ptr>::search_for_intersecting_netlinks() const
{
   outputfunc::write_banner("Searching for intersecting netlinks:");

   std::vector<intersection_triple> netlink_intersections;
   std::vector<std::pair<int,int> > netlinks=find_all_netlinks();
   for (unsigned int i=0; i<netlinks.size(); i++)
   {
      std::pair<int,int> p(netlinks[i]);
      int r=p.first;
      int q=p.second;
      threevector r_posn(get_site_data_ptr(r)->get_posn());
      threevector q_posn(get_site_data_ptr(q)->get_posn());
      linesegment l(r_posn,q_posn);

      for (unsigned int j=i+1; j<netlinks.size(); j++)
      {
         std::pair<int,int> p2(netlinks[j]);
         int r2=p2.first;
         int q2=p2.second;
         threevector r2_posn(get_site_data_ptr(r2)->get_posn());
         threevector q2_posn(get_site_data_ptr(q2)->get_posn());
         linesegment l2(r2_posn,q2_posn);

         bool intersection_pnt_on_l2,intersection_pnt_on_l;
         threevector intersection_pnt;
         l2.point_of_intersection(
            l,intersection_pnt,intersection_pnt_on_l2,
            intersection_pnt_on_l);
         if (intersection_pnt_on_l && intersection_pnt_on_l2)
         {
            double f=l.frac_distance_along_segment(intersection_pnt);
            double f2=l2.frac_distance_along_segment(intersection_pnt);
            if (f > 0.01 && f < 0.99 && f2 > 0.01 && f2 < 0.99)
            {
//               std::cout << "link 1: r = " << r << " q = " << q
//                         << std::endl;
//               std::cout << "link 2: r2 = " << r2 << " q2 = " << q2
//                         << std::endl;
//               std::cout << "links intersect at " << intersection_pnt 
//                         << std::endl; 
               netlink_intersections.push_back(
                  intersection_triple(
                     std::pair<int,int>(r,q),std::pair<int,int>(r2,q2),
                     intersection_pnt));

            } // f and f2 fraction conditionals
         } // intersection point on l and l2 conditionals
      } // loop over index j
   } // loop over index i

   return netlink_intersections;
}

// ---------------------------------------------------------------------
// Member function insert_site_at_netlink_intersection takes in
// netlink intersection information generated by member function
// search_for_intersecting_netlinks().  It generates a new site within
// the current network object at the position of the netlink
// intersection.  It then establishes links between r, q, r2 and q2
// with the new site.  It also dissolves the links between r & q and
// r2 & q2.  

template <class T_ptr> Site<T_ptr>* 
Network<T_ptr>::insert_site_at_netlink_intersection(
   Site<T_ptr> new_site,intersection_triple& netlink_intersection)
{
   int key=max_key_value+1;

   Site<T_ptr>* new_site_ptr=insert_site(key,new_site);
   new_site_ptr->get_data()->set_ID(key);
   
   int r=netlink_intersection.first.first;
   int q=netlink_intersection.first.second;
   int r2=netlink_intersection.second.first;
   int q2=netlink_intersection.second.second;
   threevector intersection_pnt(netlink_intersection.third);

   add_symmetric_link(key,r);
   add_symmetric_link(key,q);
   delete_symmetric_link(r,q);

   add_symmetric_link(key,r2);
   add_symmetric_link(key,q2);
   delete_symmetric_link(r2,q2);
      
   get_site_data_ptr(key)->set_posn(intersection_pnt);
   return new_site_ptr;
}

// ==========================================================================
// Network path finding methods
// ==========================================================================

// Member function generate_leveled_site_neighbor_list takes in site
// ID number r.  It recursively searches for all neighboring sites
// within the network which are located precisely "level" steps away
// from the rth site.  Backtracking onto a lower level neighbor is
// prohibited.  This method returns a linked list containing the IDs
// for all neighboring sites at the specified level.

template <class T_ptr> std::vector<int> 
Network<T_ptr>::find_nearest_neighbors(int r)
{
   std::vector<int> V;

   Linkedlist<int>* nearest_neighbor_list_ptr=
      generate_leveled_site_neighbor_list(r,1);
   for (Mynode<int>* neighbor_node_ptr=nearest_neighbor_list_ptr->
           get_start_ptr(); neighbor_node_ptr != NULL;
        neighbor_node_ptr=neighbor_node_ptr->get_nextptr())
   {
      V.push_back(neighbor_node_ptr->get_data());
   }
   delete nearest_neighbor_list_ptr;
   return V;
}

template <class T_ptr> Linkedlist<int>* Network<T_ptr>::
generate_leveled_site_neighbor_list(int r,int level)
{
   return generate_leveled_site_neighbor_list(r,level,sites_hashtable_ptr);
}

// ---------------------------------------------------------------------

/*

// On 6/25/07, we tried to reduce the number of dynamic creations and
// destructions within generate_leveled_site_neighbor_list by
// replacing our linkedlist class with STL vectors.  But there are at
// least 3 specialized linkedlist methods which would need to be
// rewritten for STL vectors before such a swap can be performed.  We
// leave these changes for the future... 


template <class T_ptr> std::vector<int> Network<T_ptr>::
new_generate_leveled_site_neighbor_list(
   int r,int level,Hashtable< Site<T_ptr> >* curr_sites_hashtable_ptr)
{
   Site<T_ptr>* curr_site_ptr=get_site_ptr(r,curr_sites_hashtable_ptr);

   std::vector<int> neighbors;
   if (level==0)
   {
      neighbors.push_back(r);
   }
   else if (level==1)
   {
      Linkedlist<netlink>* curr_netlink_list_ptr=
         curr_site_ptr->get_netlink_list_ptr();
      if (curr_netlink_list_ptr != NULL)
      {
         for (Mynode<netlink>* netlink_node_ptr=
                 curr_netlink_list_ptr->get_start_ptr();
              netlink_node_ptr != NULL; 
              netlink_node_ptr=netlink_node_ptr->get_nextptr())
         {
            int q=netlink_node_ptr->get_data().get_ID();
            neighbors.push_back(q);
         }
      } // curr_netlink_list_ptr != NULL conditional
   }
   else
   {
      vector<int> one_neighbors=generate_leveled_site_neighbor_list(
         r,level-1,curr_sites_hashtable_ptr);

      if (one_neighbors.size() > 0)
      {
         for (int i=0; i<one_neighbors.size(); i++)
         {
            int q=one_neighbors[i];
            vector<int> neighbor_neighbor=generate_leveled_site_neighbor_list(
                  q,1,curr_sites_hashtable_ptr);

            neighbor_list_ptr->concatenate_wo_duplication(
               neighbor_neighbor_list_ptr);

         }
      } // one_neighbors_list_ptr != NULL conditional

// Remove nodes from *neighbor_list_ptr(level) which match those
// within *neighbor_list_ptr(level-2):

      vector<int> two_neighbors=generate_leveled_site_neighbor_list(
            r,level-2,curr_sites_hashtable_ptr);

      if (two_neighbors.size() > 0)
      {
         for (int i=0; i<two_neighbors.size(); i++)
         {
            int s=two_neighbors[i];
            Mynode<int>* bogus_neighbor_node_ptr=neighbor_list_ptr->
               data_in_list(s);
            if (bogus_neighbor_node_ptr != NULL)
            {
               neighbor_list_ptr->delete_node(bogus_neighbor_node_ptr);
            }
         }
      } // two_neighbors_list_ptr != NULL conditional
   }
   return neighbors;
}
*/

// ---------------------------------------------------------------------
template <class T_ptr> Linkedlist<int>* Network<T_ptr>::
generate_leveled_site_neighbor_list(
   int r,int level,Hashtable< Site<T_ptr> >* curr_sites_hashtable_ptr)
{
   Site<T_ptr>* curr_site_ptr=get_site_ptr(r,curr_sites_hashtable_ptr);

   Linkedlist<int>* neighbor_list_ptr=new Linkedlist<int>;
   if (level==0)
   {
      neighbor_list_ptr->append_node(r);
   }
   else if (level==1)
   {
      Linkedlist<netlink>* curr_netlink_list_ptr=
         curr_site_ptr->get_netlink_list_ptr();
      if (curr_netlink_list_ptr != NULL)
      {
         for (Mynode<netlink>* netlink_node_ptr=
                 curr_netlink_list_ptr->get_start_ptr();
              netlink_node_ptr != NULL; 
              netlink_node_ptr=netlink_node_ptr->get_nextptr())
         {
            int q=netlink_node_ptr->get_data().get_ID();
            neighbor_list_ptr->append_node(q);
         }
      } // curr_netlink_list_ptr != NULL conditional
   }
   else
   {
      Linkedlist<int>* one_neighbors_list_ptr=
         generate_leveled_site_neighbor_list(
            r,level-1,curr_sites_hashtable_ptr);

      if (one_neighbors_list_ptr != NULL)
      {
         for (Mynode<int>* neighbor_node_ptr=one_neighbors_list_ptr->
                 get_start_ptr(); neighbor_node_ptr != NULL;
              neighbor_node_ptr=neighbor_node_ptr->get_nextptr())
         {
            int q=neighbor_node_ptr->get_data();
            Linkedlist<int>* neighbor_neighbor_list_ptr=
               generate_leveled_site_neighbor_list(
                  q,1,curr_sites_hashtable_ptr);
            neighbor_list_ptr->concatenate_wo_duplication(
               neighbor_neighbor_list_ptr);
            delete neighbor_neighbor_list_ptr;
         }
      } // one_neighbors_list_ptr != NULL conditional
      delete one_neighbors_list_ptr;

// Remove nodes from *neighbor_list_ptr(level) which match those
// within *neighbor_list_ptr(level-2):

      Linkedlist<int>* two_neighbors_list_ptr=
         generate_leveled_site_neighbor_list(
            r,level-2,curr_sites_hashtable_ptr);

      if (two_neighbors_list_ptr != NULL)
      {
         for (Mynode<int>* neighbor_neighbor_node_ptr=two_neighbors_list_ptr->
                 get_start_ptr();
              neighbor_neighbor_node_ptr != NULL;
              neighbor_neighbor_node_ptr=neighbor_neighbor_node_ptr->
                 get_nextptr())
         {
            int s=neighbor_neighbor_node_ptr->get_data();
            Mynode<int>* bogus_neighbor_list_ptr=neighbor_list_ptr->
               data_in_list(s);
            if (bogus_neighbor_list_ptr != NULL)
            {
               neighbor_list_ptr->delete_node(bogus_neighbor_list_ptr);
            }
         }
      } // two_neighbors_list_ptr != NULL conditional
      delete two_neighbors_list_ptr;
   }
   return neighbor_list_ptr;
}

// ---------------------------------------------------------------------
// Member function n_links_between_sites takes in ID's r and q for two
// sites within the network.  It returns the number of links between
// these sites.

template <class T_ptr> int Network<T_ptr>::n_links_between_sites(int r,int q)
{

// First verify that both sites r and q actually exist within the
// network:

   if (get_site_ptr(r) != NULL && get_site_ptr(q) != NULL)
   {
      bool found_ending_site=false;
      int level=0;
      const int max_levels=1000;
      std::vector<Linkedlist<int>*> neighbor_list_ptr(50);

      do 
      {
         neighbor_list_ptr[level]=generate_leveled_site_neighbor_list(
            r,level);
         Mynode<int>* neighbor_node_ptr=
            neighbor_list_ptr[level]->data_in_list(q);

         if (neighbor_node_ptr != NULL) 
         {
            found_ending_site=true;
         }
         else
         {
            level++;
         }
      }
      while(!found_ending_site && level < max_levels);

      for (int l=0; l<level; l++)
      {
         delete neighbor_list_ptr[l];
      }
      return level;
   } // site_ptr(r) && site_ptr(q) != NULL conditional
   else
   {
      std::cout << "Error in Network::n_links_between_sites()" << std::endl;
      std::cout << "Site does not exist within network!" << std::endl;
      return -1;
   }
}

// ---------------------------------------------------------------------
// Member function shortest_path_between_sites takes in ID's r and q
// for two sites within the network.  If both sites exist within the
// network and if at least one path connecting them also exists, this
// method returns a linked list containing site ID's for a shortest
// path (which is generally not unique in the presence of network
// loops) between r and q.   Otherwise, this method returns NULL.

template <class T_ptr> Linkedlist<int>* Network<T_ptr>::
shortest_path_between_sites(int r,int q,bool search_for_loops)
{
   return shortest_path_between_sites(
      r,q,sites_hashtable_ptr,search_for_loops);
}

template <class T_ptr> Linkedlist<int>* Network<T_ptr>::
shortest_path_between_sites(
   int r,int q,Hashtable< Site<T_ptr> >* curr_sites_hashtable_ptr,
   bool search_for_loops)
{
// First verify that both sites r and q actually exist within the
// input hashtable:

   if (get_site_ptr(r,curr_sites_hashtable_ptr) == NULL ||
       get_site_ptr(q,curr_sites_hashtable_ptr) == NULL)
   {
      std::cout << "Error in Network::shortest_path_between_sites()" 
                << std::endl;
      std::cout << "Site does not exist within network!" << std::endl;
      return NULL;
   }
   else
   {
      bool found_ending_site=false;
      bool nopath_between_sites=false;
      
      int level=0;
      const int max_levels=1000;
      std::vector<Linkedlist<int>*> neighbor_list_ptr(50);

      do 
      {
         neighbor_list_ptr[level]=generate_leveled_site_neighbor_list(
            r,level,curr_sites_hashtable_ptr);

         Mynode<int>* neighbor_node_ptr=
            neighbor_list_ptr[level]->data_in_list(q);

         if (neighbor_list_ptr[level]->size()==0)
         {
            nopath_between_sites=true;
         }
         else if (!search_for_loops && neighbor_node_ptr != NULL)
         {
            found_ending_site=true;
         }
         else if (search_for_loops && neighbor_node_ptr != NULL &&
                  level != 0)
         {
            found_ending_site=true;
         }
         else
         {
            level++;
         }
      }
      while(!found_ending_site && !nopath_between_sites && 
            level < max_levels);

      if (nopath_between_sites)
      {
//         std::cout << "No path exists between sites" << std::endl;
         return NULL;
      }
      else
      {
         
// Start with ending site q and search backwards through the network
// for a path connecting it to beginning site r:

         Linkedlist<int>* path_list_ptr=new Linkedlist<int>;
         path_list_ptr->append_node(q);
         int p=q;

         for (int l=1; l<level+1; l++)
         {
            Linkedlist<int>* backwards_neighbor_list_ptr=
               generate_leveled_site_neighbor_list(
                  p,1,curr_sites_hashtable_ptr);
         
            Mynode<int>* currnode_ptr=backwards_neighbor_list_ptr->
               get_start_ptr();
            bool overlapping_element_found=false;
            while (currnode_ptr != NULL && !overlapping_element_found)
            {
               int curr_site=currnode_ptr->get_data();
               if (neighbor_list_ptr[level-l]->data_in_list(curr_site) 
                   != NULL)
               {
                  path_list_ptr->create_and_insert_node(-1,-1,curr_site);
                  p=curr_site;
                  overlapping_element_found=true;
               }
               currnode_ptr=currnode_ptr->get_nextptr();
            }
         } // loop over index l labeling level within network diagram
         //  connecting ending site q to beginning site r

         for (int l=0; l<level; l++)
         {
            delete neighbor_list_ptr[l];
         }

// If search_for_loops flag is enabled, we look for site repetitions
// within the output path list.  Such repetitions indicate that the
// path from site r to itself is a tadpole rather than a pure loop.
// In this tadpole case, we return a new output path list containing
// only site r.

         if (search_for_loops)
         {

// First transfer all nodes' contents except last from path list to a
// vector for subsequent processing:

            std::vector<int>* reduced_path_vector_ptr=path_list_ptr->
               convert_list_to_vector();

            bool tadpole_found=false;
            for (unsigned int n=0; n<reduced_path_vector_ptr->size()-2 
                    && !tadpole_found; n++)
            {
               for (unsigned int m=n+1; m<reduced_path_vector_ptr->size()-1 
                       && !tadpole_found; m++)
               {
                  if ((*reduced_path_vector_ptr)[n] == 
                      (*reduced_path_vector_ptr)[m]) tadpole_found=true;
               }
            }
            delete reduced_path_vector_ptr;

            if (tadpole_found)
            {
               delete path_list_ptr;
               path_list_ptr=new Linkedlist<int>;
               path_list_ptr->append_node(q);

            }
         }
         return path_list_ptr;
      } // nopath_between sites conditional
   } // site_ptr(r) || site_ptr(q) == NULL conditional
}

// ---------------------------------------------------------------------
// Member function shortest_path_between_sites_thru_diminished_network
// takes in ID's r and q for two sites within the network.  It also
// takes in ID's l1 and l2 for a symmetric link to be temporarily
// deleted from the network.  It returns a linked list containing site
// ID's for a shortest path between r and q which does not pass
// through the deleted link.

template <class T_ptr> Linkedlist<int>* Network<T_ptr>::
shortest_path_between_sites_thru_diminished_network(int r,int q,int l1,int l2)
{

// First verify that both sites r and q actually exist within the
// network:

   if (get_site_ptr(r) == NULL || get_site_ptr(q) == NULL ||
       sites_hashtable_ptr==NULL)
   {
      std::cout << 
         "Error in Network::shortest_path_between_sites_thru_diminished_network()" 
                << std::endl;
      std::cout << "Site does not exist within network!" << std::endl;
      return NULL;
   }
   else
   {
      
// Create a copy of *sites_hashtable_ptr hashtable.  Subsequently
// remove link l1-l2 from the copied hashtable:

      Hashtable< Site<T_ptr> >* copy_sites_hashtable_ptr=
         new Hashtable< Site<T_ptr> >(*sites_hashtable_ptr);
      delete_symmetric_link(l1,l2,copy_sites_hashtable_ptr);

      Linkedlist<int>* path_list_ptr=
         shortest_path_between_sites(r,q,copy_sites_hashtable_ptr);
      delete copy_sites_hashtable_ptr;
      return path_list_ptr;
   } // site_ptr(r) || site_ptr(q) == NULL conditional
}

// ---------------------------------------------------------------------
// Member function find_shortest_loop takes in site ID r.  This method
// searches for nontrivial closed loops which connect r back onto
// itself.  It returns a linked list containing site ID's for a
// shortest nontrivial cycle from r back to itself if one exists.
// Otherwise, it returns a trivial list containing only site r.

template <class T_ptr> std::vector<int>* Network<T_ptr>::find_shortest_loop(
   int r)
{

// First verify that site r actually exists within the network:

   if (get_site_ptr(r) == NULL || sites_hashtable_ptr == NULL)
   {
      std::cout << "Error in Network::find_shortest_loop()" 
                << std::endl;
      std::cout << "Site does not exist within network!" << std::endl;
      return NULL;
   }
   else
   {
      Linkedlist<int>* odd_path_list_ptr=shortest_path_between_sites(
         r,r,true);
      Linkedlist<int>* even_path_list_ptr=NULL;
      
      Linkedlist<int>* one_neighbor_list_ptr=
         generate_leveled_site_neighbor_list(r,1);

      if (one_neighbor_list_ptr != NULL)
      {
         Mynode<int>* neighbor_node_ptr=one_neighbor_list_ptr->
            get_start_ptr();
         while (neighbor_node_ptr != NULL)
         {
            int q=neighbor_node_ptr->get_data();
            int extra_site_key=sites_hashtable_ptr->size();

//            Site<T_ptr>* extra_site_ptr=new Site<T_ptr>(extra_site_key);

            Site<T_ptr>* extra_site_ptr=new Site<T_ptr>;
//            T_ptr->set_ID(extra_site_key);

            insert_site_along_link(extra_site_key,*extra_site_ptr,r,q);
            Linkedlist<int>* curr_even_path_list_ptr=
               shortest_path_between_sites(r,r,true);
            delete_site_along_link(extra_site_key,r,q);
            delete extra_site_ptr;

            if (curr_even_path_list_ptr != NULL)
            {
               curr_even_path_list_ptr->delete_node(
                  curr_even_path_list_ptr->data_in_list(extra_site_key));

               if (even_path_list_ptr==NULL &&
                   curr_even_path_list_ptr->size() > 0)
               {
                  even_path_list_ptr=new Linkedlist<int>(
                     *curr_even_path_list_ptr);
               }
               else if (even_path_list_ptr != NULL &&
                        even_path_list_ptr->size()==1 &&
                        curr_even_path_list_ptr->size() > 1)
               {
                  delete even_path_list_ptr;
                  even_path_list_ptr=new Linkedlist<int>(
                     *curr_even_path_list_ptr);
               }
               else if (even_path_list_ptr != NULL &&
                        curr_even_path_list_ptr->size() > 1 &&
                        even_path_list_ptr->size() >
                        curr_even_path_list_ptr->size())
               {
                  delete even_path_list_ptr;
                  even_path_list_ptr=new Linkedlist<int>(
                     *curr_even_path_list_ptr);
               }

               delete curr_even_path_list_ptr;
            } // curr_even_path_list_ptr != NULL

            neighbor_node_ptr=neighbor_node_ptr->get_nextptr();
         } // neighbor_node_ptr != NULL conditional
      } // one_neighbor_list_ptr != NULL conditional

      Linkedlist<int>* path_list_ptr=NULL;
      if (even_path_list_ptr==NULL && odd_path_list_ptr==NULL)
      {
         path_list_ptr=new Linkedlist<int>;
         path_list_ptr->append_node(r);
      }
      else if (even_path_list_ptr==NULL && odd_path_list_ptr != NULL)
      {
         path_list_ptr=new Linkedlist<int>(*odd_path_list_ptr);
      }
      else if (odd_path_list_ptr==NULL && even_path_list_ptr != NULL)
      {
         path_list_ptr=new Linkedlist<int>(*even_path_list_ptr);
      }
      else if (odd_path_list_ptr->size()==1 &&
               even_path_list_ptr->size()==1)
      {
         path_list_ptr=new Linkedlist<int>;
         path_list_ptr->append_node(r);
      }
      else if (odd_path_list_ptr->size()==1 &&
               even_path_list_ptr->size() > 1)
      {
         path_list_ptr=new Linkedlist<int>(*even_path_list_ptr);
      }
      else if (odd_path_list_ptr->size() > 1 &&
               even_path_list_ptr->size()==1)
      {
         path_list_ptr=new Linkedlist<int>(*odd_path_list_ptr);
      }
      else if (odd_path_list_ptr->size() > 1 &&
               even_path_list_ptr->size() > 1)
      {
         if (odd_path_list_ptr->size() <= even_path_list_ptr->size())
         {
            path_list_ptr=new Linkedlist<int>(*odd_path_list_ptr);
         }
         else
         {
            path_list_ptr=new Linkedlist<int>(*even_path_list_ptr);
         }
      }
      delete odd_path_list_ptr;
      delete even_path_list_ptr;

      return path_list_ptr->convert_list_to_vector();
   } // site_ptr(r) == NULL conditional
}

// ---------------------------------------------------------------------
// Member function find_shortest_right_hand_loop takes in site ID r.

template <class T_ptr> std::vector<int>* 
Network<T_ptr>::find_shortest_right_hand_loop(int r)
{
   std::cout << "Inside Network::find_shortest_right_hand_loop()"
             << std::endl;
   
   std::vector<int>* loop_ptr=find_shortest_loop(r);

   std::cout << "Initial loop:" << std::endl;
   templatefunc::printVector(*loop_ptr);

   std::vector<threevector> vertex;
   for (unsigned int i=0; i<loop_ptr->size(); i++)
   {
      int r=(*loop_ptr)[i]; // network element number
      vertex.push_back(get_site_data_ptr(r)->get_posn());
   }

   if (!geometry_func::right_handed_vertex_ordering(vertex,z_hat))
   {
      std::vector<int>* reversed_loop_ptr=new std::vector<int>;
      for (unsigned int i=0; i<vertex.size(); i++)
      {
         reversed_loop_ptr->push_back((*loop_ptr)[vertex.size()-1-i]);
      }
      delete loop_ptr;
      loop_ptr=reversed_loop_ptr;
   }

   std::cout << "Final loop:" << std::endl;
   templatefunc::printVector(*loop_ptr);

   return loop_ptr;
}

