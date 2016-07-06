// ==========================================================================
// Roadfuncs namespace method definitions
// ==========================================================================
// Last modified on 7/29/06; 7/30/06; 12/4/10; 3/6/14
// ==========================================================================

#include <set>
#include <string>
#include <vector>
#include "math/basic_math.h"
#include "image/binaryimagefuncs.h"
#include "math/constants.h"
#include "geometry/contour.h"
#include "image/drawfuncs.h"
#include "threeDgraphics/draw3Dfuncs.h"
#include "ladar/featurefuncs.h"
#include "image/imagefuncs.h"
#include "ladar/ladarfuncs.h"
#include "ladar/ladarimage.h"
#include "geometry/mypoint.h"
#include "templates/mytemplates.h"
#include "network/netlink.h"
#include "network/Network.h"
#include "general/outputfuncs.h"
#include "geometry/parallelogram.h"
#include "urban/roadfuncs.h"
#include "urban/roadpoint.h"
#include "network/Site.h"
#include "general/stringfuncs.h"
#include "threeDgraphics/threeDstring.h"
#include "image/TwoDarray.h"
#include "urban/urbanimage.h"
#include "threeDgraphics/xyzpfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

namespace roadfunc
{
   
   Network<roadpoint*>* intersections_network_ptr=NULL;

// ==========================================================================
// Road network computation methods
// ==========================================================================

// Method search_for_closest_point_in_road_network takes in threevector
// posn.  It performs a brute-force search over the roadpoints network
// for the site which is located closest to posn.  This method returns
// the closest site's integer label.

   int search_for_closest_point_in_road_network(
      Network<roadpoint*>* roadpoints_network_ptr,
      const threevector& posn,bool exclude_intersections)
      {
         int closest_roadpoint_ID=-1;
         double min_distance_to_point=POSITIVEINFINITY;
         for (Mynode<int>* currnode_ptr=roadpoints_network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int r=currnode_ptr->get_data(); // roadpoint number
            roadpoint* curr_roadpoint_ptr=
               roadpoints_network_ptr->get_site_data_ptr(r);
            double curr_distance_to_point=
               (curr_roadpoint_ptr->get_posn()-posn).magnitude();
            if (curr_distance_to_point < min_distance_to_point)
            {
               if ((exclude_intersections && 
                    !curr_roadpoint_ptr->get_intersection()) ||
                   (!exclude_intersections))
               {
                  min_distance_to_point=curr_distance_to_point;
                  closest_roadpoint_ID=r;
               }
            }
         } // loop over index r labeling roadpoint number
         return closest_roadpoint_ID;
      }

// ---------------------------------------------------------------------
// Method search_for_closest_link_in_road_network takes in threevector
// posn.  It performs a brute-force search over the roadpoints network
// for the link which is located closest to posn.  This method returns
// the distance to the closest roadlink as well as the integer labels
// r and q for the roadlink's endpoints.

   double search_for_closest_link_in_road_network(
      Network<roadpoint*>* roadpoints_network_ptr,
      const threevector& posn,bool exclude_intersections,int& r,int& q)
      {
         double min_distance_to_link=POSITIVEINFINITY;
         r=search_for_closest_point_in_road_network(
            roadpoints_network_ptr,posn,exclude_intersections);
         roadpoint* curr_roadpoint_ptr=
            roadpoints_network_ptr->get_site_data_ptr(r);
         Site<roadpoint*>* curr_roadpoint_site_ptr=
            roadpoints_network_ptr->get_site_ptr(r);
         Linkedlist<netlink>* roadlink_list_ptr=
            curr_roadpoint_site_ptr->get_netlink_list_ptr();   

         if (roadlink_list_ptr != NULL)
         {
            for (Mynode<netlink>* currnode_ptr=roadlink_list_ptr->
                    get_start_ptr(); currnode_ptr != NULL; currnode_ptr=
                    currnode_ptr->get_nextptr())
            {
               roadpoint* neighbor_roadpoint_ptr=
                  roadpoints_network_ptr->get_site_data_ptr(
                     currnode_ptr->get_data().get_ID());
               linesegment l(curr_roadpoint_ptr->get_posn(),
                             neighbor_roadpoint_ptr->get_posn());
               double curr_distance_to_link=l.point_to_line_segment_distance(
                  posn);
               if (curr_distance_to_link < min_distance_to_link)
               {
                  min_distance_to_link=curr_distance_to_link;
                  q=currnode_ptr->get_data().get_ID();
               }
            } // loop over index r labeling roadpoint number
         } // roadlink_list_ptr != NULL conditional
   
         return min_distance_to_link;
      }

// ==========================================================================
// Road intersection methods
// ==========================================================================

// Method find_intersection_roadpoints scans through all of the sites
// within the roadpoints network.  It sets to true the intersection
// flag for any roadpoint which has more than 2 roadpoint neighbors
// lying inside the ladar data bounding box in its roadlinks list.

   void find_intersection_roadpoints(
      Network<roadpoint*>* roadpoints_network_ptr,
      parallelogram* data_bbox_ptr)
      {
         outputfunc::write_banner("Finding intersection roadpoints:");

         for (Mynode<int>* currnode_ptr=roadpoints_network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int r=currnode_ptr->get_data(); // roadpoint number
            Site<roadpoint*>* curr_roadpoint_site_ptr=
               roadpoints_network_ptr->get_site_ptr(r);
            cout << r << " " << flush; 

            Linkedlist<netlink>* curr_roadlink_list_ptr
               =curr_roadpoint_site_ptr->get_netlink_list_ptr();

// If current site has more than 2 roadpoint links, declare it to be a
// candidate intersection point:

            if (curr_roadlink_list_ptr != NULL)
            {
               if (curr_roadlink_list_ptr->size() > 2)
               {
                  curr_roadpoint_site_ptr->get_data()->set_intersection(true);
               }
            } // curr_roadlink_list_ptr != NULL conditional
         } // loop over sites in roadpoints network
         outputfunc::newline();
      }

// This next minor variant of find_intersection_roadpoints() resets
// all roadpoints' intersection boolean flags to false:

   void reset_intersection_roadpoints(
      Network<roadpoint*>* roadpoints_network_ptr)
      {
         outputfunc::write_banner("Reseting intersection roadpoints:");

         for (Mynode<int>* currnode_ptr=roadpoints_network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int r=currnode_ptr->get_data(); // roadpoint number
            Site<roadpoint*>* curr_roadpoint_site_ptr=
               roadpoints_network_ptr->get_site_ptr(r);
            Linkedlist<netlink>* curr_roadlink_list_ptr
               =curr_roadpoint_site_ptr->get_netlink_list_ptr();

// If current site has more than 2 roadpoint links, declare it to be a
// candidate intersection point:

            if (curr_roadlink_list_ptr != NULL)
            {
               curr_roadpoint_site_ptr->get_data()->set_intersection(false);
            } // curr_roadlink_list_ptr != NULL conditional
         } // loop over sites in roadpoints network
      }
   
// ---------------------------------------------------------------------
// Method find_next_intersection takes in some current roadpoint ID r
// as well as some previous roadpoint ID r_prev.  If r corresponds to
// an intersection, this method returns r.  Otherwise, it checks r's
// two neighboring roadpoints.  One of the neighbhors should be
// r_prev.  The other, q, is used to recursively call this method with
// r and r_prev respectively set equal to q and r.  This method can be
// used to compute the ID of an intersection point located at the end
// of a string of non-intersection roadpoints.  

   int find_next_intersection(
      Network<roadpoint*>* roadpoints_network_ptr,int r,int r_prev)
      {

// Terminate intersection search if current roadway path intercepts
// any corner of the data bounding box:

         if (roadpoints_network_ptr->get_site_data_ptr(r)->
             get_data_bbox_corner())
         {
            return -1;
         }

// First check whether roadpoint r is an intersection:

         Site<roadpoint*>* curr_roadpoint_site_ptr=
            roadpoints_network_ptr->get_site_ptr(r);
         if (curr_roadpoint_site_ptr->get_data()->get_intersection())
         {
            return r;
         }
         
         Linkedlist<netlink>* curr_roadlink_list_ptr
            =curr_roadpoint_site_ptr->get_netlink_list_ptr();

// Make sure current site has exactly 2 roadpoint links:

         if (curr_roadlink_list_ptr != NULL &&
             curr_roadlink_list_ptr->size()==1)
         {
            return r;
         }
         else if (curr_roadlink_list_ptr != NULL &&
             curr_roadlink_list_ptr->size()==2)
         {
            for (Mynode<netlink>* roadlink_node_ptr=
                    curr_roadlink_list_ptr->get_start_ptr();
                 roadlink_node_ptr != NULL; roadlink_node_ptr=
                    roadlink_node_ptr->get_nextptr())
            {
               int q=roadlink_node_ptr->get_data().get_ID();
               if (q != r_prev)
               {
                  return find_next_intersection(
                     roadpoints_network_ptr,q,r);
               }
            } // loop over nodes in *curr_roadlink_list_ptr
         }
         else
         {
            cout << "Error in roadfunc::find_next_intersection()" << endl;
            cout << "r = " << r << " r_prev = " << r_prev << endl;
            cout << "curr_roadlink_list_ptr->size() = "
                 << curr_roadlink_list_ptr->size() << endl;
            cout << "*curr_roadlink_list_ptr = " 
                 << *curr_roadlink_list_ptr << endl;
            exit(-1);
         }
         return -1;
      }

// ---------------------------------------------------------------------
// Method define_intersections_on_data_bbox loops over all sites
// within the roadpoints network.  It searches for road links that
// intersect the data bounding box.  This method defines the points
// where the road link intersect the bbox as new road "intersection"
// points which it adds to the roadpoints network.  It also
// establishes roadlinks between these new "intersection" points with
// each other as well as with roadpoints lying strictly inside the
// data bounding box.

