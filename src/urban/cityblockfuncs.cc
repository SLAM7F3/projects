// ==========================================================================
// Cityblockfuncs namespace method definitions
// ==========================================================================
// Last modified on 7/29/06; 6/9/12; 7/3/12; 3/6/14
// ==========================================================================

#include <set>
#include <string>
#include <vector>
#include "math/basic_math.h"
#include "image/binaryimagefuncs.h"
#include "urban/bldgstrandfuncs.h"
#include "urban/building.h"
#include "urban/cityblock.h"
#include "urban/cityblockfuncs.h"
#include "geometry/contour.h"
#include "image/drawfuncs.h"
#include "threeDgraphics/draw3Dfuncs.h"
#include "ladar/featurefuncs.h"
#include "image/graphicsfuncs.h"
#include "image/imagefuncs.h"
#include "ladar/ladarfuncs.h"
#include "datastructures/Mynode.h"
#include "templates/mytemplates.h"
#include "general/outputfuncs.h"
#include "geometry/parallelogram.h"
#include "geometry/polygon.h"
#include "urban/roadfuncs.h"
#include "urban/roadpoint.h"
#include "general/stringfuncs.h"
#include "threeDgraphics/threeDstring.h"
#include "image/TwoDarray.h"
#include "urban/urbanfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::pair;
using std::string;
using std::vector;

namespace cityblockfunc
{
   Network<cityblock*>* cityblocks_network_ptr=NULL;

   int get_n_cityblocks()
      {
         return cityblocks_network_ptr->size();
      }

   cityblock* get_cityblock_ptr(int c)
      {
         return cityblockfunc::cityblocks_network_ptr->get_site_data_ptr(c);
      }

// ==========================================================================
// City block road methods:
// ==========================================================================

// Method generate_cityblocks_network first creates a binary image of
// the input road network plus a slightly shrunken data bounding box.
// It next dynamically generates contours corresponding to cityblock
// peripheries, and it instantiates the cityblocks network within
// cityblockfunc::cityblocks_network_ptr.  This method stores the
// contour information within dynamically generated cityblock objects,
// and then it inserts the cityblocks into *cityblocks_network_ptr.

   void generate_cityblocks_network(
      string imagedir,parallelogram* data_bbox_ptr,
      Network<roadpoint*>* roadpoints_network_ptr,
      twoDarray const *ztwoDarray_ptr,
      twoDarray* cityblock_regions_twoDarray_ptr)
      {
         outputfunc::write_banner("Generating cityblocks network:");

         twoDarray* mask_twoDarray_ptr=generate_road_network_mask(
            data_bbox_ptr,roadpoints_network_ptr,ztwoDarray_ptr);
//         string mask_filename=imagedir+"mask.xyzp";   
//         xyzpfunc::write_xyzp_data(
//            ztwoDarray_ptr,mask_twoDarray_ptr,mask_filename);

         vector<contour*>* cityblock_contour_ptr=generate_cityblock_contours(
            imagedir,data_bbox_ptr,ztwoDarray_ptr,mask_twoDarray_ptr,
            cityblock_regions_twoDarray_ptr);
         delete mask_twoDarray_ptr;

// Dynamically allocate cityblocks network:

         int n_blocks=cityblock_contour_ptr->size();
         cityblocks_network_ptr=new Network<cityblock*>(n_blocks);

         for (int c=0; c<n_blocks; c++)
         {
            threevector posn( (*cityblock_contour_ptr)[c]->vertex_average() );
            cityblock* new_cityblock_ptr=new cityblock(c,posn);
            new_cityblock_ptr->set_contour_ptr(
               (*cityblock_contour_ptr)[c]);
            cityblocks_network_ptr->insert_site(
               c,Site<cityblock*>(new_cityblock_ptr));
         } // loop over index c labeling cityblocks

      }

// ---------------------------------------------------------------------
// Member function generate_road_network_mask dynamically generates
// and returns a twoDarray containing a binary mask for the road
// network and the data bounding box.

   twoDarray* generate_road_network_mask(
      parallelogram const *data_bbox_ptr,
      Network<roadpoint*>* roadpoints_network_ptr,
      twoDarray const *ztwoDarray_ptr)
      {
         outputfunc::write_banner("Generating road network mask:");   

         twoDarray* mask_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);

         polygon shrunken_data_bbox(*data_bbox_ptr);
         shrunken_data_bbox.scale(0.99);
         drawfunc::color_convex_quadrilateral_exterior(
            shrunken_data_bbox,1,mask_twoDarray_ptr);
//         const double radius=1.0;	// meter
//         drawfunc::draw_thick_polygon(shrunken_data_bbox,1,radius,
//                                      mask_twoDarray_ptr);
         roadfunc::draw_road_network(
            roadpoints_network_ptr,mask_twoDarray_ptr,false);
         binaryimagefunc::binary_threshold(0.5,mask_twoDarray_ptr,0,1);
         return mask_twoDarray_ptr;
      }

