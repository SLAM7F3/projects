// ==========================================================================
// CITYBLOCK base class member function definitions
// ==========================================================================
// Last modified on 4/28/06; 5/22/06; 6/14/06; 7/29/06; 12/4/10
// ==========================================================================

#include <algorithm>
#include <iostream>
#include <set>
#include <vector>
#include "urban/bldgstrandfuncs.h"
#include "urban/building.h"
#include "urban/cityblock.h"
#include "urban/cityblockfuncs.h"
#include "math/constants.h"
#include "geometry/contour.h"
#include "geometry/linesegment.h"
#include "datastructures/Linkedlist.h"
#include "math/mathfuncs.h"
#include "math/threevector.h"
#include "urban/oriented_box.h"
#include "geometry/parallelogram.h"
#include "geometry/polygon.h"
#include "urban/roadpoint.h"
#include "urban/roadsegment.h"
#include "urban/rooftop.h"
#include "general/stringfuncs.h"
#include "threeDgraphics/threeDstring.h"

using std::cout;
using std::endl;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void cityblock::allocate_member_objects()
{
}		       

void cityblock::initialize_member_objects()
{
   n_buildings=0;
   building_tadpoles_list_ptr=NULL;
   block_buildings_network_ptr=NULL;
   block_roadsegment_ptrs=NULL;
   strand_list_ptr=NULL;
}		       

cityblock::cityblock():
   contour_element()
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

cityblock::cityblock(int identification):
   contour_element(identification)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

cityblock::cityblock(int identification,const threevector& posn):
   contour_element(identification,posn)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

// Copy constructor:

cityblock::cityblock(const cityblock& b):
   contour_element(b)
{
   initialize_member_objects();
   allocate_member_objects();
   docopy(b);
}

cityblock::~cityblock()
{
   delete building_tadpoles_list_ptr;
   delete block_buildings_network_ptr;

// FAKE FAKE:...Must delete each of the individually allocated
// buildings and roadsegments within STL vectors before deleting the
// vectors themselves...

// Sun, Mar 13 at 2:54 pm

   delete block_roadsegment_ptrs;

   for (Mynode<Strand<building_info*>*>* currnode_ptr=
           strand_list_ptr->get_start_ptr();
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      Strand<building_info*>* curr_strand_ptr=currnode_ptr->get_data();
      for (Mynode<building_info*>* bldg_node_ptr=curr_strand_ptr->
              get_start_ptr(); bldg_node_ptr != NULL;
           bldg_node_ptr=bldg_node_ptr->get_nextptr())
      {
         building_info* curr_bldg_info_ptr=bldg_node_ptr->get_data();
         delete curr_bldg_info_ptr;
      }
      delete curr_strand_ptr;
   }
   delete strand_list_ptr;
}

// ---------------------------------------------------------------------
void cityblock::docopy(const cityblock& b)
{
   n_buildings=b.n_buildings;

// Need to dynamically allocate STL vectors and copy over their
// contents from b's vectors...  Sun Mar 13 at 2:55 pm.

//   buildings_tadpoles_list_ptr=b.buildings_tadpoles_list_ptr;
//   block_building_ptrs=b.block_building_ptrs;
//   block_road_ptrs=b.block_road_ptrs;
//   strand_list_ptr=b.strand_list_ptr;
}

// ---------------------------------------------------------------------
// Overload = operator:

cityblock& cityblock::operator= (const cityblock& b)
{
   if (this==&b) return *this;
   contour_element::operator=(b);
   docopy(b);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (std::ostream& outstream,const cityblock& b)
{
   outstream << std::endl;
   outstream << (contour_element&)b << endl;
   outstream << "n_buildings = " << b.get_n_buildings() << endl;
   return outstream;
}

// ==========================================================================
// Road spatial relationships methods
// ==========================================================================

// Method find_road_segments first determines which roadpoints within
// input *roadpoints_network_ptr lie along the cityblock's contour.
// It then searches for adjacent pairs of these roadpoints which are
// network neighbors.  This method interprets such pairs as forming a
// continguous road segment that borders the cityblock region.

// We implemented this method in Mar 05 in order to associate genuine
// road segments within the intersections network with particular
// cityblocks and to discard any segments lying along the data
// bounding box.

void cityblock::find_road_segments(
   Network<roadpoint*> const *roadpoints_network_ptr)
{
   const double max_distance=16;	// meters
   vector<int> nearby_roadpoints=identify_nearby_roadpoints(
      max_distance,roadpoints_network_ptr);
//   cout << "Nearby roadpoints to cityblock:" << endl;
//   templatefunc::printVector(nearby_roadpoints);

   int ID=0;
   set_block_roadsegment_ptrs(new vector<roadsegment*>);

   for (unsigned int i=0; i<nearby_roadpoints.size(); i++)
   {
      int r=nearby_roadpoints[i];
      int r_next=nearby_roadpoints[
         modulo(i+1,nearby_roadpoints.size())];

//      cout << "r = " << r << " r_next = " << r_next << endl;
      const Site<roadpoint*>* curr_site_ptr=roadpoints_network_ptr->
         get_site_ptr(r);
      vector<int> nearby_roadpoint_neighbors=
         curr_site_ptr->get_neighbors();

      bool road_segment_found=false;
      for (unsigned int j=0; j<nearby_roadpoint_neighbors.size(); j++)
      {
         if (nearby_roadpoint_neighbors[j]==r_next) 
            road_segment_found=true;
      }
            
      if (road_segment_found)
      {
         if (nearby_roadpoints.size() > 2 ||
             (nearby_roadpoints.size()==2 && i==0))
         {
            roadsegment* curr_segment_ptr=new roadsegment(
               ID++,r,r_next,roadpoints_network_ptr);
            get_block_roadsegment_ptrs()->push_back(curr_segment_ptr);
//            cout << "After pushing back roadsegment* curr_segment_ptr" 
//                 << endl;
//            cout << "Vector size = "
//                 << get_block_roadsegment_ptrs()->size() << endl;
//            cout << "Segment stored in STL vector = " 
//                 << *(get_block_roadsegment_ptrs()->back()) << endl;
         }
      } // road_segment_found conditional
   } // loop over roadpoints nearby cityblock 
}

// ---------------------------------------------------------------------
// Member function identify_nearby_roadpoints takes in a network of
// roadpoints within *roadpoints_network_ptr.  This method first
// converts the cityblock's contour into a polygon.  It then computes
// the distance of every roadpoint within the input network to the
// polygon.  The IDs for those roadpoints whose distance is less than
// some small distance threshold are saved and returned within an STL
// vector of integers.  The STL vector is ordered so that its entries
// form a right-handed cycle.

vector<int> cityblock::identify_nearby_roadpoints(
   double max_distance,Network<roadpoint*> const *roadpoints_network_ptr)
{
   polygon poly(get_contour_ptr());

   vector<pair<double,int> > roadpoints_near_poly;
   for (const Mynode<int>* currnode_ptr=roadpoints_network_ptr->
           get_entries_list_ptr()->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int r=currnode_ptr->get_data(); // roadpoint number
      roadpoint* curr_roadpoint_ptr=
         roadpoints_network_ptr->get_site_data_ptr(r);

      pair<double,threevector> p( poly.closest_polygon_perimeter_point(
         curr_roadpoint_ptr->get_posn()) );
      if (p.first < max_distance)
      {
         double frac=poly.frac_distance_along_polygon(p.second);
         roadpoints_near_poly.push_back(pair<double,int>(frac,r));
//         cout << "Roadpoint r = " << r << " is near block " << get_ID()
//              << " poly frac = " << frac << endl;
      }
   } // loop over nodes in *roadpoints_network_ptr

// Sort roadpoints near polygon based upon their fractional distance
// along the polygon.  This should guarantee that they will form a
// right-handed cycle wrt z_hat:

   std::sort(roadpoints_near_poly.begin(),roadpoints_near_poly.end());

   vector<int> roadpoints_near_cityblock;
   for (unsigned int i=0; i<roadpoints_near_poly.size(); i++)
   {
      int r=roadpoints_near_poly[i].second;
      roadpoints_near_cityblock.push_back(r);
//      cout << i << " roadpoint = " << roadpoints_near_cityblock.back() 
//           << endl;
   }

   return roadpoints_near_cityblock;
}

// ==========================================================================
// Building spatial relationships methods
// ==========================================================================

// Member function identify_building_tadpoles starts with buildings
// for the current cityblock object that represent genuine
// "monopoles".  It searches for their neighbors until it encounters a
// building with 3 or more neighbors.  The tadpole strands for each
// monopole are concatenated together and stored within
// *building_tadpoles_list_ptr for each cityblock.
   
void cityblock::identify_building_tadpoles()
{
   vector<int> monopoles=block_buildings_network_ptr->
      generate_npole_list(1);

   building_tadpoles_list_ptr=new Linkedlist<int>;
   for (unsigned int i=0; i<monopoles.size(); i++)
   {
      int r_monopole=monopoles[i];
      Linkedlist<int>* particular_tadpole_list_ptr=
         block_buildings_network_ptr->monopole_strand_members(r_monopole);
      building_tadpoles_list_ptr->concatenate_wo_duplication(
         particular_tadpole_list_ptr);
      delete particular_tadpole_list_ptr;
   } // loop over monopoles STL vector
//   cout << "Building tadpoles: " << *building_tadpoles_list_ptr << endl;
}

// ---------------------------------------------------------------------
// Member function identify_buildings_on_street sets to false the
// on_street flag for any building which has no nontrivial netlink
// list.

void cityblock::identify_buildings_on_street()
{
   for (Mynode<int>* currnode_ptr=block_buildings_network_ptr->
           get_entries_list_ptr()->get_start_ptr();
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int n=currnode_ptr->get_data();
      building* b_ptr=get_building_ptr(n);
      Linkedlist<netlink>* bldglink_list_ptr=
         block_buildings_network_ptr->get_site_ptr(n)->get_netlink_list_ptr();

// Define streets islands to be on street:

      if (b_ptr->get_is_street_island())
      {
         b_ptr->set_on_street(true);
      }
      else
      {
         b_ptr->set_on_street(false);
         if (bldglink_list_ptr != NULL)
         {
            if (bldglink_list_ptr->size() >= 1)
            {
               b_ptr->set_on_street(true);
            }
         }
      } // curr bldg is street island conditional
   } // loop over nodes in *block_buildings_network_ptr
}

// ---------------------------------------------------------------------
// Member function estimate_building_front_dirs loops over all
// buildings within *block_buildings_network_ptr.  If the building
// belongs to a tadpole strand, we liberally set its front direction
// based upon the location of the closest road segment.  If the
// building represents a conventional dipole, we more conservatively
// set its front direction based upon the closest road segment.

void cityblock::estimate_building_front_dirs()
{
   for (Mynode<int>* currnode_ptr=block_buildings_network_ptr->
           get_entries_list_ptr()->get_start_ptr();
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int n=currnode_ptr->get_data();
      
      building* b_ptr=get_building_ptr(n);
//      cout << "building n = " << n 
//           << " on corner = " << b_ptr->get_on_street_corner() 
//           << " island flag = " << b_ptr->get_is_street_island() << endl;
      
      if (!b_ptr->get_is_street_island())
      {
         double max_bldg_to_road_dist;
         if (b_ptr->get_on_street_corner())
         {
            max_bldg_to_road_dist=50;	// meters
            estimate_corner_building_front_dirs(n,max_bldg_to_road_dist);
         }
         else if (building_tadpoles_list_ptr->data_in_list(n) != NULL)
         {
            max_bldg_to_road_dist=50;	// meters
            estimate_building_front_dir(n,max_bldg_to_road_dist);
         }
         else
         {
            max_bldg_to_road_dist=20;	// meters
            estimate_building_front_dir(n,max_bldg_to_road_dist);
         }
      } // building not street island conditional
   } // loop over block buildings network
}

// ---------------------------------------------------------------------
// Member function estimate_building_front_dir takes in integer ID n
// for some building within the current cityblock's buildings network.
// It computes the shortest distance from the building's position to
// the block's surrounding road segments.  If that distance is less
// than some maximal threshold, the direction vector pointing from the
// building's position towards the road segment is stored within the
// building's front_dir[0] member variable.  Otherwise, front_dir[0]
// is set equal to Zero_vector.

void cityblock::estimate_building_front_dir(
   int n,double max_bldg_to_road_dist)
{
   building* b_ptr=get_building_ptr(n);
   Linkedlist<threevector>* footprint_vertices_list_ptr=
      b_ptr->generate_footprint_vertices_list();

// Initialize front direction to Zero_vector:

   b_ptr->set_front_dir(0,Zero_vector);
   
//   int closest_segment_ID=-1;
   double min_v2l_dist=POSITIVEINFINITY;
   threevector f_hat;
   for (unsigned int r=0; r<block_roadsegment_ptrs->size(); r++)
   {
      roadsegment* road_ptr=(*block_roadsegment_ptrs)[r];
      linesegment l(road_ptr->get_segment());
//      cout << "roadsegment r = " << r << " linesegment = " << l << endl;

// Loop over all vertices within current building's footprint and
// compute distance from each to linesegment l:

      for (Mynode<threevector>* currnode_ptr=footprint_vertices_list_ptr->
              get_start_ptr(); currnode_ptr != NULL; currnode_ptr=
              currnode_ptr->get_nextptr())
      {
         threevector curr_vertex(currnode_ptr->get_data());
         threevector closest_pnt_on_l;
         double v2l_dist=l.point_to_line_segment_distance(
            curr_vertex,closest_pnt_on_l);
         threevector v2l=closest_pnt_on_l-curr_vertex;

         if (v2l_dist < min_v2l_dist)
         {
            min_v2l_dist=v2l_dist;
//            closest_segment_ID=r;
            f_hat=v2l.unitvector();
         }
      } // loop over footprint vertices
//      cout << "closest segment ID = " << closest_segment_ID
//           << " min_v2l_dist = " << min_v2l_dist << endl;
   } // loop over road segments

   delete footprint_vertices_list_ptr;

   if (min_v2l_dist < max_bldg_to_road_dist)
   {
      b_ptr->set_front_dir(0,f_hat);
   }

//   cout << "building lies closest to roadsegment r= "
//        << closest_segment_ID << endl;
//   cout << "Building distance to road segment = " << min_v2l_dist << endl;
//   cout << "f_hat = " << f_hat << endl << endl;
}

// ---------------------------------------------------------------------
// Member function estimate_corner_building_front_dirs takes in the
// integer ID n for some building which has previously been classified
// as residing on a street corner.  This method loops over all road
// segments for the current cityblock and computes their distances to
// midpoint vertices on the building's footprint.  It computes the
// direction vectors to the two closest road segments which differ in
// angle by some minimal threshold.  Those direction vectors are saved
// within the building's front_dir[0] and front_dir[1] member
// variables.

void cityblock::estimate_corner_building_front_dirs(
   int n,double max_bldg_to_road_dist)
{
   building* b_ptr=get_building_ptr(n);
   Linkedlist<threevector>* footprint_vertices_list_ptr=
      b_ptr->generate_footprint_vertices_list();

// Initialize front directions to zero:

   b_ptr->set_front_dir(0,Zero_vector);
   b_ptr->set_front_dir(1,Zero_vector);

   double min_v2l_dist=POSITIVEINFINITY;
   double second_min_v2l_dist=POSITIVEINFINITY;
   const double max_dotproduct=cos(45*PI/180);
   threevector f_hat,f2_hat;
   for (unsigned int r=0; r<block_roadsegment_ptrs->size(); r++)
   {
      roadsegment* road_ptr=(*block_roadsegment_ptrs)[r];
      linesegment l(road_ptr->get_segment());
//      cout << "roadsegment r = " << r << " linesegment = " << l << endl;

// Loop over all vertices within current building's footprint and
// compute distance from each to linesegment l:

      for (Mynode<threevector>* currnode_ptr=footprint_vertices_list_ptr->
              get_start_ptr(); currnode_ptr != NULL; currnode_ptr=
              currnode_ptr->get_nextptr())
      {
         threevector curr_vertex(currnode_ptr->get_data());
         threevector closest_pnt_on_l;
         double v2l_dist=l.point_to_line_segment_distance(
            curr_vertex,closest_pnt_on_l);
         threevector v2l=closest_pnt_on_l-curr_vertex;

         if (v2l_dist < min_v2l_dist)
         {
            threevector old_f_hat(f_hat);
            double old_min_v2l_dist=min_v2l_dist;
            
            f_hat=v2l.unitvector();
            min_v2l_dist=v2l_dist;
            double dotproduct=f_hat.dot(old_f_hat);

            if (dotproduct < max_dotproduct)
            {
               f2_hat=old_f_hat;
               second_min_v2l_dist=old_min_v2l_dist;
            }
         }

         double dotproduct=f_hat.dot(v2l.unitvector());
         if (v2l_dist > min_v2l_dist && v2l_dist < second_min_v2l_dist 
             && (dotproduct < max_dotproduct))
         {
            f2_hat=v2l.unitvector();
            second_min_v2l_dist=v2l_dist;
         }

//         cout << "v2l_dist = " << v2l_dist << endl;
//         cout << "min_v2l_dist = " << min_v2l_dist
//              << " second_min_v2l_dist = " << second_min_v2l_dist << endl;
//         cout << "f_hat = " << f_hat << " f2_hat = " << f2_hat << endl;
         
      } // loop over footprint vertices
   } // loop over road segments

   delete footprint_vertices_list_ptr;

   if (min_v2l_dist < max_bldg_to_road_dist)
   {
      b_ptr->set_front_dir(0,f_hat);
   }
   if (second_min_v2l_dist < max_bldg_to_road_dist)
   {
      b_ptr->set_front_dir(1,f2_hat);
   }

//   cout << "Building distance to road segment = " << min_v2l_dist << endl;
//   cout << "Final f_hat = " << f_hat << endl;
//   cout << "Final f2_hat = " << f2_hat << endl;
}

// ---------------------------------------------------------------------
// Member function propagate_building_front_dirs loops over all
// buildings within *block_buildings_network_ptr.  It searches for
// building with no well-defined front direction but which have 2 or 3
// neighbors.  For such a building n, we demand that the front
// directions be non-zero for at least 2 of its neighbors.  If n
// represents a tripole, we believe that the front direction must
// equal zero for the 3rd neighbor.  We sort the neighbors according
// to the magnitudes of their zeroth front direction vectors, and we
// keep the 2 neighbors with the two largest (i.e. non-zero) front
// direction vectors.  

// Since one or both of the neighbors may be corner buildings, we need
// to consider their first front direction vectors as well.  We
// compute the 4 possible dot products between the zeroth and first
// dotproducts of neighbor A and neighbor B.  Out of the 4
// possibilities, we keep the front direction pair which has the
// largest dot product.  We take their average as our initial guess
// for building n's front direction.  We then execute a call to
// improve_guessed_fhat() and set building n's front direction equal
// to the result only if the initial guess intercepts some road
// segment located relatively close by to n's position.

void cityblock::propagate_building_front_dirs()
{
   for (Mynode<int>* currnode_ptr=block_buildings_network_ptr->
           get_entries_list_ptr()->get_start_ptr();
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int n=currnode_ptr->get_data();

      Site<building*>* bsite_ptr=block_buildings_network_ptr->get_site_ptr(n);
      building* b_ptr=get_building_ptr(n);
      threevector f_hat(b_ptr->get_front_dir(0));

//      cout << "building n = " << n 
//           << " on corner = " << b_ptr->get_on_street_corner() 
//           << " island flag = " << b_ptr->get_is_street_island() << endl;
      
      const double TINY=1E-8;
      if (f_hat.magnitude() < TINY)
      {
         vector<int> neighbors=bsite_ptr->get_neighbors();

         const int n_neighbors=neighbors.size();
         if (n_neighbors >= 2)
         {
            building* q_bldg_ptr[n_neighbors];
            double front0_dir_mag[n_neighbors];
            for (int i=0; i<n_neighbors; i++)
            {
               q_bldg_ptr[i]=get_building_ptr(neighbors[i]);
               front0_dir_mag[i]=
                  (q_bldg_ptr[i]->get_front_dir(0)).magnitude();
            }
            
//  Sort 2 or more neighbors according to the magnitudes of their 0th
//  front direction vectors:
            
            Quicksort(front0_dir_mag,q_bldg_ptr,n_neighbors);

            threevector fhat_00=q_bldg_ptr[n_neighbors-1]->get_front_dir(0);
            threevector fhat_10=q_bldg_ptr[n_neighbors-2]->get_front_dir(0);

            if (fhat_00.magnitude() > TINY && fhat_10.magnitude() > TINY)
            {
               threevector fhat_01=q_bldg_ptr[n_neighbors-1]->get_front_dir(1);
               threevector fhat_11=q_bldg_ptr[n_neighbors-2]->get_front_dir(1);

               double dotproduct[4];
               dotproduct[0]=fhat_00.dot(fhat_10);
               dotproduct[1]=fhat_00.dot(fhat_11);
               dotproduct[2]=fhat_01.dot(fhat_10);
               dotproduct[3]=fhat_01.dot(fhat_11);
               pair<threevector,threevector>* p=new pair<threevector,threevector>[4];
               p[0]=pair<threevector,threevector>(fhat_00,fhat_10);
               p[1]=pair<threevector,threevector>(fhat_00,fhat_11);
               p[2]=pair<threevector,threevector>(fhat_01,fhat_10);
               p[3]=pair<threevector,threevector>(fhat_01,fhat_11);

               Quicksort(dotproduct,p,4);

//               for (int j=0; j<4; j++)
//               {
//                  cout << j << " dotproduct = " << dotproduct[j] << endl;
//               }
//               cout << "p[3].first = " << p[3].first << endl;
//               cout << "p[3].second = " << p[3].second << endl;
               
               f_hat=(p[3].first+p[3].second).unitvector();
               delete [] p;
            } // fhat_00 != 0 && fhat_10 != 0 conditional
         } // neighbors.size==2 conditional
         
         if (f_hat.magnitude() > TINY)
         {
            const double max_distance=30;	// meters
            if (improve_guessed_fhat(max_distance,b_ptr->get_posn(),f_hat))
            {
               b_ptr->set_front_dir(0,f_hat);
//               cout << "building n = " << n << " front dir set to "
//                    << f_hat << endl;
            }
         } // f_hat != 0 conditional
      } // f_hat==0 conditional
   } // loop over block buildings network
}

// ---------------------------------------------------------------------
// Member function improve_guessed_fhat checks whether a guessed
// building's front direction vector intercepts any cityblock road
// segment at some reasonable distance.  If so, this boolean method
// returns true and resets f_hat to equal the direction vector
// pointing from the building's input position b_posn towards the
// intercepted road segment.

bool cityblock::improve_guessed_fhat(
   double max_distance,const threevector& b_posn,threevector& f_hat)
{
   bool fhat_improved_flag=false;
   for (unsigned int r=0; r<block_roadsegment_ptrs->size(); r++)
   {
      roadsegment* road_ptr=(*block_roadsegment_ptrs)[r];
      linesegment l(road_ptr->get_segment());

      threevector intersection_pnt;
      if (l.direction_vector_intersects_segment(
         b_posn,f_hat,intersection_pnt))
      {
         double dist_to_segment=(intersection_pnt-b_posn).magnitude();
//         const double max_distance=60;	// meters
         if (dist_to_segment < max_distance)
         {
            threevector closest_pnt_on_l;
            l.point_to_line_segment_distance(
               b_posn,closest_pnt_on_l);
            f_hat=(closest_pnt_on_l-b_posn).unitvector();
            fhat_improved_flag=true;
         }
      } // f_hat intercepts l conditional
   } // loop over cityblock's road segments
   return fhat_improved_flag;
}

// ---------------------------------------------------------------------
// Member function eliminate_building_tripoles attempts to resolve
// "tripole" buildings which have 3 "nextdoor" neighbors.  Since a
// building (generally) can have no more than 2 "nextdoor" neighbors,
// one of the 3 links must actually connect the building with its
// backyard neighbor.  We compute the dotproducts between the 3 links
// with the forward direction vector for the building.  The link whose
// dotproduct is most negative is the one which connects the building
// to its neighbor most in the rear.  We delete this link from the
// cityblock's buildings' network.
   
void cityblock::eliminate_building_tripoles()
{
   vector<int> tripoles=block_buildings_network_ptr->generate_npole_list(3);
   for (unsigned int i=0; i<tripoles.size(); i++)
   {
      int r=tripoles[i];
      Site<building*>* bsite_ptr=block_buildings_network_ptr->get_site_ptr(r);
      building* b_ptr=bsite_ptr->get_data();
      threevector f_hat(b_ptr->get_front_dir(0));
      
      const double TINY=1E-8;
      if (f_hat.magnitude() > TINY)
      {
         vector<int> tripole_neighbors=bsite_ptr->get_neighbors();
         vector<threevector> neighbor_displacements=
            block_buildings_network_ptr->rel_neighbor_displacements(r);

         vector<pair<double,int> > neighbor_dotproducts;
         for (unsigned int j=0; j<neighbor_displacements.size(); j++)
         {
            threevector n_hat(neighbor_displacements[j].unitvector());
            double dotproduct=(f_hat.dot(n_hat));
            neighbor_dotproducts.push_back(pair<double,int>(
               dotproduct,tripole_neighbors[j]));
         }

         std::sort(neighbor_dotproducts.begin(),neighbor_dotproducts.end());

//         cout << "Tripole r = " << r << endl;
//         cout << "Tripole neighbors = " << endl;
//         templatefunc::printVector(tripole_neighbors);
//         cout << "Neighbor dotproducts:" << endl;
//         for (int j=0; j<neighbor_dotproducts.size(); j++)
//         {
//            cout << "q = " << neighbor_dotproducts[j].second
//                 << " dotproduct = " 
//                 << neighbor_dotproducts[j].first << endl;
//         }

         block_buildings_network_ptr->
            delete_symmetric_link(r,neighbor_dotproducts[0].second);
      } // Tripole building's front direction is defined conditional
   } // loop over monopoles STL vector
}

// ---------------------------------------------------------------------
// Member function eliminate_inconsistent_nextdoor_links loops over
// all buildings within the cityblock.  For those buildings whose
// front directions f_hat have been defined, this method computes the
// angle between f_hat and the direction vectors to all "next-door"
// neighbor buildings.  It severs any next-door link for which the
// angle exceeds some very large threshold value.
   
void cityblock::eliminate_inconsistent_nextdoor_links()
{
   for (Mynode<int>* currnode_ptr=block_buildings_network_ptr->
           get_entries_list_ptr()->get_start_ptr();
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int n=currnode_ptr->get_data();
      Site<building*>* bsite_ptr=block_buildings_network_ptr->get_site_ptr(n);
      building* b_ptr=get_building_ptr(n);
      threevector f_hat(b_ptr->get_front_dir(0));

//      cout << "n = " << n << endl;
//      cout << "f_hat = " << f_hat << endl;

      const double TINY=1E-8;
      const double min_dotproduct=cos(150.0*PI/180.0);
      if (f_hat.magnitude() > TINY && !b_ptr->get_on_street_corner())
      {
         vector<int> neighbors=bsite_ptr->get_neighbors();
//         cout << "neighbors = " << endl;
//         templatefunc::printVector(neighbors);
         
         vector<threevector> neighbor_displacements=
            block_buildings_network_ptr->rel_neighbor_displacements(n);
         for (unsigned int i=0; i<neighbors.size(); i++)
         {
            threevector n_hat(neighbor_displacements[i].unitvector());
            double dotproduct=f_hat.dot(n_hat);
//            cout << "n_hat = " << n_hat << " dotproduct = " << dotproduct
//                 << endl;
            
            if (dotproduct < min_dotproduct)
            {
               int q=neighbors[i];
               block_buildings_network_ptr->delete_symmetric_link(n,q);
//               cout << "Deleting link between bldg n = " << n
//                    << " and q = " << q << endl;
            }
         } // loop over neighbors to building n
      } // Building's front direction is defined conditional
   } // loop over buildings within cityblock
}

// ---------------------------------------------------------------------
// Member function initialize_building_neighbor_handedness assigns
// right and left handed attributions to buildings with well-defined
// front directions and with precisely 2 next-door neighbors.  When a
// building is viewed from the street, the direction toward its RHS
// neighbor approximately equals r_hat = z_hat x f_hat.  So we use the
// dotproduct between r_hat and the direction vectors to the two
// neighbors to determine which lies on the right.

void cityblock::initialize_building_neighbor_handedness()
{
   for (Mynode<int>* currnode_ptr=block_buildings_network_ptr->
           get_entries_list_ptr()->get_start_ptr();
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int n=currnode_ptr->get_data();
      Site<building*>* bsite_ptr=block_buildings_network_ptr->get_site_ptr(n);
      building* b_ptr=get_building_ptr(n);
      threevector f_hat(b_ptr->get_front_dir(0));

//      cout << "Bldg n = " << n << endl;
//      cout << "f_hat = " << f_hat << endl;

      const double TINY=1E-8;
      if (b_ptr->get_on_street() && f_hat.magnitude() > TINY)
      {
         const threevector r_hat(z_hat.cross(f_hat));
         vector<int> neighbors=bsite_ptr->get_neighbors();
         if (neighbors.size()==2)
         {
            vector<threevector> neighbor_displacements=
               block_buildings_network_ptr->rel_neighbor_displacements(n);

            double dotproduct[2];
            for (unsigned int i=0; i<neighbors.size(); i++)
            {
               threevector n_hat(neighbor_displacements[i].unitvector());
               dotproduct[i]=r_hat.dot(n_hat);
//               cout << i << " n_hat = " << n_hat 
//                    << " dotproduct = " << dotproduct[i] << endl;
            } // loop over neighbors
            if (dotproduct[0] > dotproduct[1])
            {
               bsite_ptr->set_RHS_neighbor(neighbors[0]);
               bsite_ptr->set_LHS_neighbor(neighbors[1]);
            }
            else
            {
               bsite_ptr->set_RHS_neighbor(neighbors[1]);
               bsite_ptr->set_LHS_neighbor(neighbors[0]);
            }
//            cout << "RHS neighbor = " << bsite_ptr->get_RHS_neighbor() 
//                 << endl;
//            cout << "LHS neighbor = " << bsite_ptr->get_LHS_neighbor() 
//                 << endl;
            
         } // n_neighbors==2 conditional
      } // f_hat defined conditional
   } // loop over cityblock buildings
}

// ---------------------------------------------------------------------
// Member function propagate_building_neighbor_handedness loops over
// all buildings within the cityblock's *block_buildings_network_ptr.
// It implements trivial consistency requirements for buildings with
// known neighbor handed to constrain its own neighbor handedness
// assignments.  This method iteratively loops over itself until no
// further changes in any building's neighbor handedness can be made.

void cityblock::propagate_building_neighbor_handedness()
{
   bool some_bldg_handedness_changed;
   int while_loop_counter=0;
   do
   {
      some_bldg_handedness_changed=false;
      for (Mynode<int>* currnode_ptr=block_buildings_network_ptr->
              get_entries_list_ptr()->get_start_ptr();
           currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
      {
         int n=currnode_ptr->get_data();
         Site<building*>* bsite_ptr=block_buildings_network_ptr->
            get_site_ptr(n);
         building* b_ptr=get_building_ptr(n);
//         cout << "Bldg n = " << n << endl;

         if (b_ptr->get_on_street())
         {
            vector<int> neighbors=bsite_ptr->get_neighbors();

// If building n+1 is building n's RHS neighbor, then n is (n+1)'s LHS
// neighbor.  We implement this constraint plus its 3 permutations in
// the loop below:

            for (unsigned int j=0; j<neighbors.size(); j++)
            {
               if (neighbors[j]==bsite_ptr->get_RHS_neighbor())
               {
                  Site<building*>* rsite_ptr=block_buildings_network_ptr->
                     get_site_ptr(neighbors[j]);
                  if (rsite_ptr->get_LHS_neighbor()==-1)
                  {
                     rsite_ptr->set_LHS_neighbor(n);
                     some_bldg_handedness_changed=true;
                  }
               }
               if (neighbors[j]==bsite_ptr->get_LHS_neighbor())
               {
                  Site<building*>* lsite_ptr=block_buildings_network_ptr->
                     get_site_ptr(neighbors[j]);
                  if (lsite_ptr->get_RHS_neighbor()==-1)
                  {
                     lsite_ptr->set_RHS_neighbor(n);
                     some_bldg_handedness_changed=true;
                  }
               }
            } // loop over index j labeling neighbors to building n
          
// If building n has 2 neighbors and one is known to be its RHS
// neighbor, the other must be its LHS neighbor.  We implement this
// constraint (plus its 3 permutations) below:

            if (neighbors.size()==2)
            {
               if (bsite_ptr->get_RHS_neighbor()==neighbors[0])
               {
                  if (bsite_ptr->get_LHS_neighbor()==-1)
                  {
                     bsite_ptr->set_LHS_neighbor(neighbors[1]);
                     some_bldg_handedness_changed=true;
                  }
               }
               if (bsite_ptr->get_RHS_neighbor()==neighbors[1])
               {
                  if (bsite_ptr->get_LHS_neighbor()==-1)
                  {
                     bsite_ptr->set_LHS_neighbor(neighbors[0]);
                     some_bldg_handedness_changed=true;
                  }
               }

               if (bsite_ptr->get_LHS_neighbor()==neighbors[0])
               {
                  if (bsite_ptr->get_RHS_neighbor()==-1)
                  {
                     bsite_ptr->set_RHS_neighbor(neighbors[1]);
                     some_bldg_handedness_changed=true;
                  }
               }
               if (bsite_ptr->get_LHS_neighbor()==neighbors[1])
               {
                  if (bsite_ptr->get_RHS_neighbor()==-1)
                  {
                     bsite_ptr->set_RHS_neighbor(neighbors[0]);
                     some_bldg_handedness_changed=true;
                  }
               }

            } // n_neighbors==2 conditional

//            cout << "RHS neighbor = " << bsite_ptr->get_RHS_neighbor() 
//                 << endl;
//            cout << "LHS neighbor = " << bsite_ptr->get_LHS_neighbor() 
//                 << endl;

         } // building n on street conditional
      } // loop over cityblock buildings
      while_loop_counter++;
//      cout << "while_loop_counter = " << while_loop_counter << endl;
   }
   while (some_bldg_handedness_changed);
}

// ---------------------------------------------------------------------
// Member function infer_more_building_front_dirs uses available
// relative displacement vectors to RHS & LHS neighbors to infer
// buildings' missing front directions.  If the infered front
// direction intercepts a nearby cityblock road segment, the direction
// vector pointing from the building's position to the road segment is
// taken as the best estimate for the building's true front direction.

void cityblock::infer_more_building_front_dirs()
{
   for (Mynode<int>* currnode_ptr=block_buildings_network_ptr->
           get_entries_list_ptr()->get_start_ptr();
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int n=currnode_ptr->get_data();
      Site<building*>* bsite_ptr=block_buildings_network_ptr->
         get_site_ptr(n);
      building* b_ptr=get_building_ptr(n);
      threevector b_posn(b_ptr->get_posn());
      threevector f_hat(b_ptr->get_front_dir(0));

      const double TINY=1E-8;
      if (b_ptr->get_on_street() && f_hat.magnitude() < TINY)
      {
         vector<int> neighbors=bsite_ptr->get_neighbors();
         int r_neighbor=bsite_ptr->get_RHS_neighbor();
         int l_neighbor=bsite_ptr->get_LHS_neighbor();
         if (r_neighbor > -1 && l_neighbor > -1)
         {
            vector<threevector> neighbor_displacements=
               block_buildings_network_ptr->rel_neighbor_displacements(n);
            threevector r_hat=
               (neighbor_displacements[0]-neighbor_displacements[1]).
               unitvector();
            if (r_neighbor==neighbors[1]) r_hat=-r_hat;
            threevector f_hat=r_hat.cross(z_hat);

            const double max_distance=40;	// meters
            improve_guessed_fhat(max_distance,b_posn,f_hat);
            b_ptr->set_front_dir(0,f_hat);
         } // RHS & LHS neighbors to building n are known conditional
      } // building n on street & f_hat==0 conditional
   } // loop over cityblock buildings
}

// ---------------------------------------------------------------------
// Member function improve_corner_building_identification performs a
// series of tests on buildings which were not previously declared to
// be located at street corners.  We wrote this specialized method in
// an attempt to identify genuine corner buildings which were
// previously missed by
// cityblockfunc::identify_street_corner_buildings().  This method
// heavily uses neighbor position and front direction information to
// spot buildings which look like corners.

void cityblock::improve_corner_building_identification()
{
   for (Mynode<int>* currnode_ptr=block_buildings_network_ptr->
           get_entries_list_ptr()->get_start_ptr();
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int n=currnode_ptr->get_data();

      Site<building*>* bsite_ptr=block_buildings_network_ptr->
         get_site_ptr(n);
      building* b_ptr=get_building_ptr(n);

// FAKE FAKE:  Thurs Mar 17 at 6 07 am
//      b_ptr->set_on_street_corner(false);

      if (b_ptr->get_on_street() && !(b_ptr->get_on_street_corner()))
      {

// Require building n to lie within reasonable distance of some
// cityblock roadpoint:

         bool b_near_roadpoint=false;
         Linkedlist<threevector>* footprint_vertices_list_ptr=
            b_ptr->generate_footprint_vertices_list();
         for (unsigned int r=0; r<block_roadsegment_ptrs->size(); r++)
         {
            roadsegment* road_ptr=(*block_roadsegment_ptrs)[r];
            linesegment l(road_ptr->get_segment());

            const double max_distance=60;	// meters
            for (Mynode<threevector>* currnode_ptr=footprint_vertices_list_ptr->
                    get_start_ptr(); currnode_ptr != NULL; currnode_ptr=
                    currnode_ptr->get_nextptr())
            {
               threevector curr_vertex(currnode_ptr->get_data());
               double dist_to_roadpoint=(curr_vertex-l.get_v1()).magnitude();
               if (dist_to_roadpoint < max_distance)
               {
                  b_near_roadpoint=true;
               }
            } // loop over footprint vertices
         } // loop over index r labeling cityblock road segments
         delete footprint_vertices_list_ptr;
         
         if (b_near_roadpoint)
         {
            
// Building n must be a dipole:

            vector<int> neighbors=bsite_ptr->get_neighbors();
            if (neighbors.size()==2)
            {
               int q0=neighbors[0];
               int q1=neighbors[1];
         
//               cout << "q0 = " << q0 << " q1 = " << q1 << endl;
               
// Angle between building n's links to its two neighbors exceeds some
// large threshold:

               vector<threevector> neighbor_displacements=
                  block_buildings_network_ptr->rel_neighbor_displacements(n);
               threevector nhat_1(neighbor_displacements[0].unitvector());
               threevector nhat_2(neighbor_displacements[1].unitvector());
               double dotproduct=-nhat_1.dot(nhat_2);
               const double max_dotproduct=cos(75*PI/180);

//               cout << "dotproduct = " << dotproduct 
//                    << " max_dotproduct = " << max_dotproduct << endl;
               
               if (dotproduct < max_dotproduct)
               {

// Building n's neighbors must both have at fhat[0] defined:

                  building* q0_bldg_ptr=get_building_ptr(q0);
                  building* q1_bldg_ptr=get_building_ptr(q1);
                  threevector fhat_00=q0_bldg_ptr->get_front_dir(0);
                  threevector fhat_10=q1_bldg_ptr->get_front_dir(0);
                  const double TINY=1E-6;
                  if (fhat_00.magnitude() > TINY && fhat_10.magnitude() 
                      > TINY)
                  {
                     threevector fhat_01=q0_bldg_ptr->get_front_dir(1);
                     threevector fhat_11=q1_bldg_ptr->get_front_dir(1);

                     if (fhat_01.magnitude() < TINY) fhat_01=fhat_00;
                     if (fhat_11.magnitude() < TINY) fhat_11=fhat_10;

                     double dotproduct[4];
                     dotproduct[0]=fhat_00.dot(fhat_10);
                     dotproduct[1]=fhat_00.dot(fhat_11);
                     dotproduct[2]=fhat_01.dot(fhat_10);
                     dotproduct[3]=fhat_01.dot(fhat_11);
                     pair<threevector,threevector>* p=
                        new pair<threevector,threevector>[4];

                     p[0]=pair<threevector,threevector>(fhat_00,fhat_10);
                     p[1]=pair<threevector,threevector>(fhat_00,fhat_11);
                     p[2]=pair<threevector,threevector>(fhat_01,fhat_10);
                     p[3]=pair<threevector,threevector>(fhat_01,fhat_11);

                     Quicksort(dotproduct,p,4);

// Angle between neighbors' front direction vectors must exceed some
// minimal threshold:

                     const double max_fhat_dotproduct=cos(75*PI/180);

//                     cout << "dotproduct[0] = " << dotproduct[0] << endl;
//                     cout << "max_fhat_dotproduct = " 
//                          << max_fhat_dotproduct << endl;
                     
                     if (dotproduct[0] < max_fhat_dotproduct)
                     {
                        b_ptr->set_on_street_corner(true);
                        cout << endl;
                        cout << "Resetting building n = " << n 
                             << " to a corner building" << endl;
                        const double max_bldg_to_road_dist=50;	// meters
                        estimate_corner_building_front_dirs(
                           n,max_bldg_to_road_dist);
                     }
                  } // fhat_00 && fhat_10 != 0 conditional
               } // Angle between neighbor links conditional
            } // building n has 2 neighbors conditional
         } // b_near_roadpoint conditional
      } // building n on street & is not a street corner conditional
   } // loop over cityblock buildings
}

// ---------------------------------------------------------------------
// Member function break_remaining_building_tripoles implements
// somewhat ad hoc criteria for deleting one out of the three links
// between a tripole and its neighbors.  We created this method so
// that all street house comparisons with video could be reduced to
// strand searching.  Fortunately by the time this method is called,
// there should be very few tripoles remaining within the entire
// buildings network.

void cityblock::break_remaining_building_tripoles()
{
   vector<int> tripoles=block_buildings_network_ptr->
      generate_npole_list(3);

   for (unsigned int i=0; i<tripoles.size(); i++)
   {
      int n=tripoles[i];
      Site<building*>* bsite_ptr=block_buildings_network_ptr->
         get_site_ptr(n);

// If two links form nearly a perfect line, we assume the tripole
// corresponds to a "T" intersection.  In this case, we break the link
// between building n and the neighbor not lying along the line:

      vector<int> tripole_neighbors=bsite_ptr->get_neighbors();
      vector<threevector> neighbor_displacements=
         block_buildings_network_ptr->rel_neighbor_displacements(n);

      bool tripole_broken=false;
      for (int j=0; j<3; j++)
      {
         threevector n_hat(neighbor_displacements[j].unitvector());
         threevector next_n_hat(
            neighbor_displacements[modulo(j+1,3)].unitvector());
         double dotproduct=(n_hat.dot(next_n_hat));
         const double max_dotproduct=cos(170*PI/180);
         if (dotproduct < max_dotproduct)
         {
            int q=tripole_neighbors[modulo(j+2,3)];
            block_buildings_network_ptr->delete_symmetric_link(n,q);
            cout << "Breaking link between building n = " << n 
                 << " and tripole neighbor q = " << q << endl;
            tripole_broken=true;
         }
      } // loop over neighbor displacements

// If tripole still survives, eliminate link to neighbor that lies
// furthest away from cityblocks' road segments:
      
      if (!tripole_broken)
      {
         int q[3];
         double dist_to_roadside[3];
         for (int j=0; j<3; j++)
         {            
            q[j]=tripole_neighbors[j];
            dist_to_roadside[j]=POSITIVEINFINITY;
         }
         
         for (int j=0; j<3; j++)
         {
            for (unsigned int r=0; r<block_roadsegment_ptrs->size(); r++)
            {
               roadsegment* road_ptr=(*block_roadsegment_ptrs)[r];
               linesegment l(road_ptr->get_segment());
               double dist_to_l=l.point_to_line_segment_distance(
                  get_building_ptr(q[j])->get_posn());
               dist_to_roadside[j]=basic_math::min(
                  dist_to_roadside[j],dist_to_l);
            }
         } // loop over j labeling tripole neighbors

         Quicksort(dist_to_roadside,q,3);
         block_buildings_network_ptr->delete_symmetric_link(n,q[2]);
         cout << "Breaking link between building n = " << n 
              << " and tripole neighbor q = " << q[2] << endl;
      } // tripole unbroken conditional
   } // loop over index i labeling tripoles
}

// ---------------------------------------------------------------------
// Member function associate_buildings_with_roadsegments loops over
// each road segment within *block_roadsegment_ptrs.  It tests whether
// the front direction for building n in the cityblock intercepts road
// segment r.  If so, roadsegment r is stored within building n as a
// member variable.

void cityblock::associate_buildings_with_roadsegments()
{
   for (unsigned int r=0; r<block_roadsegment_ptrs->size(); r++)
   {
      roadsegment* road_ptr=(*block_roadsegment_ptrs)[r];
      linesegment l(road_ptr->get_segment());

      for (Mynode<int>* currnode_ptr=block_buildings_network_ptr->
              get_entries_list_ptr()->get_start_ptr();
           currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
      {
         int n=currnode_ptr->get_data();
         building* b_ptr=get_building_ptr(n);
         threevector b_posn(b_ptr->get_posn());

         threevector f_hat[2];
         f_hat[0]=b_ptr->get_front_dir(0);
         f_hat[1]=b_ptr->get_front_dir(1);

         if (l.direction_vector_intersects_segment(b_posn,f_hat[0]) ||
             l.direction_vector_intersects_segment(b_posn,f_hat[1]))
         {
            b_ptr->set_roadsegment_ID(r);
//            cout << "building n = " << n << " is associated with roadsegment"
//                 << " r = " << b_ptr->get_roadsegment_ID() << endl;
         }
      } // loop over buildings
   } // loop over index r labeling roadsegments

/*
   for (Mynode<int>* currnode_ptr=block_buildings_network_ptr->
           get_entries_list_ptr()->get_start_ptr();
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int n=currnode_ptr->get_data();
      building* b_ptr=get_building_ptr(n);
      pair<int,int> p(b_ptr->get_roadsegment_ID());
      cout << "building n = " << n 
           << "  is associated with roadsegment r0 = "
           << p.first;
      if (p.second != -1)
      {
         cout << " and roadsegment r1 = " << p.second;
      }
      cout << endl;
   }
*/
}

// ==========================================================================
// Strand construction & searching methods
// ==========================================================================

// Member function construct_building_neighbor_strands takes every
// cityblock building as a potential start point for a strand of
// adjacent street neighbors.  If input length number of RHS neighbors
// exist which all face the same street as the initial building, a
// strand of the buildings' integer labels is dynamically created.
// This strand is appended to member variable *strand_list_ptr.  The
// number of strands generated is returned by this method.

int cityblock::construct_building_neighbor_strands(int length)
{
   strand_list_ptr=new Linkedlist<Strand<building_info*>*>;

   for (Mynode<int>* currnode_ptr=block_buildings_network_ptr->
           get_entries_list_ptr()->get_start_ptr();
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int n=currnode_ptr->get_data();

// Dynamically generate a new strand whose ID equals that of its first
// building:

      Strand<building_info*>* curr_strand_ptr=new Strand<building_info*>(n);
      building* b_ptr=get_building_ptr(n);
      pair<int,int> r=b_ptr->get_roadsegment_ID();

      bool strand_OK=true;
      for (int counter=0; counter<length; counter++)
      {
         pair<int,int> p=get_building_ptr(n)->get_roadsegment_ID();
         if ( p.first==r.first || p.first==r.second || p.second==r.first 
              || (p.second==r.second && p.second != -1 ) )
         {
            building_info* curr_bldg_info_ptr=new building_info(n);
            curr_strand_ptr->append_node(curr_bldg_info_ptr);
         }
         else
         {
            strand_OK=false;
            break;
         }

// Retrieve current building's RHS neighbor:

         Site<building*>* bsite_ptr=block_buildings_network_ptr->
            get_site_ptr(n);
         n=bsite_ptr->get_RHS_neighbor();
         if (n==-1)
         {
            strand_OK=false;
            break;
         }
      } // loop over strand counter
      if (strand_OK) 
      {
//         cout << "curr strand = " << *curr_strand_ptr << endl;
         strand_list_ptr->append_node(curr_strand_ptr);
      }
      else
      {
         delete curr_strand_ptr;
      }

   } // loop over cityblock buildings
   int n_strands=strand_list_ptr->size();
   cout << "Cityblock contains " << n_strands << " strands" << endl;
   return n_strands;
}

// ---------------------------------------------------------------------
// Member function assign_strand_member_relative_heights loops over
// all strands within *strand_list_ptr.  For each strand, it compares
// the maximal heights of each of its adjacent buildings.  Buildings
// are assigned relative heights based upon a comparison with their
// LHS neighbors.

void cityblock::assign_strand_member_relative_heights()
{
   outputfunc::write_banner("Assigning strand member relative heights:");
   
   vector<double> bldg_height;
   for (Mynode<Strand<building_info*>* >* currnode_ptr=
           strand_list_ptr->get_start_ptr(); currnode_ptr != NULL; 
        currnode_ptr=currnode_ptr->get_nextptr())
   {
      Strand<building_info*>* curr_strand_ptr=currnode_ptr->get_data();
      
// Loop over buildings within current strand and retrieve their
// maximal heights:

      bldg_height.clear();
      for (Mynode<building_info*>* bldgnode_ptr=
              curr_strand_ptr->get_start_ptr(); bldgnode_ptr != NULL;
           bldgnode_ptr=bldgnode_ptr->get_nextptr())
      {
         building_info* curr_bldg_info_ptr=bldgnode_ptr->get_data();
         int n=curr_bldg_info_ptr->get_building_ID();
         building* b_ptr=get_building_ptr(n);
         bldg_height.push_back(b_ptr->find_max_height());
//         cout << "Bldg n = " << n 
//              << " Height = " << bldg_height.back() << endl;
      } // loop over buildings in current strand
      cout << endl;
      
// Loop over strand buildings again and compare their heights to those
// of their LHS neighbors.  Assign relative heights to each building
// in the strand based upon this comparison.  The first building's
// relative height remains unknown.

      const double delta_z_threshold=1.0;	 // meter
      int counter=1;
      for (Mynode<building_info*>* bldgnode_ptr=
              curr_strand_ptr->get_start_ptr()->get_nextptr(); 
           bldgnode_ptr != NULL; bldgnode_ptr=bldgnode_ptr->get_nextptr())
      {
         double delta_z=bldg_height[counter]-bldg_height[counter-1];
         building_info* curr_bldg_info_ptr=bldgnode_ptr->get_data();
         if (delta_z > delta_z_threshold)
         {
            curr_bldg_info_ptr->set_relative_height(
               building_info::greater_than);
         }
         else if (-delta_z > delta_z_threshold)
         {
            curr_bldg_info_ptr->set_relative_height(building_info::less_than);
         }
         else
         {
            curr_bldg_info_ptr->set_relative_height(building_info::equal_to);
         }
         counter++;
      } // loop over buildings in current strand
//      bldgstrandfunc::display_strand_contents(curr_strand_ptr);
   } // loop over strands in *strand_list_ptr
}

// ---------------------------------------------------------------------
// Member function assign_strand_member_spine_dirs loops over all
// strands within *strand_list_ptr.  

void cityblock::assign_strand_member_spine_dirs()
{
   outputfunc::write_banner("Assigning strand member spine directions:");
   
   for (Mynode<Strand<building_info*>* >* currnode_ptr=
           strand_list_ptr->get_start_ptr(); currnode_ptr != NULL; 
        currnode_ptr=currnode_ptr->get_nextptr())
   {
      Strand<building_info*>* curr_strand_ptr=currnode_ptr->get_data();
      
// Loop over buildings within current strand and retrieve their
// spine directions:

      for (Mynode<building_info*>* bldgnode_ptr=
              curr_strand_ptr->get_start_ptr(); bldgnode_ptr != NULL;
           bldgnode_ptr=bldgnode_ptr->get_nextptr())
      {
         building_info* curr_bldg_info_ptr=bldgnode_ptr->get_data();
         int n=curr_bldg_info_ptr->get_building_ID();
         building* b_ptr=get_building_ptr(n);

         linesegment* spine_ptr=b_ptr->get_tallest_spine_ptr();

// Recall spine's default direction is undefined:

         if (spine_ptr != NULL)
         {
            threevector f_hat;
            if (b_ptr->get_on_street_corner())
            {
// If *b_ptr lies on a street corner, it either begins or ends the
// current strand:
               
               Mynode<building_info*>* neighbor_node_ptr;
               if (bldgnode_ptr==curr_strand_ptr->get_start_ptr())
               {
                  neighbor_node_ptr=bldgnode_ptr->get_nextptr();
               }
               else
               {
                  neighbor_node_ptr=bldgnode_ptr->get_prevptr();
               }
               building_info* neighbor_bldg_info_ptr=
                  neighbor_node_ptr->get_data();
               int n_neighbor=neighbor_bldg_info_ptr->get_building_ID();
               building* neighboring_b_ptr=get_building_ptr(n_neighbor);
               f_hat=neighboring_b_ptr->get_front_dir(0);
            }
            else
            {
               f_hat=b_ptr->get_front_dir(0);
            } // building on street corner conditional
            
            double abs_dotproduct=fabs(f_hat.dot(spine_ptr->get_ehat()));
            double theta=acos(abs_dotproduct);
            if (theta < PI/4)
            {
               curr_bldg_info_ptr->set_spine_dir(
                  building_info::perpendicular);
            }
            else
            {
               curr_bldg_info_ptr->set_spine_dir(building_info::parallel);
            }
         } // spine_ptr != NULL conditional
      } // loop over buildings in current strand

      bldgstrandfunc::display_strand_contents(curr_strand_ptr);

   } // loop over strands in *strand_list_ptr
}

// ==========================================================================
// Drawing and annotation methods
// ==========================================================================

// Member function annotate_block_label generates a threeDstring
// corresponding to the current block object's ID label.  This method
// adds the threeDstring label above the block within the output xyzp
// file.

void cityblock::annotate_block_label(
   string xyzp_filename,double annotation_value)
{
   filefunc::gunzip_file_if_gzipped(xyzp_filename);
   threeDstring block_label(stringfunc::number_to_string(get_ID()));
   block_label.scale(block_label.get_origin().get_pnt(),3);
   threevector annotation_point(get_posn()+25*z_hat);
   block_label.center_upon_location(annotation_point);
   draw3Dfunc::draw_threeDstring(
      block_label,xyzp_filename,annotation_value);
//   filefunc::gzip_file(xyzp_filename);         
}

// ---------------------------------------------------------------------
// Member function annotate_block_segments loops over each roadsegment
// entry within *block_roadsegment_ptrs.  It generates a threeDstring
// for each roadsegment corresponding to its integer label.  This
// method adds the threeDstring label above the roadsegment's midpoint
// within the output xyzp file.

void cityblock::annotate_roadsegment_labels(
   string xyzp_filename,double annotation_value)
{
   filefunc::gunzip_file_if_gzipped(xyzp_filename);

   for (unsigned int r=0; r<block_roadsegment_ptrs->size(); r++)
   {
      roadsegment* curr_segment_ptr=(*block_roadsegment_ptrs)[r];
      threeDstring roadsegment_label(stringfunc::number_to_string(r));
//          roadsegment_label.scale(block_label.get_origin().get_pnt(),2);
      threevector annotation_point(
         curr_segment_ptr->get_posn()+10*z_hat);
      roadsegment_label.center_upon_location(annotation_point);
      draw3Dfunc::draw_threeDstring(
         roadsegment_label,xyzp_filename,annotation_value);
   } // loop over sites in buildings network 
}

// ---------------------------------------------------------------------
// Member function assign_strand_member_gross_vegation_dirs loops over all
// strands within *strand_list_ptr.

void cityblock::assign_strand_member_gross_vegetation_dirs(
   Linkedlist<int> const *bldgs_with_tall_trees_in_back_list_ptr,
   Linkedlist<int> const *bldgs_with_small_shrubs_on_rear_left_list_ptr)
{
   outputfunc::write_banner("Assigning strand member vegetation directions:");
   
   for (Mynode<Strand<building_info*>* >* currnode_ptr=
           strand_list_ptr->get_start_ptr(); currnode_ptr != NULL; 
        currnode_ptr=currnode_ptr->get_nextptr())
   {
      Strand<building_info*>* curr_strand_ptr=currnode_ptr->get_data();
      
// Loop over buildings within current strand and retrieve their gross
// tall tree and small shrub directions from input linked lists:

      for (Mynode<building_info*>* bldgnode_ptr=
              curr_strand_ptr->get_start_ptr(); bldgnode_ptr != NULL;
           bldgnode_ptr=bldgnode_ptr->get_nextptr())
      {
         building_info* curr_bldg_info_ptr=bldgnode_ptr->get_data();
         int n=curr_bldg_info_ptr->get_building_ID();

         if (bldgs_with_tall_trees_in_back_list_ptr->data_in_list(n) != NULL)
         {
            curr_bldg_info_ptr->set_tall_tree_posn(
               building_info::in_back);
         }
         if (bldgs_with_small_shrubs_on_rear_left_list_ptr->data_in_list(n) 
             != NULL)
         {
            curr_bldg_info_ptr->set_small_shrub_posn(
               building_info::on_left);
         }
      } // loop over buildings in current strand

      bldgstrandfunc::display_strand_contents(curr_strand_ptr);

   } // loop over strands in *strand_list_ptr
}
