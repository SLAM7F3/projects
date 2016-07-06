// ==========================================================================
// Header file for URBANIMAGE class
// ==========================================================================
// Last modified on 7/5/05; 8/3/06; 10/1/09; 3/6/14
// ==========================================================================

#ifndef URBANIMAGE_H
#define URBANIMAGE_H

#include <iostream>
#include "urban/building.h"
#include "geometry/contour.h"
#include "image/drawfuncs.h"
#include "threeDgraphics/draw3Dfuncs.h"
#include "ladar/featurefuncs.h"
#include "general/filefuncs.h"
#include "ladar/ladarimage.h"
#include "network/Network.h"
#include "network/Networkfuncs.h"
#include "general/outputfuncs.h"
#include "geometry/parallelogram.h"
#include "urban/treefuncs.h"
#include "geometry/voronoifuncs.h"
#include "threeDgraphics/xyzpfuncs.h"

class mybox;
class netlink;
class roadpoint;
template <class T> class Site;
class tree_cluster;
template <class T> class TwoDarray;
typedef TwoDarray<double> twoDarray;

class urbanimage:public ladarimage
{

  public:

// Initialization, constructor and destructor functions:

   urbanimage(void);
   urbanimage(const urbanimage& u);
   urbanimage(int nxbins,int nybins);
   virtual ~urbanimage();
   urbanimage& operator= (const urbanimage& u);

// Set & get member functions:

   void set_buildings_network_ptr(Network<building*>* b_network_ptr);
   int get_n_buildings() const;
   Network<building*>* get_buildings_network_ptr();
   Network<roadpoint*>* get_roadpoints_network_ptr();
   Site<building*>* get_building_site_ptr(int n);
   building* get_building_ptr(int n);
   Site<roadpoint*>* get_roadpoint_site_ptr(int r);
   roadpoint* get_roadpoint_ptr(int r);
   netlink* get_roadlink_ptr(int r,int q);
   std::string get_feature_name(double sentinel_value);
   template <class T> std::string get_feature_name(Network<T*>* network_ptr);

// Network methods:

   void generate_buildings_network(
      double min_footprint_area,twoDarray const *ztwoDarray_ptr,
      twoDarray const *features_twoDarray_ptr);
   void draw_building_site_pixels(
      twoDarray* ftwoDarray_ptr,double annotation_value=
      featurefunc::building_sentinel_value);
   template <class T> void generate_network_contours(
      twoDarray const *ztwoDarray_ptr,Network<T*>* network_ptr);
   void recheck_footprint_areas(
      double min_footprint_area,twoDarray const *ztwoDarray_ptr);

   template <class T> void output_network_posns(Network<T*>* network_ptr);
   template <class T> void draw_network_posns(
      Network<T*>* network_ptr,twoDarray* ftwoDarray_ptr);
   template <class T> void draw_network_contour_cylinders(
      std::string xyzp_filename,Network<T*>* network_ptr,
      double annotation_value=draw3Dfunc::annotation_value1);
   template <class T> void draw_network_subcontour_cylinders(
      std::string xyzp_filename,Network<T*>* network_ptr,
      double annotation_value=draw3Dfunc::annotation_value1);

// Templatized network neighbor methods:

   template <class T> void generate_site_voronoi_regions(
      Network<T*>* network_ptr);
   template <class T> void draw_site_voronoi_regions(
      twoDarray* ftwoDarray_ptr,Network<T*>* network_ptr);
   template <class T> void draw_nearest_neighbor_links(
      twoDarray* ftwoDarray_ptr,Network<T*>* network_ptr);
   template <class T> void draw_neighbor_links(
      int n,twoDarray* ftwoDarray_ptr,Network<T*>* network_ptr);
   template <class T> void draw_neighbor_link(
      int n,int i,twoDarray* ftwoDarray_ptr,Network<T*>* network_ptr);

// Building 2D contour & bounding box methods:

   void check_building_contour_height_variation(
      twoDarray const *xderiv_twoDarray_ptr,
      twoDarray const *yderiv_twoDarray_ptr,
      twoDarray const *features_twoDarray_ptr);
   void score_buildings_contour_edge_fits(
      twoDarray const *features_twoDarray_ptr);
   void construct_rooftop_orthogonal_polygons(
      twoDarray const *features_twoDarray_ptr);
   void simplify_rooftop_orthogonal_polygons();
   void improve_rooftop_sym_dirs();
   void decompose_rooftop_orthogonal_polygons();
   void generate_building_bboxes();
   void draw_building_bboxes(twoDarray* ftwoDarray_ptr);

   twoDarray* generate_twoDarray_for_centered_building(
      double max_x=35,double max_y=35);	// max_x,y params in meters
   double compute_building_orientation(
      int n,twoDarray* binary_mask_twoDarray_ptr);
   void draw_building_orientation(
      int n,double theta,twoDarray* ftwoDarray_ptr);
   void binary_mask_for_centered_building(
      int n,twoDarray*& binary_mask_twoDarray_ptr);
   linkedlist* generate_translated_rooftop_pixel_list(int n);
   void compute_building_bbox(
      int n,double bbox_theta,twoDarray const *ftwoDarray_ptr);
   parallelogram* compute_building_bbox(
      const threevector& COM,twoDarray const *ftwoDarray_ptr,
      double bbox_theta,double edge_fraction_of_median);
   double building_bbox_fit_score(
      int n,parallelogram const *bbox_ptr,
      twoDarray const *fbinary_twoDarray_ptr);
   double optimize_building_bbox_orientation(
      int n,parallelogram* bbox_ptr,twoDarray const *fbinary_twoDarray_ptr);

// Building 3D parallelepiped methods:

   void construct_building_wireframe_models(
      twoDarray const *ztwoDarray_ptr,
      twoDarray const *features_twoDarray_ptr);
   void generate_rooftop_pixels_profiles(
      int n,double min_theta,twoDarray*& binary_mask_twoDarray_ptr,
      double& x_min,double& x_max,double& y_min,double& y_max);
   void print_building_data();
   double compute_tallest_building_height();

// Shadow computation methods:

   void compute_building_shadows(
      const threevector& ray_hat,
      twoDarray const *ztwoDarray_ptr,twoDarray const *features_twoDarray_ptr,
      twoDarray* shadows_twoDarray_ptr);
   void compute_tree_shadows(
      Network<tree_cluster*> const *trees_network_ptr,const threevector& ray_hat,
      twoDarray const *ztwoDarray_ptr,twoDarray const *features_twoDarray_ptr,
      twoDarray* shadows_twoDarray_ptr) const;
   void reset_nonground_feature_values(
      twoDarray const *features_twoDarray_ptr,
      twoDarray* shadows_twoDarray_ptr) const;

// Road network computation methods:

   void generate_roadpoints_network(twoDarray const *ztwoDarray_ptr);
   void delete_roadlinks_too_close_to_buildings(
      twoDarray const *just_asphalt_twoDarray_ptr);
   double min_bldg_distance_to_roadlink(
      roadpoint* roadpoint1_ptr,roadpoint* roadpoint2_ptr,
      linesegment& closest_bldg_roadlink_linesegment);
   double roadlink_distance_to_bldg(
      roadpoint* roadpoint1_ptr,roadpoint* roadpoint2_ptr,
      building* bldg_ptr,threevector& closest_point_on_roadlink);
   void delete_roadlinks_passing_thru_buildings(
      twoDarray const *features_twoDarray_ptr);
   void prune_roadpoints_at_infinity();   
   void delete_lonely_roadpoints(int min_roadpoint_neighbors=1);
   void delete_roadpoints_too_far_from_asphalt(
      twoDarray const *ptwoDarray_ptr);
   double shortest_distance_between_roadpoints(int r,int q);
   void delete_roadlinks_behind_buildings(
      unsigned int n_rear_angle_zones,double half_rear_angle_extent,
      double min_distance_to_data_bbox,
      twoDarray const *just_asphalt_twoDarray_ptr);
   void mark_roadpoints_in_front_of_buildings(
      twoDarray const *just_asphalt_twoDarray_ptr);
   void identify_front_roadpoint_seeds();
   void propagate_roadpoint_frontness();
   void enumerate_roadpoint_neighbors(int r);

// Road network display methods:

   void draw_nearby_buildings_to_roadpoint(int r,twoDarray* ftwoDarray_ptr);
   void draw_bldg_roadpoint_link(int n,int r,twoDarray* ftwoDarray_ptr);
   void draw_bldg_point_link(
      int n,const threevector& currpnt,twoDarray* ftwoDarray_ptr);
   void draw_min_bldg_distances_to_roadlinks(twoDarray* ftwoDarray_ptr);

// Road extraction methods:

   void road_match_filter(
      std::string imagedir,twoDarray const *ztwoDarray_ptr,
      twoDarray const *features_twoDarray_ptr,
      twoDarray const *binary_seed_twoDarray_ptr);
   void compute_bbox_road_frac(
      parallelogram& bbox,twoDarray const *ftwoDarray_ptr,
      twoDarray const *binary_seed_twoDarray_ptr,
      twoDarray* road_twoDarray_ptr,twoDarray* tree_twoDarray_ptr,
      twoDarray* grass_twoDarray_ptr,twoDarray* null_twoDarray_ptr);
   bool compute_feature_fracs_inside_bbox(
      unsigned int min_px,unsigned int max_px,
      unsigned int min_py,unsigned int max_py,
      twoDarray const *frot_expand_twoDarray_ptr,
      double& tree_frac,double& grass_frac,double& road_frac,
      double& null_frac);
   void accumulate_roadside_info(
      double bbox_rotation_angle,
      twoDarray const *ftwoDarray_ptr,twoDarray const *fexpand_twoDarray_ptr,
      twoDarray const *road_rot_expand_twoDarray_ptr,
      twoDarray* road_summary_twoDarray_ptr,
      twoDarray* road_direction_twoDarray_ptr);

// Combined building and road network methods:

   void add_roadpoint_to_bldg_list(int r,int n);
   void add_bldg_to_roadpoint_list(int n,int r);
   void compute_asphalt_distribution_relative_to_buildings(
      twoDarray const *features_twoDarray_ptr);
   void compute_asphalt_angular_distribution_relative_to_buildings(
      twoDarray const *features_twoDarray_ptr);

// Algorithm develpoment methods:

   void generate_roadpoints_network();
   twoDarray* redraw_building_site_pixels(
      twoDarray const *features_twoDarray_ptr);
   void summarize_results();

  protected:

  private:

// Network *buildings_network_ptr holds building connectivity
// information:

   Network<building*>* buildings_network_ptr;

// Network *roadpoints_network_ptr holds road network information:

   Network<roadpoint*>* roadpoints_network_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const urbanimage& u);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void urbanimage::set_buildings_network_ptr(
   Network<building*>* b_network_ptr)
{
   buildings_network_ptr=b_network_ptr;
}

inline int urbanimage::get_n_buildings() const
{
   if (buildings_network_ptr != NULL)
   {
      return buildings_network_ptr->size();
   }
   else
   {
      return 0;
   }
}

inline Network<building*>* urbanimage::get_buildings_network_ptr()
{
   return buildings_network_ptr;
}

inline Network<roadpoint*>* urbanimage::get_roadpoints_network_ptr()
{
   return roadpoints_network_ptr;
}

inline Site<building*>* urbanimage::get_building_site_ptr(int n)
{
   return buildings_network_ptr->get_site_ptr(n);
}

inline building* urbanimage::get_building_ptr(int n)
{
   return buildings_network_ptr->get_site_data_ptr(n);
}

inline Site<roadpoint*>* urbanimage::get_roadpoint_site_ptr(int r)
{
   return roadpoints_network_ptr->get_site_ptr(r);
}

inline roadpoint* urbanimage::get_roadpoint_ptr(int r)
{
   return roadpoints_network_ptr->get_site_data_ptr(r);
}

inline netlink* urbanimage::get_roadlink_ptr(int r,int q)
{
   return roadpoints_network_ptr->get_netlink_ptr(r,q);
}