// ---------------------------------------------------------------------
// Method generate_cityblock_contours uses computer graphics region
// filling techniques to segment an intersecting road network into
// distinct city block regions.  It takes in a binary image of the raw
// road network within input twoDarray
// *road_network_mask_twoDarray_ptr.  It returns a cityblock region
// map within output *cityblock_regions_twoDarray_ptr where all pixels
// corresponding to block n are colored n+1.  It also returns an STL
// vector of cityblock contours.

   vector<contour*>* generate_cityblock_contours(
      string imagedir,
      parallelogram* data_bbox_ptr,twoDarray const *ztwoDarray_ptr,
      twoDarray const *road_network_mask_twoDarray_ptr,
      twoDarray* cityblock_regions_twoDarray_ptr)
      {
         outputfunc::write_banner("Generating cityblock contours:");
         double xlo=road_network_mask_twoDarray_ptr->get_xlo();
         double xhi=road_network_mask_twoDarray_ptr->get_xhi();
         double ylo=road_network_mask_twoDarray_ptr->get_ylo();
         double yhi=road_network_mask_twoDarray_ptr->get_yhi();
         const double xstep=5;	// meters
         const double ystep=5;	// meters
         int mbins=basic_math::round((xhi-xlo)/xstep)+1;
         int nbins=basic_math::round((yhi-ylo)/ystep)+1;
         
         int n_cityblocks=0;
         vector<contour*>* cityblock_contour_ptr=new vector<contour*>;

         const double zfill=0.5;
         const double zboundary=1;
         const double znull=0;
         polygon reduced_data_bbox(*data_bbox_ptr);
         reduced_data_bbox.scale(0.95);
         
         for (int m=0; m<mbins; m++)
         {
            double x=xlo+m*xstep;
            for (int n=0; n<nbins; n++)
            {
               double y=ylo+n*ystep;
               threevector seed_point(x,y);
               if (reduced_data_bbox.point_inside_polygon(seed_point))
               {

// Check whether seed point already resides within an existing
// cityblock contour.  Also make sure that seed point does not lie
// very near or on a border between two different city blocks:

                  double cityblock_region_intensity_integral=
                     imagefunc::integrated_intensity_inside_bbox(
                        x-2,y-2,x+2,y+2,cityblock_regions_twoDarray_ptr);
                  double mask_region_intensity_integral=
                     imagefunc::integrated_intensity_inside_bbox(
                        x-2,y-2,x+2,y+2,road_network_mask_twoDarray_ptr);
                  if (nearly_equal(cityblock_region_intensity_integral,znull)
                      && nearly_equal(mask_region_intensity_integral,znull))
                  {
                     int npixels_filled=0;
                     twoDarray* fmask_twoDarray_ptr=
                        graphicsfunc::mask_boundaryFill(
                           zfill,znull,seed_point,
                           road_network_mask_twoDarray_ptr,npixels_filled);
                     double cityblock_area=npixels_filled*
                        ztwoDarray_ptr->get_deltax()*
                        ztwoDarray_ptr->get_deltay();

                     cout << "cityblock = " << n_cityblocks
                          << " npixels filled = " << npixels_filled
                          << " area = " << cityblock_area 
                          << endl;
                     cout << "cityblock char length = " 
                          << sqrt(cityblock_area) << " meters" << endl;

                     double znew_fill=n_cityblocks+1;
                     featurefunc::transfer_feature_pixels(
                        zfill,znew_fill,fmask_twoDarray_ptr,
                        cityblock_regions_twoDarray_ptr);
//                     string fmask_filename=imagedir+"fmask_"+
//                        stringfunc::number_to_string(n_cityblocks)+".xyzp";
//                     xyzpfunc::write_xyzp_data(
//                        ztwoDarray_ptr,fmask_twoDarray_ptr,fmask_filename,
//                        false,false);
                     delete fmask_twoDarray_ptr;

                     double delta_s=2.0;	// meter
                     contour* curr_contour_ptr=
                        graphicsfunc::contour_surrounding_enclosed_region(
                           znull,zboundary,delta_s,seed_point,
                           road_network_mask_twoDarray_ptr);
                     cityblock_contour_ptr->push_back(curr_contour_ptr);
                     n_cityblocks++;

                  } // seed inside cityblock conditional
               } // seed_point inside data bbox conditional
            } // loop over n index
         } // loop over m index
         return cityblock_contour_ptr;
      }

// ---------------------------------------------------------------------
// Method find_cityblock_road_segments loops over all cityblocks
// within *cityblocks_network_ptr and finds their associated road
// segments.

   void find_cityblock_road_segments(
      Network<roadpoint*> const *roadpoints_network_ptr)
      {
         for (Mynode<int>* currnode_ptr=cityblocks_network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int c=currnode_ptr->get_data(); // cityblock number
            get_cityblock_ptr(c)->find_road_segments(roadpoints_network_ptr);
         } // loop over sites within *cityblocks_network_ptr
      }

// ---------------------------------------------------------------------
// Method identify_cityblocks_near_roadpoints loops over all
// roadpoints r within input *roadpoints_network_ptr.  It finds all of
// r's neighboring roadpoints q.  For each q, this method computes the
// angle theta between r and q.  It then computes the location of a
// sample point located 10 meters away from r at angle theta+15 degs.
// This method then looks up the cityblock ID at that sample point
// from input twoDarray *cityblock_regions_twoDarray_ptr.  Nearby
// cityblock ID, starting angle and angular extent information are
// saved within the adjacent cityblock STL vector member of each
// roadpoint.

   void identify_cityblocks_near_roadpoints(
      twoDarray const *cityblock_regions_twoDarray_ptr,
      Network<roadpoint*>* roadpoints_network_ptr)
      {
         outputfunc::write_banner("Identifying cityblocks near roadpoints:");
         for (Mynode<int>* currnode_ptr=roadpoints_network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int r=currnode_ptr->get_data(); // roadpoint number
            roadpoint* curr_roadpoint_ptr=
               roadpoints_network_ptr->get_site_data_ptr(r);
            threevector r_posn(curr_roadpoint_ptr->get_posn());

            vector<int> neighbors=roadpoints_network_ptr->get_site_ptr(r)->
               get_neighbors();

            const double distance=10;	// meters
            const double dtheta=15*PI/180;  // rads
//            cout << endl;
//            cout << "Roadpoint r = " << r << endl;
            for (unsigned int i=0; i<neighbors.size(); i++)
            {
               int q=neighbors[i];
               int p=neighbors[modulo(i+1,neighbors.size())];
               threevector q_posn(roadpoints_network_ptr->get_site_data_ptr(q)->
                               get_posn());
               threevector p_posn(roadpoints_network_ptr->get_site_data_ptr(p)->
                               get_posn());
               linesegment l(r_posn,q_posn);
               linesegment l2(r_posn,p_posn);
               double theta=atan2(l.get_ehat().get(1),l.get_ehat().get(0));
               double theta2=mathfunc::myatan2(
                  l2.get_ehat().get(1),l2.get_ehat().get(0),theta);
               if (theta2 < theta) theta2 += 2*PI;

               threevector f_hat(cos(theta+dtheta),sin(theta+dtheta));
               threevector pnt_in_block(r_posn+distance*f_hat);

               unsigned int px,py;
               bool pnt_inside=cityblock_regions_twoDarray_ptr->
                  point_to_pixel(pnt_in_block,px,py);
               if (pnt_inside)
               {
                  int c=basic_math::round(cityblock_regions_twoDarray_ptr->
                                        get(px,py))-1;
                  if (c >= 0) 
                     curr_roadpoint_ptr->get_adjacent_cityblock().
                        push_back(Triple<int,double,double>(
                           c,theta,theta2-theta));
//                  cout << " q = " << q << " p = " << p << endl;
//                  cout << " theta = " << theta*180/PI 
//                       << " theta2 = " << theta2*180/PI 
//                       << " theta2-theta = " << (theta2-theta)*180/PI << endl;
//                  cout << " block_ID = " << c << endl << endl;
               } // pnt_inside conditional
            } // loop over index i labeling neighbors
         } // loop over roadpoints network
      }

// ==========================================================================
// City block building methods:
// ==========================================================================

