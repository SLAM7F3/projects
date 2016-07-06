// ==========================================================================
// Urbanimage class member function definitions
// ==========================================================================
// Last modified on 10/1/09; 12/4/10; 3/6/14; 4/5/14
// ==========================================================================

#include <algorithm>
#include <set>
#include <vector>
#include "image/binaryimagefuncs.h"
#include "image/connectfuncs.h"
#include "datastructures/containerfuncs.h"
#include "ladar/featurefuncs.h"
#include "filter/filterfuncs.h"
#include "geometry/geometry_funcs.h"
#include "image/graphicsfuncs.h"
#include "datastructures/Hashtable.h"
#include "image/imagefuncs.h"
#include "ladar/ladarfuncs.h"
#include "math/mathfuncs.h"
#include "geometry/mybox.h"
#include "network/netlink.h"
#include "urban/oriented_box.h"
#include "geometry/parallelogram.h"
#include "plot/plotfuncs.h"
#include "geometry/polygon.h"
#include "math/prob_distribution.h"
#include "urban/roadfuncs.h"
#include "urban/roadpoint.h"
#include "network/Site.h"
#include "general/stringfuncs.h"
#include "threeDgraphics/threeDstring.h"
#include "urban/tree_cluster.h"
#include "image/TwoDarray.h"
#include "urban/urbanfuncs.h"
#include "urban/urbanimage.h"

#include "video/texture_rectangle.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void urbanimage::allocate_member_objects()
{
}		       

void urbanimage::initialize_member_objects()
{
   buildings_network_ptr=NULL;
   roadpoints_network_ptr=NULL;
}		       

urbanimage::urbanimage()
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

// Copy constructor:

urbanimage::urbanimage(const urbanimage& u):
   ladarimage(u)
{
   initialize_member_objects();
   allocate_member_objects();
   docopy(u);
}

urbanimage::urbanimage(int Nxbins,int Nybins):
   ladarimage(Nxbins,Nybins)
{
   initialize_member_objects();
   allocate_member_objects();
}

urbanimage::~urbanimage()
{
//   cout << "inside urbanimage destructor" << endl;
   Networkfunc::delete_dynamically_allocated_objects_in_network(
      buildings_network_ptr);
//   cout << "Before deleting buildings network" << endl;
   delete buildings_network_ptr;

//   Networkfunc::delete_dynamically_allocated_objects_in_network(
//      trees_network_ptr);
//   delete trees_network_ptr;

//   cout << "Before deleting roadpoints network" << endl;
   roadfunc::delete_roadpoints_network(roadpoints_network_ptr);
//   cout << "at end of urbanimage destructor" << endl;
}

// ---------------------------------------------------------------------
void urbanimage::docopy(const urbanimage& u)
{
   *buildings_network_ptr=*(u.buildings_network_ptr);
   *roadpoints_network_ptr=*(u.roadpoints_network_ptr);
}

// Overload = operator:

urbanimage& urbanimage::operator= (const urbanimage& u)
{
   if (this==&u) return *this;
   ladarimage::operator=(u);
   docopy(u);
   return *this;
}

// Note added on 7/15/05: This next method should be moved into
// featurefuncs namespace...

// ---------------------------------------------------------------------
// Member function get_feature_name takes in a sentinel value and returns
// its corresponding string label.

string urbanimage::get_feature_name(double sentinel_value)
{
   if (nearly_equal(sentinel_value,featurefunc::low_tree_sentinel_value))
   {
      return "low_tree";
   }
   else if (nearly_equal(sentinel_value,featurefunc::building_sentinel_value))
   {
      return "building";
   }
   else if (nearly_equal(sentinel_value,featurefunc::tree_sentinel_value))
   {
      return "tree";
   }
   else if (nearly_equal(sentinel_value,featurefunc::grass_sentinel_value))
   {
      return "grass";
   }
   else if (nearly_equal(sentinel_value,featurefunc::road_sentinel_value))
   {
      return "road";
   }
   else
   {
      return "unknown";
   }
}

// ==========================================================================
// Network methods
// ==========================================================================

// Member function generate_buildings_network takes in a cleaned
// features map within *features_twoDarray_ptr.  This method first
// generates a connected binary components hashtable whose entries
// correspond to linked lists of individual building rooftop pixels.
// It subsequently dynamically generates member object
// *buildings_network_ptr whose entries correspond to building
// objects.  Each building is assigned a unique ID number, and its
// center-of-mass position is saved within its own members.  A copy of
// the linked list of rooftop pixels is also saved within each of the
// building sites.

void urbanimage::generate_buildings_network(
   double min_footprint_area,twoDarray const *ztwoDarray_ptr,
   twoDarray const *features_twoDarray_ptr)
{
   outputfunc::write_banner("Generating buildings network:");
   string feature_type=get_feature_name(featurefunc::building_sentinel_value);

   Hashtable<linkedlist*>* connected_components_hashtable_ptr=
      featurefunc::extract_feature_clusters(
         min_footprint_area,featurefunc::building_sentinel_value,feature_type,
         imagedir,ztwoDarray_ptr,features_twoDarray_ptr);

   buildings_network_ptr=Networkfunc::generate_network<building>(
      ztwoDarray_ptr,connected_components_hashtable_ptr);
   urbanfunc::generate_network_contours(ztwoDarray_ptr,buildings_network_ptr);
   connectfunc::delete_connected_hashtable(
      connected_components_hashtable_ptr);
}

// ---------------------------------------------------------------------
// Member function draw_building_site_pixels loops over all sites
// within the buildings network.  It draws the pixels for each
// building onto output twoDarray *ftwoDarray_ptr.

void urbanimage::draw_building_site_pixels(
   twoDarray* ftwoDarray_ptr,double annotation_value)
{
   outputfunc::write_banner("Drawing building site pixels:");
   
   for (Mynode<int>* currnode_ptr=buildings_network_ptr->
           get_entries_list_ptr()->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int n=currnode_ptr->get_data();
      linkedlist* pixel_list_ptr=get_building_ptr(n)->get_pixel_list_ptr();

      if (pixel_list_ptr != NULL)
      {
         for (mynode* curr_pixel_ptr=pixel_list_ptr->get_start_ptr();
              curr_pixel_ptr != NULL; 
              curr_pixel_ptr=curr_pixel_ptr->get_nextptr())
         {
            int px=curr_pixel_ptr->get_data().get_var(0);
            int py=curr_pixel_ptr->get_data().get_var(1);
            ftwoDarray_ptr->put(px,py,annotation_value);
         }

      } // pixel_list_ptr != NULL conditional
   } // loop over clusters in tree network
}

// ==========================================================================
// Building 2D bounding box methods:
// ==========================================================================

// Member function check_building_contour_height_variation computes
// the average absolute height variation (in the outward radial
// direction) along the shrink-wrapped contour for each building in
// the buildings network.  For most buildings out in the clear, the
// average radial height jump from rooftop down to ground level or
// from rooftop up to taller tree levels should be sizable.

// This method was concocted to help detect clumps of tree pixels
// incorrectly classified as building rooftops.  Such tree clumps
// should exhibit small height variation relative to other surrounding
// tree pixels.  They should also be surrounded to large extent by
// other tree pixels.  If both of these conditions are satisfied, the
// "building" is removed from the buildings network.

void urbanimage::check_building_contour_height_variation(
   twoDarray const *xderiv_twoDarray_ptr,
   twoDarray const *yderiv_twoDarray_ptr,
   twoDarray const *features_twoDarray_ptr)
{
   outputfunc::write_banner("Computing building contour height variation:");
   Mynode<int>* currnode_ptr=buildings_network_ptr->get_entries_list_ptr()->
      get_start_ptr();
   while (currnode_ptr != NULL)
   {
      Mynode<int>* nextnode_ptr=currnode_ptr->get_nextptr();
      int n=currnode_ptr->get_data();
      const contour* contour_ptr=get_building_ptr(n)->get_contour_ptr();
      double avg_height_variation=featurefunc::abs_gradient_contour_integral(
         xderiv_twoDarray_ptr,yderiv_twoDarray_ptr,contour_ptr);
//      cout << "building n = " << n << " average height variation = "
//           << avg_height_variation << endl;

      const double min_avg_height_variation=6;
      if (fabs(avg_height_variation < min_avg_height_variation ))
      {
         double avg_outside_tree_frac=
            featurefunc::tree_pixels_outside_contour(
               features_twoDarray_ptr,contour_ptr);

         cout << "	*** Building n = " << n 
              << " has abs height variation = " << avg_height_variation 
              << " ***" << endl;
         cout << "Average outside tree frac = "
              << avg_outside_tree_frac << endl;

         if (avg_outside_tree_frac > 0.6)
         {
            buildings_network_ptr->delete_single_site(n);
         }
      }
      currnode_ptr=nextnode_ptr;
   } // loop over sites within buildings network
}

// ---------------------------------------------------------------------
// Member function recheck_footprint_areas

void urbanimage::recheck_footprint_areas(
   double min_footprint_area,twoDarray const *ztwoDarray_ptr)
{
   outputfunc::write_banner("Checking footprint areas:");

   const double dA=ztwoDarray_ptr->get_deltax()*ztwoDarray_ptr->get_deltay();
   for (Mynode<int>* currnode_ptr=buildings_network_ptr->
           get_entries_list_ptr()->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int n=currnode_ptr->get_data();
      linkedlist* pixel_list_ptr=get_building_ptr(n)->get_pixel_list_ptr();
      double pixel_area=pixel_list_ptr->size()*dA;
      double area_ratio=pixel_area/min_footprint_area;
      if (area_ratio < 0.5)
      {
         cout << "!!! Cluster n = " << n << " has pixel area = "
              << pixel_area << " !!!" << endl;
      }
   } // loop over sites within network
}

// ---------------------------------------------------------------------
// Member function score_buildings_contour_edge_fits takes in a
// refined feature classification map and pulls out its building
// pixels.  Looping over all entries within the buildings network, it
// uses the edge gradient score function to assess the goodness-of-fit
// of each contour edge segment to the binary building pixel map.
// This method hangs onto the top-scored segments within each contour
// and uses them to form an edge direction angle distribution.  In
// theory, this distribution should exhibit 4 peaks spaced apart in
// angle by 90 degrees for building's whose rooftops are
// well-represented by orthogonal polygons.  In order to find the
// rooftop's symmetry direction, this method convolves the edge
// direction angle distribution with a filter that has 4 narrow
// gaussians spaced apart by 90 degrees.  The location of the maximum
// within the filtered distribution determines the building's primary
// symmetry direction modulo 90 degrees.  This method next performs
// profiling along the symmetry axes to determine the rooftop's width
// and length extents.  The 2D bounding box derived by these symmetry
// methods is saved within the *bbox_ptr member of each building
// object in the buildings network.

void urbanimage::score_buildings_contour_edge_fits(
   twoDarray const *features_twoDarray_ptr)
{
   outputfunc::write_banner("Scoring buildings' contour edge fits:");   

// First generate binary feature map containing only building pixels:

   twoDarray* binary_bldg_twoDarray_ptr=new twoDarray(
      features_twoDarray_ptr);
   binaryimagefunc::binary_threshold_for_particular_cutoff(
      featurefunc::building_sentinel_value,features_twoDarray_ptr,
      binary_bldg_twoDarray_ptr,0);

// Loop over all entries within buildings network starts here:

//   twoDarray* score_twoDarray_ptr=new twoDarray(features_twoDarray_ptr);
//   score_twoDarray_ptr->initialize_values(xyzpfunc::null_value);

   for (Mynode<int>* currnode_ptr=buildings_network_ptr->
           get_entries_list_ptr()->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int n=currnode_ptr->get_data(); // contour number
      cout << n << " " << flush; 
      const contour* contour_ptr=get_building_ptr(n)->get_contour_ptr();

// Use edge gradient score function to assess goodness-of-fit between
// each edge segment in current building's contour with binary feature
// map:

      const unsigned int nvertices=contour_ptr->get_nvertices();
      const double correlation_length=2.0;	// meters

      int n_reliable_vertices=0;
      int min_n_reliable_vertices=10;
      double edge_score_threshold=0.9;
      double theta[nvertices];

      while (n_reliable_vertices < min_n_reliable_vertices)
      {
         n_reliable_vertices=0;

         for (unsigned int v=0; v<nvertices; v++)
         {
            linesegment curr_edge(contour_ptr->get_edge(v));
            double edge_score=graphicsfunc::edge_gradient_contour_integral(
               v,correlation_length,*contour_ptr,binary_bldg_twoDarray_ptr)
               /correlation_length;

            if (edge_score > edge_score_threshold)
            {
               theta[n_reliable_vertices++]=
                  atan2(curr_edge.get_ehat().get(1),
                        curr_edge.get_ehat().get(0))*180/PI;
               edge_score=10*(edge_score-edge_score_threshold);
            }
            else
            {
               edge_score=0;
            }

//         cout << "vertex v = " << v
//              << " x = " << contour_ptr->get_vertex(v).first.get(0)
//              << " y = " << contour_ptr->get_vertex(v).first.get(1)
//              << " theta = " << theta[v]
//              << " score = " << edge_score << endl;
//            drawfunc::draw_line(contour_ptr->get_edge(v),edge_score,
//                                score_twoDarray_ptr,false,true);
         } // loop over index v labeling contour vertices
         if (edge_score_threshold >= 0.4)
         {
            edge_score_threshold -= 0.1;
         }
         else
         {
            min_n_reliable_vertices--;
         }

         if (n_reliable_vertices < min_n_reliable_vertices)
         {
//            cout << "n_reliable_vertices = " << n_reliable_vertices
//                 << " edge_score_threshold = " 
//                 << edge_score_threshold << endl;
         }
      } // while n_reliable_vertices < min_n_reliable_vertices
            
      const unsigned int nbins=500;
      prob_distribution prob(n_reliable_vertices,theta,nbins);
      prob.set_densityfilenamestr("raw_edge_directions.meta");
      prob.set_freq_histogram(true);
      prob.set_xlabel("Contour edge direction angle (degs)");
//      prob.write_density_dist();
      
// Form filter with 4 narrow gaussian peaks spaced apart by 90 degrees:

      const unsigned int n_copies=4;
      const double sigma=3;	// degs
      const double x_shift=90;	// degs
      double h[nbins],h_replicate[nbins];
      filterfunc::gaussian_filter(nbins,prob.get_dx(),sigma,h);
      filterfunc::circularly_replicate_filter(
         nbins,prob.get_dx(),n_copies,x_shift,h,h_replicate);
      prob_distribution prob_filter(
         nbins,prob.get_minimum_x(),prob.get_maximum_x(),h_replicate);
      prob_filter.set_densityfilenamestr("right_angle_filter.meta");
      prob_filter.set_xlabel("Direction angle (degs)");
//      prob_filter.write_density_dist();

// Convolve edge segment direction angle distribution with 4 gaussian
// peak filter:

      double raw_p[nbins],filtered_p[nbins];
      for (unsigned int i=0; i<nbins; i++) raw_p[i]=prob.get_p(i);
      filterfunc::brute_force_filter(
         nbins,nbins,raw_p,h_replicate,filtered_p,true);

      prob_distribution prob_convolve(
         nbins,prob.get_minimum_x(),prob.get_maximum_x(),filtered_p);
      prob_convolve.set_densityfilenamestr("filtered_edge_direction.meta");
      prob_convolve.set_freq_histogram(true);
      prob_convolve.set_xlabel("Contour edge direction angle (degs)");
//      prob_convolve.write_density_dist();

// Maximum peak within convolved distribution corresponds to rooftop's
// symmetry direction (modulo 90 degrees).  Draw rotated pair of axes
// onto *score_twoDarray_ptr to display building's derived symmetry
// directions:

      int max_bin;
      max_array_value(nbins,max_bin,filtered_p);
      double symmetry_angle=prob.get_x(max_bin)*PI/180;
//         drawfunc::draw_axes(
//            colorfunc::white,score_twoDarray_ptr,
//            score_twoDarray_ptr->center_point(),symmetry_angle);

// Determine building's spatial extents along symmetry axes' directions:

      twoDarray* binary_mask_twoDarray_ptr=
         generate_twoDarray_for_centered_building();

      double x_min,x_max,y_min,y_max;
      generate_rooftop_pixels_profiles(
         n,symmetry_angle,binary_mask_twoDarray_ptr,
         x_min,x_max,y_min,y_max);
      delete binary_mask_twoDarray_ptr;

      vector<threevector> vertex;
      vertex.push_back(threevector(x_max,y_min));
      vertex.push_back(threevector(x_max,y_max));
      vertex.push_back(threevector(x_min,y_max));
      vertex.push_back(threevector(x_min,y_min));
      parallelogram *bbox_ptr=new parallelogram(vertex);

      bbox_ptr->rotate(Zero_vector,0,0,symmetry_angle);

// Set x and y coordinates of the center of each building's 2D bbox
// equal to its COM x and y coordinates:
         
      threevector COM(get_building_ptr(n)->get_posn());
      bbox_ptr->translate(threevector(COM.get(0),COM.get(1)));
      get_building_ptr(n)->set_bbox_ptr(bbox_ptr);

//      drawfunc::draw_polygon(*bbox_ptr,colorfunc::white,score_twoDarray_ptr);
      
//      drawfunc::draw_contour(
//         *contour_ptr,0.2,0.2,0.2,binary_bldg_twoDarray_ptr,true,false);
//      writeimage("edge_scores",binary_bldg_twoDarray_ptr,false,
//                 ladarimage::p_data);
//      writeimage("edge_scores",score_twoDarray_ptr,false,ladarimage::p_data);
   } // loop over nodes in buildings network entries list
   outputfunc::newline();

   delete binary_bldg_twoDarray_ptr;
//   delete score_twoDarray_ptr;
}

// ---------------------------------------------------------------------
// Member function construct_rooftop_orthogonal_polygons loops over
// each entry within the buildings network.  It first retrieves the 2D
// bounding box and shrink-wrap contour for each building in the
// network.  This method re-aligns individual edge segments within a
// contour so that they are oriented along the principle symmetry
// directions defined by the 2D bounding box.  It subsequently
// extracts approximate corner locations where the contour bends by 90
// degrees.  A reduced contour formed from these approximate corner
// locations is formed next.  The alignment procedure is performed on
// the edge segments of the reduced contour, and intersection points
// between the re-aligned edges are computed.  These intersection
// points are used to define a final contour which represents an
// orthogonal polygon for the rooftop's footprint on flattened ground.
// Each building's original contour is destroyed and replaced with the
// new & improved contour generated by this method.

