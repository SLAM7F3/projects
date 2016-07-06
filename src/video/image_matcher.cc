// =========================================================================
// Image_Matcher class member function definitions
// =========================================================================
// Last modified on 12/2/12; 5/31/13; 11/21/13
// =========================================================================

#include <iostream>
#include <map>
#include <set>
#include <string>

#include "geometry/affine_transform.h"
#include "math/genmatrix.h"
#include "video/image_matcher.h"
#include "graphs/vptree.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ios;
using std::istream;
using std::map;
using std::ofstream;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void image_matcher::allocate_member_objects()
{
   A_ptr=new genmatrix(2,2);
}

void image_matcher::initialize_member_objects()
{
   XY_vptree_ptr=NULL;
   UV_vptree_ptr=NULL;
}		 

// ---------------------------------------------------------------------
image_matcher::image_matcher(photogroup* photogroup_ptr,bool FLANN_flag):
sift_detector(photogroup_ptr,FLANN_flag)
{
   initialize_member_objects();
   allocate_member_objects();
}

// ---------------------------------------------------------------------
// Copy constructor:

image_matcher::image_matcher(const image_matcher& im) :
   sift_detector(im)
{
//   cout << "inside image_matcher copy constructor, this(image_matcher) = " << this << endl;
   initialize_member_objects();
   allocate_member_objects();
   docopy(im);
}

image_matcher::~image_matcher()
{
//   cout << "inside image_matcher destructor" << endl;
   delete XY_vptree_ptr;
   delete UV_vptree_ptr;
   delete A_ptr;
}

// ---------------------------------------------------------------------
void image_matcher::docopy(const image_matcher& im)
{
//   cout << "inside image_matcher::docopy()" << endl;
//   cout << "this = " << this << endl;
}

// Overload = operator:

image_matcher& image_matcher::operator= (const image_matcher& im)
{
//   cout << "inside image_matcher::operator=" << endl;
//   cout << "this(image_matcher) = " << this << endl;
   if (this==&im) return *this;
   sift_detector::operator=(im);
   docopy(im);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const image_matcher& im)
{
   outstream << endl;
   outstream << (sift_detector&)im << endl;
//   outstream << "Image_Matcher ID = " << e.ID << endl;
   
   return outstream;
}

// =========================================================================
// Set & get member functions
// =========================================================================

// =========================================================================
// Nearby neighbor member functions
// =========================================================================

// Member function reset_inlier_tiepoint_pairs() takes in a
// *features_map_ptr which is assumed to contain feature ID keys and
// an STL vector of fourvectors (which in turn consist of pass numbers
// and twovector feature coordinates).  Iterating over all
// entries in *features_map_ptr, this method fills member STL vectors
// inlier_tiepoint_ID,inlier_XY and inlier_UV with tiepoint pair
// information.

void image_matcher::reset_inlier_tiepoint_pairs(
   FEATURES_MAP* features_map_ptr)
{
//   cout << "inside image_matcher::reset_inlier_tiepoint_pairs()" << endl;
//   cout << "features_map_ptr = " << features_map_ptr << endl;

   get_inlier_tiepoint_ID().clear();
   get_inlier_XY().clear();
   get_inlier_UV().clear();

   for (FEATURES_MAP::const_iterator iter=features_map_ptr->begin();
        iter != features_map_ptr->end(); iter++)
   {
      int feature_ID=iter->first;
//      cout << "Feature_ID = " << feature_ID << endl;
      vector<fourvector> feature_triples=iter->second;
//      cout << "feature_triples.size() = " << feature_triples.size() << endl;
      if (feature_triples.size() <= 1) continue;

      get_inlier_tiepoint_ID().push_back(feature_ID);
      for (unsigned int j=0; j<feature_triples.size(); j++)
      {
         int pass_number=feature_triples[j].get(0);

         if (pass_number==0)
         {
            double X=feature_triples[j].get(1);
            double Y=feature_triples[j].get(2);
            get_inlier_XY().push_back(twovector(X,Y));
         }
         else if (pass_number==1)
         {
            double U=feature_triples[j].get(1);
            double V=feature_triples[j].get(2);
            get_inlier_UV().push_back(twovector(U,V));
         }
      } // loop over index j labeling feature triples
   } // loop over *features_map_ptr iterator

//   cout << "inlier_tiepoint_ID.size() = " << get_inlier_tiepoint_ID().size() 
//        << endl;
//   cout << "inlier_XY.size() = " << get_inlier_XY().size() << endl;
//   cout << "inlier_UV.size() = " << get_inlier_UV().size() << endl;
}