// Method count_buildings_in_cityblocks returns a dynamically
// allocated integer array which contains the numbers of buildings
// within each city block:

   void count_buildings_in_cityblocks(
      Network<building*>* buildings_network_ptr,
      twoDarray const *cityblock_regions_twoDarray_ptr)
      {
         outputfunc::write_banner("Counting buildings within cityblocks:");

// First reset number of buildings in each city block to 0:

         for (int c=0; c<get_n_cityblocks(); c++)
         {
            get_cityblock_ptr(c)->set_n_buildings(0);
         }

         for (const Mynode<int>* currnode_ptr=buildings_network_ptr->
                 get_entries_list_ptr()->get_start_ptr();
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int n=currnode_ptr->get_data();
            building* curr_bldg_ptr=buildings_network_ptr->
               get_site_data_ptr(n);

            unsigned int px,py;
            cityblock_regions_twoDarray_ptr->point_to_pixel(
               curr_bldg_ptr->get_posn(),px,py);
            int c=basic_math::round(
               cityblock_regions_twoDarray_ptr->get(px,py))-1;

// We discovered in March 2005 that buildings lying very close to the
// data bounding box can sometimes be chopped in such a way that their
// positions lie outside the nontrivial region of
// *cityblock_regions_twoDarray_ptr.  In this case, we search over all
// of the rooftop pixels for the nth building until we find one which
// does overlap with *cityblock_regions_twoDarray_ptr:

            if (c==-1)
            {
//               cout << "posn = " << curr_bldg_ptr->get_posn() << endl;
//               cout << "px = " << px << " py = " << py << endl;
               for (mynode* currnode_ptr=curr_bldg_ptr->get_pixel_list_ptr()->
                       get_start_ptr(); currnode_ptr != NULL && c==-1; 
                    currnode_ptr=currnode_ptr->get_nextptr())
               {
                  int px_new=basic_math::round(
                     currnode_ptr->get_data().get_var(0));
                  int py_new=basic_math::round(
                     currnode_ptr->get_data().get_var(1));
                  c=basic_math::round(
                     cityblock_regions_twoDarray_ptr->get(px_new,py_new))-1;
//                  cout << "px_new = " << px_new << " py_new = " << py_new
//                       << " c_new = " << c << endl;
               } // loop over nodes in pixel list
            }
            get_cityblock_ptr(c)->set_n_buildings(
               get_cityblock_ptr(c)->get_n_buildings()+1);
//               cout << "building n= " << n << " resides in block " 
//                    << curr_bldg_ptr->get_cityblock_ID() << endl;
         } // loop over sites within buildings network
      }

// ---------------------------------------------------------------------
// Method set_building_cityblock_IDs scans over every building within
// input *buildings_network_ptr.  It sets the cityblock_ID for each
// building equal to one less than the pixel value at the building's
// position within input *cityblock_regions_twoDarray_ptr.  This
// regions map is assumed to have first been computed via a call to
// the preceding generate_cityblock_contours() method.

   void set_building_cityblock_IDs(
      Network<building*>* buildings_network_ptr,
      twoDarray const *cityblock_regions_twoDarray_ptr)
      {
         outputfunc::write_banner("Setting building cityblock IDs:");

         for (Mynode<int>* currnode_ptr=buildings_network_ptr->
                 get_entries_list_ptr()->get_start_ptr();
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int n=currnode_ptr->get_data();
            building* curr_bldg_ptr=buildings_network_ptr->
               get_site_data_ptr(n);
            unsigned int px,py;
            cityblock_regions_twoDarray_ptr->point_to_pixel(
               curr_bldg_ptr->get_posn(),px,py);
            int c=basic_math::round(
               cityblock_regions_twoDarray_ptr->get(px,py))-1;
            curr_bldg_ptr->set_cityblock_ID(c);
//            cout << "building n= " << n << " resides in block " 
//                 << curr_bldg_ptr->get_cityblock_ID() << endl;
         } // loop over sites within buildings network
      }

// ---------------------------------------------------------------------
// Method identify_buildings_islands identifies building "islands"
// which are completely surrounded by streets and which have no
// neighbors within their own city block.

   void identify_building_islands(
      Network<building*>* buildings_network_ptr,
      twoDarray const *cityblock_regions_twoDarray_ptr)
      {
         outputfunc::write_banner("Identifying building islands:");

         count_buildings_in_cityblocks(
            buildings_network_ptr,cityblock_regions_twoDarray_ptr);
         for (int c=0; c<get_n_cityblocks(); c++)
         {
            if (get_cityblock_ptr(c)->get_n_buildings()==1)
            {
               for (Mynode<int>* currnode_ptr=buildings_network_ptr->
                       get_entries_list_ptr()->get_start_ptr();
                    currnode_ptr != NULL; currnode_ptr=currnode_ptr->
                       get_nextptr())
               {
                  int n=currnode_ptr->get_data();
                  building* curr_bldg_ptr=buildings_network_ptr->
                     get_site_data_ptr(n);
                  if (curr_bldg_ptr->get_cityblock_ID()==c)
                  {
                     curr_bldg_ptr->set_is_street_island(true);
                  }
               } // loop over sites in *buildings_network_ptr
            } // nbuildings within block==1 conditional
         } // loop over city blocks
      }

// ---------------------------------------------------------------------
// Method eliminate_blocks_with_no_buildings searches for blocks
// containing zero buildings within their borders.  It subsequently
// obtains a list of all roadpoints located along the contour of such
// empty cityblocks.  This method then deletes all such roadpoints
// which are not intersections from the input *roadpoints_network_ptr.

   void eliminate_blocks_with_no_buildings(
      Network<building*>* buildings_network_ptr,
      Network<roadpoint*>* roadpoints_network_ptr,
      twoDarray const *cityblock_regions_twoDarray_ptr)
      {
         outputfunc::write_banner("Identifying blocks with no buildings:");

         count_buildings_in_cityblocks(
            buildings_network_ptr,cityblock_regions_twoDarray_ptr);

         vector<int> blocks_to_delete;
         for (int c=0; c<get_n_cityblocks(); c++)
         {
            cityblock* curr_cityblock_ptr=get_cityblock_ptr(c);
            if (get_cityblock_ptr(c)->get_n_buildings()==0)
            {
               blocks_to_delete.push_back(c);
               cout << "Block " << c << " contains no buildings!" << endl;

               const double max_distance=10;	// meters
               vector<int> roadpoints_near_cityblock=
                  curr_cityblock_ptr->identify_nearby_roadpoints(
                     max_distance,roadpoints_network_ptr);

// As of 1/20/05, we implement a very crude means for eliminating
// cityblocks containing no buildings.  We simply delete from the
// roadpoints network every road point which is not an intersection.
// We assume that the intersections (there must be at least 2 of
// them!) surrounding the fictitious cityblock are already linked.  If
// not, we could simply add a check into the following lines which
// would add the link if it does not already exist...

               for (unsigned int i=0; i<roadpoints_near_cityblock.size(); i++)
               {
                  int r=roadpoints_near_cityblock[i];
                  roadpoint* curr_roadpoint_ptr=
                     roadpoints_network_ptr->get_site_data_ptr(r);
                  if (!curr_roadpoint_ptr->get_intersection())
                  {
                     roadpoints_network_ptr->delete_single_site(r);
                     cout << "Deleting roadpoint r = " << r << endl;
                  }
               } // loop over index i labeling roadpoints surrounding
               //  cityblock with no buildings
            }  // nbuildings within block==0 conditional
         } // loop over city blocks

// Finally, delete blocks from cityblocks_network_ptr:

         for (unsigned int i=0; i<blocks_to_delete.size(); i++)
         {
            int c=blocks_to_delete[i];
            cityblocks_network_ptr->delete_single_site(c);
         }
      }