void urbanimage::construct_rooftop_orthogonal_polygons(
   twoDarray const *features_twoDarray_ptr)
{
   outputfunc::write_banner("Constructing rooftop orthogonal polygons:");   

   twoDarray* ftwoDarray_ptr=new twoDarray(features_twoDarray_ptr);
   ftwoDarray_ptr->initialize_values(xyzpfunc::null_value);
//   features_twoDarray_ptr->copy(ftwoDarray_ptr);

// Loop over all entries within buildings network:

   for (Mynode<int>* currnode_ptr=buildings_network_ptr->
           get_entries_list_ptr()->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int n=currnode_ptr->get_data(); // contour number
      cout << n << " " << flush; 
      parallelogram* bbox_ptr=get_building_ptr(n)->get_bbox_ptr();
      threevector l_hat(bbox_ptr->get_lhat());
      threevector w_hat(bbox_ptr->get_what());

// Align individual edges within contour with 2D bounding box symmetry
// directions.  Then extract corner locations within aligned contour
// where direction changes by 90 degrees:

      contour* contour_ptr=get_building_ptr(n)->get_contour_ptr();
      contour_ptr->align_edges_with_sym_dirs(w_hat,l_hat);

// Construct linked list of building corner locations:

      Linkedlist<int>* corner_list_ptr=
         contour_ptr->locate_contour_corners(w_hat,l_hat);

      Linkedlist<threevector>* perim_posn_list_ptr=new 
         Linkedlist<threevector>;
      for (Mynode<int>* corner_node_ptr=corner_list_ptr->get_start_ptr();
           corner_node_ptr != NULL; corner_node_ptr=corner_node_ptr->
              get_nextptr())
      {
         int n_corner_edge=corner_node_ptr->get_data();
         threevector v1(contour_ptr->get_edge(n_corner_edge).get_v1());
         perim_posn_list_ptr->append_node(v1);
      }
      delete corner_list_ptr;
      
// Generate a reduced contour from just the corner locations.  Then
// re-align its edges with symmetry directions w_hat and l_hat:

      contour* reduced_contour_ptr=new contour(perim_posn_list_ptr);
      delete perim_posn_list_ptr;

      reduced_contour_ptr->align_edges_with_sym_dirs(w_hat,l_hat);
      vector<linesegment*>* perturbed_edge_ptr=
         reduced_contour_ptr->consolidate_parallel_edges();

// Find intersection points between successive edge segments for
// reduced contour.  Build final contour from these intersection points:

      bool intersection_pnt_on_curr_edge,intersection_pnt_on_next_edge;
      unsigned int n_perturbed_edges=perturbed_edge_ptr->size();
      threevector intersection_pnt;
      perim_posn_list_ptr=new Linkedlist<threevector>;

      vector<threevector> ipoint(n_perturbed_edges);
      for (unsigned int i=0; i<n_perturbed_edges; i++)
      {
         linesegment* curr_edge_ptr=(*perturbed_edge_ptr)[i];
         linesegment* next_edge_ptr=(*perturbed_edge_ptr)[
            modulo(i+1,n_perturbed_edges)];
         
//         cout << "i = " << i << endl;
//         cout << "curr_edge = " << *curr_edge_ptr << endl;
//         cout << "next_edge = " << *next_edge_ptr << endl;
//         cout << " e_hat = " << curr_edge_ptr->get_ehat() 
//              << " next e_hat = " << next_edge_ptr->get_ehat()
//              << " dotproduct = " << curr_edge_ptr->get_ehat().dot(
//                 next_edge_ptr->get_ehat()) << endl;
         curr_edge_ptr->point_of_intersection(
            *next_edge_ptr,intersection_pnt,intersection_pnt_on_curr_edge,
            intersection_pnt_on_next_edge);
         
         perim_posn_list_ptr->append_node(intersection_pnt);
         ipoint[i]=intersection_pnt;
      } // loop over index i labeling perturbed contour edge

// Recall member function contour::consolidate_parallel_edges()
// dynamically generates an STL vector of dynamically generated edge
// linesegments.  So now that we're done with *perturbed_edge_ptr, we
// need to first delete its contents before deleting the STL vector
// itself:

      for (unsigned int i=0; i<n_perturbed_edges; i++)
      {
         linesegment* curr_edge_ptr=perturbed_edge_ptr->at(i);
         delete curr_edge_ptr;
      }
      delete perturbed_edge_ptr;

      polygon p_intersect(ipoint);
      p_intersect.translate(-p_intersect.vertex_average());
      p_intersect.initialize_edge_segments();
      p_intersect.compute_perimeter();
      
// Search for any 3 contiguous intersection points which all lie very
// close to one another.  Replace such 3 points with their common COM:

      const double min_distance=0.01*p_intersect.compute_perimeter();
//      const double min_distance=basic_math::max(features_twoDarray_ptr->get_deltax(),
//                                    features_twoDarray_ptr->get_deltay());
      geometry_func::eliminate_nearly_degenerate_intersection_point_triples(
         min_distance,perim_posn_list_ptr);

      contour* final_contour_ptr=new contour(perim_posn_list_ptr);
      double footprint_area=
         final_contour_ptr->compute_orthogonal_contour_area();
      delete perim_posn_list_ptr;

//      drawfunc::draw_contour(
//         *contour_ptr,0.6,0.4,0.2,ftwoDarray_ptr,true,false);
//      drawfunc::draw_contour(
//         *reduced_contour_ptr,0.6,1.0,0.5,ftwoDarray_ptr,true,true);
//      drawfunc::draw_contour(
//         *final_contour_ptr,0.6,0.4,0.2,ftwoDarray_ptr,true,false);
//      writeimage("aligned_contour",ftwoDarray_ptr,false,ladarimage::p_data);

// Destroy original building contour and replace it with final reduced
// version:

      delete contour_ptr;
      delete reduced_contour_ptr;
      get_building_ptr(n)->set_contour_ptr(final_contour_ptr);
      get_building_ptr(n)->set_footprint_area(footprint_area);
   } // loop over nodes in buildings network entries list
   outputfunc::newline();

   delete ftwoDarray_ptr;
}

// ---------------------------------------------------------------------
// Member function simplify_rooftop_orthogonal_polygons loops over all
// entries within the buildings network.  It computes a characteristic
// length scale for each building's shrink-wrapped orthogonal contour.
// This method subsequently removes contour "corner pieces" which are
// smaller than the characteristic length scale.  We concocted this
// method in an attempt to eliminate spurious fine structure from
// extracted building wireframe models.

void urbanimage::simplify_rooftop_orthogonal_polygons()
{
   outputfunc::write_banner("Simplifying rooftop orthogonal polygons:");   

// Loop over all entries within buildings network:

   for (Mynode<int>* currnode_ptr=buildings_network_ptr->
           get_entries_list_ptr()->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int n=currnode_ptr->get_data(); // contour number
      cout << n << " " << flush; 
      contour* contour_ptr=get_building_ptr(n)->get_contour_ptr();
      int nvertices=contour_ptr->get_nvertices();

      if (nvertices < 4)
      {
         cout << "Error in urbanimage::simplify_rooftop_orthogonal_polygons()"
              << endl;
         cout << "Current contour has " << nvertices << " vertices !!" 
              << endl;
         cout << "*contour_ptr = " << *contour_ptr << endl;
      }
      else if (nvertices==4)
      {

// Rectangles cannot be simplified!         

      }
      else if (nvertices > 4)
      {

// Set characteristic length to some physically reasonable distance

         double char_length=2;	// meters
         geometry_func::remove_short_contour_edges(char_length,contour_ptr);
         double footprint_area=contour_ptr->compute_orthogonal_contour_area();
         get_building_ptr(n)->set_contour_ptr(contour_ptr);
         get_building_ptr(n)->set_footprint_area(footprint_area);
      }
      
   } // loop over nodes in buildings network entries list
   outputfunc::newline();
}

// ---------------------------------------------------------------------
// Member function improve_rooftop_sym_dirs first computes a
// "concavity" measure for each rooftop within the buildings network.
// It forms the ratio of the area of the rooftop's orthogonal contour
// to the area of rooftop footprint bounding box.  The rooftop looks
// increasingly convex [concave] as this area ratio approaches 1 [0].
// This method uses the poor-man's error function to convert this area
// ratio into a normalized weight w.  For small [large] values of w,
// the building symmetry directions determined by the orthogonal
// footprint contour [azimuthal rooftop spin] methods are more likely
// to be accurate.  This method consequently forms the weighted
// average of the symmetry directions determined by these two methods.

void urbanimage::improve_rooftop_sym_dirs()
{
   outputfunc::write_banner("Improving rooftop symmetry directions:");   

   twoDarray* binary_mask_twoDarray_ptr=
      generate_twoDarray_for_centered_building();

// Loop over all entries within buildings network:

   for (Mynode<int>* currnode_ptr=buildings_network_ptr->
           get_entries_list_ptr()->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int n=currnode_ptr->get_data(); // contour number
      cout << n << " " << flush; 

      contour const *contour_ptr=get_building_ptr(n)->get_contour_ptr();
      contour* bbox_contour_ptr=contour_ptr->orthogonal_contour_bbox();
      double area_ratio=contour_ptr->get_area()/bbox_contour_ptr->get_area();
      delete bbox_contour_ptr;

      const double mu=0.90;
      const double sigma=0.25;
      double arg=(area_ratio-mu)/(SQRT_TWO*sigma);
      double w=0.5*(1+mathfunc::poor_man_erf(arg));
//      cout << "weight = " << w << endl;

// First retrieve building symmetry directions derived via rooftop
// contour edge fitting:

      parallelogram* bbox_ptr=get_building_ptr(n)->get_bbox_ptr();
      threevector w_hat(bbox_ptr->get_what());
      double edge_fit_sym_angle=atan2(w_hat.get(1),w_hat.get(0));
//      cout << "Edge fitting symmetry angle = " << edge_fit_sym_angle*180/PI
//           << endl;

// Next use azimuthal rooftop rotation method to obtain another
// estimate for building symmetry directions:

      double z_intermediate,z_hi;
      double *xnew,*ynew,*znew,*pnew;
      xnew=ynew=znew=pnew=NULL;
      string xyzp_filename=imagedir+"rooftop_pnts_"
         +stringfunc::number_to_string(n)+".xyzp";
      linkedlist* translated_pixel_list_ptr=
         generate_translated_rooftop_pixel_list(n);

      int nclean_pixels=urbanfunc::isolate_and_clean_rooftop_pixels(
         translated_pixel_list_ptr,xyzp_filename,z_intermediate,z_hi,
         xnew,ynew,znew,pnew);
      double azimuthal_sym_angle=urbanfunc::rooftop_symmetry_directions(
         n,nclean_pixels,xnew,ynew,znew,pnew,
         binary_mask_twoDarray_ptr,z_intermediate,z_hi,xyzp_filename);
      azimuthal_sym_angle=basic_math::phase_to_canonical_interval(
         azimuthal_sym_angle,edge_fit_sym_angle-PI/4,
         edge_fit_sym_angle+PI/4);
//         cout << "azimuthal_sym_angle = " << azimuthal_sym_angle*180/PI 
//              << endl;

      double avg_sym_angle=(1-w)*edge_fit_sym_angle+w*azimuthal_sym_angle;
//         cout << "avg_sym_angle = " << avg_sym_angle*180/PI << endl;

//      draw3Dfunc::draw_symmetry_directions(
//         z_hi,azimuthal_sym_angle,xyzp_filename,
//	   draw3Dfunc::annotation_value1);
//      draw3Dfunc::draw_symmetry_directions(
//         z_hi,edge_fit_sym_angle,xyzp_filename,
// 	   draw3Dfunc::annotation_value2);
//      draw3Dfunc::draw_symmetry_directions(
//         z_hi,avg_sym_angle,xyzp_filename,0.1);

      bbox_ptr->rotate(bbox_ptr->vertex_average(),0,0,
                       avg_sym_angle-edge_fit_sym_angle);
   } // loop over nodes in buildings network entries list
   outputfunc::newline();

   delete binary_mask_twoDarray_ptr;
}

// ---------------------------------------------------------------------
// Member function decompose_rooftop_orthogonal_polygons loops over
// all entries within the buildings network.  Orthogonal contours
// surrounding all buildings in the network are presumed to have
// already been computed and stored in their *contour_ptr members.
// This member function decomposes each concave orthogonal contour
// into a sum of rectangles and stores them within each building's
// subcontour linked list member.

void urbanimage::decompose_rooftop_orthogonal_polygons()
{
   outputfunc::write_banner("Decomposing rooftop orthogonal polygons:");   

// Loop over all entries within buildings network:

//   cout << "Buildings_network has "
//        << buildings_network_ptr->get_entries_list_ptr()->size()
//        << " entries" << endl;
   
   for (Mynode<int>* currnode_ptr=buildings_network_ptr->
           get_entries_list_ptr()->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int n=currnode_ptr->get_data(); // contour number
      cout << n << " " << flush; 
      contour* contour_ptr=get_building_ptr(n)->get_contour_ptr();

      Linkedlist<contour*>* subcontour_list_ptr=new Linkedlist<contour*>;
      subcontour_list_ptr->append_node(new contour(*contour_ptr));
      geometry_func::
         decompose_orthogonal_concave_contour_into_large_rectangles(
            subcontour_list_ptr);
      geometry_func::consolidate_rectangle_subcontours(subcontour_list_ptr);
      geometry_func::delete_small_rectangle_subcontours(
         contour_ptr,subcontour_list_ptr);
      get_building_ptr(n)->set_subcontour_list_ptr(subcontour_list_ptr);

//      int counter=0;
//      for (Mynode<contour*>* currnode_ptr=get_building_ptr(n)->
//              get_subcontour_list_ptr()->get_start_ptr();
//           currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
//      {
//         contour* curr_contour_ptr=currnode_ptr->get_data();
//         cout << "**************************************************" << endl;
//         cout << "counter = " << counter 
//              << " sub contour = " << *curr_contour_ptr << endl;
//         counter++;
//      }
   } // loop over entries in buildings network
   outputfunc::newline();
}

// ---------------------------------------------------------------------
// Method generate_building_bboxes loops through all of the entries
// within the buildings hashtable.  It first computes the angular
// orientation of the rectangular side which is nearest to a
// building's center-of-mass.  It subsequently uses this orientation
// information to wrap a bounding box around the building's rooftop
// pixels.  Results are stored within the bounding box member pointer
// for each building.

void urbanimage::generate_building_bboxes()
{
   outputfunc::write_banner("Generating building bounding boxes:");   
   twoDarray* binary_mask_twoDarray_ptr=
      generate_twoDarray_for_centered_building();

   for (Mynode<int>* currnode_ptr=buildings_network_ptr->
           get_entries_list_ptr()->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int n=currnode_ptr->get_data();
      cout << "Bounding box " << n+1 << " out of " << get_n_buildings()
           << endl;
      double theta_bldg=compute_building_orientation(
         n,binary_mask_twoDarray_ptr);
      compute_building_bbox(n,theta_bldg,binary_mask_twoDarray_ptr);
   } // loop over nodes in buildings network entries list
   delete binary_mask_twoDarray_ptr;
}

// ---------------------------------------------------------------------
// Method draw_building_boxes loops through all of the entries within
// the buildings network and draws their bounding boxes to output
// twoDarray *ftwoDarray_ptr.

void urbanimage::draw_building_bboxes(twoDarray* ftwoDarray_ptr)
{
   for (Mynode<int>* currnode_ptr=buildings_network_ptr->
           get_entries_list_ptr()->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int n=currnode_ptr->get_data();
      parallelogram* bbox_ptr=get_building_ptr(n)->get_bbox_ptr();
      if (bbox_ptr != NULL)
      {
         const double radius=0.75;	// meter
         const double scalefactor=1.2;
         polygon curr_bbox(*bbox_ptr);
         curr_bbox.scale(scalefactor);
         drawfunc::draw_thick_polygon(
            curr_bbox,colorfunc::white,radius,ftwoDarray_ptr);
      }
   } // loop over nodes in buildings network entries list
//   writeimage("bldg_bboxes",ftwoDarray_ptr,false,ladarimage::p_data);
}

// ---------------------------------------------------------------------
// Method generate_twoDarray_for_centered_building assumes that each
// isolated building has x and y extents which are less than its
// constant max_x and max_y parameters.  It dynamically generates and
// returns a twoDarray which is large enough to accomodate such a
// maximally sized building.

twoDarray* urbanimage::generate_twoDarray_for_centered_building(
   double max_x,double max_y)
{
   const double minimum_maxsize=35;	// meters
   max_x=basic_math::max(max_x,minimum_maxsize);
   max_y=basic_math::max(max_y,minimum_maxsize);
   int nxbins=2*basic_math::round(max_x/z2Darray_ptr->get_deltax());
   int nybins=2*basic_math::round(max_y/z2Darray_ptr->get_deltay());

   twoDarray* centered_twoDarray_ptr=new twoDarray(nxbins,nybins);
   centered_twoDarray_ptr->init_coord_system(max_x,max_y);
   return centered_twoDarray_ptr;
}

// ---------------------------------------------------------------------
// Method compute_building_orientation takes in a building ID number
// as well as a twoDarray to hold a binary mask for the building.
// This method temporarily recenters the building about (0,0), and it
// subsequently transforms the binary mask from Cartesian to polar
// coordinates.  After integrating over the radial polar direction,
// this method generates the angular profile for the building.
// Assuming that the building is basically rectangular in shape, this
// method searches for the angle where the profile value is minimal.
// This corresponds to the angular direction where the building's side
// lies closest to the building's center-of-mass.  The method returns
// this minimal angular direction.