// ---------------------------------------------------------------------
// Member function fill_feature_coords_ID_maps()

void image_matcher::fill_feature_coords_ID_maps()
{
   unsigned int n_tiepoints=get_inlier_tiepoint_ID().size();
   for (unsigned int n=0; n<n_tiepoints; n++)
   {
      twovector curr_XY=get_inlier_XY()[n];
      twovector curr_UV=get_inlier_UV()[n];
      int feature_ID=get_inlier_tiepoint_ID()[n];
      XY_feature_coords_ID_map[curr_XY]=feature_ID;
      UV_feature_coords_ID_map[curr_UV]=feature_ID;
      XY_feature_ID_coords_map[feature_ID]=curr_XY;
      UV_feature_ID_coords_map[feature_ID]=curr_UV;
   }
}

// ---------------------------------------------------------------------
// Member function generate_VP_trees()

void image_matcher::generate_VP_trees()
{
   unsigned int n_tiepoints=get_inlier_XY().size();

   vector<descriptor*> XY_elements,UV_elements;
   XY_elements.reserve(n_tiepoints);
   UV_elements.reserve(n_tiepoints);
   for (unsigned int n=0; n<n_tiepoints; n++)
   {
      descriptor* curr_XY_ptr=new descriptor(2);
      curr_XY_ptr->put(0,get_inlier_XY()[n].get(0));
      curr_XY_ptr->put(1,get_inlier_XY()[n].get(1));

      descriptor* curr_UV_ptr=new descriptor(2);
      curr_UV_ptr->put(0,get_inlier_UV()[n].get(0));
      curr_UV_ptr->put(1,get_inlier_UV()[n].get(1));

      XY_elements.push_back(curr_XY_ptr);
      UV_elements.push_back(curr_UV_ptr);
   }

   XY_vptree_ptr=new vptree;
   UV_vptree_ptr=new vptree;
   XY_vptree_ptr->construct_tree(XY_elements);
   UV_vptree_ptr->construct_tree(UV_elements);
}

// ---------------------------------------------------------------------
// Member function find_nearest_XY_features() takes coordinates for
// some 2D point in the XY image plane.  It computes the locations and
// IDs for the 4 closest features.  The closest features' IDs are
// returned within member STL vector XY_feature_IDs.

vector<int>& image_matcher::find_nearest_XY_features(double X,double Y)
{
   descriptor* query_point_ptr=new descriptor(2);
   query_point_ptr->put(0,X);
   query_point_ptr->put(1,Y);

   int k=4;
   vector<int> nearest_neighbor_node_IDs;
   vector<double> query_to_neighbor_distances;
   vector<descriptor*> metric_space_element_ptrs;

   XY_vptree_ptr->incrementally_find_nearest_nodes(
      k,query_point_ptr,nearest_neighbor_node_IDs,
      query_to_neighbor_distances,metric_space_element_ptrs);

   feature_IDs.clear();
   for (unsigned int n=0; n<nearest_neighbor_node_IDs.size(); n++)
   {
      twovector XY_p(
         metric_space_element_ptrs[n]->get(0),
         metric_space_element_ptrs[n]->get(1));
      feature_coords_ID_iter=XY_feature_coords_ID_map.find(XY_p);
      int XY_feature_ID=feature_coords_ID_iter->second;
      feature_IDs.push_back(XY_feature_ID);

      cout << "n = " << n 
           << " Feature ID = " << XY_feature_ID
           << " X' = " << XY_p.get(0)
           << " Y' = " << XY_p.get(1)
           << " query_to_feature distance = " << query_to_neighbor_distances[n]
           << endl;
   }
   return feature_IDs;
}

// ---------------------------------------------------------------------
// Member function find_nearest_UV_features() takes coordinates for
// some 2D point in the UV image plane.  It computes the locations and
// IDs for the 4 closest features.  The closest features' IDs are
// returned within member STL vector feature_IDs.

vector<int>& image_matcher::find_nearest_UV_features(double U,double V)
{
//   cout << "inside image_matcher::find_nearest_UV_features()" << endl;
   descriptor* query_point_ptr=new descriptor(2);
   query_point_ptr->put(0,U);
   query_point_ptr->put(1,V);

   int k=4;
   vector<int> nearest_neighbor_node_IDs;
   vector<double> query_to_neighbor_distances;
   vector<descriptor*> metric_space_element_ptrs;

   UV_vptree_ptr->incrementally_find_nearest_nodes(
      k,query_point_ptr,nearest_neighbor_node_IDs,
      query_to_neighbor_distances,metric_space_element_ptrs);

   feature_IDs.clear();
   for (unsigned int n=0; n<nearest_neighbor_node_IDs.size(); n++)
   {
      twovector UV_p(
         metric_space_element_ptrs[n]->get(0),
         metric_space_element_ptrs[n]->get(1));
      feature_coords_ID_iter=UV_feature_coords_ID_map.find(UV_p);
      int UV_feature_ID=feature_coords_ID_iter->second;
      feature_IDs.push_back(UV_feature_ID);

      cout << "n = " << n 
           << " Feature ID = " << UV_feature_ID
           << " U' = " << UV_p.get(0)
           << " V' = " << UV_p.get(1)
           << " query_to_feature distance = " << query_to_neighbor_distances[n]
           << endl;
   }
   return feature_IDs;
}