// ---------------------------------------------------------------------
// Method generate_cityblock_building_network takes in a complete
// buildings network *buildings_network_ptr.  It first copies this
// full network onto a new sub-network.  This method then deletes
// every building within the sub-network whose cityblock ID does not
// equal that passed as an input parameter.  The dynamically generated
// sub-network is returned by this method.

   Network<building*>* generate_cityblock_building_network(
      int c,Network<building*> const *buildings_network_ptr)
      {
         Network<building*>* sub_network_ptr=new Network<building*>(
            *buildings_network_ptr);
         for (const Mynode<int>* currnode_ptr=buildings_network_ptr->
                 get_entries_list_ptr()->get_start_ptr();
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int r=currnode_ptr->get_data();
            building* curr_bldg_ptr=buildings_network_ptr->
               get_site_data_ptr(r);
            if (curr_bldg_ptr->get_cityblock_ID() != c)
            {
               sub_network_ptr->delete_single_site(r);
            }
         } // loop over sites within buildings network
         if (sub_network_ptr->size() <= 0)
         {
            cout << "Danger in Network::generate_cityblock_building_network()"
                 << endl;
            cout << "sub_network_ptr nsites = " << sub_network_ptr->
               size() << endl;
         }
         return sub_network_ptr;
      }

// ---------------------------------------------------------------------
// Method prune_multiblock_links takes in building link information
// within input *buildings_network_ptr.  For each building within the
// network, this method checks whether its neighbors belong to the
// same city block.  If not, the link between the building and its
// neighbor is deleted within *buildings_network_ptr.

   void prune_multiblock_links(Network<building*>* buildings_network_ptr)
      {
         outputfunc::write_banner("Pruning multiblock links:");

         vector<int> links_to_delete;
         for (Mynode<int>* currnode_ptr=buildings_network_ptr->
                 get_entries_list_ptr()->get_start_ptr();
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int n=currnode_ptr->get_data();
            cout << n << " " << flush;

            building* curr_bldg_ptr=buildings_network_ptr->
               get_site_data_ptr(n);
            int c=curr_bldg_ptr->get_cityblock_ID();

            Site<building*>* curr_bldg_site_ptr=
               buildings_network_ptr->get_site_ptr(n);
            Linkedlist<netlink>* bldglink_list_ptr=
               curr_bldg_site_ptr->get_netlink_list_ptr();   
            
            if (bldglink_list_ptr != NULL)
            {
               links_to_delete.clear();
               
               for (Mynode<netlink>* linknode_ptr=bldglink_list_ptr->
                       get_start_ptr(); linknode_ptr != NULL; linknode_ptr=
                       linknode_ptr->get_nextptr())
               {
                  int m=linknode_ptr->get_data().get_ID();
                  building* neighbor_bldg_ptr=
                     buildings_network_ptr->get_site_data_ptr(m);
                  int neighbor_cityblock_ID=neighbor_bldg_ptr->
                     get_cityblock_ID();
                  if (neighbor_cityblock_ID != c)
                  {
                     links_to_delete.push_back(m);
                  }
               } // loop over building neighbors

               for (unsigned int i=0; i<links_to_delete.size(); i++)
               {
                  buildings_network_ptr->delete_symmetric_link(
                     n,links_to_delete[i]);
               }
               
            } // bldglink_list_ptr != NULL conditional
         } // loop over sites in buildings network
         cout << endl;
      }

// ---------------------------------------------------------------------
// Method identify_nextdoor_neighbors takes in some buildings network
// or sub-network.  It first looks for tadpoles within the network.
// If any are found, they provide seeds for an outer building loop
// search.  If network contains no tadpoles, this method takes as its
// starting seed the site which is located most towards the bottom
// (i.e. minimum y value) and possibly most towards the right (maximum
// x value).  Starting from the seed location which is guaranteed to
// be located on the outer periphery of the network, this method
// sequentially finds via the right-hand rule all neighboring nodes
// which are located along the network's outer perimeter.  It returns
// "next door neighbor" site IDs within a dynamically generated STL
// vector.

   vector<int>* identify_nextdoor_neighbors(
      const Network<building*>* buildings_network_ptr)
      {
         vector<int> zeropole=buildings_network_ptr->generate_npole_list(0);
         if (zeropole.size()==buildings_network_ptr->size())
         {
//            cout << "All buildings within input network have no neighbors!"
//                 << endl;
            return NULL;
         }

         vector<int> tadpole=buildings_network_ptr->generate_npole_list(1);
         int r_prev,r;
         if (tadpole.size() > 0)
         {
            r_prev=tadpole[0];
            const Linkedlist<netlink>* netlink_list_ptr=
               buildings_network_ptr->get_site_ptr(r_prev)->
               get_netlink_list_ptr();
            r=netlink_list_ptr->get_start_ptr()->get_data().get_ID();
//            cout << "Tadpole site = " << r_prev
//                 << " tadpole neighbor = " << r << endl;
         }
         else
         {
            r_prev=buildings_network_ptr->bottom_right_site();
            r=buildings_network_ptr->bottom_right_site_RHS_neighbor();
//            cout << "Bottom right site = " << r_prev << endl;
//            cout << "Bottom right neighbor = " << r << endl;
         }
         
         building* prev_bldg_ptr=buildings_network_ptr->
            get_site_data_ptr(r_prev);
         building* curr_bldg_ptr=buildings_network_ptr->
            get_site_data_ptr(r);
         threevector prev_posn(prev_bldg_ptr->get_posn());
         threevector curr_posn(curr_bldg_ptr->get_posn());
         threevector r_hat=(curr_posn-prev_posn).unitvector();
         
         vector<int>* right_hand_loop_ptr=buildings_network_ptr->
            right_hand_loop(r,r_hat,true);
         right_hand_loop_ptr->insert(right_hand_loop_ptr->begin(),r_prev);
         if (tadpole.size() > 0) right_hand_loop_ptr->push_back(r);

         for (unsigned int i=0; i<right_hand_loop_ptr->size(); i++)
         {
//            cout << i << " node = " << (*right_hand_loop_ptr)[i]
//                 << endl;
         }

         return right_hand_loop_ptr;
      }
   
// ---------------------------------------------------------------------
// Method generate_nextdoor_neighbor_network takes in some buildings
// sub-network.  It first identifies all buildings which are located
// along the outer periphery of the sub-network.  It dynamically
// creates and returns a new sub-network in which the only links are
// between next-door neighbors.  

   Network<building*>* generate_nextdoor_neighbor_network(
      int c,Network<building*> const *buildings_subnet_ptr)
      {
         vector<int>* right_hand_loop_ptr=identify_nextdoor_neighbors(
            buildings_subnet_ptr);
       
         Network<building*>* network_ptr=new Network<building*>(
            *buildings_subnet_ptr);

         if (right_hand_loop_ptr==NULL)
         {
//            return NULL;
         }
         else
         {
            network_ptr->delete_all_neighbor_links();
            for (unsigned int i=0; i<right_hand_loop_ptr->size()-1; i++)
            {
               int r_prev=(*right_hand_loop_ptr)[i];
               int r=(*right_hand_loop_ptr)[i+1];
               network_ptr->add_symmetric_link(r_prev,r);
            }

            delete right_hand_loop_ptr;

         } // right_hand_loop_ptr == NULL conditional
         return network_ptr;
      }
   
