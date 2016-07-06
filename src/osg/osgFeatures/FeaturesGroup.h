// ==========================================================================
// Header file for FEATURESGROUP class
// ==========================================================================
// Last modified on 6/19/14; 6/21/14; 7/1/14; 7/5/14
// ==========================================================================

#ifndef FEATURESGROUP_H
#define FEATURESGROUP_H

#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "osg/osgAnnotators/AnnotatorsGroup.h"
#include "osg/CustomManipulator.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "osg/osgFeatures/Feature.h"
#include "math/genmatrix.h"
#include "video/image_matcher.h"
#include "kdtree/kdtree.h"
#include "math/ltduple.h"
#include "math/ltthreevector.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osgModels/OBSFRUSTAGROUP.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGeometry/PointsGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgAnnotators/SignPostsGroup.h"
#include "math/threevector.h"
#include "math/fourvector.h"

class AnimationController;
class CentersGroup;
class Clock;
class Ellipsoid_model;
class LineSegmentsGroup;
template <class T> class Linkedlist;
class photograph;
class photogroup;
class TrianglesGroup;
template <class T> class TwoDarray;
typedef TwoDarray<double> twoDarray;
class VidFile;

class FeaturesGroup : public osgGeometry::PointsGroup, public AnnotatorsGroup
{

  public:

// In order to rapidly construct a features table whose rows
// correspond to distinct feature IDs and whose columns correspond to
// twovector or threevector feature coordinates for distinct passes,
// define the following FEATURES_MAP type:

   typedef std::map<int,std::vector<fourvector> > FEATURES_MAP;

// independent integer = feature_ID

// 2D: Dependent fourvector = (pass_number,U,V,feature_index for curr pass)
// 3D: Dependent fourvector = (pass_number,U,V,W)

   typedef std::map<DUPLE,int,ltduple> THREEDRAYS_MAP;
   
// independent duple: int #1 = photo_ID, int #2 = feature_ID
// dependent integer = color index

   typedef std::map<threevector,std::pair<std::string,std::string>,
      ltthreevector>  MONTAGE_MAP;

// independent threevector: color,gist,texture matching scores
// dependent pair<string>: image1_filename and image2_filename   

// Initialization, constructor and destructor functions:

   FeaturesGroup(
      const int p_ndims,Pass* PI_ptr,
      osgGA::CustomManipulator* CM_ptr,threevector* GO_ptr=NULL);
   FeaturesGroup(
      const int p_ndims,Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,
      Clock* clock_ptr,Ellipsoid_model* EM_ptr,threevector* GO_ptr=NULL);
   FeaturesGroup(
      const int p_ndims,Pass* PI_ptr,
      Movie* movie_ptr,osgGA::CustomManipulator* CM_ptr,
      threevector* GO_ptr=NULL);
   FeaturesGroup(
      const int p_ndims,Pass* PI_ptr,
      CentersGroup* CG_ptr,Movie* movie_ptr,
      osgGA::CustomManipulator* CM_ptr,AnimationController* AC_ptr=NULL,
      threevector* GO_ptr=NULL);
   FeaturesGroup(
      const int p_ndims,Pass* PI_ptr,
      CentersGroup* CG_ptr,Movie* movie_ptr,
      osgGA::CustomManipulator* CM_ptr,LineSegmentsGroup* LSG_ptr,
      threevector* GO_ptr=NULL);
   FeaturesGroup(
      const int p_ndims,Pass* PI_ptr,
      CentersGroup* CG_ptr,Movie* movie_ptr,
      osgGA::CustomManipulator* CM_ptr,TrianglesGroup* TG_ptr,
      LineSegmentsGroup* LSG_ptr=NULL,AnimationController* AC_ptr=NULL,
      threevector* GO_ptr=NULL);
   FeaturesGroup(
      const int p_ndims,Pass* PI_ptr,
      osgGA::CustomManipulator* CM_ptr,TrianglesGroup* TG_ptr,
      threevector* GO_ptr=NULL);
   virtual ~FeaturesGroup();

   friend std::ostream& operator<< 
      (std::ostream& outstream,const FeaturesGroup& f);

// Set & get methods:

   void set_display_selected_bbox_flag(bool flag);
   bool get_display_selected_bbox_flag() const;
   void set_display_feature_pair_bboxes_flag(bool flag);
   bool get_display_feature_pair_bboxes_flag() const;

   void set_display_feature_scores_flag(bool flag);
   bool get_display_feature_scores_flag() const;
   void set_display_geocoords_flag(bool flag);
   void set_display_range_alt_flag(bool flag);
   void set_UV_image_flag(bool flag);
   void set_dragging_feature_flag(bool flag);
   bool get_dragging_feature_flag() const;

