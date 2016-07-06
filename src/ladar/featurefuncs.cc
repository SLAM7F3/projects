// ==========================================================================
// FEATUREFUNCS stand-alone methods
// ==========================================================================
// Last modified on 10/3/09; 1/15/10; 12/4/10; 3/6/14
// ==========================================================================

#include <set>
#include <vector>
#include "image/binaryimagefuncs.h"
#include "image/connectfuncs.h"
#include "math/constants.h"
#include "geometry/contour.h"
#include "geometry/convexhull.h"
#include "image/drawfuncs.h"
#include "threeDgraphics/draw3Dfuncs.h"
#include "ladar/featurefuncs.h"
#include "image/graphicsfuncs.h"
#include "ladar/groundfuncs.h"
#include "datastructures/Hashtable.h"
#include "image/imagefuncs.h"
#include "ladar/ladarfuncs.h"
#include "ladar/ladarimage.h"
#include "datastructures/Linkedlist.h"
#include "datastructures/Mynode.h"
#include "math/mypolynomial.h"
#include "templates/mytemplates.h"
#include "general/outputfuncs.h"
#include "geometry/parallelogram.h"
#include "geometry/polygon.h"
#include "math/prob_distribution.h"
#include "image/recursivefuncs.h"
#include "geometry/regular_polygon.h"
#include "general/stringfuncs.h"
#include "video/texture_rectangle.h"
#include "image/TwoDarray.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ofstream;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

namespace featurefunc
{

// New sentinel values which yield "greenish" colors for trees and
// grass under JET+white maps and hue+value colormaps within the Group
// 94/106 dataviewer:

   const double low_tree_sentinel_value=0.9;
   const double building_sentinel_value=0.65;
   const double tree_sentinel_value=0.55;
   const double grass_sentinel_value=0.45;
   const double road_sentinel_value=0.15;
   const double shadow_sentinel_value=0.03;

// ==========================================================================
// General feature extraction methods
// ==========================================================================

// Method mark_pixels_in_list takes in a linkedlist which is assumed
// to contain some collection of pixels.  It colors each of these
// pixels in image *ftwoDarray_ptr with the input parameter
// intensity_value.

   void mark_pixels_in_list(
      double intensity_value,linkedlist const *currlist_ptr,
      twoDarray *ftwoDarray_ptr)
      {
         if (currlist_ptr != NULL)
         {
            mynode const *curr_pixel_ptr=currlist_ptr->get_start_ptr();
            while (curr_pixel_ptr != NULL)
            {
               unsigned int px=basic_math::round(curr_pixel_ptr->get_data().
                                                 get_var(0));
               unsigned int py=basic_math::round(curr_pixel_ptr->get_data().
                                                 get_var(1));
               ftwoDarray_ptr->put(px,py,intensity_value);
               curr_pixel_ptr=curr_pixel_ptr->get_nextptr();
            }
         }
      }

// ---------------------------------------------------------------------
// Method update_feature_map takes in sentinel values for tree,
// building, grass and roadside features.  It also takes in the coarse
// feature map within twoDarray *features_twoDarray_ptr along with
// refined building rooftop information within twoDarray
// *p_refined_roof_twoDarray_ptr.  This method returns a dynamically
// generated, refined feature map in which low-lying grass and road
// side feature information is identical to that within
// *features_twoDarray_ptr.  But rooftop information is forced to
// agree with that in *p_refined_roof_twoDarray_ptr.  Every pixel
// which is not classified as road, grass, or building rooftop is
// declared to be a "tree" in the output twoDarray.

   twoDarray* update_feature_map(
      twoDarray const *features_twoDarray_ptr,
      twoDarray const *p_refined_roof_twoDarray_ptr)
      {
         outputfunc::write_banner("Updating feature map:");         

         twoDarray* new_features_twoDarray_ptr=new twoDarray(
            features_twoDarray_ptr);
         for (unsigned int px=0; px<p_refined_roof_twoDarray_ptr->get_mdim(); 
              px++)
         {
            for (unsigned int py=0; py<p_refined_roof_twoDarray_ptr->
                    get_ndim(); py++)
            {
               double curr_feature=features_twoDarray_ptr->get(px,py);
               if (nearly_equal(curr_feature,grass_sentinel_value)
                   ||
                   nearly_equal(curr_feature,road_sentinel_value))
               {
                  new_features_twoDarray_ptr->put(px,py,curr_feature);
               }
               else
               {
                  double curr_p=p_refined_roof_twoDarray_ptr->get(px,py);
                  if (nearly_equal(
                     curr_p,building_sentinel_value))
                  {
                     new_features_twoDarray_ptr->put(
                        px,py,building_sentinel_value);
                  }
                  else
                  {
                     new_features_twoDarray_ptr->put(
                        px,py,tree_sentinel_value);
                  }
               }
            } // loop over py index
         } // loop over px index
         return new_features_twoDarray_ptr;
      }

// ---------------------------------------------------------------------
// Method refine_tree_features attempts to distinguish tall trees from
// short bushes.  It sets the coloring of the latter within feature
// map twoDarray *features_twoDarray_ptr to
// low_tree_sentinel_value.

   void refine_tree_features(
      twoDarray const *ztwoDarray_ptr,twoDarray *features_twoDarray_ptr)
      {
         outputfunc::write_banner("Refining tree information:");         

         double max_bush_height=3;	// meters
         for (unsigned int px=0; px<features_twoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<features_twoDarray_ptr->get_ndim(); 
                 py++)
            {
               double curr_feature=features_twoDarray_ptr->get(px,py);
               if (curr_feature==tree_sentinel_value)
               {
                  double curr_z=ztwoDarray_ptr->get(px,py);
                  if (curr_z < max_bush_height)
                  {
                     features_twoDarray_ptr->put(
                        px,py,low_tree_sentinel_value);
                  }
               }
            } // loop over py index
         } // loop over px index
      }

// ---------------------------------------------------------------------
// Method feature_nearby generates a hexagon whose radius and center
// location are passed as inputs.  It then performs a brute force
// search over this hexagon for any pixel whose value within input
// twoDarray *features_twoDarray_ptr matches that specified by input
// parameter feature_value.  If it finds at least one such matching
// pixel, this boolean method returns true.  

   bool feature_nearby(
      double radius,const threevector& posn,double feature_value,
      const twoDarray* features_twoDarray_ptr)
      {

// Instantiate hexagon to act as a poor-man's circle:

         regular_polygon hexagon(6,radius);
         hexagon.absolute_position(posn);
      
         unsigned int min_px,max_px,min_py,max_py;
         features_twoDarray_ptr->locate_extremal_xy_pixels(
            hexagon,min_px,min_py,max_px,max_py);

         threevector currpoint;
         for (unsigned int px=min_px; px<max_px; px++)
         {
            for (unsigned int py=min_py; py<max_py; py++)
            {
               features_twoDarray_ptr->pixel_to_point(px,py,currpoint);
               if (hexagon.point_inside_polygon(currpoint))
               {
                  double curr_feature_value=
                     features_twoDarray_ptr->get(px,py);
                  if (curr_feature_value==feature_value)
                  {
                     return true;
                  }
               }
            } // loop over py index
         } // loop over px index
         return false;
      }

// ---------------------------------------------------------------------
// Method remove_isolated_height_outliers_from_feature_map resets the
// heights of outliers within asphalt, grass, tree and building
// sections of the input feature map contained in twoDarray
// *ftwoDarray_ptr equal to local means calculated from just similarly
// classified pixels.

   void remove_isolated_height_outliers_from_feature_map(
      parallelogram const *data_bbox_ptr,twoDarray* ztwoDarray_ptr,
      twoDarray const *ftwoDarray_ptr)
      {
         const int xsize=11;
         const int ysize=11;
         const int min_nonnull_pnts=basic_math::round(0.33*(xsize*ysize));
         const int max_pixels_changed=2;
         const double min_height_separation=1;	// meters
         const double max_variance_factor=sqr(2.5);
         
         int npixels_changed=-1;
         do
         {
            npixels_changed=ladarfunc::remove_isolated_outliers(
               road_sentinel_value,data_bbox_ptr,
               ztwoDarray_ptr,ftwoDarray_ptr,xsize,ysize,min_nonnull_pnts,
               min_height_separation,max_variance_factor);
         }
         while (npixels_changed > max_pixels_changed);

         do
         {
            npixels_changed=ladarfunc::remove_isolated_outliers(
               grass_sentinel_value,data_bbox_ptr,
               ztwoDarray_ptr,ftwoDarray_ptr,xsize,ysize,min_nonnull_pnts,
               min_height_separation,max_variance_factor);
         }
         while (npixels_changed > max_pixels_changed);

         do
         {
            npixels_changed=ladarfunc::remove_isolated_outliers(
               tree_sentinel_value,data_bbox_ptr,
               ztwoDarray_ptr,ftwoDarray_ptr,xsize,ysize,min_nonnull_pnts,
               min_height_separation,max_variance_factor);
         }
         while (npixels_changed > max_pixels_changed);

         do
         {
            npixels_changed=ladarfunc::remove_isolated_outliers(
               building_sentinel_value,data_bbox_ptr,
               ztwoDarray_ptr,ftwoDarray_ptr,xsize,ysize,min_nonnull_pnts,
               min_height_separation,max_variance_factor);
         }
         while (npixels_changed > max_pixels_changed);
      }

// ---------------------------------------------------------------------
// Method cull_feature_pixels takes in a feature map within twoDarray
// *ftwoDarray_ptr.  It culls all pixels whose feature value equals
// some specified input value.  This method returns the culled pixels
// in a dynamically generated twoDarray.
   
   twoDarray* cull_feature_pixels(
      double feature_value,twoDarray const *ftwoDarray_ptr)
      {
         return cull_feature_pixels(
            feature_value,xyzpfunc::null_value,ftwoDarray_ptr);
      }
   
   twoDarray* cull_feature_pixels(
      double feature_value,double null_value,twoDarray const *ftwoDarray_ptr)
      {
         twoDarray* frestricted_twoDarray_ptr=new twoDarray(ftwoDarray_ptr);
         frestricted_twoDarray_ptr->initialize_values(null_value);

         for (unsigned int px=0; px<ftwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ftwoDarray_ptr->get_ndim(); py++)
            {
               double curr_feature_value=ftwoDarray_ptr->get(px,py);
               if (nearly_equal(curr_feature_value,feature_value))
               {
                  frestricted_twoDarray_ptr->put(px,py,feature_value);
               }
            } // loop over py index
         } // loop over px index
         return frestricted_twoDarray_ptr;
      }
   
// This overloaded version of cull_feature_pixels takes in height
// image *ztwoDarray_ptr and feature image *ftwoDarray_ptr.  It scans
// through the feature image and nulls the height of any pixel which
// does not correspond to the input feature_value.  This method
// returns a dynamically generated height image for those pixels which
// do correspond to feature_value.

   twoDarray* cull_feature_pixels(
      double feature_value,twoDarray const *ztwoDarray_ptr,
      twoDarray const *ftwoDarray_ptr)
      {
         twoDarray* zrestricted_twoDarray_ptr=new twoDarray(ftwoDarray_ptr);
         zrestricted_twoDarray_ptr->initialize_values(xyzpfunc::null_value);

         for (unsigned int px=0; px<ftwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ftwoDarray_ptr->get_ndim(); py++)
            {
               double curr_feature_value=ftwoDarray_ptr->get(px,py);
               if (nearly_equal(curr_feature_value,feature_value))
               {
                  zrestricted_twoDarray_ptr->put(
                     px,py,ztwoDarray_ptr->get(px,py));
               }
            } // loop over py index
         } // loop over px index
         return zrestricted_twoDarray_ptr;
      }

// ---------------------------------------------------------------------
// Method transfer_feature_pixels takes in a feature map within
// twoDarray *ftwoDarray_ptr.  It copies over all of its pixels whose
// values equal the specified input value onto the second feature map
// within output twoDarray *fnew_twoDarray_ptr.

   void transfer_feature_pixels(
      double feature_value,twoDarray const *ftwoDarray_ptr,
      twoDarray *fnew_twoDarray_ptr)
      {
         transfer_feature_pixels(feature_value,feature_value,ftwoDarray_ptr,
                                 fnew_twoDarray_ptr);
      }

   void transfer_feature_pixels(
      double feature_value,double new_feature_value,
      twoDarray const *ftwoDarray_ptr,twoDarray *fnew_twoDarray_ptr)
      {
         int nchanged_pixels=0;
         for (unsigned int px=0; px<ftwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ftwoDarray_ptr->get_ndim(); py++)
            {
               double curr_feature_value=ftwoDarray_ptr->get(px,py);
               if (nearly_equal(curr_feature_value,feature_value))
               {
                  fnew_twoDarray_ptr->put(px,py,new_feature_value);
                  nchanged_pixels++;
               }
            } // loop over py index
         } // loop over px index
      }