// ---------------------------------------------------------------------
// Method generate_cityblock_neighbors_network loops over all city
// blocks.  For each block, it constructs a network of "next-door"
// neighbor buildings.  It returns all of these networks within an STL
// vector.

   void generate_cityblock_neighbors_networks(
      const Network<building*>* buildings_network_ptr)
      {
         outputfunc::write_banner("Generating cityblock neighbors networks:");
         
         for (int c=0; c < get_n_cityblocks(); c++)
         {
            cout << c << " " << flush;
            Network<building*>* buildings_subnet_ptr=
               generate_cityblock_building_network(c,buildings_network_ptr);
            get_cityblock_ptr(c)->set_block_buildings_network_ptr(
               generate_nextdoor_neighbor_network(c,buildings_subnet_ptr));
            delete buildings_subnet_ptr;
         }
         outputfunc::newline();
      }

// ---------------------------------------------------------------------
// Method identify_building_tadpoles identifies isolated strands of
// next-door buildings that do not reside within a genuine loop.  It
// saves this information within the building_tadpoles STL vector
// members of each cityblock in *cityblocks_network_ptr.

   void identify_building_tadpoles()
      {
         outputfunc::write_banner("Identifying building tadpole strands:");
         for (int c=0; c<get_n_cityblocks(); c++)
         {
            cout << c << " " << flush;
            get_cityblock_ptr(c)->identify_building_tadpoles();
         } // loop over city blocks
         outputfunc::newline();
      }

// ---------------------------------------------------------------------
// Method eliminate_building_tripoles loops over all cityblocks and
// tries to eliminate one out of the 3 links for "tripoles" (buildings
// with 3 "next-door neighbors") using building front direction
// information.

   void eliminate_building_tripoles()
      {
         outputfunc::write_banner("Eliminating building tripoles:");
         for (int c=0; c<get_n_cityblocks(); c++)
         {
            cout << c << " " << flush;
            get_cityblock_ptr(c)->eliminate_building_tripoles();
         } // loop over city blocks
         outputfunc::newline();
      }

// ---------------------------------------------------------------------
// Method eliminate_inconsistent_nextdoor_links loops over all
// cityblocks and deletes all "nextdoor neighbor" links which make too
// large an angle with a building's front direction.

   void eliminate_inconsistent_nextdoor_links()
      {
         outputfunc::write_banner(
            "Eliminating inconsistent next door links:");
         for (int c=0; c<get_n_cityblocks(); c++)
         {
            cout << c << " " << flush;
            get_cityblock_ptr(c)->eliminate_inconsistent_nextdoor_links();
         } // loop over city blocks
         outputfunc::newline();
      }

// ---------------------------------------------------------------------
// Method copy_only_nextdoor_neighbor_links takes input
// *buildings_network_ptr which we assume has already had all of its
// links purged.  It transfers all next-door neighbor information
// encoded within each cityblock's block_buildings_network_ptr [which
// we assume has already been filled via a call to
// cityblockfunc::generate_cityblock_neighbors_networks() ] to
// buildings_network_ptr.

   void copy_only_nextdoor_neighbor_links(
      Network<building*>* buildings_network_ptr)
      {
         outputfunc::write_banner(
            "Copying nextdoor neighbor links to buildings network:");
         
         for (int c=0; c < get_n_cityblocks(); c++)
         {
            Network<building*>* neighbors_subnet_ptr=
               get_cityblock_ptr(c)->get_block_buildings_network_ptr();

            if (neighbors_subnet_ptr != NULL)
            {
               for (Mynode<int>* currnode_ptr=neighbors_subnet_ptr->
                       get_entries_list_ptr()->get_start_ptr(); currnode_ptr 
                       != NULL; currnode_ptr=currnode_ptr->get_nextptr())
               {
                  int n=currnode_ptr->get_data();
                  Site<building*>* curr_site_ptr=
                     neighbors_subnet_ptr->get_site_ptr(n);

                  Site<building*>* bldgs_site_ptr=
                     buildings_network_ptr->get_site_ptr(n);
                  bldgs_site_ptr->set_RHS_neighbor(
                     curr_site_ptr->get_RHS_neighbor());
                  bldgs_site_ptr->set_LHS_neighbor(
                     curr_site_ptr->get_LHS_neighbor());

                  Linkedlist<netlink>* curr_bldglink_list_ptr=
                     neighbors_subnet_ptr->get_site_ptr(n)->
                     get_netlink_list_ptr();

                  if (curr_bldglink_list_ptr != NULL)
                  {
                     for (Mynode<netlink>* curr_bldglink_node_ptr=
                             curr_bldglink_list_ptr->get_start_ptr();
                          curr_bldglink_node_ptr != NULL; 
                          curr_bldglink_node_ptr=curr_bldglink_node_ptr->
                             get_nextptr())
                     {
                        int q=curr_bldglink_node_ptr->get_data().get_ID();
                        buildings_network_ptr->add_symmetric_link(n,q);
                     } // loop over netlink nodes in *curr_bldglink_list_ptr
                  } // curr_bldglink_list_ptr != NULL conditional
               } // loop over nodes in *neighbors_subnet_ptr
            } // neighbors_subnet_ptr != NULL conditional
         } // loop over cityblock IDs
      }

// ---------------------------------------------------------------------
// Auxilliary method examine_buildings_network was concocted for
// algorithm debugging purposes only.

   void examine_buildings_network(Network<building*>* buildings_network_ptr)
      {
         outputfunc::write_banner("Examining buildings network:");
         
         for (Mynode<int>* currnode_ptr=buildings_network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); currnode_ptr 
                 != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int n=currnode_ptr->get_data();
            cout << "Enter n:" << endl;
            cin >> n;
            
            cout << "n = " << n << endl;
            Site<building*>* bldgs_site_ptr=
               buildings_network_ptr->get_site_ptr(n);
            int rhs=bldgs_site_ptr->get_RHS_neighbor();
            int lhs=bldgs_site_ptr->get_LHS_neighbor();
            cout << "rhs neighbor = " << rhs << " lhs neighbor = " << lhs
                 << endl;
            building* cityimage_bldg_ptr=bldgs_site_ptr->get_data();
            cout << "*cityimage_bldg_ptr = " << *cityimage_bldg_ptr
                 << endl;
            outputfunc::enter_continue_char();
         }
      }
   
// ==========================================================================
// Building-road relationship methods
// ==========================================================================