   void define_intersections_on_data_bbox(
      Network<roadpoint*>* roadpoints_network_ptr,
      parallelogram* data_bbox_ptr)
      {
         outputfunc::write_banner("Defining intersections on data bbox:");

// Instantiate linked list to hold averaged intersection point
// information which will be added to the roadpoints network only
// AFTER all intersection points needing to be consolidated have been
// found:

         Linkedlist<pair<roadpoint*,vector<int> > >* 
            new_intersection_list_ptr=
            new Linkedlist<pair<roadpoint*,vector<int> > >;

         for (Mynode<int>* currnode_ptr=roadpoints_network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int r=currnode_ptr->get_data(); // roadpoint number
            cout << r << " " << flush;
            Site<roadpoint*>* curr_roadpoint_site_ptr=
               roadpoints_network_ptr->get_site_ptr(r);
            roadpoint* curr_roadpoint_ptr=
               roadpoints_network_ptr->get_site_data_ptr(r);
      
// Ignore roadpoints that lie outside data bounding box:

            if (data_bbox_ptr->point_inside_polygon(
               curr_roadpoint_ptr->get_posn()))
            {
               Linkedlist<netlink>* curr_roadlink_list_ptr
                  =curr_roadpoint_site_ptr->get_netlink_list_ptr();

               if (curr_roadlink_list_ptr != NULL)
               {
                  for (Mynode<netlink>* roadlink_node_ptr=
                          curr_roadlink_list_ptr->get_start_ptr();
                       roadlink_node_ptr != NULL; roadlink_node_ptr=
                          roadlink_node_ptr->get_nextptr())
                  {
                     int q=roadlink_node_ptr->get_data().get_ID();
                     roadpoint* neighbor_roadpoint_ptr=
                        roadpoints_network_ptr->get_site_data_ptr(q);
          
// Look for neighboring roadpoint which lies outside data bounding box:
               
                     if (!data_bbox_ptr->point_inside_polygon(
                        neighbor_roadpoint_ptr->get_posn()))
                     {
                        linesegment roadlink(
                           curr_roadpoint_ptr->get_posn(),
                           neighbor_roadpoint_ptr->get_posn());
                  
                        unsigned int n_intersection_pnts;
                        int intersected_poly_side[
                           data_bbox_ptr->get_nvertices()];
                        threevector intersection_pnt[
                           data_bbox_ptr->get_nvertices()];
                        if (data_bbox_ptr->
                            linesegment_intersection_with_polygon(
                               roadlink,n_intersection_pnts,
                               intersected_poly_side,intersection_pnt))
                        {
                           if (n_intersection_pnts==1)
                           {
                              pair<roadpoint*,vector<int> > p;

                              p.first=new roadpoint(intersection_pnt[0]);
                              p.first->set_intersection(true);
                              p.first->set_on_data_bbox(true);
                              p.second.push_back(r);
                              new_intersection_list_ptr->append_node(p);
                           } // n_intersection_pnts==1 conditional
                        } // line segment intersects data bbox conditional
                     } // neighbor roadpoint lies outside bbox conditional
                  } // loop over current roadpoint's roadlinks
               } // curr_roadlink_list_ptr != NULL conditional
            } // curr_roadpoint lies in data bbox conditional
         } // loop over sites in roadpoints network
         outputfunc::newline();

// Define corners of data bounding box as 4 more regular roadpoints:

         for (unsigned int i=0; i<data_bbox_ptr->get_nvertices(); i++)
         {
            pair<roadpoint*,vector<int> > p;
            p.first=new roadpoint(data_bbox_ptr->get_vertex(i));
            p.first->set_intersection(false);
            p.first->set_on_data_bbox(true);
            p.first->set_data_bbox_corner(true);
            new_intersection_list_ptr->append_node(p);
         }

// Add new intersection points to roads network:

         roadpoints_network_ptr->append_new_sites(new_intersection_list_ptr);
         delete new_intersection_list_ptr;

// Save fractional location of "intersection" points along the data
// bounding box as well as their roadpoint IDs into an STL vector for
// later roadlinking purposes:

         vector<pair<double,int> > frac_bbox_location;
         for (Mynode<int>* currnode_ptr=roadpoints_network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int r=currnode_ptr->get_data(); // network element number
            roadpoint* curr_roadpoint_ptr=roadpoints_network_ptr->
               get_site_data_ptr(r);

// Declare any genuine intersection point which lies very close to the
// data bounding box to actually lie on the bbox:

            const double min_separation=10;	// meters
            if (curr_roadpoint_ptr->get_intersection() && 
                !curr_roadpoint_ptr->get_on_data_bbox())
            {
               threevector roadpoint_posn(curr_roadpoint_ptr->get_posn());
               if (data_bbox_ptr->point_dist_to_polygon(
                  roadpoint_posn) < min_separation)
               {
                  curr_roadpoint_ptr->set_on_data_bbox(true);
               }
            }

            if ((curr_roadpoint_ptr->get_intersection() &&
                 curr_roadpoint_ptr->get_on_data_bbox()) || 
                curr_roadpoint_ptr->get_data_bbox_corner())
            {
               threevector roadpoint_posn(curr_roadpoint_ptr->get_posn());
               threevector closest_pnt_on_bbox(
                  data_bbox_ptr->closest_polygon_perimeter_point(
                     roadpoint_posn).second);
               frac_bbox_location.push_back(pair<double,int>(
                  data_bbox_ptr->frac_distance_along_polygon(
                     closest_pnt_on_bbox),r));
            }
         } // loop over all sites within *roadpoints_network_ptr

// Sort "intersection" points on data bbox according to their
// fractional location relative to bbox's zeroth vertex:

         std::sort(frac_bbox_location.begin(),frac_bbox_location.end());

// Establish road link between adjacent "intersection" points on data
// bounding box:

         for (unsigned int i=0; i<frac_bbox_location.size(); i++)
         {
            int r=frac_bbox_location[i].second;
            int s=frac_bbox_location[modulo(i+1,frac_bbox_location.size())].
               second;
            roadpoints_network_ptr->add_symmetric_link(r,s);
         }
      }
   
// ---------------------------------------------------------------------
// Method delete_roadpoints_outside_data_bbox instantiates a slightly
// enlarged copy of the data bounding box.  It then removes all sites
// which lie outside this enlarged bounding box from the roadpoints
// network.

   void delete_roadpoints_outside_data_bbox(
      Network<roadpoint*>* roadpoints_network_ptr,
      parallelogram* data_bbox_ptr)
      {
         outputfunc::write_banner("Deleting roadpoints outside data bbox:");
         polygon enlarged_data_bbox(*data_bbox_ptr);
         enlarged_data_bbox.scale(1.003);

// Instantiate linked list to hold ID labels for roadpoints which are
// located very far away from asphalt.  Since we're actually looping
// over the roadpoints themselves, we cannot delete these far-away
// roadpoints until we've finished traversing the entire roadpoints
// network:

         Linkedlist<int>* roadpoints_to_delete_list_ptr=new Linkedlist<int>;

         for (Mynode<int>* currnode_ptr=roadpoints_network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int r=currnode_ptr->get_data(); // roadpoint number
            if (!enlarged_data_bbox.point_inside_polygon(
               roadpoints_network_ptr->get_site_data_ptr(r)->get_posn()))
            {
               roadpoints_to_delete_list_ptr->append_node(r);
            }
         } // loop over sites in roadpoints network

         roadpoints_network_ptr->delete_sites(roadpoints_to_delete_list_ptr);
         delete roadpoints_to_delete_list_ptr;
      }

// ---------------------------------------------------------------------
// Method adjust_bbox_intersection_roadpoints moves all intersection
// points located along the data bounding box radially outwards by a
// small factor.  Since the data bounding box is generally contained
// INSIDE the actual ALIRT imagery data, we perform this brute force
// operation to eliminate ugly looking "roads" from appearing along
// the data bounding box within output XYZP files.