// ---------------------------------------------------------------------
// Method recolor_feature_pixels takes in a feature map within
// twoDarray *ftwoDarray_ptr.  It resets the color value of all pixels
// whose values equal the specified input value to the second
// specified output value.  We wrote this little utility method for
// algorithm debugging purposes.

   void recolor_feature_pixels(
      double old_feature_value,double new_feature_value,
      twoDarray* ftwoDarray_ptr)
      {
         for (unsigned int px=0; px<ftwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ftwoDarray_ptr->get_ndim(); py++)
            {
               double curr_feature_value=ftwoDarray_ptr->get(px,py);
               if (nearly_equal(curr_feature_value,old_feature_value))
               {
                  ftwoDarray_ptr->put(px,py,new_feature_value);
               }
            } // loop over py index
         } // loop over px index
      }

   void recolor_feature_pixels(
      double old_feature_value,double new_feature_value_lo,
      double new_feature_value_hi,double z_cutoff,
      twoDarray const *ztwoDarray_ptr,twoDarray* ftwoDarray_ptr)
      {
         int n_below_zcutoff=0;
         for (unsigned int px=0; px<ftwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ftwoDarray_ptr->get_ndim(); py++)
            {
               double curr_feature_value=ftwoDarray_ptr->get(px,py);
               if (nearly_equal(curr_feature_value,old_feature_value))
               {
                  double curr_z=ztwoDarray_ptr->get(px,py);
                  if (curr_z < z_cutoff)
                  {
                     ftwoDarray_ptr->put(px,py,new_feature_value_lo);
                     n_below_zcutoff++;
                  }
                  else
                  {
                     ftwoDarray_ptr->put(px,py,new_feature_value_hi);
                  }
               }
            } // loop over py index
         } // loop over px index
         cout << "inside featurefunc::recolor_feature_pixels()" << endl;
         cout << "n_below_zcutoff = " << n_below_zcutoff << endl;
      }

// ---------------------------------------------------------------------
// Method abs_gradient_contour_integral takes in dz/dx & dz/dy
// gradient information within *xderiv_twoDarray_ptr and
// *yderiv_twoDarray_ptr.  It also takes in contour *contour_ptr.
// This method computes and returns the average of |gradient_z dot
// r_hat| around the closed contour.  

   double abs_gradient_contour_integral(
      twoDarray const *xderiv_twoDarray_ptr,
      twoDarray const *yderiv_twoDarray_ptr,contour const *contour_ptr)
      {
         unsigned int px,py;
         double average_abs_height_fluctuation=0;
         int n_edges=0;
         for (unsigned int i=0; i<contour_ptr->get_nvertices(); i++)
         {
            linesegment curr_edge(contour_ptr->get_edge(i));
            if (xderiv_twoDarray_ptr->point_to_pixel(
               curr_edge.get_midpoint(),px,py))
            {
               threevector gradient(xderiv_twoDarray_ptr->get(px,py),
                                 yderiv_twoDarray_ptr->get(px,py));
               threevector r_hat(contour_ptr->radial_direction_vector(i));
               double curr_dotproduct(gradient.dot(r_hat));
               average_abs_height_fluctuation += fabs(curr_dotproduct);
               n_edges++;               
            }
         } // loop over index i labeling contour edge
         average_abs_height_fluctuation /= double(n_edges);
         return average_abs_height_fluctuation;
      }

// ---------------------------------------------------------------------
// Method tree_pixels_outside_contour

   double tree_pixels_outside_contour(
      twoDarray const *features_twoDarray_ptr,contour const *contour_ptr)
      {
         unsigned int px,py;
         int n_edges=0;
         int n_tree_edges=0;
         const double radial_dist=2;	// meters
         for (unsigned int i=0; i<contour_ptr->get_nvertices(); i++)
         {
            linesegment curr_edge(contour_ptr->get_edge(i));
            threevector midpoint(curr_edge.get_midpoint());
            if (features_twoDarray_ptr->point_inside_working_region(midpoint))
            {
               threevector r_hat(contour_ptr->radial_direction_vector(i));
               threevector outside_pnt(midpoint+radial_dist*r_hat);
               if (features_twoDarray_ptr->point_to_pixel(
                  outside_pnt,px,py))
               {
                  if (nearly_equal(features_twoDarray_ptr->get(px,py),
                                   tree_sentinel_value))
                  {
                     n_tree_edges++;
                  }
                  n_edges++;
               }
            }
         } // loop over index i labeling contour edge
         double avg_outside_tree_frac=double(n_tree_edges)/double(n_edges);
         return avg_outside_tree_frac;
      }

// ==========================================================================
// Road and grass extraction methods
// ==========================================================================

// Method distinguish_road_and_grass_pixels takes in a completely
// flattened height image within input twoDarray *ztwoDarray_ptr.  It
// also takes in a raw intensity image within input twoDarary
// *ptwoDarray_ptr.  This method first locally computes median
// intensity values for low-lying pixels whose heights fall below some
// cutoff.  If the median intensity in some region is relatively high,
// it is likely that the region corresponds to grass.  This method
// then tends to increase most of the pixel intensity values within
// that region.  On the other hand, if the median intensity in a
// region is low, the region probably corresponds to roadside.  This
// method then tends to decrease most of the region's pixel
// intensities.  After several iterations, this method generally
// yields a filtered intensity image where high and low intensity
// regions are clustered together.

   twoDarray* distinguish_road_and_grass_pixels(
      int n_iters,string imagedir,double p_tall_sentinel_value,
      twoDarray const *ztwoDarray_ptr,twoDarray const *ptwoDarray_ptr)
      {
         outputfunc::write_banner("Distinguishing road and grass pixels:");

         twoDarray* ptwoDarray_lo_ptr=new twoDarray(ptwoDarray_ptr);
         ptwoDarray_ptr->copy(ptwoDarray_lo_ptr);
         twoDarray* pmedian_twoDarray_ptr=new twoDarray(ptwoDarray_ptr);
         twoDarray* pfiltered_twoDarray_ptr=new twoDarray(ptwoDarray_ptr);

         const bool ignore_tall_objects=true;
         double z_threshold=0.5;	// meter
         for (int iter=0; iter<n_iters; iter++)
         {
            cout << "Iteration = " << iter << endl;
      
// Initially set all entries within p-image corresponding to
// relatively high objects to a sentinal unity value:
            
            ladarfunc::set_pixel_intensity_values_to_sentinel(
               ignore_tall_objects,z_threshold,p_tall_sentinel_value,
               ztwoDarray_ptr,ptwoDarray_lo_ptr);
   
// Compute local median intensities for all low-lying pixels:

            const unsigned int nsize=3;
            imagefunc::probability_filter(
               nsize,nsize,ptwoDarray_lo_ptr,pmedian_twoDarray_ptr,0.5,1.0);
            ladarfunc::set_pixel_intensity_values_to_sentinel(
               ignore_tall_objects,z_threshold,p_tall_sentinel_value,
               ztwoDarray_ptr,ptwoDarray_lo_ptr);

//            string median_pimage_filenamestr=imagedir+
//               "median_pimage_"+stringfunc::number_to_string(iter)+".xyzp";
//            xyzpfunc::write_xyzp_data(
//               ztwoDarray_ptr,pmedian_twoDarray_ptr,
//               median_pimage_filenamestr);
//     writeimage("pmedian",pmedian_twoDarray_ptr,false,ladarimage::p_data);

// Amplify high and low intensity clusters:

            ladarfunc::probability_filter(
               nsize,ptwoDarray_lo_ptr,pmedian_twoDarray_ptr,
               pfiltered_twoDarray_ptr,p_tall_sentinel_value);

// After performing p-image filtering on all pixels, reset all entries
// within p-image corresponding to relatively high objects to a
// sentinal unity value:

            pfiltered_twoDarray_ptr->copy(ptwoDarray_lo_ptr);
            ladarfunc::set_pixel_intensity_values_to_sentinel(
               ignore_tall_objects,z_threshold,p_tall_sentinel_value,
               ztwoDarray_ptr,ptwoDarray_lo_ptr);
         } // iter loop

         delete pmedian_twoDarray_ptr;
         delete pfiltered_twoDarray_ptr;

         string filtered_pimage_filenamestr=imagedir
            +"filtered_lo_pimage.xyzp";
         xyzpfunc::write_xyzp_data(
            ztwoDarray_ptr,ptwoDarray_lo_ptr,filtered_pimage_filenamestr);
//   writeimage("filtered_lo_pimage",ptwoDarray_lo_ptr,false,
// 	ladarimage::p_data);

         return ptwoDarray_lo_ptr;
      }

// ---------------------------------------------------------------------
// Method generate_binary_asphalt_image takes in a features
// classification map.  It dynamically generates and returns a binary
// version of just the road pixels within the input image.

   twoDarray* generate_binary_asphalt_image(
      twoDarray const *features_twoDarray_ptr)
      {
         int n_asphalt_pixels=0;
         twoDarray* binary_asphalt_twoDarray_ptr=
            new twoDarray(features_twoDarray_ptr);
         for (unsigned int px=0; px<binary_asphalt_twoDarray_ptr->get_mdim(); 
              px++)
         {
            for (unsigned int py=0; py<binary_asphalt_twoDarray_ptr->
                    get_ndim(); py++)
            {
               if (nearly_equal(features_twoDarray_ptr->get(px,py),
                                road_sentinel_value))
              {
                 binary_asphalt_twoDarray_ptr->put(px,py,1);
                 n_asphalt_pixels++;
               }
            }
         }
//         cout << "n_asphalt_pixels = " << n_asphalt_pixels << endl;
         return binary_asphalt_twoDarray_ptr;
      }
   
// ---------------------------------------------------------------------
// Method density_filter_road_content takes in an intensity image
// which is assumed to have already been segmented into road, grass
// and tall pixel regions.  It first generates a binary version of
// just the road pixels within the input image.  This method next runs
// a quasi-circular density filter over the binary image.  Binary road
// regions which are too small or stringy are reclassified as grass
// regions.

   void density_filter_road_content(
      double filter_diameter,double fill_frac_threshold,
      twoDarray* ptwoDarray_ptr)
      {
         outputfunc::write_banner("Density filtering road content:");   

         twoDarray* pbinary_twoDarray_ptr=generate_binary_asphalt_image(
            ptwoDarray_ptr);
//     writeimage("p_binary",pbinary_twoDarray_ptr,false,ladarimage::p_data);

 
// Run a quasi-circular density filter over binary intensity image.
// Retain only those pixels whose local densities exceed
// fill_frac_threshold. 

//   double diameter=5;	 // meters
//   double diameter=6;	 // meters
//   double fill_frac_threshold=0.75;
//   double fill_frac_threshold=0.85;
         twoDarray* pfiltered_twoDarray_ptr=
            binaryimagefunc::binary_density_filter(
               filter_diameter,fill_frac_threshold,pbinary_twoDarray_ptr);
         delete pbinary_twoDarray_ptr;

//   writeimage("p_filtered",pfiltered_twoDarray_ptr,false,
//		ladarimage::p_data);

         double min_projected_area=200;     // meters
         twoDarray* pfilled_twoDarray_ptr=
            ladarfunc::connect_binary_components(
               min_projected_area,pfiltered_twoDarray_ptr);
         delete pfiltered_twoDarray_ptr;

         for (unsigned int px=0; px<ptwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ptwoDarray_ptr->get_ndim(); py++)
            {
               if (ptwoDarray_ptr->get(px,py)==
                   road_sentinel_value
                   && pfilled_twoDarray_ptr->get(px,py) < 0.1)
               {
                  ptwoDarray_ptr->put(
                     px,py,grass_sentinel_value);
               }
            }
         }
         delete pfilled_twoDarray_ptr;
      }

// ---------------------------------------------------------------------
// Method locate_road_seeds returns a dynamically generated twoDarray
// containing likely locations for road points where the local asphalt
// density within the input feature map *features_twoDarray_ptr
// exceeds a very high threshold fraction.

   twoDarray* locate_road_seeds(
      string imagedir,
      twoDarray const *ztwoDarray_ptr,
      twoDarray const *features_twoDarray_ptr)
      {
         outputfunc::write_banner("Locating road seeds:");         
         
         twoDarray* binary_features_twoDarray_ptr=
            new twoDarray(features_twoDarray_ptr);
         binaryimagefunc::binary_threshold_for_particular_cutoff(
            road_sentinel_value,features_twoDarray_ptr,
            binary_features_twoDarray_ptr);

         const double diameter=8; 	// meters
         const double fill_frac_threshold=0.975;
         twoDarray* binary_road_seeds_twoDarray_ptr=
            binaryimagefunc::binary_density_filter(
               diameter,fill_frac_threshold,binary_features_twoDarray_ptr,
               false);
         delete binary_features_twoDarray_ptr;

//         string binary_filenamestr=imagedir+"binary_seeds.xyzp";
//         xyzpfunc::write_xyzp_data(
//            ztwoDarray_ptr,binary_road_seeds_twoDarray_ptr,binary_filenamestr,
//            true);

         return binary_road_seeds_twoDarray_ptr;
      }