// Method identify_buildings_on_street scans over every building
// within the input *buildings_network_ptr.  It sets to false the
// on_street flag for any building which has no nontrivial netlink
// list.

   void identify_buildings_on_street()
      {
         outputfunc::write_banner("Identifying buildings located on street:");

         for (int c=0; c<get_n_cityblocks(); c++)
         {
            cout << c << " " << flush;
            get_cityblock_ptr(c)->identify_buildings_on_street();
         } // loop over city blocks
         outputfunc::newline();
      }

// ---------------------------------------------------------------------
// Method identify_intersections_near_building_islands searches over
// the buildings network for sites which correspond to "islands"
// surrounded by streets.  It examines the roadpoint neighbors for
// such "island" buildings.  Those roadpoint neighbors which represent
// street intersections are explicitly flagged by this method.

   void identify_intersections_near_building_islands(
      const Network<building*>* buildings_network_ptr)
      {
         outputfunc::write_banner(
            "Identifying intersections near building islands:");

         for (const Mynode<int>* currnode_ptr=buildings_network_ptr->
                 get_entries_list_ptr()->get_start_ptr();
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int n=currnode_ptr->get_data();
            building* curr_bldg_ptr=buildings_network_ptr->
               get_site_data_ptr(n);
            if (curr_bldg_ptr != NULL && curr_bldg_ptr->
                get_is_street_island())
            {
               const Linkedlist<int>* roadpoint_list_ptr=
                  curr_bldg_ptr->get_nearby_roadpoint_list_ptr();
               
               for (const Mynode<int>* roadnode_ptr=
                       roadpoint_list_ptr->get_start_ptr(); roadnode_ptr !=
                       NULL; roadnode_ptr=roadnode_ptr->get_nextptr())
               {
                  int r=roadnode_ptr->get_data();
                  roadpoint* roadpoint_ptr=
                     roadfunc::intersections_network_ptr->
                     get_site_data_ptr(r);
                  if (roadpoint_ptr != NULL && 
                      roadpoint_ptr->get_intersection())
                  {
                     roadpoint_ptr->set_near_bldg_island(true);
                  }
               } // loop over *roadpoint_list_ptr
            } // street island conditional
         } // loop over sites in *buildings_network_ptr
      }

// ---------------------------------------------------------------------
// Method identify_roadpoints_near_building_islands scans over all
// buildings within input *buildings_network_ptr.  For those buildings
// which represent "islands" surrounded by roadways, this method tags
// each element within the building's nearby roadpoint list.

   void identify_roadpoints_near_building_islands(
      const Network<building*>* buildings_network_ptr,
      Network<roadpoint*>* roadpoints_network_ptr)
      {
         outputfunc::write_banner(
            "Identifying roadpoints near building islands:");

         for (const Mynode<int>* currnode_ptr=buildings_network_ptr->
                 get_entries_list_ptr()->get_start_ptr();
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int n=currnode_ptr->get_data();
            building* curr_bldg_ptr=buildings_network_ptr->
               get_site_data_ptr(n);
            if (curr_bldg_ptr != NULL && curr_bldg_ptr->
                get_is_street_island())
            {
               const Linkedlist<int>* roadpoint_list_ptr=
                  curr_bldg_ptr->get_nearby_roadpoint_list_ptr();
               
               for (const Mynode<int>* roadnode_ptr=
                       roadpoint_list_ptr->get_start_ptr(); roadnode_ptr !=
                       NULL; roadnode_ptr=roadnode_ptr->get_nextptr())
               {
                  int r=roadnode_ptr->get_data();
                  roadpoint* roadpoint_ptr=
                     roadpoints_network_ptr->get_site_data_ptr(r);
                  if (roadpoint_ptr != NULL)
                  {
                     roadpoint_ptr->set_near_bldg_island(true);
                  }
               } // loop over *roadpoint_list_ptr
            } // street island conditional
         } // loop over sites in *buildings_network_ptr
      }

// ---------------------------------------------------------------------
// Method identify_street_corner_buildings loops over all nodes within
// input *roadpoints_network_ptr.  For each roadpoint with 2 or more
// neighbors, it retrieves adjacent cityblock and street corner
// angular extent information which we assume has already been
// calculated via a call to
// cityblockfunc::identify_cityblocks_near_roadpoints().  It loops
// over each cityblock wedge whose opening angle is not too large (in
// order to avoid misclassifying buildings at T-intersections as lying
// along street corners).  This method subsequently loops over all
// buildings within the cityblock corresponding to a particular wedge.
// It searches for the building lying closest to the intersection
// point.  If the distance to the closest building is less than some
// maximal threshold, this method declares that building to lie along
// a street corner.  Buildings which have previously been declared as
// "street islands" are NOT classified as "street corners".

   void identify_street_corner_buildings(
      const Network<roadpoint*>* roadpoints_network_ptr,
      Network<building*>* buildings_network_ptr)
      {
         outputfunc::write_banner("Identifying street corner buildings:");
         
         for (const Mynode<int>* currnode_ptr=roadpoints_network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int r=currnode_ptr->get_data(); // roadpoint number
            roadpoint* curr_roadpoint_ptr=
               roadpoints_network_ptr->get_site_data_ptr(r);

            if (curr_roadpoint_ptr->get_intersection() &&
                roadpoints_network_ptr->get_site_ptr(r)->get_n_neighbors()> 1)
            {
//               cout << endl;
//               cout << "roadpoint r = " << r << endl;
               threevector r_posn(curr_roadpoint_ptr->get_posn());
               vector<Triple<int,double,double> > T=curr_roadpoint_ptr->
                  get_adjacent_cityblock();

               for (unsigned int nblock=0; nblock < T.size(); nblock++)
               {
                  int c=T[nblock].first;
//                  double theta=T[nblock].second;
                  double dtheta=T[nblock].third;

// Don't try to define a street corner building if corner's angular
// extent is too large:

//                  const double max_theta=130*PI/180;
//                  const double max_theta=120*PI/180;
                  const double max_theta=115*PI/180;
                  if (dtheta < max_theta)
                  {
                     int corner_bldg_ID=-1;
                     double min_separation=POSITIVEINFINITY;
                     for (Mynode<int>* currnode_ptr=buildings_network_ptr->
                             get_entries_list_ptr()->get_start_ptr();
                          currnode_ptr != NULL; currnode_ptr=currnode_ptr->
                             get_nextptr())
                     {
                        int n=currnode_ptr->get_data();
                        building* curr_bldg_ptr=buildings_network_ptr->
                           get_site_data_ptr(n);
                        if (curr_bldg_ptr->get_cityblock_ID()==c)
                        {
                           threevector b_posn=curr_bldg_ptr->
                              find_closest_point(r_posn);
                           
// Don't define a street corner building if it lies too far from
// intersection roadpoint:

                           const double max_separation=60;  // meters
                           double curr_separation=(b_posn-r_posn).
                              magnitude();
                           if ( curr_separation < max_separation)
                           {
                              if (curr_separation < min_separation)
                              {
                                 min_separation=curr_separation;
                                 corner_bldg_ID=n;
                              }
                           } // bldg & intersection not too far apart 
                        } // correct block ID conditional
                     } // loop over all nodes in buildings network

// Don't define street islands to be corner buidings:

                     if (corner_bldg_ID > -1)
                     {
                        building* candidate_corner_bldg_ptr=
                           buildings_network_ptr->get_site_data_ptr(
                              corner_bldg_ID);

                        if (!candidate_corner_bldg_ptr->
                            get_is_street_island())
                           candidate_corner_bldg_ptr->set_on_street_corner(
                              true);
//                        cout << "Corner building ID = " << corner_bldg_ID
//                             << endl;
                     } // corner_building_ID > -1 conditional
                  } // dtheta < max_theta conditional
               } // loop over cityblocks surrounding intersection point
            } // roadpoint has > 2 neighbors conditional
         } // loop over nodes in roadpoints network
      }