   threevector radially_scale_roadpoint_on_bbox(
      const threevector& roadpoint_posn,parallelogram* data_bbox_ptr)
      {
         mypoint new_pnt(roadpoint_posn);
         const double scale_factor=1.025;
         new_pnt.scale(data_bbox_ptr->vertex_average(),
                       threevector(scale_factor,scale_factor,1.0));
         return new_pnt.get_pnt();
      }

   void adjust_bbox_intersection_roadpoints(
      Network<roadpoint*>* roadpoints_network_ptr,
      parallelogram* data_bbox_ptr)
      {
         for (Mynode<int>* currnode_ptr=roadpoints_network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int r=currnode_ptr->get_data(); // roadpoint number
            roadpoint* roadpoint_ptr=roadpoints_network_ptr->
               get_site_data_ptr(r);
            if (roadpoint_ptr->get_intersection() && 
                roadpoint_ptr->get_on_data_bbox())
            {
//               roadpoint_ptr->set_posn(radially_scale_roadpoint_on_bbox(
//                  roadpoint_ptr->get_posn(),data_bbox_ptr));
               mypoint new_pnt(roadpoint_ptr->get_posn());
               const double scale_factor=1.025;
               new_pnt.scale(data_bbox_ptr->vertex_average(),
                             threevector(scale_factor,scale_factor,1.0));
               roadpoint_ptr->set_posn(new_pnt.get_pnt());
            } 
         } // loop over sites in roadpoints network
      }
   
// ---------------------------------------------------------------------
// Method consolidate_roadpoints_close_to_intersections scans through
// all of the street intersection points within
// *roadpoints_network_ptr.  If the distance between some roadpoint
// and an intersection is less than some minimal length, this method
// consolidates them together into a new intersection point located at
// their average position.  All road links flowing into the two
// predecessor intersection points are attached onto the new
// intersection point.  

   void consolidate_roadpoints_close_to_intersections(
      Network<roadpoint*>* roadpoints_network_ptr,int max_iters)
      {

// Iteratively consolidate intersection roadpoints which are spaced
// further and further apart:

         for (int iter=1; iter<=max_iters; iter++)
         {
            double min_distance_between_intersections=iter*5;	// meters
            string banner="Consolidating intersection roadpoints within "
               +stringfunc::number_to_string(
                  min_distance_between_intersections)+" meters";
            outputfunc::write_banner(banner);

            for (Mynode<int>* currnode_ptr=roadpoints_network_ptr->
                    get_entries_list_ptr()->get_start_ptr(); 
                 currnode_ptr != NULL; currnode_ptr=currnode_ptr->
                    get_nextptr())
            {
               int r=currnode_ptr->get_data(); // roadpoint number
               roadpoint* curr_roadpoint_ptr=
                  roadpoints_network_ptr->get_site_data_ptr(r);

// Ignore any roadpoints which are not intersection points:

               if (curr_roadpoint_ptr->get_intersection())
               {

// On 11/28/04, we empirically found that the loop over *othernode_ptr
// Mynode<int>'s may need to be performed again whenever a node is
// deleted from the roadpoints network.  We consequently implement the
// following while loop which repeatedly tries to perform roadpoint
// merging until no further merging is possible:

                  bool points_merged;
                  do
                  {
                     points_merged=false;
                     for (Mynode<int>* othernode_ptr=roadpoints_network_ptr->
                             get_entries_list_ptr()->get_start_ptr(); 
                          othernode_ptr != NULL; othernode_ptr=othernode_ptr->
                             get_nextptr())
                     {
                        int q=othernode_ptr->get_data(); // roadpoint number
                        if (q != r)
                        {
                           if (merge_nearby_roadpoints(
                              r,q,min_distance_between_intersections,
                              roadpoints_network_ptr))
                           {
                              points_merged=true;
                           }
                        } 
                     } // loop over nodes in *roadpoints_network_ptr
                  }
                  while (points_merged);
                  
               } // roadpoint intersection conditional
            } // loop over sites in roadpoints network
         } // loop over iter index
      }

// ---------------------------------------------------------------------
// Method merge_nearby_roadpoints takes in two roadpoints r and q.  If
// the distance between them is less than input parameter
// min_dist_between_pnts, this boolean method merges them together at
// their average location and returns true.

   bool merge_nearby_roadpoints(
      int r,int q,const double min_dist_between_pnts,
      Network<roadpoint*>* roadpoints_network_ptr)
      {
         threevector r_posn(roadpoints_network_ptr->
                         get_site_data_ptr(r)->get_posn());
         threevector q_posn(roadpoints_network_ptr->
                         get_site_data_ptr(q)->get_posn());

         if ((r_posn-q_posn).magnitude() < min_dist_between_pnts)
         {
            roadpoints_network_ptr->merge_nearby_sites(r,q);
            roadpoints_network_ptr->get_site_data_ptr(r)->set_posn(
               0.5*(r_posn+q_posn));
            return true;
         }
         else
         {
            return false;
         }
      }

// ---------------------------------------------------------------------
// Method find_intersection_neighbors_to_intersections takes in
// roadpoint ID r.  If this roadpoint represents an intersection, this
// method transverses each strand of sites emanating outwards from r
// until a neighboring intersection is found.  It returns an STL
// vector of neighboring intersection IDs.

   vector<int> find_intersection_neighbors_to_intersection(
      int r,Network<roadpoint*>* roadpoints_network_ptr,
      parallelogram* data_bbox_ptr)
      {
         vector<int> intersection_neighbors;
         Site<roadpoint*>* curr_roadpoint_site_ptr=
            roadpoints_network_ptr->get_site_ptr(r);
         if (curr_roadpoint_site_ptr != NULL &&
             curr_roadpoint_site_ptr->get_data()->get_intersection())
         {
            Linkedlist<netlink>* curr_roadlink_list_ptr
               =curr_roadpoint_site_ptr->get_netlink_list_ptr();

            if (curr_roadlink_list_ptr != NULL)
            {
               for (Mynode<netlink>* roadlink_node_ptr=
                       curr_roadlink_list_ptr->get_start_ptr();
                    roadlink_node_ptr != NULL; roadlink_node_ptr=
                       roadlink_node_ptr->get_nextptr())
               {
                  int q=roadlink_node_ptr->get_data().get_ID();
                  int r_next=find_next_intersection(
                     roadpoints_network_ptr,q,r);

// Do NOT form road links between "intersection" points lying along
// edges n and n+1 of the data bounding box which are linked only
// through a data bbox corner:

                  if (r_next != -1)
                  {
                     
// Do NOT form road links between "intersection" points that lie along
// the same edge of the data bounding box.

                     bool intersections_on_same_bbox_edge=false;
                     if (roadpoints_network_ptr->get_site_data_ptr(r)->
                         get_on_data_bbox() && roadpoints_network_ptr->
                         get_site_data_ptr(r_next)->get_on_data_bbox())
                     {
                        int r_edge=data_bbox_ptr->
                           closest_polygon_edge_to_point(
                              roadpoints_network_ptr->get_site_data_ptr(r)->
                              get_posn());
                        int rnext_edge=data_bbox_ptr->
                           closest_polygon_edge_to_point(
                              roadpoints_network_ptr->get_site_data_ptr(
                                 r_next)->get_posn());
                        if (r_edge==rnext_edge)
                        {
                           intersections_on_same_bbox_edge=true;
                        }
                     } // both roadpoints on data bbox conditional

                     if (!intersections_on_same_bbox_edge)
                     {
                        intersection_neighbors.push_back(r_next);
                     } // intersections not on same bbox edge conditional
                  } // r_next != -1 (bbox corner encountered) conditional
               } // loop over *curr_roadlink_list_ptr
            } // curr_roadlink_list_ptr != NULL conditional
         } // curr roadpoint is intersection conditional
         return intersection_neighbors;
      }

// ---------------------------------------------------------------------
// Method generate_intersections_network scans through input
// *roadpoints_network_ptr and searches for all roadpoints which are
// classified as intersections.  It subsequently traverses along the
// roadpoint strands emanating out from each intersection until a
// neighboring intersection is found.  Both intersection ID and
// neighboring intersection information are stored within the
// dynamically generated *intersections_network_ptr by this method.