   void set_bbox_sidelength(int s);
   void set_minimum_ground_height(double Z_min);
   void set_maximum_raytrace_range(double max_range);
   void set_raytrace_stepsize(double ds);
   Feature* get_Feature_ptr(int n) const;
   Feature* get_ID_labeled_Feature_ptr(int ID) const;
   Linkedlist<int>* get_marked_feature_list_ptr();
   const Linkedlist<int>* get_marked_feature_list_ptr() const;

   void set_EarthRegionsGroup_ptr(EarthRegionsGroup* ERG_ptr);
   void set_counterpart_FeaturesGroup_2D_ptr(FeaturesGroup* FG_2D_ptr);
   void set_FeaturesGroup_3D_ptr(FeaturesGroup* FG_3D_ptr);
   void set_fundamental_matrix_ptr(genmatrix* F_ptr);
   void set_montage_map_ptr(MONTAGE_MAP* m_ptr);
   void set_MoviesGroup_ptr(MoviesGroup* MoviesGroup_ptr);
   void set_OBSFRUSTAGROUP_ptr(OBSFRUSTAGROUP* OG_ptr);
   void set_photogroup_ptr(photogroup* pg_ptr);
   void set_PointFinder_ptr(PointFinder* PointFinder_ptr);
   void set_PolyLinesGroup_ptr(PolyLinesGroup* PLG_ptr);
   void set_SignPostsGroup_ptr(SignPostsGroup* SPG_ptr);
   void set_TrianglesGroup_ptr(TrianglesGroup* TG_ptr);

   genmatrix* get_ntiepoints_matrix_ptr();
   void set_DTED_ztwoDarray_ptr(twoDarray* ztwoDarray_ptr);
   void set_image_matcher_ptr(image_matcher* image_matcher_ptr);
   FEATURES_MAP* get_features_map_ptr();
   const FEATURES_MAP* get_features_map_ptr() const;

   Movie* const get_Movie_ptr();

// Feature creation and manipulation methods:

   Feature* generate_new_Feature(bool earth_feature_flag);
   Feature* generate_new_Feature(
      int ID=-1,int OSGsubPAT_number=0,bool earth_feature_flag=false);
   Feature* minimal_generate_new_Feature(int ID=-1);
   void edit_feature_label();
   bool erase_feature();
   bool unerase_feature();
   void mark_feature(int curr_ID);

   int destroy_feature(bool update_text_flag=true);
   bool destroy_feature(int Feature_ID);
   void destroy_all_Features();
   void change_size(double factor);
   void move_z(int sgn);

   void update_display();

// Instantaneous observation manipulation member functions:

   void consolidate_feature_coords(
      double curr_t,double other_t,FeaturesGroup* other_FeaturesGroup_ptr);

// Photograph feature member functions:

   int read_in_photo_features(
      photogroup* photogroup_ptr,std::string subdir,
      bool display_OSG_features_flag=false,
      bool hide_singleton_features_flag=false);
   void generate_features_map_for_photos(
      FEATURES_MAP* features_map_ptr,
      photogroup* photogroup_ptr,std::string subdir);
   void reorder_passes_to_maximize_tiepoint_overlap(
      double t,photogroup* photogroup_ptr,bool temporal_ordering_flag=false);
   genmatrix* count_tiepoints(double t,photogroup* photogroup_ptr);

   bool photo_feature_overlap(int i, int j);
   std::vector<int> photo_feature_overlap(
      unsigned int p, unsigned int q_start, 
      unsigned int q_stop, int n_qvalues);
   void convert_2D_coords_to_3D_rays(photogroup* photogroup_ptr);
   void convert_2D_coords_to_3D_rays(photograph* photograph_ptr);
      
// Feature output methods:

   void write_feature_html_file();
   void write_feature_html_file(unsigned int n_columns);
   void write_feature_html_file(double t,unsigned int n_columns);
   void write_feature_html_file(
      photogroup* photogroup_ptr,bool output_only_multicoord_features_flag,
      bool output_3D_rays_flag=false);
   void write_feature_html_file(
      double t,const std::vector<std::string>& column_labels,
      bool output_only_multicoord_features_flag=false,
      std::string output_html_filename="features_obs.html");
   void write_GPUSIFT_feature_file(double t);

// Ascii feature file I/O methods:

   std::string new_save_feature_info_to_file();
   std::string save_feature_info_to_file();
   std::string save_feature_info_to_file(std::string output_filename);
   std::string read_info_from_file(
      bool query_passnumber_conversion_flag=false);
   std::string read_feature_info_from_file(
      bool query_passnumber_conversion_flag=false);
   std::string read_feature_info_from_file(
      std::string input_filename,bool query_passnumber_conversion_flag=false);
   std::string read_feature_info_from_file(
      std::string input_filename,std::vector<int>& pass_number,
      bool query_passnumber_conversion_flag=false);
   
// KLT tracking methods:

   void prune_short_track_features();
   void center_imagery_on_selected_feature();
   void update_image_appearances();
   void update_feature_scores();
   void erase_unmarked_features();
   void renumber_all_features();
   void shift_all_feature_IDs(int delta_ID);
   void compute_avg_feature_offsets(
      bool temporally_filter_avg_offsets_flag=true);
   void compute_avg_feature_offsets_from_midpass_location(
      bool temporally_filter_avg_offsets_flag=true);
   void temporally_filter_stabilized_features();
   void temporally_filter_feature(
      int feature_ID,int zeroth_feature_number,std::vector<threevector>& UVW);
   void stabilize_video_imagery();
   void KLT_wander_test();
   void find_zero_value_feature(
      VidFile* VidFile_ptr,unsigned char* data_ptr,Feature* Feature_ptr);

// Triangulation methods:

   void Delaunay_triangulate();

// Homography methods:

   std::vector<int> common_unerased_feature_IDs(double t1,double t2);
   void generate_feature_kdtree(double t1,double t2);
   void find_nearby_KLT_features(
      const threevector& UV1,std::vector<threevector>& closest_curr_node);
   int compare_nearby_KLT_features(
      const std::vector<threevector>& closest_curr_node,
      const std::vector<threevector>& closest_orig_node);
   bool propagate_feature(
      double t1,double t2,const threevector& UV1,
      const std::vector<threevector>& closest_orig_node,threevector& UV2);
   void propagate_feature_over_pass(Feature* Feature_ptr);

// Feature scoring methods:

   void compute_features_scores_and_adjust_their_locations();
   void compute_features_scores_and_adjust_their_locations(
      unsigned int imagenumber_start,unsigned int imagenumber_stop,
      unsigned int delta_imagenumber,int dp_start,int dp_subsequent);
   double evaluate_feature_score(
      unsigned int px,unsigned int py,unsigned int n_size,
      double deriv_filter[],
      double second_deriv_filter[],twoDarray const *intensity_twoDarray_ptr);

// 2D parallelogram methods:

   void fit_features_to_parallelogram();

// Ray backprojection member functions:

   void backproject_rays(
      double t,int feature_ID,
      OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr,unsigned int n_video_passes);
   bool backproject_2D_features_into_3D();
   bool backproject_selected_photo_features_into_3D();
   bool backproject_selected_photo_features_as_3D_rays();
   bool display_3D_features_in_selected_photo();
   void compute_multi_line_intersection();

   void draw_backprojected_2D_features(
      std::string consolidated_features_filename);

// Fundamental matrix display member functions:

   void update_epipolar_lines();
   void update_feature_tiepoints();

// 3D point picking member functions

   void display_montage_corresponding_to_selected_3D_feature_point();
   std::vector<threevector> retrieve_3D_feature_coords();

// Feature chip member functions:

   void display_selected_feature_bbox();
   void display_feature_pair_bboxes();
   void extract_feature_chips(
      unsigned int chip_size,std::string chips_subdir,std::string chip_prefix,
      int chip_offset,int start_feature_ID,int stop_feature_ID=-1);

   void extract_feature_pair_bboxes();
   void renormalize_feature_coords();
   void extract_feature_chips(
      std::string chips_subdir,std::string chip_prefix);

  protected:

  private:

   bool display_selected_bbox_flag,display_feature_pair_bboxes_flag;
   bool display_geocoords_flag,display_range_alt_flag,UV_image_flag;
   bool stabilize_imagery_flag,center_image_on_selected_feature_flag;
   bool dragging_feature_flag;
   unsigned int centered_feature_number,prev_imagenumber;
   int min_unpropagated_feature_ID,max_unpropagated_feature_ID;
   int bbox_sidelength;
   double minimum_ground_height,maximum_raytrace_range,raytrace_stepsize;

   bool display_image_appearances_flag;
   bool display_feature_scores_flag;
   bool erase_unmarked_features_flag;
   double crosshairs_size[4];
   double crosshairs_text_size[4]; 
   Linkedlist<int>* marked_feature_list_ptr;

   osg::ref_ptr<osgGA::CustomManipulator> CustomManipulator_refptr;