double urbanimage::compute_building_orientation(
   int n,twoDarray* binary_mask_twoDarray_ptr)
{
// First draw building's binary footprint onto universally sized
// twoDarray *binary_mask_twoDarray_ptr after building's COM has been
// translated to origin (0,0):

   binary_mask_for_centered_building(n,binary_mask_twoDarray_ptr);
//   writeimage("centered_mask_"+stringfunc::number_to_string(n),
//              binary_mask_twoDarray_ptr,false,ladarimage::p_data);

// Convert binary house mask data from Cartesian to polar coordinates:

   twoDarray* fpolar_twoDarray_ptr=
      imagefunc::convert_intensities_to_polar_coords(
         binary_mask_twoDarray_ptr);
   myimage* polarimage_ptr=imagefunc::generate_polar_image(
      imagedir,colortablefilename,fpolar_twoDarray_ptr);

// Integrate out radial dimension and generate filtered angular
// profile:

   double* filtered_theta_profile;
   int ntheta_bins=polarimage_ptr->compute_polar_angle_intensity_profile(
      filtered_theta_profile,fpolar_twoDarray_ptr);
   delete polarimage_ptr;

// Compute angle for which filtered angular profile is minimal:

   int min_bin;
//   double min_profile_value=min_array_value(
   min_array_value(ntheta_bins,min_bin,filtered_theta_profile);
   delete filtered_theta_profile;
//   cout << "min_profile_value = " << min_profile_value << endl;

   double theta_bldg=fpolar_twoDarray_ptr->get_ylo()+
      min_bin*fpolar_twoDarray_ptr->get_deltay();
   delete fpolar_twoDarray_ptr;
//   cout << "theta_bldg = " << theta_bldg << endl;
   theta_bldg *= PI/180;
   return theta_bldg;
}

// ---------------------------------------------------------------------
// Method draw_building_orientation takes in a building ID number, an
// angle theta which corresponds to the angular direction where the
// building's side is closest to its COM as well as an output
// twoDarray *ftwoDarray_ptr.  It draws a white line segment running
// from the building's COM along the theta direction to indicate the
// building's orientation.
// 

void urbanimage::draw_building_orientation(
   int n,double theta,twoDarray* ftwoDarray_ptr)
{
   threevector COM(get_building_ptr(n)->get_posn());
   threevector n_hat(cos(theta),sin(theta));
   linesegment sym_dir(COM,COM+10*n_hat);
   drawfunc::draw_line(sym_dir,colorfunc::white,ftwoDarray_ptr);
}

// ---------------------------------------------------------------------
// Method binary_mask_for_centered_building takes in a building's ID
// number as well as an output twoDarray *binary_mask_twoDarray_ptr.
// It first generates a linked list of building rooftop pixels which
// are translated so that the building's center-of-mass lies at (0,0).
// It subsequently draws the rooftop onto *binary_mask_twoDarray_ptr
// as a binary image.

void urbanimage::binary_mask_for_centered_building(
   int n,twoDarray*& binary_mask_twoDarray_ptr)
{
   binary_mask_twoDarray_ptr->clear_values();
   linkedlist* translated_pixel_list_ptr=
      generate_translated_rooftop_pixel_list(n);

   bool resize_binary_mask_flag=false;
   unsigned int px,py;
   double currx,curry;
   if (translated_pixel_list_ptr != NULL)
   {
      for (mynode* curr_pixel_ptr=translated_pixel_list_ptr->get_start_ptr();
           curr_pixel_ptr != NULL; curr_pixel_ptr=curr_pixel_ptr->
              get_nextptr())
      {

// Note: px and py values stored within 0th and 1st datapoint
// variables represent UNTRANSLATED pixel coordinates.  In order to
// find the pixel coordinates corresponding to the translated (x,y)
// values within *translated_pixel_list_ptr, we need to issue an
// explicit call to point_to_pixel(x,y,px,py)...

         currx=curr_pixel_ptr->get_data().get_var(2);
         curry=curr_pixel_ptr->get_data().get_var(3);

         if (binary_mask_twoDarray_ptr->point_to_pixel(currx,curry,px,py))
         {
            binary_mask_twoDarray_ptr->put(px,py,1);
         }
         else
         {
//            cout << "inside urbanimage::binary_mask_for_centered_building()" 
//                 << endl;
//            cout << "currx = " << curr_pixel_ptr->get_data().get_var(2) 
//                 << " curry = " << curr_pixel_ptr->get_data().get_var(3)
//                 << " lies outside binary mask twoDarray!" << endl;
            resize_binary_mask_flag=true;
            break;
         }
      } // loop over translated pixels within linked list
   } // translated_pixel_list_ptr != NULL conditional

   delete translated_pixel_list_ptr;

// If binary mask array is too small to accomodate building n,
// destroy it, replace with a larger array and call this method again:

   if (resize_binary_mask_flag)
   {
      delete binary_mask_twoDarray_ptr;
      binary_mask_twoDarray_ptr=generate_twoDarray_for_centered_building(
         fabs(currx)+10,fabs(curry)+10);
      binary_mask_for_centered_building(n,binary_mask_twoDarray_ptr);
   }
}

// FAKE FAKE: Move this next method to urbanfunc namespace...3:18 pm
// on friday,July 23

// ---------------------------------------------------------------------
// Method generate_translated_rooftop_pixel_list takes in a building's
// ID number.  Assuming that the building's rooftop pixel locations
// have already been identified and stored within the building's
// *pixel_list_ptr linked list, this method translates each pixel
// position so that their collective center-of-mass moves to the
// origin (0,0).  This method returns a new, dynamically generated
// linked list containing the translated rooftop pixel information.

linkedlist* urbanimage::generate_translated_rooftop_pixel_list(int n)
{
   const int n_node_indep_vars=4;
   const int n_node_depend_vars=2;
   double var[n_node_indep_vars];
   double func_value[n_node_depend_vars];

   linkedlist* translated_pixel_list_ptr=NULL;
   building* curr_house_ptr=get_building_ptr(n);
   if (curr_house_ptr != NULL)
   {
      threevector COM=curr_house_ptr->get_posn();

      linkedlist* curr_pixel_list_ptr=curr_house_ptr->get_pixel_list_ptr();
      if (curr_pixel_list_ptr != NULL)
      {
         translated_pixel_list_ptr=new linkedlist;
         mynode* curr_pixel_ptr=curr_pixel_list_ptr->get_start_ptr();
            
// Translate rooftop pixels' positions (but not pixels' coordinates!)
// so that COM is reset to origin:

         while (curr_pixel_ptr != NULL)
         {
            var[0]=curr_pixel_ptr->get_data().get_var(0);
            var[1]=curr_pixel_ptr->get_data().get_var(1);
            var[2]=curr_pixel_ptr->get_data().get_var(2)-COM.get(0);
            var[3]=curr_pixel_ptr->get_data().get_var(3)-COM.get(1);
            func_value[0]=curr_pixel_ptr->get_data().get_func(0);
            func_value[1]=curr_pixel_ptr->get_data().get_func(1);

            translated_pixel_list_ptr->append_node(
               datapoint(n_node_indep_vars,n_node_depend_vars,
                         var,func_value));
            curr_pixel_ptr=curr_pixel_ptr->get_nextptr();
         }
      }
   }
   return translated_pixel_list_ptr;
}

// ---------------------------------------------------------------------
// Method compute_building_bbox takes in a building's ID number as
// well as the angular orientation of the side lying closest to the
// building's center-of-mass.  It also takes in a binary mask of the
// building's rooftop pixels within input twoDarray *ftwoDarray_ptr.
// This method assumes that the building's shape is well approximated
// by a rectangle.  It first rotates the building so that it is
// aligned with the horizontal and vertical directions.  It
// subsequently computes the binary mask's horizontal and vertical
// profiles.  The edges of the profiles are used to determine the size
// of the bounding box surrounding the rooftop pixels.  The bounding
// box is rotated back to the building's original orientation and it
// is centered upon the building's center-of-mass.

void urbanimage::compute_building_bbox(
   int n,double bbox_theta,twoDarray const *ftwoDarray_ptr)
{
   twoDarray* fexpand_twoDarray_ptr=ftwoDarray_ptr->expand_for_rotation(
      0,true);
//   writeimage(
//      "expanded_rectangle",fexpand_twoDarray_ptr,false,ladarimage::p_data);
   twoDarray* frot_twoDarray_ptr=new twoDarray(fexpand_twoDarray_ptr);
   fexpand_twoDarray_ptr->rotate_about_center(
      -bbox_theta,frot_twoDarray_ptr,0,true);
   delete fexpand_twoDarray_ptr;

// Compute horizontal and vertical profiles:

//   bool plot_profile=true;
   bool plot_profile=false;
   double horizontal_profile[frot_twoDarray_ptr->get_mdim()];
   double vertical_profile[frot_twoDarray_ptr->get_ndim()];
   unsigned int px_min,px_max,py_min,py_max;
   int jmax,kmax;
   double horiz_profile_median,vert_profile_median;

   imagenumber=n;
   generate_horizontal_profile(
      frot_twoDarray_ptr->get_xlo(),frot_twoDarray_ptr->get_xhi(),
      frot_twoDarray_ptr->get_ylo(),frot_twoDarray_ptr->get_yhi(),
      frot_twoDarray_ptr,px_min,px_max,kmax,
      horizontal_profile,horiz_profile_median,plot_profile);
   generate_vertical_profile(
      frot_twoDarray_ptr->get_xlo(),frot_twoDarray_ptr->get_xhi(),
      frot_twoDarray_ptr->get_ylo(),frot_twoDarray_ptr->get_yhi(),
      frot_twoDarray_ptr,py_min,py_max,jmax,vertical_profile,
      vert_profile_median,plot_profile);

//   cout << "horiz_profile_median = " << horiz_profile_median 
//        << " vert_profile_median = " << vert_profile_median << endl;

   double minimum_x,maximum_x,minimum_y,maximum_y;
   double edge_fraction_of_median=0.5;
   imagefunc::find_horiz_vert_profile_edges(
      edge_fraction_of_median,jmax,kmax,px_min,px_max,py_min,py_max,
      horiz_profile_median,vert_profile_median,
      horizontal_profile,vertical_profile,
      frot_twoDarray_ptr,minimum_x,maximum_x,minimum_y,maximum_y);
//   cout << "min_x = " << minimum_x << " max_x = " << maximum_x << endl;
//   cout << "min_y = " << minimum_y << " max_y = " << maximum_y << endl;

// Don't instantiate building bbox if it's ridiculously undersized:

   const double min_bbox_edge_length=5;	// meters
   if (maximum_x-minimum_x > min_bbox_edge_length && 
       maximum_y-minimum_y > min_bbox_edge_length)
   {
      parallelogram* bbox_ptr=new parallelogram(
         maximum_x-minimum_x,maximum_y-minimum_y);
      double bbox_score=optimize_building_bbox_orientation(
         n,bbox_ptr,frot_twoDarray_ptr);
//      double bbox_score=building_bbox_fit_score(n,bbox_ptr,frot_twoDarray_ptr) ;

      const double max_bad_pixel_ratio=0.25;
      if (bbox_score < max_bad_pixel_ratio)
      {
         cout << "Building bbox looks OK" << endl;
//         drawfunc::draw_polygon(
//            *bbox_ptr,colorfunc::white,frot_twoDarray_ptr);
//         writeimage("rotated_bldg_"+stringfunc::number_to_string(n),
//                    frot_twoDarray_ptr,false,ladarimage::p_data);
         bbox_ptr->rotate(Zero_vector,0,0,bbox_theta);
         bbox_ptr->translate(get_building_ptr(n)->get_posn());
         get_building_ptr(n)->set_bbox_ptr(bbox_ptr);
      }
      else
      {
         cout << "Building bbox is probably BAD" << endl;
      }
      outputfunc::newline();
   }
   delete frot_twoDarray_ptr;
}

// ---------------------------------------------------------------------
// This overloaded version of compute_building_bbox() takes in a
// binary mask for some building within *ftwoDarray_ptr as well as its
// world-space COM.  It first rotates the mask by -bbox_theta so that
// it aligns reasonably well with the XY axes.  This method next
// computes the horizontal and vertical profiles of the rotated mask.
// After computing the edges of the profiles, a 3D bounding box is
// generated whose size, orientation and position are set so as to
// (hopefully) snuggly encompass the original building's binary mask.

parallelogram* urbanimage::compute_building_bbox(
   const threevector& COM,twoDarray const *ftwoDarray_ptr,double bbox_theta,
   double edge_fraction_of_median)
{
//   cout << "inside urbanimage::compute_building_bbox()" << endl;
//   cout << "bbox_theta = " << bbox_theta*180/PI << endl;
//   cout << "edge_fraction_of_median = " << edge_fraction_of_median << endl;

   twoDarray* frot_twoDarray_ptr=new twoDarray(ftwoDarray_ptr);
   ftwoDarray_ptr->copy(frot_twoDarray_ptr);
   frot_twoDarray_ptr->rotate(COM,-bbox_theta);
   frot_twoDarray_ptr->translate(frot_twoDarray_ptr->center_point()-COM);

//   texture_rectangle* rot_texture_rectangle_ptr=new texture_rectangle(
//      frot_twoDarray_ptr->get_mdim(),frot_twoDarray_ptr->get_ndim(),1,3,NULL);
//   rot_texture_rectangle_ptr->initialize_twoDarray_image(frot_twoDarray_ptr);
//   rot_texture_rectangle_ptr->write_curr_frame("rot_p_roof_binary.png");
//   delete rot_texture_rectangle_ptr;

// Compute horizontal and vertical profiles:

//   bool plot_profile=true;
   bool plot_profile=false;
   double horizontal_profile[frot_twoDarray_ptr->get_mdim()];
   double vertical_profile[frot_twoDarray_ptr->get_ndim()];
   unsigned int px_min,px_max,py_min,py_max;
   int jmax,kmax;
   double horiz_profile_median,vert_profile_median;

   generate_horizontal_profile(
      frot_twoDarray_ptr->get_xlo(),frot_twoDarray_ptr->get_xhi(),
      frot_twoDarray_ptr->get_ylo(),frot_twoDarray_ptr->get_yhi(),
      frot_twoDarray_ptr,px_min,px_max,kmax,
      horizontal_profile,horiz_profile_median,plot_profile);
//   cout << "horiz_profile_median = " << horiz_profile_median << endl;
   generate_vertical_profile(
      frot_twoDarray_ptr->get_xlo(),frot_twoDarray_ptr->get_xhi(),
      frot_twoDarray_ptr->get_ylo(),frot_twoDarray_ptr->get_yhi(),
      frot_twoDarray_ptr,py_min,py_max,jmax,vertical_profile,
      vert_profile_median,plot_profile);
//   cout << "vert_profile_median = " << vert_profile_median << endl;

   double minimum_x,maximum_x,minimum_y,maximum_y;
   imagefunc::find_horiz_vert_profile_edges(
      edge_fraction_of_median,jmax,kmax,px_min,px_max,py_min,py_max,
      horiz_profile_median,vert_profile_median,
      horizontal_profile,vertical_profile,
      frot_twoDarray_ptr,minimum_x,maximum_x,minimum_y,maximum_y);
//   cout << "min_x = " << minimum_x << " max_x = " << maximum_x << endl;
//   cout << "min_y = " << minimum_y << " max_y = " << maximum_y << endl;

// Don't instantiate building bbox if it's ridiculously undersized:

   parallelogram* bbox_ptr=NULL;
   
   const double min_bbox_edge_length=5;	// meters
   if (maximum_x-minimum_x > min_bbox_edge_length && 
       maximum_y-minimum_y > min_bbox_edge_length)
   {
      bbox_ptr=new parallelogram(maximum_x-minimum_x,maximum_y-minimum_y);
      bbox_ptr->rotate(Zero_vector,0,0,bbox_theta);
      bbox_ptr->translate(COM);
//      cout << "bbox = " << *bbox_ptr << endl;
   }
   delete frot_twoDarray_ptr;
   return bbox_ptr;
}

// ---------------------------------------------------------------------
// Method building_bbox_fit_score

double urbanimage::building_bbox_fit_score(
   int n,parallelogram const *bbox_ptr,twoDarray const *fbinary_twoDarray_ptr)
{
   double maximum_x=bbox_ptr->get_width()/2.0;
   double minimum_x=-maximum_x;
   double maximum_y=bbox_ptr->get_length()/2.0;
   double minimum_y=-maximum_y;

   const double zmin=0.5;
   int npixels_inside_bbox;
   int n_lit_interior_pixels=
      imagefunc::strict_count_pixels_above_zmin_inside_bbox(
         minimum_x,minimum_y,maximum_x,maximum_y,zmin,
         fbinary_twoDarray_ptr,npixels_inside_bbox);
   int n_unlit_interior_pixels=npixels_inside_bbox-n_lit_interior_pixels;
   int ntotal_lit_pixels=
      imagefunc::count_pixels_above_zmin(zmin,fbinary_twoDarray_ptr);
   int n_lit_exterior_pixels=ntotal_lit_pixels-n_lit_interior_pixels;

//   double unlit_intensity=0.3;
//   double lit_intensity=0.6;
//   twoDarray* flit_twoDarray_ptr=new twoDarray(fbinary_twoDarray_ptr);
//   fbinary_twoDarray_ptr->copy(flit_twoDarray_ptr);
//   drawfunc::count_and_color_lit_pixels_inside_bbox(
//      minimum_x,minimum_y,maximum_x,maximum_y,
//      zmin,lit_intensity,unlit_intensity,flit_twoDarray_ptr,
//      npixels_inside_bbox);
//   writeimage("lit_pixels_"+stringfunc::number_to_string(n),
//              flit_twoDarray_ptr,false,ladarimage::p_data);
//   delete flit_twoDarray_ptr;

   int nbad_pixels=n_unlit_interior_pixels+n_lit_exterior_pixels;
   double bad_pixel_ratio=double(nbad_pixels)/double(npixels_inside_bbox);
//   double lit_pixel_ratio=
//      double(n_lit_interior_pixels)/double(npixels_inside_bbox);
//   outputfunc::newline();
//   cout << "bbox n = " << n << endl;
//   cout << "n_lit_interior_pixels = " << n_lit_interior_pixels << endl;
//   cout << "n_unlit_interior_pixels = " << n_unlit_interior_pixels << endl;
//   cout << "n_lit_exterior_pixels = " << n_lit_exterior_pixels << endl;
//   cout << "nbad_pixels = " << nbad_pixels << endl;
//   cout << "npixels_in_bbox = " << npixels_inside_bbox << endl;
//   cout << "lit_pixel_ratio = " << lit_pixel_ratio << endl;
//   cout << "bad_pixel_ratio = " << bad_pixel_ratio << endl;
//   outputfunc::newline();

   return bad_pixel_ratio;
}

// ---------------------------------------------------------------------
// Method optimize_building_bbox_orientation