// ==========================================================================
// Templatized methods
// ==========================================================================

template <class T> std::string urbanimage::get_feature_name(
   Network<T*>* network_ptr)
{
   if (buildings_network_ptr==(Network<building*>*) network_ptr)
   {
      return "bldg";
   }
   else if (roadpoints_network_ptr==(Network<roadpoint*>*) network_ptr)
   {
      return "road";
   }
   else if (treefunc::trees_network_ptr==
            (Network<tree_cluster*>*) network_ptr)
   {
      return "tree";
   }
   else
   {
      return "unknown";
   }
}

// ---------------------------------------------------------------------
// Member function generate_network_contours scans over all sites
// within input network *network_ptr.  For each one, this method
// converts its pixel list into a temporary binary image.  It
// subsequently computes the contour which "hugs" the object's
// footprint within the xy-plane.  The contour is stored within a
// member variable of each site in the network.

template <class T> void urbanimage::generate_network_contours(
   twoDarray const *ztwoDarray_ptr,Network<T*>* network_ptr)
{
   std::string feature_type=get_feature_name(network_ptr);
   outputfunc::write_banner("Generating "+feature_type+" contours:");   

// Allocate a scratch image array:

   twoDarray* fmask_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);

   int n_cluster=0;
   for (Mynode<int>* currnode_ptr=network_ptr->
           get_entries_list_ptr()->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int n=currnode_ptr->get_data();
      std::cout << n << " " << std::flush; 

      const linkedlist* pixel_list_ptr=network_ptr->get_site_data_ptr(n)->
         get_pixel_list_ptr();
      
      fmask_twoDarray_ptr->initialize_values(xyzpfunc::null_value);
      connectfunc::convert_pixel_list_to_twoDarray(
         pixel_list_ptr,fmask_twoDarray_ptr);
      
//      std::cout << "Connected "+feature_type+" component #" 
//           << n_cluster+1 << " of " << network_ptr->size() 
//           << " contains " << pixel_list_ptr->size() 
//           << " pixels" << std::endl;

      contour* feature_contour_ptr=
         featurefunc::generate_feature_cluster_contour(
            n_cluster,pixel_list_ptr,ztwoDarray_ptr,fmask_twoDarray_ptr);
      network_ptr->get_site_data_ptr(n)->set_contour_ptr(feature_contour_ptr);

// Initially set center points of all feature clusters equal to the
// contour origins which should generally equal the COMs of the
// largest triangles in the triangulation of each cluster.  Later on,
// building centers, for example, are reset to equal the midpoints of
// the parallelepipeds used to model building structures...

      network_ptr->get_site_data_ptr(n)->set_center(
         feature_contour_ptr->get_origin());

      n_cluster++;
   } // loop over nodes in network entries list
   std::cout << std::endl;

   delete fmask_twoDarray_ptr;
}

// ---------------------------------------------------------------------
// Method output_network_posns loops over all of the entries within
// *network_ptr and writes network sites' position to an ascii output
// file.

template <class T> inline void urbanimage::output_network_posns(
   Network<T*>* network_ptr)
{
   std::string filenamestr=imagedir+get_feature_name(network_ptr)+".posns";
   Networkfunc::output_site_posns(filenamestr,network_ptr);
}

// ---------------------------------------------------------------------
// Method draw_network_posns loops over all sites within input network
// *network_ptr and draws their positions onto output twoDarray
// *ftwoDarray_ptr as white points.

template <class T> void urbanimage::draw_network_posns(
   Network<T*>* network_ptr,twoDarray* ftwoDarray_ptr)
{
   const double radius=1.0;	 // meter

   for (Mynode<int>* currnode_ptr=network_ptr->
           get_entries_list_ptr()->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int n=currnode_ptr->get_data();
      drawfunc::draw_hugepoint(
         network_ptr->get_site_data_ptr(n)->get_posn(),radius,
         colorfunc::white,ftwoDarray_ptr);
   } // loop over nodes in network entries list
}