   void generate_intersections_network(
      Network<roadpoint*>* roadpoints_network_ptr,
      parallelogram* data_bbox_ptr)
      {
         outputfunc::write_banner("Generating intersections network:");

         intersections_network_ptr=new Network<roadpoint*>(
            roadpoints_network_ptr->size());

         for (Mynode<int>* currnode_ptr=roadpoints_network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int r=currnode_ptr->get_data(); // roadpoint number
            Site<roadpoint*>* curr_roadpoint_site_ptr=
               roadpoints_network_ptr->get_site_ptr(r);
            if (curr_roadpoint_site_ptr->get_data()->get_intersection())
            {
               roadpoint* new_intersection_ptr=new roadpoint(
                  r,curr_roadpoint_site_ptr->get_data()->get_posn());
               new_intersection_ptr->set_intersection(true);
               new_intersection_ptr->set_on_data_bbox(
                  curr_roadpoint_site_ptr->get_data()->get_on_data_bbox());
               intersections_network_ptr->insert_site(
                  r,Site<roadpoint*>(new_intersection_ptr));

               vector<int> intersection_neighbors=
                  find_intersection_neighbors_to_intersection(
                     r,roadpoints_network_ptr,data_bbox_ptr);
               for (unsigned int i=0; i<intersection_neighbors.size(); i++)
               {
                  intersections_network_ptr->add_to_neighbor_list(
                     r,intersection_neighbors[i]);
               }

            } // curr roadpoint is intersection conditional
         } // loop over sites in roadpoints network
         outputfunc::newline();
      }

// ---------------------------------------------------------------------
// Method delete_roadpoints_network loops over all dynamically
// allocated roadpoints within input *roadpoints_network_ptr and
// deletes them.  It then deletes the roadpoints network itself.

   void delete_roadpoints_network(
      Network<roadpoint*>* roadpoints_network_ptr)
      {
         Networkfunc::delete_dynamically_allocated_objects_in_network(
            roadpoints_network_ptr);
         delete roadpoints_network_ptr;
      }

// ==========================================================================
// Intersection perturbation methods:
// ==========================================================================

// Method fill_intersections_vector returns an STL vector containing
// ID labels for all intersection points within the intersections
// network.

   vector<int> fill_intersections_vector()
      {
         vector<int> intersections;
         intersections.reserve(intersections_network_ptr->size());
         for (Mynode<int>* currnode_ptr=intersections_network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            intersections.push_back(currnode_ptr->get_data());
         }
         return intersections;
      }

// This overloaded version of fill_intersections_vector takes in an
// arbitrary roadpoints network.  It scans through the network for
// sites which are labeled as intersections.  This method returns an
// STL vector containing the ID labels for those intersection sites.

   vector<int> fill_intersections_vector(
      Network<roadpoint*> const *roadpoints_network_ptr)
      {
         vector<int> intersections;
         for (const Mynode<int>* currnode_ptr=roadpoints_network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int r=currnode_ptr->get_data(); // roadpoint number
            roadpoint* curr_roadpoint_ptr=
               roadpoints_network_ptr->get_site_data_ptr(r);
            if (curr_roadpoint_ptr->get_intersection())
               intersections.push_back(r);
         }
         return intersections;
      }

// ---------------------------------------------------------------------
// Method fill_neighbors_vector takes in ID label r for some site
// within the intersections network.  It returns an STL vector
// containing ID labels for the intersection neighbors of r.

   vector<int> fill_neighbors_vector(int r)
      {
         vector<int> neighbor_intersection;
         Linkedlist<netlink>* roadlink_list_ptr=intersections_network_ptr->
            get_site_ptr(r)->get_netlink_list_ptr();   
         if (roadlink_list_ptr != NULL)
         {
            for (Mynode<netlink>* currnode_ptr=roadlink_list_ptr->
                    get_start_ptr(); currnode_ptr != NULL; currnode_ptr=
                    currnode_ptr->get_nextptr())
            {
               int q=currnode_ptr->get_data().get_ID();
               neighbor_intersection.push_back(q);
            } // loop over link neighbors for node r
         }
         return neighbor_intersection;
      }

// ---------------------------------------------------------------------
// Method improve_intersections_network loops over all intersection
// points within the intersections network.  For each intersection
// site, this method fills an STL vector with its neighboring
// intersection IDs.  It then calls perturb_intersection_position
// which maximizes a score function based upon the product of thick
// line integrals from the intersection to all of its neighbors.  This
// perturbation operation is performed for max_iters iterations.

   void improve_intersections_network(
      int max_iters,parallelogram* data_bbox_ptr,
      twoDarray const *binary_asphalt_twoDarray_ptr)
      {
         outputfunc::write_banner("Refining intersections network:");
         vector<int> intersections=fill_intersections_vector();
         for (int iter=0; iter<max_iters; iter++)
         {
            cout << "Iteration = " << iter+1 << " of " << max_iters 
                 << endl << endl;
            for (unsigned int i=0; i<intersections.size(); i++)
            {
               int r=intersections[i];
               vector<int> neighbor_intersection=fill_neighbors_vector(r);
               perturb_intersection_position(
                  r,neighbor_intersection,intersections_network_ptr,
                  data_bbox_ptr,binary_asphalt_twoDarray_ptr);
            } // loop over i index
         } // loop over iter index
      }
         
// ---------------------------------------------------------------------
// Method perturb_intersection_position takes in some intersection
// roadpoint r, its intersection neighbors within STL vector
// neighbor_intersection, and a features score map within
// *binary_asphalt_twoDarray_ptr.  It first computes candidate
// locations within the immediate vicinity of the r intersection.  If
// r lies along the data bounding box, the candidate locations are
// restricted to also lie along the bbox.  On the other hand,
// candidates within an entire region surrounding r are considered if
// r lies inside the bbox.

// For each candidate location for r, this method computes a thick
// line integral score for each segment running between r and all of
// its intersection neighbors.  The total score for some candidate r
// location equals the product of each individual line segment score.
// This method resets the position of r intersection equal to the
// candidate locations which maximize the total score.
   