double urbanimage::optimize_building_bbox_orientation(
   int n,parallelogram* bbox_ptr,twoDarray const *fbinary_twoDarray_ptr)
{
// First make copy of initial best estimate for building bounding box:

   const double theta_max=16*PI/180;
   const double theta_min=-theta_max;
   const double dtheta=2*PI/180;

   unsigned int nbins=basic_math::round((theta_max-theta_min)/dtheta)+1;
   double best_bbox_score=POSITIVEINFINITY;

   double best_theta=0;
   twoDarray* frotprime_twoDarray_ptr=new twoDarray(fbinary_twoDarray_ptr);
   for (unsigned int i=0; i<nbins; i++)
   {
      double theta=theta_min+i*dtheta;
      
      frotprime_twoDarray_ptr->clear_values();
      fbinary_twoDarray_ptr->rotate_about_center(
         -theta,frotprime_twoDarray_ptr,0,true);

//      writeimage("frotprime_"+stringfunc::number_to_string(i),
//                 frotprime_twoDarray_ptr,false,ladarimage::p_data);

      double curr_score=building_bbox_fit_score(
         n,bbox_ptr,frotprime_twoDarray_ptr) ;

      if (curr_score < best_bbox_score)
      {
         best_theta=theta;
         best_bbox_score=curr_score;
      }

//      cout << "bldg n = " << n << " theta = " << theta*180/PI
//           << " score = " << curr_score << " best_score = " << best_bbox_score
//           << " best_theta = " << best_theta*180/PI << endl;

   } // loop over index i labeling angle theta
   delete frotprime_twoDarray_ptr;

   bbox_ptr->rotate(Zero_vector,0,0,best_theta);
   return best_bbox_score;
}

// ==========================================================================
// Building 3D parallelepiped methods
// ==========================================================================

// Member function construct_building_wireframe_models loops over all
// entries within the buildings network.  This high-level method
// constructs oriented 3D boxes which may include rooftops for each
// region within the decomposed orthogonal contour footprint.

void urbanimage::construct_building_wireframe_models(
   twoDarray const *ztwoDarray_ptr,twoDarray const *features_twoDarray_ptr)
{
   outputfunc::write_banner("Constructing building wireframe models:");   

   for (Mynode<int>* currnode_ptr=buildings_network_ptr->
           get_entries_list_ptr()->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int n=currnode_ptr->get_data(); // contour number
      cout << n << " " << flush; 
      building* b_ptr=get_building_ptr(n);
      twoDarray* binary_mask_twoDarray_ptr=
         generate_twoDarray_for_centered_building();

      urbanfunc::construct_building_wireframe_model(
         b_ptr,ztwoDarray_ptr,features_twoDarray_ptr,
         binary_mask_twoDarray_ptr);
      delete binary_mask_twoDarray_ptr;
   } // loop over entries in buildings network

   outputfunc::newline();
}

// ---------------------------------------------------------------------
// Method generate_rooftop_pixels_profiles estimates the spatial
// extents of a building's set of rooftop voxels after the set's basic
// symmetry directions have been determined.  It first forms a binary
// mask for the nth building's rooftop footprint on the ground ( = xy
// plane).  The method rotates this footprint about the z axis running
// through its center by -min_theta in order to align the footprint
// with the x and y axes.  The method subsequently forms the
// horizontal and vertical profiles of the rotated footprint.  After
// converting the profiles into probability distributions, this method
// returns the values x_min and x_max [y_min and y_max] corresponding
// to the 3% and 97% points within the horizontal [vertical]
// cumulative probability distributions.

void urbanimage::generate_rooftop_pixels_profiles(
   int n,double min_theta,twoDarray*& binary_mask_twoDarray_ptr,
   double& x_min,double& x_max,double& y_min,double& y_max)
{
// Generate mask for rooftop pixels' x-y footprint.  Then rotate
// binary mask so that footprint is aligned with the xy axes:

   binary_mask_for_centered_building(n,binary_mask_twoDarray_ptr);
   twoDarray* fexpand_twoDarray_ptr=binary_mask_twoDarray_ptr->
      expand_for_rotation(0,true);
//   writeimage(
//      "expanded_binary_mask",fexpand_twoDarray_ptr,false,ladarimage::p_data);

   twoDarray* frot_twoDarray_ptr=new twoDarray(fexpand_twoDarray_ptr);
   fexpand_twoDarray_ptr->rotate_about_center(
      -min_theta,frot_twoDarray_ptr,0,true);
   delete fexpand_twoDarray_ptr;
//   writeimage(
//      "rotated_binary_mask",frot_twoDarray_ptr,false,ladarimage::p_data);
   
//   bool plot_profile=true;
   bool plot_profile=false;
   unsigned int mdim=frot_twoDarray_ptr->get_mdim();
   unsigned int ndim=frot_twoDarray_ptr->get_ndim();
   double *horizontal_profile,*vertical_profile;
   new_clear_array(horizontal_profile,mdim);
   new_clear_array(vertical_profile,ndim);
   generate_horizontal_profile(
      frot_twoDarray_ptr->get_xlo(),frot_twoDarray_ptr->get_xhi(),
      frot_twoDarray_ptr->get_ylo(),frot_twoDarray_ptr->get_yhi(),
      frot_twoDarray_ptr,horizontal_profile,plot_profile);
   generate_vertical_profile(
      frot_twoDarray_ptr->get_xlo(),frot_twoDarray_ptr->get_xhi(),
      frot_twoDarray_ptr->get_ylo(),frot_twoDarray_ptr->get_yhi(),
      frot_twoDarray_ptr,vertical_profile,plot_profile);

   imagefunc::invert_vertical_profile(ndim,vertical_profile);

   prob_distribution prob_x(
      mdim,frot_twoDarray_ptr->get_xlo(),frot_twoDarray_ptr->get_xhi(),
      horizontal_profile);
   prob_distribution prob_y(
      ndim,frot_twoDarray_ptr->get_ylo(),frot_twoDarray_ptr->get_yhi(),
      vertical_profile);

   delete [] horizontal_profile;
   delete [] vertical_profile;

   prob_y.set_densityfilenamestr(imagedir+"prob_y_density.meta");
   prob_y.set_cumulativefilenamestr(imagedir+"prob_y_cumulative.meta");
   prob_y.set_xlabel("Row integral");
//   prob_y.writeprobdists();

   const double prob_lo=0.03;
   const double prob_hi=0.97;
   x_min=prob_x.find_x_corresponding_to_pcum(prob_lo);
   x_max=prob_x.find_x_corresponding_to_pcum(prob_hi);
   y_min=prob_y.find_x_corresponding_to_pcum(prob_lo);
   y_max=prob_y.find_x_corresponding_to_pcum(prob_hi);
   
//   cout << "x_max = " << x_max << " x_min = " << x_min << endl;
//   cout << "y_max = " << y_max << " y_min = " << y_min << endl;
}

// ---------------------------------------------------------------------
// Method print_building_data loops over all of the entries within the
// buildings network and prints out parallelpiped side face plus
// rooftop informatoin for each oriented box making up each building.

void urbanimage::print_building_data()
{
   for (Mynode<int>* currnode_ptr=buildings_network_ptr->
           get_entries_list_ptr()->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int n=currnode_ptr->get_data();
      cout << "====================================================" << endl;
      cout << "Building ID = " << n << endl;
      cout << *get_building_ptr(n) << endl;
   } // loop over nodes in buildings network entries list
}

// ---------------------------------------------------------------------
// Method compute_tallest_building_height loops over all nodes within
// the buildings network.  It returns the tallest building's height.
// We cooked up this little method in Feb 2005 in order to nicely
// display buildings and road networks using z-information...

double urbanimage::compute_tallest_building_height()
{
   double max_bldg_height=-1;
   for (Mynode<int>* currnode_ptr=buildings_network_ptr->
           get_entries_list_ptr()->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int n=currnode_ptr->get_data();
      get_building_ptr(n)->find_max_height();
      max_bldg_height=basic_math::max(max_bldg_height,get_building_ptr(n)->
                          get_max_height());
   } // loop over nodes in buildings network entries list
   return max_bldg_height;
}

// ==========================================================================
// Shadow computation methods
// ==========================================================================

// Method compute_building_shadows loops over all entries within the
// buildings network.  It projects each side face of each building's
// parallelepiped onto the flattened z=0 ground plane.  The projection
// of the side face in the xy plane is then colored equal to
// featurefunc::shadow_sentinel_value within output
// *shadows_twoDarray_ptr via the fast
// color_convex_quadrilateral_interior() method.  By the end of this
// method, all points which lie within shadow of some building are
// marked within *shadows_twoDarray_ptr.

void urbanimage::compute_building_shadows(
   const threevector& ray_hat,twoDarray const *ztwoDarray_ptr,
   twoDarray const *features_twoDarray_ptr,
   twoDarray* shadows_twoDarray_ptr) 
{
   outputfunc::write_banner("Computing building shadows:");

   polygon xy_sideface_projection;
   for (Mynode<int>* currnode_ptr=buildings_network_ptr->
           get_entries_list_ptr()->get_start_ptr(); currnode_ptr != NULL; 
        currnode_ptr=currnode_ptr->get_nextptr())
   {
      int n=currnode_ptr->get_data();
      cout << n << " " << flush;
      Linkedlist<oriented_box*>* box_list_ptr=get_building_ptr(n)->
         get_box_list_ptr();
      for (Mynode<oriented_box*>* box_node_ptr=box_list_ptr->get_start_ptr(); 
           box_node_ptr != NULL; box_node_ptr=box_node_ptr->get_nextptr())
      {
         oriented_box* oriented_box_ptr=box_node_ptr->get_data();
         for (unsigned int i=0; i<oriented_box_ptr->get_n_sidefaces(); i++)
         {
            if (oriented_box_ptr->get_sideface(i).
                projection_into_xy_plane_along_ray(
                   ray_hat,xy_sideface_projection))
            {
               drawfunc::color_convex_quadrilateral_interior(
                  xy_sideface_projection,featurefunc::shadow_sentinel_value,
                  shadows_twoDarray_ptr);
            }
         } // loop over index i labeling side faces
      } // loop over oriented box subcomponents of nth building
   } // loop over nodes in buildings network entries list
   outputfunc::newline();
}

// ---------------------------------------------------------------------
// Method compute_tree_shadows loops over all entries within
// *trees_network_ptr.  It computes individual side faces for each tree
// cluster contour.  This method then projects each side face onto the
// flattened z=0 ground plane.  The projection of the side face in the
// xy plane is then colored equal to featurefunc::shadow_sentinel_value
// within output *shadows_twoDarray_ptr via the fast
// color_convex_quadrilateral_interior() method.  By the end of this
// method, all points which lie within shadow of some tree or bush are
// marked within *shadows_twoDarray_ptr.

void urbanimage::compute_tree_shadows(
   Network<tree_cluster*> const *trees_network_ptr,
   const threevector& ray_hat,
   twoDarray const *ztwoDarray_ptr,twoDarray const *features_twoDarray_ptr,
   twoDarray* shadows_twoDarray_ptr) const
{
   outputfunc::write_banner("Computing tree shadows:");
//   static threevector v[4];
   static vector<threevector> v(4);
   polygon xy_sideface_projection;

   for (const Mynode<int>* currnode_ptr=trees_network_ptr->
           get_entries_list_ptr()->get_start_ptr(); currnode_ptr != NULL; 
        currnode_ptr=currnode_ptr->get_nextptr())
   {
      int n=currnode_ptr->get_data();
      cout << n << " " << flush;
      contour* curr_contour_ptr=trees_network_ptr->
         get_site_data_ptr(n)->get_contour_ptr();

// In order to cut down on ray tracing time, we generate side faces
// whose contour vertices are spaced apart by contour_vertex_skip steps:

      int vertex_skip=1;	
      for (unsigned int i=0; i<curr_contour_ptr->get_nvertices(); i += vertex_skip)
      {
         int next_i=modulo(i+vertex_skip,curr_contour_ptr->get_nvertices());
         v[0]=curr_contour_ptr->get_vertex(next_i).first;
         v[1]=curr_contour_ptr->get_vertex(i).first;
         v[2]=threevector(v[1].get(0),v[1].get(1));
         v[3]=threevector(v[0].get(0),v[0].get(1));
         polygon side_face(v);
         if (side_face.projection_into_xy_plane_along_ray(
            ray_hat,xy_sideface_projection))
         {
            drawfunc::color_convex_quadrilateral_interior(
               xy_sideface_projection,featurefunc::shadow_sentinel_value,
               shadows_twoDarray_ptr);
         }
      } // loop over index i labeling contour vertices
   } // loop over nodes in trees network entries list
   outputfunc::newline();
}

// ---------------------------------------------------------------------
// Method reset_nonground_feature_values takes in
// *features_twoDarray_ptr along with shadow information within
// *shadows_twoDarray_ptr.  For purposes of illustrating satellite
// communication obstruction, we are primarily interested in GROUND
// regions that lie within the shadows of tall buildings and/or trees.
// So this method resets all pixels within *shadows_twoDarray_ptr
// which do not correspond to either grass or asphalt back to their
// original unshadowed feature values (even if they are shadowed by
// taller non-ground points).

void urbanimage::reset_nonground_feature_values(
   twoDarray const *features_twoDarray_ptr,twoDarray* shadows_twoDarray_ptr) 
   const
{
   outputfunc::write_banner("Resetting non-ground feature values:");

   for (unsigned int px=0; px<shadows_twoDarray_ptr->get_mdim(); px++)
   {
      for (unsigned int py=0; py<shadows_twoDarray_ptr->get_ndim(); py++)
      {
         double curr_f=features_twoDarray_ptr->get(px,py);
         if (nearly_equal(curr_f,featurefunc::tree_sentinel_value))
         {
            shadows_twoDarray_ptr->put(
               px,py,featurefunc::tree_sentinel_value);
         }
         else if (nearly_equal(curr_f,featurefunc::building_sentinel_value))
         {
            shadows_twoDarray_ptr->put(
               px,py,featurefunc::building_sentinel_value);
         }
      } // loop over py index
   } // loop over px index
}

// ==========================================================================
// Road network computation methods
// ==========================================================================

// Member function generate_roadpoints_network dynamically generates
// member network *roadpoints_network_ptr whose entries correspond
// to roadpoint objects.  Each roadpoint is assigned a unique ID
// number, and its position is saved within its own members.

// The initial set of roadpoints is taken to be the vertices of all
// the Voronoi region polygons.  Connectivity information between all
// of the Voronoi vertices is stored within the *roadlink_list_ptr
// linked list for each roadpoint in the hashtable.  The ID numbers
// for the buildings located within the Voronoi regions which meet at
// a Voronoi polygon vertex are also stored within the
// *nearby_bldg_list_ptr linked list for each roadpoint.

void urbanimage::generate_roadpoints_network(twoDarray const *ztwoDarray_ptr)
{
   outputfunc::write_banner("Generating roadpoints network:");

   Hashtable<threevector>* tmp_roadpoints_hashtable_ptr=
      new Hashtable<threevector>(10*get_n_buildings());

   roadpoints_network_ptr=new Network<roadpoint*>(10*get_n_buildings());

   int n_roadpoints=0;
   for (Mynode<int>* currnode_ptr=buildings_network_ptr->
           get_entries_list_ptr()->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int n=currnode_ptr->get_data();
      cout << n << " " << flush; 
      polygon* curr_voronoi_region_ptr=
         get_building_ptr(n)->get_voronoi_region_ptr();
      for (unsigned int i=0; i<curr_voronoi_region_ptr->get_nvertices(); i++)
      {
         threevector curr_vertex=curr_voronoi_region_ptr->get_vertex(i);

         if (tmp_roadpoints_hashtable_ptr->data_in_hashtable(curr_vertex)
             ==NULL)
         {
            tmp_roadpoints_hashtable_ptr->insert_key(
               n_roadpoints,curr_vertex);
            roadpoint* curr_newroadpoint_ptr=new roadpoint(
               n_roadpoints,curr_vertex);

// Treat any roadpoint lying outside the ladar data bounding box as a
// "point at infinity":

            if (!data_bbox_ptr->point_inside_polygon(curr_vertex))
            {
               curr_newroadpoint_ptr->set_at_infinity(true);
            }
            roadpoints_network_ptr->insert_site(
               n_roadpoints,Site<roadpoint*>(
                  curr_newroadpoint_ptr));
            n_roadpoints++;
         }
      } // loop over index i labeling polygon vertex
   } // loop over nodes in buildings network entries list
   outputfunc::newline();
   
// Next establish links between roadpoints and nearby buildings as
// well as between different roadpoints within road network:

   for (Mynode<int>* currnode_ptr=buildings_network_ptr->
           get_entries_list_ptr()->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int n=currnode_ptr->get_data();
      cout << n << " " << flush; 
      polygon* curr_voronoi_region_ptr=
         get_building_ptr(n)->get_voronoi_region_ptr();
      unsigned int nvertices=curr_voronoi_region_ptr->get_nvertices();
      for (unsigned int i=0; i<nvertices; i++)
      {
         threevector curr_vertex(curr_voronoi_region_ptr->get_vertex(i));
         Mynode<threevector>* currnode_posn_ptr=
            tmp_roadpoints_hashtable_ptr->data_in_hashtable(curr_vertex);
         int key=currnode_posn_ptr->get_ID();

         Site<roadpoint*>* currsite_ptr=roadpoints_network_ptr->
            get_site_ptr(key);
         int site_ID=currsite_ptr->get_data()->get_ID();
//         cout << "bldg n = " << n 
//              << " roadpoint ID = " << site_ID << endl;

// Add current roadpoint as a nearby roadpoint to building n's
// *nearby_roadpoint_list_ptr linked list:

         add_roadpoint_to_bldg_list(site_ID,n);

// Add building n as a nearby building to current roadpoint's
// *nearby_bldg_list_ptr linked list:

         add_bldg_to_roadpoint_list(n,site_ID);

// Forward direction roadpoint neighbor:
         
         threevector next_vertex=curr_voronoi_region_ptr->
            get_vertex(modulo(i+1,nvertices));
         Mynode<threevector>* nextnode_posn_ptr=
            tmp_roadpoints_hashtable_ptr->data_in_hashtable(next_vertex);
         int next_key=nextnode_posn_ptr->get_ID();
         Site<roadpoint*>* nextsite_ptr=roadpoints_network_ptr->
            get_site_ptr(next_key);
         int nextsite_ID=nextsite_ptr->get_data()->get_ID();

// Backward direction roadpoint neighbor:

         threevector prev_vertex=curr_voronoi_region_ptr->
            get_vertex(modulo(i-1,nvertices));
         Mynode<threevector>* prevnode_posn_ptr=
            tmp_roadpoints_hashtable_ptr->data_in_hashtable(prev_vertex);
         int prev_key=prevnode_posn_ptr->get_ID();
         Site<roadpoint*>* prevsite_ptr=roadpoints_network_ptr->
            get_site_ptr(prev_key);
         int prevsite_ID=prevsite_ptr->get_data()->get_ID();

         roadpoints_network_ptr->add_symmetric_link(site_ID,nextsite_ID);
         roadpoints_network_ptr->add_symmetric_link(site_ID,prevsite_ID);

//         cout << "nodeID = " << node_ID << " nextnode_ID = " << nextnode_ID
//              << " prevnode_ID = " << prevnode_ID << endl;
         
      } // loop over index i labeling polygon vertex
   } // loop over nodes in buildings network entries list
   outputfunc::newline();
   delete tmp_roadpoints_hashtable_ptr;
}

