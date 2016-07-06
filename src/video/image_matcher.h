// ==========================================================================
// Header file for image_matcher class
// ==========================================================================
// Last modified on 11/12/12; 12/2/12; 1/30/13
// ==========================================================================

#ifndef IMAGE_MATCHER_H
#define IMAGE_MATCHER_H

#include <map>
#include <vector>
#include "math/lttwovector.h"
#include "video/sift_detector.h"

class genmatrix;
class vptree;

class image_matcher: public sift_detector
{

  public:

   image_matcher(photogroup* photogroup_ptr,bool FLANN_flag=true);
   image_matcher(const image_matcher& im);
   ~image_matcher();
   image_matcher& operator= (const image_matcher& im);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const image_matcher& im);

// Set and get member functions:

//    void set_FLANN_flag(bool flag);

// Nearby neighbor member functions

   void reset_inlier_tiepoint_pairs(FEATURES_MAP* features_map_ptr);
   void fill_feature_coords_ID_maps();
   void generate_VP_trees();
   std::vector<int>& find_nearest_XY_features(double X,double Y);
   std::vector<int>& find_nearest_UV_features(double U,double V);
   void find_nearest_matching_UV_features();
   void find_nearest_matching_XY_features();
   twovector find_UV_matching_XY(const twovector& XY);
   twovector find_XY_matching_UV(const twovector& UV);

  private: 

   typedef std::map<twovector,int,lttwovector> FEATURE_COORDS_ID_MAP;
// Independent twovector = Image plane coordinate for feature
// Dependent integer = Feature ID

   FEATURE_COORDS_ID_MAP XY_feature_coords_ID_map,UV_feature_coords_ID_map;
   FEATURE_COORDS_ID_MAP::iterator feature_coords_ID_iter;

   typedef std::map<int,twovector> FEATURE_ID_COORDS_MAP;

// Independent integer = Feature ID
// Dependent twovector = Image plane coordinate for feature

   FEATURE_ID_COORDS_MAP XY_feature_ID_coords_map,UV_feature_ID_coords_map;
   FEATURE_ID_COORDS_MAP::iterator feature_ID_coords_iter;

   vptree *XY_vptree_ptr,*UV_vptree_ptr;
   std::vector<int> feature_IDs;
   std::vector<twovector> matching_UVs,matching_XYs;

   genmatrix* A_ptr;
   twovector trans;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const image_matcher& im);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

/*
inline void image_matcher::set_FLANN_flag(bool flag)
{
   FLANN_flag=flag;
}
*/


#endif  // image_matcher.h