// ---------------------------------------------------------------------
// Method classify_road_grass_pixels takes in twoDarray
// *ptwoDarray_ptr along with tall object sentinel value.  It returns
// a dynamically generated twoDarray in which every pixel is assigned
// one of these values, road_sentinel_value or
// grass_sentinel_value, to indicate that it corresponds to
// either grass, roadside or a tall object.

   twoDarray* classify_road_grass_pixels(
      double p_tall_sentinel_value,twoDarray const *ptwoDarray_ptr)
      {
         const double small_pvalue=0.002;
         
         twoDarray* ftwoDarray_ptr=new twoDarray(ptwoDarray_ptr);
         ptwoDarray_ptr->copy(ftwoDarray_ptr);
         for (unsigned int px=0; px<ftwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ftwoDarray_ptr->get_ndim(); py++)
            {
               double curr_p=ptwoDarray_ptr->get(px,py);
               if (curr_p != p_tall_sentinel_value)
               {
                  if (curr_p < small_pvalue)
                  {
                     ftwoDarray_ptr->put(
                        px,py,road_sentinel_value);
                  }
                  else
                  {
                     ftwoDarray_ptr->put(
                        px,py,grass_sentinel_value);
                  }
               }
            } // py loop
         } // px loop
         return ftwoDarray_ptr;
      }

// ---------------------------------------------------------------------
// Method remove_feature_holes_from_feature_map takes in a features
// map which we assume has already been significantly cleaned and
// processed.  This high-level method attempts to find islands of
// non-feature_sentinel_valued pixels (e.g. grass or trees) within
// otherwise continguous regions of feature_sentinel_valued pixels
// (e.g. asphalt).  Since such islands seem highly unlikely to occur
// in reality, this method recursively empties them and resets their
// feature pixel values to feature_sentinel_value.  At the end of this
// method, the feature_sentinel_valued content of
// *features_twoDarray_ptr should hopefully be more realistic.

   void remove_feature_holes_from_feature_map(
      double feature_sentinel_value,
      twoDarray* features_twoDarray_ptr,twoDarray const *ztwoDarray_ptr,
      string particular_feature_name,string imagedir)
      {
         twoDarray* f_feature_twoDarray_ptr=cull_feature_pixels(
            feature_sentinel_value,features_twoDarray_ptr);
         string f_feature_filename=imagedir+particular_feature_name+
            "_with_holes.xyzp";
         xyzpfunc::write_xyzp_data(
            ztwoDarray_ptr,f_feature_twoDarray_ptr,f_feature_filename);
         draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
            f_feature_twoDarray_ptr,f_feature_filename);

// Recursively eliminate small unwanted islands (e.g. of grass or
// trees) which are surrounded by dominant feature (e.g. asphalt)
// pixels:

         twoDarray* filled_feature_twoDarray_ptr=
            fill_feature_islands(f_feature_twoDarray_ptr,
                                 feature_sentinel_value);
         delete f_feature_twoDarray_ptr;

         string filled_feature_filename=imagedir+"filled_"
            +particular_feature_name+".xyzp"; 
         xyzpfunc::write_xyzp_data(
            ztwoDarray_ptr,filled_feature_twoDarray_ptr,
            filled_feature_filename);
         draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
            filled_feature_twoDarray_ptr,filled_feature_filename);

// Transfer filled feature pixels back onto full features map:

         transfer_feature_pixels(
            feature_sentinel_value,filled_feature_twoDarray_ptr,
            features_twoDarray_ptr);
         delete filled_feature_twoDarray_ptr;
      }

// ---------------------------------------------------------------------
// Method fill_feature_islands takes in a feature map which is assumed
// to contain only pixels whose values equal input parameter
// feature_sentinel_value.  It recursively replaces
// non-feature_sentinel_valued islands within the feature map with
// feature_sentinel_valued pixels.  This method returns a dynamically
// generated twoDarray which hopefully contains an island-free set of
// pixels.

   twoDarray* fill_feature_islands(
      twoDarray const *ftwoDarray_ptr,double feature_sentinel_value)
      {
         outputfunc::write_banner("Filling in feature islands:");

         twoDarray* all_feature_twoDarray_ptr=new twoDarray(ftwoDarray_ptr);
         all_feature_twoDarray_ptr->initialize_values(
            feature_sentinel_value);
         
         twoDarray* feature_filled_twoDarray_ptr=new twoDarray(
            ftwoDarray_ptr);
         ftwoDarray_ptr->copy(feature_filled_twoDarray_ptr);

//         int nrecursion_max=15;
//         const int n_iters=4;
         int nrecursion_max,n_iters;

// Perform less recursive cleaning in order to eliminate islands
// within grass than in asphalt and building rooftops:

         if (nearly_equal(feature_sentinel_value,
                          grass_sentinel_value))
         {
            nrecursion_max=10;
            n_iters=2;
         }
         else
         {
            nrecursion_max=15;
            n_iters=4;
         }
         
         for (int n=0; n<n_iters; n++)
         {
            cout << "Recursive fill iteration " << n << " of " 
                 << n_iters << endl;
            recursivefunc::recursive_fill(
               nrecursion_max,feature_filled_twoDarray_ptr,
               all_feature_twoDarray_ptr,false);
         }
         delete all_feature_twoDarray_ptr;
         return feature_filled_twoDarray_ptr;
      }

// ==========================================================================
// Tree and building extraction methods
// ==========================================================================

// Method distinguish_tree_from_bldg_pixels takes in a completely
// flattened height image within input twoDarray *ztwoDarray_ptr along
// with a raw intensity image within twoDarray *ptwoDarray_ptr.  It
// first ignores all pixels corresponding to low-lying road or grass
// features.  It then uses height fluctuation information to help
// discriminate trees from buildings.  Since chlorophyl in vegetation
// reflects well in the red, trees generally appear brighter in the
// p-image than do building rooftops.  So this method iteratively
// performs recursive emptying and local intensity contrast
// amplification to help distinguish tree and building pixels.

   twoDarray* distinguish_tree_from_bldg_pixels(
      int n_iters,string imagedir,double p_low_sentinel_value,
      twoDarray const *ztwoDarray_ptr,twoDarray const *ptwoDarray_ptr,
      twoDarray const *norm_zfluc_twoDarray_ptr)
      {
         outputfunc::write_banner(
            "Distinguishing tree from building pixels:");

         const bool ignore_tall_objects=false;
         const double z_threshold=3;	// meters

         twoDarray* ptwoDarray_hi_ptr=new twoDarray(ptwoDarray_ptr);
         ptwoDarray_ptr->copy(ptwoDarray_hi_ptr);
         twoDarray* pnew_twoDarray_ptr=new twoDarray(ptwoDarray_ptr);
         twoDarray* pbinary_twoDarray_ptr=new twoDarray(ptwoDarray_ptr);

         for (int iter=0; iter<n_iters; iter++)
         {
            cout << "Iteration = " << iter << endl;
      
// Initially set all entries within the p-image corresponding to
// relatively low-lying objects equal to p_low_sentinal_value:

            ladarfunc::set_pixel_intensity_values_to_sentinel(
               ignore_tall_objects,z_threshold,p_low_sentinel_value,
               ztwoDarray_ptr,ptwoDarray_hi_ptr);
//      writeimage("p_sentinel",ptwoDarray_hi_ptr,false,ladarimage::p_data);

// Use z fluctuation information to help discriminate trees from buildings:

            double p_threshold=0.2;
            double norm_zfluc_threshold=0.8;
            double p_new_min=0.35;
            ladarfunc::increase_intensities_for_large_height_fluctuations(
               p_threshold,norm_zfluc_threshold,p_new_min,ptwoDarray_hi_ptr,
               norm_zfluc_twoDarray_ptr,pnew_twoDarray_ptr);
//      writeimage("p_increase",pnew_twoDarray_ptr,false,ladarimage::p_data);

// Use global thresholding to help isolate small islands of hot
// intensity which are surrounded by oceans of cold intensity for
// current ptwoDarray:

            p_threshold=0.3;
            imagefunc::threshold_intensities_below_cutoff(
               pnew_twoDarray_ptr,p_threshold,xyzpfunc::null_value);
//      writeimage("p_threshold",pnew_twoDarray_ptr,false,ladarimage::p_data);

// Recursively empty intensity image of small hot noise islands.  

            recursivefunc::recursive_empty(
               1,pnew_twoDarray_ptr,false,xyzpfunc::null_value);
//      writeimage("p_empty",pnew_twoDarray_ptr,false,ladarimage::p_data);
//      string pempty_filenamestr=imagedir+"p_empty.xyzp";      
//      xyzpfunc::write_xyzp_data(
//         ztwoDarray_ptr,pnew_twoDarray_ptr,pempty_filenamestr);

// Try to recursively fill in isolated islands of null values within p
// image:

            binaryimagefunc::abs_binary_threshold(
               -0.5*xyzpfunc::null_value,pnew_twoDarray_ptr,
               pbinary_twoDarray_ptr,0);
//      writeimage("p_binary",pbinary_twoDarray_ptr,false,ladarimage::p_data);
            recursivefunc::binary_fill(7,0,ptwoDarray_ptr->get_mdim(),
                                       0,ptwoDarray_ptr->get_ndim(),
                                       1,0,pbinary_twoDarray_ptr);
            recursivefunc::binary_fill(1,0,ptwoDarray_ptr->get_mdim(),
                                       0,ptwoDarray_ptr->get_ndim(),
                                       0,1,pbinary_twoDarray_ptr);
            recursivefunc::binary_fill(20,0,ptwoDarray_ptr->get_mdim(),
                                       0,ptwoDarray_ptr->get_ndim(),
                                       1,0,pbinary_twoDarray_ptr);
//      writeimage("p_fill",pbinary_twoDarray_ptr,false,ladarimage::p_data);
//      string pfill_filenamestr=imagedir+"p_fill.xyzp";      
//      xyzpfunc::write_xyzp_data(
//         ztwoDarray_ptr,pbinary_twoDarray_ptr,pfill_filenamestr);

            recursivefunc::binary_filter(
               ptwoDarray_hi_ptr,pbinary_twoDarray_ptr,
               xyzpfunc::null_value,0.5);
//      writeimage("p_filter",ptwoDarray_ptr,false,ladarimage::p_data);
//      string pfilter_filenamestr=imagedir+"p_filter.xyzp";      
//      xyzpfunc::write_xyzp_data(
//         ztwoDarray_ptr,ptwoDarray_ptr,pfilter_filenamestr);

// After performing p-image filtering on all pixels, reset all entries
// within p-image corresponding to relatively low objects to a
// sentinal zero value:

            ladarfunc::set_pixel_intensity_values_to_sentinel(
               ignore_tall_objects,z_threshold,p_low_sentinel_value,
               ztwoDarray_ptr,ptwoDarray_hi_ptr);
         } // iter loop

         delete pnew_twoDarray_ptr;
         delete pbinary_twoDarray_ptr;

         string filtered_pimage_filenamestr;
         filtered_pimage_filenamestr=imagedir+"filtered_hi_pimage.xyzp";
         xyzpfunc::write_xyzp_data(
            ztwoDarray_ptr,ptwoDarray_ptr,filtered_pimage_filenamestr);

// Again use z fluctuation information to help discriminate trees from
// buildings:

         double p_threshold=0.1;
         double norm_zfluc_threshold=0.95;
         double p_new_min=0.35;
         twoDarray* pfinal_twoDarray_ptr=new twoDarray(ptwoDarray_ptr);
         ladarfunc::increase_intensities_for_large_height_fluctuations(
            p_threshold,norm_zfluc_threshold,p_new_min,ptwoDarray_hi_ptr,
            norm_zfluc_twoDarray_ptr,pfinal_twoDarray_ptr);
         delete ptwoDarray_hi_ptr;

         return pfinal_twoDarray_ptr;
      }

// ---------------------------------------------------------------------
// Method locate_building_clusters takes in p-image *ptwoDarray_ptr
// which is assumed to basically contain binary tree information.
// Building rooftops within the input p-image are generally
// null-valued.  This method also takes in twoDarray *ztwoDarray_ptr
// which is assumed to contain completely flattened height
// information.  It first generates a binary p-image whose rooftop
// pixels are generally unit-valued.  We then perform recursive
// emptying and density filtering in order to eliminate as many
// unit-valued tree pixel locations as possible from the binary
// p-image.  This member function then calculates connected components
// within the binary image, and it retains only those whose footprints
// on the ground exceed some minimum area.  This method returns
// connected height component information within the dynamically
// generated output hashtable *connected_heights_hashtable_ptr.  It
// also returns a dynamically generated twoDarray containing this
// connected building component information.

   twoDarray* locate_building_clusters(
      string imagedir,twoDarray *ptwoDarray_ptr,
      twoDarray const *ztwoDarray_ptr)
      {
         outputfunc::write_banner("Locating building clusters:");

//         string p_trees_filenamestr=imagedir+"p_trees_copy.xyzp";
//         xyzpfunc::write_xyzp_data(
//            ztwoDarray_ptr,ptwoDarray_ptr,p_trees_filenamestr);
//         writeimage("p_trees_copy",ptwoDarray_ptr,false,ladarimage::p_data);

         twoDarray* z_wotrees_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
         ztwoDarray_ptr->copy(z_wotrees_twoDarray_ptr);

// First generate binary intensity image which contains 1's where
// building rooftops are located.  Unfortunately, this binary image
// also is unit-valued where tree height gradients are strong.  Also,
// reset to zero the height of as many tree pixels as possible within
// *z_wotrees_twoDarray_ptr:

//         const double zmin=2;	// meters
         const double zmin=1.5;	// meters
//         const double zmin=1;	// meters
         for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
            {
               if (ptwoDarray_ptr->get(px,py)==xyzpfunc::null_value &&
                   ztwoDarray_ptr->get(px,py) > zmin)
               {
                  ptwoDarray_ptr->put(px,py,1);
               }
               else if (ptwoDarray_ptr->get(px,py) > 0)
               {
                  z_wotrees_twoDarray_ptr->put(px,py,0);
                  ptwoDarray_ptr->put(px,py,0);
               }
            }
         }
         string zwotrees_filenamestr=imagedir+"z_wotrees.xyzp";
         xyzpfunc::write_xyzp_data(
            z_wotrees_twoDarray_ptr,ptwoDarray_ptr,zwotrees_filenamestr);