// ---------------------------------------------------------------------
// Method delete_roadlinks_too_close_to_buildings conducts a search
// over all roadlinks between roadpoints r and q.  If the distance
// from a roadlink to a building is less than
// min_allowed_bldg_dist_to_roadlink, q is removed from roadpoint r's
// linked roadlink list and r is removed from q's linked roadlink
// list.  This member function is meant to be high-level and
// user-friendly.

void urbanimage::delete_roadlinks_too_close_to_buildings(
   twoDarray const *just_asphalt_twoDarray_ptr)
{
   linesegment closest_bldg_roadlink_linesegment;
   
   for (Mynode<int>* currnode_ptr=roadpoints_network_ptr->
           get_entries_list_ptr()->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int r=currnode_ptr->get_data(); // roadpoint number
      roadpoint* curr_roadpoint_ptr=get_roadpoint_ptr(r);
      Site<roadpoint*>* curr_roadpoint_site_ptr=get_roadpoint_site_ptr(r);

      Linkedlist<netlink>* curr_roadlink_list_ptr
         =curr_roadpoint_site_ptr->get_netlink_list_ptr();
      if (curr_roadlink_list_ptr != NULL)
      {
         Mynode<netlink>* roadlink_node_ptr=
            curr_roadlink_list_ptr->get_start_ptr();

         while (roadlink_node_ptr != NULL)
         {
            Mynode<netlink>* next_roadlink_node_ptr=
               roadlink_node_ptr->get_nextptr();
            int q=roadlink_node_ptr->get_data().get_ID();
            roadpoint* neighbor_roadpoint_ptr=get_roadpoint_ptr(q);
            double bldg_dist_to_roadlink=
               min_bldg_distance_to_roadlink(
                  curr_roadpoint_site_ptr->get_data(),
                  neighbor_roadpoint_ptr,
                  closest_bldg_roadlink_linesegment);

// Do NOT delete roadlink if almost all of it lies on top of asphalt:

            const double max_asphalt_length_frac=0.95;
            linesegment edge(curr_roadpoint_ptr->get_posn(),
                             neighbor_roadpoint_ptr->get_posn());
            double asphalt_length_frac=
               urbanfunc::frac_link_length_over_asphalt(
                  edge,just_asphalt_twoDarray_ptr);

            const double min_allowed_bldg_dist_to_roadlink=4.5;  // meters
            if (bldg_dist_to_roadlink < min_allowed_bldg_dist_to_roadlink &&
                asphalt_length_frac < max_asphalt_length_frac)
            {

// Delete road link between roadpoints r and q:
                  
               roadpoints_network_ptr->delete_symmetric_link(r,q);
            }
            roadlink_node_ptr=next_roadlink_node_ptr;
         } // while roadlink_node_ptr != NULL 
      } // curr_roadlink_list_ptr != NULL conditional
   } // loop over sites within roadpoints network
}

// ---------------------------------------------------------------------
// Method min_bldg_distance_to_roadlink takes in two roadpoints
// *roadpoint1_ptr and roadpoint2_ptr.  It returns the minimum
// distance between the line segment connecting these two roadpoints
// and all of the buildings located nearby this segment.

double urbanimage::min_bldg_distance_to_roadlink(
   roadpoint* roadpoint1_ptr,roadpoint* roadpoint2_ptr,
   linesegment& closest_bldg_roadlink_linesegment)
{
   double min_distance=POSITIVEINFINITY;
   threevector closest_point_on_roadlink;

   Linkedlist<int>* bldg_list_ptr=roadpoint1_ptr->get_nearby_bldg_list_ptr();
   for (Mynode<int>* currnode_bldgID_ptr=bldg_list_ptr->get_start_ptr();
        currnode_bldgID_ptr != NULL; currnode_bldgID_ptr=
           currnode_bldgID_ptr->get_nextptr())
   {
      building* curr_bldg_ptr=get_building_ptr(
         currnode_bldgID_ptr->get_data());
      double curr_distance=roadlink_distance_to_bldg(
         roadpoint1_ptr,roadpoint2_ptr,curr_bldg_ptr,
         closest_point_on_roadlink);
      if (curr_distance < min_distance)
      {
         min_distance=curr_distance;
         closest_bldg_roadlink_linesegment=
            linesegment(closest_point_on_roadlink,curr_bldg_ptr->get_posn());
      }
   }
   
   bldg_list_ptr=roadpoint2_ptr->get_nearby_bldg_list_ptr();
   for (Mynode<int>* currnode_bldgID_ptr=bldg_list_ptr->get_start_ptr();
        currnode_bldgID_ptr != NULL; currnode_bldgID_ptr=
           currnode_bldgID_ptr->get_nextptr())
   {
      building* curr_bldg_ptr=get_building_ptr(
         currnode_bldgID_ptr->get_data());
      double curr_distance=roadlink_distance_to_bldg(
         roadpoint1_ptr,roadpoint2_ptr,curr_bldg_ptr,
         closest_point_on_roadlink);
      if (curr_distance < min_distance)
      {
         min_distance=curr_distance;
         closest_bldg_roadlink_linesegment=
            linesegment(closest_point_on_roadlink,curr_bldg_ptr->get_posn());
      }
   }
   
   return min_distance;
}

// ---------------------------------------------------------------------
// Method roadlink_distance_to_bldg takes in roadpoints
// *roadpoint1_ptr and *roadpoint2_ptr.  It instantiates a line
// segment between these two roadpoints.  This method also takes in
// building *bldg_ptr.  It returns the distance between the building's
// center-of-mass and the linesegment road link.

double urbanimage::roadlink_distance_to_bldg(
   roadpoint* roadpoint1_ptr,roadpoint* roadpoint2_ptr,
   building* bldg_ptr,threevector& closest_point_on_roadlink)
{
   threevector r1(roadpoint1_ptr->get_posn());
   threevector r2(roadpoint2_ptr->get_posn());
   linesegment l(r1,r2);
   
// Loop over building's footprint contour and locate contour vertex
// which is closest to linesegment l:

   contour* bldg_contour_ptr=bldg_ptr->get_contour_ptr();

// On 8/2/04, we empirically observed that deleting roadlinks which
// pass too close to building contours fails if the building is very
// large compared to a canonical suburban home.  Representxing such
// buildings by their COMs in Voronoi diagrams becomes a poor
// approximation, and Voronoi edge can lie very close to the
// building's contour.  In this case, we revert to using the
// building's center-of-mass to set the roadlink distance to the
// building...

   double min_distance=POSITIVEINFINITY;
   const double max_contour_perimeter=100;	// meters
   if (bldg_contour_ptr->get_perimeter() < max_contour_perimeter)
   {
      for (unsigned int i=0; i<bldg_contour_ptr->get_nvertices(); i++)
      {
         linesegment contour_edge(bldg_contour_ptr->get_edge(i));
         threevector closest_point_on_l;
         double curr_distance=l.point_to_line_segment_distance(
            contour_edge.get_v1(),closest_point_on_l);
         if (curr_distance < min_distance)
         {
            min_distance=curr_distance;
            closest_point_on_roadlink=closest_point_on_l;
         }
      }
   }
   else
   {
      min_distance=l.point_to_line_segment_distance(
         bldg_ptr->get_posn(),closest_point_on_roadlink);
   }
   
   return min_distance;
}

// ---------------------------------------------------------------------
// Method delete_roadlinks_passing_thru_buildings conducts a search
// over all roadlinks between roadpoints r and q.  If a roadlink
// passes through a building, q is removed from roadpoint r's linked
// roadlink list and r is removed from q's linked roadlink list.  This
// member function is meant to be high-level and user-friendly.

void urbanimage::delete_roadlinks_passing_thru_buildings(
   twoDarray const *features_twoDarray_ptr)
{
//   string just_bldgs_filename=imagedir+"just_bldgs.xyzp";   
   twoDarray* just_bldgs_twoDarray_ptr=featurefunc::cull_feature_pixels(
      featurefunc::building_sentinel_value,0.0,features_twoDarray_ptr);
//   ladarfunc::write_xyzp_data(
//      ztwoDarray_ptr,just_bldgs_twoDarray_ptr,just_bldgs_filename);
//   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
//      ztwoDarray_ptr,just_bldgs_filename);

   for (Mynode<int>* currnode_ptr=roadpoints_network_ptr->
           get_entries_list_ptr()->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int r=currnode_ptr->get_data(); // roadpoint number
      Site<roadpoint*>* curr_roadpoint_site_ptr=get_roadpoint_site_ptr(r);
      Linkedlist<netlink>* curr_roadlink_list_ptr
         =curr_roadpoint_site_ptr->get_netlink_list_ptr();

      if (curr_roadlink_list_ptr != NULL)
      {
         Mynode<netlink>* roadlink_node_ptr=
            curr_roadlink_list_ptr->get_start_ptr();

         while (roadlink_node_ptr != NULL)
         {
            Mynode<netlink>* next_roadlink_node_ptr=
               roadlink_node_ptr->get_nextptr();
            int q=roadlink_node_ptr->get_data().get_ID();
            roadpoint* neighbor_roadpoint_ptr=get_roadpoint_ptr(q);

            if (urbanfunc::link_passes_through_building(
               curr_roadpoint_site_ptr->get_data(),
               neighbor_roadpoint_ptr,just_bldgs_twoDarray_ptr))
            {

// Delete road link between roadpoints r and q:
                  
               roadpoints_network_ptr->delete_symmetric_link(r,q);
            }
            roadlink_node_ptr=next_roadlink_node_ptr;
         }
      }
   } // loop over sites in roadpoints network
   delete just_bldgs_twoDarray_ptr;
}

// ---------------------------------------------------------------------
// Member function prune_roadpoints_at_infinity scans over all
// roadpoints which are located outside the working region of the
// ladar image.  This method systematically deletes road links between
// two different points at infinity.  It also deletes any road link
// between a point at infinity and a finite point lying outside the
// ladar data bounding box.  A subsequent call to member function
// delete_lonely_roadpoints() will eliminate all roadpoints at
// infinity except for those which are linked to one or more finite
// roadpoints.

void urbanimage::prune_roadpoints_at_infinity()
{
   outputfunc::write_banner("Pruning roadpoints at infinity:");
   for (Mynode<int>* currnode_ptr=roadpoints_network_ptr->
           get_entries_list_ptr()->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int r=currnode_ptr->get_data(); // roadpoint number
      Site<roadpoint*>* curr_roadpoint_site_ptr=get_roadpoint_site_ptr(r);
      roadpoint* curr_roadpoint_ptr=curr_roadpoint_site_ptr->get_data();
      if (curr_roadpoint_ptr->get_at_infinity())
      {
//         bool infinite_roadpoint_has_only_infinite_neighbors=true;

         Linkedlist<netlink>* curr_roadlink_list_ptr
            =curr_roadpoint_site_ptr->get_netlink_list_ptr();
            
         if (curr_roadlink_list_ptr != NULL)
         {
            Mynode<netlink>* curr_roadlink_node_ptr=
               curr_roadlink_list_ptr->get_start_ptr();

            while (curr_roadlink_node_ptr != NULL)
            {
               int q=curr_roadlink_node_ptr->get_data().get_ID();
               threevector neighbor_posn(get_roadpoint_ptr(q)->get_posn());

               Mynode<netlink>* next_roadlink_node_ptr=
                  curr_roadlink_node_ptr->get_nextptr();
               if (get_roadpoint_ptr(q)->get_at_infinity() ||
                   !(data_bbox_ptr->point_inside_polygon(
                      neighbor_posn)))
               {
                  roadpoints_network_ptr->delete_symmetric_link(r,q);
               }
               else
               {
//                  infinite_roadpoint_has_only_infinite_neighbors=false;
               }
               curr_roadlink_node_ptr=next_roadlink_node_ptr;
            }
         } // curr_roadlink_list_ptr != NULL conditional
      } // roadpoint at infinity conditional
   } // loop over sites in roadpoints network
}

// ---------------------------------------------------------------------
// Method delete_lonely_roadpoints removes any finite entry from the
// roadpoints hashtable which has one or less neighboring roadpoints
// in its *roadlink_list_ptr linked list.  If the "lonely" entry
// corresponds to a "point at infinity" (coming from a very large
// Voronoi polygon vertex), this method removes it from the hashtable
// if it has no neighboring roadpoints.

void urbanimage::delete_lonely_roadpoints(int min_roadpoint_neighbors)
{
   outputfunc::write_banner("Deleting lonely roadpoints:");

   bool points_deleted;
   do
   {
      points_deleted=false;      
      for (Mynode<int>* currnode_ptr=roadpoints_network_ptr->
              get_entries_list_ptr()->get_start_ptr(); 
           currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
      {
         int r=currnode_ptr->get_data(); // roadpoint number
         Site<roadpoint*>* curr_roadpoint_site_ptr=get_roadpoint_site_ptr(r);
         int n_roadpoint_neighbors=curr_roadpoint_site_ptr->
            get_n_neighbors();
         bool at_infinity=curr_roadpoint_site_ptr->get_data()->
            get_at_infinity();
         if ((n_roadpoint_neighbors == 0 && at_infinity) ||
             (n_roadpoint_neighbors <= min_roadpoint_neighbors 
              && !at_infinity))
         {
            roadpoints_network_ptr->delete_single_site(r);
            points_deleted=true;
         }
      } // loop over sites in roadpoints network
   } 
   while (points_deleted);
}

// ---------------------------------------------------------------------
// Member function delete_roadpoints_too_far_from_asphalt scans over
// all entries within the roadpoints network.  It performs an
// extensive leveled-site search for neighbors of each candidate
// roadpoint.  As it goes down the leveled-site neighbor tree, it
// keeps track of the minimum distance between the candidate roadpoint
// and its nearest neighbor which sits on asphalt.  If this minimum
// distance exceeds some large physical distance, the candidate
// roadpoint is declared to lie too far away from asphalt.  The
// candidate roadpoint is ultimately removed from the roadpoints
// network by this method.

void urbanimage::delete_roadpoints_too_far_from_asphalt(
   twoDarray const *ptwoDarray_ptr)
{
   outputfunc::write_banner("Deleting roadpoints too far from asphalt:");

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
      Site<roadpoint*>* curr_roadpoint_site_ptr=get_roadpoint_site_ptr(r);
      cout << r << " " << flush; 
      if (!(curr_roadpoint_site_ptr->get_data()->get_at_infinity()))
      {
         double min_distance_to_asphalt=POSITIVEINFINITY;

         const unsigned int max_level=15;
         for (unsigned int level=0; level<max_level; level++)
         {
            Linkedlist<int>* neighbor_list_ptr=roadpoints_network_ptr->
               generate_leveled_site_neighbor_list(r,level);

            if (neighbor_list_ptr != NULL)
            {
               Mynode<int>* curr_neighbor_node_ptr=neighbor_list_ptr->
                  get_start_ptr();
               while (curr_neighbor_node_ptr != NULL)
               {
                  int neighbor_ID=curr_neighbor_node_ptr->get_data();
                  roadpoint* neighbor_roadpoint_ptr=get_roadpoint_ptr(
                     neighbor_ID);
                  if (neighbor_roadpoint_ptr != NULL)
                  {
                     threevector roadpoint_posn(
                        neighbor_roadpoint_ptr->get_posn());
                     unsigned int px,py;
                     if (ptwoDarray_ptr->point_to_pixel(roadpoint_posn,px,py))
                     {
                        if (nearly_equal(ptwoDarray_ptr->get(px,py),
                                         featurefunc::road_sentinel_value))
                        {
                           double curr_distance_to_asphalt=
                              shortest_distance_between_roadpoints(
                                 r,neighbor_ID);
                           min_distance_to_asphalt=basic_math::min(
                              curr_distance_to_asphalt,
                              min_distance_to_asphalt);
                        }
                     }
                  } // curr_roadpoint_ptr != NULL conditional
                  curr_neighbor_node_ptr=curr_neighbor_node_ptr->
                     get_nextptr();
               }
            }
            delete neighbor_list_ptr;
         } // loop over level index

         const double max_distance=75;	// meters
         if (min_distance_to_asphalt > max_distance)
         {
//            cout << "Deleting site r = " << r << " level = " << level << endl;
//            cout << " posn = " << curr_roadpoint_site_ptr->get_data()->
//               get_posn() << endl;
            roadpoints_to_delete_list_ptr->append_node(r);
         }
      } // roadpoint not at infinity conditional
   } // loop over sites in roadpoints network
   outputfunc::newline();

// Now actually remove from roads network sites located too far from
// asphalt:

   roadpoints_network_ptr->delete_sites(roadpoints_to_delete_list_ptr);
   delete roadpoints_to_delete_list_ptr;
}

// ---------------------------------------------------------------------
// Member function shortest_distance_between_roadpoints takes in
// labels r and q for two roadpoints.  It generates the shortest path
// between these two sites.  This method subsequently returns the sum
// of the distances between each pair of sites within the path.

double urbanimage::shortest_distance_between_roadpoints(int r,int q)
{
   double distance_traveled=0;
   roadpoint* curr_roadpoint_ptr=get_roadpoint_ptr(r);
   if (curr_roadpoint_ptr != NULL)
   {
      Linkedlist<int>* neighbor_list_ptr=roadpoints_network_ptr->
         shortest_path_between_sites(r,q);

      for (Mynode<int>* curr_neighbor_node_ptr=neighbor_list_ptr->
              get_start_ptr(); curr_neighbor_node_ptr != NULL; 
           curr_neighbor_node_ptr=curr_neighbor_node_ptr->get_nextptr())
      {
         int neighbor_ID=curr_neighbor_node_ptr->get_data();
         roadpoint* neighbor_roadpoint_ptr=get_roadpoint_ptr(neighbor_ID);
         if (neighbor_roadpoint_ptr != NULL)
         {
            distance_traveled += (neighbor_roadpoint_ptr->get_posn()-
                                  curr_roadpoint_ptr->get_posn()).magnitude();
            curr_roadpoint_ptr=neighbor_roadpoint_ptr;
         } // neighbor_roadpoint_ptr != NULL conditional
      } // loop over nodes within *neighbor_list_ptr
      delete neighbor_list_ptr;
   } // curr_roadpoint_ptr != NULL conditional
   return distance_traveled;
}