// ---------------------------------------------------------------------
// Method draw_network_contour_cylinders loops over all sites within
// input network *network_ptr and draws their contour cylinders onto
// the binary xyzp file specified by input string xyzp_filename.

template <class T> void urbanimage::draw_network_contour_cylinders(
   std::string xyzp_filename,Network<T*>* network_ptr,
   double annotation_value)
{
   filefunc::gunzip_file_if_gzipped(xyzp_filename);
   for (Mynode<int>* currnode_ptr=network_ptr->
           get_entries_list_ptr()->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int n=currnode_ptr->get_data();
      const contour* curr_contour_ptr=network_ptr->get_site_data_ptr(n)->
         get_contour_ptr();

// Next conditional for 2006 ISDS abstract preparation only...

//      if (n != 169 && n != 60 && n != 202 && n != 147 &&
//          n != 191 && n != 106 && n != 112 && n != 185 && n != 88)
//      {
         draw3Dfunc::draw_contour_cylinder(
            *curr_contour_ptr,xyzp_filename,annotation_value);
//      }

   } // loop over nodes in network entries list
//   filefunc::gzip_file_if_gunzipped(xyzp_filename);
}

template <class T> void urbanimage::draw_network_subcontour_cylinders(
   std::string xyzp_filename,Network<T*>* network_ptr,
   double annotation_value)
{
   filefunc::gunzip_file_if_gzipped(xyzp_filename);
   for (Mynode<int>* currnode_ptr=network_ptr->
           get_entries_list_ptr()->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int n=currnode_ptr->get_data();
      const Linkedlist<contour*>* subcontour_list_ptr=
         network_ptr->get_site_data_ptr(n)->get_subcontour_list_ptr();

      for (const Mynode<contour*>* currnode_ptr=subcontour_list_ptr->
              get_start_ptr(); currnode_ptr != NULL; 
           currnode_ptr=currnode_ptr->get_nextptr())
      {
         const contour* subcontour_ptr=currnode_ptr->get_data();
         draw3Dfunc::draw_contour_cylinder(
            *subcontour_ptr,xyzp_filename,annotation_value);
      } // loop over subcontour nodes within linked list
   } // loop over nodes in network entries list
//   filefunc::gzip_file_if_gunzipped(xyzp_filename);
}

// ==========================================================================
// Templatized network neighbor methods:
// ==========================================================================

// Method generate_site_voronoi_regions fills up a site array with COM
// locations.  It dynamically generates a linked list of polygons
// corresponding to each site's Voronoi region.  This method assigns
// each site object's voronoi_region_ptr polygon pointers to the data
// members in this polygonal linked list.

template <class T> void urbanimage::generate_site_voronoi_regions(
   Network<T*>* network_ptr)
{
   std::string feature_type=get_feature_name(network_ptr);
   outputfunc::write_banner(
      "Generating "+feature_type+" Voronoi regions:");

   std::vector<threevector> site=Networkfunc::generate_site_posn_array(
      network_ptr);
   Linkedlist<polygon>* region_poly_list_ptr=
      voronoifunc::generate_voronoi_regions(site);

   Mynode<polygon>* polynode_ptr=region_poly_list_ptr->get_start_ptr();
   for (Mynode<int>* currnode_ptr=network_ptr->
           get_entries_list_ptr()->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int n=currnode_ptr->get_data();
      std::cout << n << " " << std::flush;
      T* curr_data_ptr=network_ptr->get_site_data_ptr(n);

      polygon* voronoi_poly_ptr=new polygon(polynode_ptr->get_data());

// Set the Voronoi polygon's origin to equal its COM location,
// provided it lies within the ladar data bounding box.  If not, set
// the origin equal to the site object's COM:

      threevector origin(voronoi_poly_ptr->compute_COM());
      if (!data_bbox_ptr->point_inside_polygon(origin))
      {
         origin=curr_data_ptr->get_posn();
      }
      voronoi_poly_ptr->set_origin(origin);
      curr_data_ptr->set_voronoi_region_ptr(voronoi_poly_ptr);

      polynode_ptr=polynode_ptr->get_nextptr();
   } // loop over nodes in network entries list
   delete region_poly_list_ptr;
   outputfunc::newline();
}