//   writeimage("ptwoDarray",ptwoDarray_ptr,false,ladarimage::p_data);

// Recursively empty binary ptwoDarray image to eliminate as many
// isolated tree pixels as possible:

         int nrecursion_max=5;
         recursivefunc::recursive_empty(
            nrecursion_max,ptwoDarray_ptr,false,0);

         string z_recur_filenamestr=imagedir+"z_recur1.xyzp";
         xyzpfunc::write_xyzp_data(
            z_wotrees_twoDarray_ptr,ptwoDarray_ptr,z_recur_filenamestr);

         recursivefunc::recursive_empty(
            nrecursion_max,ptwoDarray_ptr,false,0);

         z_recur_filenamestr=imagedir+"z_recur2.xyzp";
         xyzpfunc::write_xyzp_data(
            z_wotrees_twoDarray_ptr,ptwoDarray_ptr,z_recur_filenamestr);

// Run a quasi-circular density filter over recusively emptied binary
// intensity image.  Retain only those pixels whose local densities
// exceed fill_frac_threshold.  This tends to eliminate long skinny
// strands of tree pixels while retaining solid clusters of building
// pixels:

         double diameter=5;	 // meters
         double fill_frac_threshold=0.65;
         twoDarray* pfiltered_twoDarray_ptr=
            binaryimagefunc::binary_density_filter(
               diameter,fill_frac_threshold,ptwoDarray_ptr);

         string pfiltered_filenamestr=imagedir+"pfiltered.xyzp";
         xyzpfunc::write_xyzp_data(
            z_wotrees_twoDarray_ptr,pfiltered_twoDarray_ptr,
            pfiltered_filenamestr);

         for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
            {
               if (pfiltered_twoDarray_ptr->get(px,py) < 0.1)
               {
                  z_wotrees_twoDarray_ptr->put(px,py,0);
               }
            }
         }

         string p_bldgs_filenamestr=imagedir+"p_bldgs.xyzp";
         xyzpfunc::write_xyzp_data(
            z_wotrees_twoDarray_ptr,pfiltered_twoDarray_ptr,
            p_bldgs_filenamestr);
//   writeimage("p_bldgs",pfiltered_twoDarray_ptr,false,ladarimage::p_data);

// Compute connected components within binary intensity image.  Retain
// only those components whose footprint on the ground exceeds some
// minimal projected area value:

//         double min_projected_area=25;    // m**2
         double min_projected_area=20;    // m**2
//         double min_projected_area=15;    // m**2
         Hashtable<linkedlist*>* connected_heights_hashtable_ptr=
            ladarfunc::connect_height_components(
               zmin,min_projected_area,z_wotrees_twoDarray_ptr);
         delete z_wotrees_twoDarray_ptr;

         twoDarray* zconnected_components_twoDarray_ptr=
            new twoDarray(ztwoDarray_ptr);
         connectfunc::decode_connected_hashtable(
            connected_heights_hashtable_ptr,
            zconnected_components_twoDarray_ptr);
         connectfunc::delete_connected_hashtable(
            connected_heights_hashtable_ptr);
//         delete connected_heights_hashtable_ptr;

         string zconnected_filenamestr=imagedir+"zconnected.xyzp";
         xyzpfunc::write_xyzp_data(
            zconnected_components_twoDarray_ptr,pfiltered_twoDarray_ptr,
            zconnected_filenamestr);
         delete pfiltered_twoDarray_ptr;

         return zconnected_components_twoDarray_ptr;
      }

// ---------------------------------------------------------------------
// Method detect_tiered_roofs computes the height distribution for
// each building within input hashtable *connected_hashtable_ptr.
// After applying Savitzky-Golay filtering to the raw distributions,
// this method looks for peaks within the smoothed distributions which
// are separated by at least 2 meters in height.  It subsequently
// matches gaussians to the extracted peaks by fitting quadratic
// polynomials to the log-z distribution in the vicinity of each peak.
// If at least 2 tall, skinny gaussian peaks are detected, this method
// records the particular building number along with the peaks'
// z-values within the output vector.

   vector<pair<int,vector<double> > >* detect_tiered_roofs(
      Hashtable<linkedlist*> const *connected_hashtable_ptr,
      twoDarray const *ztwoDarray_ptr)
      {
         outputfunc::write_banner("Detecting tiered roofs:");

         int n_extracted_rooftops=connected_hashtable_ptr->size();
         vector<pair<int,vector<double> > >* tiered_bldg_ptr=
            new vector<pair<int,vector<double> > >;
         
         for (int n=0; n<n_extracted_rooftops; n++)
         {
//            cout << " Looking for multi-tiered roofs within cluster " << n
//                 << " out of " << n_extracted_rooftops << endl;
            
            linkedlist* currlist_ptr=
               ladarfunc::retrieve_connected_pixel_list(
                  n,connected_hashtable_ptr);
            if (currlist_ptr != NULL)
            {
               int counter=0;
               double* z=new double[currlist_ptr->size()];
               mynode* curr_voxel_ptr=currlist_ptr->get_start_ptr();
               while (curr_voxel_ptr != NULL)
               {
                  unsigned int px=basic_math::round(curr_voxel_ptr->get_data().
                                                    get_var(0));
                  unsigned int py=basic_math::round(curr_voxel_ptr->get_data().
                                                    get_var(1));
                  z[counter++]=ztwoDarray_ptr->get(px,py);
                  curr_voxel_ptr=curr_voxel_ptr->get_nextptr();
               }

               prob_distribution prob_z(counter,z,500);
               delete [] z;

               prob_z.compute_cumulative_distribution();
               prob_z.compute_density_distribution();
               prob_z.smooth_density_distribution();

               const double x_interval=2;	// meters
               vector<pair<int,mypolynomial> >* peak_bin_ptr=
                  prob_z.locate_density_peaks(x_interval,2,10);
               prob_z.fit_gaussians_to_density_peaks(peak_bin_ptr,1);

               int n_narrow_gaussians=0;
               vector<double> tier_height;
               for (unsigned int i=0; i<peak_bin_ptr->size(); i++)
               {
                  int peak_bin=((*peak_bin_ptr)[i]).first;
                  double z_peak=prob_z.get_x(peak_bin);

                  mypolynomial quad_poly=((*peak_bin_ptr)[i]).second;

                  double coeff0=quad_poly.get_coeff(0);
                  double coeff1=quad_poly.get_coeff(1);
                  double coeff2=quad_poly.get_coeff(2);

                  if (peak_bin_ptr->size() >= 2 && 
                      exp(coeff0) > 0.5 && fabs(coeff1) < 1.5 && coeff2 < -5)
                  {
//                     cout << "z_peak = " << z_peak << endl;
//                     cout << "coeff[0] = " << coeff0 << endl;
//                     cout << "coeff[1] = " << coeff1 << endl;
//                     cout << "coeff[2] = " << coeff2 << endl;
//                     cout << "coeff[2]/coeff[0] = " << coeff2/coeff0 << endl;
               
//                     prob_z.densityfilenamestr="roof_height_dist_"+
//                        stringfunc::number_to_string(n)+".meta";
//                     prob_z.xlabel="Z (meters)";
//                     prob_z.xmin=-0.01;
//                     prob_z.xmax=12;
//                     prob_z.xtic=1;
//                     prob_z.xsubtic=1;
//                     prob_z.write_density_dist();

                     tier_height.push_back(z_peak);
                     n_narrow_gaussians++;
                  }
               } // loop over index i labeling peak number
               delete peak_bin_ptr;

               if (n_narrow_gaussians >= 2)
               {
//                  for (int i=0; i<n_narrow_gaussians; i++)
//                  {
//                     cout << "Narrow gaussian " << i 
//                          << " found for rooftop n = " << n << endl;
//                     cout << "z_peak = " << tier_height[i] << endl;
//                  }
                  pair<int,vector<double> > roof_tier_pair;
                  roof_tier_pair.first=n;
                  roof_tier_pair.second=tier_height;
                  tiered_bldg_ptr->push_back(roof_tier_pair);
               }
            } // currlist_ptr != NULL conditional
         } // loop over index n labeling connected component

         for (unsigned int i=0; i<tiered_bldg_ptr->size(); i++)
         {
            cout << "Tiered bldg i = " << i << endl;
            cout << "Rooftop number = " << ((*tiered_bldg_ptr)[i]).first 
                 << endl;
            vector<double> v=((*tiered_bldg_ptr)[i]).second;
            for (unsigned int j=0; j<v.size(); j++)
            {
               cout << "Narrow peak j = " << j << " lies at z = "
                    << v[j] << endl;
            } // loop over index j
         } // loop over index i

         return tiered_bldg_ptr;
      }

