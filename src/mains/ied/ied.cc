// ==========================================================================
// Program IED reads in and interprets raw ALIRT xyzp data.  In
// particular, it attempts to classify trees, buildings, grass and
// roadsides on a per-pixel basis
// ==========================================================================
// Last updated on 5/22/05; 4/24/06; 10/30/07
// ==========================================================================

#include <iomanip>
#include <iostream>
#include <set>
#include <string>
#include <vector>
#include "image/binaryimagefuncs.h"
#include "image/connectfuncs.h"
#include "threeDgraphics/draw3Dfuncs.h"
#include "ladar/featurefuncs.h"
#include "general/filefuncs.h"
#include "image/graphicsfuncs.h"
#include "ladar/groundfuncs.h"
#include "datastructures/Hashtable.h"
#include "image/imagefuncs.h"
#include "ladar/ladarfuncs.h"
#include "general/outputfuncs.h"
#include "image/recursivefuncs.h"
#include "general/sysfuncs.h"
#include "image/TwoDarray.h"
#include "urban/urbanfuncs.h"
#include "urban/urbanimage.h"
#include "threeDgraphics/xyzpfuncs.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ifstream;
   using std::ios;
   using std::istream;
   using std::ofstream;
   using std::ostream;
   using std::pair;
   using std::string;
   using std::vector;

   std::set_new_handler(sysfunc::out_of_memory);
  
   bool input_param_file;
   unsigned int ninputlines,currlinenumber;
   string inputline[200];

   filefunc::parameter_input(
      argc,argv,input_param_file,inputline,ninputlines);
   currlinenumber=0;

// ==========================================================================
// Raw image initialization
// ==========================================================================

// Read contents of raw binary xyzp file:
   
   urbanimage cityimage;
//    cityimage.set_public_software(true);

// Chimney footprint dimensions:
//   const double delta_x=0.3;	// meters
//   const double delta_y=0.3;	// meters
   const double delta_x=0.15;	// meters
   const double delta_y=0.15;	// meters
   cityimage.initialize_image(input_param_file,inputline,currlinenumber);
   cityimage.parse_and_store_input_data(
      delta_x,delta_y,true,false,false,false,0);
   cityimage.compute_trivial_xy_data_bbox(cityimage.get_z2Darray_ptr());

// ==========================================================================
// Median filling
// ==========================================================================

// Use median filling and isolated outlier elimination to clean up raw
// height imagery.  On 8/12/04, we discovered that rotated IED xyzp
// data appears to have systematic blank spots.  We definitely need to
// perform a little bit of median filling in order to remove these
// spots.  The ALIRT scan pattern also leaves holes in the data which
// are probably best filled in as well.

   int niters=5;
   int nsize=3;
   ladarfunc::median_fill_image(niters,nsize,cityimage.get_data_bbox_ptr(),
                                cityimage.get_z2Darray_ptr());
   ladarfunc::remove_isolated_outliers(
      cityimage.get_data_bbox_ptr(),cityimage.get_z2Darray_ptr());

// Plot height distribution for image with original ground surface:

   ladarfunc::compute_z_distribution(
      cityimage.get_imagedir(),cityimage.get_z2Darray_ptr());

   string filled_zimage_filename=cityimage.get_imagedir()+
      "filled_zimage.xyzp";
   xyzpfunc::write_xyzp_data(
      cityimage.get_z2Darray_ptr(),cityimage.get_p2Darray_ptr(),
      filled_zimage_filename);
   draw3Dfunc::append_fake_z_points_in_twoDarray_middle(
      cityimage.get_z2Darray_ptr(),filled_zimage_filename);

   filefunc::gunzip_file_if_gzipped(filled_zimage_filename);
//   draw3Dfunc::draw_thick_polygon(
//      (*cityimage.get_data_bbox_ptr()),filled_zimage_filename,
//      urbanimage::annot1_value);
//   ladarfunc::draw_xy_coordinate_system(
//      filled_zimage_filename,100,cityimage.get_z2Darray_ptr(),50);

// Compute distributions for filled z and p images:
   
//   cityimage.plot_zp_distributions(
//      cityimage.get_z2Darray_ptr(),cityimage.get_p2Darray_ptr());