// ---------------------------------------------------------------------
// Method draw_site_voronoi_regions loops through every site within the
// input network *network_ptr.  It assumes that the Voronoi region for
// each building has already been allocated and assigned.  This method
// simply draws the polygon for each building onto output twoDarray
// *ftwoDarray_ptr.

template <class T> void urbanimage::draw_site_voronoi_regions(
   twoDarray* ftwoDarray_ptr,Network<T*>* network_ptr)
{
   for (Mynode<int>* currnode_ptr=network_ptr->
           get_entries_list_ptr()->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int n=currnode_ptr->get_data();
      T* curr_data_ptr=network_ptr->get_site_data_ptr(n);
      polygon curr_poly(*(curr_data_ptr->get_voronoi_region_ptr()));

//      double curr_intensity=0.1*modulo(1+5*(n+1),11);
//      drawfunc::color_polygon_interior(
//         curr_poly,curr_intensity,ftwoDarray_ptr);
      const double point_radius=1;	// meter
      drawfunc::draw_thick_polygon(curr_poly,colorfunc::white,
                                   point_radius,ftwoDarray_ptr);
   } // loop over nodes in buildings network entries list
}

// ---------------------------------------------------------------------
// Method draw_nearest_neighbor_links loops over the all sites within
// *network_ptr. It draws linesegment links between each site and its
// nearest neighbors onto output twoDarray *ftwoDarray_ptr.

template <class T> void urbanimage::draw_nearest_neighbor_links(
   twoDarray* ftwoDarray_ptr,Network<T*>* network_ptr)
{
   if (network_ptr != NULL)
   {
      draw_network_posns(network_ptr,ftwoDarray_ptr);
      for (Mynode<int>* currnode_ptr=network_ptr->
              get_entries_list_ptr()->get_start_ptr(); 
           currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
      {
         int n=currnode_ptr->get_data();
         draw_neighbor_links(n,ftwoDarray_ptr,network_ptr);
      } // loop over nodes in buildings network entries list
   }
}

// ---------------------------------------------------------------------
// Method draw_neighbor_links takes in ID label n for some site in
// *network_pt.  It draws all of the links between this site and its
// nearest neighbors onto output twoDarray *ftwoDarray_ptr.

template <class T> void urbanimage::draw_neighbor_links(
   int n,twoDarray* ftwoDarray_ptr,Network<T*>* network_ptr)
{
   Site<T*>* curr_site_ptr=network_ptr->get_site_ptr(n);
   if (curr_site_ptr != NULL)
   {
      Linkedlist<netlink>* curr_link_list_ptr
         =curr_site_ptr->get_netlink_list_ptr();

      if (curr_link_list_ptr != NULL)
      {
         Mynode<netlink>* link_node_ptr=
            curr_link_list_ptr->get_start_ptr();

         while (link_node_ptr != NULL)
         {
            int i=link_node_ptr->get_data().get_ID();
            draw_neighbor_link(n,i,ftwoDarray_ptr,network_ptr);
            link_node_ptr=link_node_ptr->get_nextptr();
         } // link_node_ptr != NULL conditional
      }
   }
}

// ---------------------------------------------------------------------
// Member function draw_neighbor_link takes in ID labels n and i for
// two neighboring network sites.  It draws a line segment between
// them onto output twoDarray *ftwoDarray_ptr.

template <class T> void urbanimage::draw_neighbor_link(
   int n,int i,twoDarray* ftwoDarray_ptr,Network<T*>* network_ptr)
{
   linesegment l(network_ptr->get_site_data_ptr(n)->get_posn(),
                 network_ptr->get_site_data_ptr(i)->get_posn());
   const double radius=0.75;	// meter
   drawfunc::draw_thick_line(l,colorfunc::pink,radius,ftwoDarray_ptr);
//   drawfunc::draw_thick_line(l,colorfunc::white,radius,ftwoDarray_ptr);
}

#endif // urbanimage.h