// ---------------------------------------------------------------------
// Method refine_building_extraction takes in hashtable
// *connected_hashtable_ptr which contains linkedlists of connected
// pixel components corresponding to building rooftops.  For each
// rooftop cluster of pixels, it first locates seed pixels which are
// highly likely to actually like on the building's roof top.  It then
// performs an "oozing" procedure in which it looks for neighboring
// rooftop pixels whose gradients relative to the seeds are small in
// magnitude.

   twoDarray* refine_building_extraction(
      string imagedir,vector<pair<int,vector<double> > >* tiered_bldg_ptr,
      Hashtable<linkedlist*> const *connected_hashtable_ptr,
      twoDarray const *ztwoDarray_ptr,twoDarray *ptwoDarray_ptr)
      {
         outputfunc::write_banner("Refining building rooftop extraction:");

         twoDarray* p_refined_roof_twoDarray_ptr=new twoDarray(
            ptwoDarray_ptr);
         twoDarray* tmp_ptwoDarray_ptr=new twoDarray(ptwoDarray_ptr);

         int n_extracted_rooftops=connected_hashtable_ptr->size();
         for (int n=0; n<n_extracted_rooftops; n++)
         {
            cout << endl;
            cout << "----------------------------------------------------"
                 << endl;
            cout << "Refining building cluster " << n
                 << " out of " << n_extracted_rooftops << endl;
            cout << "----------------------------------------------------"
                 << endl;
            
            linkedlist* currlist_ptr=
               ladarfunc::retrieve_connected_pixel_list(
                  n,connected_hashtable_ptr);
            if (currlist_ptr != NULL)
            {
               linkedlist* roofseeds_list_ptr=locate_rooftop_seed_pixels(
                  n,tiered_bldg_ptr,currlist_ptr,ztwoDarray_ptr);

               const double bbox_tolerance_distance=1;	// meters
               unsigned int min_px,min_py,max_px,max_py;
               double min_x,min_y,max_x,max_y,x_center,y_center,radius;
               compute_building_bbox(
                  currlist_ptr,ztwoDarray_ptr,bbox_tolerance_distance,
                  min_x,min_y,max_x,max_y,x_center,y_center,radius,
                  min_px,min_py,max_px,max_py);

// Write initial rooftop pixel list to output xyzp file for algorithm
// development purposes:

//               tmp_ptwoDarray_ptr->clear_values();
//               connectfunc::convert_pixel_list_to_twoDarray(
//                  currlist_ptr,tmp_ptwoDarray_ptr);
//               string pixellist_filenamestr=imagedir+"init_roof_pixels_"
//                  +stringfunc::number_to_string(n)+".xyzp";
//               ladarfunc::write_local_xyzp_data(
//                  x_center,y_center,radius,
//                  ztwoDarray_ptr,tmp_ptwoDarray_ptr,pixellist_filenamestr,
//                  false);

               tmp_ptwoDarray_ptr->clear_values();
               ooze_rooftop_pixels(
                  n,min_x,min_y,max_x,max_y,min_px,min_py,max_px,max_py,
                  roofseeds_list_ptr,ztwoDarray_ptr,tmp_ptwoDarray_ptr,
                  imagedir);
               delete roofseeds_list_ptr;

// Compute convex hull of primary roof component pixels:

               polygon* convexhull_ptr=
                  graphicsfunc::connected_region_convex_hull(
                     tmp_ptwoDarray_ptr,min_px,max_px,min_py,max_py);

/*               
               string rooftop_filenamestr=imagedir+"roof_"+
                  stringfunc::number_to_string(n)+".xyzp";
               ladarfunc::write_local_xyzp_data(
                  x_center,y_center,radius,ztwoDarray_ptr,tmp_ptwoDarray_ptr,
                  rooftop_filenamestr,false);
               if (convexhull_ptr != NULL)
               {
                  filefunc::gunzip_file_if_gzipped(rooftop_filenamestr);
                  draw3Dfunc::draw_polygon(
                     *convexhull_ptr,rooftop_filenamestr,
                     draw3Dfunc::annotation_value2);
//                  filefunc::gzip_file(rooftop_filenamestr);   
               }
*/

// Add binary rooftop information contained in *tmp_ptwoDarray_ptr
// onto cumulative binary image *p_refined_roof_twoDarray_ptr:

               double z_primary_roof_COM=accumulate_binary_rooftop_info(
                  min_px,min_py,max_px,max_py,ztwoDarray_ptr,
                  tmp_ptwoDarray_ptr,p_refined_roof_twoDarray_ptr);

// Scan through original list of rooftop pixels and subtract those
// which reside within *p_refined_roof_twoDarray_ptr:

               subtract_oozed_pixels(
                  currlist_ptr,p_refined_roof_twoDarray_ptr);

               tmp_ptwoDarray_ptr->clear_values();
               connectfunc::convert_pixel_list_to_twoDarray(
                  currlist_ptr,tmp_ptwoDarray_ptr);

//               cout << "Drawing residual roof pixels" << endl;
//               string residual_list_filenamestr=imagedir+"resid_roof_pixels_"
//                  +stringfunc::number_to_string(n)+".xyzp";
//               ladarfunc::write_local_xyzp_data(
//                  x_center,y_center,radius,
//                  ztwoDarray_ptr,tmp_ptwoDarray_ptr,
// 		    residual_list_filenamestr,false);

// Compute connected residual building components within binary
// intensity image.  Retain only those components whose footprint on
// the ground exceeds some minimal projected area value:

               const double pmin=building_sentinel_value-0.1;
//               const double min_projected_area=1;    	// m**2	
               const double min_projected_area=2;    	// m**2	
//               const double min_projected_area=2.5;    	// m**2	

               Hashtable<linkedlist*>* connected_bldgparts_hashtable_ptr=
                  ladarfunc::connect_height_components(
                     pmin,min_projected_area,tmp_ptwoDarray_ptr);

// Compare residual building parts' average z-values with primary
// roof's average z-value:

               tmp_ptwoDarray_ptr->clear_values();
               if (connected_bldgparts_hashtable_ptr != NULL)
               {
                  const double bbox_extension=15;	// meters
                  clean_ancillary_building_parts(
                     n,imagedir,
                     min_x-bbox_extension,min_y-bbox_extension,
                     max_x+bbox_extension,max_y+bbox_extension,
                     z_primary_roof_COM,ztwoDarray_ptr,convexhull_ptr,
                     tmp_ptwoDarray_ptr,
                     connected_bldgparts_hashtable_ptr);
               } 
               delete convexhull_ptr;

               twoDarray* pconnected_components_twoDarray_ptr=new twoDarray(
                  ztwoDarray_ptr);
               connectfunc::decode_connected_hashtable(
                  connected_bldgparts_hashtable_ptr,
                  pconnected_components_twoDarray_ptr,true);
               delete connected_bldgparts_hashtable_ptr;

/*
               string connected_house_parts_filenamestr=
                  imagedir+"house_parts_"+stringfunc::number_to_string(n)
                  +".xyzp";
               ladarfunc::write_local_xyzp_data(
                  x_center,y_center,radius,
                  ztwoDarray_ptr,pconnected_components_twoDarray_ptr,
                  connected_house_parts_filenamestr);
               filefunc::gunzip_file_if_gzipped(connected_house_parts_filenamestr);
               draw3Dfunc::append_fake_xyzp_points_for_dataviewer_coloring(
                  connected_house_parts_filenamestr,
                  threevector(x_center,y_center,0));
               filefunc::gzip_file(connected_house_parts_filenamestr);   
*/

// Add extra building parts information contained in
// *pconnected_components_twoDarray_ptr onto cumulative image in
// *p_refined_roof_twoDarray_ptr:

               accumulate_building_parts(
                  min_px,min_py,max_px,max_py,
                  tmp_ptwoDarray_ptr,
//                  pconnected_components_twoDarray_ptr,
                  p_refined_roof_twoDarray_ptr);

               delete pconnected_components_twoDarray_ptr;

            } // currlist_ptr != NULL conditional
         } // loop over index n labeling connected component
         delete tmp_ptwoDarray_ptr;
         
         string rooftop_filenamestr=imagedir+"roofs.xyzp";
         xyzpfunc::write_xyzp_data(
            ztwoDarray_ptr,p_refined_roof_twoDarray_ptr,rooftop_filenamestr,
            true);
         draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
            p_refined_roof_twoDarray_ptr,rooftop_filenamestr);
         return p_refined_roof_twoDarray_ptr;
      }

// ---------------------------------------------------------------------
// Method locate_rooftop_seed_pixels takes in linked list
// *currlist_ptr which contains candidate rooftop pixels.  It first
// computes the COM in (x,y) space for this pixel cluster.  This
// method then dynamically generates a new linked list containing just
// those pixels which lie within some small radius of the (x,y) COM
// point.  It computes the median heights for this smaller bunch of
// pixels.  It subsequently keeps only those pixels whose heights lie
// within some small distance of the median height value.  The
// surviving pixels are assumed to definitely lie on a single rooftop,
// and they are returned as a dynamically generated linked list by
// this method.  They become seeds for the rooftop oozing procedure.

   linkedlist* locate_rooftop_seed_pixels(
      int n,vector<pair<int,vector<double> > >* tiered_bldg_ptr,
      linkedlist* currlist_ptr,twoDarray const *ztwoDarray_ptr)
      {
         bool multi_tiered_bldg=false;
         for (unsigned int i=0; i<tiered_bldg_ptr->size(); i++)
         {
            if (n==((*tiered_bldg_ptr)[i]).first)
            {
               multi_tiered_bldg=true;
            }
         }
         
         const double height_tolerance=0.5;	// meter
         if (multi_tiered_bldg)
         {
            linkedlist* close_pixels_list_ptr=new linkedlist;
            for (unsigned int i=0; i<tiered_bldg_ptr->size(); i++)
            {
               vector<double> z_peak=((*tiered_bldg_ptr)[i]).second;
               for (unsigned int j=0; j<z_peak.size(); j++)
               {
                  connectfunc::similar_height_pixels(
                     currlist_ptr,ztwoDarray_ptr,z_peak[j],height_tolerance,
                     close_pixels_list_ptr);
               }
            }
            return close_pixels_list_ptr;
         }
         else
         {
            
// Compute COM of connected pixel list for singly-tiered building:

            threevector COM=connectfunc::pixel_list_COM(
               currlist_ptr,ztwoDarray_ptr,true);
//         cout << "Cluster COM = " << COM << endl;

            double radius=3;	// meters
            linkedlist* close_pixels_list_ptr=
               connectfunc::pixels_close_to_point(
                  currlist_ptr,ztwoDarray_ptr,COM,radius);

            return median_height_seed_voxels(
               height_tolerance,close_pixels_list_ptr,ztwoDarray_ptr);
         }
      }

// ---------------------------------------------------------------------
// Method median_height_seed_voxels takes in a linked list of voxels
// and computes their median height.  It then discards those points
// from the linked list which do not lie within +/- height_tolernace
// of the median height value:

   linkedlist* median_height_seed_voxels(
      double height_tolerance,linkedlist* voxel_list_ptr,
      twoDarray const *ztwoDarray_ptr)
      {
         double z_median=ladarfunc::compute_voxel_median_height(
            voxel_list_ptr,ztwoDarray_ptr);

         mynode* curr_voxel_ptr=voxel_list_ptr->get_start_ptr();
         while (curr_voxel_ptr != NULL)
         {
            unsigned int px=
               basic_math::round(curr_voxel_ptr->get_data().get_var(0));
            unsigned int py=
               basic_math::round(curr_voxel_ptr->get_data().get_var(1));
            double curr_z=ztwoDarray_ptr->get(px,py);

            mynode* next_voxel_ptr=curr_voxel_ptr->get_nextptr();
            if (fabs(curr_z-z_median) > height_tolerance)
            {
               voxel_list_ptr->delete_node(curr_voxel_ptr);
            }
            curr_voxel_ptr=next_voxel_ptr;
         }
         return voxel_list_ptr;
      }

// ---------------------------------------------------------------------
// Method compute_building_bbox takes in a linked list of pixels which
// are assumed to correspond to some connected rooftop component
// within the height image in twoDarray *ztwoDarray_ptr.  It returns
// the vertex locations of a vertically aligned bounding box which
// encloses all of the pixels within the linked list. 

   void compute_building_bbox(
      linkedlist* curr_list_ptr,twoDarray const *ztwoDarray_ptr,
      const double bbox_tolerance_distance,
      double& minimum_x,double& minimum_y,
      double& maximum_x,double& maximum_y,
      double& x_center,double& y_center,double& char_radius,
      unsigned int& min_px,unsigned int& min_py,
      unsigned int& max_px,unsigned int& max_py)
      {
         minimum_x=minimum_y=POSITIVEINFINITY;
         maximum_x=maximum_y=NEGATIVEINFINITY;
         
         double currx,curry;
         mynode* curr_pixel_ptr=curr_list_ptr->get_start_ptr();
         while (curr_pixel_ptr != NULL)
         {
            unsigned int px=basic_math::round(
               curr_pixel_ptr->get_data().get_var(0));
            unsigned int py=basic_math::round(
               curr_pixel_ptr->get_data().get_var(1));
            ztwoDarray_ptr->pixel_to_point(px,py,currx,curry);
            minimum_x=basic_math::min(minimum_x,currx);
            maximum_x=basic_math::max(maximum_x,currx);
            minimum_y=basic_math::min(minimum_y,curry);
            maximum_y=basic_math::max(maximum_y,curry);
            curr_pixel_ptr=curr_pixel_ptr->get_nextptr();
         }

// Enlarge bounding box limits by the input tolerance distance:

         minimum_x=basic_math::max(minimum_x-bbox_tolerance_distance,
                       ztwoDarray_ptr->get_xlo());
         maximum_x=basic_math::min(maximum_x+bbox_tolerance_distance,
                       ztwoDarray_ptr->get_xhi());
         minimum_y=basic_math::max(minimum_y-bbox_tolerance_distance,
                       ztwoDarray_ptr->get_ylo());
         maximum_y=basic_math::min(maximum_y+bbox_tolerance_distance,
                       ztwoDarray_ptr->get_yhi());

// Compute bounding box center as well as its characteristic radius
// (for local xyzp file generation purposes):

         x_center=0.5*(minimum_x+maximum_x);
         y_center=0.5*(minimum_y+maximum_y);
         char_radius=2*basic_math::max((maximum_x-minimum_x),
                                       (maximum_y-minimum_y));

// Compute bounding box corners' pixel coordinates:

         ztwoDarray_ptr->bbox_corners_to_pixels(
            minimum_x,minimum_y,maximum_x,maximum_y,
            min_px,min_py,max_px,max_py);
      }
   