   CentersGroup* CentersGroup_ptr;
   EarthRegionsGroup* EarthRegionsGroup_ptr;
   FeaturesGroup *FeaturesGroup_3D_ptr,*counterpart_FeaturesGroup_2D_ptr;
   genmatrix* fundamental_ptr;
   image_matcher* image_matcher_ptr;
   LineSegmentsGroup* LineSegmentsGroup_ptr;
   MONTAGE_MAP* montage_map_ptr;
   MoviesGroup* MoviesGroup_ptr;
   Movie* Movie_ptr;
   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr;
   photogroup* photogroup_ptr;
   PointFinder* PointFinder_ptr;
   PolyLinesGroup* PolyLinesGroup_ptr;
   SignPostsGroup* SignPostsGroup_ptr;
   THREEDRAYS_MAP threeDrays_map;
   TrianglesGroup* TrianglesGroup_ptr;

   KDTree::KDTree<2, threevector>* kdtree_ptr;
   genmatrix* ntiepoints_matrix_ptr;
   std::map<int,int> tiepoint_map;
   twoDarray* DTED_ztwoDarray_ptr; // just pointer, not object
   FEATURES_MAP* features_map_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const FeaturesGroup& f);

   osgGA::CustomManipulator* const get_CustomManipulator_ptr();

   void initialize_new_Feature(
      Feature* curr_Feature_ptr,int OSGsubPAT_number=0);
   int accumulate_feature_info(
      FEATURES_MAP* features_map_ptr,
      bool display_OSG_features_flag=false,
      bool hide_singleton_features_flag=false,
      int individual_pass_number=-1);

   int count_tiepoints(double t,int passnumber_1,int passnumber_2);
   void compute_pass_compositing_order(double t,photogroup* photogroup_ptr,
                                       bool temporal_ordering_flag=false);
   void generate_ntiepoints_matrix_ptr(genmatrix* M_ptr);
   std::string update_feature_label(
      const threevector& closest_worldspace_point,
      const threevector& camera_posn);
   std::string update_geocoords_label(double range,double altitude);
   std::string update_range_label(double longitude,double latitude);
      
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void FeaturesGroup::set_display_selected_bbox_flag(bool flag)
{
   display_selected_bbox_flag=flag;
}

inline bool FeaturesGroup::get_display_selected_bbox_flag() const
{
   return display_selected_bbox_flag;
}

inline void FeaturesGroup::set_display_feature_pair_bboxes_flag(bool flag)
{
   display_feature_pair_bboxes_flag=flag;
}

inline bool FeaturesGroup::get_display_feature_pair_bboxes_flag() const
{
   return display_feature_pair_bboxes_flag;
}

inline void FeaturesGroup::set_display_feature_scores_flag(bool flag)
{
   display_feature_scores_flag=flag;
}

inline bool FeaturesGroup::get_display_feature_scores_flag() const
{
   return display_feature_scores_flag;
}

inline void FeaturesGroup::set_display_geocoords_flag(bool flag)
{
   display_geocoords_flag=flag;
}

inline void FeaturesGroup::set_display_range_alt_flag(bool flag)
{
   display_range_alt_flag=flag;
}

inline void FeaturesGroup::set_UV_image_flag(bool flag)
{
   UV_image_flag=flag;
}

inline void FeaturesGroup::set_dragging_feature_flag(bool flag)
{
   dragging_feature_flag=flag;
}

inline bool FeaturesGroup::get_dragging_feature_flag() const
{
   return dragging_feature_flag;
}

// --------------------------------------------------------------------------
inline void FeaturesGroup::set_bbox_sidelength(int s)
{
   bbox_sidelength=s;
}

// ---------------------------------------------------------------------
inline void FeaturesGroup::set_minimum_ground_height(double Z_min)
{
   minimum_ground_height=Z_min;
}

// ---------------------------------------------------------------------
inline void FeaturesGroup::set_maximum_raytrace_range(double max_range)
{
   maximum_raytrace_range=max_range;
}

// ---------------------------------------------------------------------
inline void FeaturesGroup::set_raytrace_stepsize(double ds)
{
   raytrace_stepsize=ds;
}


// --------------------------------------------------------------------------
inline Feature* FeaturesGroup::get_Feature_ptr(int n) const
{
   return dynamic_cast<Feature*>(get_Graphical_ptr(n));
}

// --------------------------------------------------------------------------
inline Feature* FeaturesGroup::get_ID_labeled_Feature_ptr(int ID) const
{
   return dynamic_cast<Feature*>(get_ID_labeled_Graphical_ptr(ID));
}

// ---------------------------------------------------------------------
inline Movie* const FeaturesGroup::get_Movie_ptr()
{
   return Movie_ptr;
}

