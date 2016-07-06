// ==========================================================================
// NETWORKFUNCS stand-alone methods
// ==========================================================================
// Last modified on 4/13/06; 6/14/06; 8/3/06; 8/8/06; 4/5/14
// ==========================================================================

#include <fstream>
#include <iostream>
#include "image/connectfuncs.h"
#include "general/filefuncs.h"
#include "datastructures/Hashtable.h"
#include "datastructures/Linkedlist.h"
#include "math/mathfuncs.h"
#include "network/netlink.h"
#include "network/Network.h"
#include "network/Networkfuncs.h"
#include "general/outputfuncs.h"
#include "image/TwoDarray.h"

namespace Networkfunc
{

// Method generate_network takes in a hashtable of connected pixel
// components for features such as buildings or trees.  For each
// object within the hashtable, it generates a network site and stores
// the object's center-of-mass position as well as its pixel list into
// the site's data member.  This method returns a pointer to the
// network which it dynamically generates.

   template <class T> Network<T*>* generate_network(
      twoDarray const *ztwoDarray_ptr,
      Hashtable<linkedlist*>* connected_pixellist_hashtable_ptr)
      {

// Set size of network to 2 times the initial number of elements
// within input hashtable in order to minimize network hashtable
// collisions:

         int n_init_sites=connected_pixellist_hashtable_ptr->size();
         Network<T*>* network_ptr=new Network<T*>(2*n_init_sites);

         int n=0;	// site number label
         for (unsigned int j=0; j<connected_pixellist_hashtable_ptr->
                 get_table_capacity(); j++)
         {
            Linkedlist<linkedlist*>* hashlist_ptr=
               connected_pixellist_hashtable_ptr->get_list_ptr(j);
            if (hashlist_ptr != NULL)
            {
               Mynode<linkedlist*>* hashnode_ptr=
                  hashlist_ptr->get_start_ptr();
         
               while (hashnode_ptr != NULL)
               {
                  linkedlist* currlist_ptr=hashnode_ptr->get_data();

                  if (currlist_ptr != NULL)
                  {

// Dynamically generate data for new site and store center-of-mass,
// and connected pixel information within this new object prior to its
// being inserted into the network:

                     T* curr_data_ptr=new T;
                     curr_data_ptr->set_ID(n);
                     threevector COM(connectfunc::pixel_list_COM(
                        currlist_ptr,ztwoDarray_ptr,true));
                     curr_data_ptr->set_posn(COM);

//                  std::cout << "n = " << n 
//                       << " COM = " << curr_data_ptr->get_posn() 
// 			 << std::endl;
                     
// Dynamically generate new linked list of connected pixels.  Store
// heights corresponding to pixel (px,py) coordinates within linked
// list's func[0] elements.  Subsequently insert this list into the
// dynamically generated data objects which are placed into the
// network:

                        linkedlist* copylist_ptr=
                           new linkedlist(*currlist_ptr);

                        for (mynode* curr_pixel_ptr=copylist_ptr->
                                get_start_ptr(); curr_pixel_ptr != NULL; 
                             curr_pixel_ptr=curr_pixel_ptr->get_nextptr())
                        {
                           int px=basic_math::round(
                              curr_pixel_ptr->get_data().get_var(0));
                           int py=basic_math::round(
                              curr_pixel_ptr->get_data().get_var(1));
                           double currz=ztwoDarray_ptr->get(px,py);
                           curr_pixel_ptr->get_data().set_func(0,currz);
                        }

                        curr_data_ptr->set_pixel_list_ptr(copylist_ptr);
                        network_ptr->insert_site(n,Site<T*>(curr_data_ptr));
                        n++;
                  } // currlist_ptr != NULL conditional
                  hashnode_ptr=hashnode_ptr->get_nextptr();
               } // hashnode_ptr != NULL while loop
            } // hashlist_ptr != NULL conditional
         } // loop over index j labeling input hashtable's linked lists
         return network_ptr;
      }

// ---------------------------------------------------------------------
// Method delete_dynamically_allocated_objects_in_network destorys
// each of the dynamically allocated objects which correspond to the
// entries in the sites hashtable. 

   template <class T> void delete_dynamically_allocated_objects_in_network(
      const Network<T*>* network_ptr)
      {
         std::cout << " inside Networkfunc::delete_dyn_all_objs_in_net"
                   << std::endl;
         if (network_ptr != NULL &&
             network_ptr->get_sites_hashtable_ptr() != NULL)
         {
            for (unsigned int r=0; r<network_ptr->size(); r++)
            {
               std::cout << "r = " << r << std::endl;
               std::cout << "network_ptr = " << network_ptr << std::endl;
               std::cout << "network_ptr->get_site_data_ptr(r) = "
                         << network_ptr->get_site_data_ptr(r)
                         << std::endl;
               delete network_ptr->get_site_data_ptr(r);
            }
         }
      }

// ---------------------------------------------------------------------
// Method generate_site_posn_array loops over all of the sites within
// input network *network_ptr 

   template <class T> std::vector<threevector> generate_site_posn_array(
      Network<T*>* network_ptr)
      {
         int b=0;
         std::vector<threevector> site(network_ptr->size());
         for (Mynode<int>* currnode_ptr=network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int n=currnode_ptr->get_data();
            site[b++]=network_ptr->get_site_data_ptr(n)->get_posn();
         } // loop over nodes in network entries list
         return site;
      }

// ---------------------------------------------------------------------
// Method output_site_posns loops over all of the sites within input
// network *network_ptr and writes their positions to an ascii output
// file.

   template <class T> void output_site_posns(
      std::string filenamestr,Network<T*>* network_ptr)
      {
         std::ofstream posnstream;
         filefunc::openfile(filenamestr,posnstream);

         for (Mynode<int>* currnode_ptr=network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int n=currnode_ptr->get_data();
            threevector currposn(
               network_ptr->get_site_data_ptr(n)->get_posn());
            posnstream << currposn.get(0) << "    " << currposn.get(1) 
                       << std::endl;
         } // loop over nodes in network entries list
         filefunc::closefile(filenamestr,posnstream);
      }
         
// ---------------------------------------------------------------------
// Method initialize_site_neighbors takes in integer array
// delaunay_triangle_vertex[] which contains a compactified
// representation for the nearest Delaunay site neighbors.  The first
// 3 elements of delaunay_triangle_vertex[] represent nearest Delaunay
// triangle neighbors.  The next 3 elements also represents nearest
// Delaunay triangle neighbors, etc.  This compactified information is
// parsed and converted into individual site neighbor ID's.

   template <class T> void initialize_site_neighbors(
      unsigned int number_of_delaunay_triangles,int* delaunay_triangle_vertex,
      Network<T*>* network_ptr)
      {
         Linkedlist<int>* entries_list_ptr=network_ptr->
            get_entries_list_ptr();

         for (unsigned int n=0; n<number_of_delaunay_triangles; n++)
         {

// Integers a, b, c represent node indices within the entries linked
// list of the buildings network:

            int a=delaunay_triangle_vertex[3*n+0];
            int b=delaunay_triangle_vertex[3*n+1];
            int c=delaunay_triangle_vertex[3*n+2];

// Integers i, j, k represent the ath, bth and cth building's ID numbers:

            int i=entries_list_ptr->get_node(a)->get_data();
            int j=entries_list_ptr->get_node(b)->get_data();
            int k=entries_list_ptr->get_node(c)->get_data();

            network_ptr->add_symmetric_link(i,j);
            network_ptr->add_symmetric_link(j,k);
            network_ptr->add_symmetric_link(k,i);
         }
      }

} // Networkfunc namespace