// ---------------------------------------------------------------------
// Method delete_roadlinks_behind_buildings scans over every entry
// within the building network.  It checks whether a front direction
// for a building has previously been defined (e.g. based upon a
// surrounding asphalt distribution).  If so, this method checks every
// edge in the building's Voronoi polygon and determines whether it
// lies to the rear of the building.  If so, the roadlink
// corresponding to the Voronoi edge is deleted from the roadpoints
// network.

void urbanimage::delete_roadlinks_behind_buildings(
   unsigned int n_rear_angle_zones,double half_rear_angle_extent,
   double min_distance_to_data_bbox,
   twoDarray const *just_asphalt_twoDarray_ptr)
{
   outputfunc::write_banner("Deleting roadlinks behind buildings:");

// Instantiate linked list to hold ID labels for roadpoints which are
// located very far away from asphalt.  Since we're actually looping
// over the roadpoints themselves, we cannot delete these far-away
// roadpoints until we've finished traversing the entire roadpoints
// network:

   Linkedlist<int>* roadpoints_to_delete_list_ptr=new Linkedlist<int>;

   for (Mynode<int>* currnode_ptr=buildings_network_ptr->
           get_entries_list_ptr()->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int n=currnode_ptr->get_data();
      cout << n << " " << flush;       
      building* curr_building_ptr=get_building_ptr(n);
      threevector front_direction(curr_building_ptr->get_front_dir(0));
      if (front_direction != Zero_vector)
      {
         polygon* voronoi_region_ptr=curr_building_ptr->
            get_voronoi_region_ptr();
         voronoi_region_ptr->initialize_edge_segments();
         voronoi_region_ptr->compute_edges();

         const double start_angle=180-half_rear_angle_extent;  // degs
         const double stop_angle=180+half_rear_angle_extent;   // degs
         double d_angle=(stop_angle-start_angle)/double(n_rear_angle_zones-1);
         for (unsigned int j=0; j<n_rear_angle_zones; j++)
         {
            double curr_angle=(start_angle+j*d_angle)*PI/180;
            mypoint b_hat(front_direction);
            b_hat.rotate(Zero_vector,0,0,curr_angle);

            for (unsigned int i=0; i<voronoi_region_ptr->get_nvertices(); i++)
            {
               linesegment edge(voronoi_region_ptr->get_edge(i));
               threevector intersection_point;
               bool edge_behind_building=edge.
                  direction_vector_intersects_segment(
                     curr_building_ptr->get_posn(),b_hat.get_pnt(),
                     intersection_point);
               if (edge_behind_building)
               {

// Check whether road link endpoints correspond to sites in the
// roadpoints network:

                  int r=roadpoints_network_ptr->search_for_element(
                     edge.get_v1());
                  int q=roadpoints_network_ptr->search_for_element(
                     edge.get_v2());

// Don't delete road link if it is located too far from building's
// center-of-mass:
                     
                  const double max_bldg_dist_to_edge=30;
                  double bldg_dist_to_edge=
                     edge.point_to_line_segment_distance(
                        curr_building_ptr->get_posn());

// Don't delete road link if building lies too close to data bounding
// box:

                  double bldg_distance_to_bbox=data_bbox_ptr->
                     point_dist_to_polygon(curr_building_ptr->get_posn());

// Don't delete road link if intersection point lies outside data
// bounding box:

                  bool intersection_pnt_in_data_bbox=
                     data_bbox_ptr->point_inside_polygon(
                        intersection_point);

// Perform sanity check whether candidate edge for deletion lies
// mostly on top of asphalt:

                  const double min_asphalt_length_frac=0.33;
                  double asphalt_length_frac=
                     urbanfunc::frac_link_length_over_asphalt(
                        edge,just_asphalt_twoDarray_ptr);

// Check whether any asphalt pixels actually lie quite close to
// candidate roadpoint for deletion:

//                     const double radius=5;	 // meters
//                     bool asphalt_nearby_roadpoint=
//                        featurefunc::feature_nearby(
//                           radius,edge_endpoint,featurefunc::road_sentinel_value,
//                           just_asphalt_twoDarray_ptr);

                  if (r > -1 && q > -1 && intersection_pnt_in_data_bbox &&
                      bldg_dist_to_edge < max_bldg_dist_to_edge &&
                      bldg_distance_to_bbox > min_distance_to_data_bbox &&
                      asphalt_length_frac < min_asphalt_length_frac)
                  {
                     roadpoints_network_ptr->delete_symmetric_link(r,q);

//                     if (r==258 || r==76 || q==258 || q==76)
//                     {
//                        cout << "Deleting link between r = " << r << endl;
//                        cout << "and q = " << q << endl;
//                        cout << "Building n = " << n << endl;
//                        cout << "edge = " << edge << endl;
//                        cout << "Building COM distance to edge = "
//                             << bldg_dist_to_edge << endl;
//                        cout << "intersection_point = " 
//                             << intersection_point << endl;
//                     }
                  } // r > -1 && etc conditional
               } // edge_behind_building conditional
                  
// Note added in early April 04: We still need to add code which will
// delete roadpoint from building list;

            } // loop over index i labeling voronoi region polygon vertices
         } // loop over index j labeling backwards angular zone
      } // front_direction != Zero_vector conditional
   } // loop over nodes in buildings network entries list
   outputfunc::newline();

   roadpoints_network_ptr->delete_sites(roadpoints_to_delete_list_ptr);
   delete roadpoints_to_delete_list_ptr;
}

// ---------------------------------------------------------------------
// Member function mark_roadpoints_in_front_of_buildings scans over all sites
// within the roadpoints network.  It instantiates line segments
// between each pair of neighboring roadpoints, and then computes the
// asphalt content along those segments.  If the fraction of asphalt
// along a segment exceeds some large threshold, the in_front_of_bldg
// boolean flags for both endpoints of the line segment are set to
// true.

// As of 8/2/04, this method is deprecated and no longer used...

void urbanimage::mark_roadpoints_in_front_of_buildings(
   twoDarray const *just_asphalt_twoDarray_ptr)
{
   outputfunc::write_banner("Mark roadpoints in front of buildings:");   

   for (Mynode<int>* currnode_ptr=roadpoints_network_ptr->
           get_entries_list_ptr()->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int r=currnode_ptr->get_data(); // roadpoint number
      cout << r << " " << flush;
      Site<roadpoint*>* curr_roadpoint_site_ptr=get_roadpoint_site_ptr(r);

      Linkedlist<netlink>* curr_roadlink_list_ptr
         =curr_roadpoint_site_ptr->get_netlink_list_ptr();

      if (curr_roadlink_list_ptr != NULL)
      {
         for (Mynode<netlink>* roadlink_node_ptr=curr_roadlink_list_ptr->
                 get_start_ptr(); roadlink_node_ptr != NULL;
              roadlink_node_ptr=roadlink_node_ptr->get_nextptr())
         {
            int q=roadlink_node_ptr->get_data().get_ID();
            roadpoint* curr_roadpoint_ptr(get_roadpoint_ptr(r));
            roadpoint* neighbor_roadpoint_ptr(get_roadpoint_ptr(q));
            linesegment curr_edge(curr_roadpoint_ptr->get_posn(),
                                  neighbor_roadpoint_ptr->get_posn());

// Perform sanity check whether candidate edge for deletion lies
// mostly on top of asphalt:

            double asphalt_length_frac=
               urbanfunc::frac_link_length_over_asphalt(
                  curr_edge,just_asphalt_twoDarray_ptr);
            if (asphalt_length_frac > 0.9)
            {
               curr_roadpoint_ptr->set_in_front_of_bldg(true);
               neighbor_roadpoint_ptr->set_in_front_of_bldg(true);
            }
         } // loop of roadlinks
      } // curr_roadlink_list_ptr != NULL conditional
   } // loop over sites in roadpoints network
   outputfunc::newline();
   
   delete just_asphalt_twoDarray_ptr;
}

// ---------------------------------------------------------------------
// Method identify_front_roadpoint_seeds loops over all sites within
// the buildings network.  It examines the Voronoi region edges for
// each building where a front direction has been defined.  It focuses
// upon the roadpoint for each front edge which lies closest to the
// point where the building's front ray intersects the edge.  If this
// intersection point does not lie too far away from the center of the
// building's Voronoi region, this method declares the edge endpoint
// to lie "in front" of some building.  

void urbanimage::identify_front_roadpoint_seeds()
{
   outputfunc::write_banner("Identifying front roadpoint seeds");

   for (Mynode<int>* currnode_ptr=buildings_network_ptr->
           get_entries_list_ptr()->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int n=currnode_ptr->get_data();
      cout << n << " " << flush;       
      building* curr_building_ptr=get_building_ptr(n);
      threevector front_direction(curr_building_ptr->get_front_dir(0));
      if (front_direction != Zero_vector)
      {
         polygon* voronoi_region_ptr=curr_building_ptr->
            get_voronoi_region_ptr();
         voronoi_region_ptr->initialize_edge_segments();
         voronoi_region_ptr->compute_edges();

         for (unsigned int i=0; i<voronoi_region_ptr->get_nvertices(); i++)
         {
            linesegment edge(voronoi_region_ptr->get_edge(i));

// Do not classify any roadpoint which lies outside the ladar data
// bounding box:

            if (data_bbox_ptr->point_inside_polygon(edge.get_v1()) &&
                data_bbox_ptr->point_inside_polygon(edge.get_v2()))
            {
               threevector intersection_point;
               bool edge_in_front_of_building=edge.
                  direction_vector_intersects_segment(
                     voronoi_region_ptr->get_origin(),front_direction,
                     intersection_point);
               if (edge_in_front_of_building)
               {

// Check whether v1 or v2 is closer to intersection point:

                  threevector edge_endpoint;
                  if ((edge.get_v1()-intersection_point).magnitude() < 
                      (edge.get_v2()-intersection_point).magnitude())
                  {
                     edge_endpoint=edge.get_v1();
                  }
                  else
                  {
                     edge_endpoint=edge.get_v2();
                  }

// Check whether distance from edge endpoint to voronoi region center
// is not too large:

                  const double max_radial_distance=25; // meters
                  double radial_distance=(
                     threevector(voronoi_region_ptr->get_origin())-
                     edge_endpoint).magnitude();

// Check whether edge endpoint corresponds to a site in the roadpoints
// network:

                  int r=roadpoints_network_ptr->search_for_element(
                     edge_endpoint);
                  if (radial_distance < max_radial_distance && r > -1)
                  {
                     get_roadpoint_ptr(r)->set_in_front_of_bldg(true);
//                     cout << "Roadpoint r = " << r << " lies in front" 
//                          << endl;
                  } 
               } // edge_in_front_of_building conditional
            } // point_inside_polygon conditional
         } // loop over index i labeling voronoi region polygon vertices
      } // front_direction != Zero_vector conditional
   } // loop over nodes in buildings network entries list
   outputfunc::newline();
}

// ---------------------------------------------------------------------
// Method propagate_roadpoint_frontness scans over all sites within
// the roadpoints network which have already been labeled as lying in
// front of some building.  For each such roadpoint, it checks how
// many roadpoint neighbors it has.  If the number is 2 or less, this
// method classifies the neighbors as also lying in front of some
// building.  This method continues updating roadpoints' "frontness"
// classification until no further changes can be made.

