// ==========================================================================
// Header file for standalong urban object manipulation methods
// ==========================================================================
// Last modified on 4/18/05; 12/8/05; 7/29/06; 10/3/09; 4/5/14
// ==========================================================================

#ifndef URBANFUNCS_H
#define URBANFUNCS_H

#include <string>
#include <vector>
#include "image/connectfuncs.h"
#include "geometry/contour.h"
#include "datastructures/datapoint.h"
#include "threeDgraphics/draw3Dfuncs.h"
#include "ladar/featurefuncs.h"
#include "kdtree/kdtree.h"
#include "network/Network.h"
#include "geometry/polyhedron.h"
#include "math/threevector.h"
#include "image/TwoDarray.h"
#include "threeDgraphics/xyzpfuncs.h"

class building;
class linesegment;
template <class T> class Linkedlist;
typedef Linkedlist<datapoint> linkedlist;
class oriented_box;
class roadpoint;
class tree_cluster;
typedef TwoDarray<double> twoDarray;

namespace urbanfunc
{

// Rooftop abstraction methods:

   void clean_rooftop_pixels(linkedlist*& pixel_list_ptr);
   int isolate_and_clean_rooftop_pixels(
      linkedlist *translated_pixel_list_ptr,std::string xyzp_filename,
      double& zlo,double& zhi,
      double*& xnew,double*& ynew,double*& znew,double*& pnew);
   void rooftop_height_limits(unsigned int npixels,double z[],double& zlo,double& zhi);
   double rooftop_symmetry_directions(
      int n,unsigned int npixels,
      double xnew[],double ynew[],double znew[],double pnew[],
      twoDarray* binary_mask_twoDarray_ptr,double zlo,double zhi,
      std::string filenamestr);
   int rooftop_spine_pixel_height_interval(
      unsigned int npixels,double z_roof[],double& min_height,double& max_height);
   int rooftop_spine_pixel_height_interval(
      const std::vector<double>& subregion_height,
      double& min_height,double& max_height);

   threevector rooftop_spine_COM(
      unsigned int npixels,double min_height,double max_height,
      double x_roof[],double y_roof[],
      double z_roof[],double p_roof[],twoDarray* ftwoDarray_ptr);
   threevector rooftop_spine_COM(
      unsigned int npixels,double min_height,double max_height,
      double max_transverse_dist,const threevector& old_rooftop_COM,
      const threevector& Imax_hat,double x_roof[],double y_roof[],
      double z_roof[],double p_roof[],twoDarray* ftwoDarray_ptr);
   threevector rooftop_spine_COM(
      double min_height,double max_height,
      const std::vector<threevector>& xy_translated_subregion_point,
      twoDarray* ftwoDarray_ptr);
   threevector rooftop_spine_COM(
      double min_height,double max_height,double max_transverse_dist,
      const threevector& old_rooftop_COM,const threevector& Imax_hat,
      const std::vector<threevector>& xy_translated_subregion_point,
      twoDarray* ftwoDarray_ptr);
   double compute_initial_rooftop_moi_ratio(
      unsigned int npixels,double x_roof[],double y_roof[],double z_roof[],
      double p_roof[],twoDarray* ftwoDarray_ptr,threevector& rooftop_COM);
   double compute_initial_rooftop_moi_ratio(
      std::vector<threevector>& xy_translated_subregion_point,
      std::vector<double>& subregion_height,twoDarray* ftwoDarray_ptr,
      threevector& rooftop_COM);
   bool pyramidal_rooftop_structure(
      unsigned int npixels,double x_roof[],double y_roof[],double z_roof[],
      double p_roof[],std::string xyzp_filename,double moi_ratio,
      double rooftop_pitch_angle,const threevector& rooftop_COM);
   bool pyramidal_rooftop_structure(
      double moi_ratio,double rooftop_pitch_angle,
      const std::vector<threevector>& xy_translated_subregion_point,
      const std::vector<double>& subregion_height,
      const threevector& rooftop_COM);
   linesegment* rooftop_spine_extraction(
      unsigned int npixels,double x_roof[],double y_roof[],double z_roof[],
      double p_roof[],threevector& rooftop_COM,
      std::string xyzp_filename,twoDarray* ftwoDarray_ptr);
   linesegment* rooftop_spine_extraction(
      threevector& rooftop_COM,
      const std::vector<threevector>& xy_translated_subregion_point,
      const std::vector<double>& subregion_height,twoDarray* ftwoDarray_ptr);
   void generate_rooftop_spine_binary_image(
      unsigned int npixels,double min_height,
      double x_roof[],double y_roof[],double z_roof[],double p_roof[],
      double max_transverse_dist,const threevector& COM,
      const threevector& Imax_hat,twoDarray *ftwoDarray_ptr);
   void generate_rooftop_spine_binary_image(
      double min_height,double max_transverse_dist,
      const std::vector<threevector>& xy_translated_subregion_point,
      const threevector& COM,const threevector& Imax_hat,
      twoDarray* ftwoDarray_ptr);
   void plot_projected_rooftop_area(
      std::string filename,double min_theta,linkedlist& proj_area_list);
   double minimal_projected_rooftop_area_orientation(
      unsigned int n_angular_bins,double theta[],double filtered_proj_area[],
      std::string proj_area_filename);
   void filter_projected_rooftop_areas(
      unsigned int n_angular_bins,double dtheta,
      double raw_proj_area[],double filtered_proj_area[]);
   void align_rooftop_and_building_parallelepiped(
      linesegment* spine_ptr,oriented_box* box_3D_ptr);
   void align_box_and_rooftop_spine_orientations(
      oriented_box* oriented_box_ptr);
   void align_bldg_and_all_rooftop_spine_orientations(
      Linkedlist<oriented_box*>* box_list_ptr);

// Building wireframe model methods:

