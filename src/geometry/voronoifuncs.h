// =========================================================================
// Header file for stand-alone Voronoi region functions.
// =========================================================================
// Last modified on 8/5/06; 8/6/06; 2/17/11; 1/23/14
// =========================================================================

#ifndef VORONOIFUNCS_H
#define VORONOIFUNCS_H

#include <iostream>
#include <vector>
#include "delaunay/delaunay.h"
#include "datastructures/Linkedlist.h"
#include "math/threevector.h"
#include "network/Network.h"
#include "network/Networkfuncs.h"
#include "geometry/polygon.h"
#include "math/threevector.h"
#include "image/TwoDarray.h"

namespace voronoifunc
{   
   Linkedlist<polygon>* generate_Delaunay_triangle_list(
      const std::vector<threevector>& site);
   std::vector<polygon> generate_Delaunay_triangles(
      const std::vector<threevector>& site);

   Linkedlist<polygon>* generate_Delaunay_triangle_list(
      const std::vector<threevector>& site,int*& delaunay_triangle_vertex);
   std::vector<polygon> generate_Delaunay_triangles(
      const std::vector<threevector>& site,int*& delaunay_triangle_vertex);


   Linkedlist<polygon>* generate_Delaunay_triangle_list(
      int number_of_delaunay_triangles,int* delaunay_triangle_vertex,
      const std::vector<threevector>& site);
   std::vector<polygon> generate_Delaunay_triangles(
      int number_of_delaunay_triangles,int* delaunay_triangle_vertex,
      const std::vector<threevector>& site);

   Linkedlist<polygon>* generate_OSG_Delaunay_triangle_list(
      const std::vector<threevector>& vertices);

   polygon* infinite_voronoi_vertices(
      double infinite_distance_scale,
      std::vector<threevector>& site,std::vector<int>& site_order,
      Linkedlist<delaunay::VoronoiStructure>& voronoi_list);

   void compute_finite_voronoi_vertices(
      int number_of_delaunay_triangles,int* delaunay_triangle_vertex,
      threevector site[],Linkedlist<delaunay::VoronoiStructure>& voronoi_list);
   void compute_finite_voronoi_vertices(
      int number_of_delaunay_triangles,int* delaunay_triangle_vertex,
      std::vector<threevector>& site,
      Linkedlist<delaunay::VoronoiStructure>& voronoi_list);
   void circumcircle_center_of_delaunay_triangle(
      const threevector site[],delaunay::VoronoiStructure& voronoi_vertex);
   void circumcircle_center_of_delaunay_triangle(
      const std::vector<threevector>& site,
      delaunay::VoronoiStructure& voronoi_vertex);
   void report_voronoi_vertices(
      Linkedlist<delaunay::VoronoiStructure>& voronoi_list);

   Linkedlist<polygon>* generate_voronoi_regions(
      const std::vector<threevector>& site);
   Linkedlist<polygon>* generate_voronoi_regions_list(
      int nsites,Linkedlist<delaunay::VoronoiStructure>& voronoi_list);

// Neighbor methods:

   bool immediate_voronoi_region_neighbors(
      const threevector& point1,const threevector& point2,
      polygon& region1,polygon& region2,double dx,double dy);


// ---------------------------------------------------------------------
// Method prune_neighbor_links takes in ID label n for some site in
// the input network *network_ptr.  It scans through all of the
// entries within its linked list of neighbors.  Any neighbor whose
// Voronoi region does not directly touch its own is removed by this
// method from the neighbor linked list for site n.

   template <class T> void prune_neighbor_links(
      int n,twoDarray const *ztwoDarray_ptr,Network<T*>* network_ptr)
      {
         Site<T*>* curr_site_ptr=network_ptr->get_site_ptr(n);
         if (curr_site_ptr != NULL)
         {
            Linkedlist<netlink>* curr_sitelink_list_ptr
               =curr_site_ptr->get_netlink_list_ptr();
            
            if (curr_sitelink_list_ptr != NULL)
            {
               Mynode<netlink>* sitelink_node_ptr=
                  curr_sitelink_list_ptr->get_start_ptr();

               while (sitelink_node_ptr != NULL)
               {
                  Mynode<netlink>* next_sitelink_node_ptr=
                     sitelink_node_ptr->get_nextptr();
                  int i=sitelink_node_ptr->get_data().get_ID();

                  if (!voronoifunc::immediate_voronoi_region_neighbors(
                     network_ptr->get_site_data_ptr(n)->get_posn(),
                     network_ptr->get_site_data_ptr(i)->get_posn(),
                     *(network_ptr->get_site_data_ptr(n)->
                       get_voronoi_region_ptr()),
                     *(network_ptr->get_site_data_ptr(i)->
                       get_voronoi_region_ptr()),
                     ztwoDarray_ptr->get_deltax(),
                     ztwoDarray_ptr->get_deltay()))
                  {
                     network_ptr->delete_symmetric_link(n,i);
//               std::cout << "Symmetrically deleting link between site i = " 
//                    << i << " & site n = " << n << std::endl;
                  }
                  sitelink_node_ptr=next_sitelink_node_ptr;
               }
            }
         } // curr_site_ptr != NULL conditional
      }

// ---------------------------------------------------------------------
// Method prune_distant_neighbors loops over all of the entries within
// input network *network_ptr and prunes away entries from a site's
// linked list of neighbors whose Voronoi regions do not directly
// touch its own.

   template <class T> void prune_distant_neighbors(
      twoDarray const *ztwoDarray_ptr,Network<T*>* network_ptr)
      {
         for (Mynode<int>* currnode_ptr=network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int n=currnode_ptr->get_data();
            prune_neighbor_links(n,ztwoDarray_ptr,network_ptr);
         } // loop over nodes in network entries list
      }


// ---------------------------------------------------------------------
// Method generate_nearest_neighbor_links enables a high-level call to
// fill each building object with a linked list of nearest neighboring
// buildings.  It utilizes both Delaunay triangulation and Voronoi
// region algorithms to determine the nearest neighbors for each
// building in the urbanimage.  

   template <class T> void generate_nearest_neighbor_links(
      twoDarray const *ztwoDarray_ptr,Network<T*>* network_ptr)
      {
         outputfunc::write_banner("Generating nearest neighbor links:");

         std::vector<threevector> site=
            Networkfunc::generate_site_posn_array(network_ptr);
 
// Generate Delaunay triangles for buildings:

         int* delaunay_triangle_vertex;
         Linkedlist<polygon>* triangle_list_ptr=
            voronoifunc::generate_Delaunay_triangle_list(
               site,delaunay_triangle_vertex);

// Update each building's linked list of neighboring buildings:

         Networkfunc::initialize_site_neighbors(
            triangle_list_ptr->size(),delaunay_triangle_vertex,
            network_ptr);
         delete triangle_list_ptr;

         prune_distant_neighbors(ztwoDarray_ptr,network_ptr);
         delete [] delaunay_triangle_vertex;
      }

}

#endif // voronoifuncs.h