   void perturb_intersection_position(
      int r,vector<int> neighbor_intersection,
      Network<roadpoint*>* roadpoints_network_ptr,
      parallelogram* data_bbox_ptr,
      twoDarray const *binary_asphalt_twoDarray_ptr)
      {
         roadpoint* r_roadpoint_ptr=roadpoints_network_ptr->
            get_site_data_ptr(r);
         threevector r_posn(r_roadpoint_ptr->get_posn());
         threevector best_r_posn(r_posn);

// Determine whether site r lies along a data bbox edge:

         bool r_on_bbox=r_roadpoint_ptr->get_on_data_bbox();
         int r_edge_number=data_bbox_ptr->closest_polygon_edge_to_point(
            r_posn);

//         double displacement_dist=10;	// meters
         double displacement_dist=30;	// meters

// In Dec 04, we empirically observed that the raw intersection
// network in the vicinity of building "islands" surrounded by streets
// is usually pretty good.  So we do not let intersection points
// nearby building islands wander as far as those which are not nearby
// islands:

         if (r_roadpoint_ptr->get_intersection() &&
             r_roadpoint_ptr->get_near_bldg_island())
         {
//            displacement_dist=30;	// meters
            displacement_dist=20;	// meters
//            displacement_dist=10;	// meters
//            displacement_dist=5;	// meters
//            displacement_dist=2;	// meters
         }
         
         vector<threevector> r_testpnts;
         if (r_on_bbox)
         {
            threevector r_hat(data_bbox_ptr->get_edge(r_edge_number).get_ehat());
            r_testpnts=generate_intersection_testpnts(
               r_on_bbox,data_bbox_ptr,displacement_dist,r_posn,r_hat);
         }
         else
         {
            r_testpnts=generate_intersection_spread(
               displacement_dist,r_posn);
         }
         
         double best_score=0;
         for (unsigned int i=0; i<r_testpnts.size(); i++)
         {
            threevector curr_r_posn=r_testpnts[i];
            double curr_score=1;
            for (unsigned int n=0; n<neighbor_intersection.size(); n++)
            {
               int q=neighbor_intersection[n];
               roadpoint* q_roadpoint_ptr=roadpoints_network_ptr->
                  get_site_data_ptr(q);
               threevector q_posn(q_roadpoint_ptr->get_posn());
               bool q_on_bbox=q_roadpoint_ptr->get_on_data_bbox();
               int q_edge_number=data_bbox_ptr->
                  closest_polygon_edge_to_point(q_posn);

               const double min_sqrd_separation=sqr(2);	// meters
               if ((curr_r_posn-q_posn).sqrd_magnitude() <
                   min_sqrd_separation)
               {
                  curr_score=0;
               }

// Do not evaluate any thick line integrals between two intersection
// points which lie along the same edge of the data bounding box:

               else if (r_on_bbox && q_on_bbox &&
                        r_edge_number==q_edge_number)
               {
                  curr_score *= 1;
               }
               else
               {
                  curr_score *= score_link(
                     curr_r_posn,q_posn,binary_asphalt_twoDarray_ptr);
               } // sqrd magnitude conditional
               if (curr_score==0) break;
               
            } // loop over index n labeling q neighbors to r


// On 1/17/05, we realized that the following section actually had no
// effect upon the intersections network, for we were incorrectly not
// passing curr_r_posn but rather using r_posn in the
// angles_between_neighboring_links method below.  We will need to do
// much more work before angular relationships between different road
// links will be correctly taken into account...

/*
// Take angular relationships between different road links coming into
// site r into account:

            if (neighbor_intersection.size() > 1)
            {
               vector<double> dtheta(
                  roadpoints_network_ptr->
                  angles_between_neighboring_links(r,curr_r_posn));

               double weight=1.0;
               for (unsigned int j=0; j<dtheta.size(); j++)
               {
                  int q1=neighbor_intersection[j];
                  int q2=neighbor_intersection[modulo(j+1,dtheta.size())];
                  int q3=neighbor_intersection[modulo(j+2,dtheta.size())];

                  bool q1_on_bbox=roadpoints_network_ptr->
                     get_site_data_ptr(q1)->get_on_data_bbox();
                  bool q2_on_bbox=roadpoints_network_ptr->
                     get_site_data_ptr(q2)->get_on_data_bbox();
                  bool q3_on_bbox=roadpoints_network_ptr->
                     get_site_data_ptr(q3)->get_on_data_bbox();

//                  bool q1_near_bldg_island=roadpoints_network_ptr->
//                     get_site_data_ptr(q1)->get_near_bldg_island();
//                  bool q2_near_bldg_island=roadpoints_network_ptr->
//                     get_site_data_ptr(q2)->get_near_bldg_island();
//                  bool q3_near_bldg_island=roadpoints_network_ptr->
//                     get_site_data_ptr(q3)->get_near_bldg_island();

// Penalize adjacent road links whose angular separation is small:

                  if (!(r_on_bbox && q1_on_bbox) && 
                      !(r_on_bbox && q2_on_bbox) &&
                      !(q1_on_bbox && q2_on_bbox))
                  {
                     double sigma=40*PI/180;
                     if (neighbor_intersection.size()==2) sigma=135*PI/180;
                     weight *= 1-exp(-0.5*sqr(dtheta[j]/sigma));
                  }
               
// Reward adjacent road links which are separated in angle by nearly
// 180 degrees:

                  if (!(q1_on_bbox && q2_on_bbox))
                  {
                     double dtheta_shift=
                        mathfunc::phase_to_canonical_interval(
                           dtheta[j],0,2*PI);
                     const double sigma=15*PI/180;
                     if (neighbor_intersection.size() > 2)
                        weight *= 1+2*exp(-0.5*sqr((dtheta_shift-PI)/sigma));
                  }

// Reward next-to-adjacent road links which are nearly 180 degrees
// apart in angle:

                  if (!(q1_on_bbox && q3_on_bbox))
                  {
                     double dtheta_shift1=
                        mathfunc::phase_to_canonical_interval(
                           dtheta[j],0,2*PI);
                     double dtheta_shift2=
                        mathfunc::phase_to_canonical_interval(
                           dtheta[modulo(j+1,dtheta.size())],0,2*PI);
                     double dtheta_shift=dtheta_shift1+dtheta_shift2;
                     const double sigma=15*PI/180;
                     if (neighbor_intersection.size() > 2)
                        weight *= 1+2*exp(-0.5*sqr((dtheta_shift-PI)/sigma));
                  }
               } // number of neighbor links for site r > 1 conditional

               curr_score *= weight;
//               cout << "weight = " << weight << " curr_score = " 
//                    << curr_score << endl;
            } // neighbor_intersection.size() > 1 conditional
*/

//               cout << "i = " << i << " curr_r_posn = " << curr_r_posn
//                    << " r_posn = " << r_posn << endl;
//               cout << "curr_score = " << curr_score << " best_score = "
//                    << best_score << endl;

            if (curr_score > best_score)
            {
               best_score=curr_score;
               best_r_posn=curr_r_posn;
            }

         } // loop over index i labeling candidate r points

         r_roadpoint_ptr->set_posn(best_r_posn);

//         cout << "r = " << r << " best score = " << best_score << endl;
//         cout << "final r posn = " 
//              << roadpoints_network_ptr->get_site_data_ptr(r)->get_posn()
//              << endl << endl;
      }

// ---------------------------------------------------------------------
// Method generate_intersection_spread takes in a starting location
// for some intersection as well as a displacement distance.  It
// generates a lattice of intersection testpoints centered upon the
// starting point.  The results are returned within an STL vector of
// test locations for the road intersection.

   vector<threevector> generate_intersection_spread(
      double displacement_dist,const threevector& start_pnt)
      {
//         const double delta_displacement=1.0;	// meter
         const double delta_displacement=2.5;	// meter
         int nbins=basic_math::round(displacement_dist/delta_displacement);
         vector<threevector> intersection_testpnts;
         intersection_testpnts.reserve(sqr(2*nbins+1));

         const threevector x_hat(1,0);
         const threevector y_hat(0,1);
         for (int i=-nbins; i<=nbins; i++)
         {
            for (int j=-nbins; j<=nbins; j++)
            {
               intersection_testpnts.push_back(
                  start_pnt+i*delta_displacement*x_hat
                  +j*delta_displacement*y_hat);
            }
         }
         return intersection_testpnts;
      }

// ---------------------------------------------------------------------
// Method generate_intersection_testpnts takes in a starting location
// for some intersection as well as a displacement direction vector
// and distance.  It returns an STL vector of test locations for the
// road intersection.

   vector<threevector> generate_intersection_testpnts(
      bool startpnt_on_data_bbox,parallelogram* data_bbox_ptr,
      double displacement_dist,const threevector& start_pnt,
      const threevector& e_hat)
      {
//         const double delta_displacement=1.0;	// meter
         const double delta_displacement=2.5;	// meter
         int nbins=basic_math::round(displacement_dist/delta_displacement);
         vector<threevector> intersection_testpnts;
         intersection_testpnts.reserve(2*nbins+1);

         bool testpnts_computed=false;
         if (startpnt_on_data_bbox)
         {
            int closest_corner=-1;
            double min_sqrd_dist=POSITIVEINFINITY;
            for (int i=0; i<4; i++)
            {
               double sqrd_dist_to_corner=(data_bbox_ptr->get_vertex(i)-
                                           start_pnt).sqrd_magnitude();
               if (sqrd_dist_to_corner < min_sqrd_dist)
               {
                  min_sqrd_dist=sqrd_dist_to_corner;
                  closest_corner=i;
               }
            }
            if (min_sqrd_dist < sqr(2*displacement_dist))
            {
               const double corner_offset=40; // meters
               int nbins=basic_math::round(corner_offset/delta_displacement);
               for (int n=-nbins; n<0; n++)
               {
                  threevector e_hat(data_bbox_ptr->get_edge(
                     modulo(closest_corner-1,4)).get_ehat());
                  intersection_testpnts.push_back(
                     data_bbox_ptr->get_vertex(closest_corner)+
                     n*delta_displacement*e_hat);
               }
               for (int n=0; n<=nbins; n++)
               {
                  threevector e_hat(data_bbox_ptr->get_edge(
                     modulo(closest_corner,4)).get_ehat());
                  intersection_testpnts.push_back(
                     data_bbox_ptr->get_vertex(closest_corner)+
                     n*delta_displacement*e_hat);
               }
               testpnts_computed=true;
//               cout << "Corner start points:" << endl;
//               templatefunc::printVector(intersection_testpnts);
            } // min_sqrd_dist < displacement_dist**2 conditional
         }

         if (!testpnts_computed)
         {
            for (int n=-nbins; n<=nbins; n++)
            {
               intersection_testpnts.push_back(
                  start_pnt+n*delta_displacement*e_hat);
            }
         } // startpnt_on_data_bbox conditional
         return intersection_testpnts;
      }

// ---------------------------------------------------------------------
// Method score_link computes a fast, thick line integral of input
// *binary_asphalt_twoDarray_ptr over the segment defined by r_posn
// and q_posn.  It returns a score equal to the ratio of the number of
// asphalt pixels over the number of total pixels.  
   
   double score_link(
      int r,int q,Network<roadpoint*> const *roadpoints_network_ptr,
      twoDarray const *binary_asphalt_twoDarray_ptr)
      {
         threevector r_posn(roadpoints_network_ptr->
                         get_site_data_ptr(r)->get_posn());
         threevector q_posn(roadpoints_network_ptr->
                         get_site_data_ptr(q)->get_posn());
         return score_link(r_posn,q_posn,binary_asphalt_twoDarray_ptr);
      }

   double score_link(
      const threevector& r_posn,const threevector& q_posn,
      twoDarray const *binary_asphalt_twoDarray_ptr)
      {
         const double line_thickness=8; // meters
         linesegment l(r_posn,q_posn);
         pair<int,double> p=
            imagefunc::fast_thick_line_integral_along_segment(
               line_thickness,xyzpfunc::null_value,
               binary_asphalt_twoDarray_ptr,l);

         double score=0;
         if (p.first > 0)
         {
            score=p.second/double(p.first);
            score=basic_math::max(0.0,score);
         } // p.first > 0 conditional
         return score;
      }
   
// ---------------------------------------------------------------------
// Method delete_low_score_segments loops over all roadpoints within
// roadfunc namespace object *intersections_network_ptr.  It thick
// line integrals of binary asphalt information to score each
// roadpoint's link to all its neighbors.  Links with small fractional
// asphalt content are deleted from *intersections_network_ptr.