void urbanimage::propagate_roadpoint_frontness()
{
   outputfunc::write_banner("Propagating roadpoint frontness:");   

   bool front_flag_reset;
   do
   {
      front_flag_reset=false;
      for (Mynode<int>* currnode_ptr=roadpoints_network_ptr->
              get_entries_list_ptr()->get_start_ptr(); 
           currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
      {
         int r=currnode_ptr->get_data(); // roadpoint number
         cout << r << " " << flush;

         roadpoint* curr_roadpoint_ptr(get_roadpoint_ptr(r));
         if (curr_roadpoint_ptr->get_in_front_of_bldg() &&
             !curr_roadpoint_ptr->get_at_infinity())
         {
            Linkedlist<netlink>* curr_roadlink_list_ptr
               =get_roadpoint_site_ptr(r)->get_netlink_list_ptr();
            
            if (curr_roadlink_list_ptr != NULL &&
                curr_roadlink_list_ptr->size() <= 2)
            {
               for (Mynode<netlink>* roadlink_node_ptr=
                       curr_roadlink_list_ptr->get_start_ptr(); 
                    roadlink_node_ptr != NULL;
                    roadlink_node_ptr=roadlink_node_ptr->get_nextptr())
               {
                  int q=roadlink_node_ptr->get_data().get_ID();
                  roadpoint* neighbor_roadpoint_ptr(get_roadpoint_ptr(q));
                  if (!neighbor_roadpoint_ptr->get_in_front_of_bldg() &&
                      !neighbor_roadpoint_ptr->get_at_infinity())
                  {
                     neighbor_roadpoint_ptr->set_in_front_of_bldg(true);
                     front_flag_reset=true;
                  }
               } // loop over neighboring roadlinks
            } // curr_roadlink_list_ptr != NULL conditional
         } // !intersection conditional
      } // loop over sites in roadpoints network
      outputfunc::newline();
   }
   while (front_flag_reset);
}

// ---------------------------------------------------------------------
// Method enumerate_roadpoint_neighbors takes in a roadpoint ID label
// and lists all its neighbors to cout.

void urbanimage::enumerate_roadpoint_neighbors(int r)
{
   cout << "Roadpoint r = " << r << endl;
   Site<roadpoint*>* roadpoint_site_ptr=get_roadpoint_site_ptr(r);
   if (roadpoint_site_ptr==NULL)
   {
      cout << "Site does NOT exist in roadpoint network!" << endl;
   }
   else
   {
      roadpoint_site_ptr->display_neighbor_list();
   }
   outputfunc::newline();
}


// ==========================================================================
// Road network display methods
// ==========================================================================

// Method draw_nearby_buildings_to_roadpoint takes in ID label r for
// some entry in the roadpoints hashtable.  It draws line segments to
// the closest buildings located nearby this roadpoint onto output
// twoDarray *ftwoDarray_ptr.

void urbanimage::draw_nearby_buildings_to_roadpoint(
   int r,twoDarray* ftwoDarray_ptr)
{
   roadpoint* curr_roadpoint_ptr=get_roadpoint_ptr(r);
   if (curr_roadpoint_ptr != NULL)
   {
      Linkedlist<int>* curr_bldg_list_ptr
         =curr_roadpoint_ptr->get_nearby_bldg_list_ptr();
            
      if (curr_bldg_list_ptr != NULL)
      {
         Mynode<int>* curr_bldg_node_ptr=
            curr_bldg_list_ptr->get_start_ptr();
         while (curr_bldg_node_ptr != NULL)
         {
            int n=curr_bldg_node_ptr->get_data();
            draw_bldg_roadpoint_link(n,r,ftwoDarray_ptr);
            curr_bldg_node_ptr=curr_bldg_node_ptr->get_nextptr();
         }
      }
   }
}

// ---------------------------------------------------------------------
// Method draw_bldg_roadpoint_link takes in building ID label n and
// roadpoint ID label r.  It draws a line segment between these two
// objects onto output twoDarray *ftwoDarray_ptr.

void urbanimage::draw_bldg_roadpoint_link(
   int n,int r,twoDarray* ftwoDarray_ptr)
{
   linesegment l(get_building_ptr(n)->get_posn(),
                 get_roadpoint_ptr(r)->get_posn());
//   const double radius=0.75;	// meter
//   drawfunc::draw_thick_line(l,colorfunc::white,radius,ftwoDarray_ptr);
   drawfunc::draw_line(l,0.05,ftwoDarray_ptr);
}

// ---------------------------------------------------------------------
// Method draw_min_bldg_distances_to_roadlinks conducts a search over
// all roadlinks between roadpoints r and q.  It computes the minimum
// distance between a roadlink and all nearby buildings.  (If the
// roadlink lies precisely on a Voronoi edge, then by definition two
// buildings should be equidistant from the roadlink.)  This method
// draws a small red line from the roadlink to the closest building.
// Output from member function draw_min_bldg_distances_to_roadlinks is
// best viewed using metafiles rather than the 3D dataviewer.

void urbanimage::draw_min_bldg_distances_to_roadlinks(
   twoDarray* ftwoDarray_ptr)
{
   linesegment closest_bldg_roadlink_linesegment;

   for (Mynode<int>* currnode_ptr=roadpoints_network_ptr->
           get_entries_list_ptr()->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int r=currnode_ptr->get_data(); // roadpoint number
      Site<roadpoint*>* curr_roadpoint_site_ptr=get_roadpoint_site_ptr(r);

      Linkedlist<netlink>* curr_roadlink_list_ptr
         =curr_roadpoint_site_ptr->get_netlink_list_ptr();

      if (curr_roadlink_list_ptr != NULL)
      {
         Mynode<netlink>* roadlink_node_ptr=
            curr_roadlink_list_ptr->get_start_ptr();

         while (roadlink_node_ptr != NULL)
         {
            int q=roadlink_node_ptr->get_data().get_ID();
            roadpoint* neighbor_roadpoint_ptr=get_roadpoint_ptr(q);
            double bldg_dist_to_roadlink=
               min_bldg_distance_to_roadlink(
                  curr_roadpoint_site_ptr->get_data(),
                  neighbor_roadpoint_ptr,
                  closest_bldg_roadlink_linesegment);

            const double min_allowed_bldg_dist_to_roadlink=10;  // meters
//             const double min_allowed_bldg_dist_to_roadlink=1000;  // meters
            if (bldg_dist_to_roadlink < min_allowed_bldg_dist_to_roadlink)
            {
               drawfunc::draw_line(
                  closest_bldg_roadlink_linesegment,1.0,ftwoDarray_ptr);
//               const double radius=0.75;	// meter
//               drawfunc::draw_thick_line(
//                  closest_bldg_roadlink_linesegment,colorfunc::white,
//                  radius,ftwoDarray_ptr);
            }
            roadlink_node_ptr=roadlink_node_ptr->get_nextptr();
         }
      } // curr_roadlink_list_ptr != NULL conditional
   } // loop over sites in roadpoints network
}

// ==========================================================================
// Road extraction methods:
// ==========================================================================

// Method road_match_filter rotates the input feature map through a
// series of angles.

void urbanimage::road_match_filter(
   string imagedir,twoDarray const *ztwoDarray_ptr,
   twoDarray const *features_twoDarray_ptr,
   twoDarray const *binary_seed_twoDarray_ptr)
{
   outputfunc::write_banner("Match filtering for roads:");

//   writeimage("ztwoDarray",ztwoDarray_ptr);
   writeimage("features",features_twoDarray_ptr,false,ladarimage::p_data);
//   writeimage("binary_seed",binary_seed_twoDarray_ptr,false,
// 	ladarimage::p_data);

   twoDarray* zexpand_twoDarray_ptr=ztwoDarray_ptr->
      expand_for_rotation(xyzpfunc::null_value);
   twoDarray* fexpand_twoDarray_ptr=features_twoDarray_ptr->
      expand_for_rotation(xyzpfunc::null_value);
   twoDarray* binary_seed_expand_twoDarray_ptr=
      binary_seed_twoDarray_ptr->expand_for_rotation(
         xyzpfunc::null_value);

//         string fexpand_filenamestr=imagedir+"expand_features.xyzp";
//         ladarfunc::write_xyzp_data(
//            zexpand_twoDarray_ptr,fexpand_twoDarray_ptr,fexpand_filenamestr,
//            true);

//   writeimage("zexpand",zexpand_twoDarray_ptr);
//   writeimage("fexpand",fexpand_twoDarray_ptr,false,ladarimage::p_data);
//   writeimage("binary_expand",binary_seed_expand_twoDarray_ptr,false,
//              ladarimage::p_data);

   twoDarray* zrot_expand_twoDarray_ptr=new twoDarray(
      zexpand_twoDarray_ptr);
   twoDarray* frot_expand_twoDarray_ptr=new twoDarray(
      fexpand_twoDarray_ptr);
   twoDarray* binary_seed_rot_expand_twoDarray_ptr=new twoDarray(
      binary_seed_expand_twoDarray_ptr);
   twoDarray* road_rot_expand_twoDarray_ptr=new twoDarray(
      fexpand_twoDarray_ptr);
   twoDarray* tree_rot_expand_twoDarray_ptr=new twoDarray(
      fexpand_twoDarray_ptr);
   twoDarray* grass_rot_expand_twoDarray_ptr=new twoDarray(
      fexpand_twoDarray_ptr);
   twoDarray* null_rot_expand_twoDarray_ptr=new twoDarray(
      fexpand_twoDarray_ptr);

// Store roadside probability and direction information in
// *road_summary_twoDarray_ptr and *road_direction_twoDarray_ptr:

   twoDarray* road_summary_twoDarray_ptr=new twoDarray(
      features_twoDarray_ptr);
   twoDarray* road_direction_twoDarray_ptr=new twoDarray(
      features_twoDarray_ptr);
   road_summary_twoDarray_ptr->initialize_values(xyzpfunc::null_value);
   road_direction_twoDarray_ptr->initialize_values(xyzpfunc::null_value);

// Instantiate long, skinny rectangular bounding box:

   const double bbox_width=6;
   const double bbox_length=90;

   parallelogram bbox(bbox_width,bbox_length);

//   const int nsteps=1;
//   const int nsteps=5;
   const unsigned int nsteps=36;
   const double theta_start=90;
   const double theta_stop=-90;
   const double dtheta=(theta_stop-theta_start)/nsteps;

   for (unsigned int n=0; n<nsteps; n++)
   {
      double theta=theta_start+n*dtheta;
      cout << "Bounding box rotation angle = " << theta << endl;
      theta *= PI/180;
   
      zexpand_twoDarray_ptr->rotate_about_center(
         theta,zrot_expand_twoDarray_ptr,xyzpfunc::null_value);
      fexpand_twoDarray_ptr->rotate_about_center(
         theta,frot_expand_twoDarray_ptr,xyzpfunc::null_value);
      binary_seed_expand_twoDarray_ptr->rotate_about_center(
         theta,binary_seed_rot_expand_twoDarray_ptr,xyzpfunc::null_value);

//      writeimage("zrot_expand",zrot_expand_twoDarray_ptr);
//      writeimage("frot_expand",frot_expand_twoDarray_ptr,
//                 false,ladarimage::p_data);
//      writeimage("binary_seed_rot_expand",
//                 binary_seed_rot_expand_twoDarray_ptr,
//                 false,ladarimage::p_data);

      compute_bbox_road_frac(
         bbox,frot_expand_twoDarray_ptr,
         binary_seed_rot_expand_twoDarray_ptr,
         road_rot_expand_twoDarray_ptr,tree_rot_expand_twoDarray_ptr,
         grass_rot_expand_twoDarray_ptr,null_rot_expand_twoDarray_ptr);

/*
  writeimage("road_rot_expand",
  road_rot_expand_twoDarray_ptr,
  false,ladarimage::p_data);
  writeimage("tree_rot_expand",
  tree_rot_expand_twoDarray_ptr,
  false,ladarimage::p_data);
  writeimage("grass_rot_expand",
  grass_rot_expand_twoDarray_ptr,
  false,ladarimage::p_data);
  writeimage("null_rot_expand",
  null_rot_expand_twoDarray_ptr,
  false,ladarimage::p_data);
*/

      accumulate_roadside_info(
         theta,features_twoDarray_ptr,fexpand_twoDarray_ptr,
         road_rot_expand_twoDarray_ptr,road_summary_twoDarray_ptr,
         road_direction_twoDarray_ptr);
   }

   writeimage("road_summary",road_summary_twoDarray_ptr,
              false,ladarimage::p_data);
   writeimage("road_direction",road_direction_twoDarray_ptr,
              false,ladarimage::direction_data);

   delete zexpand_twoDarray_ptr;
   delete fexpand_twoDarray_ptr;
   delete zrot_expand_twoDarray_ptr;
   delete frot_expand_twoDarray_ptr;
   delete binary_seed_rot_expand_twoDarray_ptr;
   delete road_rot_expand_twoDarray_ptr;
   delete tree_rot_expand_twoDarray_ptr;
   delete grass_rot_expand_twoDarray_ptr;
   delete null_rot_expand_twoDarray_ptr;

   delete road_summary_twoDarray_ptr;
   delete road_direction_twoDarray_ptr;
}

// ---------------------------------------------------------------------
// Method compute_bbox_road_frac takes in a feature map within
// *ftwoDarray_ptr along with a binary road seed map within
// *binary_seed_twoDarray_ptr.  Both input maps are assumed to have
// already been expanded and rotated about their center points.  This
// method instantiates a rectangular bounding box of size bbox_width
// and bbox_length.  It then systematically moves this bounding box
// across the rotated and expanded image.  If the center of the
// bounding box is located on top of a seed point, this method
// computes the fractions of tree, grass, road and null pixels located
// inside the bbox.  As of now, this method saves the road fraction
// information within output twoDarray *road_twoDarray_ptr.

void urbanimage::compute_bbox_road_frac(
   parallelogram& bbox,twoDarray const *ftwoDarray_ptr,
   twoDarray const *binary_seed_twoDarray_ptr,
   twoDarray* road_twoDarray_ptr,twoDarray* tree_twoDarray_ptr,
   twoDarray* grass_twoDarray_ptr,twoDarray* null_twoDarray_ptr)
{
   unsigned int mdim=ftwoDarray_ptr->get_mdim();
   unsigned int ndim=ftwoDarray_ptr->get_ndim();
   threevector curr_center;

   road_twoDarray_ptr->initialize_values(xyzpfunc::null_value);

   for (unsigned int px=0; px<mdim; px += 3)
   {
      if (px%100==0)
      {
//         cout << "px = " << px << " mdim = " 
//              << ftwoDarray_ptr->get_mdim() << endl;
      }
      
      for (unsigned int py=0; py<ndim; py += 3)
      {
         int n_eff=ndim*px+py;

// Don't bother to perform bounding box feature fraction computation
// unless the bbox is centered upon a seed location:

         if (binary_seed_twoDarray_ptr->get(px,py) > 0)
         {

// Center bbox at pixel location (px,py):

            ftwoDarray_ptr->pixel_to_point(px,py,curr_center);
            bbox.absolute_position(curr_center);
            unsigned int min_px,min_py,max_px,max_py;
            ftwoDarray_ptr->locate_extremal_xy_pixels(
               bbox,min_px,min_py,max_px,max_py);

            double tree_frac,grass_frac,road_frac,null_frac;
            if (!compute_feature_fracs_inside_bbox(
               min_px,max_px,min_py,max_py,
               ftwoDarray_ptr,tree_frac,grass_frac,road_frac,null_frac))
            {
               road_twoDarray_ptr->put(n_eff,road_frac);
               tree_twoDarray_ptr->put(n_eff,tree_frac);
               grass_twoDarray_ptr->put(n_eff,grass_frac);
               null_twoDarray_ptr->put(n_eff,null_frac);
            }
         } // binary_seed > 0 conditional
      } // loop over py index
   } // loop over px index
}
   
// ---------------------------------------------------------------------
// Method compute_feature_fracs_inside_bbox

// It has been intentionally stripped down to its barest essentials in
// order to increase execution speed as much as possible:
   
bool urbanimage::compute_feature_fracs_inside_bbox(
   unsigned int min_px,unsigned int max_px,
   unsigned int min_py,unsigned int max_py,
   twoDarray const *frot_expand_twoDarray_ptr,
   double& tree_frac,double& grass_frac,double& road_frac,
   double& null_frac)
{
   bool building_pixel_encountered=false;
   int n_tree_pixels=0;
   int n_grass_pixels=0;
   int n_road_pixels=0;
   int n_null_pixels=0;
   tree_frac=grass_frac=road_frac=null_frac=0;

   for (unsigned int i=min_px; i<max_px; i++)
   {
      for (unsigned int j=min_py; j<max_py; j++)
      {
         double curr_feature=frot_expand_twoDarray_ptr->get(i,j);
         if (nearly_equal(curr_feature,featurefunc::building_sentinel_value))
         {
            building_pixel_encountered=true;
            return building_pixel_encountered;
         }
         else
         {
            if (nearly_equal(curr_feature,featurefunc::tree_sentinel_value))
            {
               n_tree_pixels++;
            }
            else if (nearly_equal(
               curr_feature,featurefunc::grass_sentinel_value))
            {
               n_grass_pixels++;
            }
            else if (nearly_equal(
               curr_feature,featurefunc::road_sentinel_value))
            {
               n_road_pixels++;
            }
            else if (nearly_equal(curr_feature,xyzpfunc::null_value))
            {
               n_null_pixels++;
            }
         }
      } // loop over index j
   } // loop over index i

   double n_total_pixels=
      double(n_tree_pixels+n_grass_pixels+n_road_pixels
             +n_null_pixels);
   tree_frac=double(n_tree_pixels)/n_total_pixels;
   grass_frac=double(n_grass_pixels)/n_total_pixels;
   road_frac=double(n_road_pixels)/n_total_pixels;
   null_frac=double(n_null_pixels)/n_total_pixels;
   return building_pixel_encountered;
}

// ---------------------------------------------------------------------
// Member function accumulate_roadside_info 

void urbanimage::accumulate_roadside_info(
   double bbox_rotation_angle,
   twoDarray const *ftwoDarray_ptr,twoDarray const *fexpand_twoDarray_ptr,
   twoDarray const *road_rot_expand_twoDarray_ptr,
   twoDarray* road_summary_twoDarray_ptr,
   twoDarray* road_direction_twoDarray_ptr)
{
   outputfunc::write_banner("Accumulate roadside score info:");
   
   const double roadside_score_threshold=0.85;
   for (unsigned int px=0; px<ftwoDarray_ptr->get_mdim(); px++)
   {
      for (unsigned int py=0; py<ftwoDarray_ptr->get_ndim(); py++)
      {
         double currf=ftwoDarray_ptr->get(px,py);
         if (currf > xyzpfunc::null_value)
         {
            unsigned int px_rot,py_rot;
            ftwoDarray_ptr->rotated_pixel_coords(
               px,py,bbox_rotation_angle,px_rot,py_rot,fexpand_twoDarray_ptr);
            double curr_roadside_score=road_rot_expand_twoDarray_ptr->
               get(px_rot,py_rot);
            if (curr_roadside_score > roadside_score_threshold &&
                curr_roadside_score > road_summary_twoDarray_ptr->get(px,py))
            {
               road_summary_twoDarray_ptr->put(px,py,curr_roadside_score);
               road_direction_twoDarray_ptr->put(
                  px,py,0.5*PI-bbox_rotation_angle);
            }
         }
      } // loop over py index
   } // loop over px index
}

// ==========================================================================
// Combined building and road network methods
// ==========================================================================

// Method add_roadpoint_to_bldg_list adds input roadpoint r as a
// nearby neighbor to building n's *nearby_roadpoint_list_ptr linked
// list.

void urbanimage::add_roadpoint_to_bldg_list(int r,int n)
{
   building* curr_building_ptr=get_building_ptr(n);
   Linkedlist<int>* curr_roadpoint_list_ptr=
      curr_building_ptr->get_nearby_roadpoint_list_ptr();
   if (curr_roadpoint_list_ptr==NULL)
   {
      curr_roadpoint_list_ptr=new Linkedlist<int>;
      curr_building_ptr->set_nearby_roadpoint_list_ptr(
         curr_roadpoint_list_ptr);
   }
         
   if (curr_roadpoint_list_ptr->data_in_list(r)==NULL)
   {
      curr_roadpoint_list_ptr->append_node(r);
   }
}

// ---------------------------------------------------------------------
// Method add_bldg_to_roadpoint_list adds input building n as a
// nearby neighbor to roadpoint r's *nearby_bldg_list_ptr linked list.

void urbanimage::add_bldg_to_roadpoint_list(int n,int r)
{
   roadpoint* curr_roadpoint_ptr=get_roadpoint_ptr(r);
   Linkedlist<int>* curr_bldg_list_ptr=
      curr_roadpoint_ptr->get_nearby_bldg_list_ptr();   

   if (curr_bldg_list_ptr==NULL)
   {
      curr_bldg_list_ptr=new Linkedlist<int>;
      curr_roadpoint_ptr->set_nearby_bldg_list_ptr(curr_bldg_list_ptr);
   }
         
   if (curr_bldg_list_ptr->data_in_list(n)==NULL)
   {
      curr_bldg_list_ptr->append_node(n);
   }
}

// ---------------------------------------------------------------------
// Method compute_asphalt_distribution_relative_to_buildings computes
// the average direction vector from each building's Voronoi region
// center to all asphalt pixels within the Voronoi region.  It
// subsequently evaluates the dotproduct between each individual
// asphalt element direction vector with the average direction vector.
// If every asphalt element lies within some prescribed angular zone
// about the average direction vector, this method sets the building's
// front direction member equal to the average direction vector.

void urbanimage::compute_asphalt_distribution_relative_to_buildings(
   twoDarray const *features_twoDarray_ptr)
{
   outputfunc::write_banner(
      "Computing asphalt distribution relative to buildings:");

   for (Mynode<int>* currnode_ptr=buildings_network_ptr->
           get_entries_list_ptr()->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int n=currnode_ptr->get_data();
      cout << n << " " << flush;

      polygon* curr_voronoi_region_ptr(
         get_building_ptr(n)->get_voronoi_region_ptr());
      threevector origin(curr_voronoi_region_ptr->get_origin());

// To avoid spurious edge effects, do not attempt to compute asphalt
// distribution for buildings located too close to data bounding box:

      const double min_distance_to_bbox=30; // meters
      double distance_to_bbox=data_bbox_ptr->point_dist_to_polygon(origin);

      if (distance_to_bbox > min_distance_to_bbox)
      {
         unsigned int min_px,max_px,min_py,max_py;
         features_twoDarray_ptr->locate_extremal_xy_pixels(
            *curr_voronoi_region_ptr,min_px,min_py,max_px,max_py);

         int asphalt_pixel_count=0;
         threevector currpoint;
         threevector nhat_sum(Zero_vector);
         threevector nhatsq_sum(Zero_vector);
         for (unsigned int px=min_px; px<max_px; px++)
         {
            for (unsigned int py=min_py; py<max_py; py++)
            {
               features_twoDarray_ptr->pixel_to_point(px,py,currpoint);
               if (curr_voronoi_region_ptr->
                   point_inside_polygon(currpoint))
               {
                  double curr_feature_value=features_twoDarray_ptr->
                     get(px,py);
                  if (nearly_equal(
                     curr_feature_value,featurefunc::road_sentinel_value))
                  {
                     asphalt_pixel_count++;
                     threevector nhat((currpoint-origin).unitvector());
                     nhat_sum += nhat;
                     nhatsq_sum += threevector(
                        sqr(nhat.get(0)),sqr(nhat.get(1)));
                  } // curr_feature_value==road_sentinel_value conditional
               } 
            } // loop over py index
         } // loop over px index
   
         if (nhat_sum != Zero_vector)
         {
            threevector nhat_mean=nhat_sum/double(asphalt_pixel_count);
            threevector front_direction(nhat_mean.unitvector());

            double dotproduct[asphalt_pixel_count];
            int count=0;
            for (unsigned int px=min_px; px<max_px; px++)
            {
               for (unsigned int py=min_py; py<max_py; py++)
               {
                  features_twoDarray_ptr->pixel_to_point(px,py,currpoint);
                  if (curr_voronoi_region_ptr->
                      point_inside_polygon(currpoint))
                  {
                     double curr_feature_value=features_twoDarray_ptr->
                        get(px,py);
                     if (nearly_equal(
                        curr_feature_value,featurefunc::road_sentinel_value))
                     {
                        threevector nhat((currpoint-origin).unitvector());
                        dotproduct[count++]=nhat.dot(front_direction);
                     } // curr_feature_value==road_sentinel_value conditional
                  } 
               } // loop over py index
            } // loop over px index

            prob_distribution prob(asphalt_pixel_count,dotproduct,30);
//         prob.set_densityfilenamestr(imagedir+"dotproduct_density.meta");
//         prob.set_cumulativefilenamestr(imagedir+"dotproduct_cumulative.meta");
//         prob.set_xlabel("Dotproduct");
//         prob.writeprobdists();

            double dotproduct_1_percent=
               prob.find_x_corresponding_to_pcum(0.01);
            double dotproduct_10_percent=
               prob.find_x_corresponding_to_pcum(0.1);
            double dotproduct_50_percent=
               prob.find_x_corresponding_to_pcum(0.5);

            const double cos_1_percent=cos(90*PI/180);
            const double cos_10_percent=cos(75*PI/180);
            const double cos_50_percent=cos(60*PI/180);
            if (dotproduct_1_percent > cos_1_percent &&
                dotproduct_10_percent > cos_10_percent &&
                dotproduct_50_percent > cos_50_percent)
            {
               get_building_ptr(n)->set_front_dir(0,front_direction);
            }
         } // nhat_sum != Zero_vector conditional
      } // distance_to_bbox > min_distance_to_bbox conditional
   } // loop over nodes in buildings network entries list

   outputfunc::newline();
}

// ---------------------------------------------------------------------
// Method compute_asphalt_distribution_relative_to_buildings computes
// the average direction vector from each building's Voronoi region
// center to all asphalt pixels within the Voronoi region.  It
// subsequently evaluates the dotproduct between each individual
// asphalt element direction vector with the average direction vector.
// If every asphalt element lies within some prescribed angular zone
// about the average direction vector, this method sets the building's
// front direction member equal to the average direction vector.

void urbanimage::compute_asphalt_angular_distribution_relative_to_buildings(
   twoDarray const *features_twoDarray_ptr)
{
   outputfunc::write_banner(
      "Computing asphalt angular distribution relative to buildings:");

// Instantiate twoDarray holding only asphalt features with all others
// set to zero:

   twoDarray* just_asphalt_twoDarray_ptr=featurefunc::cull_feature_pixels(
      featurefunc::road_sentinel_value,0.0,features_twoDarray_ptr);

// Set up array to hold asphalt angular distribution input information:

   const double theta_start=0;
   const double theta_stop=2*PI;
   const double dtheta=2*PI/180;
   const unsigned int nbins=basic_math::round((theta_stop-theta_start)/dtheta);
   double asphalt_along_ray[nbins];

   for (Mynode<int>* currnode_ptr=buildings_network_ptr->
           get_entries_list_ptr()->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int n=currnode_ptr->get_data();
      cout << n << " " << flush;
      building* curr_bldg_ptr=get_building_ptr(n);
      threevector origin(curr_bldg_ptr->get_posn());

// First compute a reasonable radial search distance based upon
// distances from voronoi vertices to building COM:

      double radius=NEGATIVEINFINITY;
      polygon* voronoi_region_ptr(curr_bldg_ptr->get_voronoi_region_ptr());
      for (unsigned int i=0; i<voronoi_region_ptr->get_nvertices(); i++)
      {
         radius=basic_math::max(
            radius,(voronoi_region_ptr->get_vertex(i)-origin).
            magnitude());
      }
      const double min_radius=10;	// meters
      const double max_radius=40;	// meters
      radius=basic_math::max(min_radius,1.25*radius);
      radius=basic_math::min(max_radius,radius);

      int nrays_with_asphalt=0;
      for (unsigned int i=0; i<nbins; i++)
      {
         double theta=theta_start+i*dtheta;
         threevector e_hat(cos(theta),sin(theta));
         linesegment ray(origin,origin+radius*e_hat);

         const double SMALL=0.01;
         asphalt_along_ray[i]=0.0;
         if (urbanfunc::frac_link_length_over_asphalt(
            ray,just_asphalt_twoDarray_ptr) > SMALL)
         {
            asphalt_along_ray[i]=1.0;
            nrays_with_asphalt++;
         }
      } // loop over index i labeling ray angle

// Search for "building islands" which are surrounded by "oceans of
// asphalt":

      double asphalt_angular_frac=double(nrays_with_asphalt)/double(nbins);
      if (asphalt_angular_frac > 0.95)
      {
//         curr_bldg_ptr->set_is_street_island(true);


//         cout << "*******************************************" << endl;
//         cout << "Building n = " << n << " is a street island" << endl;
//         cout << "*******************************************" << endl;

         Linkedlist<int>* nearby_roadpoint_list_ptr=
            curr_bldg_ptr->get_nearby_roadpoint_list_ptr();
         for (Mynode<int>* nearby_roadpoint_node_ptr=
                 nearby_roadpoint_list_ptr->get_start_ptr();
              nearby_roadpoint_node_ptr != NULL; nearby_roadpoint_node_ptr=
                 nearby_roadpoint_node_ptr->get_nextptr())
         {
            roadpoint* nearby_roadpoint_ptr=
               get_roadpoint_ptr(nearby_roadpoint_node_ptr->get_data());
            if (nearby_roadpoint_ptr != NULL)
               nearby_roadpoint_ptr->set_in_front_of_bldg(true);
         }
      } // angular frac > 0.95 conditional
      else if (asphalt_angular_frac < 0.05)
      {
//         cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
//         cout << "Building n = " << n << " is surrounded by grass" << endl;
//         cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
      }
      else
      {
         if (asphalt_angular_frac > 0.90 && asphalt_angular_frac <= 0.95)
         {
//            curr_bldg_ptr->set_is_street_peninsula(true);
//            outputfunc::newline();
//            cout << "**********************************************" << endl;
//            cout << "Building n = " << n << " is a street peninsula" << endl;
//            cout << "asphalt_angular_frac = " << asphalt_angular_frac
//                 << endl;
//            cout << "**********************************************" << endl;
//            outputfunc::newline();
         }
         
// Search for longest string of zero values within asphalt_along_ray
// array:

         int max_zero_count=NEGATIVEINFINITY;
         double theta_non_asphalt=0;
         for (unsigned int i=0; i<nbins; i++)
         {
            unsigned int j=i;
            if (asphalt_along_ray[j]==0)
            {
               int zero_count=0;
               while (asphalt_along_ray[modulo(j,nbins)]==0 && j<i+nbins)
               {
                  zero_count++;
                  j++;
               }
               if (zero_count > max_zero_count)
               {
                  max_zero_count=zero_count;
                  int avg_j=modulo(basic_math::round(0.5*(i+j)),nbins);
                  theta_non_asphalt=theta_start+avg_j*dtheta;    
               }
            }
         } // loop over index i;

// Search for longest string of unity values within asphalt_along_ray
// array:

         int max_one_count=NEGATIVEINFINITY;
         double theta_asphalt=0;
         for (unsigned int i=0; i<nbins; i++)
         {
            unsigned int j=i;
            if (asphalt_along_ray[j]==1)
            {
               int one_count=0;
               while (asphalt_along_ray[modulo(j,nbins)]==1 && j<i+nbins)
               {
                  one_count++;
                  j++;
               }
               if (one_count > max_one_count)
               {
                  max_one_count=one_count;
                  int avg_j=modulo(basic_math::round(0.5*(i+j)),nbins);
                  theta_asphalt=theta_start+avg_j*dtheta;    
               }
            }
         } // loop over index i;
         
         double theta=0.5*(theta_asphalt+
                           basic_math::phase_to_canonical_interval(
                              theta_non_asphalt+PI,theta_asphalt-PI,
                              theta_asphalt+PI));
         curr_bldg_ptr->set_front_dir(0,threevector(cos(theta),sin(theta)));
      }
   } // loop over nodes in buildings network entries list
   outputfunc::newline();

   delete just_asphalt_twoDarray_ptr;
}

// ==========================================================================
// Algorithm development member functions
// ==========================================================================

// This overloaded version of generate_roadpoints_network is for
// algorithm development purposes only.

void urbanimage::generate_roadpoints_network()
{
   outputfunc::write_banner("Generating fake roadpoints hashtable:");

//   int n_roadpoints=4;
   unsigned int n_roadpoints=5;
//   int n_roadpoints=8;
//   int n_roadpoints=11;
//   int n_roadpoints=15;
   threevector vertex[n_roadpoints];
   
   const threevector origin(Zero_vector);
   const threevector xhat(3,0,0);
   const threevector yhat(0,3,0);

// Nearly intersecting topology:

   vertex[0]=origin+xhat;
   vertex[1]=origin-xhat;
   vertex[2]=vertex[1]-yhat;
   vertex[3]=vertex[2]+xhat;
   vertex[4]=vertex[3]+0.9*yhat;

// Intersecting topology:

//   vertex[0]=origin+xhat+yhat;
//   vertex[1]=origin+xhat-yhat;
//   vertex[2]=origin-xhat-yhat;
//   vertex[3]=origin-xhat+yhat;

/*  
// Branching topology:

   vertex[0]=origin;
   vertex[1]=vertex[0]+xhat;
   vertex[2]=vertex[1]+xhat+yhat;
   vertex[3]=vertex[1]+xhat-yhat;
   vertex[4]=vertex[2]+xhat+yhat;
   vertex[5]=vertex[2]+xhat-yhat;
   vertex[6]=vertex[3]+xhat-yhat;
   vertex[7]=vertex[0]-xhat-yhat;
   vertex[8]=vertex[0]-xhat+yhat;
   vertex[9]=vertex[8]-xhat;
   vertex[10]=vertex[7]-xhat;
*/

// Loop topology:

/*
  vertex[0]=origin+3*yhat;
  vertex[1]=vertex[0]+xhat-yhat;
  vertex[2]=vertex[1]-yhat;
  vertex[3]=vertex[2]-yhat;
  vertex[4]=vertex[3]-yhat;
  vertex[5]=vertex[4]-yhat;
  vertex[6]=vertex[5]-2*xhat;
  vertex[7]=vertex[6]+yhat;
  vertex[8]=vertex[7]+yhat;
  vertex[9]=vertex[8]+yhat;
  vertex[10]=vertex[9]+yhat;
*/

/*
  vertex[0]=origin+xhat;
  vertex[1]=origin+yhat;
  vertex[2]=origin-xhat;
  vertex[3]=origin-yhat;
*/

/*
  vertex[0]=origin+xhat+2*yhat;
  vertex[1]=origin+xhat+yhat;
  vertex[2]=origin+xhat;
  vertex[3]=origin+xhat-yhat;
  vertex[4]=origin-xhat-yhat;
  vertex[5]=origin-xhat;
  vertex[6]=origin-xhat+yhat;
  vertex[7]=origin-xhat+2*yhat;
*/

// "Mickey Mouse"

/*
   vertex[0]=origin-2*xhat+2*yhat;
   vertex[1]=origin-xhat+2*yhat;
   vertex[2]=origin-2*xhat+yhat;
   vertex[3]=origin-xhat+yhat;
   vertex[4]=origin-xhat;
   vertex[5]=origin-xhat-yhat;
   vertex[6]=origin-yhat;
   vertex[7]=origin;
   vertex[8]=origin+yhat;
   vertex[9]=origin+0.5*xhat+yhat;
   vertex[10]=origin+xhat+0.5*yhat;
   vertex[11]=origin+1.5*xhat+1.5*yhat;
   vertex[12]=origin+2.5*xhat+yhat;
   vertex[13]=origin+xhat;
   vertex[14]=origin+xhat-yhat;
*/

   roadpoints_network_ptr=new Network<roadpoint*>(2*n_roadpoints);

   for (unsigned int n=0; n<n_roadpoints; n++)
   {
      roadpoint* curr_newroadpoint_ptr=new roadpoint(n,vertex[n]);
      roadpoints_network_ptr->insert_site(
         n,Site<roadpoint*>(curr_newroadpoint_ptr));
   } 

// Next establish links between roadpoints in hashtable:

// Nearly intersecting topology:

  roadpoints_network_ptr->add_symmetric_link(0,1);
  roadpoints_network_ptr->add_symmetric_link(1,2);
  roadpoints_network_ptr->add_symmetric_link(2,3);
  roadpoints_network_ptr->add_symmetric_link(3,4);

// Intersecting topology:

//  roadpoints_network_ptr->add_symmetric_link(0,1);
//  roadpoints_network_ptr->add_symmetric_link(1,2);
//  roadpoints_network_ptr->add_symmetric_link(2,3);
//  roadpoints_network_ptr->add_symmetric_link(3,0);
//  roadpoints_network_ptr->add_symmetric_link(2,0);
//  roadpoints_network_ptr->add_symmetric_link(3,1);

/*
// Branching topology:

  roadpoints_network_ptr->add_symmetric_link(0,1);
  roadpoints_network_ptr->add_symmetric_link(1,2);
  roadpoints_network_ptr->add_symmetric_link(2,4);
  roadpoints_network_ptr->add_symmetric_link(2,5);
  roadpoints_network_ptr->add_symmetric_link(1,3);
  roadpoints_network_ptr->add_symmetric_link(3,6);
  roadpoints_network_ptr->add_symmetric_link(0,8);
  roadpoints_network_ptr->add_symmetric_link(8,9);
  roadpoints_network_ptr->add_symmetric_link(0,7);
  roadpoints_network_ptr->add_symmetric_link(7,10);

// roadpoints_network_ptr->add_symmetric_link(5,6);
// roadpoints_network_ptr->add_symmetric_link(3,5);
*/

// Loop topology:

/*
  roadpoints_network_ptr->add_symmetric_link(0,1);
  roadpoints_network_ptr->add_symmetric_link(1,2);
  roadpoints_network_ptr->add_symmetric_link(2,3);
  roadpoints_network_ptr->add_symmetric_link(3,4);
  roadpoints_network_ptr->add_symmetric_link(4,5);
  roadpoints_network_ptr->add_symmetric_link(5,6);
  roadpoints_network_ptr->add_symmetric_link(6,7);
  roadpoints_network_ptr->add_symmetric_link(7,8);
  roadpoints_network_ptr->add_symmetric_link(8,9);
  roadpoints_network_ptr->add_symmetric_link(9,10);
  roadpoints_network_ptr->add_symmetric_link(10,0);

  roadpoints_network_ptr->add_symmetric_link(9,2);
*/

/*
  roadpoints_network_ptr->add_symmetric_link(0,1);
  roadpoints_network_ptr->add_symmetric_link(1,2);
  roadpoints_network_ptr->add_symmetric_link(2,3);
  roadpoints_network_ptr->add_symmetric_link(3,0);
  roadpoints_network_ptr->add_symmetric_link(0,2);
*/

/*
  roadpoints_network_ptr->add_symmetric_link(0,1);
  roadpoints_network_ptr->add_symmetric_link(1,2);
  roadpoints_network_ptr->add_symmetric_link(2,3);
  roadpoints_network_ptr->add_symmetric_link(3,4);
  roadpoints_network_ptr->add_symmetric_link(4,5);
  roadpoints_network_ptr->add_symmetric_link(5,6);
  roadpoints_network_ptr->add_symmetric_link(6,7);
  roadpoints_network_ptr->add_symmetric_link(7,0);
  roadpoints_network_ptr->add_symmetric_link(6,1);
*/

// Mickey Mouse:
/*
   roadpoints_network_ptr->add_symmetric_link(0,1);
   roadpoints_network_ptr->add_symmetric_link(1,3);
   roadpoints_network_ptr->add_symmetric_link(3,2);
   roadpoints_network_ptr->add_symmetric_link(2,0);
   roadpoints_network_ptr->add_symmetric_link(3,4);
   roadpoints_network_ptr->add_symmetric_link(4,5);
   roadpoints_network_ptr->add_symmetric_link(5,6);
   roadpoints_network_ptr->add_symmetric_link(6,14);
   roadpoints_network_ptr->add_symmetric_link(14,13);
   roadpoints_network_ptr->add_symmetric_link(13,10);

   roadpoints_network_ptr->add_symmetric_link(10,9);
   roadpoints_network_ptr->add_symmetric_link(9,8);
   roadpoints_network_ptr->add_symmetric_link(8,3);
   roadpoints_network_ptr->add_symmetric_link(9,11);
   roadpoints_network_ptr->add_symmetric_link(11,12);
   roadpoints_network_ptr->add_symmetric_link(12,10);
*/

   cout << "Fake roadpoint network = " << endl;
   cout << *roadpoints_network_ptr << endl;
}

// ---------------------------------------------------------------------
// Auxilliary member function redraw_building_site_pixels recolors
// building rooftop pixels following checking of building height
// variations along shrink wrapped contours.  In cases where clumps of
// pixels buried within large tree clusters were originally
// misclassified as buildings, this method illustrates their
// classification following the radial gradient height variation
// check.  This method was concocted for algorithm design purposes
// only...

twoDarray* urbanimage::redraw_building_site_pixels(
   twoDarray const *features_twoDarray_ptr)
{
   outputfunc::write_banner("Redrawing building site pixels:");   
   twoDarray* new_features_twoDarray_ptr=new twoDarray(
      features_twoDarray_ptr);
   features_twoDarray_ptr->copy(new_features_twoDarray_ptr);
   
   for (Mynode<int>* currnode_ptr=buildings_network_ptr->
           get_entries_list_ptr()->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int n=currnode_ptr->get_data();
      linkedlist* pixel_list_ptr=get_building_ptr(n)->get_pixel_list_ptr();
      
      if (pixel_list_ptr != NULL)
      {

// FAKE FAKE cluge cluge...7/12/04 at 1:48 pm

         double building_value=get_building_ptr(n)->get_Imin();
         mynode* curr_pixel_ptr=pixel_list_ptr->get_start_ptr();
         while (curr_pixel_ptr != NULL)
         {
            int px=basic_math::round(curr_pixel_ptr->get_data().get_var(0));
            int py=basic_math::round(curr_pixel_ptr->get_data().get_var(1));
            new_features_twoDarray_ptr->put(px,py,building_value);
            curr_pixel_ptr=curr_pixel_ptr->get_nextptr();
         }
      } // pixel_list_ptr != NULL conditional
      else
      {
         cout << "pixel_list_ptr = NULL for building n = " << n << " !! " 
              << endl;
      }
   } // loop over sites in buildings network

   return new_features_twoDarray_ptr;
}

// ==========================================================================
void urbanimage::summarize_results()
{
   string data_filename=imagedir+"results.summary";
   ofstream datastream;
   
   filefunc::openfile(data_filename,datastream);
   outputfunc::write_initial_results_info(datastream);
   datastream << "Input xyz filename = " << xyz_filenamestr << endl;
   datastream << "Output directory containing all system results = "
              << imagedir << endl << endl;
   datastream << "xorigin " << image_origin.get(0) 
              << " yorigin = " << image_origin.get(1)
              << " zorigin = " << image_origin.get(2) 
              << " measured relative to HAFB"
              << endl;

   datastream << "Number of entries in buildings network = " 
              << get_n_buildings() << endl;

   outputfunc::report_processing_time_info(
      start_processing_time,cout);
   outputfunc::report_processing_time_info(
      start_processing_time,datastream);
   filefunc::closefile(data_filename,datastream);
}