// ---------------------------------------------------------------------
// Method compute_building_front_dirs scans over every building within
// each cityblock in *cityblocks_network_ptr.  It set the
// front_direction members of each building equal to the direction
// pointing from the building to the closest road segment if the
// distance between the building and road is less than some maximal
// threshold.

   void compute_building_front_dirs()
      {
         outputfunc::write_banner("Computing building front directions:");

         for (int c=0; c<get_n_cityblocks(); c++)
         {
            cout << c << " " << flush;
            get_cityblock_ptr(c)->estimate_building_front_dirs();
         } // loop over city blocks
         outputfunc::newline();
      }

   void propagate_building_front_dirs()
      {
         outputfunc::write_banner("Propagating building front directions:");

         for (int c=0; c<get_n_cityblocks(); c++)
         {
            cout << c << " " << flush;
            get_cityblock_ptr(c)->propagate_building_front_dirs();
         } // loop over city blocks
         outputfunc::newline();
      }

// ---------------------------------------------------------------------
// Method establish_building_neighbor_handedness uses building front
// direction information to establish RHS and LHS neighbors for
// buildings located on streets.

   void establish_building_neighbor_handedness()
      {
         outputfunc::write_banner(
            "Establishing building neighbor handedness:");

         for (int c=0; c<get_n_cityblocks(); c++)
         {
            cout << c << " " << flush;
            get_cityblock_ptr(c)->initialize_building_neighbor_handedness();
            get_cityblock_ptr(c)->propagate_building_neighbor_handedness();
         } // loop over city blocks
         outputfunc::newline();
      }

// ---------------------------------------------------------------------
   void infer_more_building_front_dirs()
      {
         outputfunc::write_banner("Infering more building front directions:");

         for (int c=0; c<get_n_cityblocks(); c++)
         {
            cout << c << " " << flush;
            get_cityblock_ptr(c)->infer_more_building_front_dirs();
         } // loop over city blocks
         outputfunc::newline();
      }

// ---------------------------------------------------------------------
   void improve_corner_building_identification()
      {
         outputfunc::write_banner(
            "Improving corner building identification:");

         for (int c=0; c<get_n_cityblocks(); c++)
         {
            cout << c << " " << flush;
            get_cityblock_ptr(c)->improve_corner_building_identification();
         } // loop over city blocks
         outputfunc::newline();
      }

// ---------------------------------------------------------------------
   void break_remaining_building_tripoles()
      {
         outputfunc::write_banner("Breaking remaining building tripoles:");

         for (int c=0; c<get_n_cityblocks(); c++)
         {
            cout << c << " " << flush;
            get_cityblock_ptr(c)->break_remaining_building_tripoles();
         } // loop over city blocks
         outputfunc::newline();
      }

// ---------------------------------------------------------------------
   void associate_buildings_with_roadsegments()
      {
         outputfunc::write_banner("Associating buildings with roadsegments:");

         for (int c=0; c<get_n_cityblocks(); c++)
         {
            cout << c << " " << flush;
            get_cityblock_ptr(c)->associate_buildings_with_roadsegments();
         } // loop over city blocks
         outputfunc::newline();
      }

// ==========================================================================
// Strand construction & searching methods
// ==========================================================================

// Method construct_building_neighbor_strands fills each cityblock's
// member variable *strand_list_ptr with strands whose number of nodes
// is set by input parameter length.  It then fills these nodes'
// building_info with data about the buildings' relative heights,
// spine directions and gross directions to tall trees and small
// shrubs.

   void construct_building_neighbor_strands(
      int strand_length,
      Linkedlist<int> const *bldgs_with_tall_trees_in_back_list_ptr,
      Linkedlist<int> const *bldgs_with_small_shrubs_on_rear_left_list_ptr)
      {
         outputfunc::write_banner("Constructing building neighbor strands:");

         int n_total_strands=0;
         for (int c=0; c<get_n_cityblocks(); c++)
         {

// FAKE FAKE:  Monday, April 18 at 11:19 am

//            int c=8;
            
            cout << c << " " << flush;
            int n_strands=get_cityblock_ptr(c)->
               construct_building_neighbor_strands(strand_length);
            get_cityblock_ptr(c)->assign_strand_member_relative_heights();
            get_cityblock_ptr(c)->assign_strand_member_spine_dirs();
            get_cityblock_ptr(c)->assign_strand_member_gross_vegetation_dirs(
               bldgs_with_tall_trees_in_back_list_ptr,
               bldgs_with_small_shrubs_on_rear_left_list_ptr);
            n_total_strands += n_strands;

         } // loop over city blocks
         outputfunc::newline();
         cout << "n_total_strands = " << n_total_strands << endl;
      }

// ---------------------------------------------------------------------
// Method score_strands_agreement_with_video_data loops over all
// cityblocks and then over all strands within each cityblock's
// *strand_list_ptr.  Each strand is scored based upon its comparison
// with input *video_strand_ptr.  Those strands whose scores exceed
// some threshold value are deleted from its *strand_list_ptr.

   void score_strands_agreement_with_video_data(
      Strand<building_info*> const *video_strand_ptr,
      const int score_threshold)
      {
         outputfunc::write_banner("Scoring strands against video:");

         int n_strands_before_scoring=0;
         int n_strands_after_scoring=0;
         for (int c=0; c<get_n_cityblocks(); c++)
         {
            cout << " Cityblock c = " << c << endl;
            Linkedlist<Strand<building_info*>*>* strand_list_ptr=
               get_cityblock_ptr(c)->get_strand_list_ptr();
            
            n_strands_before_scoring += strand_list_ptr->size();

            Mynode<Strand<building_info*>*>* currnode_ptr=
               strand_list_ptr->get_start_ptr(); 
            while (currnode_ptr != NULL)
            {
               Mynode<Strand<building_info*>*>* nextnode_ptr=
                  currnode_ptr->get_nextptr();
               Strand<building_info*>* curr_strand_ptr=
                  currnode_ptr->get_data();
               int score=
                  bldgstrandfunc::score_strand_agreement_with_video_data(
                     curr_strand_ptr,video_strand_ptr);
               cout << "Strand ID = " << curr_strand_ptr->get_ID()
                    << " difference score = " << score << endl;

               if (score > score_threshold)
               {
                  strand_list_ptr->delete_node(currnode_ptr);
               }
               currnode_ptr=nextnode_ptr;
            } // while loop over nodes in *strand_list_ptr
            n_strands_after_scoring += strand_list_ptr->size();
         } // loop over city blocks
         outputfunc::newline();
         cout << "Score threshold = " << score_threshold << endl;
         cout << "Before scoring, n_strands = "
              << n_strands_before_scoring << endl;
         cout << "After scoring, n_strands = "
              << n_strands_after_scoring << endl;
      }