   void delete_low_score_segments(
      double min_frac_score,twoDarray const *binary_asphalt_twoDarray_ptr)
      {
         outputfunc::write_banner("Deleting low scored links:");
         vector<int> intersections=fill_intersections_vector();
         for (unsigned int i=0; i<intersections.size(); i++)
         {
            int r=intersections[i];
            vector<int> neighbor_intersection=fill_neighbors_vector(r);
            delete_low_score_link(
               r,min_frac_score,
               neighbor_intersection,intersections_network_ptr,
               binary_asphalt_twoDarray_ptr);
         } // loop over i index
      }
         
// ---------------------------------------------------------------------
// Method delete_low_score_link takes in ID label r for some roadpoint
// within input *roadpoints_network_ptr.  It loops over each q
// neighbor to r and scores its link to r based upon the fractional
// asphalt content between r and q.  Links with fractional asphalt
// scores less than input parameter min_frac_score are symmetrically
// deleted from *roadpoints_network_ptr.
   
   void delete_low_score_link(
      int r,double min_frac_score,vector<int> neighbor_intersection,
      Network<roadpoint*>* roadpoints_network_ptr,
      twoDarray const *binary_asphalt_twoDarray_ptr)
      {
         roadpoint* r_roadpoint_ptr=roadpoints_network_ptr->
            get_site_data_ptr(r);
         threevector r_posn(r_roadpoint_ptr->get_posn());

         for (unsigned int n=0; n<neighbor_intersection.size(); n++)
         {
            int q=neighbor_intersection[n];
            roadpoint* q_roadpoint_ptr=roadpoints_network_ptr->
               get_site_data_ptr(q);
            threevector q_posn(q_roadpoint_ptr->get_posn());
            double roadlink_score=score_link(
               r_posn,q_posn,binary_asphalt_twoDarray_ptr);
            
            if (roadlink_score < min_frac_score &&
                !(r_roadpoint_ptr->get_near_bldg_island() &&
                  q_roadpoint_ptr->get_near_bldg_island()))
            {
               cout << "Deleting link between r = " << r << " and q = " 
                    << q << endl;
               cout << "roadlink score = " << roadlink_score
                    << " min_frac_score = " << min_frac_score << endl;
               cout << "r_posn = " << r_posn << " q_posn = "
                    << q_posn << endl;
               if (r_roadpoint_ptr->get_near_bldg_island())
               {
                  cout << "r is near a bldg island" << endl;
               }
               if (q_roadpoint_ptr->get_near_bldg_island())
               {
                  cout << "q is near a bldg island" << endl;
               }
               roadpoints_network_ptr->delete_symmetric_link(r,q);
            }
         } // loop over index n labeling q neighbors to r
      }

// ---------------------------------------------------------------------
// Method insert_fake_intersections_nearby_bldg_islands scans over the
// entire intersections network.  It looks for pairs of intersection
// sites which are both located nearby a building island.  It adds a
// fake site between two such genuine intersections into the
// intersections network.  This operation should hopefully allow for
// greater flexibility in refining the intersections network in the
// vicinity of building islands.

   void insert_fake_intersections_nearby_bldg_islands()
      {
         outputfunc::write_banner(
            "Inserting fake intersections nearby building islands:");

         vector<int> intersections=fill_intersections_vector();
         for (unsigned int i=0; i<intersections.size(); i++)
         {
            int r=intersections[i];
            vector<int> neighbor_intersection=fill_neighbors_vector(r);
            test_intersections_near_bldg_islands(r,neighbor_intersection);
         } // loop over i index
      }
   
// ---------------------------------------------------------------------
// Method test_intersections_near_bldg_islands takes in ID label r for
// some roadpoint within the intersections network.  If this
// intersection site lies near some building island, this method
// checks its neighboring intersection sites.  For each neighbor q
// which also lies near a building island, this method inserts a fake
// site half-way between r and q.

   void test_intersections_near_bldg_islands(
      int r,vector<int>& neighbor_intersection)
      {
         roadpoint* r_roadpoint_ptr=intersections_network_ptr->
            get_site_data_ptr(r);
         if (r_roadpoint_ptr->get_near_bldg_island())
         {
            threevector r_posn(r_roadpoint_ptr->get_posn());

            for (unsigned int n=0; n<neighbor_intersection.size(); n++)
            {
               int q=neighbor_intersection[n];
               roadpoint* q_roadpoint_ptr=intersections_network_ptr->
                  get_site_data_ptr(q);

               if (q_roadpoint_ptr->get_near_bldg_island())
               {
                  threevector q_posn(q_roadpoint_ptr->get_posn());
                  threevector midpoint(0.5*(r_posn+q_posn));
                  roadpoint* new_roadpoint_ptr=new roadpoint(midpoint);
                  new_roadpoint_ptr->set_intersection(true);
                  new_roadpoint_ptr->set_near_bldg_island(true);
                  intersections_network_ptr->
                     insert_site_along_link(Site<roadpoint*>(
                        new_roadpoint_ptr),r,q);
               } // q near bldg island conditional
            } // loop over index n labeling neighbor roadpoints
         } // r near bldg island conditional
      }

// ---------------------------------------------------------------------
// Method insert_sites_at_netlink_intersections takes in some
// roadpoints network.  It searches for all intersections between
// pairs of road links.  This method instantiates new sites at those
// intersection pointsand inserts them into the roadpoints network.

   void insert_sites_at_netlink_intersections(
      Network<roadpoint*>* roadpoints_network_ptr)
      {
         vector<intersection_triple> netlink_intersections
            =roadpoints_network_ptr->search_for_intersecting_netlinks();

         for (unsigned int i=0; i<netlink_intersections.size(); i++)
         {
            intersection_triple intersection(netlink_intersections[i]);
//            int r=intersection.first.first;
//            int q=intersection.first.second;
//            int r2=intersection.second.first;
//            int q2=intersection.second.second;
            threevector intersection_posn(intersection.third);
      
//            cout << "i = " << i << " r = " << r << " q = " << q << endl;
//            cout << "r2 = " << r2 << " q2 = " << q2 << endl;
//            cout << "posn = " << intersection_posn << endl;

            roadpoint* new_intersection_ptr=new roadpoint(intersection_posn);
            new_intersection_ptr->set_intersection(true);
            roadpoints_network_ptr->insert_site_at_netlink_intersection(
               Site<roadpoint*>(new_intersection_ptr),
               netlink_intersections[i]);
         } // loop over index i
      }

// ---------------------------------------------------------------------
// Method test_linear_links_between_intersections takes in a
// roadpoints network which is assumed to have already had significant
// processing applied in order to locate intersection points.  This
// method attempts to eliminate glaring kinks between intersections.
// In particular, it computes the asphalt fractional score for linear
// links between all pairs of genuine intersections which have
// intervening non-intersection roadpoints between them.  If the
// fractional score exceeds some large threshold value, the
// intervening roadpoints are deleted from the network, and a
// symmetric direct link between the intersections is generated.  

   void test_linear_links_between_intersections(
      Network<roadpoint*>* roadpoints_network_ptr,
      parallelogram* data_bbox_ptr,
      twoDarray const *binary_asphalt_twoDarray_ptr)
      {
         outputfunc::write_banner(
            "Testing linear links between intersections:");

         for (Mynode<int>* currnode_ptr=roadpoints_network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int r=currnode_ptr->get_data(); // roadpoint number
            Site<roadpoint*>* curr_roadpoint_site_ptr=
               roadpoints_network_ptr->get_site_ptr(r);
            
            if (curr_roadpoint_site_ptr->get_data()->get_intersection())
            {
               vector<int> intersection_neighbors=
                  roadfunc::find_intersection_neighbors_to_intersection(
                     r,roadfunc::intersections_network_ptr,data_bbox_ptr);

// Loop over all genuine intersection neighbors to intersection r:

               for (unsigned int i=0; i<intersection_neighbors.size(); i++)
               {
                  int q=intersection_neighbors[i];
                  if (!roadfunc::intersections_network_ptr->neighboring_site(
                     r,q))
                  {
                     threevector r_posn(roadfunc::intersections_network_ptr->
                                     get_site_data_ptr(r)->get_posn());
                     threevector q_posn(roadfunc::intersections_network_ptr->
                                     get_site_data_ptr(q)->get_posn());
                     double straight_line_score=roadfunc::score_link(
                        r_posn,q_posn,binary_asphalt_twoDarray_ptr);
//                     cout << "straight line score = " 
//                          << straight_line_score << endl;

// If straight line score between intersections r and q exceeds some
// threshold, delete all intermediate roadpoints between r & q from
// roadpoints network and create symmetric direct link between r & q:

                     const double min_straight_line_score=0.95;
                     if (straight_line_score > min_straight_line_score)
                     {
                        Linkedlist<int>* intermediate_sites_ptr=
                           roadfunc::intersections_network_ptr->
                           shortest_path_between_sites(r,q);
//                        cout << "shortest path between r and q = "
//                             << *intermediate_sites_ptr << endl;
                        for (Mynode<int>* currnode_ptr=
                                intermediate_sites_ptr->get_start_ptr()->
                                get_nextptr(); currnode_ptr != 
                                intermediate_sites_ptr->get_stop_ptr(); 
                             currnode_ptr=currnode_ptr->get_nextptr())
                        {
                           roadfunc::intersections_network_ptr->
                              delete_single_site(currnode_ptr->get_data());
                        } // loop over intermediate sites between r and q
                        roadfunc::intersections_network_ptr->
                           add_symmetric_link(r,q);
                     } // straight line score > threshold conditional
                  } // r & q are not directly adjacent neighbors conditional
               } // loop over index i labeling intersection neighbors to r
            } // curr roadpoint is intersection conditional
         } // loop over all roadpoints within input network
      }

// ---------------------------------------------------------------------
// Method move_roadpoints_near_data_bbox_onto_bbox loops over all
// roadpoints within input network *roadpoints_network_ptr.  It checks
// how close each roadpoint is to the input data bounding box.  If the
// distance is less than some small threshold, this method resets the
// position of such roadpoints equal to the closest point on the
// bounding box.  This method was created for the purposes of
// eliminating aesthetically displeasing intersections which appear
// too close to the edge of a Lowell chunk.