   void construct_building_wireframe_model(
      building* b_ptr,twoDarray const *ztwoDarray_ptr,
      twoDarray const *features_twoDarray_ptr,
      twoDarray* binary_mask_twoDarray_ptr);
   void decompose_rooftop_pixels(
      int n_subregions,const threevector& COM_xy,
      Linkedlist<contour*> const *subcontour_list_ptr,
      linkedlist const *pixel_list_ptr,std::vector<parallelogram>& p,
      std::vector<threevector> xy_translated_subregion_point[],
      std::vector<double> subregion_height[]);
   void determine_subregion_rooftype(
      double zroof_hi,std::vector<threevector>& xy_translated_subregion_point,
      std::vector<double>& subregion_height,
      twoDarray* binary_mask_twoDarray_ptr,oriented_box* oriented_box_ptr);
   double refine_ground_height_estimate(      
      oriented_box const *box_3D_ptr,twoDarray const *ztwoDarray_ptr,
      twoDarray const *features_twoDarray_ptr);
   void adjust_oriented_boxes_relative_to_ground(
      int n_grnd_measurements,double ground_height_sum,
      Linkedlist<oriented_box*>* box_list_ptr);
   polyhedron construct_3D_bldg_bbox(
      double theta,double max_roof_z,const threevector& COM,
      twoDarray* p_roof_binary_twoDarray_ptr);

// Network methods:

   bool link_passes_through_building(
      roadpoint const *curr_roadpoint_ptr,
      roadpoint const *neighbor_roadpoint_ptr,
      twoDarray const *just_bldgs_twoDarray_ptr);
   double frac_link_length_over_asphalt(
      const linesegment& roadlink,
      twoDarray const *just_asphalt_twoDarray_ptr);
   void accumulate_building_heights(
      Network<building*> const *buildings_network_ptr,
      std::vector<double>& building_heights);
   void plot_building_height_distribution(
      const std::vector<double>& building_heights,std::string imagedir);
   void compute_building_height_distribution(
      std::string imagedir,Network<building*> const *buildings_network_ptr);

// Object annotation methods:

   void annotate_building_labels(
      Network<building*> const *buildings_network_ptr,
      std::string xyzp_filename,double annotation_value=
      draw3Dfunc::annotation_value1,bool display_cityblock_IDs=false);
   void annotate_particular_building(
      int n,Network<building*> const *buildings_network_ptr,
      std::string xyzp_filename,double annotation_value=
      draw3Dfunc::annotation_value1);
   void annotate_street_islands_and_peninsulas(
      Network<building*> const *buildings_network_ptr,
      std::string xyzp_filename,double annotation_value=
      draw3Dfunc::annotation_value1);
   void annotate_treecluster_labels(
      Network<tree_cluster*> const *trees_network_ptr,
      std::string xyzp_filename,double annotation_value);

// Coloring methods:

   twoDarray* color_feature_heights(
      twoDarray const *ztwoDarray_ptr,twoDarray const *ftwoDarray_ptr);
   twoDarray* fuse_height_feature_and_prob_images(
      double p_hi,double p_lo,
      twoDarray const *ztwoDarray_ptr,twoDarray const *features_twoDarray_ptr,
      twoDarray const *ptwoDarray_ptr);

// 3D drawing methods:

   void draw_rooftop_structure(
      oriented_box const *oriented_box_ptr,std::string xyzp_filename,
      double annotation_value);
   void draw_3D_buildings(
      Network<building*> const *buildings_network_ptr,
      std::string xyzp_filename,double annotation_value,
      bool gzip_output_file=true);
   void draw_3D_buildings(
      Linkedlist<int> const *bldgs_near_trees_list_ptr,
      Network<building*> const *buildings_network_ptr,
      std::string xyzp_filename,double annotation_value1,
      double annotation_value2,bool gzip_output_file=true);
   void draw_3D_buildings(
      Network<building*> const *buildings_network_ptr,
      twoDarray* ztwoDarary_ptr);
   void draw_particular_3D_building(
      int n,Network<building*> const *buildings_network_ptr,
      std::string xyzp_filename,double annotation_value);
   void draw_particular_3D_building(
      int n,Network<building*> const *buildings_network_ptr,
      twoDarray* ztwoDarray_ptr);
   void draw_3D_building_front_dirs(
      Network<building*> const *buildings_network_ptr,
      std::string xyzp_filename,double annotation_value=
      draw3Dfunc::annotation_value1,bool gzip_output_file=true);

// High-level feature information input/output methods:

   void print_out_particular_building(
      int n,Network<building*> const *buildings_network_ptr,
      std::string output_filename);
   void writeout_building_wall_polys(
      int n,Network<building*> const *buildings_network_ptr,
      std::string output_filename);
   std::vector<polygon>* readin_building_wall_polys(
      std::string input_filename);

// Buildings network text input/output methods:

   void output_buildings_network_to_textfile(
      Network<building*> const *buildings_network_ptr,
      std::string output_filename);
   void output_building_and_its_links(
      Network<building*> const *buildings_network_ptr,
      int n,std::ofstream& outstream);
   Network<building*>* readin_buildings_network_from_textfile(
      std::string input_filename);

// Building-tree spatial relationship methods:
   
   Linkedlist<int>* search_for_trees_near_bldgs(
      Network<building*>* buildings_network_ptr,
      KDTree::KDTree<3, threevector> const *kdtree_ptr,
      double max_bldg_to_tree_dist,
      double min_angle_rel_to_front,double max_angle_rel_to_front,
      double min_tree_height,double max_tree_height);
   bool search_for_trees_near_bldg(
      building* b_ptr,double max_bldg_to_tree_dist,
      double min_angle_rel_to_front,double max_angle_rel_to_front,
      double min_tree_height,double max_tree_height,
      KDTree::KDTree<3, threevector> const *kdtree_ptr);

// ==========================================================================
// Templatized methods
// ==========================================================================

// Method generate_network_contours scans over all sites within input
// network *network_ptr.  For each one, this method converts its pixel
// list into a temporary binary image.  It subsequently computes the
// contour which "hugs" the object's footprint within the xy-plane.
// The contour is stored within a member variable of each site in the
// network.

   template <class T> void generate_network_contours(
      twoDarray const *ztwoDarray_ptr,Network<T*>* network_ptr)
      {

// Allocate a scratch image array:

         twoDarray* fmask_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);

         int n_cluster=0;
         for (Mynode<int>* currnode_ptr=network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int n=currnode_ptr->get_data();
            std::cout << n << " " << std::flush; 

            const linkedlist* pixel_list_ptr=network_ptr->
               get_site_data_ptr(n)->get_pixel_list_ptr();
      
            fmask_twoDarray_ptr->initialize_values(xyzpfunc::null_value);
            connectfunc::convert_pixel_list_to_twoDarray(
               pixel_list_ptr,fmask_twoDarray_ptr);
      
//            std::cout << "Connected component #" 
//                      << n_cluster+1 << " of " << network_ptr->size() 
//                      << " contains " << pixel_list_ptr->size() 
//                      << " pixels" << std::endl;

            contour* feature_contour_ptr=
               featurefunc::generate_feature_cluster_contour(
                  n_cluster,pixel_list_ptr,ztwoDarray_ptr,
                  fmask_twoDarray_ptr);

            network_ptr->get_site_data_ptr(n)->set_contour_ptr(
               feature_contour_ptr);

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

} // urbanfunc namespace

#endif // urbanfuncs.h