// Create binary mask of partially cleaned height image:

   twoDarray* mask_twoDarray_ptr=new twoDarray(cityimage.get_z2Darray_ptr());
   binaryimagefunc::binary_threshold(
      0.5*xyzpfunc::null_value,cityimage.get_z2Darray_ptr(),
      mask_twoDarray_ptr,xyzpfunc::null_value);

   int n_iters=5;
   int n_size=5;
   graphicsfunc::turtle_erode_binary_region(
      n_iters,n_size,mask_twoDarray_ptr,xyzpfunc::null_value);

   string mask_filename=cityimage.get_imagedir()+"mask.xyzp";
   xyzpfunc::write_xyzp_data(
      cityimage.get_z2Darray_ptr(),mask_twoDarray_ptr,mask_filename);
//   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
//      mask_twoDarray_ptr,mask_filename);

// ==========================================================================
// Mario bashing and local ground oozing
// ==========================================================================

// Copy current working image into a new, dynamically generated
// twoDarray *zground_silhouetted_twoDarray_ptr which will be
// destructively processsed:

   twoDarray* zground_silhouetted_twoDarray_ptr=
      new twoDarray(cityimage.get_z2Darray_ptr());
   cityimage.get_z2Darray_ptr()->copy(zground_silhouetted_twoDarray_ptr);

// Use gradient field information to locate relatively high objects
// within median filled height image:

   const double spatial_resolution=0.5*(delta_x+delta_y);	// meter
   twoDarray* xderiv_twoDarray_ptr=new twoDarray(cityimage.get_z2Darray_ptr());
   twoDarray* yderiv_twoDarray_ptr=new twoDarray(cityimage.get_z2Darray_ptr());

   bool mask_points_near_border=true;
   cityimage.estimate_ground_surface_using_gradient_info(
      spatial_resolution,xderiv_twoDarray_ptr,yderiv_twoDarray_ptr,
      zground_silhouetted_twoDarray_ptr,mask_points_near_border,
      mask_twoDarray_ptr);
   delete xderiv_twoDarray_ptr;
   delete yderiv_twoDarray_ptr;
   delete mask_twoDarray_ptr;

   twoDarray* zlevel_twoDarray_ptr=
      cityimage.interpolate_and_flatten_ground_surface(
         cityimage.get_z2Darray_ptr(),zground_silhouetted_twoDarray_ptr,
         cityimage.get_p2Darray_ptr(),15);
   delete zground_silhouetted_twoDarray_ptr;
   recursivefunc::binary_null(
      0.5*xyzpfunc::null_value,zlevel_twoDarray_ptr,
      cityimage.get_z2Darray_ptr(),xyzpfunc::null_value);

   string zlevel_filename=cityimage.get_imagedir()+"zlevel.xyzp";
   xyzpfunc::write_xyzp_data(
      zlevel_twoDarray_ptr,cityimage.get_p2Darray_ptr(),
      zlevel_filename);
   draw3Dfunc::append_fake_z_points_in_twoDarray_middle(
      cityimage.get_z2Darray_ptr(),zlevel_filename);

// Find relatively low and high regions within z-image.  Then use
// "oozing" technique to completely flatten ground:

   twoDarray* zhilo_twoDarray_ptr=new twoDarray(zlevel_twoDarray_ptr);
   groundfunc::find_low_local_pixels(
      zlevel_twoDarray_ptr,zhilo_twoDarray_ptr);
//   string zhilo_filename=cityimage.get_imagedir()+"zhilo.xyzp";
//   xyzpfunc::write_xyzp_data(
//      zhilo_twoDarray_ptr,cityimage.get_p2Darray_ptr(),zhilo_filename);

   twoDarray* zcompletely_flattened_twoDarray_ptr=
      groundfunc::completely_flatten_ground(
         zlevel_twoDarray_ptr,zhilo_twoDarray_ptr);
   delete zlevel_twoDarray_ptr;

   string flat_filenamestr=cityimage.get_imagedir()+"complete_flat_"
      +cityimage.get_xyz_filenamestr();
   xyzpfunc::write_xyzp_data(
      zcompletely_flattened_twoDarray_ptr,cityimage.get_p2Darray_ptr(),
      flat_filenamestr);
   draw3Dfunc::append_fake_z_points_in_twoDarray_middle(
      cityimage.get_z2Darray_ptr(),flat_filenamestr);