   void move_roadpoints_near_data_bbox_onto_bbox(
      Network<roadpoint*>* roadpoints_network_ptr,
      parallelogram* data_bbox_ptr)
      {
         outputfunc::write_banner(
            "Moving roadpoints near data bbox onto bbox:");

         for (Mynode<int>* currnode_ptr=roadpoints_network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int r=currnode_ptr->get_data(); // roadpoint number
            roadpoint* curr_roadpoint_ptr=
               roadpoints_network_ptr->get_site_data_ptr(r);

            pair<double,threevector> p(
               data_bbox_ptr->closest_polygon_perimeter_point(
                  curr_roadpoint_ptr->get_posn()));

            const double min_distance=10;	// meters
            if (p.first < min_distance)
            {
//               curr_roadpoint_ptr->set_posn(p.second);
               curr_roadpoint_ptr->set_posn(radially_scale_roadpoint_on_bbox(
                  p.second,data_bbox_ptr));
               curr_roadpoint_ptr->set_on_data_bbox(true);
            }
         } // loop over sites in roadpoints network
      }

// ==========================================================================
// Road network display methods
// ==========================================================================

// Method draw_road_network loops over all entries within the
// roadpoints network.  It draws 3D linesegments between each
// roadpoint and its nearest roadpoint neighbors onto the output XYZP
// file.  The segments are translated in the z direction by input
// parameter delta_z.  Because the segments are drawn in
// three-dimensions rather than draped onto a twoDarray, roads links
// can appear underneath overhanging trees.

   void draw_road_network(
      Network<roadpoint*>* roadpoints_network_ptr,string xyzp_filename,
      double annotation_value,double delta_z)
      {
         filefunc::gunzip_file_if_gzipped(xyzp_filename);
         draw3Dfunc::draw_thick_lines=true;

         for (Mynode<int>* currnode_ptr=roadpoints_network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int r=currnode_ptr->get_data();
            roadpoint* curr_roadpoint_ptr=roadpoints_network_ptr->
               get_site_data_ptr(r);
            threevector r_posn(curr_roadpoint_ptr->get_posn()+1*z_hat);

            Linkedlist<netlink>* curr_roadlink_list_ptr
               =roadpoints_network_ptr->get_site_ptr(r)->
               get_netlink_list_ptr();

            if (curr_roadlink_list_ptr != NULL)
            {
               for (Mynode<netlink>* roadlink_node_ptr=
                       curr_roadlink_list_ptr->get_start_ptr();
                    roadlink_node_ptr != NULL; roadlink_node_ptr=
                       roadlink_node_ptr->get_nextptr())
               {
                  int q=roadlink_node_ptr->get_data().get_ID();

// Require q > r in order to avoid drawing r-q road link more than
// once:

                  if (q > r)
                  {
                     roadpoint* next_roadpoint_ptr=roadpoints_network_ptr->
                        get_site_data_ptr(q);
                     threevector q_posn(next_roadpoint_ptr->get_posn()+1*z_hat);
                     linesegment l(r_posn,q_posn);
                     l.translate(delta_z*z_hat);
                     draw3Dfunc::draw_line(l,xyzp_filename,annotation_value);
                  }
               } // loop over road links for current road point
            } // curr_roadlink_list_ptr != NULL conditional
         } // loop over roadpoints network
      }
   
// ---------------------------------------------------------------------
// This overloaded version of draw_road_network loops over all entries
// within the roadpoints hashtable. It draws linesegments between each
// roadpoint and its nearest roadpoint neighbors onto output twoDarray
// *ftwoDarray_ptr.  It can also draw linesegments between each
// roadpoint and the closest nearby buildings.  This particular method
// is intended to be high-level and user-friendly.

   void draw_road_network(
      Network<roadpoint*>* roadpoints_network_ptr,twoDarray* ftwoDarray_ptr,
      bool display_roadpoints_flag,bool display_bbox_intersections)
      {
         outputfunc::write_banner("Drawing road network:");

         if (roadpoints_network_ptr==NULL) 
         {
            cout << "inside roadfunc::draw_road_network()" << endl;
            cout << "Roadpoints_network_ptr = NULL !!!" << endl;
            exit(-1);
         }
         
         for (Mynode<int>* currnode_ptr=roadpoints_network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int r=currnode_ptr->get_data(); // roadpoint number
            draw_roadpoint_links(roadpoints_network_ptr,r,ftwoDarray_ptr);
//      draw_nearby_buildings_to_roadpoint(r,ftwoDarray_ptr);
         }
         if (display_roadpoints_flag) draw_roadpoints(
            roadpoints_network_ptr,ftwoDarray_ptr,display_bbox_intersections);
      }

// ---------------------------------------------------------------------
// Method draw_roadpoints loops over all entries within the roadpoints
// hashtable and draws their positions onto output twoDarray
// *ftwoDarray_ptr as white points.

   void draw_roadpoints(Network<roadpoint*>* roadpoints_network_ptr,
                        twoDarray* ftwoDarray_ptr,
                        bool display_bbox_intersections)
      {
         outputfunc::write_banner("Drawing road points:");
         double radius,color;
         for (Mynode<int>* currnode_ptr=roadpoints_network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int r=currnode_ptr->get_data(); // roadpoint number
            roadpoint* curr_roadpoint_ptr=roadpoints_network_ptr->
               get_site_data_ptr(r);
            if (curr_roadpoint_ptr != NULL)
            {
               if (curr_roadpoint_ptr->get_intersection())
               {
                  radius=4.0;
                  color=draw3Dfunc::annotation_value2;
                  if (curr_roadpoint_ptr->get_on_data_bbox())
                  {
//                     color=draw3Dfunc::annotation_value3;
                  }
                  if (display_bbox_intersections ||
                      (!display_bbox_intersections && 
                       !curr_roadpoint_ptr->get_on_data_bbox()))
                  {
                     drawfunc::draw_hugepoint(
                        curr_roadpoint_ptr->get_posn(),radius,color,
                        ftwoDarray_ptr);
                  }
               }
               else
               {
//                  radius=2.5;
//                  radius=1.5;
                  radius=0.5;
//                  color=draw3Dfunc::annotation_value1;
               }
         
//         if (curr_roadpoint_ptr->get_in_front_of_bldg())
//         {
//            radius=2.5;
//            color=urbanimage::annot3_value;
//         }
               drawfunc::draw_hugepoint(
                  curr_roadpoint_ptr->get_posn(),radius,color,ftwoDarray_ptr);
            }
         } // loop over sites in roadpoints network
      }

// ---------------------------------------------------------------------
// Method draw_roadpoint_links takes in ID label r for some entry in
// the roadpoints hashtable.  It draws all of the links between this
// roadpoint and its connecting roadpoint neighbors onto output
// twoDarray *ftwoDarray_ptr.