// ==========================================================================
// Drawing and annotation methods
// ==========================================================================

// Method annotate_block_labels loops over each entry within
// *cityblocks_network_ptr.  It generates a threeDstring for each
// block corresponding to its integer label.  This method adds the
// threeDstring label above the block within the output xyzp file.

   void annotate_block_labels(string xyzp_filename,double annotation_value)
      {
         filefunc::gunzip_file_if_gzipped(xyzp_filename);

         for (Mynode<int>* currnode_ptr=cityblocks_network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int c=currnode_ptr->get_data(); // cityblock number
            get_cityblock_ptr(c)->annotate_block_label(
               xyzp_filename,annotation_value);
         } // loop over sites in cityblocks network
//         filefunc::gzip_file(xyzp_filename);         
      }

// ---------------------------------------------------------------------
// Method recolor_cityblocks takes in integer-valued
// *cityblock_regions_twoDarray_ptr which labels city blocks.  For
// display purposes, we prefer to randomize the coloring of the blocks
// so that they do not appear with a definite trend from cool to warm
// colors.  This method assigns a deterministic yet pseudo-random
// fraction ranging over the interval [0,0.95] to each of the city
// block pixels.  

   twoDarray* recolor_cityblocks(
      twoDarray const *cityblock_regions_twoDarray_ptr)
      {
         outputfunc::write_banner("Recoloring city blocks:");

         twoDarray* colored_blocks_twoDarray_ptr=new twoDarray(
            cityblock_regions_twoDarray_ptr);
         cityblock_regions_twoDarray_ptr->copy(colored_blocks_twoDarray_ptr);

         vector<double> block_color;
         block_color.push_back(0.02);
         block_color.push_back(0.92);
         block_color.push_back(0.18);
         block_color.push_back(1.0);
         block_color.push_back(0.86);
         block_color.push_back(0.14);
         block_color.push_back(0.24);
         block_color.push_back(0.50);

         const int multiplier=13;
         const int addend=17;
         int nblocks=get_n_cityblocks();
         for (unsigned int px=0; px<colored_blocks_twoDarray_ptr->get_mdim(); 
              px++)
         {
            for (unsigned int py=0; py<colored_blocks_twoDarray_ptr->
                    get_ndim(); py++)
            {
               double b=colored_blocks_twoDarray_ptr->get(px,py);
//               cout << "px = " << px << " py = " << py << " b = " << b
//                    << endl;
  
               if (b > xyzpfunc::null_value)
               {
                  double bnew=0.95*double(
                     modulo(multiplier*basic_math::round(b)+addend,nblocks))/
                     double(nblocks);

// FAKE FAKE:  For poster purposes only  Thurs, July 14 at 3:13 pm

                  int bint=basic_math::round(b-1);
                  
                  double red=0.02;
                  double turquoise=0.50;
                  double dark_blue=0.86;
                  double dirty_yellow=0.18;
                  double white=1.0;
                  double purple=0.92;
//                   double orange=0.14;
                  double yellow=0.2;
                  double olive=0.24;

                  if (bint < 0)
                  {
                     bnew=featurefunc::road_sentinel_value;
//                     bnew=dark_blue;
                  }
                  else
                  {
                     switch(bint)
                     {
                        case 0: 
                           bnew=purple;
                           break;
                        case 2: 
                           bnew=red;
                           break;
                        case 3: 
                           bnew=dark_blue;
                           break;
                        case 9: 
                           bnew=turquoise;
                           break;
                        case 8: 
                           bnew=yellow;
                           break;
                        case 4: 
                           bnew=red;
                           break;
                        case 7: 
                           bnew=purple;
                           break;
                        case 20: 
                           bnew=turquoise;
                           break;
                        case 5:
                           bnew=turquoise;
                           break;
                        case 1:
                           bnew=dark_blue;
                           break;
                        case 6:
                           bnew=dirty_yellow;
                           break;
                        case 10:
                           bnew=red;
                           break;
                        case 11:
                           bnew=white;
                           break;
                        case 12:
                           bnew=dark_blue;
                           break;
                        case 13:
                           bnew=dirty_yellow;
                           break;
                        case 14:
                           bnew=purple;
                           break;
                        case 15:
                           bnew=yellow;
                           break;
                        case 16:
                           bnew=white;
                           break;
                        case 17:
                           bnew=olive;
                           break;
                        case 18:
                           bnew=dark_blue;
                           break;
                        case 19:
                           bnew=yellow;
                           break;
                        default:
                           bnew=block_color[modulo(basic_math::round(b),
                                                   block_color.size())];
                     }
                  } // bint < 0 conditional
                  

                  colored_blocks_twoDarray_ptr->put(px,py,bnew);
               }
            } // loop over py
         } // loop over px

         return colored_blocks_twoDarray_ptr;
      }

// ---------------------------------------------------------------------
// Method draw_3D_strands

   void draw_3D_strands(Network<building*> const *buildings_network_ptr,
                        string xyzp_filename,double annotation_value)
      {
         outputfunc::write_banner("Drawing 3D strands:");

         int strand_counter=0;
         for (int c=0; c<get_n_cityblocks(); c++)
         {
            Linkedlist<Strand<building_info*>*>* strand_list_ptr=
               get_cityblock_ptr(c)->get_strand_list_ptr();

            for (Mynode<Strand<building_info*>*>* currnode_ptr=
                    strand_list_ptr->get_start_ptr(); currnode_ptr != NULL;
                 currnode_ptr=currnode_ptr->get_nextptr())
            {
               Strand<building_info*>* curr_strand_ptr=
                  currnode_ptr->get_data();


// FAKE FAKE:  Mon Apr 18 at 1:03 pm. for viewgraph only...

//               if (curr_strand_ptr->get_ID()==40)
               {
                  bldgstrandfunc::draw_3D_strand(
                     buildings_network_ptr,curr_strand_ptr,xyzp_filename,
                     annotation_value);
               }
               
               strand_counter++;
            } // loop over strands in *strand_list_ptr
         } // loop over index c labeling city blocks
      }
         
} // cityblockfunc namespace