// Set all entries within refined ground mask image corresponding to
// relatively high objects equal to xyzpfunc::null_value:

   twoDarray* groundmask_twoDarray_ptr=new twoDarray(cityimage.get_z2Darray_ptr());
   cityimage.get_z2Darray_ptr()->copy(groundmask_twoDarray_ptr);
   recursivefunc::binary_null(
      1.0,groundmask_twoDarray_ptr,zhilo_twoDarray_ptr,xyzpfunc::null_value,
      true);
   delete zhilo_twoDarray_ptr;
   string groundmask_filename=cityimage.get_imagedir()+"groundmask.xyzp";
   xyzpfunc::write_xyzp_data(
      groundmask_twoDarray_ptr,cityimage.get_p2Darray_ptr(),
      groundmask_filename);
   draw3Dfunc::append_fake_z_points_in_twoDarray_middle(
      cityimage.get_z2Darray_ptr(),groundmask_filename);

// Perform final round of ground extraction using refined version of
// silhouetted ground mask:

   twoDarray* zlevel2_twoDarray_ptr=
      cityimage.interpolate_and_flatten_ground_surface(
         cityimage.get_z2Darray_ptr(),groundmask_twoDarray_ptr,
         cityimage.get_p2Darray_ptr(),3);
   delete groundmask_twoDarray_ptr;
   recursivefunc::binary_null(
      0.5*xyzpfunc::null_value,zlevel2_twoDarray_ptr,
      cityimage.get_z2Darray_ptr(),xyzpfunc::null_value);

   string zlevel2_filename=cityimage.get_imagedir()+"zlevel2.xyzp";
   xyzpfunc::write_xyzp_data(
      zlevel2_twoDarray_ptr,cityimage.get_p2Darray_ptr(),zlevel2_filename);
   draw3Dfunc::append_fake_z_points_in_twoDarray_middle(
      cityimage.get_z2Darray_ptr(),zlevel2_filename);

   zlevel2_twoDarray_ptr->copy(cityimage.get_z2Darray_ptr());
   delete zlevel2_twoDarray_ptr;

// ==========================================================================
// Grass and asphalt pixel detection
// ==========================================================================