   void draw_roadpoint_links(
      Network<roadpoint*>* roadpoints_network_ptr,
      int r,twoDarray* ftwoDarray_ptr)
      {
         Site<roadpoint*>* curr_roadpoint_site_ptr=
            roadpoints_network_ptr->get_site_ptr(r);
         Linkedlist<netlink>* curr_roadlink_list_ptr
            =curr_roadpoint_site_ptr->get_netlink_list_ptr();
            
         if (curr_roadlink_list_ptr != NULL)
         {
            for (Mynode<netlink>* curr_roadlink_node_ptr=
                    curr_roadlink_list_ptr->get_start_ptr();
                 curr_roadlink_node_ptr != NULL; 
                 curr_roadlink_node_ptr=curr_roadlink_node_ptr->get_nextptr())
            {
               int q=curr_roadlink_node_ptr->get_data().get_ID();
               
// Avoid drawing road links twice by requiring q > r:

               if (q > r) draw_road_link(
                  roadpoints_network_ptr,r,q,ftwoDarray_ptr);
            }
         } // curr_roadlink_list_ptr != NULL conditional
      }

// ---------------------------------------------------------------------
// Method draw_road_link takes in ID labels r and q for two
// roadpoints.  It draws a line segment between them onto output
// twoDarray *ftwoDarray_ptr.

   void draw_road_link(Network<roadpoint*>* roadpoints_network_ptr,
                       int r,int q,twoDarray* ftwoDarray_ptr)
      {
         linesegment l(
            roadpoints_network_ptr->get_site_data_ptr(r)->get_posn(),
            roadpoints_network_ptr->get_site_data_ptr(q)->get_posn());
//         const double radius=0.5;	// meter
//         const double radius=0.75;	// meter
         const double radius=1.0;	// meter

         if (roadpoints_network_ptr->get_site_data_ptr(r)->get_at_infinity() 
             ||
             roadpoints_network_ptr->get_site_data_ptr(q)->get_at_infinity())
         {
            drawfunc::draw_thick_line(
               l,colorfunc::white,radius,ftwoDarray_ptr);
         }
         else
         {
            drawfunc::draw_thick_line(
               l,colorfunc::white,radius,ftwoDarray_ptr);

// On 11/29/04, we empirically observed that widths of most
// suburban Lowell roads are equal or less than 10 meters.

//            threevector eperp_hat(-l.get_ehat().get(1),l.get_ehat().get(0));
//            double displacement=10;	// meters
//            threevector v1(l.v1+displacement*eperp_hat);
//            threevector v2(l.v2+displacement*eperp_hat);
//            linesegment lnew(v1,v2);
//            drawfunc::draw_thick_line(
//               lnew,colorfunc::white,radius,ftwoDarray_ptr);
         }
      }
   
// ---------------------------------------------------------------------
// Method annotate_roadpoint_labels loops over each entry within the
// roadpoints network.  It generates a threeDstring for each roadpoint
// corresponding to its integer label.  This method adds the
// threeDstring label above the roadpoint within the output xyzp file.

   void annotate_roadpoint_labels(
      Network<roadpoint*>* roadpoints_network_ptr,
      string xyzp_filename,double annotation_value)
      {
         filefunc::gunzip_file_if_gzipped(xyzp_filename);
         const double roadpoint_label_size=0.5;

         for (Mynode<int>* currnode_ptr=roadpoints_network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int r=currnode_ptr->get_data();
            threeDstring roadpoint_label(stringfunc::number_to_string(r));
            roadpoint_label.scale(roadpoint_label.get_origin().get_pnt(),
                                  roadpoint_label_size);
            roadpoint* curr_roadpoint_ptr=roadpoints_network_ptr->
               get_site_data_ptr(r);

            threevector annotation_point(
               curr_roadpoint_ptr->get_posn()+10*z_hat);
            roadpoint_label.center_upon_location(annotation_point);

            draw3Dfunc::draw_threeDstring(
               roadpoint_label,xyzp_filename,annotation_value);
         } // loop over sites in roadpoints network 
//         filefunc::gzip_file(xyzp_filename);         
      }

// ==========================================================================
// Road network text input/output methods:
// ==========================================================================

// Member function output_road_network_to_textfile loops over all
// nodes within the input *roadpoints_network_ptr.  For each
// roadpoint, it writes to an output text file its ID value, its
// 3-position, various boolean flags and the IDs of each its
// neighboring roadpoints.

   void output_road_network_to_textfile(
      Network<roadpoint*>* roadpoints_network_ptr,
      parallelogram* data_bbox_ptr,string output_filename)
      {
         outputfunc::write_banner("Writing road network to output textfile:");
         
         ofstream outstream;
         filefunc::deletefile(output_filename);
         filefunc::openfile(output_filename,outstream);

         for (Mynode<int>* currnode_ptr=roadpoints_network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int r=currnode_ptr->get_data(); // roadpoint number
            output_roadpoint_and_its_links(
               roadpoints_network_ptr,data_bbox_ptr,r,outstream);
         }
         filefunc::closefile(output_filename,outstream);
      }

// ---------------------------------------------------------------------
// Method output_roadpoint_and_its_links takes in ID label r for some
// entry in the roadpoints hashtable.  It writes to an output text
// file information about this roadpoint, the number of its roadpoints
// neighbors and the ID labels of its roadpoint neighbors.

   void output_roadpoint_and_its_links(
      Network<roadpoint*>* roadpoints_network_ptr,
      parallelogram* data_bbox_ptr,int r,ofstream& outstream)
      {
         roadpoint* curr_roadpoint_ptr=
            roadpoints_network_ptr->get_site_data_ptr(r);

         outstream << r << endl;
         threevector road_posn(curr_roadpoint_ptr->get_posn());
         outstream << road_posn.get(0) << " "
                   << road_posn.get(1) << " "
                   << road_posn.get(2) << endl;
         outstream << curr_roadpoint_ptr->get_intersection() 
                   << " # intersection" << endl;
         outstream << curr_roadpoint_ptr->get_in_front_of_bldg() 
                   << " # in front of bldg boolean" << endl;
         outstream << curr_roadpoint_ptr->get_on_data_bbox() 
                   << " # on data bbox boolean" << endl;
         outstream << curr_roadpoint_ptr->get_data_bbox_corner() 
                   << " # data bbox corner boolean" << endl;

         Site<roadpoint*>* curr_roadpoint_site_ptr=
            roadpoints_network_ptr->get_site_ptr(r);
         Linkedlist<netlink>* curr_roadlink_list_ptr
            =curr_roadpoint_site_ptr->get_netlink_list_ptr();
            
         if (curr_roadlink_list_ptr != NULL)
         {
            outstream << curr_roadlink_list_ptr->size() << endl;
            for (Mynode<netlink>* curr_roadlink_node_ptr=
                    curr_roadlink_list_ptr->get_start_ptr();
                 curr_roadlink_node_ptr != NULL; 
                 curr_roadlink_node_ptr=curr_roadlink_node_ptr->get_nextptr())
            {
               int q=curr_roadlink_node_ptr->get_data().get_ID();
               outstream << q << " # roadpoint neighbor ID" << endl;
            }
         } // curr_roadlink_list_ptr != NULL conditional
         else
         {
            outstream << "0  # No neighbors" << endl;	 
				// no neighboring roadpoints
         }
         outstream << endl;
      }

// ---------------------------------------------------------------------
// Method readin_road_network_from_textfile performs the inverse
// operation of method output_road_network_to_textfile.  It
// dynamically generates a road network based upon the information
// read in from a text file generated by
// output_road_network_to_textfile.

   Network<roadpoint*>* readin_road_network_from_textfile(
      string input_filename)
      {
         outputfunc::write_banner("Reading in road network from text file:");
         vector<string> line;
         filefunc::ReadInfile(input_filename,line);
         stringfunc::comment_strip(line);

         Network<roadpoint*>* roadpoints_network_ptr=
            new Network<roadpoint*>(10*line.size());

         unsigned int i=0;
         while (i<line.size())
         {
            int r=stringfunc::string_to_number(line[i++]);
            vector<double> V=stringfunc::string_to_numbers(line[i++]);
            threevector r_posn(V[0],V[1],V[2]);
            roadpoint* curr_roadpoint_ptr=new roadpoint(r,r_posn);

// For neighbor drawing purposes, we set each roadpoint's "center"
// equal to its "position":

            curr_roadpoint_ptr->set_center(r_posn);

            bool intersection=stringfunc::string_to_number(line[i++]);
            bool ifob=stringfunc::string_to_number(line[i++]);
            bool odb=stringfunc::string_to_number(line[i++]);
            bool dbc=stringfunc::string_to_number(line[i++]);
            curr_roadpoint_ptr->set_intersection(intersection);
            curr_roadpoint_ptr->set_in_front_of_bldg(ifob);
            curr_roadpoint_ptr->set_on_data_bbox(odb);
            curr_roadpoint_ptr->set_data_bbox_corner(dbc);

            roadpoints_network_ptr->insert_site(
               r,Site<roadpoint*>(curr_roadpoint_ptr));

            int n_neighbors=stringfunc::string_to_number(line[i++]);
            for (int n=0; n<n_neighbors; n++)
            {
               int q=stringfunc::string_to_number(line[i++]);
               roadpoints_network_ptr->add_to_neighbor_list(r,q);
            }
         } // i < line.size() conditional
         return roadpoints_network_ptr;
      }

} // roadfunc namespace