// ---------------------------------------------------------------------
inline osgGA::CustomManipulator* const FeaturesGroup::
get_CustomManipulator_ptr()
{
   return CustomManipulator_refptr.get();
}

// ---------------------------------------------------------------------
inline Linkedlist<int>* FeaturesGroup::get_marked_feature_list_ptr()
{
   return marked_feature_list_ptr;
}

inline const Linkedlist<int>* FeaturesGroup::get_marked_feature_list_ptr() 
   const
{
   return marked_feature_list_ptr;
}

// ---------------------------------------------------------------------
inline void FeaturesGroup::set_EarthRegionsGroup_ptr(
   EarthRegionsGroup* ERG_ptr)
{
   EarthRegionsGroup_ptr=ERG_ptr;
}

// ---------------------------------------------------------------------
inline void FeaturesGroup::set_FeaturesGroup_3D_ptr(FeaturesGroup* FG_3D_ptr)
{
   FeaturesGroup_3D_ptr=FG_3D_ptr;
}

inline void FeaturesGroup::set_counterpart_FeaturesGroup_2D_ptr(
   FeaturesGroup* FG_2D_ptr)
{
   counterpart_FeaturesGroup_2D_ptr=FG_2D_ptr;
}

// ---------------------------------------------------------------------
inline void FeaturesGroup::set_fundamental_matrix_ptr(genmatrix* F_ptr)
{
   fundamental_ptr=F_ptr;
}

// --------------------------------------------------------------------------
inline void FeaturesGroup::set_montage_map_ptr(MONTAGE_MAP* m_ptr)
{
   montage_map_ptr=m_ptr;
}

// --------------------------------------------------------------------------
inline void FeaturesGroup::set_MoviesGroup_ptr(MoviesGroup* MoviesGroup_ptr)
{
   this->MoviesGroup_ptr=MoviesGroup_ptr;
}

// ---------------------------------------------------------------------
inline void FeaturesGroup::set_OBSFRUSTAGROUP_ptr(OBSFRUSTAGROUP* OG_ptr)
{
   OBSFRUSTAGROUP_ptr=OG_ptr;
}


// --------------------------------------------------------------------------
inline void FeaturesGroup::set_photogroup_ptr(photogroup* pg_ptr)
{
   photogroup_ptr=pg_ptr;
}

// --------------------------------------------------------------------------
inline void FeaturesGroup::set_PointFinder_ptr(PointFinder* PointFinder_ptr)
{
   this->PointFinder_ptr=PointFinder_ptr;
}

// ---------------------------------------------------------------------
inline void FeaturesGroup::set_PolyLinesGroup_ptr(PolyLinesGroup* PLG_ptr)
{
   PolyLinesGroup_ptr=PLG_ptr;
}

// ---------------------------------------------------------------------
inline void FeaturesGroup::set_SignPostsGroup_ptr(SignPostsGroup* SPG_ptr)
{
   SignPostsGroup_ptr=SPG_ptr;
}

// ---------------------------------------------------------------------
inline void FeaturesGroup::set_TrianglesGroup_ptr(TrianglesGroup* TG_ptr)
{
   TrianglesGroup_ptr=TG_ptr;
}

// ---------------------------------------------------------------------
inline genmatrix* FeaturesGroup::get_ntiepoints_matrix_ptr()
{
   return ntiepoints_matrix_ptr;
}

inline void FeaturesGroup::generate_ntiepoints_matrix_ptr(genmatrix* M_ptr)
{
   std::string banner="Generating ntiepoints matrix";
   outputfunc::write_banner(banner);
   ntiepoints_matrix_ptr=new genmatrix(*M_ptr);
//   std::cout << "inside FG::generate_ntiepoints_matrix_ptr()" << std::endl;
//   std::cout << "*ntiepoints_matrix_ptr = "
//             << *ntiepoints_matrix_ptr << std::endl;
}

// ---------------------------------------------------------------------
inline void FeaturesGroup::set_DTED_ztwoDarray_ptr(twoDarray* ztwoDarray_ptr)
{
   DTED_ztwoDarray_ptr=ztwoDarray_ptr;
}

// ---------------------------------------------------------------------
inline void FeaturesGroup::set_image_matcher_ptr(
   image_matcher* image_matcher_ptr)
{
   this->image_matcher_ptr=image_matcher_ptr;
}

// ---------------------------------------------------------------------
inline FeaturesGroup::FEATURES_MAP* FeaturesGroup::get_features_map_ptr()
{
   return features_map_ptr;
}

inline const FeaturesGroup::FEATURES_MAP* FeaturesGroup::get_features_map_ptr() const
{
   return features_map_ptr;
}

#endif // FeaturesGroup.h