/*
// At this stage, the machine should have a very good idea of which
// pixels correspond to low-lying objects (roads & grass) and which
// pixels corresond to tall objects (buildings and trees).  We now use
// p-image information to distinguish the low-lying road and grass
// pixels:

   int nlo_iters=20;
   double p_tall_sentinel_value=1.0;
   twoDarray* ptwoDarray_lo_ptr=
      featurefunc::distinguish_road_and_grass_pixels(
         nlo_iters,cityimage.get_imagedir(),p_tall_sentinel_value,
         zcompletely_flattened_twoDarray_ptr,cityimage.get_p2Darray_ptr());
//   cityimage.writeimage("ptwoDarray_lo",ptwoDarray_lo_ptr,
//                       false,ladarimage::p_data);

   twoDarray* lo_features_twoDarray_ptr=
      featurefunc::classify_road_grass_pixels(
         p_tall_sentinel_value,ptwoDarray_lo_ptr);
   delete ptwoDarray_lo_ptr;

// Try to eliminate small pockets of roadside pixels.  We're much more
// interested in long, connected swaths of roadside:
   
   featurefunc::density_filter_road_content(
      6,0.85,lo_features_twoDarray_ptr);
   string lo_features_filenamestr=cityimage.get_imagedir()+"lo_features.xyzp";
   xyzpfunc::write_xyzp_data(
      cityimage.get_z2Darray_ptr(),lo_features_twoDarray_ptr,
      lo_features_filenamestr,false);
//   cityimage.writeimage("lo_features",lo_features_twoDarray_ptr,
//                       false,ladarimage::p_data);

// ==========================================================================
// Tree and rooftop pixel detection
// ==========================================================================

// The machine should by now have performed a reasonable separation
// between low-lying roadside and grass pixels.  We next want it to
// distinguish tree and buildings pixels.  We first find pixel
// locations where the height function rapidly fluctuates:

   twoDarray* norm_fluc_twoDarray_ptr=
      cityimage.compute_z_fluctuations(cityimage.get_z2Darray_ptr());

// Use height fluctuation along with intensity image information to
// distinguish buildings from trees:

   int nhi_iters=5;
   double p_low_sentinel_value=0.0;
   twoDarray* ptwoDarray_hi_ptr=
      featurefunc::distinguish_tree_from_bldg_pixels(
         nhi_iters,cityimage.get_imagedir(),p_low_sentinel_value,
         zcompletely_flattened_twoDarray_ptr,cityimage.get_p2Darray_ptr(),
         norm_fluc_twoDarray_ptr);
   delete norm_fluc_twoDarray_ptr;

   string p_trees_filenamestr=cityimage.get_imagedir()+"p_trees.xyzp";
   xyzpfunc::write_xyzp_data(
      zcompletely_flattened_twoDarray_ptr,ptwoDarray_hi_ptr,
      p_trees_filenamestr);

   twoDarray* zconnected_components_twoDarray_ptr=
      featurefunc::locate_building_clusters(
         cityimage.get_imagedir(),ptwoDarray_hi_ptr,
         zcompletely_flattened_twoDarray_ptr);
//         cityimage.get_connected_heights_hashtable_ptr());
   delete ptwoDarray_hi_ptr;

   double tall_object_null_value=0.5;
   twoDarray* zbuilding_twoDarray_ptr=new twoDarray(cityimage.get_z2Darray_ptr());
   ladarfunc::mark_tall_clusters(
      zcompletely_flattened_twoDarray_ptr,zconnected_components_twoDarray_ptr,
      zbuilding_twoDarray_ptr,tall_object_null_value);
   delete zconnected_components_twoDarray_ptr;

   string zbldg_filenamestr=cityimage.get_imagedir()+"z_building.xyzp";
   xyzpfunc::write_xyzp_data(
      cityimage.get_z2Darray_ptr(),zbuilding_twoDarray_ptr,zbldg_filenamestr,false);
//   cityimage.writeimage("zbuilding",ftwoDarray_ptr,false);   

// Fuze together flattened height and filtered intensity images:

//   double zmax=40;
//   double zmin=-1;
//   double v_hi=1;
//   double v_lo=0;
//   twoDarray* ftwoDarray_ptr=ladarfunc::fuse_z_and_p_images(
//      v_hi,v_lo,zmax,zmin,zcompletely_flattened_twoDarray_ptr,
//      pmerge_twoDarray_ptr);
//   string fuse_filenamestr=cityimage.get_imagedir()+"fuse_merge_10.xyzp";
//   xyzpfunc::write_xyzp_data(
//      zcompletely_flattened_twoDarray_ptr,ftwoDarray_ptr,
//      fuse_filenamestr,false);
//   delete ftwoDarray_ptr;

//   v_hi=0;
//   v_lo=1;
//   ftwoDarray_ptr=ladarfunc::fuse_z_and_p_images(
//      v_hi,v_lo,zmax,zmin,zcompletely_flattened_twoDarray_ptr,
//      pmerge_twoDarray_ptr);
//   fuse_filenamestr=cityimage.get_imagedir()+"fuse_merge_01.xyzp";
//   xyzpfunc::write_xyzp_data(
//      zcompletely_flattened_twoDarray_ptr,ftwoDarray_ptr,fuse_filenamestr,
//      false);
   delete zcompletely_flattened_twoDarray_ptr;   

// ==========================================================================
// Feature map construction
// ==========================================================================

// Generate features map which indicates trees, buildings, grass and
// roadsides in separate colors:

   twoDarray* features_twoDarray_ptr=
      featurefunc::classify_tree_building_pixels(
         p_tall_sentinel_value,lo_features_twoDarray_ptr,
         zbuilding_twoDarray_ptr);

   string features_filenamestr=cityimage.get_imagedir()+"features.xyzp";
   xyzpfunc::write_xyzp_data(
      cityimage.get_z2Darray_ptr(),features_twoDarray_ptr,features_filenamestr);
//   cityimage.writeimage("features",features_twoDarray_ptr,false);   
   
   delete zbuilding_twoDarray_ptr;
   delete lo_features_twoDarray_ptr;

// ==========================================================================
// Rooftop extraction refinement
// ==========================================================================

// The feature map which has been generated at this point is coarse.
// We need to clean it up in order to find shape information for the
// buildings.  We first generate a binary image which pulls out just
// rooftop information from the feature map:

   twoDarray* pbinary_twoDarray_ptr=new twoDarray(cityimage.get_z2Darray_ptr());
   binaryimagefunc::binary_threshold_for_particular_cutoff(
      urbanimage::building_sentinel_value,
      features_twoDarray_ptr,pbinary_twoDarray_ptr,0);

// Perform a little bit of recursive emptying to eliminate possible
// small strands linking together distinct buildings:

   double diameter=1;	// meters
   double fill_frac_threshold=0.75;
   twoDarray* pbinary_new_twoDarray_ptr=
      binaryimagefunc::binary_density_filter(
         diameter,fill_frac_threshold,pbinary_twoDarray_ptr);
   delete pbinary_twoDarray_ptr;
   recursivefunc::recursive_empty(2,pbinary_new_twoDarray_ptr,false);
   pbinary_twoDarray_ptr=pbinary_new_twoDarray_ptr;

//   string binary_bldg_filename=cityimage.get_imagedir()+"binary_bldg.xyzp";   
//   xyzpfunc::write_xyzp_data(
//      cityimage.get_z2Darray_ptr(),pbinary_new_twoDarray_ptr,binary_bldg_filename,
//      false);

// Generate connected components binary map as well as hashtable
// containing linkedlists of connected rooftop pixels:

   double min_projected_area=0.01;	// meters**2
   twoDarray* pconnected_twoDarray_ptr=ladarfunc::connect_binary_components(
      min_projected_area,pbinary_twoDarray_ptr);

   string connected_filename=cityimage.get_imagedir()+"connected.xyzp";   
   xyzpfunc::write_xyzp_data(
      cityimage.get_z2Darray_ptr(),pconnected_twoDarray_ptr,connected_filename,
      false);

   Hashtable<linkedlist*>* connected_components_hashtable_ptr=
      ladarfunc::generate_connected_binary_components_hashtable(
         min_projected_area,pbinary_twoDarray_ptr);
   delete pbinary_twoDarray_ptr;

// Identify buildings with multi-tiered rooftops:

   vector<pair<int,vector<double> > >* tierred_bldg_ptr=
      featurefunc::detect_tiered_roofs(
         connected_components_hashtable_ptr,cityimage.get_z2Darray_ptr());

// Find COM locations for each connected rooftop location.  Choose
// pixels within linked lists which are highly likely to lie on actual
// rooftops.  Then perform an "oozing" operation to find other pixels
// located nearby the seed pixels which also are very likely to lie on
// individual rooftops:

   twoDarray* p_refined_roof_twoDarray_ptr=
      featurefunc::refine_building_extraction(
         cityimage.get_imagedir(),tierred_bldg_ptr,
         connected_components_hashtable_ptr,
         cityimage.get_z2Darray_ptr(),pconnected_twoDarray_ptr);

   delete tierred_bldg_ptr;
   delete pconnected_twoDarray_ptr;
   connectfunc::delete_connected_hashtable(
      connected_components_hashtable_ptr);

// ==========================================================================
// Feature map refinement
// ==========================================================================

// Update feature map following refinement of building rooftop
// extraction:

   twoDarray* refined_features_twoDarray_ptr=featurefunc::update_feature_map(
      features_twoDarray_ptr,p_refined_roof_twoDarray_ptr);
   delete features_twoDarray_ptr;
   delete p_refined_roof_twoDarray_ptr;

// Eliminate height outliers and remove grass/tree islands within
// asphalt regions:

   featurefunc::remove_isolated_height_outliers_from_feature_map(
      cityimage.get_data_bbox_ptr(),
      cityimage.get_z2Darray_ptr(),refined_features_twoDarray_ptr);
   featurefunc::remove_feature_holes_from_feature_map(
      urbanimage::road_sentinel_value,refined_features_twoDarray_ptr,
      cityimage.get_z2Darray_ptr(),"asphalt",cityimage.get_imagedir());
   featurefunc::remove_feature_holes_from_feature_map(
      urbanimage::building_sentinel_value,refined_features_twoDarray_ptr,
      cityimage.get_z2Darray_ptr(),"building",cityimage.get_imagedir());
   featurefunc::remove_feature_holes_from_feature_map(
      urbanimage::grass_sentinel_value,refined_features_twoDarray_ptr,
      cityimage.get_z2Darray_ptr(),"grass",cityimage.get_imagedir());
   featurefunc::remove_isolated_height_outliers_from_feature_map(
      cityimage.get_data_bbox_ptr(),
      cityimage.get_z2Darray_ptr(),refined_features_twoDarray_ptr);

   string refined_features_filename=cityimage.get_imagedir()+
      "refined_features.xyzp";
   xyzpfunc::write_xyzp_data(
      cityimage.get_z2Darray_ptr(),refined_features_twoDarray_ptr,
      refined_features_filename);
   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
      refined_features_twoDarray_ptr,refined_features_filename);

// Generate network containing linkedlists of connected rooftop
// pixels.  Only include buildings into network whose footprint areas
// exceed min_footprint_area.  Rub out all building pixels within
// refined feature map and then restore just those pixels
// corresponding to sites within the buildings network:

   double min_footprint_area=30;	// meters**2
   cityimage.generate_buildings_network(
      min_footprint_area,cityimage.get_z2Darray_ptr(),
      refined_features_twoDarray_ptr);
   featurefunc::recolor_feature_pixels(
      urbanimage::building_sentinel_value,urbanimage::tree_sentinel_value,
      refined_features_twoDarray_ptr);
   cityimage.draw_building_site_pixels(refined_features_twoDarray_ptr);

// Compute partial derivative dz/dx and dz/dy fields.  Then look for
// "buildings" whose height derivatives in the outward radial
// direction are suspiciously small.  If such "buildings" are
// surrounded by trees, we reclassify them as tree pixel clumps rather
// than as rooftop pixel clumps:

   xderiv_twoDarray_ptr=new twoDarray(cityimage.get_z2Darray_ptr());
   yderiv_twoDarray_ptr=new twoDarray(cityimage.get_z2Darray_ptr());
   imagefunc::compute_x_y_deriv_fields(
      spatial_resolution,cityimage.get_z2Darray_ptr(),xderiv_twoDarray_ptr,
      yderiv_twoDarray_ptr);
   cityimage.check_building_contour_height_variation(
      xderiv_twoDarray_ptr,yderiv_twoDarray_ptr,
      refined_features_twoDarray_ptr);
   delete xderiv_twoDarray_ptr;
   delete yderiv_twoDarray_ptr;

// Set all previously classified building pixels within features map
// to tree color.  Then redraw pixels associated with surviving sites
// in buildings network onto features map:

   featurefunc::recolor_feature_pixels(
      urbanimage::building_sentinel_value,urbanimage::tree_sentinel_value,
      refined_features_twoDarray_ptr);
   cityimage.draw_building_site_pixels(refined_features_twoDarray_ptr);
   string final_features_filename=cityimage.get_imagedir()+"final_features.xyzp";
   xyzpfunc::write_xyzp_data(
      cityimage.get_z2Darray_ptr(),refined_features_twoDarray_ptr,
      final_features_filename);
   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
      cityimage.get_z2Darray_ptr(),final_features_filename);

// On 4/17/04, we wrote a simple yet highly effective method which
// fuses together feature and height information within a single xyzp
// output file.  The fused results look quite striking!

   twoDarray* features_and_heights_twoDarray_ptr=
      urbanfunc::color_feature_heights(
         cityimage.get_z2Darray_ptr(),refined_features_twoDarray_ptr);

   string features_and_heights_filename=cityimage.get_imagedir()+
      "features_and_heights.xyzp";   
   xyzpfunc::write_xyzp_data(
      cityimage.get_z2Darray_ptr(),features_and_heights_twoDarray_ptr,
      features_and_heights_filename);
   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
      features_and_heights_twoDarray_ptr,features_and_heights_filename);


   delete refined_features_twoDarray_ptr;
   delete features_and_heights_twoDarray_ptr;
*/

   cityimage.summarize_results();
   cityimage.update_logfile("ied");
}