// ---------------------------------------------------------------------
// Method ooze_rooftop_pixels takes in linkedlist *seed_list_ptr of
// pixels which are assumed to definitely be located on the
// n_rooftop'th building.  It also takes in the limits of a bounding
// box assumed to enclose the n_rooftop'th building.  This method
// starts with the seed pixels and looks for all nearby pixels whose
// gradients relative to the seeds are small in magnitude.  Such
// nearby pixels are then classified as belonging to the n_rooftop'th
// building.  This method also performs a little bit of recursive
// filling in order to fill in rooftop holes.  Binary rooftop
// classification results are returned within twoDarray
// *p_roof_binary_twoDarray_ptr.

   void ooze_rooftop_pixels(
      int n_rooftop,
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      unsigned int min_px,unsigned int min_py,
      unsigned int max_px,unsigned int max_py,
      linkedlist* seed_list_ptr,twoDarray const *ztwoDarray_ptr,
      twoDarray* p_roof_binary_twoDarray_ptr,string imagedir)
      {
         const int ndim=ztwoDarray_ptr->get_ndim();
         const double zlow_threshold=0.5;	// meters
         const double dx=ztwoDarray_ptr->get_deltax();
         const double dy=ztwoDarray_ptr->get_deltay();

         const double max_gradient_magnitude_0=2.5;
         const double max_gradient_magnitude_1=2.75;
         const double max_gradient_magnitude_2=3.0;

         vector<double> delta_s=
            graphicsfunc::compute_delta_s_values(dx,dy);

// First generate binary twoDarray *p_roof_binary_twoDarray_ptr which
// classifies pixels as either belonging to rooftops or not:

         connectfunc::convert_pixel_list_to_binary_image(
            seed_list_ptr,p_roof_binary_twoDarray_ptr);

//         double x0=0.5*(minimum_x+maximum_x);
//         double y0=0.5*(minimum_y+maximum_y);
//         double r0=2*basic_math::max((maximum_x-minimum_x),(maximum_y-minimum_y));
//         string seedimage_filenamestr=imagedir+"rooftop_seeds_"
//            +stringfunc::number_to_string(n_rooftop)+".xyzp";
//         cout << "seedimage_filenamestr = " << seedimage_filenamestr << endl;
//         ladarfunc::write_local_xyzp_data(
//            x0,y0,r0,
 //           ztwoDarray_ptr,p_roof_binary_twoDarray_ptr,seedimage_filenamestr,
//            false);

// Scan over every unclassified pixel located inside the n_rooftop'th
// bounding box.  If it is adjacent to some pixel which has been
// classified as belong to the n_rooftop'th rooftop, form crude height
// derivative between that neighbor and the current unclassified
// pixel.  If the derivative's magnitude is sufficiently small, record
// the existence of a nearby neighbor whose height is similar to
// candidate pixel.  If a sufficient number of such nearby neighbors
// exist, declare the candidate pixel to belong to the rooftop.  In
// this fashion, "rooftop" classification oozes throughout the
// bounding box.
   
         unsigned int nchanges;
//       unsigned int iter=0;
         do
         {
            nchanges=0;
            for (unsigned int px=min_px; px<max_px; px++)
            {
               for (unsigned int py=min_py; py<max_py; py++)
               {
                  int n_eff=ndim*px+py;
                  double curr_z=ztwoDarray_ptr->get(n_eff);
                  if (curr_z > zlow_threshold &&
                      p_roof_binary_twoDarray_ptr->get(n_eff)==0)
                  {
                     int counter=0;
                     int n0_gradients=0;
                     int n1_gradients=0;
                     int n2_gradients=0;
                     
                     for (int i=-1; i<=1; i++)
                     {
                        for (int j=-1; j<=1; j++)
                        {
                           int n2_eff=ndim*(px+i)+(py+j);
                           if (p_roof_binary_twoDarray_ptr->get(n2_eff)==1)
                           {
                              double abs_z_deriv=
                                 fabs(ztwoDarray_ptr->get(n2_eff)-curr_z)/
                                 delta_s[counter];
                              if (abs_z_deriv < max_gradient_magnitude_0)
                              {
                                 n0_gradients++;
                              }
                              if (abs_z_deriv < max_gradient_magnitude_1)
                              {
                                 n1_gradients++;
                              }
                              if (abs_z_deriv < max_gradient_magnitude_2)
                              {
                                 n2_gradients++;
                              }
                           }
                           counter++;
                        } // loop over j index
                     } // loop over i index

                     if (n2_gradients >= 3 &&
                         n1_gradients >= 2 &&
                         n0_gradients >= 1)
                     {
                        p_roof_binary_twoDarray_ptr->put(n_eff,1);
                        nchanges++;
                     }
                  } // curr_z > zlow_threshold && p_roof_binary==0 conditional
               } // loop over py index
            } // loop over px index
//            if (iter%10==0)
//            {
//               cout << "iter = " << iter << " nchanges = " << nchanges 
//                    << endl;
//            }
//            iter++;
         }
         while (nchanges > 0);
//         outputfunc::newline();

// Recursively empty *p_roof_binary_twoDarray_ptr to eliminate small
// noise islands:

         binaryimagefunc::binary_reverse(
            min_px,max_px,min_py,max_py,p_roof_binary_twoDarray_ptr);
         recursivefunc::binary_fill(
            2,min_px,max_px,min_py,max_py,1,0,p_roof_binary_twoDarray_ptr);
         binaryimagefunc::binary_reverse(
            min_px,max_px,min_py,max_py,p_roof_binary_twoDarray_ptr);

/*
         string rooftop_filenamestr=imagedir+"roof_"+
            stringfunc::number_to_string(n_rooftop)+".xyzp";
         ladarfunc::write_local_xyzp_data(
            x0,y0,r0,
            ztwoDarray_ptr,p_roof_binary_twoDarray_ptr,rooftop_filenamestr,
            false);
*/

      }

// ---------------------------------------------------------------------
// Method accumulate_binary_rooftop_info adds binary information
// contained in *tmp_ptwoDarray_ptr onto cumulative binary image
// *p_refined_roof_twoDarray_ptr.  It also computes and returns the
// average height of the unit-valued entries within
// *tmp_ptwoDarray_ptr.

   double accumulate_binary_rooftop_info(
      unsigned int min_px,unsigned int min_py,
      unsigned int max_px,unsigned int max_py,
      twoDarray const *ztwoDarray_ptr,twoDarray const *tmp_ptwoDarray_ptr,
      twoDarray* p_refined_roof_twoDarray_ptr)
      {
         int ndim=p_refined_roof_twoDarray_ptr->get_ndim();
         int n_primary_roof_pixels=0;
         double z_sum=0;

         for (unsigned int px=min_px; px<max_px; px++)
         {
            for (unsigned int py=min_py; py<max_py; py++)
            {
               int n_eff=ndim*px+py;
               double curr_p=tmp_ptwoDarray_ptr->get(n_eff);
               if (curr_p > 0)
               {
                  p_refined_roof_twoDarray_ptr->put(
                     px,py,building_sentinel_value);
                  z_sum += ztwoDarray_ptr->get(n_eff);
                  n_primary_roof_pixels++;
               }
            }
         }
         double z_primary_roof_COM=z_sum/double(n_primary_roof_pixels);
         return z_primary_roof_COM;
      }

// ---------------------------------------------------------------------
// Method subtract_oozed_pixels scans through the original list of
// rooftop pixels and removes those which reside within
// *p_refined_roof_twoDarray_ptr:

   void subtract_oozed_pixels(
      linkedlist* currlist_ptr,twoDarray const *p_refined_roof_twoDarray_ptr)
      {
         mynode* curr_pixel_ptr=currlist_ptr->get_start_ptr();
         while (curr_pixel_ptr != NULL)
         {
            mynode* next_pixel_ptr=curr_pixel_ptr->get_nextptr();
            unsigned int px=basic_math::round(
               curr_pixel_ptr->get_data().get_var(0));
            unsigned int py=basic_math::round(
               curr_pixel_ptr->get_data().get_var(1));
            if (p_refined_roof_twoDarray_ptr->get(px,py)==
                building_sentinel_value)
            {
               currlist_ptr->delete_node(curr_pixel_ptr);
            }
            curr_pixel_ptr=next_pixel_ptr;
         }
      }

// ---------------------------------------------------------------------
// Method clean_ancillary_building_parts takes in hashtable
// *connected_bldgparts_hashtable_ptr for building n.  It compares
// the average height of each residual building part with the primary
// roof component's average z-value.  This method removes from the
// hashtable any "roof" component which lies higher than the primary
// roof component (and which in fact most likely corresponds to tree
// signal).

   void clean_ancillary_building_parts(
      int n_rooftop,string imagedir,
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      double z_primary_roof_COM,twoDarray const *ztwoDarray_ptr,
      polygon const *convexhull_ptr,
      twoDarray* p_ancillary_roof_twoDarray_ptr,
      Hashtable<linkedlist*>* connected_bldgparts_hashtable_ptr)
      {
         for (unsigned int j=0; j<connected_bldgparts_hashtable_ptr->
                 get_table_capacity(); j++)
         {
            Linkedlist<linkedlist*>* hashlist_ptr=
               connected_bldgparts_hashtable_ptr->get_list_ptr(j);
            if (hashlist_ptr != NULL)
            {
               Mynode<linkedlist*>* hashnode_ptr=hashlist_ptr->
                  get_start_ptr();

               while (hashnode_ptr != NULL)
               {
                  Mynode<linkedlist*>* next_hashnode_ptr=
                     hashnode_ptr->get_nextptr();
                  linkedlist* residual_pixels_list_ptr=
                     hashnode_ptr->get_data();

// Compute average z-value for residual pixels within jth connected
// component of nth building.  Then to reduce building extraction
// errors due to trees, discard any residual pixels which lie more
// than some maximum distance ABOVE the primary roof's COM.
                           
                  double residual_zCOM=pixels_average_height(
                     residual_pixels_list_ptr,ztwoDarray_ptr);

                  const double max_bldg_part_height_diff=0; // meters
                  if (residual_zCOM-z_primary_roof_COM >
                      max_bldg_part_height_diff)
                  {
//                     cout << "*********************************" 
//                          << endl;
//                     cout << "Building index n = " << n_rooftop << endl;
//                     cout << "Residual rooftop part index j = " 
//                          << j << endl;
//                     cout << "Residual z COM = " << residual_zCOM
//                          << endl;
//                     cout << "z_primary_roof_COM = " 
//                          << z_primary_roof_COM << endl;
//                     cout << "*********************************" 
//                          << endl;
                     hashlist_ptr->delete_node(hashnode_ptr);
                     connected_bldgparts_hashtable_ptr->
                        decrement_nkeys_in_table();
                  }
                  else
                  {
                     linkedlist* ancillary_seeds_list_ptr=
                        locate_ancillary_bldg_part_seeds(
                           convexhull_ptr,residual_pixels_list_ptr,
                           ztwoDarray_ptr);

                     unsigned int min_px,min_py,max_px,max_py;
                     ztwoDarray_ptr->bbox_corners_to_pixels(
                        minimum_x,minimum_y,maximum_x,maximum_y,
                        min_px,min_py,max_px,max_py);

                     ooze_rooftop_pixels(
                        n_rooftop,minimum_x,minimum_y,maximum_x,maximum_y,
                        min_px,min_py,max_px,max_py,
                        ancillary_seeds_list_ptr,ztwoDarray_ptr,
                        p_ancillary_roof_twoDarray_ptr,imagedir);

                     delete ancillary_seeds_list_ptr;
                  }
                  hashnode_ptr=next_hashnode_ptr;
               } // hashnode_ptr != NULL while loop
            } // hashlist_ptr != NULL conditional
         } // loop over index j labeling hashtable's linked lists
      }

// ---------------------------------------------------------------------
// Method pixels_average_height computes the average z-value for the
// pixels within the input linked list.

   double pixels_average_height(linkedlist const *pixels_list_ptr,
                                twoDarray const *ztwoDarray_ptr)
      {
         double zCOM=0;
         if (pixels_list_ptr != NULL)
         {
            double zsum=0;
            for (mynode const *curr_pixel_ptr=pixels_list_ptr->
                    get_start_ptr(); curr_pixel_ptr != NULL;
                 curr_pixel_ptr=curr_pixel_ptr->get_nextptr())
            {
               unsigned int px=basic_math::round(
                  curr_pixel_ptr->get_data().get_var(0));
               unsigned int py=basic_math::round(
                  curr_pixel_ptr->get_data().get_var(1));
               zsum += ztwoDarray_ptr->get(px,py);
            }
            zCOM=zsum/double(pixels_list_ptr->size());
         } // pixels_list_ptr != NULL conditional
         return zCOM;
      }

// ---------------------------------------------------------------------
// Method locate_ancillary_bldg_part_seeds takes in the convex hull
// for some building's primary rooftop component within polygon
// *hull_ptr.  It also takes in linked list *pixels_list_ptr for some
// connected ancillary component of the building's rooftop pixels.
// This method first computes the distance from each pixel within this
// list to the convex hull and discards any pixel which lies too far
// away from the primary rooftop component.  It subsequently forms the
// height distribution of the surviving pixels and computes the
// distribution's median.  All ancillary component pixels whose
// heights lie too far away from the median are discarded.  The
// surviving ancillarly component pixels are returned within a linked
// list.

   linkedlist* locate_ancillary_bldg_part_seeds(
      polygon const *hull_ptr,linkedlist const *pixels_list_ptr,
      twoDarray const *ztwoDarray_ptr)
      {
         if (pixels_list_ptr != NULL)
         {
            int counter=0;
            int n_nodes=pixels_list_ptr->size();
            unsigned int px[n_nodes],py[n_nodes];
            double pixel_to_hull_dist[n_nodes],z[n_nodes];
            threevector residual_point;

            const double max_ancillary_dist_from_primary_rooftop=2; // meters
            
            for (mynode const *curr_pixel_ptr=pixels_list_ptr->
                    get_start_ptr(); curr_pixel_ptr != NULL;
                 curr_pixel_ptr=curr_pixel_ptr->get_nextptr())
            {
               int curr_px=basic_math::round(
                  curr_pixel_ptr->get_data().get_var(0));
               int curr_py=basic_math::round(
                  curr_pixel_ptr->get_data().get_var(1));
               ztwoDarray_ptr->pixel_to_point(
                  curr_px,curr_py,residual_point);
               double curr_pixel_to_hull_dist=hull_ptr->
                  min_dist_to_convex_polygon(residual_point);
               if (curr_pixel_to_hull_dist < 
                   max_ancillary_dist_from_primary_rooftop)
               {
                  px[counter]=curr_px;
                  py[counter]=curr_py;
                  z[counter]=ztwoDarray_ptr->get(curr_px,curr_py);
                  pixel_to_hull_dist[counter]=curr_pixel_to_hull_dist;
                  counter++;
               }
            }

            prob_distribution prob_z(counter,z,30);
            double z_median=prob_z.median();
            const double max_z_discrepancy=2;	// meters

            linkedlist* seed_pixels_list_ptr=new linkedlist;

            const int n_indep_variables=2;
            const int n_depend_variables=2;
            double var[n_indep_variables],func[n_depend_variables];
            for (int n=0; n<counter; n++)
            {
               if (fabs(z[n]-z_median) < max_z_discrepancy)
               {
                  var[0]=px[n];
                  var[1]=py[n];
                  func[0]=z[n];
                  func[1]=pixel_to_hull_dist[n];
                  seed_pixels_list_ptr->append_node(
                     datapoint(n_indep_variables,n_depend_variables,
                               var,func));
               }
            }
            return seed_pixels_list_ptr;
         } // pixels_list_ptr != NULL conditional
         else
         {
            return NULL;
         }
      }