// ---------------------------------------------------------------------
// Member function find_nearest_matching_UV_features() takes in
// feature IDs generated by find_nearest_XY_features().  It fills
// member STL vectors matching_UVs and matching_XYs with corresponding
// 2D coordinates for the tiepoint features in the UV and XY image
// planes.

void image_matcher::find_nearest_matching_UV_features()
{
   matching_UVs.clear();
   matching_XYs.clear();
   for (unsigned int i=0; i<feature_IDs.size(); i++)
   {
      feature_ID_coords_iter=UV_feature_ID_coords_map.find(feature_IDs[i]);
      matching_UVs.push_back(feature_ID_coords_iter->second);
      feature_ID_coords_iter=XY_feature_ID_coords_map.find(feature_IDs[i]);
      matching_XYs.push_back(feature_ID_coords_iter->second);
   }

//   mathfunc::fit_2D_affine_transformation(
//      matching_XYs,matching_UVs,*A_ptr,trans);

   affine_transform at;
   at.parse_affine_transform_inputs(matching_XYs,matching_UVs);
   at.fit_affine_transformation();
//   double RMS_residual=at.check_affine_transformation(
//      curr_SURF_centers,next_SURF_centers);
//   cout << "RMS_residual = " << RMS_residual << endl;
   *A_ptr=*(at.get_A_ptr());
   trans=at.get_trans();
}

// ---------------------------------------------------------------------
// Member function find_nearest_matching_XY_features() takes in
// feature IDs generated by find_nearest_UV_features().  It fills
// member STL vectors matching_XYs and matching_UVs with corresponding
// 2D coordinates for the tiepoint features in the XY and UV image
// planes.

void image_matcher::find_nearest_matching_XY_features()
{
   matching_XYs.clear();
   matching_UVs.clear();
   for (unsigned int i=0; i<feature_IDs.size(); i++)
   {
      feature_ID_coords_iter=XY_feature_ID_coords_map.find(feature_IDs[i]);
      matching_XYs.push_back(feature_ID_coords_iter->second);
      feature_ID_coords_iter=UV_feature_ID_coords_map.find(feature_IDs[i]);
      matching_UVs.push_back(feature_ID_coords_iter->second);
   }

//   mathfunc::fit_2D_affine_transformation(
//      matching_UVs,matching_XYs,*A_ptr,trans);

   affine_transform at;
   at.parse_affine_transform_inputs(matching_UVs,matching_XYs);
   at.fit_affine_transformation();
//   double RMS_residual=at.check_affine_transformation(
//      curr_SURF_centers,next_SURF_centers);
//   cout << "RMS_residual = " << RMS_residual << endl;
   *A_ptr=*(at.get_A_ptr());
   trans=at.get_trans();
}

// ---------------------------------------------------------------------
// Member function find_UV_matching_XY() takes in some 2D point within
// the XY image plane.  It first finds the closest 4 features to the
// input XY point.  This method next retrieves the UV counterpoints to
// the 4 XY features.  After establishing an affine transformation
// from the UV tiepoints, this method computes and returns the fitted
// location within the UV image plane that corresponds to the input XY
// point.

twovector image_matcher::find_UV_matching_XY(const twovector& XY)
{
   find_nearest_XY_features(XY.get(0),XY.get(1));
   find_nearest_matching_UV_features();
   twovector UV=*A_ptr * XY + trans;
   return UV;
}

// ---------------------------------------------------------------------
// Member function find_XY_matching_UV() takes in

twovector image_matcher::find_XY_matching_UV(const twovector& UV)
{
//   cout << "inside image_matcher::find_XY_matching_UV()" << endl;
//   cout << "UV = " << UV << endl;
   find_nearest_UV_features(UV.get(0),UV.get(1));
   find_nearest_matching_XY_features();
//   cout << "*A_ptr = " << *A_ptr << " trans = " << trans << endl;
   twovector XY=*A_ptr * UV + trans;
   
   return XY;
}