// ---------------------------------------------------------------------
// Method accumulate_building_parts adds extra building parts
// information (e.g. awnings, decks, multi-tiered roofs) contained
// within input twoDarray *pconnected_components_twoDarray_ptr onto
// the cumulative image in *p_refined_roof_twoDarray_ptr.

   void accumulate_building_parts(
      unsigned int min_px,unsigned int min_py,
      unsigned int max_px,unsigned int max_py,
      twoDarray const *pconnected_components_twoDarray_ptr,
      twoDarray *p_refined_roof_twoDarray_ptr)
      {
         unsigned int ndim=pconnected_components_twoDarray_ptr->get_ndim();
         for (unsigned int px=min_px; px<max_px; px++)
         {
            for (unsigned int py=min_py; py<max_py; py++)
            {
               int n_eff=ndim*px+py;
               double curr_p=pconnected_components_twoDarray_ptr->get(n_eff);
               if (curr_p > 0)
               {
                  p_refined_roof_twoDarray_ptr->put(
                     px,py,draw3Dfunc::annotation_value2);
               }
            }
         }
      }

// ---------------------------------------------------------------------
// Method compute_buildings_COM_locations takes in hashtable
// *connected_hashtable_ptr whicih contains linkedlists of connected
// pixel components corresponding to building rooftops.  For each
// rooftop cluster of pixels, it calculates the center-of-mass.

   void compute_building_COM_locations(
      Hashtable<linkedlist*> const *connected_hashtable_ptr,
      twoDarray *ptwoDarray_ptr)
      {
         outputfunc::write_banner("Computing building COM positions:");

         threevector COM;
         for (unsigned int n=0; n<connected_hashtable_ptr->size(); n++)
         {
            linkedlist* currlist_ptr=
               ladarfunc::retrieve_connected_pixel_list(
               n,connected_hashtable_ptr);
            if (currlist_ptr != NULL)
            {
               COM=connectfunc::pixel_list_COM(
                  currlist_ptr,ptwoDarray_ptr,true);

               double radius=1.5;	// meter
               double intensity_value=0.5;
//               double intensity_value=100;
               drawfunc::draw_hugepoint(
                  COM,radius,intensity_value,ptwoDarray_ptr);
            
               outputfunc::newline();
               cout << "COM = " << COM << endl;
               outputfunc::newline();
            } // currlist_ptr != NULL conditional
         } // loop over index n labeling connected component
      }

// ---------------------------------------------------------------------
// Method classify_tree_building_pixels takes in an intensity
// twoDarray *ptwoDarray_ptr, a marked height twoDarray
// *ztwoDarray_ptr and a tall object sentinel value.  It returns a
// dynamically generated twoDarray in which every tall pixel is
// assigned either a building or tree sentinel value.

   twoDarray* classify_tree_building_pixels(
      double p_tall_sentinel_value,twoDarray const *ptwoDarray_ptr,
      twoDarray const *ztall_twoDarray_ptr)
      {
         const double building_zvalue=50;	 // meters

         twoDarray* ftwoDarray_ptr=new twoDarray(ptwoDarray_ptr);
         ptwoDarray_ptr->copy(ftwoDarray_ptr);
         for (unsigned int px=0; px<ftwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ftwoDarray_ptr->get_ndim(); py++)
            {
               double curr_p=ptwoDarray_ptr->get(px,py);
               if (curr_p==p_tall_sentinel_value)
               {
                  double curr_z=ztall_twoDarray_ptr->get(px,py);
                  if (curr_z > building_zvalue)
                  {
                     ftwoDarray_ptr->put(
                        px,py,building_sentinel_value);
                  }
                  else
                  {
                     ftwoDarray_ptr->put(
                        px,py,tree_sentinel_value);
                  }
               }
            } // py loop
         } // px loop
         return ftwoDarray_ptr;
      }

// ==========================================================================
// Feature clustering & contour methods
// ==========================================================================

// Method extract_feature_clusters generates a binary image which
// pulls out specific feature information from the input feature map.
// It subsequently generates and returns a hashtable whose linked
// lists contain connected pixels corresponding to the input
// feature_sentinel_value.

   Hashtable<linkedlist*>* extract_feature_clusters(
      double min_footprint_area,double feature_sentinel_value,
      string feature_type,string imagedir,
      twoDarray const *ztwoDarray_ptr,twoDarray const *features_twoDarray_ptr)
      {
         twoDarray* pbinary_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
         binaryimagefunc::binary_threshold_for_particular_cutoff(
            feature_sentinel_value,features_twoDarray_ptr,
            pbinary_twoDarray_ptr,0);

//         string binary_feature_filename=imagedir+"binary_"+feature_type
//            +".xyzp";   
//         xyzpfunc::write_xyzp_data(
//            ztwoDarray_ptr,pbinary_twoDarray_ptr,binary_feature_filename,
//            false);

         Hashtable<linkedlist*>* connected_feature_components_hashtable_ptr=
            connected_feature_components(
               min_footprint_area,feature_type,imagedir,
               ztwoDarray_ptr,pbinary_twoDarray_ptr);
         delete pbinary_twoDarray_ptr;

         return connected_feature_components_hashtable_ptr;
      }

// ---------------------------------------------------------------------
// Method connected_feature_components generates and returns a
// hashtable containing linked lists of connected pixels corresponding
// to some particular urban image feature (e.g. trees or building
// rooftops).

   Hashtable<linkedlist*>* connected_feature_components(
      double min_footprint_area,string feature_type,string imagedir,
      twoDarray const *ztwoDarray_ptr,twoDarray const *pbinary_twoDarray_ptr)
      {
//         twoDarray* pconnected_twoDarray_ptr=
//            ladarfunc::connect_binary_components(
//               min_footprint_area,pbinary_twoDarray_ptr);
//         string connected_filename=imagedir+"connected_"+
//           feature_type+"_components.xyzp";
//         xyzpfunc::write_xyzp_data(
//            ztwoDarray_ptr,pconnected_twoDarray_ptr,
//            connected_filename,false);
//         delete pconnected_twoDarray_ptr;
         
         Hashtable<linkedlist*>* 
            connected_feature_components_hashtable_ptr=
            ladarfunc::generate_connected_binary_components_hashtable(
               min_footprint_area,pbinary_twoDarray_ptr);
         cout << "Number of connected "+feature_type+" components = "
              << connected_feature_components_hashtable_ptr->
            size() << endl;
         return connected_feature_components_hashtable_ptr;
      }

// ---------------------------------------------------------------------
// Method fit_deformable_contours_around_feature_clusters loops over
// each connected feature component within the input hashtable.  It
// fits a deformable contour around each feature cluster and draws a
// contour cylinder into the xyzp binary file specified by input
// string features_filenamestr.  

   void fit_deformable_contours_around_feature_clusters(
      string feature_type,string imagedir,string features_filenamestr,
      twoDarray const *ztwoDarray_ptr,
      Hashtable<linkedlist*>* connected_feature_components_hashtable_ptr)
      {
         twoDarray* fmask_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);

         int n_cluster=0;
         for (unsigned int i=0; i<connected_feature_components_hashtable_ptr->
                 get_table_capacity(); i++)
         {
            Linkedlist<linkedlist*>* hashlist_ptr=
               connected_feature_components_hashtable_ptr->get_list_ptr(i);
            if (hashlist_ptr != NULL)
            {
               Mynode<linkedlist*>* hashnode_ptr=hashlist_ptr->
                  get_start_ptr();
               while (hashnode_ptr != NULL)
               {
                  linkedlist* pixel_list_ptr=hashnode_ptr->get_data();

                  fmask_twoDarray_ptr->initialize_values(
                     xyzpfunc::null_value);
                  connectfunc::convert_pixel_list_to_twoDarray(
                     pixel_list_ptr,fmask_twoDarray_ptr);

                  cout << "Connected "+feature_type+" component #" 
                       << n_cluster+1 << " of " 
                       << connected_feature_components_hashtable_ptr->size()
                       << " contains " << pixel_list_ptr->size()
                       << " pixels" << endl;

// Compute contour for current connected feature component:

                  contour* feature_contour_ptr=
                     generate_feature_cluster_contour(
                        n_cluster,pixel_list_ptr,
                        ztwoDarray_ptr,fmask_twoDarray_ptr);

// Draw cylinder surrounding pixel cluster within feature map:

                  draw3Dfunc::draw_contour_cylinder(
                     *feature_contour_ptr,features_filenamestr,
                     draw3Dfunc::annotation_value1);

// Generate rasterized mask of feature cluster's contour:

                  rasterize_feature_cluster_contour(
                     n_cluster,imagedir,*feature_contour_ptr,
                     ztwoDarray_ptr,fmask_twoDarray_ptr);

// Append filled footprint to XYZP file showing feature cluster and its
// contour cylinder:

                  double minimum_x,minimum_y,maximum_x,maximum_y;
                  feature_contour_ptr->locate_extremal_xy_points(
                     minimum_x,minimum_y,maximum_x,maximum_y);
                  draw3Dfunc::color_binary_region(
                     features_filenamestr,minimum_x,minimum_y,maximum_x,
                     maximum_y,0.0,draw3Dfunc::annotation_value2,
                     fmask_twoDarray_ptr);

                  delete feature_contour_ptr;

                  n_cluster++;
                  hashnode_ptr=hashnode_ptr->get_nextptr();

               } // hashnode_ptr != NULL while loop
            } // hashlist_ptr != NULL conditional
         } // loop over index i labeling linked lists in connected feature 
          //    components hashtable
         delete fmask_twoDarray_ptr;
      }
   
// ---------------------------------------------------------------------
// Method generate_feature_cluster_contour takes in linkedlist
// *pixel_list_ptr containing contiguous pixels representing some
// feature.  It first computes the convex hull for this pixel cluster.
// It next uses the hull to initialize a deformable contour
// surrounding the feature clump labeled by input integer n_cluster.
// It subsequently shrinks the contour around the pixel cluster so
// that it tightly "hugs" the feature cluster pixels.  This method
// returns a pointer to the dynamically generated feature cluster
// contour.
   
   contour* generate_feature_cluster_contour(
      int n_cluster,linkedlist const *pixel_list_ptr,
      twoDarray const *ztwoDarray_ptr,twoDarray* fmask_twoDarray_ptr)
      {

// Compute convex hull for current feature cluster:

         polygon* hull_poly_ptr=convexhull::convex_hull_poly(pixel_list_ptr);
         ztwoDarray_ptr->convert_pixel_to_posn_polygon_vertices(
            hull_poly_ptr);

//            drawfunc::draw_polygon(*hull_poly_ptr,colorfunc::red,
//                                   fmask_twoDarray_ptr);
//            string hull_filename=imagedir+
//               "hull_"+stringfunc::number_to_string(n_cluster)+".xyzp";   
//            xyzpfunc::write_xyzp_data(
//               ztwoDarray_ptr,fmask_twoDarray_ptr,hull_filename,
// 		 false);

         contour* feature_contour_ptr=new contour(hull_poly_ptr);
         delete hull_poly_ptr;

         const double delta_s=1.0;	// meter
//         const double delta_s=1.25;	// meter

         feature_contour_ptr->regularize_vertices(delta_s);
//         string contour_filename=
//            "contour_"+stringfunc::number_to_string(n_cluster)+".xyzp";
//         xyzpfunc::write_xyzp_data(
//            ztwoDarray_ptr,fmask_twoDarray_ptr,contour_filename,false);

// On 4/18/05, we discovered that erroneously huge tree clusters
// (which really need to be decomposed into smaller groups of
// clusters) require more than 100 iterations of shrink wrapping.  So
// we allow n_max_iters to grow with the number of vertices within
// *feature_contour_ptr:
         
         const double delta_r=0.33;	// meter
         int n_max_iters=basic_math::max(
	   100,int(feature_contour_ptr->get_nvertices()));

         graphicsfunc::shrink_wrap_regularized_contour(
            xyzpfunc::null_value,*feature_contour_ptr,fmask_twoDarray_ptr,
            delta_r,n_max_iters);
         graphicsfunc::avg_heights_at_contour_vertices(
            xyzpfunc::null_value,*feature_contour_ptr,fmask_twoDarray_ptr,
            ztwoDarray_ptr);
         feature_contour_ptr->filter_z_values(2.0);

//         drawfunc::draw_contour(
//            *feature_contour_ptr,4,7,0.5,fmask_twoDarray_ptr,true,true);

         return feature_contour_ptr;
      }

// ---------------------------------------------------------------------
// This overloaded version of generate_feature_cluster_contour() was
// stripped down from the preceding version in Aug 04 for cityblock
// extraction purposes.

   contour* generate_feature_cluster_contour(
      double delta_s,double znull,linkedlist const *pixel_list_ptr,
      twoDarray const *fmask_twoDarray_ptr)
      {
         polygon* hull_poly_ptr=convexhull::convex_hull_poly(pixel_list_ptr);
         fmask_twoDarray_ptr->convert_pixel_to_posn_polygon_vertices(
            hull_poly_ptr);

         contour* feature_contour_ptr=new contour(hull_poly_ptr);
         delete hull_poly_ptr;
         feature_contour_ptr->regularize_vertices(delta_s);

         const double delta_r=0.33;	// meter
         graphicsfunc::shrink_wrap_regularized_contour(
            znull,*feature_contour_ptr,fmask_twoDarray_ptr,delta_r);
         return feature_contour_ptr;
      }

// ---------------------------------------------------------------------
// Method rasterize_feature_cluster_contour returns a rasterized mask
// of the input feature cluster's contour within *fmask_twoDarray_ptr.

   void rasterize_feature_cluster_contour(
      int n_cluster,string imagedir,const contour& c,
      twoDarray const *ztwoDarray_ptr,twoDarray* fmask_twoDarray_ptr)
      {
         const double p_boundary=2;
         const double p_vertex=p_boundary;

// In general, contour c has non-zero valued z entries at each of its
// vertices.  For rasterization purposes, we need to work with the
// projection of contour c into the xy plane:

         contour c_projection(c);
         c_projection.xy_projection();

         fmask_twoDarray_ptr->initialize_values(xyzpfunc::null_value);
         drawfunc::draw_contour(
            c_projection,p_boundary,p_vertex,0.5,fmask_twoDarray_ptr,
            true,false);
//            string contour_filenamestr=imagedir+
//               "contour_"+stringfunc::number_to_string(n_cluster)+".xyzp";
//            xyzpfunc::write_xyzp_data(
//               ztwoDarray_ptr,fmask_twoDarray_ptr,contour_filenamestr,
//		 true);

         threevector origin(c_projection.robust_locate_origin());
         if (!c_projection.point_inside(origin))
         {
            cout << "Trouble in featurefunc::rasterize_feature_cluster_contour"
                 << endl;
            cout << "origin = " << origin << endl;
            cout << "origin may not lie inside contour!!!" << endl;
            cout << "n_cluster = " << n_cluster << endl;
//            cout << "contour = " << c << endl;
         }

         unsigned int px_origin,py_origin;
         fmask_twoDarray_ptr->point_to_pixel(origin,px_origin,py_origin);

// Perform "paint program" fill to color contour's interior:

         int npixels_filled=0;
         const double p_fill=1;
         recursivefunc::boundaryFill(
            npixels_filled,px_origin,py_origin,p_fill,p_boundary,
            fmask_twoDarray_ptr);
//         cout << "npixels filled = " << npixels_filled << endl;
//         string mask_filenamestr=imagedir+
//            "mask_"+stringfunc::number_to_string(n_cluster)+".xyzp";
//         xyzpfunc::write_xyzp_data(
//            ztwoDarray_ptr,fmask_twoDarray_ptr,mask_filenamestr,true);
      }
   
// ==========================================================================
// Car detection methods
// ==========================================================================

// Method subtract_genuine_ground_asphalt_pixels takes in a cleaned,
// flattened height image within *ztwoDarray_ptr as well as a finer
// height image for asphalt pixels within *ooze_asphalt_twoDarray_ptr.
// This method subtracts the latter from the former and returns the
// height difference image within a dynamically generated twoDarray.
// The output twoDarray hopefully provides starting seed information
// for car detection.

   twoDarray* subtract_genuine_ground_asphalt_pixels(
      twoDarray const *ztwoDarray_ptr,twoDarray const *ftwoDarray_ptr,
      twoDarray const *ooze_asphalt_twoDarray_ptr)
      {
         twoDarray* z_cars_twoDarray_ptr=new twoDarray(ftwoDarray_ptr);
         z_cars_twoDarray_ptr->initialize_values(xyzpfunc::null_value);

         const double min_height_threshold=0.75;	// meter
         const double max_height_threshold=4;	// meters
         for (unsigned int px=0; px<ftwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ftwoDarray_ptr->get_ndim(); py++)
            {
               double curr_z=ztwoDarray_ptr->get(px,py);
               double curr_f=ftwoDarray_ptr->get(px,py);
               double curr_zground=ooze_asphalt_twoDarray_ptr->get(px,py);
               double delta_height=curr_z-curr_zground;
               if (nearly_equal(curr_f,road_sentinel_value) &&
                   delta_height > min_height_threshold &&
                   delta_height < max_height_threshold)
               {
                  z_cars_twoDarray_ptr->put(px,py,delta_height);
               }
            } // loop over py index
         } // loop over px index

// Apply a little bit of recursive emptying to remove isolated height
// noise spikes:

         int nrecursion_max=3;
         double zmin=-1;	// meter
         recursivefunc::recursive_empty(
            nrecursion_max,zmin,z_cars_twoDarray_ptr,false,
            xyzpfunc::null_value);

         return z_cars_twoDarray_ptr;
      }

// ---------------------------------------------------------------------
// Method color_local_height_bumps takes in a height image which is
// assumed to basically be a binary map indicating local height
// clusters.  The color of every non-null valued pixel within this
// input height image is reset to annotation_value in the features
// image within *features_twoDarray_ptr.

   void color_local_height_bumps(
      twoDarray const *zbump_twoDarray_ptr,
      twoDarray* features_twoDarray_ptr,double annotation_value)
      {
         for (unsigned int px=0; px<features_twoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<features_twoDarray_ptr->get_ndim(); 
                 py++)
            {
               double curr_z=zbump_twoDarray_ptr->get(px,py);
               if (curr_z > xyzpfunc::null_value)
               {
                  features_twoDarray_ptr->put(px,py,annotation_value);
               }
            } // loop over py index
         } // loop over px index
      }

// ---------------------------------------------------------------------
// This overloaded version of ooze_rooftop_pixels() was written in Oct
// 2009 for automatically constructing 3D bboxes around NYC
// skyscrapers.  It instantiates a mask within
// *p_roof_binary_twoDarray_ptr and sets its world-space coordinate
// system to be centered around input rooftop_posn with XY extents
// fixed by max_radius.  Assuming that the center pixel within
// *p_roof_binary_twoDarray_ptr corresponds to a building's rooftop,
// this method brute-force iterates over all pixels within the mask
// and computes which are connected to rooftop-classified pixels and
// have heights greater than a height threshold fraction of
// rooftop_posn.get(2).  This method returns within
// *p_roof_binary_twoDarray_ptr a mask for all tall pixels connected
// to the initial rooftop_posn pixel.

   twoDarray* ooze_rooftop_pixels(
      double max_radius,const threevector& rooftop_posn,
      twoDarray const *ztwoDarray_ptr,double& max_roof_z)
      {
         cout << "inside featurefunc::ooze_rooftop_pixels()" << endl;
//         cout << "*ztwoDarray_ptr = " << *ztwoDarray_ptr << endl;
//         cout << "rooftop_posn = " << rooftop_posn << endl;
         
         const double dx=ztwoDarray_ptr->get_deltax();
         const double dy=ztwoDarray_ptr->get_deltay();
         double ds=basic_math::min(dx,dy);
         unsigned int p_mdim=2*max_radius/ds+1;
         unsigned int p_ndim=p_mdim;

         twoDarray* p_roof_binary_twoDarray_ptr=new twoDarray(p_mdim,p_ndim);

         p_roof_binary_twoDarray_ptr->init_coord_system(
            rooftop_posn.get(0)-max_radius,rooftop_posn.get(0)+max_radius,
            rooftop_posn.get(1)-max_radius,rooftop_posn.get(1)+max_radius);
         p_roof_binary_twoDarray_ptr->clear_values();
         p_roof_binary_twoDarray_ptr->put(p_mdim/2,p_ndim/2,1);

//         cout << "*p_roof_binary_twoDarray_ptr = "
//              << *p_roof_binary_twoDarray_ptr << endl;

         int iter=0;
         int n_changes=1;
         int n_rooftop_pixels=0;
         max_roof_z=rooftop_posn.get(2);

         while (n_changes > 0)
         {
            n_changes=0;
            double x,y;
            for (unsigned int px=1; px<p_mdim-1; px++)
            {
               p_roof_binary_twoDarray_ptr->px_to_x(px,x);
               for (unsigned int py=1; py<p_ndim-1; py++)
               {
                  p_roof_binary_twoDarray_ptr->py_to_y(py,y);

                  if (p_roof_binary_twoDarray_ptr->get(px,py)==1) continue;

                  unsigned int qx,qy;
                  if (!ztwoDarray_ptr->point_to_pixel(x,y,qx,qy)) continue;
                  double z=ztwoDarray_ptr->get(qx,qy);

                  double height_frac=z/rooftop_posn.get(2);
//                  const double height_frac_threshold=0.25;
                  const double height_frac_threshold=0.50;
                  if (height_frac > height_frac_threshold)
                  {
                     bool continuous_roof_flag=false;
                     for (int i=-1; i<=1; i++)
                     {
                        for (int j=-1; j<=1; j++)
                        {
                           if (p_roof_binary_twoDarray_ptr->get(px+i,py+j)==1)
                           {
                              continuous_roof_flag=true;
//                              break;
                           }
                        } // loop over index j
                     } // loop over index i
                  
                     if (continuous_roof_flag)
                     {
                        p_roof_binary_twoDarray_ptr->put(px,py,1);
                        max_roof_z=basic_math::max(max_roof_z,z);
                        n_rooftop_pixels++;
//                        cout << "px = " << px << " py = " << py
//                             << " frac = " << height_frac << endl;
                        n_changes++;
                     }
                  } // z > height threshold conditional
               } // loop over py index
            } // loop over px index
         
            iter++;
            cout << "iter = " << iter 
                 << " n_rooftop_pixels = " << n_rooftop_pixels 
                 << " sqrt(pixels) = " << sqrt(n_rooftop_pixels)
                 << endl;
         } // while n_changes > 0 

         return p_roof_binary_twoDarray_ptr;
      }

// ---------------------------------------------------------------------
// Method construct_rooftop_binary_mask() takes in the approximate
// posn for some NYC skyscraper rooftop center as well as some height
// map within *ztwoDarray_ptr.  After performing an oozing operation,
// this method computes a binary mask for all pixels connected to the
// rooftop_posn pixel with heights about a certain fractional
// threshold.  It then computes the mask's XY angular orientation theta in
// the XY plane from its moment of inertia eigenvalues and
// eigenvectors.  

   twoDarray* construct_rooftop_binary_mask(
      const threevector& rooftop_posn,twoDarray const *ztwoDarray_ptr,
      threevector& COM,double& theta,double& max_roof_z)
      {
         cout << "inside featurefunc::construct_rooftop_binary_mask()" 
              << endl;

         cout << "rooftop posn = " << rooftop_posn << endl;
         
         double max_radius=250;	// meters
         twoDarray* p_roof_binary_twoDarray_ptr=ooze_rooftop_pixels(
            max_radius,rooftop_posn,ztwoDarray_ptr,max_roof_z);
         cout << "max_roof_z = " << max_roof_z << endl;

//         int mdim=p_roof_binary_twoDarray_ptr->get_mdim();
//         int ndim=p_roof_binary_twoDarray_ptr->get_ndim();
//         texture_rectangle* texture_rectangle_ptr=
//            new texture_rectangle(mdim,ndim,1,3,NULL);
//         texture_rectangle_ptr->initialize_twoDarray_image(
//            p_roof_binary_twoDarray_ptr);
//         texture_rectangle_ptr->write_curr_frame("p_roof_binary.png");
//         delete texture_rectangle_ptr;

         threevector Imin_hat,Imax_hat;
         imagefunc::center_of_mass(p_roof_binary_twoDarray_ptr,COM);
         cout << "COM = " << COM << endl;
         double Imin,Imax;
         imagefunc::moment_of_inertia(
            COM,Imin,Imax,Imin_hat,Imax_hat,
            p_roof_binary_twoDarray_ptr);
         cout << "Imin = " << Imin << " Imax = " << Imax << endl;
         cout << "Imin_hat = " << Imin_hat 
              << " Imax_hat = " << Imax_hat << endl;

         theta=atan2(Imax_hat.get(1),Imax_hat.get(0));
         cout << "theta = " << theta*180/PI << endl;

//         p_roof_binary_twoDarray_ptr->rotate(COM,-theta);
//         texture_rectangle* rot_texture_rectangle_ptr=new texture_rectangle(
//            mdim,ndim,1,3,AnimationController_ptr);
//         rot_texture_rectangle_ptr->initialize_twoDarray_image(
//            p_roof_binary_twoDarray_ptr);
//         rot_texture_rectangle_ptr->write_curr_frame("rot_p_roof_binary.png");
//         delete rot_texture_rectangle_ptr;

         return p_roof_binary_twoDarray_ptr;
      }
   
} // featurefunc namespace




